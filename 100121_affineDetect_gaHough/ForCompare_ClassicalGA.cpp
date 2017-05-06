#include "ForCompare_ClassicalGA.h"

ForCompare_ClassicalGA::ForCompare_ClassicalGA(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, 
												double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->popSize = popSize;	this->halfPopSize = popSize/2;
	this->pc = pc;
	this->pm = pm;

	this->aRangeStart = aRangeStart;	this->aRangeEnd = aRangeEnd;		this->aRange = int((aRangeEnd - aRangeStart) * MUL_FACTOR_10K + 1);
	this->bRangeStart = bRangeStart;	this->bRangeEnd = bRangeEnd;		this->bRange = int((bRangeEnd - bRangeStart) * MUL_FACTOR_10K + 1);
}

ForCompare_ClassicalGA::~ForCompare_ClassicalGA(void) {
	for (int i = 0; i < popSize; i++)	delete chromosomes[i];		delete chromosomes;
	delete[] errors;
}

double ForCompare_ClassicalGA::getBestA11()			{	return chromosomes[minErrorIndex][0];	}
double ForCompare_ClassicalGA::getBestA22()			{	return chromosomes[minErrorIndex][1];	}
double ForCompare_ClassicalGA::getBestB1()			{	return chromosomes[minErrorIndex][2];	}
double ForCompare_ClassicalGA::getBestB2()			{	return chromosomes[minErrorIndex][3];	}
double ForCompare_ClassicalGA::getBestError()		{	return errors[minErrorIndex];			}
double** ForCompare_ClassicalGA::getChromosomes()	{	return chromosomes;						}

// --------------------------------------------------------------------------------------

void ForCompare_ClassicalGA::init() {	
	this->chromosomes = new double*[popSize]; 
	for (int i = 0; i < popSize; i++)	this->chromosomes[i] = new double[N_ITEM];
	
	this->errors = new double[popSize]; 

	// tao ra seed dung cho ngau nhien
	__int64 timeForRandSeed;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
	unsigned int t = (unsigned int)(timeForRandSeed % 1000);
	srand(t);
}

void ForCompare_ClassicalGA::createPopulation() {
	for (int i = 0; i < popSize; i++) {	
		chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

double ForCompare_ClassicalGA::fitness() { // is the compensation error
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

void ForCompare_ClassicalGA::selection() {
	for (int i = 0; i < popSize; i++) {
		int randN = rand() % popSize;
		if (errors[i] > errors[randN]) {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[i][j] = chromosomes[randN][j];
		} else {
			for (int j = 0; j < N_ITEM; j++) 	chromosomes[randN][j] = chromosomes[i][j];
		}
	}
}

void ForCompare_ClassicalGA::crossover() {
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

void ForCompare_ClassicalGA::mutation() {
	for (int i = 0; i < popSize; i++) {
		double randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;		if (randN < this->pm) 	chromosomes[i][0] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][1] = double(rand() % aRange)/MUL_FACTOR_10K + aRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][2] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
		randN = double(rand() % MUL_FACTOR_10001) / MUL_FACTOR_10K;				if (randN < this->pm) 	chromosomes[i][3] = double(rand() % bRange)/MUL_FACTOR_10K + bRangeStart;
	}
}

void ForCompare_ClassicalGA::exchangeChromosomes(double** toChromosomes) {
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