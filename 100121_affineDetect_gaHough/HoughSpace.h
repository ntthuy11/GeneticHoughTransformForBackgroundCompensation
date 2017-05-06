#pragma once

#include "cv.h"
#include "highgui.h"
#include "Util.h"

#define DOUBLE_PRECISION (2)

class HoughSpace
{
public:
	HoughSpace(double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes);
	~HoughSpace(void);

	double getBestTheta();
	double getBestRho();
	int getPeakValue();

	void create();
	void accumulate(int x, int y);
	void reset();
	void release();

private:
	int generateSinThetaAndCosThetaList();

	double thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes;
	double thetaRange, rhoRange;
	
	double bestTheta, bestRho;
	int peakValue;

	double *sinThetaList, *cosThetaList;
	int nTheta, nRho, nRhoHalf;

	CvMat* accumulator;
	int *accData;
	int accCols;
};
