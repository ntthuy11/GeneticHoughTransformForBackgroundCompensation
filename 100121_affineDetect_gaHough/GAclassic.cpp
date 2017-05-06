#include "GAclassic.h"

GAclassic::GAclassic(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, 
					 double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->popSize = popSize;	this->halfPopSize = popSize/2;
	this->pc = pc;
	this->pm = pm;

	this->aRangeStart = aRangeStart;	this->aRangeEnd = aRangeEnd;		this->aRange = int((aRangeEnd - aRangeStart) * MUL_FACTOR_10K + 1);
	this->bRangeStart = bRangeStart;	this->bRangeEnd = bRangeEnd;		this->bRange = int((bRangeEnd - bRangeStart) * MUL_FACTOR_10K + 1);

	this->nExistChromosome = 0;

	// dung cho fitness prediction
	similaritiesStorage = cvCreateMemStorage(0);			similarities = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(double), similaritiesStorage);
	sortedSimilaritiesStorage = cvCreateMemStorage(0);		sortedSimilarities = cvCreateSeq(CV_SEQ_ELTYPE_GENERIC, sizeof(CvSeq), sizeof(double), sortedSimilaritiesStorage);
}

GAclassic::~GAclassic(void) {
	for (int i = 0; i < popSize; i++)	delete chromosomes[i];		delete chromosomes;
	delete[] errors;

	// dung cho fitness prediction
	cvReleaseMemStorage(&similaritiesStorage);
	cvReleaseMemStorage(&sortedSimilaritiesStorage);
}

double GAclassic::getBestA11()			{	return chromosomes[minErrorIndex][0];	}
double GAclassic::getBestA22()			{	return chromosomes[minErrorIndex][1];	}
double GAclassic::getBestB1()			{	return chromosomes[minErrorIndex][2];	}
double GAclassic::getBestB2()			{	return chromosomes[minErrorIndex][3];	}
double GAclassic::getBestError()		{	return errors[minErrorIndex];			}
double** GAclassic::getChromosomes()	{	return chromosomes;						}

// --------------------------------------------------------------------------------------

void GAclassic::init() {
	this->chromosomes = new double*[popSize]; // assign chromosomes
	for (int i = 0; i < popSize; i++)	this->chromosomes[i] = new double[N_ITEM];
	
	this->errors = new double[popSize]; 

	// tao ra seed dung cho ngau nhien
	/*__int64 timeForRandSeed;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
	unsigned int t = (unsigned int)(timeForRandSeed % 1000);
	srand(t);*/
}

void GAclassic::insertChromosome(double a11, double a22, double b1, double b2) {
	//if(nExistChromosome < popSize) {
		chromosomes[nExistChromosome][0] = a11;
		chromosomes[nExistChromosome][1] = a22;
		chromosomes[nExistChromosome][2] = b1;
		chromosomes[nExistChromosome][3] = b2;
		nExistChromosome++;
	//}
}

double GAclassic::fitness() { // is the compensation error
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

void GAclassic::selection() {
	for (int i = 0; i < popSize; i++) {
		int randN = rand() % popSize;
		if (errors[i] > errors[randN]) {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[i][j] = chromosomes[randN][j];
		} else {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[randN][j] = chromosomes[i][j];
		}
	}
}

void GAclassic::crossover() {
	for (int i = 0; i < halfPopSize; i++) {
		for (int j = 0; j < N_ITEM; j++) {
			double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;
			if (randN < this->pc) {
				int halfPopSize_i = i + halfPopSize;
				double tmp = chromosomes[i][j];
				chromosomes[i][j] = chromosomes[halfPopSize_i][j];
				chromosomes[halfPopSize_i][j] = tmp;
			}
		}
	}
}

void GAclassic::mutation() {
	for (int i = 0; i < popSize; i++) {
		double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;		if (randN < this->pm) 	chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

void GAclassic::exchangeChromosomes(double** toChromosomes) {
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

// -------------------------------------------------------------------

void GAclassic::interpolateErrors() {
	double errorMin = 1000;
	for (int i = 0; i < popSize; i++) {	
		errors[i] = estimateError(chromosomes[i]);
		if (errorMin > errors[i]) {
			errorMin = errors[i];
			minErrorIndex = i;	// used to get the smallest error
		}
	}
}

double GAclassic::estimateError(double* chr) {
	similarityBtwOneChrWithPopulation(chr); // luu nhung similarities giua 1 chr voi nhung chr trong population
	sortedSimilarities = cvCloneSeq(similarities, sortedSimilaritiesStorage); // copy list cac similarities nay ra 1 list khac de sap xep
	cvSeqSort(sortedSimilarities, cmp_func); // sap xep list cac similarities lai de tim ra N similarities lon nhat
	double totalSimi = totalSimilarity();	

	// sau khi tim ra N similarity lon nhat thi tim lai index tuong ung cua no, du*.a vao list cac similarities chua sap xep
	double estimatedError = 0;;
	
	// estimate error dua tren 2 giong nhat
	double* element0 = (double *)cvGetSeqElem(sortedSimilarities, 0);
	int indexOfElement0 = 0;
	for (int j = 0; j < similarities->total; j++) { 	double* e = (double *)cvGetSeqElem(similarities, j);	if (*element0 == *e) indexOfElement0 = j;	}

	double* element1 = (double *)cvGetSeqElem(sortedSimilarities, 1);
	int indexOfElement1 = 0;
	for (int j = 0; j < similarities->total; j++) { 	double* e = (double *)cvGetSeqElem(similarities, j);	if (*element1 == *e) indexOfElement1 = j;	}
	
	estimatedError = ((*element0) / totalSimi) * errors[indexOfElement0];
	estimatedError += ((*element1) / totalSimi) * errors[indexOfElement1];

	//
	cvClearSeq(similarities);
	cvClearSeq(sortedSimilarities);
	return estimatedError;
}

double GAclassic::totalSimilarity() {
	double totalSimi = 0;
	for (int i = 0; i < N_RELATED_CHR; i++) {
		double* element = (double *)cvGetSeqElem(sortedSimilarities, i);
		totalSimi += *element;
	}
	return totalSimi;
}

void GAclassic::similarityBtwOneChrWithPopulation(double* chr) { 
	for (int i = 0; i < popSize; i++) {
		double s = similarity(chr, chromosomes[i]);
		cvSeqPush(similarities, &s);
	}
}

double GAclassic::euclideanDistance(double* chr1, double* chr2) {	
	double d0 = chr1[0] - chr2[0];
	double d1 = chr1[1] - chr2[1];
	double d2 = chr1[2] - chr2[2];
	double d3 = chr1[3] - chr2[3];
	return sqrt(d0*d0 + d1*d1 + d2*d2 + d3*d3);
}

double GAclassic::similarity(double* chr1, double* chr2) {
	double distance = euclideanDistance(chr1, chr2);
	if (distance == 0) return 1000000;
	else return 1.0 / distance;
}