#include "tests/test_bytearray.h"

int main()
{
    test_bytearray();
    look_and_say("3", 10);  // with seed of 3, results in A006715
    return 0;
}
