#include "AffineDetect.h"

AffineDetect::AffineDetect(IplImage* previousImg, IplImage* currentImg, 
						   int nBinImg, 
						   double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes,
						   int similarityWindow, double similarityThr, int houghSpaceThr) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;
	this->nBinImg = nBinImg;			this->range = 256/nBinImg;

	this->imgHeight = previousImg->height;
	this->imgWidth = previousImg->width;
	this->imgStep = previousImg->widthStep;

	this->thetaRangeStart = thetaRangeStart;
	this->thetaRangeEnd = thetaRangeEnd;
	this->thetaRes = thetaRes;
	this->rhoRangeStart = rhoRangeStart;
	this->rhoRangeEnd = rhoRangeEnd;
	this->rhoRes = rhoRes;	

	this->similarityWindow = similarityWindow;
	this->similarityThr = similarityThr;
	this->houghSpaceThr = houghSpaceThr;
}

AffineDetect::~AffineDetect(void) { }

double AffineDetect::getA11() { return a11; }
double AffineDetect::getA22() { return a22; }
double AffineDetect::getB1() { return b1; }
double AffineDetect::getB2() { return b2; }


// ---------------------------------------- vertical histograms ----------------------------------------------------

void AffineDetect::createVerBinHist() {
	
	// init vertical histograms cho tung binary images. Binary images nay duoc tinh tu gray image (gray value duoc chia nho ra lam nhieu phan)
	verBinHist_prevImg = new int*[nBinImg];
	verBinHist_currImg = new int*[nBinImg];
	for(int i = 0; i < nBinImg; i++) {		
		verBinHist_prevImg[i] = new int[imgWidth];
		verBinHist_currImg[i] = new int[imgWidth];
		for(int j = 0; j < imgWidth; j++) 
			verBinHist_prevImg[i][j] = verBinHist_currImg[i][j] = 0;
	}

	// lay tung gray value trong previous & current image de "tinh luy" (tao histogram) vao cac vertical binary histograms
	const uchar* prevImgData = (uchar *)previousImg->imageData;
	const uchar* currImgData = (uchar *)currentImg->imageData;
	int valueAtPos_prevImg, valueAtPos_currImg, pos;
	for(int i = 0; i < imgHeight; i++) {		
		for (int j = 0; j < imgWidth; j++) {
			pos = i*imgStep + j;
			valueAtPos_prevImg = prevImgData[pos] / range;		verBinHist_prevImg[valueAtPos_prevImg][j]++;
			valueAtPos_currImg = currImgData[pos] / range;		verBinHist_currImg[valueAtPos_currImg][j]++;
		}
	}

	// init vertical Hough space
	verHoughSpace = new HoughSpace(thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes);
	verHoughSpace->create();	
}


void AffineDetect::accumulateToVerHoughSpace() {

	for(int i = 0; i < N_ACCUMULATE; i++) { // i = N_ACCUMULATE is the threshold to stop the accumulation iteration
		accumulateTwoSimilarVerHist();
		if(verHoughSpace->getPeakValue() > houghSpaceThr)
			break; // da chon duoc cell co value tot
	}
	
	double bestTheta = verHoughSpace->getBestTheta();
	double bestRho = verHoughSpace->getBestRho();
	a11 = - cos(bestTheta) / sin(bestTheta);
	b1 = - bestRho / sin(bestTheta); // b1 = bestRho / sin(bestTheta);

	// reset ver. Hough space (all cells to zero)
	verHoughSpace->reset();
}


void AffineDetect::releaseVerBinHist() {
	for(int i = 0; i < nBinImg; i++) {		
		delete[] verBinHist_prevImg[i];	
		delete[] verBinHist_currImg[i];
	}
	delete[] verBinHist_prevImg;
	delete[] verBinHist_currImg;

	verHoughSpace->release();
}


// ---------------------------------------- horizontal histograms ----------------------------------------------------


void AffineDetect::createHorBinHist() {
	
	// init hor. histograms cho tung binary images. Binary images nay duoc tinh tu gray image (gray value duoc chia nho ra lam nhieu phan)
	horBinHist_prevImg = new int*[nBinImg];
	horBinHist_currImg = new int*[nBinImg];
	for(int i = 0; i < nBinImg; i++) {		
		horBinHist_prevImg[i] = new int[imgHeight];
		horBinHist_currImg[i] = new int[imgHeight];
		for(int j = 0; j < imgHeight; j++) 
			horBinHist_prevImg[i][j] = horBinHist_currImg[i][j] = 0;
	}

	// lay tung gray value trong previous & current image de "tinh luy" (tao histogram) vao cac hor. binary histograms
	const uchar* prevImgData = (uchar *)previousImg->imageData;
	const uchar* currImgData = (uchar *)currentImg->imageData;
	int valueAtPos_prevImg, valueAtPos_currImg, pos;
	for(int i = 0; i < imgHeight; i++) {		
		for (int j = 0; j < imgWidth; j++) {
			pos = i*imgStep + j;
			valueAtPos_prevImg = prevImgData[pos] / range;		horBinHist_prevImg[valueAtPos_prevImg][i]++;
			valueAtPos_currImg = currImgData[pos] / range;		horBinHist_currImg[valueAtPos_currImg][i]++;
		}
	}

	// init vertical Hough space
	horHoughSpace = new HoughSpace(thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes);
	horHoughSpace->create();	
}


void AffineDetect::accumulateToHorHoughSpace() {

	for(int i = 0; i < N_ACCUMULATE; i++) { // i = N_ACCUMULATE is the threshold to stop the accumulation iteration
		accumulateTwoSimilarHorHist();
		if(horHoughSpace->getPeakValue() > houghSpaceThr)
			break; // da chon duoc cell co value tot
	}
	
	double bestTheta = horHoughSpace->getBestTheta();
	double bestRho = horHoughSpace->getBestRho();
	a22 = - cos(bestTheta) / sin(bestTheta);
	//double b2 = bestRho / sin(bestTheta);
	b2 = - bestRho / sin(bestTheta);

	// reset hor. Hough space (all cells to zero)
	horHoughSpace->reset();
}


void AffineDetect::releaseHorBinHist() {
	for(int i = 0; i < nBinImg; i++) {		
		delete[] horBinHist_prevImg[i];	
		delete[] horBinHist_currImg[i];
	}
	delete[] horBinHist_prevImg;
	delete[] horBinHist_currImg;

	horHoughSpace->release();
}


// ================================ PRIVATE ================================


void AffineDetect::accumulateTwoSimilarVerHist() {	

	// chon ra 2 verHist giong nhau (thoa nguong)
	for(int i = 0; i < N_SELECT_TWO_RANDOM_HIST; i++) { // i = N_SELECT_TWO_RANDOM_HIST is the threshold to stop the random-iteration
		QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
		unsigned int t = (unsigned int)(timeForRandSeed % 1000);
		srand(t);

		int randomBinImg = rand() % nBinImg;
		randomVerPosForPrevImg = rand() % imgWidth;

		if(verBinHist_prevImg[randomBinImg][randomVerPosForPrevImg] > 0) {
			randomVerPosForCurrImg = randomVerPosForPrevImg + (rand()%(similarityWindow*2) - similarityWindow);
			if(randomVerPosForCurrImg < 0)	
				randomVerPosForCurrImg = 0;
			else if(randomVerPosForCurrImg >= imgWidth)
				randomVerPosForCurrImg = imgWidth - 1;

			int nominator = abs(verBinHist_prevImg[randomBinImg][randomVerPosForPrevImg] - verBinHist_currImg[randomBinImg][randomVerPosForCurrImg]);
			int denominator = verBinHist_prevImg[randomBinImg][randomVerPosForPrevImg];
			if( (nominator*1.0 / denominator) < similarityThr ) { // da chon duoc 2 verHist giong nhau
				verHoughSpace->accumulate(randomVerPosForPrevImg, randomVerPosForCurrImg);
				break;
			}
		}
	}
}


void AffineDetect::accumulateTwoSimilarHorHist() {

	// chon ra 2 verHist giong nhau (thoa nguong)
	for(int i = 0; i < N_SELECT_TWO_RANDOM_HIST; i++) { // i = N_SELECT_TWO_RANDOM_HIST is the threshold to stop the random-iteration
		QueryPerformanceCounter((LARGE_INTEGER*)&timeForRandSeed);
		unsigned int t = (unsigned int)(timeForRandSeed % 1000);
		srand(t);

		int randomBinImg = rand() % nBinImg;
		randomHorPosForPrevImg = rand() % imgHeight;

		if(horBinHist_prevImg[randomBinImg][randomHorPosForPrevImg] > 0) {
			randomHorPosForCurrImg = randomHorPosForPrevImg + (rand()%(similarityWindow*2) - similarityWindow);
			if(randomHorPosForCurrImg < 0)	
				randomHorPosForCurrImg = 0;
			else if(randomHorPosForCurrImg >= imgHeight)
				randomHorPosForCurrImg = imgHeight - 1;

			int nominator = abs(horBinHist_prevImg[randomBinImg][randomHorPosForPrevImg] - horBinHist_currImg[randomBinImg][randomHorPosForCurrImg]);
			int denominator = horBinHist_prevImg[randomBinImg][randomHorPosForPrevImg];
			if( (nominator*1.0 / denominator)  < similarityThr ) {  // da chon duoc 2 horHist giong nhau
				horHoughSpace->accumulate(randomHorPosForPrevImg, randomHorPosForCurrImg);
				break;
			}
		}
	}
}