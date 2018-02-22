#include <stdio.h>
#include "queue.h"

#define SIZE 100
int main(int argc, char** argv)
{
    int a[SIZE];
    Queue *q = queue_init(SIZE);
    if (q == NULL) {
        printf("queue init malloc error\n");
    } else {
        printf("size_q:%d, size_mem:%d\n", q->size_q, q->size_mem);
        printf("queue full:%d\n", queue_full(q));
        printf("queue empty:%d\n", queue_empty(q));
    }

    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        if (0 != queue_push(q, (void *)(&a[i]))) {
            printf("push a[%d] error\n", i);
        }
    }
    printf("size_q:%d, size_mem:%d\n", q->size_q, q->size_mem);
    printf("queue full:%d\n", queue_full(q));
    printf("queue empty:%d\n", queue_empty(q));

    if (0 != queue_push(q, (void *)(&a[0]))) {
        printf("push a[%d] error\n", 0);
    }
    printf("size_q:%d, size_mem:%d\n", q->size_q, q->size_mem);

    int *c = NULL;
    for (int i = 0; i < 10; i++) {
        if (0 != queue_pop(q, (void **)&c)) {
            printf("pop a[%d] error\n", i);
        }
        printf("size_q:%d, size_mem:%d\n", q->size_q, q->size_mem);
        printf("*c:%d\n", *c);
    }
    printf("queue full:%d\n", queue_full(q));
    printf("queue empty:%d\n", queue_empty(q));
    for (int i = 0; i < SIZE; i++) {
        if (0 != queue_pop(q, (void **)&c)) {
            printf("pop a[%d] error\n", i);
        }
    }
    printf("queue full:%d\n", queue_full(q));
    printf("queue empty:%d\n", queue_empty(q));
    queue_free(q);
    return 0;
}

