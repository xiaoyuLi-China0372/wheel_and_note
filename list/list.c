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
    } else if (list->head == NULL) {
        list->head = elt;
    }
    list->tail = elt;

    return 0;
}

void list_reverse(List *list)
{
    Element *head = NULL;
    Element *front = list->head;
    Element *rear = NULL;

    if (front == NULL)
        return;

    while (front->next != NULL) {
        rear = front->next;
        front->next = head;
        head = front;
        front = rear;
    }
    front->next = head;
    head = front;

    list->tail = list->head;
    list->head = head;
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
