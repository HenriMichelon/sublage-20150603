#include <stdio.h>
#include "unittest.h"
#include "tests_common_byteorder.h"
#include "tests_common_strbuffer.h"
#include "tests_common_linkedlist.h"

int tests_run = 0;

static char* all_tests() {
    ut_run_test(tests_byteorder);
    ut_run_test(tests_strbuffer);
    ut_run_test(tests_linkedlist);
    return UT_NOERROR;
}

int main(int argc, char** argv) {
   char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
 
    return result != 0;
}