#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int uint32_t;

void max_child(char *in)
{
    int n = strlen(in);
    int *num = (int*) malloc(n * sizeof(int));
    int index = 0;
    int i = 0;
    int j = 0;
    int max = 1;
    for (i = 0; i < n; i++) {
        num[i] = 1;
    }

    for (i = 0; i < n; i++) {
        int max_num[2] = {1, 1};
        for (j = 1; i+j < n && j <= i; j++) {
            if (in[i+j] == in[i-j]){
                max_num[0] = j * 2 + 1;
            } else {
                break;
            }
        }
        for (j = 1; i+j < n && j <= i + 1; j++) {
            if (in[i+j] == in[i-j+1]){
                max_num[1] = j * 2;
            } else {
                break;
            }
        }
        num[i] = max_num[0] > max_num[1] ? max_num[0] : max_num[1];
        if (num[i] > max) {
            index = i;
            max = num[i];
        }
    }

    int t1 = max / 2;
    int t2 = (max % 2) ? 0 : 1;
    for (i = index - t1 + t2; i <= index + t1; i++) {
        printf("%c", in[i]);
    }
    printf("\n");
    free(num);
}

int main(int argc, char** argv)
{
    char input[100];
    fgets(input, 100, stdin);

    max_child(input);

    return 0;
}
