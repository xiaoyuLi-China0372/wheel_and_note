#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED
#include <stdbool.h>
#include <stddef.h>

typedef struct Element {
	void *data;
	struct Element *next;
} Element;

typedef struct List {
	Element *head;
	Element *tail;
	Element *curr;
} List;

List* list_create();

static inline void list_set_curr_2_head(List *list)
{
    list->curr = list->head;
}

static inline bool list_curr_is_end(List *list)
{
    return list->curr != NULL ? false : true;
}

static inline void *list_curr_data(List * list)
{
    void *ret = list->curr->data;
    list->curr = list->curr->next;
    return ret;
}

int list_append_head(List *list, void *data);

int list_append_tail(List *list, void *data);

void list_free(List *list);

#endif // LIST_H_INCLUDED

