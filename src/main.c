#include "tests/test_string.h"
#include "tests/test_math.h"

int main()
{

    // FIXME: Use something better than NDEBUG (maybe a new macro
    //        RUN_UNIT_TESTS or something similar. NDEBUG isn't great because
    //        it means that profiling (e.g. using valgrind) can only be done
    //        using debug (not release) builds.
#ifndef NDEBUG
    test_bytearray();
    test_gcd();
#endif

    return 0;
}
