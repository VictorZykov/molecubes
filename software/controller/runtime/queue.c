#include "global.h"
#include "queue.h"
#include "malloc.h"
#include "rprintf.h"

#define NULL ((void *) 0)

void initQueueArray(QUEUE *q, int count) {
	int i;
	for(i=0; i<count; i++)
	{
		initQueue(&q[i]);
	}
}

void initQueue(QUEUE *q) {
	q->count = 0;
	q->id = 0xFE; // default address
	q->head = NULL;
	q->tail = NULL;
}

void enqueue(QUEUE *q, NODE *lnode) {
	if(q->head == NULL) {
		q->head = lnode;
		lnode->prev = NULL;
		q->count = 1;
	} else {
		q->tail->next = lnode;
		lnode->prev = q->tail;
		q->count++;
	}

	q->tail = lnode;
	lnode->next = NULL;
}

u08 dequeue(QUEUE *q)
{
	if(q->head == NULL) // queue empty error
		return -1;
	if(q->tail == q->head) // final element in queue
	{
		free(q->tail);
		q->tail = NULL;
		q->head = NULL;
		q->count = 0;
	}
	else
	{
		q->head = q->head->next;
		free(q->head->prev);
		q->count--;
	}
	return 0;
}

NODE *getHead(QUEUE *q) {
	return q->head;
}

NODE *getTail(QUEUE *q) {
	return q->tail;
}

void *getNodePtr(NODE *n) {
	return n->ptr;
}

u08 getQueueID(QUEUE *q) {
	return q->id;
}

void setQueueID(QUEUE *q, u08 id){
	q->id = id;
}

// returns first Queue with particular id
int findQueueWithAddress(u08 id, QUEUE *Qarray, u08 count, int *index) {
	int i;
	for(i=0; i<count; i++)
	{
		if(Qarray[i].id == id)
		{
			*index = i;
			return TRUE;
		}
	}
	return FALSE;
}

