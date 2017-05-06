#pragma once

#include "AffineDetect.h"
#include "GAclassic.h"

class AffineDetectMain
{
public:
	AffineDetectMain( /* AffineDetect parameters */ IplImage* previousImg, IplImage* currentImg, 
					int nBinImg, 
					double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes,
					int similarityWindow, double similarityThr, int houghSpaceThr,
					/* GA parameters */ int popSize, double pc, double pm, 
					double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd,
					int gaIteration);
	~AffineDetectMain(void);

	void run(double *result);

private:
	IplImage *previousImg, *currentImg;
	
	int nBinImg;
	double thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes;
	int similarityWindow;
	double similarityThr;
	int houghSpaceThr;

	int popSize;
	double pc, pm;
	double aRangeStart, aRangeEnd, bRangeStart, bRangeEnd;
	int gaIteration;

	unsigned long timeDiff;
	__int64 freq, tStart, tStop;	
};
