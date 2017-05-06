#pragma once

#include "cv.h"
#include "highgui.h"
#include "Util.h"

#define N_ITEM				(4)			// a1, a4, b1, b2
#define MUL_FACTOR_10K		(10000)
#define MUL_FACTOR_10001	(10001)

// dung cho fitness prediction
#define N_RELATED_CHR		(2)

class GAclassic
{
public:
	GAclassic(IplImage* previousImg, IplImage* currentImg, int popSize, double pc, double pm, double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd);
	~GAclassic(void);

	double getBestA11();
	double getBestA22();
	double getBestB1();
	double getBestB2();
	double getBestError();
	double** getChromosomes();

	void init();
	void insertChromosome(double a11, double a22, double b1, double b2);
	double fitness();
	void selection();
	void crossover();
	void mutation();

	void exchangeChromosomes(double** toChromosomes);

	void interpolateErrors();
	double estimateError(double* chr);
	double totalSimilarity();
	void similarityBtwOneChrWithPopulation(double* chr);
	double euclideanDistance(double* chr1, double* chr2);
	double similarity(double* chr1, double* chr2);

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

	int nExistChromosome;

	// dung cho fitness prediction
	CvMemStorage* similaritiesStorage;		CvSeq* similarities;
	CvMemStorage* sortedSimilaritiesStorage;	CvSeq* sortedSimilarities;

	static int cmp_func(const void* _a, const void* _b, void* userData_notUsed) {
		double* a = (double*)_a;
		double* b = (double*)_b;
		return (*a > *b ? -1 : 1);
	}
};
