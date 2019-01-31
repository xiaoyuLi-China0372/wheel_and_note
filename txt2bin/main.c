#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "file_op.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("./run bin_file txt_file\n");
        printf("./run txt_file bin_file -r\n");
        return -1;
    }

    uint32_t size = get_file_size(argv[1]);
    printf("file size:%u\n", size);

    uint8_t *buffer = (uint8_t *)malloc(size * sizeof(uint8_t));
    if (buffer == NULL) {
        printf("buffer malloc fails\n");
        return -1;
    }

    if (argc > 3 && !strcmp(argv[3], "-r")) {
        size = read_debug_data(argv[1], buffer, size);
        printf("file size:%u\n", size);
        write_bin_data(argv[2], buffer, size);
    } else {
        read_bin_data(argv[1], buffer, size);
        write_debug_data(argv[2], buffer, size);
    }

    free(buffer);
    return 0;
}
