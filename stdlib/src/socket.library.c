#include "sublage/vmcontext.h"
#include "sublage/strbuffer.h"
# include <string.h>
# include <unistd.h>
#ifdef WIN32
#include <winsock2.h>
#define socklen_t int
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#endif
# include <sys/time.h>
# include <sys/types.h>
# include <sys/param.h>
# include <fcntl.h>
# include <errno.h>

typedef struct {
    int socket;
    struct sockaddr_in sin;
} vmSocket_t;

const int BUFFER_LEN = 1;

 void native_socket(VmContext *vc) {
    vmContextSetError(vc, VM_ERROR_NATIVE_ERROR, "unimplemented");
}

 void native_recv(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    vmSocket_t *s = (vmSocket_t*)so->data.privateData;
    int str_len = 0;
    char* str_buffer = memAlloc(BUFFER_LEN);
    int result = 0;
    do {
        result = recv(s->socket, str_buffer + str_len, BUFFER_LEN, 0);
        if (result > 0) {
            if (str_buffer[str_len] == '\r') {
                continue;
            } else if (str_buffer[str_len] == '\n') {
                break;
            }
            str_len += result;
            str_buffer = memRealloc(str_buffer, str_len+1);
        }
    } while ((result == BUFFER_LEN) ||
             ((result == -1) && (errno == EAGAIN)));
    str_buffer[str_len] = 0;
    vmContextPush(vc, stackObjectNewString(vc, str_buffer));
    memFree(str_buffer);
}

 void native_send(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    StackObject *msg = vmContextPopOpcode(vc, OPCODE_DATA);
    if ((so == NULL) || (msg == NULL)) { return; }
    vmSocket_t *s = (vmSocket_t*)so->data.privateData;
    StackObjectData *data = msg->data.privateData;
    send(s->socket, data->data, data->nbytes, 0);
}

 void native_send_line(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    StackObject *msg = vmContextPopOpcode(vc, OPCODE_STRING);
    if ((so == NULL) || (msg == NULL)) { return; }
    vmSocket_t *s = (vmSocket_t*)so->data.privateData;
    char* str = vmContextGetString(vc, msg);
    send(s->socket, str, strlen(str), 0);
    send(s->socket, "\r\n", 2, 0);
}


 void native_accept(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) { return; }
    vmSocket_t *s = (vmSocket_t*)so->data.privateData;
    vmSocket_t *sc = memAlloc(sizeof(vmSocket_t));
    socklen_t len = 0;
    sc->socket = accept(s->socket, (struct sockaddr*)&sc->sin, &len);
    if (sc->socket == -1) {
        vmContextPush(vc, stackObjectNewNull(vc));
        return;
    }
#ifdef WIN32
    u_long iMode=1;
    if ((ioctlsocket(sc->socket, FIONBIO, &iMode)) != 0) {
#else
    if ((fcntl(sc->socket, F_SETFL, fcntl(sc->socket, F_GETFL) | O_NONBLOCK) < 0) == -1) {
#endif
        vmContextPush(vc, stackObjectNewNull(vc));
        return;
    }
    vmContextPush(vc, stackObjectNewPrivate(vc, sc));
}

 void native_close(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) return;
    vmSocket_t *s = (vmSocket_t*)so->data.privateData;
    close(s->socket);
    memFree(s);
}

 void native_socket_server(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
    int port = so->data.intValue;

    vmSocket_t *s = memAlloc(sizeof(vmSocket_t));
    s->socket = socket (PF_INET, SOCK_STREAM, 0);
    if (s->socket == -1) {
        vmContextSetError(vc, VM_ERROR_NATIVE_ERROR, "socketserver : socket error");
        return;
    }
    int one = 1;
#ifdef WIN32
    setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
#else
    setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif
#if (!defined(__linux)) && (!defined(WIN32))
    s->sin.sin_len = sizeof(struct sockaddr_in);
#endif
    s->sin.sin_family = AF_INET;
    s->sin.sin_addr.s_addr = INADDR_ANY;
    s->sin.sin_port = htons(port);
    if (bind(s->socket, (struct sockaddr*)&s->sin, sizeof(struct sockaddr_in)) == -1) {
        close(s->socket);
        vmContextSetError(vc, VM_ERROR_NATIVE_ERROR, "socketserver : bind error");
        return;
    }
    if (listen(s->socket, -1) == -1) {
        close(s->socket);
        vmContextSetError(vc, VM_ERROR_NATIVE_ERROR, "socketserver : listen error");
        return;
    }
    
    vmContextPush(vc, stackObjectNewPrivate(vc, s));
}
