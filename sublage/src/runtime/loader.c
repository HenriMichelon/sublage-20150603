#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(__linux)
# include <linux/limits.h>
#elif defined(__FreeBSD__)
# include <limits.h>
#elif defined(__APPLE__)
# include <sys/syslimits.h>
#endif
#include <sys/stat.h>
#include <sublage/dynload.h>
#include "sublage/mem.h"
#include "sublage/byteorder.h"
#include "sublage/strbuffer.h"
#include "sublage/linkedlist.h"
#include "sublage/binexec.h"
#include "sublage/loader.h"

LoaderErrorCode loaderErrorCode = LOADER_NOERROR;

static const char* loaderErrorMessage[] = {
    "no error",
    "error opening file",
    "error reading file",
    "error reading header",
    "bad magic number",
    "unsupported version",
    "error reading code",
    "error reading imports table offset",
    "error reading imported functions table offset",
    "error reading native functions table offset",
    "error reading strings table offset",
    "error reading arrays table offset",
    "error reading classes table offset",
    "error reading internal functions table",
    "error reading native functions table",
    "error reading imports table",
    "error reading imported functions table",
    "error reading strings table",
    "error reading arrays table",
    "error reading classes table",
    "unknown opcode",
    "error loading dynamic library for native functions"
};

LoaderErrorCode loaderGetErrorCode() {
    return loaderErrorCode;
}

const char* loaderGetErrorMessage() {
    return loaderErrorMessage[loaderErrorCode];
}

void loaderError(LoaderErrorCode error) {
    loaderErrorCode = error;
}

#if defined(_WIN32)
#define PATHSEPARATOR ';'
#define SEPARATOR '\\'
#else
#define PATHSEPARATOR ':'
#define SEPARATOR '/'
#endif

char* loaderFindInPath(const char *name, const char *variable) {
    char *path, *p;
    char buf[PATH_MAX];
    size_t lp;
    if ((name == NULL) || (name[0] == '\0')) {
        return NULL;
    }
    if (!(path = getenv(variable))) {
        return NULL;
    }
    size_t ln = strlen(name);
    do {
        /* Find the end of this path element. */
        for (p = path; *path != 0 && *path != PATHSEPARATOR; path++) {
            continue;
        }
        if (p == path) {
            p = ".";
            lp = 1;
        } else {
            lp = path - p;
        }
        if (lp + ln + 2 > sizeof (buf)) {
            continue;
        }
        memcpy(buf, p, lp);
        buf[lp] = SEPARATOR;
        memcpy(buf + lp + 1, name, ln);
        buf[lp + ln + 1] = '\0';
        //printf("findInPath %s : %s (0x%llx)\n", name, buf, in);
        FILE *in = fopen(buf, "r");
        if (in != NULL) {
            fclose(in);
            return strbufferClone(buf);
        }
    } while (*path++ == PATHSEPARATOR);
    return NULL;
}

char* loaderLoadString(FILE *in) {
    char *name = strbufferCreate();
    char c = 0;
    do {
        if (fread(&c, sizeof (char), 1, in) != 1) {
            strbufferDestroy(name);
            return NULL;
        }
        name = strbufferAppendChar(name, c);
    } while (c != '\0');
    return name;
}

BinExecFile* loaderLoadFileFromFileName(char*inputFileName) {
    FILE *in = fopen(inputFileName, "r");
    if (in == NULL) {
        char* libraryFileName = loaderFindInPath(inputFileName, "PATH");
        if (libraryFileName == NULL) {
            libraryFileName = strbufferClone(inputFileName);
            libraryFileName = strbufferAppendStr(libraryFileName, ".library", -1);
            in = fopen(libraryFileName, "r");
            if (in == NULL) {
                char *libraryFileNameFound = loaderFindInPath(libraryFileName, "LD_LIBRARY_PATH");
                strbufferDestroy(libraryFileName);
                libraryFileName = libraryFileNameFound;
                if (libraryFileName != NULL) {
                    in = fopen(libraryFileName, "r");
                }
                if (in == NULL) {
                    strbufferDestroy(libraryFileName);
                    loaderError(LOADER_ERROR_OPEN);
                    return NULL;
                }
            }
        } else {
            in = fopen(libraryFileName, "r");
        }
        if (in == NULL) {
            return NULL;
        }
        inputFileName = libraryFileName;
    } else {
        inputFileName = strbufferClone(inputFileName);
    }
    BinExecFile *bef = loaderLoadFile(in, inputFileName);
    fclose(in);
    if (bef == NULL) {
        return NULL;
    }
    /*printf("`%s` loaded : %u bytes of code.\n",
            inputFileName,
            bef->codeSize);*/
    return bef;
}

int32_t loaderStackObjectSize(uint8_t opcode) {
    switch (opcode) {
        case OPCODE_NULL:
        case OPCODE_RETURN:
            return 0;
            break;
        case OPCODE_INTERNAL:
        case OPCODE_BOOLEAN:
            return sizeof (uint8_t);
            break;
        case OPCODE_EXTERNALCALL:
        case OPCODE_EXTERNALCALL_REF:
        case OPCODE_NATIVECALL:
        case OPCODE_NATIVECALL_REF:
        case OPCODE_STRING:
        case OPCODE_ARRAY:
        case OPCODE_VAR_GET:
        case OPCODE_VAR_REF:
        case OPCODE_VAR_SET:
        case OPCODE_CLASSREF:
        case OPCODE_IFIELD_SET:
        case OPCODE_IFIELDCALL:
        case OPCODE_IFIELDCALL_REF:
            return sizeof (int32_t);
            break;
        case OPCODE_INT:
        case OPCODE_FLOAT:
        case OPCODE_JUMP:
        case OPCODE_JUMPIFNOT:
        case OPCODE_JUMPIF:
        case OPCODE_INTERNALCALL:
        case OPCODE_INTERNALCALL_REF:
            return sizeof (int64_t);
            break;
    }
    return -1;
}

void loaderLoadStackObjectData(StackObject *so) {
    switch (so->opcode) {
        case OPCODE_NULL:
        case OPCODE_INTERNAL:
        case OPCODE_BOOLEAN:
        case OPCODE_RETURN:
            break;
        case OPCODE_INT:
            so->data.intValue = vmtohll(so->data.intValue);
            break;
        case OPCODE_FLOAT:
            so->data.floatValue = vmtohd(so->data.floatValue);
            break;
        case OPCODE_STRING:
            so->data.stringIndex = vmtohl(so->data.stringIndex);
            break;
        case OPCODE_ARRAY:
            so->data.arrayIndex = vmtohl(so->data.arrayIndex);
            break;
        case OPCODE_INTERNALCALL:
        case OPCODE_INTERNALCALL_REF:
            so->data.functionOffset = vmtohll(so->data.functionOffset);
            break;
        case OPCODE_EXTERNALCALL:
        case OPCODE_EXTERNALCALL_REF:
            so->data.importIndex = vmtohl(so->data.importIndex);
            break;
        case OPCODE_NATIVECALL:
        case OPCODE_NATIVECALL_REF:
            so->data.nativeIndex = vmtohl(so->data.nativeIndex);
            break;
        case OPCODE_JUMP:
        case OPCODE_JUMPIFNOT:
        case OPCODE_JUMPIF:
            so->data.jumpOffset = vmtohll(so->data.jumpOffset);
            break;
        case OPCODE_VAR_GET:
        case OPCODE_VAR_SET:
        case OPCODE_VAR_REF:
        case OPCODE_IFIELD_SET:
        case OPCODE_IFIELDCALL:
        case OPCODE_IFIELDCALL_REF:
            so->data.variableIndex = vmtohl(so->data.variableIndex);
            break;
        case OPCODE_CLASSREF:
            so->data.classIndex = vmtohl(so->data.classIndex);
            break;
        default:
            loaderError(LOADER_ERROR_UNKNOWNOPCODE);
            return;
    }
}

StackObject* loaderNextStackObject(BinExecFile *bef, uint64_t *index) {
    if (*index >= binexecGetCodeSize(bef)) {
        return NULL;
    }
    StackObject *so = (StackObject*) (bef->codeStack + *index);
    uint32_t size = loaderStackObjectSize(so->opcode);
    if (size == -1) {
        return NULL;
    }
    *index += size + sizeof (uint8_t);
    return so;
}

BinExecFile* loaderLoadFile(FILE *in, char* name) {
    BinExecFile *bef = memAlloc(sizeof (BinExecFile));
    memset(bef, 0, sizeof (BinExecFile));

    // read header
    if (fread(bef, sizeof (BinExecHeader), 1, in) != 1) {
        loaderError(LOADER_ERROR_READHEADER);
        binexecDestroy(bef);
        return NULL;
    }
    bef->header.magic = vmtohs(bef->header.magic);
    if (bef->header.magic != BINEXEC_MAGIC) {
        loaderError(LOADER_ERROR_BADMAGIC);
        binexecDestroy(bef);
        return NULL;
    }
    if ((bef->header.majorVersion != 0x00) ||
            (bef->header.minorVersion != 0x00)) {
        loaderError(LOADER_ERROR_UNSUPPORTEDVERSION);
        binexecDestroy(bef);
        return NULL;
    }
    bef->header.tablesOffset = vmtohll(bef->header.tablesOffset);
    bef->fileName = name;

    // read code
    bef->codeSize = bef->header.tablesOffset - sizeof (BinExecHeader);
    bef->codeStack = memAlloc(bef->codeSize);
    if (bef->codeSize > 0) {
        if (fread(bef->codeStack, bef->codeSize, 1, in) != 1) {
            loaderError(LOADER_ERROR_READCODE);
            binexecDestroy(bef);
            return NULL;
        }
    }
    
    uint64_t index = 0;
    StackObject *so = NULL;
    while ((so = loaderNextStackObject(bef, &index)) != NULL) {
        loaderLoadStackObjectData(so);
    }
    if (loaderErrorCode != LOADER_NOERROR) {
        binexecDestroy(bef);
        return NULL;
    }

    // read tables offsets
    uint64_t importsTableOffset;
    if (fread(&importsTableOffset, sizeof (importsTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READIMPORTSTABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    importsTableOffset = vmtohll(importsTableOffset);

    uint64_t importedFunctionsTableOffset;
    if (fread(&importedFunctionsTableOffset, sizeof (importedFunctionsTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READIMPORTEDFUNCTIONSTABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    importedFunctionsTableOffset = vmtohll(importedFunctionsTableOffset);

    uint64_t nativeFunctionsTableOffset;
    if (fread(&nativeFunctionsTableOffset, sizeof (nativeFunctionsTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READNATIVETABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    nativeFunctionsTableOffset = vmtohll(nativeFunctionsTableOffset);

    uint64_t arraysTableOffset;
    if (fread(&arraysTableOffset, sizeof (arraysTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READSTRINGSTABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    arraysTableOffset = vmtohll(arraysTableOffset);

    uint64_t classesTableOffset;
    if (fread(&classesTableOffset, sizeof (classesTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READCLASSESTABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    classesTableOffset = vmtohll(classesTableOffset);

    uint64_t stringsTableOffset;
    if (fread(&stringsTableOffset, sizeof (stringsTableOffset), 1, in) != 1) {
        loaderError(LOADER_ERROR_READSTRINGSTABLEOFFSET);
        binexecDestroy(bef);
        return NULL;
    }
    stringsTableOffset = vmtohll(stringsTableOffset);

    // read functions table
    bef->functions = linkedListCreate();
    while (ftell(in) < importsTableOffset) {
        FunctionPointer *f = memAlloc(sizeof (FunctionPointer));
        f->name = loaderLoadString(in);
        if (f->name == NULL || fread(&f->offset, sizeof (f->offset), 1, in) != 1) {
            if (f->name != NULL) {
                strbufferDestroy(f->name);
            }
            memFree(f);
            loaderError(LOADER_ERROR_READFUNCTIONSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        f->offset = vmtohll(f->offset);
        linkedListAppend(bef->functions, f);
    }

    // read imports
    bef->imports = linkedListCreate();
    while (ftell(in) < importedFunctionsTableOffset) {
        char* import = loaderLoadString(in);
        if (import == NULL) {
            loaderError(LOADER_ERROR_READIMPORTSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        linkedListAppend(bef->imports, import);
    }

    // read imported functions
    bef->importedFunctions = linkedListCreate();
    while (ftell(in) < nativeFunctionsTableOffset) {
        uint32_t index;
        FunctionPointer *f = memAlloc(sizeof (FunctionPointer));
        if (fread(&index, sizeof (index), 1, in) != 1) {
            memFree(f);
            if (!feof(in)) {
                loaderError(LOADER_ERROR_READIMPORTEDFUNCTIONSTABLE);
                binexecDestroy(bef);
                return NULL;
            }
            break;
        }
        f->offset = vmtohl(index);
        f->name = loaderLoadString(in);
        if (f->name == NULL) {
            memFree(f);
            loaderError(LOADER_ERROR_READIMPORTEDFUNCTIONSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        linkedListAppend(bef->importedFunctions, f);
    }

    // read native functions
    bool have_internal_native = false;
    bef->nativesFunctions = linkedListCreate();
    while (ftell(in) < arraysTableOffset) {
        NativePointer *f = memAlloc(sizeof (NativePointer));
        if (fread(&f->import, sizeof (f->import), 1, in) != 1) {
            memFree(f);
            if (!feof(in)) {
                loaderError(LOADER_ERROR_READNATIVETABLE);
                binexecDestroy(bef);
                return NULL;
            }
            break;
        }
        f->name = loaderLoadString(in);
        if (f->name == NULL) {
            memFree(f);
            loaderError(LOADER_ERROR_READIMPORTEDFUNCTIONSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        f->import = vmtohl(f->import);
        linkedListAppend(bef->nativesFunctions, f);
        if (f->import == -1) {
            have_internal_native = true;
        }
    }
    if (have_internal_native) {
        char* soname = strbufferClone(name);
        soname = strbufferAppendStr(soname, ".native", -1);
#ifdef WIN32
        char buffer[4096];
        char** lppPart = {NULL};
        if (GetFullPathName(soname, 4096, buffer, lppPart) != 0) {
            bef->dlHandle = DynloadLoad(buffer);
        } else {
            bef->dlHandle = DynloadLoad(soname);
        }
#else
        bef->dlHandle = DynloadLoad(soname);
#endif
        if (bef->dlHandle == NULL) {
            char *fullname = loaderFindInPath(soname, "LD_LIBRARY_PATH");
            if (fullname == NULL) {
                fullname = loaderFindInPath(soname, "PATH");
            }
            if (fullname != NULL) {
#ifdef WIN32
                if (GetFullPathName(fullname, 4096, buffer, lppPart) != 0) {
                    bef->dlHandle = DynloadLoad(buffer);
                } else {
                    bef->dlHandle = DynloadLoad(fullname);
                }
#else 
                bef->dlHandle = DynloadLoad(fullname);
#endif
                strbufferDestroy(fullname);
            }
        }
        strbufferDestroy(soname);
        if (bef->dlHandle == NULL) {
            loaderError(LOADER_ERROR_LOADDYNAMICLIBRARY);
            fprintf(stderr, "%s\n", DynloadLastError());
            binexecDestroy(bef);
            return NULL;
        }
    } else {
        bef->dlHandle = NULL;
    }

    // read arrays table
    bef->arrays = linkedListCreate();
    while (ftell(in) < classesTableOffset) {
        uint32_t size = 0;
        if (fread(&size, sizeof (size), 1, in) != 1) {
            if (feof(in)) {
                break;
            }
            loaderError(LOADER_ERROR_READARRAYSSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        size = vmtohl(size);
        LinkedList *array = linkedListCreate();
        for (uint32_t i = 0; i < size; i++) {
            StackObject *so = memAlloc(sizeof (StackObject));
            if (fread(&so->opcode, sizeof (so->opcode), 1, in) != 1) {
                loaderError(LOADER_ERROR_READARRAYSSTABLE);
                binexecDestroy(bef);
                return NULL;
            }
            uint32_t sosize = loaderStackObjectSize(so->opcode);
            if ((sosize > 0) && (fread(&so->data, sosize, 1, in) != 1)) {
                loaderError(LOADER_ERROR_READARRAYSSTABLE);
                binexecDestroy(bef);
                return NULL;
            }
            loaderLoadStackObjectData(so);
            linkedListAppend(array, so);
        }
        linkedListAppend(bef->arrays, array);
    }
    
    // read classes table
    bef->classes = linkedListCreate();
    while (ftell(in) < stringsTableOffset) {
        ClassPointer *c = memAlloc(sizeof(ClassPointer));
        c->name = loaderLoadString(in);
        if (c->name == NULL) {
            loaderError(LOADER_ERROR_READCLASSESSSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        fread(&c->import, sizeof(c->import), 1, in);
        c->import = vmtohl(c->import);
        c->parent = NULL;
        if (c->import == -1) {
            // read parent class desc
            c->parentImport = -1;
            fread(&c->parentImport, sizeof(c->parentImport), 1, in);
            c->parentImport = vmtohl(c->parentImport);
            c->parentClassName = loaderLoadString(in);
            // read instance variables
            c->variables = linkedListCreate();
            uint32_t n = 0;
            fread(&n, sizeof(n), 1, in);
            n = vmtohl(n);
            while (n > 0) {
                InstanceVariablePointer *ivar = memAlloc(sizeof(InstanceVariablePointer));
                ivar->name = loaderLoadString(in);
                uint8_t access;
                fread(&access, sizeof(access), 1, in);
                ivar->readAccess = access != 0;
                ivar->readMethod = loaderLoadString(in);
                fread(&access, sizeof(access), 1, in);
                ivar->writeAccess = access != 0;
                ivar->writeMethod = loaderLoadString(in);
                linkedListAppend(c->variables, ivar);
                n--;
            }
            
            // read instance methods
            c->functions = linkedListCreate();
            fread(&n, sizeof(n), 1, in);
            n = vmtohl(n);
            while (n > 0) {
                InstanceFunctionPointer *fp = memAlloc(sizeof(InstanceFunctionPointer));
                fp->name = loaderLoadString(in);
                uint8_t access;
                fread(&access, sizeof(access), 1, in);
                fp->private = access != 0;
                fread(&fp->offset, sizeof(fp->offset), 1, in);
                fp->offset = htovmll(fp->offset);
                linkedListAppend(c->functions, fp);
                n--;
            }
        }
        linkedListAppend(bef->classes, c);
    }

    // read strings table
    bef->strings = linkedListCreate();
    while (!feof(in)) {
        char* string = loaderLoadString(in);
        if (string == NULL) {
            if (feof(in)) {
                break;
            }
            loaderError(LOADER_ERROR_READSTRINGSSTABLE);
            binexecDestroy(bef);
            return NULL;
        }
        linkedListAppend(bef->strings, string);
    }
    return bef;
}




