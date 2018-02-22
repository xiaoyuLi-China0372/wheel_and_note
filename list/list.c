#include <stdlib.h>
#include "list.h"

List* list_create()
{
    List *list = (List *)malloc(sizeof(List));
    if (list == NULL) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->curr = NULL;

    return list;
}

void* list_curr_data(List *list)
{
    void *ret = NULL;
    if (list->curr != NULL) {
        ret = list->curr->data;
        list->curr = list->curr->next;
    }

    return ret;
}

int list_append_head(List *list, void *data)
{
    Element *elt = (Element *)malloc(sizeof(Element));
    if (elt == NULL) {
        return -1;
    }

    elt->data = data;
    elt->next = list->head;

    list->head = elt;
    if (list->tail == NULL) {
        list->tail = elt;
    }

    return 0;
}

int list_append_tail(List *list, void *data)
{
    Element *elt = (Element *)malloc(sizeof(Element));
    if (elt == NULL) {
        return -1;
    }

    elt->data = data;
    elt->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = elt;
    } else if (list->head == NULL){
        list->head = elt;
    }
    list->tail = elt;

    return 0;
}

void list_free(List *list)
{
    Element *elt = list->head;
    while (elt != NULL) {
        Element *next = elt->next;
        free(elt);
        elt = next;
    }

    free(list);
}
