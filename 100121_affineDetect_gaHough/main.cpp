#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "ForCompare_Main.h"
#include "AffineDetectMain.h"
#include "Util.h"

#define BATCH_N_IMG				150
#define RESULT_FILENAME			"result.txt"
#define BATCH_IMG_FILENAME		"F:\\_Ntt\\_MyDocuments\\101MSDCF\\100929_MOV06059\\1\\gray\\%04d.bmp"


#define IMG0_NAME		"F:\\_Ntt\\_MyDocuments\\101MSDCF\\100929_MOV06059\\1\\gray\\0000.bmp"
#define IMG1_NAME		"F:\\_Ntt\\_MyDocuments\\101MSDCF\\100929_MOV06059\\1\\gray\\0001.bmp"

#define N_ITERATION			(1)		// so lan cha.y lai (de duoc KQ tot hon)
#define TMP_ERROR			(12)	// dung de loc cac KQ error tot


IplImage *srcImg0, *srcImg1, *grayImg0, *grayImg1;


// =====================================================================


void load2Img() {
	srcImg0 = cvvLoadImage(IMG0_NAME);		int srcImgWidth = srcImg0->width, srcImgHeight = srcImg0->height;
	grayImg0 = cvCreateImage(cvSize(srcImgWidth, srcImgHeight), IPL_DEPTH_8U, 1);		cvCvtColor(srcImg0, grayImg0, CV_RGB2GRAY);
	srcImg1 = cvvLoadImage(IMG1_NAME);	
	grayImg1 = cvCreateImage(cvSize(srcImgWidth, srcImgHeight), IPL_DEPTH_8U, 1);		cvCvtColor(srcImg1, grayImg1, CV_RGB2GRAY);	
}


void runOtherGA() {
	double totalTime = 0;
	double totalError = 0;

	FILE *fp;
	char charFn[256];


	// parameters for GA
	/* UNCHANGED */ int popSize = 50;
	/* UNCHANGED */ double pc = 0.5;
	/* UNCHANGED */ double pm = 0.05;
	double aRangeStart = 0.99;
	double aRangeEnd = 1.01;
	double bRangeStart = -3;
	double bRangeEnd = 3; // 5, 25, 50
	/* UNCHANGED */ int gaIteration = 20;

	
	// main run
	for(int i = 1; i < BATCH_N_IMG; i++) {

		// ---------- print the image number ----------
		fp = fopen(RESULT_FILENAME, "a");		fprintf(fp, "%d   ", i);		fclose(fp);


		// ---------- load two images ----------
		sprintf(charFn, BATCH_IMG_FILENAME, i-1);												srcImg0 = cvvLoadImage(charFn);
		grayImg0 = cvCreateImage(cvSize(srcImg0->width, srcImg0->height), IPL_DEPTH_8U, 1);		cvCvtColor(srcImg0, grayImg0, CV_RGB2GRAY);
		
		sprintf(charFn, BATCH_IMG_FILENAME, i);													srcImg1 = cvvLoadImage(charFn);
		grayImg1 = cvCreateImage(cvSize(srcImg0->width, srcImg0->height), IPL_DEPTH_8U, 1);		cvCvtColor(srcImg1, grayImg1, CV_RGB2GRAY);	


		// ---------- RUN ----------
		ForCompare_Main forCompareMain(grayImg0, grayImg1, popSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd, gaIteration);

		double* result = new double[6];
		forCompareMain.runClassicalGA(result);
		//forCompareMain.runMoscheniGA(result);
		//forCompareMain.runMutohGA(result);

		double tmpA11 = result[0];
		double tmpA22 = result[1];
		double tmpB1 = result[2];
		double tmpB2 = result[3];
		double tmpError = result[4];		totalError += tmpError;
		double tmpTime = result[5];			totalTime += tmpTime;

		fp = fopen(RESULT_FILENAME, "a");		fprintf(fp, "a11=%1.3f, a22=%1.3f, b1=%1.3f, b2=%1.3f, e=%2.3f, t=%d \n", tmpA11, tmpA22, tmpB1, tmpB2, tmpError, int(tmpTime));		fclose(fp);

		delete[] result;
	}

	// write total/average information	
	fp = fopen(RESULT_FILENAME, "a");		fprintf(fp, "avgError=%2.3f, avgTime = %2.3f\n", totalError/(BATCH_N_IMG-1), totalTime/(BATCH_N_IMG-1));		fclose(fp);
}


void runGHT() {
	load2Img();

	// parameters for GA
	/* UNCHANGED */ int popSize = 50;
	/* UNCHANGED */ double pc = 0.5;
	/* UNCHANGED */ double pm = 0.05;
	double aRangeStart = 0.97; // 0.97
	double aRangeEnd = 1.03; // 1.03
	double bRangeStart = -6;
	double bRangeEnd = -bRangeStart; // 5, 10, 25, 50
	int gaIteration = 10; // 10 or 5

	// parameters for AffineDetect
	/* UNCHANGED */ int nBinImg = 32;
	/* UNCHANGED */ double thetaRangeStart = atan(-1.0/aRangeStart);	// a_min = 0.9 => thetaRangeStart = -0.838
	/* UNCHANGED */ double thetaRangeEnd = atan(-1.0/aRangeEnd);		// a_max = 1.1 => thetaRangeEnd = -0.738
	double thetaRes = 0.0003;
	/* UNCHANGED */ double rhoRangeStart = bRangeEnd * Util::roundDouble(sin(thetaRangeStart), 2);	// b_min = -5 => rhoRangeStart = -3.715
	/* UNCHANGED */ double rhoRangeEnd = bRangeStart * Util::roundDouble(sin(thetaRangeStart), 2);	// b_max = 5 => rhoRangeEnd = 3.715
	double rhoRes = 0.0003;
	/* UNCHANGED */ int similarityWindow = int(fabs(bRangeStart) + fabs(bRangeEnd));
	/* UNCHANGED */ double similarityThr = 0.1;
	int houghSpaceThr = 10;

	//
	AffineDetectMain affDetectMain(grayImg0, grayImg1, 
		nBinImg, thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes, similarityWindow, similarityThr, houghSpaceThr,
		popSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd, gaIteration);

	// write result to file
	CStdioFile f;	f.Open(RESULT_FILENAME, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate);		f.SeekToEnd();
	CString text;

	for(int i = 0; i < N_ITERATION; i++) {
		double* result = new double[6];
		affDetectMain.run(result);

		double tmpA11 = result[0];
		double tmpA22 = result[1];
		double tmpB1 = result[2];
		double tmpB2 = result[3];
		double tmpError = result[4];
		double tmpTime = result[5];

		//if(tmpError < TMP_ERROR) {
			text.Format("a11=%1.3f, a22=%1.3f, b1=%1.3f, b2=%1.3f, e=%2.3f, t=%4d \n", tmpA11, tmpA22, tmpB1, tmpB2, tmpError, int(tmpTime));
			f.WriteString(text);
		//}

		delete[] result;
	}

	f.Close();
}


void release() {
	cvReleaseImage(&srcImg0);	cvReleaseImage(&grayImg0);
	cvReleaseImage(&srcImg1);	cvReleaseImage(&grayImg1);	
}


void main() {
	runOtherGA();
	//runGHT();
	release();
}