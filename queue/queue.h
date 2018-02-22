#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED
#include <stdbool.h>

typedef struct Queue {
	int size_mem;
	int size_q;
	void **head;
	void **front;
	void **rear;
} Queue;

Queue* queue_init(int size);

static inline bool queue_full(Queue *q)
{
    return q->size_q >= q->size_mem ? true : false;
}

static inline bool queue_empty(Queue *q)
{
    return q->size_q <= 0 ? true : false;
}

void queue_free(Queue *q);

int queue_push(Queue *q, void *node);

int queue_pop(Queue *q, void **node);

#endif // QUEUE_H_INCLUDED

