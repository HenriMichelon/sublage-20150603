#pragma once

#include "sublage/vmcontext.h"

typedef void (*InternalFunction)(VmContext *vc);
extern const InternalFunction codeArray[];

