#include "ForCompare_MutohGA.h"

/*	- estimate 4 thong so cho affine: a1, a4, b1, b2. Su dung truc tiep so thuc
	- fitness function: tinh tren compensation error
	  dung fitness predictor giong nhu trong bai bao
	- selection: pair-wise tournament
	- crossover: multiple crossover per couple (MCPC), uniform
	- mutation: bit-wise mutation */

ForCompare_MutohGA::ForCompare_MutohGA(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, 
										double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->popSize = popSize;	this->halfPopSize = popSize/2;
	this->pc = pc;
	this->pm = pm;

	this->aRangeStart = aRangeStart;	this->aRangeEnd = aRangeEnd;		this->aRange = int((aRangeEnd - aRangeStart) * MUL_FACTOR_10K + 1);
	this->bRangeStart = bRangeStart;	this->bRangeEnd = bRangeEnd;		this->bRange = int((bRangeEnd - bRangeStart) * MUL_FACTOR_10K + 1);
}

ForCompare_MutohGA::~ForCompare_MutohGA(void) {
	for (int i = 0; i < popSize; i++)	delete chromosomes[i];		delete chromosomes;
	delete[] errors;
}

double ForCompare_MutohGA::getBestA11()			{	return chromosomes[minErrorIndex][0];	}
double ForCompare_MutohGA::getBestA22()			{	return chromosomes[minErrorIndex][1];	}
double ForCompare_MutohGA::getBestB1()			{	return chromosomes[minErrorIndex][2];	}
double ForCompare_MutohGA::getBestB2()			{	return chromosomes[minErrorIndex][3];	}
double ForCompare_MutohGA::getBestError()		{	return errors[minErrorIndex];			}
double** ForCompare_MutohGA::getChromosomes()	{	return chromosomes;						}

// --------------------------------------------------------------------------------------

void ForCompare_MutohGA::init() {	
	this->chromosomes = new double*[popSize]; 
	for (int i = 0; i < popSize; i++)	this->chromosomes[i] = new double[N_ITEM];
	
	this->errors = new double[popSize]; 

	// tao ra seed dung cho ngau nhien
	__int64 timeForRandSeed;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
	unsigned int t = (unsigned int)(timeForRandSeed % 1000);
	srand(t);
}

void ForCompare_MutohGA::createPopulation() {
	for (int i = 0; i < popSize; i++) {	
		chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

double ForCompare_MutohGA::fitness() { // is the compensation error
	double errorMin = 1000;
	for (int i = 0; i < popSize; i++) {	
		errors[i] = Util::calculateAvgIntensityWrtAffine(previousImg, currentImg, chromosomes[i][0], chromosomes[i][1], chromosomes[i][2], chromosomes[i][3]);
		if (errorMin > errors[i]) {
			errorMin = errors[i];
			minErrorIndex = i;	// used to get the smallest error
		}
	}
	return errorMin;
}

void ForCompare_MutohGA::selection() {
	for (int i = 0; i < popSize; i++) {
		int randN = rand() % popSize;
		if (errors[i] > errors[randN]) {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[i][j] = chromosomes[randN][j];
		} else {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[randN][j] = chromosomes[i][j];
		}
	}
}

void ForCompare_MutohGA::crossover() { // co them buoc Multiple Crossover Per Couple with Prediction (MCPCP)
	double* bestChr1 = new double[N_ITEM];
	double* bestChr2 = new double[N_ITEM];	

	for (int i = 0; i < halfPopSize; i++) {
		double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K; 

		if (randN < pc) {
			int r = i + halfPopSize;//rand() % halfPopSize + halfPopSize;
			findTwoBestChromosomes(chromosomes[i], chromosomes[r], bestChr1, bestChr2); // chon 2 chro tot nhat trong so chro phat sinh ra tu Parent1 va Parent2

			for (int j = 0; j < N_ITEM; j++) {
				chromosomes[i][j] = bestChr1[j];
				chromosomes[r][j] = bestChr2[j];
			}
		}
	}

	// release
	delete bestChr1;
	delete bestChr2;
}

void ForCompare_MutohGA::mutation() {
	for (int i = 0; i < popSize; i++) {
		double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;		if (randN < this->pm) 	chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

void ForCompare_MutohGA::exchangeChromosomes(double** toChromosomes) {
	for (int i = 0; i < popSize; i++) {
		double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;
		if (randN < this->pc) {
			for (int j = 0; j < N_ITEM; j++) {
				double tmp = chromosomes[i][j];
				chromosomes[i][j] = toChromosomes[i][j];
				toChromosomes[i][j] = tmp;
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------------------

void ForCompare_MutohGA::findTwoBestChromosomes(double* srcChr1, double* srcChr2, double* bestChr1, double* bestChr2) {
	double A = srcChr1[0], B = srcChr1[1], C = srcChr1[2], D = srcChr1[3];
	double E = srcChr2[0], F = srcChr2[1], G = srcChr2[2], H = srcChr2[3];

	int nChildren = 16;
	double** chr = new double*[nChildren];		for (int i = 0; i < nChildren; i++) chr[i] = new double[N_ITEM];
	double* errors = new double[nChildren];

	CvMemStorage* estimatedErrorsStorage = cvCreateMemStorage(0);
	CvSeq* estimatedErrors = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(double), estimatedErrorsStorage);

	chr[0][0] = A;	chr[0][1] = B;	chr[0][2] = C;	chr[0][3] = D;		double e = estimateError(chr[0]);		errors[0] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[1][0] = E;	chr[1][1] = F;	chr[1][2] = G;	chr[1][3] = H;		e = estimateError(chr[1]);				errors[1] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[2][0] = A;	chr[2][1] = F;	chr[2][2] = C;	chr[2][3] = D;		e = estimateError(chr[2]);				errors[2] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[3][0] = E;	chr[3][1] = B;	chr[3][2] = G;	chr[3][3] = H;		e = estimateError(chr[3]);				errors[3] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[4][0] = A;	chr[4][1] = B;	chr[4][2] = G;	chr[4][3] = D;		e = estimateError(chr[4]);				errors[4] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[5][0] = E;	chr[5][1] = F;	chr[5][2] = C;	chr[5][3] = H;		e = estimateError(chr[5]);				errors[5] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[6][0] = A;	chr[6][1] = B;	chr[6][2] = C;	chr[6][3] = H;		e = estimateError(chr[6]);				errors[6] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[7][0] = E;	chr[7][1] = F;	chr[7][2] = G;	chr[7][3] = D;		e = estimateError(chr[7]);				errors[7] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[8][0] = E;	chr[8][1] = B;	chr[8][2] = C;	chr[8][3] = D;		e = estimateError(chr[8]);				errors[8] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[9][0] = A;	chr[9][1] = F;	chr[9][2] = G;	chr[9][3] = H;		e = estimateError(chr[9]);				errors[9] =	e;		cvSeqPush(estimatedErrors, &e); 
	chr[10][0] = E;	chr[10][1] = F;	chr[10][2] = C;	chr[10][3] = D;		e = estimateError(chr[10]);				errors[10] = e;		cvSeqPush(estimatedErrors, &e); 
	chr[11][0] = A;	chr[11][1] = B;	chr[11][2] = G;	chr[11][3] = H;		e = estimateError(chr[11]);				errors[11] = e;		cvSeqPush(estimatedErrors, &e); 
	chr[12][0] = E;	chr[12][1] = B;	chr[12][2] = G;	chr[12][3] = D;		e = estimateError(chr[12]);				errors[12] = e;		cvSeqPush(estimatedErrors, &e); 
	chr[13][0] = A;	chr[13][1] = F;	chr[13][2] = C;	chr[13][3] = H;		e = estimateError(chr[13]);				errors[13] = e;		cvSeqPush(estimatedErrors, &e); 
	chr[14][0] = E;	chr[14][1] = B;	chr[14][2] = C;	chr[14][3] = H;		e = estimateError(chr[14]);				errors[14] = e;		cvSeqPush(estimatedErrors, &e); 
	chr[15][0] = A;	chr[15][1] = F;	chr[15][2] = G;	chr[15][3] = D;		e = estimateError(chr[15]);				errors[15] = e;		cvSeqPush(estimatedErrors, &e); 
	
	cvSeqSort(estimatedErrors, cmp_func);
	double* minError0 = (double *)cvGetSeqElem(estimatedErrors, 0);
	double* minError1 = (double *)cvGetSeqElem(estimatedErrors, 1);

	for (int i = 0; i < nChildren; i++)	{
		if (*minError0 == errors[i]) {
			for (int j = 0; j < N_ITEM; j++) bestChr1[j] = chr[i][j];
			break;
		} 		
	}
	for (int i = 0; i < nChildren; i++)	{
		if (*minError1 == errors[i]) {
			for (int j = 0; j < N_ITEM; j++) bestChr2[j] = chr[i][j];
			break;
		} 
	}

	// release
	for (int i = 0; i < nChildren; i++)		delete chr[i];		delete chr;
	delete errors;
	cvClearSeq(estimatedErrors);
}

double ForCompare_MutohGA::estimateError(double* chr) {
	double totalSimi = totalSimilarity(chr);
	double estimatedError = 0;
	for (int i = 0; i < popSize; i++) 	estimatedError += weight(chr, chromosomes[i], totalSimi) * errors[i];
	return estimatedError;
}

double ForCompare_MutohGA::weight(double* chr1, double* chr2, double totalSimi) {
	return similarity(chr1, chr2) / totalSimi;
}

double ForCompare_MutohGA::totalSimilarity(double* chr) {
	double totalSimi = 0;
	for (int i = 0; i < popSize; i++) 	totalSimi += similarity(chr, chromosomes[i]);
	return totalSimi;
}

double ForCompare_MutohGA::similarity(double* chr1, double* chr2) {
	double distance = euclideanDistance(chr1, chr2);
	if (distance == 0) return 1000000;
	else return 1.0 / distance;
}

double ForCompare_MutohGA::euclideanDistance(double* chr1, double* chr2) {	
	double d0 = chr1[0] - chr2[0];
	double d1 = chr1[1] - chr2[1];
	double d2 = chr1[2] - chr2[2];
	double d3 = chr1[3] - chr2[3];
	return sqrt(d0*d0 + d1*d1 + d2*d2 + d3*d3);
}
