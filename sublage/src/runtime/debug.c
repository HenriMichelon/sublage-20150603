#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "sublage/debug.h"
#include "sublage/loader.h"
#include "sublage/byteorder.h"
#include "sublage/strbuffer.h"
#include "sublage/dump.h"

#ifndef WIN32
static struct timespec ts_pause = {0, 10000};
#endif

typedef void (*debugAction) (VmContext *vc);

void debugPushInPausedStack(VmContext *vc) {
    vc->isPaused = true;
    linkedListRemove(vc->debugContext->pausedContexts, vc);
    stackPush(vc->debugContext->pausedContexts, vc);
}

void debugRemoveFromPausedStack(VmContext *vc) {
    vc->isPaused = false;
    vc->exitFromPause = true;
    linkedListRemove(vc->debugContext->pausedContexts, vc);
}

void debugSendStack(VmContext *vc) {
    LinkedListIterator *lit = linkedListCreateIterator(vmContextGetStack(vc));
    uint32_t i = (uint32_t) linkedListSize(vmContextGetStack(vc));
    StackObject *so = NULL;
    char buf[120];
    while ((so = linkedListIteratorNext(lit)) != NULL) {
        dumpFormatObject(buf, 120, so, vc, true);
        debugSendMessage(vc->debugContext, "%d,%d:%s", vc->contextID, i--, buf);
    }
    linkedListIteratorDestroy(lit);
    debugSendAck(vc->debugContext);
}

void debugStop(VmContext *vc) {
    vc->isRunning = false;
    debugRemoveFromPausedStack(vc);
}

void debugStepOver(VmContext *vc) {
    if (vc->isPaused) {
        vc->stepOver = true;
    }
}

void debugStepInto(VmContext *vc) {
    if (vc->isPaused) {
        vc->stepInto = true;
    }
}

void debugStepOut(VmContext *vc) {
    if (vc->isPaused) {
        vc->stepOut = true;
    }
}

void debugSendVarList(VmContext *vc) {
    BinExecImg *img = vmContextGetCurrentImage(vc);
    BinExecFile *bef = img->bef;
    char buf[120];
    for (uint32_t i = 0; i < bef->header.numberOfVariables; i++) {
        const char* varname = debugFindVariableName(bef, i);
        if (img->variables[i] == NULL) {
            debugSendMessage(vc->debugContext, "%s:%d,empty", varname,
                             OPCODE_NULL);
        } else {
            dumpFormatObject(buf, 120, img->variables[i], vc, true);
            debugSendMessage(vc->debugContext, "%s:%d,%s", varname,
                             img->variables[i]->opcode, buf);
        }
    }
}

void debugSendVarGet(VmContext *vc, char *varname) {
    BinExecImg *img = vmContextGetCurrentImage(vc);
    BinExecFile *bef = img->bef;
    int32_t index = debugFindVariableIndex(bef, varname);
    if (index == -1) {
        debugSendMessage(vc->debugContext, "%s:unknown", varname);
    } else if (index == -2) {
        debugSendMessage(vc->debugContext, "%s:(no debug symbols)", varname);
    } else {
        char buf[120];
        if (img->variables[index] == NULL) {
            debugSendMessage(vc->debugContext, "%s:empty", varname, buf);
        } else {
            dumpFormatObject(buf, 120, img->variables[index], vc, true);
            debugSendMessage(vc->debugContext, "%s:%s", varname, buf);
        }
    }
}

void debugCommandPop(DebugContext *ctx, debugAction action) {
    ThreadMutexLock(&ctx->mutex);
    VmContext *vc = linkedListGetLast(ctx->pausedContexts);
    if (vc != NULL) {
        action(stackPop(ctx->pausedContexts));
    }
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandLast(DebugContext *ctx, debugAction action) {
    ThreadMutexLock(&ctx->mutex);
    VmContext *vc = linkedListGetLast(ctx->pausedContexts);
    if (vc != NULL) {
        action(vc);
    }
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
    
}

void debugCommandThread(DebugContext *ctx, uint64_t id, debugAction cmd) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->contexts);
    VmContext *vc;
    while ((vc = linkedListIteratorNext(it)) != NULL) {
        if (id == vmContextGetID(vc)) {
            cmd(vc);
            break;
        }
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandPause(DebugContext *ctx) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->contexts);
    VmContext *vc;
    while ((vc = linkedListIteratorNext(it)) != NULL) {
        debugPushInPausedStack(vc);
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandStop(DebugContext *ctx) {
    ThreadMutexLock(&ctx->mutex);
    ctx->debugging = false;
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandThreadVarGet(DebugContext *ctx, uint64_t id, char* varname) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->contexts);
    VmContext *vc;
    while ((vc = linkedListIteratorNext(it)) != NULL) {
        if (id == vmContextGetID(vc)) {
            debugSendVarGet(vc, varname);
            break;
        }
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandThreadList(DebugContext *ctx) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->contexts);
    VmContext *vc;
    while ((vc = linkedListIteratorNext(it)) != NULL) {
        BinExecImg *img = vmContextGetCurrentImage(vc);
        DebugSymbol *symbol = debugFindSymbol(img->bef, img->lastCodeIndex);
        if (symbol != NULL) {
            char *text = debugGetText(img->bef, symbol->startingChar, symbol->endingChar);
            if (text == NULL) {
                text = strbufferClone(binexecGetSourceFileName(img->bef));
                text = strbufferAppendStr(text, " source file not found" , -1);
            }
            debugSendMessage(ctx, "%d:%s,0x%04llx,%s,%lld,%s",
                             vmContextGetID(vc),
                             (vc->isPaused ? "paused" : "running"),
                             img->lastCodeIndex,
                             binexecGetSourceFileName(img->bef),
                             symbol->lineNumber,
                             text);
            strbufferDestroy(text);
        } else {
            debugSendMessage(ctx, "%d:%s,0x%04llx,%s,%lld",
                             vmContextGetID(vc),
                             (vc->isPaused ? "paused" : "running"),
                             img->lastCodeIndex,
                             binexecGetSourceFileName(img->bef),
                             debugFindLineNumber(img->bef, img->lastCodeIndex));
        }
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandThreadPaused(DebugContext *ctx) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->pausedContexts);
    VmContext *vc;
    while ((vc = linkedListIteratorNext(it)) != NULL) {
        if (vc->isPaused) {
            BinExecImg *img = vmContextGetCurrentImage(vc);
            DebugSymbol *symbol = debugFindSymbol(img->bef, img->lastCodeIndex);
            if (symbol != NULL) {
                char *text = debugGetText(img->bef, symbol->startingChar, symbol->endingChar);
                if (text == NULL) {
                    text = strbufferClone(binexecGetSourceFileName(img->bef));
                    text = strbufferAppendStr(text, " source file not found" , -1);
                }
                debugSendMessage(ctx, "%d:%s,0x%04llx,%s,%lld,%s",
                                 vmContextGetID(vc),
                                 "paused",
                                 img->lastCodeIndex,
                                 binexecGetSourceFileName(img->bef),
                                 symbol->lineNumber,
                                 text);
                strbufferDestroy(text);
            } else {
                debugSendMessage(ctx, "%d:%s,0x%04llx,%s,%lld",
                                 vmContextGetID(vc),
                                 "paused",
                                 img->lastCodeIndex,
                                 binexecGetSourceFileName(img->bef),
                                 debugFindLineNumber(img->bef, img->lastCodeIndex));
            }
        }
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandBreakSet(DebugContext *ctx, const char* source, uint64_t line) {
    ThreadMutexLock(&ctx->mutex);
    DebugBreakpoint *bp = memAlloc(sizeof (DebugBreakpoint));
    bp->source = strbufferClone(source);
    bp->line = line;
    linkedListAppend(ctx->breakpoints, bp);
    /*LinkedListIterator *it = linkedListCreateIterator(ctx->breakpoints);
     uint64_t index = 0;
     while ((bp = linkedListIteratorNext(it)) != NULL) {
     printf("%lld:%s,%lld\n", index++, bp->source, bp->line);
     fflush(stdout);
     }
     linkedListIteratorDestroy(it);*/
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
    
}

void debugCommandBreakList(DebugContext *ctx) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->breakpoints);
    DebugBreakpoint *bp;
    uint64_t index = 0;
    while ((bp = linkedListIteratorNext(it)) != NULL) {
        debugSendMessage(ctx, "%lld:%s,%lld", index++, bp->source, bp->line);
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandBreakUnset(DebugContext *ctx, const char* source, uint64_t line) {
    ThreadMutexLock(&ctx->mutex);
    LinkedListIterator *it = linkedListCreateIterator(ctx->breakpoints);
    DebugBreakpoint *bp;
    while ((bp = linkedListIteratorNext(it)) != NULL) {
        if (strbufferEquals(source, bp->source) &&
            line == bp->line) {
            linkedListRemove(ctx->breakpoints, bp);
        }
    }
    linkedListIteratorDestroy(it);
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

void debugCommandVarGet(DebugContext *ctx, char *varname) {
    ThreadMutexLock(&ctx->mutex);
    VmContext *vc = linkedListGetLast(ctx->contexts);
    if (vc != NULL) {
        debugSendVarGet(vc, varname);
    }
    debugSendAck(ctx);
    ThreadMutexUnlock(&ctx->mutex);
}

#define SCANF_BUFSIZE 200
#define SCANF_PARAM_STR "%200s"
#define SCANF_PARAM_UINT "%llu"
#define SCANF_PARAM_STR_UINT "%200s %llu"
#define SCANF_PARAM_UINT_STR "%llu %200s"

void debugProcessCommand(DebugContext *ctx) {
    char cmd[SCANF_BUFSIZE];
    if (fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd) != 1) {
        ctx->debugging = false;
        return;
    }
    if (strbufferEquals(cmd, "run")) {
        ctx->running = true;
        debugSendAck(ctx);
    } else if (strbufferEquals(cmd, "continue")) {
        debugCommandPop(ctx, debugRemoveFromPausedStack);
    } else if (strbufferEquals(cmd, "pause")) {
        debugCommandPause(ctx);
    } else if (strbufferEquals(cmd, "stop")) {
        debugCommandStop(ctx);
    } else if (strbufferEquals(cmd, "thread")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "list")) {
            debugCommandThreadList(ctx);
            return;
        } else if (strbufferEquals(cmd, "paused")) {
            debugCommandThreadPaused(ctx);
            return;
        } else if (strbufferEquals(cmd, "continue")) {
            uint64_t id;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT, &id) == 1) {
                debugCommandThread(ctx, id, debugRemoveFromPausedStack);
                return;
            }
            debugSendInfoMessage(ctx, "usage: thread run thread_id");
            return;
        } else if (strbufferEquals(cmd, "pause")) {
            uint64_t id;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT, &id) == 1) {
                debugCommandThread(ctx, id, debugPushInPausedStack);
                return;
            }
            debugSendInfoMessage(ctx, "usage: thread pause thread_id");
            return;
        } else if (strbufferEquals(cmd, "stop")) {
            uint64_t id;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT, &id) == 1) {
                debugCommandThread(ctx, id, debugStop);
                return;
            }
            debugSendInfoMessage(ctx, "usage: thread stop thread_id");
            return;
        } else if (strbufferEquals(cmd, "step")) {
            uint64_t id;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_STR_UINT, cmd, &id) == 2) {
                if (strbufferEquals(cmd, "over")) {
                    debugCommandThread(ctx, id, debugStepOver);
                    return;
                } else if (strbufferEquals(cmd, "into")) {
                    debugCommandThread(ctx, id, debugStepInto);
                    return;
                } else if (strbufferEquals(cmd, "out")) {
                    debugCommandThread(ctx, id, debugStepOut);
                    return;
                }
            }
            debugSendInfoMessage(ctx, "usage: thread step over|into|out thread_id");
            return;
        } else if (strbufferEquals(cmd, "stack")) {
            fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
            if (strbufferEquals(cmd, "dump")) {
                uint64_t id;
                if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT, &id) == 1) {
                    debugCommandThread(ctx, id, debugSendStack);
                    return;
                }
                debugSendInfoMessage(ctx, "usage: thread stack dump thread_id");
                return;
            }
            debugSendInfoMessage(ctx, "usage: thread stack dump");
            return;
        } else if (strbufferEquals(cmd, "var")) {
            fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
            if (strbufferEquals(cmd, "list")) {
                uint64_t id;
                if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT, &id) == 1) {
                    debugCommandThread(ctx, id, debugSendVarList);
                } else {
                    debugSendInfoMessage(ctx, "usage: thread var list thread_id");
                }
                return;
            } else if (strbufferEquals(cmd, "get")) {
                uint64_t id;
                if (fscanf(ctx->debugFileIn, SCANF_PARAM_UINT_STR, &id, cmd) == 2) {
                    debugCommandThreadVarGet(ctx, id, cmd);
                } else {
                    debugSendInfoMessage(ctx, "usage: thread var get thread_id var_name");
                }
                return;
            }
            debugSendInfoMessage(ctx, "usage: thread var list|get");
            return;
        }
        debugSendInfoMessage(ctx, "usage: thread list|continue|pause|stop|step|stack|paused");
    } else if (strbufferEquals(cmd, "break")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "set")) {
            uint64_t line;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_STR_UINT, cmd, &line) == 2) {
                debugCommandBreakSet(ctx, cmd, line);
            } else {
                debugSendInfoMessage(ctx, "usage: break set source_file_name line_number");
            }
            return;
        } else if (strbufferEquals(cmd, "unset")) {
            uint64_t line;
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_STR_UINT, cmd, &line) == 2) {
                debugCommandBreakUnset(ctx, cmd, line);
            } else {
                debugSendInfoMessage(ctx, "usage: break unset breakpoint_index");
            }
            return;
        } else if (strbufferEquals(cmd, "list")) {
            debugCommandBreakList(ctx);
            return;
        }
        debugSendInfoMessage(ctx, "usage: break set|unset|list");
    } else if (strbufferEquals(cmd, "step")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "over")) {
            debugCommandPop(ctx, debugStepOver);
        } else if (strbufferEquals(cmd, "into")) {
            debugCommandPop(ctx, debugStepInto);
        } else if (strbufferEquals(cmd, "out")) {
            debugCommandPop(ctx, debugStepOut);
        } else {
            debugSendInfoMessage(ctx, "usage: step over|into|out");
        }
    } else if (strbufferEquals(cmd, "stack")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "dump")) {
            debugCommandLast(ctx, debugSendStack);
        } else {
            debugSendInfoMessage(ctx, "usage: stack dump");
        }
    } else if (strbufferEquals(cmd, "var")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "list")) {
            debugCommandLast(ctx, debugSendVarList);
            return;
        } else if (strbufferEquals(cmd, "get")) {
            if (fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd) == 1) {
                debugCommandVarGet(ctx, cmd);
                return;
            }
            debugSendInfoMessage(ctx, "usage: var get variable_name");
        } else {
            debugSendInfoMessage(ctx, "usage: var list|get");
        }
    } else if (strbufferEquals(cmd, "verbose")) {
        fscanf(ctx->debugFileIn, SCANF_PARAM_STR, cmd);
        if (strbufferEquals(cmd, "on")) {
            ctx->verbose = true;
        } else if (strbufferEquals(cmd, "off")) {
            ctx->verbose = false;
        } else {
            debugSendInfoMessage(ctx, "usage: verbose on|off");
        }
    } else {
        debugSendInfoMessage(ctx,
                             "debug commands: run,continue,pause,stop,thread,break,step,stack,var,verbose");
    }
}

ThreadRoutineReturnType __debugthread(void* param) {
    DebugContext *ctx = (DebugContext*) param;
#ifndef WIN32
    struct timespec ts = {0, 200000000L};
#endif
    if (ctx->debugFileIn == stdin) {
#ifndef WIN32
        nanosleep(&ts, NULL);
#else
        Sleep(200);
#endif
    }
    while (ctx->debugging) {
        if (ctx->debugFileIn == stdin) {
#ifndef WIN32
            nanosleep(&ts, NULL);
#else
            Sleep(200);
#endif
            ThreadMutexLock(&ctx->mutex);
            fprintf(ctx->debugFileOut, "debug>");
            ThreadMutexUnlock(&ctx->mutex);
        }
        debugProcessCommand(ctx);
    }
    #
    return ThreadRoutineReturn;
}

bool debugIsAtLineNumber(BinExecFile *bef, uint64_t offset, uint64_t line) {
    if (bef->sourceFileName == NULL) {
        return false;
    }
    for (uint64_t i = 0; i < bef->debugSymbolsCount; i++) {
        if (bef->debugSymbols[i].lineNumber == line) {
            return bef->debugSymbols[i].codeOffset == offset;
        }
    }
    return false;
}

bool debugIsAtBreakpoint(VmContext *vc, uint64_t codeIndex) {
    DebugContext *ctx = vc->debugContext;
    LinkedListIterator *it = linkedListCreateIterator(ctx->breakpoints);
    BinExecImg *img = vmContextGetCurrentImage(vc);
    BinExecFile *bef = img->bef;
    const char *source = binexecGetSourceFileName(bef);
    DebugBreakpoint *bp;
    while ((bp = linkedListIteratorNext(it)) != NULL) {
        /*printf("%s %lld / %s %lld\n",
               source,
               debugFindLineNumber(bef, codeIndex),
               bp->source, bp->line);
        fflush(stdout);*/
        if (strbufferEquals(source, bp->source) &&
            debugIsAtLineNumber(bef, codeIndex, bp->line)) {
            linkedListIteratorDestroy(it);
            return true;
        }
    }
    linkedListIteratorDestroy(it);
    return false;
}

bool debugDebug(VmContext *vc, uint64_t codeIndex) {
    BinExecImg *img = vmContextGetCurrentImage(vc);
    BinExecFile *bef = img->bef;
    DebugContext *ctx = vc->debugContext;
    char threadid[100];
    char sourcemsg[200];
    if (ctx->verbose) {
        snprintf(threadid, 100, "thread #%d", vmContextGetID(vc));
    }
    ThreadMutexUnlock(&ctx->mutex);
    vc->stepInto = vc->stepOver = vc->stepOut = false;
    DebugSymbol *symbol = debugFindSymbol(bef, codeIndex);
    if (ctx->verbose) {
        if (symbol == NULL) {
            snprintf(sourcemsg, 200, "at 0x%04llx in %s as line %lld : (no debug symbol)",
                     codeIndex,
                     binexecGetSourceFileName(bef),
                     debugFindLineNumber(bef, codeIndex));
        } else {
            char *text = debugGetText(bef, symbol->startingChar, symbol->endingChar);
            snprintf(sourcemsg, 200, "at 0x%04llx in %s line %lld : %s",
                     codeIndex,
                     binexecGetSourceFileName(bef),
                     symbol->lineNumber,
                     (text == NULL ? "(no debug symbols or source file not found)"
                      : text));
            if (text != NULL) {
                strbufferDestroy(text);
            }
        }
    }
    if (!vc->isPaused) {
        vc->isPaused = debugIsAtBreakpoint(vc, codeIndex);
    }
    if (ctx->debugging && vc->isPaused && (symbol != NULL)) {
        debugPushInPausedStack(vc);
        debugSendInfoMessage(ctx, "%s paused %s", threadid, sourcemsg);
        while (vc->isPaused && ctx->debugging) {
            ThreadMutexUnlock(&ctx->mutex);
#ifndef WIN32
        nanosleep(&ts_pause, NULL);
#else
        Sleep(10);
#endif 
            ThreadMutexLock(&ctx->mutex);
            if (vc->stepInto || vc->stepOut || vc->stepOver) {
                break;
            }
        }
    }
    if (ctx->debugging) {
        if (vc->stepOut) {
            vc->inStepOver = 1;
            vc->isPaused = false;
        }
    } else {
        if (vmContextGetID(vc) == 1) {
            while (linkedListSize(ctx->contexts) > 1) {
                ThreadMutexUnlock(&ctx->mutex);
#ifndef WIN32
        nanosleep(&ts_pause, NULL);
#else
        Sleep(10);
#endif 
                ThreadMutexLock(&ctx->mutex);
            }
        }
        debugSendInfoMessage(ctx, "%s stopped %s", threadid, sourcemsg);
    }
    ThreadMutexUnlock(&ctx->mutex);
    return ctx->debugging;
}

DebugContext* debugCreateContext(FILE* debugFileIn, FILE* debugFileOut) {
    DebugContext *ctx = memAlloc(sizeof (DebugContext));
    ctx->debugging = true;
    ctx->running = false;
    ctx->verbose = debugFileIn == stdin;
    ctx->debugFileIn = debugFileIn;
    ctx->debugFileOut = debugFileOut;
    ctx->breakpoints = linkedListCreate();
    ctx->contexts = linkedListCreate();
    ctx->pausedContexts = stackCreate();
#ifndef WIN32
    setlinebuf(debugFileIn);
#endif
    debugSendMessage(ctx, "sublage debugger running");
    ThreadMutexCreate(&ctx->mutex);
    ThreadCreate(&ctx->thread, __debugthread, (void*) ctx);
    return ctx;
}

void debugDestroyContext(DebugContext * ctx) {
    ctx->debugging = false;
    ThreadMutexDestroy(&ctx->mutex);
    //fclose(ctx->debugFileIn);
    //ThreadJoin(ctx->thread);
    LinkedListIterator *it = linkedListCreateIterator(ctx->breakpoints);
    DebugBreakpoint *bp;
    while ((bp = linkedListIteratorNext(it)) != NULL) {
        strbufferDestroy(bp->source);
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(ctx->breakpoints, true);
    linkedListDestroy(ctx->contexts, false);
    stackDestroy(ctx->pausedContexts, false);
    memFree(ctx);
}

void debugSendAck(DebugContext *ctx) {
    if (ctx->debugFileIn != stdin) {
        debugSendMessage(ctx, "");
    }
}

void debugSendInfoMessage(DebugContext *ctx, const char* fmt, ...) {
    if (ctx->verbose) {
        char msg[300];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(msg, 300, fmt, ap);
        va_end(ap);
        fprintf(ctx->debugFileOut, "%s\n", msg);
        fflush(ctx->debugFileOut);
    }
}

void debugSendMessage(DebugContext *ctx, const char* fmt, ...) {
    char msg[300];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, 300, fmt, ap);
    va_end(ap);
    fprintf(ctx->debugFileOut, "%s\n", msg);
    fflush(ctx->debugFileOut);
    /*fprintf(stdout, "msg:-%s-\n", msg);
    fflush(stdout);*/
}

void debugAddContext(DebugContext *ctx, VmContext * vc) {
    ThreadMutexLock(&ctx->mutex);
    linkedListAppend(ctx->contexts, vc);
    /*if (vmContextGetID(vc) == 1) {
        debugPushInPausedStack(vc);
        //vc->isPaused = true;
    }*/
    ThreadMutexUnlock(&ctx->mutex);
}

void debugRemoveContext(DebugContext *ctx, VmContext * vc) {
    ThreadMutexLock(&ctx->mutex);
    linkedListRemove(ctx->contexts, vc);
    ThreadMutexUnlock(&ctx->mutex);
}

const char* debugFindVariableName(BinExecFile *bef, uint32_t var_index) {
    if (bef->sourceFileName == NULL) {
        return "(no debug symbols)";
    }
    return linkedListGet(bef->variablesName, var_index);
}

char* debugGetText(BinExecFile *bef, uint64_t start, uint64_t end) {
    if (bef->sourceFileName == NULL) {
        return NULL;
    }
    if (bef->sourceText == NULL) {
        return NULL;
    }
    return strbufferSubStr(bef->sourceText, start, end - start);
}

int32_t debugFindVariableIndex(BinExecFile *bef, char *varname) {
    if (bef->sourceFileName == NULL) {
        return -2;
    }
    LinkedListIterator *it = linkedListCreateIterator(bef->variablesName);
    int32_t index = 0;
    char* name;
    while ((name = linkedListIteratorNext(it)) != NULL) {
        if (strbufferEquals(name, varname)) {
            linkedListIteratorDestroy(it);
            return index;
        }
        index++;
    }
    linkedListIteratorDestroy(it);
    return -1;
}

DebugSymbol* debugFindSymbol(BinExecFile *bef, uint64_t offset) {
    if (bef->sourceFileName == NULL) {
        return NULL;
    }
    for (uint64_t i = 0; i < bef->debugSymbolsCount; i++) {
        if (bef->debugSymbols[i].codeOffset == offset) {
            return &bef->debugSymbols[i];
        }
    }
    return NULL;
}

uint64_t debugFindLineNumber(BinExecFile *bef, uint64_t offset) {
    if (bef->sourceFileName == NULL) {
        return 0;
    }
    for (uint64_t i = 0; i < bef->debugSymbolsCount; i++) {
        if (bef->debugSymbols[i].codeOffset >= offset) {
            return bef->debugSymbols[i].lineNumber;
        }
    }
    return 0;
}

void debugLoadFile(BinExecFile * bef) {
    if (bef->sourceFileName != NULL) {
        return;
    }
    char* debugFileName = strbufferClone(bef->fileName);
    debugFileName = strbufferAppendStr(debugFileName, ".debug", -1);
    FILE *debug = fopen(debugFileName, "r");
    strbufferDestroy(debugFileName);
    if (debug == NULL) {
        return;
    }
    uint16_t magic;
    if (fread(&magic, sizeof (magic), 1, debug) != 1) {
        fclose(debug);
        return;
    }
    if (magic != vmtohs(0xED0C)) {
        fclose(debug);
        return;
    }
    bef->sourceFileName = loaderLoadString(debug);
    if (bef->sourceFileName == NULL) {
        fclose(debug);
        return;
    }
    bef->variablesName = linkedListCreate();
    for (uint32_t i = 0; i < bef->header.numberOfVariables; i++) {
        char *name = loaderLoadString(debug);
        if (name == NULL) {
            fclose(debug);
            return;
        }
        linkedListAppend(bef->variablesName, name);
    }
    if (fread(&bef->debugSymbolsCount, sizeof(bef->debugSymbolsCount), 1, debug) != 1) {
        fclose(debug);
        return;
    }
    bef->debugSymbolsCount = vmtohll(bef->debugSymbolsCount);
    bef->debugSymbols = memAlloc(sizeof(DebugSymbol) * bef->debugSymbolsCount);
    bef->debugSymbolsCount = fread(bef->debugSymbols, sizeof(DebugSymbol),
                                   bef->debugSymbolsCount, debug);
    for (uint64_t i = 0; i < bef->debugSymbolsCount; i++) {
        DebugSymbol *ds = &bef->debugSymbols[i];
        ds->codeOffset = vmtohll(ds->codeOffset);
        ds->lineNumber = vmtohll(ds->lineNumber);
        ds->symbol = vmtohl(ds->symbol);
        ds->startingChar = vmtohll(ds->startingChar);
        ds->endingChar = vmtohll(ds->endingChar);
    }
    int source = open(bef->sourceFileName, O_RDONLY);
    if (source != -1) {
        struct stat stats;
        if (fstat(source, &stats) != -1) {
            bef->sourceText = memAlloc(stats.st_size);
            read(source, bef->sourceText, stats.st_size);
            close(source);
        }
    }
}
