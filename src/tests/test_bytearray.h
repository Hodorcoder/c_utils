#ifndef CU_INCLUDE_TEST_BA
#define CU_INCLUDE_TEST_BA

#include "../bytearray.h"

#ifndef NDEBUG
void test_bytearray(void);
bytearray *look_and_say(const char *seed_str, int terms);
#endif // NDEBUG
#endif
