#pragma once

#include "sublage/types.h"

char *strbufferCreate(void);
void strbufferDestroy(char*);
char *strbufferAppendChar(char*, char);
char *strbufferAppendStr(char*, const char*, uint64_t);
char *strbufferClone(const char*);
char *strbufferSubStr(const char*, int64_t, uint64_t);
char *strbufferLeft(const char*, int64_t);
bool strbufferEquals(const char*, const char*);
int64_t strbufferStrPos(const char*, const char*);
int64_t strbufferStrLastPos(const char*, const char*);
