#include "Init.h"
#include "Thread.h"
#include "Scheduler.h"
#include "Plus.h"
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

int	RunScheduler( void )
{
	sleep(1);
	while(1)
	{
	//	printf("scheduler countR : %d\n", countR);
		if((countR ==1 && pCurTCB == NULL) || countR == 0) //first node
		{
			Thread* tcb = ReadyQHead;
			if(tcb != NULL)
			{	DetachedTCB(tcb,NULL, 0);
			pCurTCB = tcb;
			}
			else
				tcb = pCurTCB;
			
			
			__thread_wakeup(tcb);
			
			sleep(7);
		}
		else
		{
			Thread* tcb = pCurTCB;
			
		//put running thread to tail of ready Queue
			if(pCurTCB->pExitCode == NULL && pCurTCB->status != 2)
				InsertTCB(pCurTCB, NULL, 0);
			else
			{
				int i=0;
				
				for(i=0; i<10; i++)
				{
					if(qcbtable[i].tid == pCurTCB->tid)
						break;
				}

				int id = qcbtable[i].msqid;
				if(TraverseTCB(pCurTCB->tid, qcbTblEntry[id].pQcb, 1) == NULL)
				InsertTCB(pCurTCB, qcbTblEntry[id].pQcb,1);
			}
			pCurTCB = ReadyQHead;
			DetachedTCB(pCurTCB, NULL, 0);
			
			__ContextSwitch(tcb, pCurTCB);
		}
			sleep(TIMESLICE);
	}
}

void __ContextSwitch(Thread* pCurThread, Thread* pNewThread)
{	
	//stop current running thread
	if(pCurThread->status != 2)
	pthread_kill(pCurThread->tid, SIGUSR1); 
	//run pNewThread
	__thread_wakeup(pNewThread);
}

