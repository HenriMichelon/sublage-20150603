#include "unittest.h"
#include "sublage/linkedlist.h"

static char* tests_linkedlist() {
    LinkedList* foo = linkedListCreate();

    ut_assert("linkedListSize|0", linkedListSize(foo) == 0);
    ut_assert("linkedListGet|0", linkedListGet(foo, 0) == NULL);    
    ut_assert("linkedListGetFirst|0", linkedListGetFirst(foo) == NULL);    
    ut_assert("linkedListGetLast|0", linkedListGetLast(foo) == NULL); 

    char* data1 = "1";
    linkedListAppend(foo, data1);
    // 1
    ut_assert("linkedListSize|1", linkedListSize(foo) == 1);
    ut_assert("linkedListGet|1", linkedListGet(foo, 0) == data1);
    ut_assert("linkedListGetFirst|1", linkedListGetFirst(foo) == data1);
    ut_assert("linkedListGetLast|1", linkedListGetLast(foo) == data1);
    
    char* data2 = "2";
    linkedListAppend(foo, data2);
    // 1 2
    ut_assert("linkedListSize|2", linkedListSize(foo) == 2);
    ut_assert("linkedListGet|2/0", linkedListGet(foo, 0) == data1);
    ut_assert("linkedListGet|2/1", linkedListGet(foo, 1) == data2);
    ut_assert("linkedListGetLast|2", linkedListGetLast(foo) == data2);
    
    char* data3 = "3";
    linkedListAppend(foo, data3);
    // 1 2 3
    ut_assert("linkedListSize|3", linkedListSize(foo) == 3);
    ut_assert("linkedListGet|3/0", linkedListGet(foo, 0) == data1);
    ut_assert("linkedListGet|3/1", linkedListGet(foo, 1) == data2);
    ut_assert("linkedListGet|3/2", linkedListGet(foo, 2) == data3);
    ut_assert("linkedListGetLast|3", linkedListGetLast(foo) == data3);

    char* data4 = "4";
    linkedListInsert(foo, 0, data4);
    // 4 1 2 3
    ut_assert("linkedListSize|4", linkedListSize(foo) == 4);
    ut_assert("linkedListGet|4/0", linkedListGet(foo, 0) == data4);
    ut_assert("linkedListGet|4/1", linkedListGet(foo, 1) == data1);
    ut_assert("linkedListGet|4/2", linkedListGet(foo, 2) == data2);
    ut_assert("linkedListGetFirst|4", linkedListGetFirst(foo) == data4);

    char* data5 = "5";
    linkedListInsert(foo, 1, data5);
    // 4 5 1 2 3
    ut_assert("linkedListSize|5", linkedListSize(foo) == 5);
    ut_assert("linkedListGet|5/0", linkedListGet(foo, 0) == data4);
    ut_assert("linkedListGet|5/1", linkedListGet(foo, 1) == data5);
    ut_assert("linkedListGet|5/2", linkedListGet(foo, 2) == data1);
    ut_assert("linkedListGetFirst|5", linkedListGetFirst(foo) == data4);
    
    char* data6 = "6";
    // 4 5 1 2 6 3
    linkedListInsert(foo, 4, data6);
    ut_assert("linkedListSize|6", linkedListSize(foo) == 6);
    ut_assert("linkedListGet|6/3", linkedListGet(foo, 3) == data2);
    ut_assert("linkedListGet|6/4", linkedListGet(foo, 4) == data6);
    ut_assert("linkedListGet|6/5", linkedListGet(foo, 5) == data3);
    ut_assert("linkedListGetLast|6", linkedListGetLast(foo) == data3);

    char* data7 = "7";
    linkedListInsert(foo, 6, data7);
    // 4 5 1 2 6 3 7
    ut_assert("linkedListSize|7", linkedListSize(foo) == 7);
    ut_assert("linkedListGet|7/5", linkedListGet(foo, 5) == data3);
    ut_assert("linkedListGet|7/6", linkedListGet(foo, 6) == data7);
    ut_assert("linkedListGetLast|7", linkedListGetLast(foo) == data7);
    
    char* data8 = "8";
    linkedListSet(foo, data8, 3);
    // 4 5 1 8 6 3 7
    ut_assert("linkedListGet|8/3", linkedListGet(foo, 3) == data8);

    ut_assert("linkedListRemoveFirst|9", linkedListRemoveFirst(foo) == data4);
                 
    // 5 1 8 6 3 7
    ut_assert("linkedListSize|9", linkedListSize(foo) == 6);
    ut_assert("linkedListGetFirst|9", linkedListGetFirst(foo) == data5);
    
    ut_assert("linkedListRemoveLast|10", linkedListRemoveLast(foo) == data7);
    // 5 1 8 6 3
    ut_assert("linkedListSize|10", linkedListSize(foo) == 5);
    ut_assert("linkedListGetLast|10", linkedListGetLast(foo) == data3);
    
    ut_assert("linkedListRemoveAt|11", linkedListRemoveAt(foo, 0) == data5);
    
    // 1 8 6 3
    ut_assert("linkedListRemoveAt|12", linkedListRemoveAt(foo, 3) == data3);
    // 1 8 6
    ut_assert("linkedListRemoveAt|13", linkedListRemoveAt(foo, 1) == data8);
    // 1 6
    ut_assert("linkedListGetFirst|14", linkedListGetFirst(foo) == data1);
    ut_assert("linkedListGetLast|14", linkedListGetLast(foo) == data6);

    linkedListAppend(foo, data2);
    linkedListAppend(foo, data3);
    // 1 6 2 3
    linkedListRemove(foo, data6);
    // 1 2 3
    ut_assert("linkedListGetFirst|15", linkedListGetFirst(foo) == data1);
    ut_assert("linkedListGetLast|15", linkedListGetLast(foo) == data3);
    
    linkedListRemove(foo, data1);
    // 2 3
    ut_assert("linkedListGetFirst|16", linkedListGetFirst(foo) == data2);
    ut_assert("linkedListGetLast|16", linkedListGetLast(foo) == data3);
    
    linkedListRemove(foo, data3);
    // 2
    ut_assert("linkedListSize|17", linkedListSize(foo) == 1);
    ut_assert("linkedListGetFirst|17", linkedListGetFirst(foo) == data2);
    ut_assert("linkedListGetLast|17", linkedListGetLast(foo) == data2);
    
    linkedListClear(foo, false);
    ut_assert("linkedListSize|18", linkedListSize(foo) == 0);

    linkedListDestroy(foo, false);
    return UT_NOERROR;
}