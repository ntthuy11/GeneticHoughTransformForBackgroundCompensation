#pragma once

#include "ForCompare_ClassicalGA.h"
#include "ForCompare_MoscheniGA.h"
#include "ForCompare_MutohGA.h"

class ForCompare_Main
{
public:
	ForCompare_Main(IplImage* previousImg, IplImage* currentImg,
					/* GA parameters */ int popSize, double pc, double pm, 
					double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd,
					int gaIteration);
	~ForCompare_Main(void);

	void runClassicalGA(double *result);
	void runMoscheniGA(double *result);
	void runMutohGA(double *result);

private:
	IplImage *previousImg, *currentImg;

	int popSize;
	double pc, pm;
	double aRangeStart, aRangeEnd, bRangeStart, bRangeEnd;
	int gaIteration;

	unsigned long timeDiff;
	__int64 freq, tStart, tStop;
};
