#include "Thread.h"
#include "Init.h"
#include "Scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include "Plus.h"

ThreadStatus stat; //enum {}을 ThreadStatus type으로 치환, enum 변수 선언

void retcc(void);

void InsertTCB(Thread* TCB, Qcb* qcb, char flag) //무조건 뒤로 
{//flag : 0->ready, flag : 1->wait

//	printf("insert at %d\n", flag);
	if(flag == 0){
		if(countR == 0)
			ReadyQHead = ReadyQTail = TCB;
		else{	
			ReadyQTail->pNext = TCB;
			TCB->pPrev = ReadyQTail;
			TCB->pNext = NULL;
			TCB->bRunnable = 0;
			TCB->status = 1;
			ReadyQTail = TCB;
		}
			++countR;
	}
	else if(qcb != NULL && flag ==1)
	{
		if(qcb->pThreadHead == NULL)
			qcb->pThreadHead = qcb-> pThreadTail= TCB;
		else
		{
			qcb->pThreadTail-> pNext =TCB;
			TCB->pPrev= qcb->pThreadTail;
			TCB->pNext = NULL;
			TCB->bRunnable=0;
			TCB->status = 2;
			qcb->pThreadTail = TCB;
		}
		++(qcb->waitThreadCount);
	}
	else
	{
		if(WaitQHead == NULL)
			WaitQHead = WaitQTail = TCB;
		else
		{
			WaitQTail -> pNext =TCB;
			TCB->pPrev= WaitQTail;
			TCB->pNext = NULL;
			TCB->bRunnable=0;
			TCB->status = 2;
			WaitQTail = TCB;
		}
		++countW;
	}
}

void DeleteTCB(Thread* TCB) //삭제
{

	pthread_cond_destroy(&(TCB->readyCond));
	pthread_mutex_destroy(&(TCB->readyMutex));
	free(TCB);
}

Thread* TraverseTCB(thread_t ttid, Qcb* qcb, char flag)
{	
	Thread* tTCB;
	if(flag == 0)
		tTCB = ReadyQHead;
	else
		tTCB = qcb->pThreadHead;

	while(tTCB != NULL)
	{
		char fl = pthread_equal((tTCB->tid), ttid);
		if(fl)
			return tTCB;
		tTCB = tTCB->pNext;
	}
	return NULL; //못찾음
}

void DetachedTCB(Thread* TCB, Qcb* qcb, char flag) //TCB 떼내기
{
//	printf("detach at %d\n", flag);
	if(flag == 0){ //0 : ready
		if(countR == 0)
		{
			printf("error : don't exist tcb to detach\n");
			exit(0);
		}
		else if(countR == 1)
		{
			ReadyQHead = ReadyQTail = NULL;
		}
		else
		{ //중간 TCB or HEAD TCB
			Thread* tcb = TCB->pPrev; 
			if(tcb != NULL)
			{
				tcb->pNext = TCB->pNext;
				tcb = TCB->pNext;
				if(tcb==NULL)
					ReadyQTail = tcb;
				tcb->pPrev = TCB->pPrev;
			}
			else
			{
				ReadyQHead = TCB->pNext;
				ReadyQHead->pPrev = NULL;
			}
		}
		TCB->pPrev = TCB->pNext =NULL;
		--countR;
	}
	else if(qcb != NULL && flag == 1)
	{ //1 : wait
		if(qcb->waitThreadCount == 0)
			printf("no TCB detached in waiting Q\n");
		else if(qcb->waitThreadCount == 1)
		{
			qcb->pThreadHead = qcb->pThreadTail = NULL;
		}
		else
		{
	//		Thread* tcb = TCB->pPrev; 
	//		if(TCB->pPrev != NULL)
			if(TCB != qcb->pThreadHead)
			{
				TCB->pPrev->pNext = TCB->pNext;
			//	tcb = TCB->pNext;
				if(TCB->pNext == NULL)
				{
					qcb->pThreadTail = TCB;
				}
				TCB->pNext->pPrev = TCB->pPrev;
			}
			else 
			{	
				TCB->pNext->pPrev = NULL;
				qcb->pThreadHead = TCB->pNext;
			}
		}
		TCB->pPrev = TCB->pNext =NULL;
		--(qcb->waitThreadCount);
	}
}

int thread_create(thread_t *thread, thread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	Thread* TCB = (Thread*)malloc(sizeof(Thread));
	if(TCB == NULL)
	{
		printf("malloc error!!\n");
		exit(0);
	}
	printf("thread creating..\n");	
	WrapperArg Warg;
	Warg.funcPtr = start_routine;
	Warg.funcArg = arg;
	pthread_create(thread, attr, Wrapper, &Warg); //thread 생성. thread 변수에 tid 저장
	//초기화
	pthread_mutex_t tmu = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t tco = PTHREAD_COND_INITIALIZER;
	
	sleep(1);
	stat = THREAD_STATUS_READY;
	TCB->status = stat;
	TCB->tid = *thread;
	TCB->readyCond = tco;
	TCB->readyMutex = tmu;
	TCB->pPrev = TCB->pNext = NULL;
	TCB->pExitCode == NULL;
	TCB->parentTid = thread_self();
	//readyQueue
	InsertTCB(TCB, NULL, 0);	
	signal(SIGUSR2, retcc);
	pthread_kill(*thread, SIGUSR2); //tid, signo
		
	printf("create\n");
	return 1;
}

void __thread_wait_handler(int signo){
	Thread *pTh;

	pTh = TraverseTCB(thread_self(), NULL, 0); //readyQ에서 찾음
	if(pTh == NULL)
	{
		int i = 0;
		for(i =0;i<tindex;i++)
			if(qcbtable[i].tid == thread_self())
				break;
		int ind = qcbtable[i].msqid;
		pTh = TraverseTCB(thread_self(), qcbTblEntry[ind].pQcb,1); //waitQ에서 찾음
	}
	if(pTh == NULL)
		pTh = pCurTCB;
	if(pTh == NULL)
		exit(5);
	
//	printf("tid : %d  start to sleep..\n", pTh->tid);
	pthread_mutex_lock(&(pTh->readyMutex));
	while(pTh->bRunnable == 0)
	{
		pthread_cond_wait(&(pTh->readyCond), &(pTh->readyMutex));
	}
	pthread_mutex_unlock(&(pTh->readyMutex));
}

void* Wrapper(void* arg){
	void* ret = NULL;
	WrapperArg* pArg = (WrapperArg*)arg;
	sigset_t set;
	int retSig;
	
	void *(*funcPtrR)(void*) = pArg->funcPtr;
	void* funcArgR = pArg->funcArg;

	//sleep until TCB is initialized
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2); //SIGUSR1로 해서 handler실행되지 않게
	sigwait(&set, &retSig); //SIGUSR2 빼고 반환
	//child ready to run
	__thread_wait_handler(0);
	
	//child execute
	ret = funcPtrR(funcArgR);
	return ret;
}

void __thread_wakeup(Thread* pTh)
{
//	printf("tid : %d  wake\n", pTh->tid);
	pthread_mutex_lock(&(pTh->readyMutex));
	pTh->bRunnable = 1;
	pCurTCB = pTh;
	pTh->status = THREAD_STATUS_RUN;
	pthread_cond_signal(&(pTh->readyCond));
	pthread_mutex_unlock(&(pTh->readyMutex));
}

int thread_join(thread_t thread, void **retval)
{
//	printf("current TCB count : %d\n", countR);
	Thread* joinTCB;
	joinTCB = TraverseTCB(thread, NULL, 1); //보류
	if(joinTCB == NULL)
		joinTCB = TraverseTCB(thread, NULL, 0); //traverse at readyQ
	pthread_mutex_lock(&(pCurTCB->readyMutex));
	while((joinTCB->pExitCode)==NULL)
	{
		pthread_cond_wait(&(joinTCB->readyCond), &(pCurTCB->readyMutex));	
	}
	pthread_mutex_unlock(&(pCurTCB->readyMutex));
//	printf("join\n");
	*retval = (void *)(joinTCB->pExitCode); 
	DetachedTCB(joinTCB, NULL, 1); //readyQ
	
//	printf("delete\n");

	return 0;
}


int 	thread_suspend(thread_t tid)
{
	Thread* tcb = TraverseTCB(tid, NULL, 0);
	tcb->status = THREAD_STATUS_BLOCKED;
	tcb->bRunnable = -1;	
	InsertTCB(tcb,NULL, 1);
	DetachedTCB(tcb, NULL, 0);
}


int	thread_resume(thread_t tid)
{

	Thread* tcb = TraverseTCB(tid, NULL, 1); //보류
	tcb->status = THREAD_STATUS_READY;
	tcb->bRunnable = 0;	
	InsertTCB(tcb, NULL, 0);
	DetachedTCB(tcb, NULL, 1);
}

thread_t	thread_self() //return present thread id
{
	return pthread_self();
}
int thread_exit(void* retval)
{
//	Thread* ptcb;
//	ptcb = TraverseTCB((pCurTCB->parentTid), 0);
	pthread_mutex_lock(&(pCurTCB->readyMutex));
	pCurTCB->pExitCode = retval;	
//	DetachedTCB(pCurTCB,0);
	InsertTCB(pCurTCB, NULL, 1);
	pthread_cond_signal(&(pCurTCB->readyCond));
	pthread_mutex_unlock(&(pCurTCB->readyMutex));

	return 1;
}
void retcc(void)
{
	return;
}
