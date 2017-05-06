#include "HoughSpace.h"

HoughSpace::HoughSpace(double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes) {
	this->thetaRangeStart = thetaRangeStart;		this->thetaRange = fabs(thetaRangeEnd - thetaRangeStart);
	this->thetaRangeEnd = thetaRangeEnd;
	this->thetaRes = thetaRes;

	this->rhoRangeStart = rhoRangeStart;			this->rhoRange = fabs(rhoRangeEnd - rhoRangeStart);
	this->rhoRangeEnd = rhoRangeEnd;
	this->rhoRes = rhoRes;	

	this->peakValue = 0;
}

HoughSpace::~HoughSpace(void) { }

double HoughSpace::getBestTheta()	{	return bestTheta;	}
double HoughSpace::getBestRho()		{	return bestRho;		}
int HoughSpace::getPeakValue()		{	return peakValue;	}

// ------------------------------------------------------------------

void HoughSpace::create() {
	// init theta & rho
	nTheta = generateSinThetaAndCosThetaList();
	nRho = cvRound(rhoRange/rhoRes);
	nRhoHalf = nRho / 2;

	// init accumulator
	accumulator = cvCreateMat(nTheta, nRho, CV_32SC1);
	cvZero(accumulator);
	accCols = accumulator->cols;
	accData = accumulator->data.i;
}

void HoughSpace::accumulate(int x /* j */, int y /* j' */) {

	for(int i = 0; i < nTheta; i++) {
		//double rho = x*cosThetaList[i] + y*sinThetaList[i];
		double rho = y*cosThetaList[i] + x*sinThetaList[i];

		if(rhoRangeStart <= rho && rho <= rhoRangeEnd) {
			int indexOfRho = cvRound(rho/rhoRes) + nRhoHalf;

			int accPos = i*accCols + indexOfRho;
			accData[accPos]++;

			if(accData[accPos] > peakValue) {
				peakValue = accData[accPos];
				bestTheta = i*thetaRes + thetaRangeStart;
				bestRho = rho;
			}
		}
	}
}

void HoughSpace::reset() {
	cvZero(accumulator);
	peakValue = 0;
	bestTheta = -1;
	bestRho = -1;
}

void HoughSpace::release() {
	delete[] sinThetaList;
	delete[] cosThetaList;
	cvReleaseMat(&accumulator);
}

// ===================================== PRIVATE =====================================

int HoughSpace::generateSinThetaAndCosThetaList() { 
	int nTheta = cvRound(thetaRange / thetaRes);
	sinThetaList = new double[nTheta];
	cosThetaList = new double[nTheta];
	for(int i = 0; i < nTheta; i++) {
		double theta = i*thetaRes + thetaRangeStart;
		//sinThetaList[i] = sin(theta);
		//cosThetaList[i] = cos(theta);
		sinThetaList[i] = Util::roundDouble(sin(theta), 3);
		cosThetaList[i] = Util::roundDouble(cos(theta), 3);
	}
	return nTheta;
}