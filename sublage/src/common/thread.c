#include "sublage/thread.h"

#if defined(__APPLE__) || defined (__linux) || defined(__FreeBSD__)

bool ThreadMutexCreate(ThreadMutex *mutex) {
    return pthread_mutex_init(mutex, NULL) == 0;
}

bool ThreadCreate(ThreadThread *thread, void *(*handler)(void *), void *data) {
    return pthread_create(thread, NULL, handler, data) == 0;
}

#elif defined(WIN32) 

bool ThreadMutexCreate(ThreadMutex *mutex) {
    *mutex = CreateMutex(NULL, FALSE, NULL);
    return (*mutex) != NULL;
}

bool ThreadCreate(ThreadThread *thread, 
        ThreadRoutineReturnType(*handler)(void *), 
        void *data) {
    DWORD id;
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handler, data, 0, &id);
    return (*thread) != NULL;
}

#endif
