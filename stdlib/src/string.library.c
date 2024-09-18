#include "sublage/vmcontext.h"
#include "sublage/strbuffer.h"
#include <string.h>

 void native_substring(VmContext *vc) {
    StackObject *end = vmContextPopOpcode(vc, OPCODE_INT);
    if (end == NULL) return;
    StackObject *begin = vmContextPopOpcode(vc, OPCODE_INT);
    if (begin == NULL) return;
    StackObject *str = vmContextPopOpcode(vc, OPCODE_STRING);
    if (str == NULL) return;
    if (begin->data.intValue < 0) {
        begin->data.intValue = 0;
    }
    if (end->data.intValue < begin->data.intValue) {
        vmContextPush(vc, stackObjectNewString(vc,
                                               strbufferSubStr(vmContextGetString(vc, str), begin->data.intValue,
                                                               -1)));
    } else {
        vmContextPush(vc, stackObjectNewString(vc,
                                               strbufferSubStr(vmContextGetString(vc, str), begin->data.intValue,
                                                               end->data.intValue - begin->data.intValue)));        
    }
}

 void native_starts_with(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* b = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* a = vmContextGetString(vc, so);
    vmContextPush(vc, stackObjectNewBool(vc, strbufferStrPos(a, b) == 0));
}

 void native_ends_with(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* b = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* a = vmContextGetString(vc, so);
    vmContextPush(vc, stackObjectNewBool(vc, strbufferStrPos(a, b) == 
            (strlen(a) - strlen(b))));
}

 void native_index_of(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* b = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* a = vmContextGetString(vc, so);
    vmContextPush(vc, stackObjectNewInt(vc, strbufferStrPos(a, b)));
}
 

 void native_last_index_of(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* b = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* a = vmContextGetString(vc, so);
    vmContextPush(vc, stackObjectNewInt(vc, strbufferStrLastPos(a, b)));
}
 
void native_contains(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* b = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* a = vmContextGetString(vc, so);
    vmContextPush(vc, stackObjectNewBool(vc, strbufferStrPos(a, b) != -1));
}

 void native_length(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    vmContextPush(vc, stackObjectNewInt(vc, strlen(vmContextGetString(vc, so))));
}
 
 void native_is_empty(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    vmContextPush(vc, stackObjectNewBool(vc, strlen(vmContextGetString(vc, so)) == 0));
}

void native_char_at(VmContext *vc) {
    StackObject *index = vmContextPopOpcode(vc, OPCODE_INT);
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char *str = vmContextGetString(vc, so);
    char *result = memAlloc(2);
    result[0] = str[index->data.intValue];
    result[1] = 0;
    vmContextPush(vc, stackObjectNewString(vc, result));
}

void native_replace(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* new = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* old = vmContextGetString(vc, so);
    so = vmContextPopOpcode(vc, OPCODE_STRING);
    if (so == NULL) return;
    char* src = vmContextGetString(vc, so);
    char *ch;
    if ((ch = strstr(src, old)) != NULL) {
        char *buffer = memAlloc(strlen(src) - strlen(new) + strlen(old) + 1);
        strncpy(buffer, src, ch-src);  
        buffer[ch-src] = 0;
        sprintf(buffer+(ch-src), "%s%s", new, ch+strlen(old));
        vmContextPush(vc, stackObjectNewString(vc, buffer));
    } else {
        vmContextPush(vc, stackObjectNewString(vc, src));
    }
}
