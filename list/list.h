#ifndef _LIST_H_INCLUDED_
#define _LIST_H_INCLUDED_
#include "common.h"
#include <stdbool.h>
#include <stddef.h>

BEGIN_DECLS

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

static inline void *list_curr_data(List* list)
{
    void *ret = list->curr->data;
    list->curr = list->curr->next;
    return ret;
}

int list_remove(List *list, Element *elt);

int list_insert_pre(List *list, Element *elt, Element *in);

int list_insert_post(List *list, Element *elt, Element *in);

int list_append_head(List *list, void *data);

int list_append_tail(List *list, void *data);

int list_reverse(List *list);

void list_free(List *list);

END_DECLS

#endif

