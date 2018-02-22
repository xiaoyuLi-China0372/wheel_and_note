#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

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

void* list_curr_data(List *list);

int list_append_head(List *list, void *data);

int list_append_tail(List *list, void *data);

void list_free(List *list);

#endif // QUEUE_H_INCLUDED

