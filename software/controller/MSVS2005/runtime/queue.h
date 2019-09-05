#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "global.h"

typedef struct node {
	void *ptr;
	struct node *next;
	struct node *prev;
}NODE;

typedef struct queue {
	u08 id;
	int count;
	NODE *head;
	NODE *tail;
} QUEUE;

void initQueueArray(QUEUE *q, int count);
void initQueue(QUEUE *q);
void enqueue(QUEUE *q, NODE *lnode);
u08 dequeue(QUEUE *q);
NODE *getHead(QUEUE *q);
NODE *getTail(QUEUE *q);
void *getNodePtr(NODE *n);
u08 getQueueID(QUEUE *q);
void setQueueID(QUEUE *q, u08 id);
int findQueueWithAddress(u08 id, QUEUE *Qarray, u08 count, int *index);

#endif
