#include "utils.h"

 void native_flush(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    fflush(file);
    vmContextPush(vc, stackObjectClone(vc, sofile));
}

 void native_read_char(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    char* buffer = (char*) memAlloc(sizeof (char)*2);
    buffer[1] = 0;
    buffer[0] = fgetc(file);
    vmContextPush(vc, stackObjectClone(vc, sofile));
    vmContextPush(vc, stackObjectNewString(vc, buffer));
    memFree(buffer);
}

 void native_read_bytes(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    StackObject *nbytes = vmContextPopOpcode(vc, OPCODE_INT);
    if (nbytes == NULL) return;
    char *buf = memAlloc(nbytes->data.intValue);
    size_t read = fread(buf, 1, nbytes->data.intValue, file);
    vmContextPush(vc, stackObjectClone(vc, sofile));
    if (read == 0) {
        vmContextPush(vc, stackObjectNewNull(vc));        
    } else {
        vmContextPush(vc, stackObjectNewData(vc, buf, read));
    }
}

 void native_read_line(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    int size = 200;
    int length = 0;
    char* buffer = (char*) memAlloc(sizeof (char)*size);
    char c = 0;
    while ((c != '\n') && (c != EOF)) {
        if (length == size) {
            size *= 2;
            buffer = (char*) memRealloc(buffer, sizeof (char)*size);
        }
        c = fgetc(file);
        if ((c != '\n') && (c != EOF)) {
            buffer[length++] = c;
        }
    }
    vmContextPush(vc, stackObjectClone(vc, sofile));
    if ((length == 0) && (c == EOF))  {
        vmContextPush(vc, stackObjectNewNull(vc));        
    } else {
        buffer[length] = 0;
        vmContextPush(vc, stackObjectNewString(vc, buffer));
    }
    memFree(buffer);
}

 void native_new_line(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    fprintf(file, "\n");
    fflush(file);
    vmContextPush(vc, stackObjectClone(vc, sofile));
}

 void native_close(VmContext *vc) {
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    FILE *file = sofile->data.privateData;
    fclose(file);
}

 void native_open_file(VmContext *vc) {
    StackObject *somode = vmContextPopOpcode(vc, OPCODE_STRING);
    if (somode == NULL) return;
    StackObject *soname = vmContextPopOpcode(vc, OPCODE_STRING);
    if (soname == NULL) return;
    FILE *file = fopen(vmContextGetString(vc, soname),
            vmContextGetString(vc, somode));
    if (file == NULL) {
        vmContextPush(vc, stackObjectNewNull(vc));
    } else {
        vmContextPush(vc, stackObjectNewPrivate(vc, file));
    }
}

 void native_print(VmContext *vc) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    StackObject *sofile = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (sofile == NULL) return;
    util_printStackObject(vc, sofile->data.privateData, so);
    vmContextPush(vc, stackObjectClone(vc, sofile));
}