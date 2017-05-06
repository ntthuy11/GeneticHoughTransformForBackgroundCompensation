#pragma once

#include "cv.h"
#include "highgui.h"

#include "HoughSpace.h"

#define N_SELECT_TWO_RANDOM_HIST	(10)
#define N_ACCUMULATE				(1000)

class AffineDetect
{
public:
	AffineDetect(IplImage* previousImg, IplImage* currentImg, 
			   int nBinImg, 
			   double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes,
			   int similarityWindow, double similarityThr, int houghSpaceThr);
	~AffineDetect(void);

	double getA11();
	double getA22();
	double getB1();
	double getB2();

	void createVerBinHist();		void accumulateToVerHoughSpace();	void releaseVerBinHist();
	void createHorBinHist();		void accumulateToHorHoughSpace();	void releaseHorBinHist();

private:
	void accumulateTwoSimilarVerHist();
	void accumulateTwoSimilarHorHist();

	IplImage *previousImg, *currentImg;
	int imgHeight, imgWidth, imgStep;
	int nBinImg, range;
	double thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes;
	double similarityThr;
	int similarityWindow, houghSpaceThr;

	int **verBinHist_prevImg, **horBinHist_prevImg;
	int **verBinHist_currImg, **horBinHist_currImg;

	HoughSpace *verHoughSpace, *horHoughSpace;
	int randomVerPosForPrevImg, randomVerPosForCurrImg;
	int randomHorPosForPrevImg, randomHorPosForCurrImg;

	double a11, a22, b1, b2;

	__int64 timeForRandSeed;
};
