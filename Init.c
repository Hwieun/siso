#include "Thread.h"

#include "Init.h"
#include "MsgQueue.h"
#include <pthread.h>
#include <signal.h>
#include "Plus.h"

void Init(){
	int i =0;
	
	for(i=0;i<MAX_QCB_SIZE;i++)
	{	
		qcbTblEntry[i].key = -1;
		qcbTblEntry[i].pQcb = NULL;
	}//table init
	
	ReadyQHead = ReadyQTail = NULL;
	WaitQHead = WaitQTail = NULL;
	pCurTCB = NULL;
	countR = countW = 0;	
	tindex = 0;
	signal(SIGUSR1, __thread_wait_handler);
}
