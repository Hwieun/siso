#include "Thread.h"
#include "MsgQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "Plus.h"

void _InitMsgQueue(void)
{
		
}

int 	mymsgget(int key, int msgflg)
{
	int msqid;
	Qcb* qcb = (Qcb*)malloc(sizeof(Qcb)); //Message Q
	int entry = 0; 
	if(qcb == NULL)
	{	
		printf("malloc error!!\n");
		exit(0);
	}
//	printf("message queue get!!\n");
	qcbtable[tindex].tid = pCurTCB->tid;
	int i =0;
	for(i = 0; i<10; i++)
	{
		if(qcbTblEntry[i].key == key)
		{
			qcbtable[tindex].msqid = i;
			return i;
		}
	}

	qcb->msgCount = 0; //# of message
	qcb->pMsgHead = qcb->pMsgTail = NULL; //linked list of message
	qcb->waitThreadCount = 0; //# of thread node in waiting queue 
	qcb->pThreadHead = qcb->pThreadTail = NULL; 
	//head, tail of waitingQ

	entry = SearchEntry(); //search empty entry

	qcbtable[tindex].msqid = entry;
	qcbTblEntry[entry].key = key;
	qcbTblEntry[entry].pQcb = qcb;
		
	tindex++;
	return entry; //or change to msqid(real)
}

int 	mymsgsnd(int msqid, const void *msgp, int msgsz, int msgflg)
{
	Qcb* qcb;
	int i = 0; 
	Thread* tcb; 
	Message* msg = (Message*)malloc(sizeof(Message));
	if(msg == NULL)
		perror("malloc error\n");
	memset(msg, 0, sizeof(msg));
	//	printf("start to send message\n");
	strncpy(&(msg->type), msgp, sizeof long);
	strcpy(msg->data, ((Message *)msgp)->data);
	pCurTCB->status = 1;
//	printf("msg data : %s ver. SendFunc\n", msg->data);
	msg->size = msgsz;
	
	qcb = qcbTblEntry[msqid].pQcb;
	InsertMsg(qcb, msg);
//	printf("send\n");
//	printf("send type : %d\n", msg->type);
//	sleep(TIMESLICE);
	tcb = qcb->pThreadHead; 
	while(tcb != NULL) 
	{
		if(tcb->type == msg->type)
		{
			tcb->status = 1;
			DetachedTCB(tcb, qcb, 1);	
			InsertTCB(tcb, NULL, 0);
			pCurTCB = tcb;
			break;
		}
		tcb = tcb->pNext;
	}
return msgsz;
}

int	mymsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
	//ReadyQ HEAD TCB
	Thread* tcb = TraverseTCB(pCurTCB->parentTid, NULL, 0);
	pthread_t pptid = tcb->parentTid;
	tcb = pCurTCB;
	Qcb* qcb = qcbTblEntry[msqid].pQcb;
//	printf("in RCB\n");
	Message* msg = TraverseMsg(qcb, msgtyp);
	int sz;
	if(msg == NULL)
	{
		pCurTCB->status = 2; 
		pCurTCB->type = msgtyp;	
//		printf("receive type : %d\n", pCurTCB->type);
		pthread_kill(pptid, SIGUSR2); //sleep
		usleep(100);
		while(1)
		{		
			msg = TraverseMsg(qcb, msgtyp);
			if(msg==NULL)
			{
//				printf("no message\n");
				pCurTCB = tcb;	
				pCurTCB->status = 2;
				pCurTCB->type = msgtyp;	
				pthread_kill(pptid, SIGUSR2);
				usleep(100);
			}
			else 
			{
				pCurTCB->status = 1;
				break;
			}
		}
	}
//	printf("complete copy\n");	
	strncpy(msgp, &(msg->type), 4);
	sz = strlen(msg->data);
	strcpy(((Message*)msgp)->data, msg->data);
//	printf("complete copy\n");	
	DeleteMsg(qcb, msg);
//	printf("finish to receive message\n");
	return sz;
}

int 	mymsgctl(int msqid, int cmd, void* buf)
{
	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	if(qcb->msgCount != 0)
		return -1;
	int i = 0; 
		for(i= 0;i<tindex; i++)
			if(qcbtable[i].msqid == msqid)
			{
				qcbtable[i].tid = 0;
				qcbtable[i].msqid = 0;
			}
//	int key=0;
//	printf("message Q Delete!!\n");
	qcbTblEntry[msqid].key = -1;
	qcbTblEntry[msqid].pQcb = NULL;

	free(qcb);
//	exit(0);
	return 0;
}
