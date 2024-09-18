#include "sublage/byteorder.h"

float32_t _floatSwap(float32_t value) {
    union {
        float32_t f;
        uint32_t i;
    } val;
    val.f = value;
    val.i = __bswap32(val.i);
    return val.f;
}

double64_t _doubleSwap(double64_t value) {
    union {
        double64_t f;
        uint64_t i;
    } val;
    val.f = value;
    val.i = __bswap64(val.i);
    return val.f;
};
