#ifndef __PLUS_H_
#define __PLUS_H_
#include "Thread.h"
#include "MsgQueue.h"
#include <unistd.h>

Thread* pCurTCB;

void InsertTCB(Thread* TCB, Qcb* qcb, char flag);
Thread* TraverseTCB(thread_t ttid, Qcb* qcb, char flag);
void DetachedTCB(Thread* TCB, Qcb* qcb, char flag);
void DeleteTCB(Thread* TCB);
void __thread_wait_handler(int signo);
void* Wrapper(void* arg);
void __thread_wakeup(Thread* pTh);
void PrintTCB(Thread* TCB);
int SearchEntry(void);
void InsertMsg(Qcb* qcb, Message* msg);
Message* TraverseMsg(Qcb* qcb, long type);
void DeleteMsg(Qcb* qcb, Message* msg);

int countR, countW;

int tindex;
typedef struct
{
	pthread_t tid;
	int msqid;
}Schtbl;

Schtbl qcbtable[MAX_QCB_SIZE];

#endif
