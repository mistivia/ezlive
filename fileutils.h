#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool fwrite_word16le(FILE* fp, uint16_t x);
bool fwrite_word24le(FILE* fp, uint32_t x);
bool fwrite_word32le(FILE* fp, uint32_t x);

bool fwrite_word16be(FILE* fp, uint16_t x);
bool fwrite_word24be(FILE* fp, uint32_t x);
bool fwrite_word32be(FILE* fp, uint32_t x);

bool fwrite_char(FILE* fp, uint8_t x);

#endif