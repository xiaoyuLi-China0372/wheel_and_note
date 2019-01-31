#include "file_op.h"
#include <sys/stat.h>

uint32_t get_file_size(const char *path)
{
    uint32_t fileSize = 0;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        printf("ERROR: get file size fails\n");
    }else{
        fileSize = statbuff.st_size;
    }
    return fileSize;
}

void read_bin_data(char *fileName, uint8_t *buf, int size)
{
    FILE *file;
    int rc;

    file = fopen(fileName, "rb");
    if (file == NULL) {
        printf("ERROR: fopen\n");
    }
    rc = fseek(file, 0, SEEK_SET);
    if (rc) {
        printf("ERROR: fseek\n");
    }

    rc = fread(buf, sizeof(uint8_t), size, file);

    rc = fclose(file);
    if (rc) {
        printf("ERROR : fclose\n");
    }
}

void write_bin_data(char *fileName, uint8_t *buf, int size)
{
    FILE *file;
    int rc;

    file = fopen(fileName, "wb");
    if (file == NULL) {
        printf("ERROR: fopen\n");
    }
    rc = fseek(file, 0, SEEK_SET);
    if (rc) {
        printf("ERROR: fseek\n");
    }

    rc = fwrite(buf, sizeof(uint8_t), size, file);

    rc = fclose(file);
    if (rc) {
        printf("ERROR : fclose\n");
    }
}

void write_debug_data(char *fileName, uint8_t *buf, int size)
{
    FILE *file;
    int rc;
    const int col_n = 16;

    file = fopen(fileName, "w");
    if (file == NULL) {
        printf("ERROR: fopen\n");
    }
    rc = fseek(file, 0, SEEK_SET);
    if (rc) {
        printf("ERROR: fseek\n");
    }

    uint8_t *ptr = buf;
    for (int i = 0; i < size / col_n; i++) {
        for (int j = 0; j < col_n; j++) {
            fprintf(file, "0x%02x, ", *ptr++);
        }
        fprintf(file, "\n");
    }
    for (int i = 0; i < size % col_n; i++) {
        fprintf(file, "0x%02x, ", *ptr++);
    }
    fprintf(file, "\n");

    rc = fclose(file);
    if (rc) {
        printf("ERROR : fclose\n");
    }
}

int read_debug_data(char *fileName, uint8_t *buf, int size)
{
    FILE *file;
    int rc;
    int i = 0;
    uint32_t temp = 0;

    file = fopen(fileName, "rb");
    if (file == NULL) {
        printf("ERROR: fopen\n");
    }
    rc = fseek(file, 0, SEEK_SET);
    if (rc) {
        printf("ERROR: fseek\n");
    }

    while (fscanf(file, "0x%02x, ", &temp) != EOF) {
        buf[i++] = temp;
    }

    rc = fclose(file);
    if (rc) {
        printf("ERROR : fclose\n");
    }

    return i;
}

