#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "bytearray.h"

#ifndef CU_BYTEARRAY_CHUNK_SIZE
#   define CU_BYTEARRAY_CHUNK_SIZE    255
#endif
#define CU_BYTEARRAY_INITIAL_MEM      CU_BYTEARRAY_CHUNK_SIZE

/* ===========================================================================
   Private functions
   =========================================================================*/

#define BA_CHUNKED_SZ(i) (((i) / CU_BYTEARRAY_CHUNK_SIZE + 1) * CU_BYTEARRAY_CHUNK_SIZE)

static struct bytearray *ba_init(struct bytearray *ba, int sz)
{
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

    return ba;
}

static bytearray *ba_resize(bytearray *ba, size_t max_elements)
{
    size_t len;

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
    ba->resize_flags = flags;
}


void ba_destroy(bytearray **ba)
{
    free((*ba)->mem);
    free(*ba);
    *ba = NULL;
}

bool ba_isfull(const bytearray *ba)
{
    return ba->elements_used >= ba->max_elements;
}

size_t ba_max_elements(const bytearray *ba)
{
    return ba->max_elements;
}

size_t ba_size(const bytearray *ba)
{
    return ba->elements_used;
}

const char *ba_cstr(const bytearray *ba)
{
    return (const char *)ba->mem;
}

bytearray *ba_clear(bytearray *ba)
{
    if (ba->mem) {
        ba->mem[0] = '\0';
        ba->elements_used = 0;
    }
    return ba;
}

bytearray *ba_shrinktofit(bytearray *ba)
{
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
    return ba_set(ba, (const BYTE *)str, strlen(str));
}

bytearray *ba_set(bytearray *ba, const BYTE *from, size_t len)
{
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
    return ba_append(ba, (const BYTE *)str, strlen(str));
}

bytearray *ba_append(bytearray *ba, const BYTE *bytes, size_t len)
{
    size_t newlen;

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

void ba_hexdump(FILE *f, bytearray *ba, int bytesperline)
{
    size_t i;

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
    if (pos >= ba->elements_used)
        return '\0';
    return ba->mem[pos];
}
