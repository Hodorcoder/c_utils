#ifndef CU_INCLUDE_TEST_BA
#define CU_INCLUDE_TEST_BA

#include "../cutil_string.h"

#ifndef NDEBUG
void test_bytearray(void);
cuStr *look_and_say(const char *seed_str, int terms);
#endif // NDEBUG
#endif
