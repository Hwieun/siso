#include "Plus.h"
#include "MsgQueue.h"
#include <stdio.h>

int SearchEntry(void) //flag : 0 emptyentry
	//flag : 1 sameentry
{
	int i, rkey;
	
	for(i=0; i<MAX_QCB_SIZE; i++)
	{
		rkey = qcbTblEntry[i].key;
		if(rkey == -1)
			return i;
	}
	if(i == MAX_QCB_SIZE)
		return -1;
}
void InsertMsg(Qcb* qcb, Message* msg)
{
	if((qcb->msgCount) == 0)
	{
		qcb->pMsgHead = qcb->pMsgTail = msg;
	}
	else
	{
		msg->pPrev = qcb->pMsgTail;
		qcb->pMsgTail->pNext = msg;
		msg->pNext = NULL;
		qcb->pMsgTail = msg;
	}
	(qcb->msgCount)++;
}

Message* TraverseMsg(Qcb* qcb, long type)
{
	int i=0;
	Message* msg = qcb->pMsgHead;
	for(i=0; i<(qcb->msgCount); i++)
	{
		if(msg->type == type)
			return msg;
			msg = msg->pNext;
	}
	return NULL; //못찾음
}
void DeleteMsg(Qcb* qcb, Message* msg)
{
	if((qcb->msgCount)==1)
		qcb->pMsgHead = qcb->pMsgTail = NULL;
	else
	{
		if(msg->pPrev == NULL)
		{
			qcb->pMsgHead = msg->pNext;
			qcb->pMsgHead->pPrev = NULL;
		}
		else if(msg->pNext == NULL)
		{
			qcb->pMsgTail = msg->pPrev;
			msg->pPrev->pNext = NULL;
		}
		else
		{
			msg->pPrev->pNext = msg->pNext;
			msg->pNext->pPrev = msg->pPrev;
		}
	}
	(qcb->msgCount)--;
	free(msg);
}
