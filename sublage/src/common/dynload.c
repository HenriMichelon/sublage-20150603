#include "sublage/dynload.h"
#include <stdio.h>

#ifdef WIN32

static char dynloaderrbuf[20];

const char* DynloadLastError() {
    snprintf(dynloaderrbuf, 20, "%d", GetLastError()); 
    return dynloaderrbuf;
}

#endif