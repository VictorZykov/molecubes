#include "stdafx.h"

typedef struct node {
	void *ptr;
	struct node *next;
	struct node *prev;
}NODE;

typedef struct queue {
	int count;
	NODE *head;
	NODE *tail;
} QUEUE;

void initQueue(QUEUE *q, int count);
void enqueue(QUEUE *q, NODE *lnode);
bool dequeue(QUEUE *q);
NODE *getHead(QUEUE *q);
NODE *getTail(QUEUE *q);
void *getNodePtr(NODE *n);

