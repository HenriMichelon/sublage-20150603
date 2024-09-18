#include "unittest.h"
#include "sublage/strbuffer.h"

static char* tests_strbuffer() {
    char *foo = strbufferCreate();
    ut_assert("strbufferEquals", strbufferEquals("Azerty", "Azerty"));
    ut_assert("strbufferEquals", !strbufferEquals("Azerty", "qwerty"));
    foo = strbufferAppendChar(foo, 'A'); 
    ut_assert("strbufferAppendChar", strbufferEquals(foo, "A"));
    foo = strbufferAppendStr(foo, "ZERTY", 2);
    ut_assert("strbufferAppendStr", strbufferEquals(foo, "AZE"));
    foo = strbufferAppendStr(foo, "RTY", -1);
    ut_assert("strbufferAppendStr", strbufferEquals(foo, "AZERTY"));
    char* foo_clone = strbufferClone(foo);
    ut_assert("strbufferClone", strbufferEquals(foo, foo_clone));
    strbufferDestroy(foo_clone);
    char* foo_sub = strbufferSubStr(foo, 0, 3);
    ut_assert("strbufferSubStr", strbufferEquals(foo_sub, "AZE"));
    strbufferDestroy(foo_sub);
    foo_sub = strbufferSubStr(foo, 1, 3);
    ut_assert("strbufferSubStr", strbufferEquals(foo_sub, "ZER"));
    strbufferDestroy(foo_sub);
    foo_sub = strbufferSubStr(foo, 2, -1);
    ut_assert("strbufferSubStr", strbufferEquals(foo_sub, "ERTY"));
    strbufferDestroy(foo_sub);
    foo_sub = strbufferSubStr(foo, 10, -1);
    ut_assert("strbufferSubStr", strbufferEquals(foo_sub, ""));
    strbufferDestroy(foo_sub);
    foo_sub = strbufferLeft(foo, 2);
    ut_assert("strbufferLeft", strbufferEquals(foo_sub, "AZ"));
    strbufferDestroy(foo_sub);
    ut_assert("strbufferStrPos", strbufferStrPos(foo, "AZ") == 0);
    ut_assert("strbufferStrPos", strbufferStrPos(foo, "TY") == 4);
    ut_assert("strbufferStrPos", strbufferStrPos(foo, "FOO") == -1);
    strbufferDestroy(foo);
    return UT_NOERROR;
}
