#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include "bytearray.h"

/* ===========================================================================
   Private functions
   =========================================================================*/

/* i is the amount (before chunked) requested
 * z is the chunck size
 */
#define BA_CHUNKED_SZ(i, z) (((i) / (z) + 1) * z)

// TODO: FIXME: Move to a file (maybe cu_math) of its own
int gcd(int a, int b)
{
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;

    while (b) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static struct bytearray *ba_init(struct bytearray *ba, int sz, bool use_exact_sz)
{
    assert(ba != NULL); // pre-condition

    ba->elements_used = 0;

    if (sz == 0) {
        ba->mem = NULL;
        ba->max_elements = 0;
    }
    else {
        if (!use_exact_sz || sz < 1) {
            sz = sz == -1 ? CU_DEFAULT_BYTEARRAY_INITIAL_MEM
                          : BA_CHUNKED_SZ(sz, CU_DEFAULT_BYTEARRAY_CHUNK_SIZE);
        }

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
    }

    ba_set_chunksize(ba, CU_DEFAULT_BYTEARRAY_CHUNK_SIZE);
    return ba;
}

static bytearray *ba_resize(bytearray *ba, unsigned max_elements)
{
    size_t len;

    assert(ba != NULL); // pre-condition

    if (max_elements == ba->max_elements) {
        return ba;
    } else if (max_elements > ba->max_elements) {
        if (ba->resize_flags & BA_RESIZE_UP_CHUNKED) {
            len = BA_CHUNKED_SZ(max_elements, ba->chunk_size);
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

static void ba_rotate_left(bytearray *ba, int shift, int n)
{
    int i, j, k;
    int g_cd;
    char tmp;

    unsigned char *arr = ba->mem;

    g_cd = gcd(shift, n);
    for (i = 0; i < g_cd; i++) {
        tmp = arr[i];
        j = i;
        while (1) {
            k = j + shift;
            if (k >= n)
                k -= n;
            if (k == i)
                break;
            arr[j] = arr[k];
            j = k;
        }
        arr[j] = tmp;
    }
}

static void ba_rotate_right(bytearray *ba, int shift, int n)
{
    int i, j, k;
    int g_cd;
    char tmp;

    unsigned char *arr = ba->mem;

    g_cd = gcd(shift, n);
    for (i = 0; i < g_cd; i++) {
        tmp = arr[i];
        j = i;
        while (1) {
            k = j - shift;
            if (k < 0)
                k += n;
            if (k == i)
                break;
            arr[j] = arr[k];
            j = k;
        }
        arr[j] = tmp;
    }
}

/* ===========================================================================
   Public functions
   =========================================================================*/

bytearray *ba_new(int sz)
{
    struct bytearray *ba;

    if ((ba = malloc(sizeof *ba)) != NULL) {
        if (!ba_init(ba, sz, false)) {
            free(ba);
            ba = NULL;
        }
    }

    return ba;
}

bytearray *ba_copy(const bytearray *ba)
{
    bytearray *ba_copy;

    assert(ba != NULL); // pre-condition

    if ((ba_copy = malloc(sizeof *ba_copy)) != NULL) {
        /* copy everything except the memory pointer to ensure that the copy
         * has the same flags, chunk size etc as the original
         */
        memcpy(ba_copy, ba, sizeof (*ba_copy));
        ba_copy->mem = NULL;
        if (!ba_init(ba_copy, ba->max_elements, true)) {
            free(ba_copy);
            ba_copy = NULL;
        }

        memcpy(ba_copy->mem, ba->mem, ba->elements_used + 1);
        ba_copy->elements_used = ba->elements_used;
    }

    return ba_copy;
}

void ba_set_chunksize (bytearray *ba, unsigned sz)
{
    if (sz == 0)
        sz = 1;
    ba->chunk_size = sz;
    if (sz == 1) {
        ba->resize_flags |= BA_RESIZE_UP_EXACT;
        ba->resize_flags &= ~BA_RESIZE_UP_CHUNKED;
    } else {
        ba->resize_flags &= ~BA_RESIZE_UP_EXACT;
        ba->resize_flags |= BA_RESIZE_UP_CHUNKED;
    }
}

size_t ba_chunksize(const bytearray *ba)
{
    return ba->chunk_size;
}

void ba_set_resize_strategy(bytearray *ba, unsigned flags)
{
    assert(ba != NULL); // pre-condition

    ba->resize_flags = flags;
}


void ba_destroy(bytearray **ba)
{
    assert(ba != NULL); // pre-condition

    if (*ba) {
        free((*ba)->mem);
        free(*ba);
    }
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

bytearray *ba_set(bytearray *ba, const BYTE *from, unsigned len)
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

bytearray *ba_append(bytearray *ba, const BYTE *bytes, unsigned len)
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
    va_end(args);

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
    va_end(args);
    return written_count;
}

int ba_printf_append(bytearray *ba, const char *format, ...)
{
    size_t len;
    va_list args;
    int written_count;

    assert(ba != NULL && format != NULL); // pre-conditions

    va_start(args, format);
    written_count = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (written_count < 0)
        return written_count;

    len = written_count + ba->elements_used;

    if (ba->max_elements < len) {
        if (!ba_resize(ba, len))
            return -1;
    }

    len++;  // Additional room for the '\0' has been allocated by ba_resize()

    va_start(args, format);
    written_count = vsnprintf((char *)ba->mem + ba->elements_used, len,
                              format, args);
    if (written_count < 0)
        return written_count;
    ba->elements_used += written_count;
    va_end(args);
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

int ba_strcmp(const bytearray *ba1, const bytearray *ba2)
{
    assert (ba1 != NULL && ba2 != NULL); // pre-conditions

    return strcmp((const char*)ba1->mem, (const char*)ba2->mem);
}

int ba_strcmp_cstr(const bytearray *ba, const char *s)
{
    assert (ba != NULL && s != NULL); // pre-conditions

    return strcmp((const char*)ba->mem, s);
}

int ba_cmp(const bytearray *ba1, const bytearray *ba2)
{
    assert (ba1 != NULL && ba2 != NULL); // pre-conditions
    size_t eu1 = ba1->elements_used,
           eu2 = ba2->elements_used;
    if (eu1 != eu2)
        return eu1 < eu2 ? -1 : 1;

    return memcmp(ba1->mem, ba2->mem, ba1->elements_used);
}

void ba_rotate(bytearray *ba, int shift, int n)
{
    bool go_left;

    if (ba->elements_used < 2)
        return;

    if (n == -1)
        n = ba->elements_used;
    else if (n > ba->elements_used)
        n = ba->elements_used;

    /* Rotating 0 or 1 characters of the total sequence makes no sense because
     * the result would be the same as no rotatation at all for any value of
     * 'shift'
     */
    if (n < 2)
        return;

    go_left = shift < 0;

    /* No point rotating more than we need to so limit shift.
     */
    shift %= n;

    if (go_left)
        ba_rotate_left(ba, -shift, n);
    else
        ba_rotate_right(ba, shift, n);
}
