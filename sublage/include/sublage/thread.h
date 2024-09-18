#pragma once

#include <stdbool.h>

#if defined(__APPLE__) || defined(__linux) || defined(__FreeBSD__)

#include <pthread.h>
typedef pthread_t ThreadThread;
typedef void* ThreadRoutineReturnType;
typedef pthread_mutex_t ThreadMutex;
#define ThreadRoutineReturn NULL
#define ThreadJoin(thread) pthread_join(thread, NULL)
#define ThreadMutexLock(mutex) pthread_mutex_lock(mutex)
#define ThreadMutexUnlock(mutex) pthread_mutex_unlock(mutex)
#define ThreadMutexDestroy(mutex) pthread_mutex_destroy(mutex)

#elif defined(WIN32)
#define VC_EXTRA_LEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef HANDLE ThreadThread;
typedef HANDLE ThreadMutex;
typedef DWORD ThreadRoutineReturnType;
#define ThreadRoutineReturn 0
#define ThreadJoin(thread)  WaitForSingleObject(thread, INFINITE)
#define ThreadMutexLock(mutex) WaitForSingleObject(mutex, INFINITE)
#define ThreadMutexUnlock(mutex) ReleaseMutex(mutex)
#define ThreadMutexDestroy(mutex) CloseHandle(mutex)
#endif

bool ThreadCreate(ThreadThread *thread, 
        ThreadRoutineReturnType(*handler)(void *), 
        void *data);
bool ThreadMutexCreate(ThreadMutex *mutex);
