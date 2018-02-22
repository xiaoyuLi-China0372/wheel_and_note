#include <stdlib.h>
#include "queue.h"


Queue* queue_init(int size)
{
    if (size <= 0) {
        return NULL;
    }
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == NULL) {
        return NULL;
    }
    q->size_mem = size;
    q->size_q = 0;
    q->head = (void **)malloc(size * sizeof(void *));
    if (q->head == NULL) {
        free(q);
        return NULL;
    }
    q->front = q->head;
    q->rear = q->head;

    return q;
}

void queue_free(Queue *q)
{
    free(q->head);
    free(q);
}

int queue_push(Queue *q, void *node)
{
    if (queue_full(q)) {
        return -1;
    }

    *(q->rear) = node;
    q->size_q++;

    if (q->rear < (q->head + q->size_mem - 1)) {
        q->rear++;
    } else {
        q->rear = q->head;
    }
    return 0;
}

int queue_pop(Queue *q, void **node)
{
    if (queue_empty(q)) {
        return -1;
    }

    *node = *(q->front);
    q->size_q--;

    if (q->front < (q->head + q->size_mem - 1)) {
        q->front++;
    } else {
        q->front = q->head;
    }
    return 0;
}

