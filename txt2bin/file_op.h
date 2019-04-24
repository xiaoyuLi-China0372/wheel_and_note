#ifndef FILE_OP_H_INCLUDED
#define FILE_OP_H_INCLUDED
#include <stdio.h>
#include <stdint.h>

uint32_t get_file_size(const char *path);
void read_bin_data(char *fileName, uint8_t *buf, int size);
void write_bin_data(char *fileName, uint8_t *buf, int size);
void write_debug_data(char *fileName, uint8_t *buf, int size);
int read_debug_data(char *fileName, uint8_t *buf, int size);
const char *baseName(const char *path);

#endif /* FILE_OP_H_INCLUDED */
