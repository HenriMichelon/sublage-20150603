/*
    Binary file (.binary, .library)
*/
#pragma once

#include "sublage/types.h"
#include "sublage/dynload.h"
#include "sublage/linkedlist.h"
#include "sublage/stackobject.h"
#include "sublage/lexical.h"

#define BINEXEC_MAGIC 0xC0DE 

/*
    .binary file header.
*/
typedef struct {
    uint16_t  magic; // Magic number for file format detection and verification.
    uint8_t   majorVersion; // Binary file format version.
    uint8_t   minorVersion; // Binary file format version.
    uint32_t  numberOfVariables;
    uint64_t  tablesOffset; // Offset to data tables (in bytes, from start of file).
} __attribute__((packed)) BinExecHeader;

/* Debug symbols for .debug files */
typedef struct {
    uint64_t      codeOffset;
    LexicalSymbol symbol;
    uint64_t      lineNumber;
    uint64_t      startingChar;
    uint64_t      endingChar;
} __attribute__((packed)) DebugSymbol;

/*
    In-memory .binary file description.
*/
typedef struct {
    BinExecHeader header;
    uint8_t *codeStack; // Executable code. Array of StackObject.
    uint64_t codeSize; // ‘codeStack‘ number of bytes.
    LinkedList *functions;              // FunctionPointer*
    LinkedList *imports;                // char*
    LinkedList *importedFunctions;      // FunctionPointer*
    LinkedList *nativesFunctions;       // NativePointer*
    LinkedList *strings;
    LinkedList *arrays;
    LinkedList *classes;                // ClassPointer*
    DynloadLibrary dlHandle;           // OS native file handler.
    char       *fileName;           // Binary file name.
    char       *sourceFileName;     // only when .debug file loaded
    LinkedList *variablesName;      // only when .debug file loaded
    uint64_t   debugSymbolsCount;   // only when .debug file loaded
    DebugSymbol *debugSymbols;      // only when .debug file loaded
    char*      sourceText;          // only when .debug and .source file loaded
} BinExecFile;

typedef struct {
    char *name;
    uint64_t offset;
} FunctionPointer;

typedef struct {
    char *name;
    int32_t import;
} NativePointer;

typedef struct {
    char *name;
    uint64_t offset;
    bool private;
} InstanceFunctionPointer;

typedef struct {
    char        *name;
    bool        readAccess;
    char        *readMethod;
    bool        writeAccess;
    char        *writeMethod;
} InstanceVariablePointer;

typedef struct ClassPointer {
    char        *name;
    int32_t     import;     // -1 = binary internal class
    uint32_t    imageIndex;
    LinkedList  *variables; // InstanceVariablePointer*
    LinkedList  *functions; // FunctionPointer*
    struct ClassPointer *parent;
    int32_t     parentImport;
    char        *parentClassName;
} ClassPointer;

void binexecDestroy(BinExecFile *bef);
FunctionPointer* binexecFindFunction(BinExecFile *bef, char *name);
ClassPointer* binexecFindClass(BinExecFile *bef, char *name);
int32_t binexecFindNativeFunctionIndex(BinExecFile *bef, char *name);
LinkedList* binexecGetFunctions(BinExecFile *bef);
LinkedList* binexecGetClasses(BinExecFile *bef);
LinkedList* binexecGetImports(BinExecFile *bef);
LinkedList* binexecGetImportedFunctions(BinExecFile *bef);
LinkedList* binexecGetNativeFunctions(BinExecFile *bef);
char* binexecGetString(BinExecFile *bef, uint32_t stringIndex);
LinkedList* binexecGetArray(BinExecFile *bef, uint32_t arrayIndex);
uint8_t* binexecGetCodeStack(BinExecFile *bef);
uint64_t binexecGetCodeSize(BinExecFile *bef);
const char* binexecGetSourceFileName(BinExecFile *bef);
void* binexecGetNativeHandler(BinExecFile *bef);
