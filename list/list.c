#include "list.h"
#include "log.h"

List* list_create()
{
    List *list = (List *)cmn_alloc(sizeof(List));
    if (list == NULL) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->curr = NULL;

    return list;
}

int list_remove(List *list, Element *elt)
{
    Element *pre = NULL;
    Element *curr = list->head;
    while (curr != NULL) {
        if (curr == elt) {
            if (curr == list->head)
                list->head = curr->next;
            else
                pre->next = curr->next;

            if (curr == list->tail)
                list->tail = pre;

            return 0;
        }
        pre = curr;
        curr = curr->next;
    }

    return -1;
}

int list_insert_pre(List *list, Element *elt, Element *in)
{
    Element *pre = NULL;
    Element *curr = list->head;
    if (curr == NULL && elt == NULL) {
        in->next = NULL;
        list->head = in;
        list->tail = in;
    }

    while (curr != NULL) {
        if (curr == elt) {
            in->next = curr;
            if (curr == list->head)
                list->head = in;
            else
                pre->next = in;

            return 0;
        }
        pre = curr;
        curr = curr->next;
    }

    return -1;
}

int list_insert_post(List *list, Element *elt, Element *in)
{
    if (elt == NULL && list->head == NULL) {
        in->next = NULL;
        list->head = in;
        list->tail = in;
    } else if (elt != NULL && list->head != NULL) {
        in->next = elt->next;
        elt->next = in;

        if (elt == list->tail)
            list->tail = in;

    } else {
        return -1;
    }

    return 0;
}

int list_append_head(List *list, void *data)
{
    Element *elt = (Element *)cmn_alloc(sizeof(Element));
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
    Element *elt = (Element *)cmn_alloc(sizeof(Element));
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

int list_reverse(List *list)
{
    Element *head = NULL;
    Element *front = list->head;
    Element *rear = NULL;
    int num = 1;

    if (front == NULL) {
        return 0;
    }

    while (front->next != NULL) {
        rear = front->next;
        front->next = head;
        head = front;
        front = rear;
        num++;
    }
    front->next = head;
    head = front;

    list->tail = list->head;
    list->head = head;

    return num;
}

void list_free(List *list)
{
    if (list == NULL)
        return;

    Element *elt = list->head;
    while (elt != NULL) {
        Element *next = elt->next;
        cmn_free(elt);
        elt = next;
    }

    cmn_free(list);
}

