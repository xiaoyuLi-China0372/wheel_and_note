#include "file_op.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

uint32_t get_file_size(const char *path)
{
    uint32_t fileSize = 0;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0){
        printf("%s: stat errno:%d\n", path, errno);
        FILE *file = fopen(path, "rb");
        if (file == NULL)
            printf("ERROR: fopen:%s\n", path);
        int rc = fseek(file, 0, SEEK_END);
        if (rc)
            printf("ERROR: fseek end\n");

        fileSize = ftell(file);

        rc = fclose(file);
        if (rc)
            printf("ERROR : fclose\n");
    } else {
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

    const char *base_name = baseName(fileName);
    char data_name[strlen(base_name) + 1];
    strcpy(data_name, base_name);
    for (char *ptr = data_name; *ptr != '\0'; ptr++) {
        if (*ptr == '.') {
            *ptr = '\0';
            break;
        }
    }

    fprintf(file, "unsigned char %s[] = {\n", data_name);
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
    fprintf(file, "};\n");

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

const char *baseName(const char *path)
{
    const char *base = path;
    if (base != NULL) {
        while(*path != '\0') {
            if (*path == '/')
                base = path + 1;
            path++;
        }
    }
    return base;
}
