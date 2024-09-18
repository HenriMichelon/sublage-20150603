#include "utils.h"

#ifndef WIN32
#include <termios.h>

static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new = old; /* make new settings same as old settings */
    new.c_lflag &= ~ICANON; /* disable buffered i/o */
    new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}
#endif

 void native_flush(VmContext *vc) {
    fflush(stdout);
}

 void native_read_char(VmContext *vc) {
    char* buffer = (char*) memAlloc(sizeof (char)*2);
    buffer[1] = 0;
#ifdef WIN32
    buffer[0] = getchar();
#else
    buffer[0] = getch_(1);
#endif
    vmContextPush(vc, stackObjectNewString(vc, buffer));
    memFree(buffer);
}

 void native_read_line(VmContext *vc) {
    int size = 200;
    int length = 0;
    char* buffer = (char*) memAlloc(sizeof (char)*size);
    char c = 'a';
    while ((c != '\n') && (c != EOF)) {
        if (length == size) {
            size *= 2;
            buffer = (char*) memRealloc(buffer, sizeof (char)*size);
        }
        c = getc(stdin);
        if ((c != '\n') && (c != EOF)) {
            buffer[length++] = c;
        }
    }
    buffer[length] = 0;
    vmContextPush(vc, stackObjectNewString(vc, buffer));
    memFree(buffer);
}

 void native_new_line(VmContext *vc) {
    fprintf(stdout, "\n");
    fflush(stdout);
}

 void native_print(VmContext *vc) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    util_printStackObject(vc, stdout, so);
}
