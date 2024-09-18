#pragma once

#include "sublage/linkedlist.h"

typedef LinkedList Stack;

Stack* stackCreate();
void stackDestroy(Stack *s, bool destroyDatas);
void stackPush(Stack *s, void *data);
void* stackPop(Stack *s);
void* stackPopFirst(Stack *s);
