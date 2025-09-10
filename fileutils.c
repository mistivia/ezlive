#include "fileutils.h"

bool fwrite_word16le(FILE* fp, uint16_t x) {
    uint8_t buf[2];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    int r = fwrite(buf, 1, 2, fp);
    if (r != 2) return false;
    return true;
}

bool fwrite_word32le(FILE* fp, uint32_t x) {
    bool ret = false;
    uint16_t buf[2];

    buf[0] = x & 0xffff;
    buf[1] = (x >> 16) & 0xffff;
    ret = fwrite_word16le(fp, buf[0]);
    if (!ret) return ret;
    ret = fwrite_word16le(fp, buf[1]);
    if (!ret) return ret;
    return true;
}

bool fwrite_word16be(FILE* fp, uint16_t x) {
    uint8_t buf[2];
    buf[1] = x & 0xff;
    buf[0] = (x >> 8) & 0xff;
    int r = fwrite(buf, 1, 2, fp);
    if (r != 2) return false;
    return true;
}

bool fwrite_word32be(FILE* fp, uint32_t x) {
    bool ret = false;
    uint16_t buf[2];

    buf[1] = x & 0xffff;
    buf[0] = (x >> 16) & 0xffff;
    ret = fwrite_word16be(fp, buf[0]);
    if (!ret) return ret;
    ret = fwrite_word16be(fp, buf[1]);
    if (!ret) return ret;
    return true;
}

bool fwrite_word24le(FILE* fp, uint32_t x) {
    uint8_t buf[3];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    buf[2] = (x >> 16) & 0xff;
    int r = fwrite(buf, 1, 3, fp);
    if (r != 3) return false;
    return true;
}

bool fwrite_word24be(FILE* fp, uint32_t x) {
    uint8_t buf[3];
    buf[2] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    buf[0] = (x >> 16) & 0xff;
    int r = fwrite(buf, 1, 3, fp);
    if (r != 3) return false;
    return true;
}

bool fwrite_char(FILE* fp, uint8_t x) {
    uint8_t buf[1];
    buf[0] = x;
    int ret = fwrite(buf, 1, 1, fp);
    return ret == 1;
}