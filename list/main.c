#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int main(int argc, char** argv)
{
    List *l1 = list_create();
    if (l1 == NULL) {
        printf("can't create l1 because of malloc fail!\n");
    }

    for (int i = 0; i < 10; i++) {
        List *l2 = list_create();
        if (l2 == NULL) {
            printf("can't create l2:%d because of malloc fail!\n", i);
        }

        for (int j = 0; j < 10; j++) {
            int *a = (int *)malloc(sizeof(int));
            *a = i * 10 + j + 1;
            if (0 != list_append_tail(l2, a)) {
                printf("can't create node:%d because of malloc fail!\n", i * 10 + j);
            }
        }

        if (0 != list_append_head(l1, l2)) {
            printf("can't create node:%d because of malloc fail!\n", i);
        }
    }
    
    list_set_curr_2_head(l1);
    List *l3 = (List *)list_curr_data(l1);
    while (l3 != NULL) {
        list_set_curr_2_head(l3);
        int *p = (int *)list_curr_data(l3);
        while (p != NULL) {
            printf("%d,", *p);
            p = (int *)list_curr_data(l3);
        }
        printf("\n");
        l3 = (List *)list_curr_data(l1);
    }
 
    list_set_curr_2_head(l1);
    l3 = (List *)list_curr_data(l1);
    while (l3 != NULL) {
        list_set_curr_2_head(l3);
        int *p = (int *)list_curr_data(l3);
        while (p != NULL) {
            free(p);
            p = (int *)list_curr_data(l3);
        }
        list_free(l3);
        l3 = (List *)list_curr_data(l1);
    }

    list_free(l1);
    return 0;
}

