#include "sublage/vmcontext.h"
#include <stdlib.h>

 void native_random(VmContext *vc) {
#ifdef __linux
    vmContextPush(vc, stackObjectNewInt(vc, random()));
#elif WIN32
    vmContextPush(vc, stackObjectNewInt(vc, rand()));
#else
    vmContextPush(vc, stackObjectNewInt(vc, arc4random()));
#endif
}
