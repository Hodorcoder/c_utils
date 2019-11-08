#include "cutil_math.h"

unsigned gcd(int a, int b)
{
    /* Binary GCD algorithm
     * c.f. Knuth, TAOCP Vol. 2, 3rd Ed. (1998), p. 338
     */
    unsigned shift, u, v;

    if (a == 0) return b;
    if (b == 0) return a;

    u = a < 0 ? -a : a;
    v = b < 0 ? -b : b;

    /* Can also calculate shift using Intel CTZ (count trailing zeros)
     * but keeping code portable...
     */
    for (shift = 0; ((u | v) & 1) == 0; shift++) {
        u >>= 1;
        v >>= 1;
    }
    while (!(u & 1))    // While u is even, halve it
        u >>= 1;

    do {
        while ((v & 1) == 0)    // alternatively, could shift by CTZ
            v >>= 1;
        if (v < u) {
            unsigned t = u;
            u = v;
            v = t;
        }
        v -= u;
    } while (v);

    return u << shift;
}

