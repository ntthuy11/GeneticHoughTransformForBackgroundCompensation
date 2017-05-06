#include "stdafx.h"
#include "afxmt.h"
#include "ForCompare_Main.h"

ForCompare_Main::ForCompare_Main(IplImage* previousImg, IplImage* currentImg,
								 /* GA parameters */ int popSize, double pc, double pm, 
								 double aRangeStart, double aRangeEnd, double bRangeStart, double bRangeEnd,
								 int gaIteration) {
	this->previousImg = previousImg;
	this->currentImg = currentImg;

	this->popSize = popSize;
	this->pc = pc;
	this->pm = pm;
	this->aRangeStart = aRangeStart;
	this->aRangeEnd = aRangeEnd;
	this->bRangeStart = bRangeStart;
	this->bRangeEnd = bRangeEnd;
	this->gaIteration = gaIteration;
}

ForCompare_Main::~ForCompare_Main(void) { }

// =====================================================================================================

// ---------- threads for classical GA ----------

CSemaphore semaClassicalGA1, semaClassicalGA2; 

UINT classicalGA1Thread(LPVOID pParam) {
	CSingleLock semlock(&semaClassicalGA1);
	semlock.Lock();
	ForCompare_ClassicalGA* ga = (ForCompare_ClassicalGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}
UINT classicalGA2Thread(LPVOID pParam) {
	CSingleLock semlock(&semaClassicalGA2);
	semlock.Lock();
	ForCompare_ClassicalGA* ga = (ForCompare_ClassicalGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}

UINT classicalGA1Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaClassicalGA1);
	semlock.Lock();
	ForCompare_ClassicalGA* ga = (ForCompare_ClassicalGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}
UINT classicalGA2Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaClassicalGA2);
	semlock.Lock();
	ForCompare_ClassicalGA* ga = (ForCompare_ClassicalGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}

// ---------- threads for Moscheni GA ----------

CSemaphore semaMoscheniGA1, semaMoscheniGA2; 

UINT moscheniGA1Thread(LPVOID pParam) {
	CSingleLock semlock(&semaMoscheniGA1);
	semlock.Lock();
	ForCompare_MoscheniGA* ga = (ForCompare_MoscheniGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}
UINT moscheniGA2Thread(LPVOID pParam) {
	CSingleLock semlock(&semaMoscheniGA2);
	semlock.Lock();
	ForCompare_MoscheniGA* ga = (ForCompare_MoscheniGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}

UINT moscheniGA1Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaMoscheniGA1);
	semlock.Lock();
	ForCompare_MoscheniGA* ga = (ForCompare_MoscheniGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}
UINT moscheniGA2Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaMoscheniGA2);
	semlock.Lock();
	ForCompare_MoscheniGA* ga = (ForCompare_MoscheniGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}

// ---------- threads for Mutoh GA ----------

CSemaphore semaMutohGA1, semaMutohGA2; 

UINT mutohGA1Thread(LPVOID pParam) {
	CSingleLock semlock(&semaMutohGA1);
	semlock.Lock();
	ForCompare_MutohGA* ga = (ForCompare_MutohGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}
UINT mutohGA2Thread(LPVOID pParam) {
	CSingleLock semlock(&semaMutohGA2);
	semlock.Lock();
	ForCompare_MutohGA* ga = (ForCompare_MutohGA *)pParam;		ga->fitness();		ga->selection();	ga->crossover();	ga->mutation();
	semlock.Unlock();
	return 0;
}

UINT mutohGA1Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaMutohGA1);
	semlock.Lock();
	ForCompare_MutohGA* ga = (ForCompare_MutohGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}
UINT mutohGA2Thread_end(LPVOID pParam) {
	CSingleLock semlock(&semaMutohGA2);
	semlock.Lock();
	ForCompare_MutohGA* ga = (ForCompare_MutohGA *)pParam;		ga->fitness();
	semlock.Unlock();
	return 0;
}

// =====================================================================================================

void ForCompare_Main::runClassicalGA(double *result) {
	int halfPopSize = popSize/2;
	ForCompare_ClassicalGA ga1(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga1.init();		ga1.createPopulation();	// dung thread cho init nay cung duoc
	ForCompare_ClassicalGA ga2(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga2.init();		ga2.createPopulation();

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// ---- threads for running GA ----
	for(int i = 0; i < gaIteration; i++) {
		CWinThread * pclassicalGA1Thread = AfxBeginThread(classicalGA1Thread, &ga1);		pclassicalGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pclassicalGA1Thread->ResumeThread();
		CWinThread * pclassicalGA2Thread = AfxBeginThread(classicalGA2Thread, &ga2);		pclassicalGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pclassicalGA2Thread->ResumeThread();
		CSingleLock ga1Lock(&semaClassicalGA1);		ga1Lock.Lock();
		CSingleLock ga2Lock(&semaClassicalGA2);		ga2Lock.Lock();
		ga1.exchangeChromosomes(ga2.getChromosomes()); // <==
		ga1Lock.Unlock();
		ga2Lock.Unlock();
	}

	// fitness cuoi cung cua vong lap
	CWinThread * pclassicalGA1Thread = AfxBeginThread(classicalGA1Thread_end, &ga1);		pclassicalGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pclassicalGA1Thread->ResumeThread();
	CWinThread * pclassicalGA2Thread = AfxBeginThread(classicalGA2Thread_end, &ga2);		pclassicalGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pclassicalGA2Thread->ResumeThread();
	CSingleLock ga1Lock(&semaClassicalGA1);		ga1Lock.Lock();
	CSingleLock ga2Lock(&semaClassicalGA2);		ga2Lock.Lock();
	// do nothing here
	ga1Lock.Unlock();
	ga2Lock.Unlock();

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	unsigned long timeDiff = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP

	// ---- return ----
	result[0] =  ga1.getBestA11();
	result[1] =  ga1.getBestA22();
	result[2] =  ga1.getBestB1();
	result[3] =  ga1.getBestB2();
	result[4] =  ga1.getBestError();
	result[5] =  double(timeDiff);
}

void ForCompare_Main::runMoscheniGA(double *result) {
	int halfPopSize = popSize/2;
	ForCompare_MoscheniGA ga1(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga1.init();		ga1.createPopulation();	// dung thread cho init nay cung duoc
	ForCompare_MoscheniGA ga2(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga2.init();		ga2.createPopulation();

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// ---- threads for running GA ----
	for(int i = 0; i < gaIteration; i++) {
		CWinThread * pmoscheniGA1Thread = AfxBeginThread(moscheniGA1Thread, &ga1);		pmoscheniGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmoscheniGA1Thread->ResumeThread();
		CWinThread * pmoscheniGA2Thread = AfxBeginThread(moscheniGA2Thread, &ga2);		pmoscheniGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmoscheniGA2Thread->ResumeThread();
		CSingleLock ga1Lock(&semaMoscheniGA1);		ga1Lock.Lock();
		CSingleLock ga2Lock(&semaMoscheniGA2);		ga2Lock.Lock();
		ga1.exchangeChromosomes(ga2.getChromosomes()); // <==
		ga1Lock.Unlock();
		ga2Lock.Unlock();
	}

	// fitness cuoi cung cua vong lap
	CWinThread * pmoscheniGA1Thread = AfxBeginThread(moscheniGA1Thread_end, &ga1);		pmoscheniGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmoscheniGA1Thread->ResumeThread();
	CWinThread * pmoscheniGA2Thread = AfxBeginThread(moscheniGA2Thread_end, &ga2);		pmoscheniGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmoscheniGA2Thread->ResumeThread();
	CSingleLock ga1Lock(&semaMoscheniGA1);		ga1Lock.Lock();
	CSingleLock ga2Lock(&semaMoscheniGA2);		ga2Lock.Lock();
	// do nothing here
	ga1Lock.Unlock();
	ga2Lock.Unlock();

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	unsigned long timeDiff = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP

	// ---- return ----
	result[0] =  ga1.getBestA11();
	result[1] =  ga1.getBestA22();
	result[2] =  ga1.getBestB1();
	result[3] =  ga1.getBestB2();
	result[4] =  ga1.getBestError();
	result[5] =  double(timeDiff);
}

void ForCompare_Main::runMutohGA(double *result) {
	int halfPopSize = popSize/2;
	ForCompare_MutohGA ga1(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga1.init();		ga1.createPopulation();	// dung thread cho init nay cung duoc
	ForCompare_MutohGA ga2(previousImg, currentImg, halfPopSize, pc, pm, aRangeStart, aRangeEnd, bRangeStart, bRangeEnd);		ga2.init();		ga2.createPopulation();

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);	QueryPerformanceCounter((LARGE_INTEGER*)&tStart); // tick count START

	// ---- threads for running GA ----
	for(int i = 0; i < gaIteration; i++) {
		CWinThread * pmutohGA1Thread = AfxBeginThread(mutohGA1Thread, &ga1);		pmutohGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmutohGA1Thread->ResumeThread();
		CWinThread * pmutohGA2Thread = AfxBeginThread(mutohGA2Thread, &ga2);		pmutohGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmutohGA2Thread->ResumeThread();
		CSingleLock ga1Lock(&semaMutohGA1);		ga1Lock.Lock();
		CSingleLock ga2Lock(&semaMutohGA2);		ga2Lock.Lock();
		ga1.exchangeChromosomes(ga2.getChromosomes()); // <==
		ga1Lock.Unlock();
		ga2Lock.Unlock();
	}

	// fitness cuoi cung cua vong lap
	CWinThread * pmutohGA1Thread = AfxBeginThread(mutohGA1Thread_end, &ga1);		pmutohGA1Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmutohGA1Thread->ResumeThread();
	CWinThread * pmutohGA2Thread = AfxBeginThread(mutohGA2Thread_end, &ga2);		pmutohGA2Thread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);//		pmutohGA2Thread->ResumeThread();
	CSingleLock ga1Lock(&semaMutohGA1);		ga1Lock.Lock();
	CSingleLock ga2Lock(&semaMutohGA2);		ga2Lock.Lock();
	// do nothing here
	ga1Lock.Unlock();
	ga2Lock.Unlock();

	QueryPerformanceCounter((LARGE_INTEGER*)&tStop);	unsigned long timeDiff = (unsigned long)((tStop - tStart) * 1000 / freq); // tick count STOP

	// ---- return ----
	result[0] =  ga1.getBestA11();
	result[1] =  ga1.getBestA22();
	result[2] =  ga1.getBestB1();
	result[3] =  ga1.getBestB2();
	result[4] =  ga1.getBestError();
	result[5] =  double(timeDiff);
}