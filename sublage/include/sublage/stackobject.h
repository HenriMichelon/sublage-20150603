/*
    Stack objects (in binary files and memory).
*/
#pragma once

#include <math.h>
#include "sublage/types.h"
#include "sublage/opcode.h"

typedef struct {
    uint64_t nbytes;
    void*    data;
} StackObjectData;

typedef struct {
    uint32_t classIndex;
    uint32_t fieldIndex;
} __attribute__((packed)) StackObjectClassField;

typedef struct {
    uint8_t opcode;

    union {
        uint8_t internal;
        uint8_t booleanValue;
        int64_t intValue;
        double64_t floatValue;
        uint64_t functionOffset;
        uint32_t importIndex;
        int64_t jumpOffset;
        uint32_t stringIndex;
        uint32_t arrayIndex;
        uint32_t nativeIndex;
        uint32_t variableIndex;
        uint32_t classIndex;
        void* privateData;
    } __attribute__((packed)) data;
} __attribute__((packed)) StackObject;



