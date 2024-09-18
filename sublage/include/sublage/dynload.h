#pragma once
#include <stdbool.h>

#if defined(__APPLE__) || defined(__linux) || defined(__FreeBSD__)

#include <dlfcn.h>
typedef void* DynloadLibrary;
#define DynloadLoad(name) dlopen(name, RTLD_NOW | RTLD_GLOBAL)
#define DynloadUnload(library) dlclose(library)
#define DynloadGetFunction(library, name) dlsym(library, name)
#define DynloadLastError() dlerror()

#elif defined(WIN32)

#define VC_EXTRA_LEAN 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef HMODULE DynloadLibrary;
#define DynloadLoad(name) LoadLibrary((LPCTSTR)name)
#define DynloadUnload(library) FreeLibrary(library)
#define DynloadGetFunction(library, name) GetProcAddress(library, (LPCSTR)name)
const char* DynloadLastError();

#endif
