#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include "bytearray.h"

#ifndef CU_BYTEARRAY_CHUNK_SIZE
#   define CU_BYTEARRAY_CHUNK_SIZE    255
#endif
#define CU_BYTEARRAY_INITIAL_MEM      CU_BYTEARRAY_CHUNK_SIZE

// FIXME: TODO: !!!!! Any code in here that uses size_t needs to be checked
// to ensure that, e.g. max_elements, is never more than size_t - 1 because
// all these functions allocate an extra element so that there is always
// room for a terminating '\0'. CHECK AND FIX WHERE NECESSARY

// FIXME: There's a problem with "chuncked" allocation; i.e. if resizing
// up overflows there's a small problem. Add checks to fix this

/* ===========================================================================
   Private functions
   =========================================================================*/

#define BA_CHUNKED_SZ(i) (((i) / CU_BYTEARRAY_CHUNK_SIZE + 1) * CU_BYTEARRAY_CHUNK_SIZE)

static struct bytearray *ba_init(struct bytearray *ba, int sz)
{
    assert(ba != NULL); // pre-condition

    ba->elements_used = 0;

    if (sz == -1 || sz > 0) {
        sz = sz == -1 ? CU_BYTEARRAY_INITIAL_MEM : BA_CHUNKED_SZ(sz);

        /* Always want to have an extra element for '\0'
         */
        ba->mem = malloc((sz + 1) * sizeof ba->mem);

        /* Always NUL terminate so a call to ba_cstr(), for example, won't
         * return a non-terminated string if the byte array hasn't been
         * populated with anything.
         */
        if (ba->mem) {
            ba->mem[0] = '\0';
        }
        ba->max_elements = sz;
    } else {
        ba->mem = NULL;
        ba->max_elements = 0;
    }

    ba->resize_flags = BA_RESIZE_UP_CHUNKED | BA_RESIZE_DN_EXACT;
    //ba->resize_flags = BA_RESIZE_UP_EXACT | BA_RESIZE_DN_EXACT;

    return ba;
}

static bytearray *ba_resize(bytearray *ba, size_t max_elements)
{
    size_t len;

    assert(ba != NULL); // pre-condition

    if (max_elements == ba->max_elements) {
        return ba;
    } else if (max_elements > ba->max_elements) {
        if (ba->resize_flags & BA_RESIZE_UP_CHUNKED) {
            len = BA_CHUNKED_SZ(max_elements);
        } else {
            assert(ba->resize_flags & BA_RESIZE_UP_EXACT);
            len = max_elements;
        }
    } else {
        // TODO: Implement downsizing properly
        len = max_elements;
    }

    len += 1;   // Always allow room for '\0'

    unsigned char *new_mem = realloc(ba->mem, len);
    if (!new_mem)
        return NULL;
    ba->mem = new_mem;
    ba->max_elements = len - 1; // -1 because there is extra space for '\0'
    return ba;
}

/* ===========================================================================
   Public functions
   =========================================================================*/

bytearray *ba_new(int sz)
{
    struct bytearray *ba;

    if ((ba = malloc(sizeof *ba)) != NULL) {
        if (!ba_init(ba, sz)) {
            free(ba);
            ba = NULL;
        }
    }

    return ba;
}

void ba_set_resize_strategy(bytearray *ba, unsigned flags)
{
    assert(ba != NULL); // pre-condition

    ba->resize_flags = flags;
}


void ba_destroy(bytearray **ba)
{
    assert(ba != NULL); // pre-condition

    free((*ba)->mem);
    free(*ba);
    *ba = NULL;
}

bool ba_isfull(const bytearray *ba)
{
    assert(ba != NULL); // pre-condition
    return ba->elements_used >= ba->max_elements;
}

size_t ba_max_elements(const bytearray *ba)
{
    assert(ba != NULL); // pre-condition
    return ba->max_elements;
}

size_t ba_size(const bytearray *ba)
{
    assert(ba != NULL); // pre-condition
    return ba->elements_used;
}

const char *ba_cstr(const bytearray *ba)
{
    assert(ba != NULL); // pre-condition
    return (const char *)ba->mem;
}

bytearray *ba_clear(bytearray *ba)
{
    assert(ba != NULL); // pre-condition

    if (ba->mem) {
        ba->mem[0] = '\0';
        ba->elements_used = 0;
    }
    return ba;
}

bytearray *ba_shrinktofit(bytearray *ba)
{
    assert(ba != NULL); // pre-condition

    if (ba->max_elements > ba->elements_used) {
        BYTE *newmem;
        size_t newlen = ba->elements_used + 1;
        if ((newmem = realloc(ba->mem, newlen)) == NULL) {
            return NULL;    // TODO: Indicate error somehow
        }
        ba->mem = newmem;
        newlen--;   // reserve the '\0' space
        ba->max_elements = newlen;
        ba->elements_used = newlen;
        ba->mem[newlen] = '\0'; // make sure it's still null terminated
    }
    return ba;
}

bytearray *ba_set_from_cstr(bytearray *ba, const char *str)
{
    assert(ba != NULL);  // pre-condition
    assert(str != NULL); // pre-condition

    return ba_set(ba, (const BYTE *)str, strlen(str));
}

bytearray *ba_set(bytearray *ba, const BYTE *from, size_t len)
{
    assert(ba != NULL);   // pre-condition
    assert(from != NULL); // pre-condition

    if (len > ba->max_elements) {
        bytearray *tmp = ba_resize(ba, len);
        if (!tmp)
            return NULL; // TODO: should we destroy the original byte array?
    }
    memcpy(ba->mem, from, len);
    ba->elements_used = len;
    ba->mem[len] = '\0';
    return ba;
}

bytearray *ba_append_cstr(bytearray *ba, const char *str)
{
    assert(ba != NULL);  // pre-condition
    assert(str != NULL); // pre-condition

    return ba_append(ba, (const BYTE *)str, strlen(str));
}

bytearray *ba_append(bytearray *ba, const BYTE *bytes, size_t len)
{
    size_t newlen;

    assert(ba != NULL);    // pre-condition
    assert(bytes != NULL); // pre-condition

    newlen = len + ba->elements_used;
    if (newlen > ba->max_elements) {
        if (!ba_resize(ba, len + ba->max_elements))
            return NULL; // TODO: should we destroy the original byte array?
    }
    memcpy(ba->mem + ba->elements_used, bytes, len);
    ba->mem[newlen] = '\0';
    ba->elements_used = newlen;
    return ba;
}

int ba_printf(bytearray *ba, const char *format, ...)
{
    /* Limitation: vsnprintf() returns an int so the max size that
     * this function can set the byte array to is limited by that
     * (also considering that we always want to be able to have space
     * for a NULL terminating character. Therefore we can't do anything more
     * than INT_MAX - 1.
     * For vsnprintf() to calculate how much room is needed, i.e. by passing
     * NULL as the destination, requires at least c99.
     */
    size_t len;
    va_list args;
    int written_count;

    assert(ba != NULL && format != NULL); // pre-conditions

    va_start(args, format);
    written_count = vsnprintf(NULL, 0, format, args);
    if (written_count < 0)
        return written_count;

    len = written_count;

    if (ba->max_elements < len) {
        if (!ba_resize(ba, len))
            return -1;
    }

    len++;  // Additional room for the '\0' has been allocated by ba_resize()

    va_start(args, format);
    written_count = vsnprintf((char *)ba->mem, len, format, args);
    if (written_count < 0)
        return written_count;
    ba->elements_used = written_count;
    return written_count;
}

void ba_hexdump(FILE *f, bytearray *ba, int bytesperline)
{
    size_t i;

    assert(f != NULL);  // pre-condition
    assert(ba != NULL); // pre-condition

    if (bytesperline < 1)
        bytesperline = 8;

    for (i = 0; i < ba->elements_used; i++) {
        fprintf(f, "%02x%c", ba->mem[i],
                !((i + 1) % bytesperline) ? '\n' : ' ' );
    }
    if (i % bytesperline)
        fprintf(f, "\n");
}

BYTE ba_at(const bytearray *ba, size_t pos)
{
    assert(ba != NULL);  // pre-condition

    if (pos >= ba->elements_used)
        return '\0';
    return ba->mem[pos];
}
