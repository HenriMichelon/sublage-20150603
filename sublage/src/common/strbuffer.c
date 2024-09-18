#include "sublage/mem.h"
#include <string.h>
#include "sublage/strbuffer.h"

//static int ccount = 0;

char *strbufferCreate(void) {
    char *s = (char*) memAlloc(sizeof (char));
    *s = 0;
    //printf("%d: %x\n", ++ccount, s);
    return s;
}

void strbufferDestroy(char*str) {
    memFree(str);
}

char *strbufferAppendChar(char*str, char c) {
    size_t l = strlen(str);
    char *s = (char*) memRealloc(str, l + 2);
    s[l] = c;
    s[l + 1] = 0;
    return s;
}

char *strbufferAppendStr(char*str, const char*src, uint64_t nbchars) {
    size_t l = strlen(src);
    nbchars = (l < nbchars ? l : nbchars);
    l = strlen(str);
    char* s = (char*) memRealloc(str, l + nbchars + 1);
    strncat(s + l, src, nbchars);
    return s;
}

int64_t strbufferStrPos(const char*src, const char*str) {
    char * pos = strstr(src, str);
    if (pos == NULL) {
        return -1;
    }
    return pos - src;
}

int indexOf (const char* base, const char* str, int startIndex) {
        int result;
        int baselen = strlen(base);
        // str should not longer than base
        if (strlen(str) > baselen || startIndex > baselen) {
                result = -1;
        } else {
                if (startIndex < 0 ) {
                        startIndex = 0;
                }
                char* pos = strstr(base+startIndex, str);
                if (pos == NULL) {
                        result = -1;
                } else {
                        result = pos - base;
                }
        }
        return result;
}

int64_t strbufferStrLastPos(const char*src, const char*str) {
    int64_t result;
    if (strlen(str) > strlen(src)) {
        result = -1;
    } else {
        int start = 0;
        int endinit = strlen(src) - strlen(str);
        int end = endinit;
        int endtmp = endinit;
        while (start != end) {
            start = indexOf(src, str, start);
            end = indexOf(src, str, end);

            if (start == -1) {
                end = -1;
            } else if (end == -1) {
                if (endtmp == (start + 1)) {
                    end = start;
                } else {
                    end = endtmp - (endtmp - start) / 2;
                    if (end <= start) {
                        end = start + 1;
                    }
                    endtmp = end;
                }
            } else {
                start = end;
                end = endinit;
            }
        }
        result = start;
    }
    return result;
}

char *strbufferLeft(const char*src, int64_t nbchars) {
    return strbufferSubStr(src, 0, nbchars);
}

char *strbufferSubStr(const char*src, int64_t start, uint64_t nbchars) {
    size_t len = strlen(src);
    if (start >= len) {
        return strbufferCreate();
    }
    len -= start;
    nbchars = (len < nbchars ? len : nbchars);
    char* s = (char*) memAlloc(nbchars + 1);
    strncpy(s, src + start, nbchars);
    s[nbchars] = 0;
    //printf("%x %s\n", s, s);
    return s;
}

char *strbufferClone(const char*src) {
    char *dst = (char*) memAlloc(strlen(src) + 1);
    strcpy(dst, src);
    //printf("%x %s\n", dst, dst);
    return dst;
}

bool strbufferEquals(const char*s1, const char*s2) {
    return strcmp(s1, s2) == 0;
}
