#pragma once

#include "cv.h"
#include "highgui.h"
#include "Util.h"

#define N_ITEM				(4)			// a1, a4, b1, b2
#define MUL_FACTOR_10K		(10000)
#define MUL_FACTOR_10001	(10001)

class ForCompare_MutohGA
{
public:
	ForCompare_MutohGA(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd);
	~ForCompare_MutohGA(void);

	double getBestA11();
	double getBestA22();
	double getBestB1();
	double getBestB2();
	double getBestError();
	double** getChromosomes();

	void init();
	void createPopulation();
	double fitness();
	void selection();
	void crossover();
	void mutation();

	void exchangeChromosomes(double** toChromosomes);

private:
	double** chromosomes;
	int popSize, halfPopSize;
	double pc, pm;
	
	double aRangeStart, aRangeEnd, bRangeStart, bRangeEnd;
	int aRange, bRange;

	double* errors;
	int minErrorIndex;

	IplImage* previousImg;
	IplImage* currentImg;

	// for MCPCP
	void findTwoBestChromosomes(double* srcChr1, double* srcChr2, double* bestChr1, double* bestChr2);
	double estimateError(double* chr);
	double weight(double* chr1, double* chr2, double totalSimi);
	double totalSimilarity(double* chr);
	double similarity(double* chr1, double* chr2);
	double euclideanDistance(double* chr1, double* chr2);

	static int cmp_func(const void* _a, const void* _b, void* userData_notUsed) {
		double* a = (double*)_a;
		double* b = (double*)_b;
		return (*a > *b ? -1 : 1);
	}
};
