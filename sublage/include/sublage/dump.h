#pragma once

#include <stdio.h>
#include "sublage/vmcontext.h"

void dumpStart();
void dumpEnd();
void dumpFormatObject(char* buf, size_t bufsize, StackObject *so, VmContext *vc,
                      bool stringfrompool);
void dumpContext(VmContext *vc);
void dumpStackObject(StackObject *so, VmContext *vc);
