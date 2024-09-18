#include "sublage/types.h"
#include "sublage/mem.h"
#define _LINKEDLIST_PRIVATE_

typedef struct LinkedListNode {
    struct LinkedListNode *nextNode;
    void *data;
} LinkedListNode;

typedef struct {
    LinkedListNode *currentNode;
} LinkedListIterator;

typedef struct {
    LinkedListNode *listHead;
    LinkedListNode *listTail;
    uint64_t size;
} LinkedList;

#include "sublage/linkedlist.h"

LinkedList* linkedListCreate() {
    LinkedList *ll = memAlloc(sizeof (LinkedList));
    ll->listHead = NULL;
    ll->listTail = NULL;
    ll->size = 0;
    return ll;
}

void linkedListClear(LinkedList *ll, bool destroyDatas) {
    LinkedListNode *n = ll->listHead;
    while (n != NULL) {
        LinkedListNode *next = n->nextNode;
        if (destroyDatas && (n->data != NULL)) {
            memFree(n->data);
        }
        memFree(n);
        n = next;
    }
    ll->listHead = NULL;
    ll->listTail = NULL;
    ll->size = 0;
}

int64_t linkedListIndexOf(LinkedList *ll, void *data) {
    LinkedListNode *prev = NULL;
    LinkedListNode *node = ll->listHead;
    uint64_t index = 0;
    while (node != NULL) {
        if (node->data == data) {
            return index;
        }
        prev = node;
        node = node->nextNode;
        index++;
    }
    return -1;
}

void linkedListRemove(LinkedList *ll, void *data) {
    LinkedListNode *prev = NULL;
    LinkedListNode *node = ll->listHead;
    while (node != NULL) {
        if (node->data == data) {
            if (prev == NULL) {
                ll->listHead = node->nextNode;
            } else {
                prev->nextNode = node->nextNode;
                if (node == ll->listTail) {
                    ll->listTail = prev;
                }
            }
            memFree(node);
            ll->size--;
            break;
        }
        prev = node;
        node = node->nextNode;
    }
}

void linkedListDestroy(LinkedList *ll, bool destroyDatas) {
    linkedListClear(ll, destroyDatas);
    memFree(ll);
}

void linkedListAppend(LinkedList *ll, void *data) {
    LinkedListNode *node = memAlloc(sizeof (LinkedListNode));
    //printf("append: %x -> %x\n", node, data);
    node->nextNode = NULL;
    node->data = data;
    if (ll->listHead == NULL) {
        ll->listHead = node;
    } else {
        ll->listTail->nextNode = node;
    }
    ll->listTail = node;
    ll->size++;
}

void linkedListAppendList(LinkedList *ll, LinkedList *src) {
    LinkedListNode *node = src->listHead;
    while (node != NULL) {
        linkedListAppend(ll, node->data);
        node = node->nextNode;
    }
}

void* linkedListRemoveFirst(LinkedList *ll) {
    LinkedListNode *node = ll->listHead;
    if (node != NULL) {
        ll->listHead = node->nextNode;
        ll->size--;
    }
    if (ll->size == 1) {
        ll->listTail = ll->listHead;
    } else if (ll->size == 0) {
        ll->listTail = NULL;
    }
    void* data = node->data;
    memFree(node);
    return data;
}

void* linkedListGetFirst(LinkedList *ll) {
    if (ll->listHead != NULL) {
        return ll->listHead->data;
    }
    return NULL;
}

void* linkedListRemoveLast(LinkedList *ll) {
    LinkedListNode *node = ll->listHead;
    while (node != NULL) {
        if (node->nextNode == ll->listTail) {
            void *data = ll->listTail->data;
            memFree(ll->listTail);
            ll->listTail = node;
            node->nextNode = NULL;
            ll->size--;
            return data;
        } else if (node->nextNode == NULL) {
            void *data = node->data;
            memFree(node);
            ll->listHead = NULL;
            ll->listTail = NULL;
            ll->size--;
            return data;
        }
        node = node->nextNode;
    }
    return NULL;
}

void* linkedListGetLast(LinkedList *ll) {
    if (ll->listTail != NULL) {
        return ll->listTail->data;
    }
    return NULL;
}

void linkedListInsert(LinkedList *ll, uint64_t index, void *data) {
    LinkedListNode *node = memAlloc(sizeof (LinkedListNode));
    node->data = data;
    if (index == 0) {
        node->nextNode = ll->listHead;
        ll->listHead = node;
        if (ll->size == 0) {
            ll->listTail = node;
        }
        ll->size++;
    } else if (index == ll->size) {
        ll->listTail->nextNode = node;
        ll->listTail = node;
        ll->size++;
    } else {
        LinkedListNode *temp = ll->listHead;
        while ((temp != NULL) && (--index)) {
            temp = temp->nextNode;
        }
        if (temp != NULL) {
            node->nextNode = temp->nextNode;
            temp->nextNode = node;
            ll->size++;
        }
    }
}

void* linkedListGet(LinkedList *ll, uint64_t index) {
    LinkedListNode *node = ll->listHead;
    while ((node != NULL) && (index--)) {
        node = node->nextNode;
    }
    if (node != NULL) {
        return node->data;
    }
    return NULL;
}

void* linkedListRemoveAt(LinkedList *ll, uint64_t index) {
    LinkedListNode *prev = NULL;
    LinkedListNode *node = ll->listHead;
    while ((node != NULL) && (index--)) {
        prev = node;
        node = node->nextNode;
    }
    if (node != NULL) {
        void *data = node->data;
        if (prev == NULL) {
            ll->listHead = node->nextNode;
        } else {
            prev->nextNode = node->nextNode;
            if (node == ll->listTail) {
                ll->listTail = prev;
            }
        }
        memFree(node);
        ll->size--;
        return data;
    }
    return NULL;
}


void linkedListSet(LinkedList *ll, void* data, uint64_t index) {
    LinkedListNode *node = ll->listHead;
    while ((node != NULL) && (index--)) {
        node = node->nextNode;
    }
    if (node != NULL) {
        node->data = data;
    }
}

uint64_t linkedListSize(LinkedList *ll) {
    return ll->size;
}

//static int cicount = 0;

LinkedListIterator* linkedListCreateIterator(LinkedList *ll) {
    LinkedListIterator *lli = memAlloc(sizeof (LinkedListIterator));
    //printf("%d : %x\n", ++cicount, lli);
    lli->currentNode = ll->listHead;
    return lli;
}

LinkedListIterator* linkedListCreateIteratorAtPos(LinkedList *ll, uint64_t pos) {
    if (pos >= ll->size) {
        return NULL;
    }
    LinkedListIterator *lli = memAlloc(sizeof (LinkedListIterator));
    lli->currentNode = ll->listHead;
    for (uint64_t i = 1; i <= pos; i++) {
        lli->currentNode = lli->currentNode->nextNode;
    }
    return lli;
}


void *linkedListIteratorNext(LinkedListIterator *lli) {
    if (lli->currentNode != NULL) {
        void *data = lli->currentNode->data;
        lli->currentNode = lli->currentNode->nextNode;
        return data;
    }
    return NULL;
}

void linkedListIteratorDestroy(LinkedListIterator *lli) {
    memFree(lli);
}
