#include "unittest.h"
#include "sublage/byteorder.h"

static char* tests_byteorder() {
    ut_assert("vmtohll/htovmll", htovmll(vmtohll(12345)) == 12345);
    ut_assert("vmtohl/htovml", htovml(vmtohl(12345)) == 12345);
    ut_assert("vmtohs/htovms", htovms(vmtohs(123)) == 123);
    ut_assert("vmtohf/htovmf", htovmf(vmtohf(123.45f)) == 123.45f);
    ut_assert("vmtohd/htovmd", htovmd(vmtohd(123.45)) == 123.45);
    return UT_NOERROR;
}