#include "ForCompare_MoscheniGA.h"

/*	- estimate 4 thong so cho affine: a1, a4, b1, b2. Su dung truc tiep so thuc
	- fitness function: tinh tren compensation error
	- selection: pair-wise tournament
	- crossover: weighted (using a lambda, where lambda is a random variable uniformly distributed on the unit interval)
	- mutation: mutate with a Gaussian distributed random vector */

ForCompare_MoscheniGA::ForCompare_MoscheniGA(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, 
												double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->popSize = popSize;	this->halfPopSize = popSize/2;
	this->pc = pc;
	this->pm = pm;

	this->aRangeStart = aRangeStart;	this->aRangeEnd = aRangeEnd;		this->aRange = int((aRangeEnd - aRangeStart) * MUL_FACTOR_10K + 1);
	this->bRangeStart = bRangeStart;	this->bRangeEnd = bRangeEnd;		this->bRange = int((bRangeEnd - bRangeStart) * MUL_FACTOR_10K + 1);
}

ForCompare_MoscheniGA::~ForCompare_MoscheniGA(void) {
	for (int i = 0; i < popSize; i++)	delete chromosomes[i];		delete chromosomes;
	delete[] errors;
}

double ForCompare_MoscheniGA::getBestA11()			{	return chromosomes[minErrorIndex][0];	}
double ForCompare_MoscheniGA::getBestA22()			{	return chromosomes[minErrorIndex][1];	}
double ForCompare_MoscheniGA::getBestB1()			{	return chromosomes[minErrorIndex][2];	}
double ForCompare_MoscheniGA::getBestB2()			{	return chromosomes[minErrorIndex][3];	}
double ForCompare_MoscheniGA::getBestError()		{	return errors[minErrorIndex];			}
double** ForCompare_MoscheniGA::getChromosomes()	{	return chromosomes;						}

// --------------------------------------------------------------------------------------

void ForCompare_MoscheniGA::init() {	
	this->chromosomes = new double*[popSize]; 
	for (int i = 0; i < popSize; i++)	this->chromosomes[i] = new double[N_ITEM];
	
	this->errors = new double[popSize]; 

	// tao ra seed dung cho ngau nhien
	__int64 timeForRandSeed;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
	unsigned int t = (unsigned int)(timeForRandSeed % 1000);
	srand(t);

	// for mutation
	double seed;
	seed = rand()*1.0/RAND_MAX;		mutationGaussGenA11.setSeed(seed, 0, 0.001);
	seed = rand()*1.0/RAND_MAX;		mutationGaussGenA22.setSeed(seed, 0, 0.001);
	seed = rand()*1.0/RAND_MAX;		mutationGaussGenB1.setSeed(seed, 0, 0.1);
	seed = rand()*1.0/RAND_MAX;		mutationGaussGenB2.setSeed(seed, 0, 0.1);
}

void ForCompare_MoscheniGA::createPopulation() {
	for (int i = 0; i < popSize; i++) {	
		chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

double ForCompare_MoscheniGA::fitness() { // is the compensation error
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

void ForCompare_MoscheniGA::selection() {
	for (int i = 0; i < popSize; i++) {
		int randN = rand() % popSize;
		if (errors[i] > errors[randN]) {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[i][j] = chromosomes[randN][j];
		} else {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[randN][j] = chromosomes[i][j];
		}
	}
}

void ForCompare_MoscheniGA::crossover() {
	for (int i = 0; i < halfPopSize; i++) {
		for (int j = 0; j < N_ITEM; j++) {
			double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K; // random variable distributed on the unit interval
			chromosomes[i][j] = randN*chromosomes[i][j] + (1-randN)*chromosomes[i + halfPopSize][j];
		}
	}
}

void ForCompare_MoscheniGA::mutation() {
	for (int i = 0; i < popSize; i++) {
		double tmp = chromosomes[i][0] + mutationGaussGenA11.rnd();		if (aRangeStart <= tmp && tmp <= aRangeEnd)	chromosomes[i][0] = tmp;
		tmp = chromosomes[i][1] + mutationGaussGenA22.rnd();			if (aRangeStart <= tmp && tmp <= aRangeEnd)	chromosomes[i][1] = tmp;
		tmp = chromosomes[i][2] + mutationGaussGenB1.rnd();				if (bRangeStart <= tmp && tmp <= bRangeEnd)	chromosomes[i][2] = tmp;
		tmp = chromosomes[i][3] + mutationGaussGenB2.rnd();				if (bRangeStart <= tmp && tmp <= bRangeEnd)	chromosomes[i][3] = tmp;
	}
}

void ForCompare_MoscheniGA::exchangeChromosomes(double** toChromosomes) {
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