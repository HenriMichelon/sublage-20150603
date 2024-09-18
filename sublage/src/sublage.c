#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#ifdef __linux
# include <getopt.h>
# include <time.h>
#endif
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#define socklen_t int
#define socketerror GetLastError()
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#define socketerror errno
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>
#include "sublage/loader.h"
#include "sublage/strbuffer.h"
#include "sublage/dump.h"
#include "sublage/vmerrors.h"
#include "sublage/binexec.h"
#include "sublage/context.h"

void version(char *name) {
    printf("version: %s r%d\n", basename(name), SUBLAGE_REVISION);
}

void usage(char *name) {
    printf("usage: %s [-d stdin|socket|hostname_or_ip -p debug_tcp_port -s stackdump_file_name|stdout|socket -q stackdump_tcp_port -v] binary_file_name\n", basename(name));
}

VmContext *vc = NULL;
DebugContext *dc = NULL;
FILE *stackDumpFile = NULL;
int stackDumpServerSocket = -1;
int stackDumpSocket = -1;
int debugServerSocket = -1;
int debugSocket = -1;

#ifndef WIN32
struct timespec ts_rpause = {0, 100000};
#endif

void cleanupContext() {
    dumpEnd();
    if (vc != NULL) {
        vmContextDestroy(vc);
    }
    if (dc != NULL) {
        debugDestroyContext(dc);
    }
    if (debugSocket != -1) {
        closesocket(debugSocket);
    }
    if (debugServerSocket != -1) {
        closesocket(debugServerSocket);
    }
    if (stackDumpFile != NULL) {
        fclose(stackDumpFile);
    }
    if (stackDumpSocket != -1) {
        closesocket(stackDumpSocket);
    }
    if (stackDumpServerSocket != -1) {
        closesocket(stackDumpServerSocket);
    }
#ifdef WIN32
    WSACleanup();
#endif
}

void vmRunFile(VmContext *vc, char *name) {
    BinExecFile *bef = loaderLoadFileFromFileName(name);
    if (bef == NULL) {
        vmContextSetError(vc, VM_ERROR_LOADINGBINEXEC, name);
        return;
    }
    FunctionPointer *run = binexecFindFunction(bef, "run");
    if (run == NULL) {
        binexecDestroy(bef);
        vmContextSetError(vc, VM_ERROR_NORUNFUNCTION, name);
        return;
    }
    BinExecImg *img = binexecimgCreate(bef);
    uint32_t image_index = vmContextAddImage(vc, img);
    if ((image_index == 0) || (vmContextGetError(vc) != VM_NOERROR)) {
        return;
    }
    if (vc->debugContext != NULL) {
        while ((!vc->debugContext->running) && (vc->debugContext->debugging)) {
#ifdef WIN32
            Sleep(10);
#else
            nanosleep(&ts_rpause, NULL);
#endif
        }
    }
    for (uint32_t i = 0; i < vc->imagesCount; i++) {
        FunctionPointer *onload = binexecFindFunction(vc->images[i]->bef, "onload");
        if (onload != NULL) {
            vmContextSetCurrentImage(vc, i);
            vmContextRun(vc, onload->offset, NULL);
        }
    }
    vmContextSetCurrentImage(vc, image_index);
    vmContextRun(vc, run->offset, NULL);
}

int main(int argc, char **argv) {
    const char *optString = "vd:p:s:q:h?";
    int stackDumpSocketPort = 1876;
    int debugSocketPort = 9876;
    char* debuggerHostName = NULL;

#ifndef NDEBUG
    memDebugInit();
#endif
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif

    atexit(cleanupContext);

    int optch;
    while ((optch = getopt(argc, argv, optString)) != -1) {
        switch (optch) {
            case 'd':
            {
                debuggerHostName = optarg;
                break;
            }
            case 'p':
            {
                debugSocketPort = atoi(optarg);
                break;
            }
            case 's':
            {
                if (strbufferEquals(optarg, "stdout")) {
                    stackDumpFile = stdout;
                } else if (strbufferEquals(optarg, "socket")) {
                    stackDumpServerSocket = socket(AF_INET, SOCK_STREAM, 0);
                    if (stackDumpServerSocket == -1) {
                        fprintf(stderr, "error creating stack dump socket\n");
                        return 1;
                    }
                } else {
                    stackDumpFile = fopen(optarg, "w");
                    if (stackDumpFile == NULL) {
                        fprintf(stderr, "error creating %s\n", optarg);
                        return 1;
                    }
                }
                break;
            }
            case 'q':
            {
                stackDumpSocketPort = atoi(optarg);
                break;
            }
            case 'v':
                version(argv[0]);
                return 0;
            case 'h':
            case '?':
                usage(argv[0]);
                return 2;
        }
    }
    if ((argc - optind) <= 0) {
        usage(argv[0]);
        return 2;
    }
    if (stackDumpServerSocket != -1) {
        int one = 1;
#ifdef WIN32
            setsockopt(debugServerSocket, SOL_SOCKET, SO_REUSEADDR,(char*)&one, sizeof (one));
#else
            setsockopt(debugServerSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof (one));
#endif
        struct sockaddr_in stackDumpSin;
#if (!defined(__linux)) && (!defined(WIN32))
        stackDumpSin.sin_len = sizeof (struct sockaddr_in);
#endif
        stackDumpSin.sin_family = AF_INET;
        stackDumpSin.sin_addr.s_addr = INADDR_ANY;
        stackDumpSin.sin_port = htons(stackDumpSocketPort);
        if (bind(stackDumpServerSocket, (struct sockaddr*) &stackDumpSin, sizeof (struct sockaddr_in)) == -1) {
            close(stackDumpServerSocket);
            fprintf(stderr, "error binding stack dump socket on port %d\n", stackDumpSocketPort);
            return 1;
        }
        if (listen(stackDumpServerSocket, -1) == -1) {
            close(stackDumpServerSocket);
            fprintf(stderr, "error listening on stack dump socket\n");
            return 1;
        }
        socklen_t *len = memAlloc(sizeof(socklen_t));
        //printf("waiting for a client for stack dumping...\n");
        stackDumpSocket = accept(stackDumpServerSocket, (struct sockaddr*) &stackDumpSin, len);
        memFree(len);
        if (stackDumpSocket == -1) {
            close(stackDumpServerSocket);
            fprintf(stderr, "error accepting on stack dump socket\n");
            return 1;
        }
        stackDumpFile = fdopen(stackDumpSocket, "w");
    }

    if (debuggerHostName != NULL) {
        if (strbufferEquals(debuggerHostName, "stdin")) {
            dc = debugCreateContext(stdin, stdout);
        } else if (strbufferEquals(debuggerHostName, "socket")) {
            debugServerSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (debugServerSocket == -1) {
                fprintf(stderr, "error creating debug server socket: %d.\n", socketerror);
                return 1;
            }
            int one = 1;
#ifdef WIN32
            setsockopt(debugServerSocket, SOL_SOCKET, SO_REUSEADDR,(char*)&one, sizeof (one));
#else
            setsockopt(debugServerSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof (one));
#endif
            struct sockaddr_in stackDumpSin;
#if (!defined(__linux)) && (!defined(WIN32))
            stackDumpSin.sin_len = sizeof (struct sockaddr_in);
#endif
            stackDumpSin.sin_family = AF_INET;
            stackDumpSin.sin_addr.s_addr = INADDR_ANY;
            stackDumpSin.sin_port = htons(debugSocketPort);
            if (bind(debugServerSocket, (struct sockaddr*) &stackDumpSin, sizeof (struct sockaddr_in)) == -1) {
                close(debugServerSocket);
                fprintf(stderr, "error binding debug server socket on port %d\n", debugSocketPort);
                return 1;
            }
            if (listen(debugServerSocket, -1) == -1) {
                close(debugServerSocket);
                fprintf(stderr, "error listening on debug server socket\n");
                return 1;
            }
            socklen_t *len = memAlloc(sizeof(socklen_t));
            //printf("waiting for a remote debug client ...\n");
            debugSocket = accept(debugServerSocket, (struct sockaddr*) &stackDumpSin, len);
            memFree(len);
            if (debugSocket == -1) {
                close(debugServerSocket);
                fprintf(stderr, "error accepting on debug server socket: %d\n", socketerror);
                return 1;
            }
            dc = debugCreateContext(fdopen(debugSocket, "r"),
                    fdopen(dup(debugSocket), "w"));
        } else {
            debugSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (debugSocket == -1) {
                fprintf(stderr, "error creating debug socket : %d.\n", socketerror);
                return 1;
            }
            struct hostent *host;
            struct sockaddr_in adresse;
            if ((host = gethostbyname(debuggerHostName)) == NULL) {
                fprintf(stderr, "error host name `%s` unknown.\n",
                        debuggerHostName);
                return 1;
            }
            memcpy(&adresse.sin_addr,
                    host->h_addr_list[0],
                    sizeof (struct in_addr));
            adresse.sin_port = htons(debugSocketPort);
            adresse.sin_family = AF_INET;
            if (connect(debugSocket, (struct sockaddr*) &adresse, sizeof (adresse)) == -1) {
                fprintf(stderr, "error attaching process to debugger on machine `%s`: %d.\n",
                        debuggerHostName, socketerror);
                return 1;
            }
            dc = debugCreateContext(fdopen(debugSocket, "r"),
                    fdopen(dup(debugSocket), "w"));
        }
    }

    char *inputFileName = strbufferClone(argv[optind]);
    vc = vmContextCreate(stackDumpFile, dc, NULL, 0);
    dumpStart();
    vmRunFile(vc, inputFileName);
    dumpEnd();
    if (vmContextGetError(vc) != VM_NOERROR) {
        if (!vmContextIsRunning(vc)) {
            fprintf(stderr, "can't start program `%s`: %s.\n", inputFileName,
                    vmContextGetErrorMessage(vc));
        }
        strbufferDestroy(inputFileName);
        return 1;
    }
    strbufferDestroy(inputFileName);
    return 0;
}
