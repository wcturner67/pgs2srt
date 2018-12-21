#pragma once
#ifndef BRW_H
#define BRW_H

#include <stdint.h>
#include "intreadwrite.h"

#define DEF(type, name, bytes, read, write)                                  \
static type bytestream_get_ ## name(char **b)                                  \
{                                                                              \
    (*b) += bytes;                                                             \
    return read(*b - bytes);                                                   \
}                                                                              \
static void bytestream_put_ ## name(uint8_t **b, const type value)             \
{                                                                              \
    write(*b, value);                                                          \
    (*b) += bytes;                                                             \
}

DEF(unsigned int, byte, 1, AV_RB8, AV_WB8)
DEF(unsigned int, be16, 2, AV_RB16, AV_WB16)
DEF(unsigned int, be24, 3, AV_RB24, AV_WB24)
DEF(unsigned int, be32, 4, AV_RB32, AV_WB32)
DEF(uint64_t, be64, 8, AV_RB64, AV_WB64)

#endif // BRW_H