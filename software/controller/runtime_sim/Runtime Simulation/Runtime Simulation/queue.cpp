#include "stdafx.h"
#include "queue.h"
#include "malloc.h"

void initQueue(QUEUE *q, int count) {
	int i;
	for(i=0; i<count; i++)
	{
		q[i].count = 0;
		q[i].head = NULL;
		q[i].tail = NULL;
	}
	
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

bool dequeue(QUEUE *q)
{
	if(q->head == NULL)
		return false;
	if(q->tail == q->head)
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
	return true;
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
