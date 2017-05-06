#include "stdafx.h"
#include "afxmt.h"
#include "AffineDetectMain.h"

// dung cho FP
bool isTheTimeToRunFitnessPredictor = false;


AffineDetectMain::AffineDetectMain( /* AffineDetect parameters */ IplImage* previousImg, IplImage* currentImg, 
						   int nBinImg, 
						   double thetaRangeStart, double thetaRangeEnd, double thetaRes, double rhoRangeStart, double rhoRangeEnd, double rhoRes,	// co the dung thetaRangeStart ~ aRangeStart, thetaRangeEnd ~ aRangeEnd
						   int similarityWindow, double similarityThr, int houghSpaceThr,									// nhung su dung ca 2 dde^? de^~ quan sat
						   /* GA parameters */ int popSize, double pc, double pm, 
						   double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd,
						   int gaIteration) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->nBinImg = nBinImg;
	this->thetaRangeStart = thetaRangeStart;
	this->thetaRangeEnd = thetaRangeEnd;
	this->thetaRes = thetaRes;
	this->rhoRangeStart = rhoRangeStart;
	this->rhoRangeEnd = rhoRangeEnd;
	this->rhoRes = rhoRes;
	this->similarityWindow = similarityWindow;
	this->similarityThr = similarityThr;
	this->houghSpaceThr = houghSpaceThr;

	this->popSize = popSize;
	this->pc = pc;
	this->pm = pm;
	this->aRangeStart = aRangeStart;
	this->aRangeEnd = aRangeEnd;
	this->bRangeStart = bRangeStart;
	this->bRangeEnd = bRangeEnd;
	this->gaIteration = gaIteration;
}

AffineDetectMain::~AffineDetectMain(void) { }


// ========================================================================

// ---------- threads for AffineDetect ----------

double _a11, _a22, _b1, _b2;
CSemaphore semaAffVer, semaAffHor; 

UINT initAffVerThread(LPVOID pParam) { // ------- threads for init -------
	CSingleLock semlock(&semaAffVer);
	semlock.Lock();
	AffineDetect* affDetect = (AffineDetect *)pParam;		affDetect->createVerBinHist();
	semlock.Unlock();
	return 0;
}

UINT initAffHorThread(LPVOID pParam) { 
	CSingleLock semlock(&semaAffHor);
	semlock.Lock();
	AffineDetect* affDetect = (AffineDetect *)pParam;		affDetect->createHorBinHist();
	semlock.Unlock();
	return 0;
}

UINT affVerThread(LPVOID pParam) { // ------- threads for running -------
	CSingleLock semlock(&semaAffVer);
	semlock.Lock();

	AffineDetect* affDetect = (AffineDetect *)pParam;
	affDetect->accumulateToVerHoughSpace();
	_a11 = affDetect->getA11();
	_b1 = affDetect->getB1();

	semlock.Unlock();
	return 0;
}

UINT affHorThread(LPVOID pParam) { 
	CSingleLock semlock(&semaAffHor);
	semlock.Lock();

	AffineDetect* affDetect = (AffineDetect *)pParam;
	affDetect->accumulateToHorHoughSpace();
	_a22 = affDetect->getA22();
	_b2 = affDetect->getB2();

	semlock.Unlock();
	return 0;
}

UINT releaseAffVerThread(LPVOID pParam) { // ------- threads for releasing -------
	CSingleLock semlock(&semaAffVer);
	semlock.Lock();
	AffineDetect* affDetect = (AffineDetect *)pParam;		affDetect->releaseVerBinHist();
	semlock.Unlock();
	return 0;
}

UINT releaseAffHorThread(LPVOID pParam) {
	CSingleLock semlock(&semaAffHor);
	semlock.Lock();
	AffineDetect* affDetect = (AffineDetect *)pParam;		affDetect->releaseHorBinHist();
	semlock.Unlock();
	return 0;
}


// ---------- threads for GA ----------

CSemaphore semaGA1, semaGA2; 

UINT ga1Thread(LPVOID pParam) {
	CSingleLock semlock(&semaGA1);
	semlock.Lock();

	GAclassic* ga = (GAclassic *)pParam;
	
	if(isTheTimeToRunFitnessPredictor)
		ga->interpolateErrors();
	else
		ga->fitness();

	ga->selection();
	ga->crossover();
	ga->mutation();

	semlock.Unlock();
	return 0;
}

UINT ga2Thread(LPVOID pParam) {
	CSingleLock semlock(&semaGA2);
	semlock.Lock();
	
	GAclassic* ga = (GAclassic *)pParam;
	
	if(isTheTimeToRunFitnessPredictor)
		ga->interpolateErrors();
	else
		ga->fitness();

	ga->selection();
	ga->crossover();
	ga->mutation();

	semlock.Unlock();
	return 0;
}

UINT ga1Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaGA1);
	semlock.Lock();
	GAclassic* ga = (GAclassic *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}

UINT ga2Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaGA2);
	semlock.Lock();
	GAclassic* ga = (GAclassic *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}

// =====================================================================================================

void AffineDetectMain::run(double *result) { // result = [a11, a22, b1, b2, error, time]
	AffineDetect affDetect(previousImg, currentImg, nBinImg, thetaRangeStart, thetaRangeEnd, thetaRes, rhoRangeStart, rhoRangeEnd, rhoRes, similarityWindow, similarityThr, houghSpaceThr);

	int halfPopSize = popSize/2;
	GAclassic ga1(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga1.init(); // dung thread cho init nay cung duoc
	GAclassic ga2(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga2.init();


	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// ---- threads for init AffineDetect ----
	CWinThread *pAffVerThread = AfxBeginThread(initAffVerThread, &affDetect);//		pAffVerThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffVerThread->ResumeThread();
	CWinThread *pAffHorThread = AfxBeginThread(initAffHorThread, &affDetect);//		pAffHorThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffHorThread->ResumeThread();
	CSingleLock lockAffVer0(&semaAffVer);		lockAffVer0.Lock();
	CSingleLock lockAffHor0(&semaAffHor);		lockAffHor0.Lock();
	// do nothing here
	lockAffVer0.Unlock();
	lockAffHor0.Unlock();

	// ---- threads for running AffineDetect ----
	for(int i = 0; i < halfPopSize; i++) {
		pAffVerThread = AfxBeginThread(affVerThread, &affDetect);//		pAffVerThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffVerThread->ResumeThread();
		pAffHorThread = AfxBeginThread(affHorThread, &affDetect);//		pAffHorThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffHorThread->ResumeThread();
		CSingleLock lockAffVer(&semaAffVer);		lockAffVer.Lock();
		CSingleLock lockAffHor(&semaAffHor);		lockAffHor.Lock();	
		ga1.insertChromosome(_a11, _a22, _b1, _b2); // <== insert gia tri moi tim duoc vao GA populuation coi nhu la 1 chromosome
		lockAffVer.Unlock();
		lockAffHor.Unlock();
	}
	for(int i = halfPopSize; i < popSize; i++) {
		pAffVerThread = AfxBeginThread(affVerThread, &affDetect);//		pAffVerThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffVerThread->ResumeThread();		
		pAffHorThread = AfxBeginThread(affHorThread, &affDetect);//		pAffHorThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffHorThread->ResumeThread();			
		CSingleLock lockAffVer(&semaAffVer);		lockAffVer.Lock();
		CSingleLock lockAffHor(&semaAffHor);		lockAffHor.Lock();	
		ga2.insertChromosome(_a11, _a22, _b1, _b2); // <== insert gia tri moi tim duoc vao GA populuation coi nhu la 1 chromosome
		lockAffVer.Unlock();
		lockAffHor.Unlock();
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	unsigned long timeDiff1 = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP


	// ---- threads for releasing AffineDetect ----
	pAffVerThread = AfxBeginThread(releaseAffVerThread, &affDetect);//		pAffVerThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffVerThread->ResumeThread();
	pAffHorThread = AfxBeginThread(releaseAffHorThread, &affDetect);//		pAffHorThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//	pAffHorThread->ResumeThread();			
	CSingleLock lockAffVer(&semaAffVer);		lockAffVer.Lock();
	CSingleLock lockAffHor(&semaAffHor);		lockAffHor.Lock();
	// do nothing here
	lockAffVer.Unlock();
	lockAffHor.Unlock();


	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// ---- threads for running GA ----
	for(int i = 0; i < gaIteration; i++) {
		// check dieu kien de chay fitness predictor
		//if (i == 3 || i == 6 || i == 9)	isTheTimeToRunFitnessPredictor = true; // coi nhu la so generation = 10
		//else isTheTimeToRunFitnessPredictor = false;
		//isTheTimeToRunFitnessPredictor = (i % 3 == 0); // 3 la vi chay 2 FE va 1 FP

		CWinThread * pGA1Thread = AfxBeginThread(ga1Thread, &ga1);//		pGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pGA1Thread->ResumeThread();
		CWinThread * pGA2Thread = AfxBeginThread(ga2Thread, &ga2);//		pGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pGA2Thread->ResumeThread();
		CSingleLock ga1Lock(&semaGA1);		ga1Lock.Lock();
		CSingleLock ga2Lock(&semaGA2);		ga2Lock.Lock();
		ga1.exchangeChromosomes(ga2.getChromosomes()); // <==
		ga1Lock.Unlock();
		ga2Lock.Unlock();
	}

	// fitness cuoi cung cua vong lap
	CWinThread * pGA1Thread = AfxBeginThread(ga1Thread_end, &ga1);//		pGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pGA1Thread->ResumeThread();
	CWinThread * pGA2Thread = AfxBeginThread(ga2Thread_end, &ga2);//		pGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pGA2Thread->ResumeThread();
	CSingleLock ga1Lock(&semaGA1);		ga1Lock.Lock();
	CSingleLock ga2Lock(&semaGA2);		ga2Lock.Lock();
	// do nothing here
	ga1Lock.Unlock();
	ga2Lock.Unlock();

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	unsigned long timeDiff2 = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP


	// ---- return ----
	result[0] =  ga1.getBestA11();
	result[1] =  ga1.getBestA22();
	result[2] =  ga1.getBestB1();
	result[3] =  ga1.getBestB2();
	result[4] =  ga1.getBestError();
	result[5] =  double(timeDiff1 + timeDiff2);
}
