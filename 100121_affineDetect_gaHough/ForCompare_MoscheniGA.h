#pragma once

#include "cv.h"
#include "highgui.h"
#include "Util.h"
#include "UniformGen.h"

#define N_ITEM				(4)			// a1, a4, b1, b2
#define MUL_FACTOR_10K		(10000)
#define MUL_FACTOR_10001	(10001)

class ForCompare_MoscheniGA
{
public:
	ForCompare_MoscheniGA(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd);
	~ForCompare_MoscheniGA(void);

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

	CGaussianGen mutationGaussGenA11;
	CGaussianGen mutationGaussGenA22;
	CGaussianGen mutationGaussGenB1;
	CGaussianGen mutationGaussGenB2;
};
