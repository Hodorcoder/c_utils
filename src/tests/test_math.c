#include <stdio.h>
#include "test_math.h"
#include "../cutil_math.h"

typedef struct {
    int a, b;
    unsigned result;
} gcd_calc;

static void print_gcd_calc(const gcd_calc *g)
{
    printf("gcd(%d,%d) = %u", g->a, g->b, g->result);
}

int test_gcd()
{
    gcd_calc r;

    r.a = 114;
    r.b = -65535;
    r.result = gcd(r.a, r.b);
    print_gcd_calc(&r); printf("\n");

    return 1;
}

