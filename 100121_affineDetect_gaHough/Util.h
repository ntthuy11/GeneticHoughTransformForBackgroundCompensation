#pragma once

class Util
{
public:
	Util(void);
public:
	~Util(void);
	

	// http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=14154
	static double roundDouble(double doValue, int nPrecision) {
		static const double doBase = 10.0;
		double doComplete5, doComplete5i;
	    
		doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));
	    
		if(doValue < 0.0) doComplete5 -= 5.0;
		else doComplete5 += 5.0;
	    
		doComplete5 /= doBase;
		modf(doComplete5, &doComplete5i);
	    
		return doComplete5i / pow(doBase, (double) nPrecision);
	}


	static double calculateAvgIntensityWrtAffine(IplImage* src1, IplImage* src2, double a1, double a4, double b1, double b2) {

		// truoc tien phai tinh nghich dao cho ma tran A (a1, a2 = a3 = 0, a4) va B (b1, b2)
		double a1_ = 1/a1;		double a4_ = 1/a4;
		double b1_ = a1_*b1;	double b2_ = a4_*b2;

		// main run
		int step = src1->widthStep, width = src1->width, height = src1->height;
		const uchar* src1Data = (uchar *)src1->imageData;
		const uchar* src2Data = (uchar *)src2->imageData;

		int hDiv2 = height/2;
		int wDiv2 = width/2;

		int count = 0;
		int totalError = 0;

		for (int i = 0; i < height; i++) {
			int newY = i - hDiv2;

			for (int j = 0; j < width; j++) {
				int newX = j - wDiv2;				
				int pos = i*step + j;
				double x = a1_*newX - b1_;		x += wDiv2;
				double y = a4_*newY - b2_;		y += hDiv2;	

				if (0 <= x && x < width - 1 && 0 <= y && y < height - 1) { // minimum neighborhood
					int i_ = int(y);
					int j_ = int(x);
					int des1 = src1Data[i_*step + j_]		- src2Data[pos];	if (des1 < 0) des1 = -des1;
					int des2 = src1Data[i_*step + (j_+1)]	- src2Data[pos];	if (des2 < 0) des2 = -des2;		if (des1 > des2) des1 = des2;
					des2 = src1Data[(i_+1)*step + j_]		- src2Data[pos];	if (des2 < 0) des2 = -des2;		if (des1 > des2) des1 = des2;
					des2 = src1Data[(i_+1)*step + (j_+1)]	- src2Data[pos];	if (des2 < 0) des2 = -des2;		if (des1 > des2) des1 = des2;

					count++;
					totalError += des1;
				}
			}
		}

		return totalError * 1.0 / count;
	}
};