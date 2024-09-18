#include "sublage/mem.h"
#include <string.h>
#include "function.h"
#include "sublage/strbuffer.h"

Function *functionCreate(char *name) {
  Function *f = memAlloc(sizeof(Function));
  f->private = (name[0] == '(');
  if (f->private) {
      // undecorate name
      f->name = strbufferSubStr(name, 1, strlen(name)-2);
  } else {
      f->name = strbufferClone(name);
  }
  f->offset = 0;
  f->code = linkedListCreate();
  f->calls = linkedListCreate();
  return f;
}

void functionDestroy(Function *f) {
  memFree(f->name);
  LinkedListIterator *lli = linkedListCreateIterator(f->code);
  LexicalToken *lt = NULL;
  while ( (lt = linkedListIteratorNext(lli)) != NULL) {
    lexicalDestroyToken(lt);
  }
  linkedListIteratorDestroy(lli);
  linkedListDestroy(f->code, false);
  linkedListDestroy(f->calls, true);
  memFree(f);
}

/*void functionAppendCode(Function *f, LexicalToken *lt) {
  LexicalToken *t = lexicalCloneToken(lt);
  linkedListAppend(f->code, t);
}*/

void functionAppendCall(Function *f, uint64_t offset) {
  uint64_t *o = memAlloc(sizeof(uint64_t));
  *o = offset;
  linkedListAppend(f->calls, o);
}

LinkedList *functionGetCodeList(Function *f) {
  return f->code;
}

char * functionGetName(Function *f) {
  return f->name;
}

LinkedList *functiongetGetCallsList(Function *f) {
  return f->calls;
}

void functionSetOffset(Function *f, uint64_t offset) {
  f->offset = offset;
}

uint64_t functionGetOffset(Function *f) {
  return f->offset;
}
