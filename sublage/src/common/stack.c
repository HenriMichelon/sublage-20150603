#include "sublage/mem.h"
#include "sublage/stack.h"

Stack* stackCreate() {
  return (Stack*)linkedListCreate();
}

void stackDestroy(Stack *s, bool destroyDatas) {
  linkedListDestroy((LinkedList*)s, destroyDatas);
}

void stackPush(Stack *s, void *data) {
  linkedListAppend((LinkedList*)s, data);
}

void* stackPop(Stack *s) {
  return linkedListRemoveLast((LinkedList*)s);
}
