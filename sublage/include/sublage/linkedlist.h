#pragma once

#include "sublage/types.h"

#ifndef _LINKEDLIST_PRIVATE_
struct _LinkedListIterator;
struct _LinkedList;
typedef struct _LinkedListIterator LinkedListIterator;
typedef struct _LinkedList LinkedList;
#endif

LinkedList* linkedListCreate();
void linkedListDestroy(LinkedList *ll, bool destroyDatas);

void linkedListAppend(LinkedList *ll, void *data);
void linkedListInsert(LinkedList *ll, uint64_t index, void *data);
void linkedListAppendList(LinkedList *ll, LinkedList *src);

void linkedListSet(LinkedList *ll, void *data, uint64_t index);
void* linkedListGet(LinkedList *ll, uint64_t index);
int64_t linkedListIndexOf(LinkedList *ll, void *data);
void* linkedListGetFirst(LinkedList *ll);
void* linkedListGetLast(LinkedList *ll);

void linkedListRemove(LinkedList *ll, void *data);
void* linkedListRemoveAt(LinkedList *ll, uint64_t);
void* linkedListRemoveFirst(LinkedList *ll);
void* linkedListRemoveLast(LinkedList *ll);
void linkedListClear(LinkedList *ll, bool destroyDatas);

uint64_t linkedListSize(LinkedList *ll);

LinkedListIterator* linkedListCreateIterator(LinkedList *ll);
LinkedListIterator* linkedListCreateIteratorAtPos(LinkedList *ll, uint64_t pos);
void *linkedListIteratorNext(LinkedListIterator *lli);
void linkedListIteratorDestroy(LinkedListIterator *lli);

