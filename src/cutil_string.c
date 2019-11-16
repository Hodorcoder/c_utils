#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include "cutil_string.h"
#include "cutil_math.h"

const char *empty_str = "";

/* ===========================================================================
   Private functions
   =========================================================================*/

/* i is the amount (before chunked) requested
 * z is the chunck size
 */
#define cuStrCHUNKED_SZ(i, z) (((i) / (z) + 1) * z)

static struct cuStr *cuStr_init(struct cuStr *cus, int sz, bool use_exact_sz)
{
    assert(cus != NULL); // pre-condition

    cus->elements_used = 0;

    if (sz == 0) {
        cus->mem = NULL;
        cus->max_elements = 0;
    }
    else {
        if (!use_exact_sz || sz < 1) {
            sz = sz == -1 ? CUSTR_DEFAULT_INITIAL_MEM
                          : cuStrCHUNKED_SZ(sz, CUSTR_DEFAULT_CHUNK_SIZE);
        }

        /* Allocate memory, always with an extra element for '\0'
         */
        if ((cus->mem = malloc((sz + 1) * sizeof cus->mem)) == NULL) {
            cus->max_elements = 0;
            return NULL;
        }

        /* Always NUL terminate so a call to cuStrcstr(), for example, won't
         * return a non-terminated string if the byte array hasn't been
         * populated with anything.
         */
        cus->mem[0] = '\0';
        cus->max_elements = sz;
    }

    cuStr_set_chunksize(cus, CUSTR_DEFAULT_CHUNK_SIZE);
    return cus;
}

static cuStr *cuStr_dealloc_mem(cuStr *cus)
{
    free(cus->mem);
    cus->mem = NULL;
    cus->max_elements = cus->elements_used = 0;
    return cus;
}

static cuStr *cuStr_resize(cuStr *cus, unsigned max_elements)
{
    size_t len;

    assert(cus != NULL); // pre-condition

    if (max_elements == cus->max_elements) {
        return cus;
    } else if (max_elements == 0) {
        return cuStr_dealloc_mem(cus);
    } else if (max_elements > cus->max_elements) {
        if (cus->resize_flags & CUSTR_RESIZE_UP_CHUNKED) {
            len = cuStrCHUNKED_SZ(max_elements, cus->chunk_size);
        } else {
            assert(cus->resize_flags & CUSTR_RESIZE_UP_EXACT);
            len = max_elements;
        }
    } else {
        len = max_elements;
    }

    len += 1;   // Always allow room for '\0'

    char *new_mem = realloc(cus->mem, len);
    if (!new_mem)
        return NULL;
    cus->mem = new_mem;
    cus->max_elements = len - 1; // -1 because there is extra space for '\0'
    return cus;
}

inline static void cuStr_rotate_lr(cuStr *cus, int shift, int n)
{
    int i;
    int g_cd;

    char *arr = cus->mem;

    assert(arr != NULL);

    /* in-place "juggle" algorithm for rotation
     */

    g_cd = gcd(shift, n);
    for (i = 0; i < g_cd; i++) {
        int j;
        char tmp = arr[i];
        j = i;
        while (1) {
            int k = j - shift;
            if (shift < 0) {
                if (k >= n)
                    k -= n;
            } else {
                if (k < 0)
                    k += n;
            }
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

cuStr *cuStr_new(int sz)
{
    struct cuStr *cus;

    if ((cus = malloc(sizeof *cus)) != NULL) {
        if (!cuStr_init(cus, sz, false)) {
            free(cus);
            cus = NULL;
        }
    }

    return cus;
}

cuStr *cuStr_copy(const cuStr *cus)
{
    cuStr *cuStrcopy;

    assert(cus != NULL); // pre-condition

    if ((cuStrcopy = malloc(sizeof *cuStrcopy)) != NULL) {
        /* copy everything except the memory pointer to ensure that the copy
         * has the same flags, chunk size etc as the original
         */
        memcpy(cuStrcopy, cus, sizeof (*cuStrcopy));
        cuStrcopy->mem = NULL;
        if (!cuStr_init(cuStrcopy, cus->max_elements, true)) {
            free(cuStrcopy);
            cuStrcopy = NULL;
            return NULL;
        }

        if (cus->mem == NULL) {
            assert(cus->elements_used == 0);
        } else {
            memcpy(cuStrcopy->mem, cus->mem, cus->elements_used + 1);
            cuStrcopy->elements_used = cus->elements_used;
        }
    }

    return cuStrcopy;
}

void cuStr_set_chunksize (cuStr *cus, unsigned sz)
{
    if (sz == 0)
        sz = 1;
    cus->chunk_size = sz;
    if (sz == 1) {
        cus->resize_flags |= CUSTR_RESIZE_UP_EXACT;
        cus->resize_flags &= ~CUSTR_RESIZE_UP_CHUNKED;
    } else {
        cus->resize_flags &= ~CUSTR_RESIZE_UP_EXACT;
        cus->resize_flags |= CUSTR_RESIZE_UP_CHUNKED;
    }
}

size_t cuStr_chunksize(const cuStr *cus)
{
    return cus->chunk_size;
}

void cuStrset_resize_strategy(cuStr *cus, unsigned flags)
{
    assert(cus != NULL); // pre-condition

    cus->resize_flags = flags;
}


void cuStr_destroy(cuStr **cus)
{
    assert(cus != NULL); // pre-condition

    if (*cus) {
        free((*cus)->mem);
        free(*cus);
    }
    *cus = NULL;
}

bool cuStr_isfull(const cuStr *cus)
{
    assert(cus != NULL); // pre-condition
    return cus->elements_used >= cus->max_elements;
}

size_t cuStr_max_elements(const cuStr *cus)
{
    assert(cus != NULL); // pre-condition
    return cus->max_elements;
}

size_t cuStr_len(const cuStr *cus)
{
    assert(cus != NULL); // pre-condition
    return cus->elements_used;
}

const char *cuStr_cstr(const cuStr *cus)
{
    assert(cus != NULL); // pre-condition
    return cus->mem ? cus->mem : empty_str;
}

cuStr *cuStr_clear(cuStr *cus)
{
    assert(cus != NULL); // pre-condition

    if (cus->mem) {
        cus->mem[0] = '\0';
    }
    cus->elements_used = 0;

    return cus;
}

cuStr *cuStr_shrinktofit(cuStr *cus)
{
    assert(cus != NULL); // pre-condition

    if (cus->max_elements > cus->elements_used) {
        char *newmem;

        size_t newlen = cus->elements_used;
        if (newlen == 0) {
            return cuStr_dealloc_mem(cus);
        }

        newlen++; // Room for \0
        if ((newmem = realloc(cus->mem, newlen)) == NULL) {
            return NULL;    // TODO: Indicate error somehow
        }
        cus->mem = newmem;
        newlen--;   // reserve the '\0' space
        cus->max_elements = newlen;
        cus->elements_used = newlen;
        cus->mem[newlen] = '\0'; // make sure it's still null terminated
    }
    return cus;
}

cuStr *cuStr_set(cuStr *cus, const char *str)
{
    assert(cus != NULL);  // pre-condition
    assert(str != NULL); // pre-condition

    return cuStr_set_fromarray(cus, str, strlen(str));
}

cuStr *cuStr_set_fromarray(cuStr *cus, const char *from, unsigned len)
{
    assert(cus != NULL);   // pre-condition
    assert(from != NULL); // pre-condition

    if (len > cus->max_elements) {
        cuStr *tmp = cuStr_resize(cus, len);
        if (!tmp)
            return NULL; // TODO: should we destroy the original byte array?
    }
    memcpy(cus->mem, from, len);
    cus->elements_used = len;
    cus->mem[len] = '\0';
    return cus;
}

cuStr *cuStr_append(cuStr *cus, const char *str)
{
    assert(cus != NULL);  // pre-condition
    assert(str != NULL); // pre-condition

    return cuStr_append_array(cus, str, strlen(str));
}

cuStr *cuStr_append_array(cuStr *cus, const char *arr, unsigned len)
{
    size_t newlen;

    assert(cus != NULL);    // pre-condition
    assert(arr != NULL); // pre-condition

    newlen = len + cus->elements_used;
    if (newlen > cus->max_elements) {
        if (!cuStr_resize(cus, len + cus->max_elements))
            return NULL; // TODO: should we destroy the original byte array?
    }
    memcpy(cus->mem + cus->elements_used, arr, len);
    cus->mem[newlen] = '\0';
    cus->elements_used = newlen;
    return cus;
}

int cuStr_printf(cuStr *cus, const char *format, ...)
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

    assert(cus != NULL && format != NULL); // pre-conditions

    va_start(args, format);
    written_count = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (written_count < 0)
        return written_count;

    len = written_count;

    if (cus->max_elements < len) {
        if (!cuStr_resize(cus, len))
            return -1;
    }

    len++;  // Additional room for the '\0' has been allocated by cuStr_resize()

    va_start(args, format);
    written_count = vsnprintf((char *)cus->mem, len, format, args);
    if (written_count < 0)
        return written_count;
    cus->elements_used = written_count;
    va_end(args);
    return written_count;
}

int cuStr_printf_append(cuStr *cus, const char *format, ...)
{
    size_t len;
    va_list args;
    int written_count;

    assert(cus != NULL && format != NULL); // pre-conditions

    va_start(args, format);
    written_count = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (written_count < 0)
        return written_count;

    len = written_count + cus->elements_used;

    if (cus->max_elements < len) {
        if (!cuStr_resize(cus, len))
            return -1;
    }

    len++;  // Additional room for the '\0' has been allocated by cuStr_resize()

    va_start(args, format);
    written_count = vsnprintf((char *)cus->mem + cus->elements_used, len,
                              format, args);
    if (written_count < 0)
        return written_count;
    cus->elements_used += written_count;
    va_end(args);
    return written_count;
}

void cuStr_hexdump(FILE *f, cuStr *cus, int bytesperline)
{
    size_t i;

    assert(f != NULL);  // pre-condition
    assert(cus != NULL); // pre-condition

    if (bytesperline < 1)
        bytesperline = 8;

    for (i = 0; i < cus->elements_used; i++) {
        fprintf(f, "%02x%c", cus->mem[i],
                !((i + 1) % bytesperline) ? '\n' : ' ' );
    }
    if (i % bytesperline)
        fprintf(f, "\n");
}

char cuStr_at(const cuStr *cus, unsigned pos)
{
    assert(cus != NULL);  // pre-condition

    if (pos >= cus->elements_used)
        return '\0';
    return cus->mem[pos];
}

int cuStr_strcmp(const cuStr *cus1, const cuStr *cus2)
{
    assert (cus1 != NULL && cus2 != NULL); // pre-conditions

    if (cus1->mem == NULL || cus2->mem == NULL) {
        return cus1->elements_used - cus2->elements_used;   // FIXME: Check order
    }

    return strcmp(cus1->mem, (const char*)cus2->mem);
}

int cuStr_strcmp_cstr(const cuStr *cus, const char *s)
{
    assert (cus != NULL && s != NULL); // pre-conditions

    if (cus->mem == NULL) {
        return -1;          // FIXME: Check order
    }

    return strcmp(cus->mem, s);
}

int cuStr_cmp(const cuStr *cus1, const cuStr *cus2)
{
    assert (cus1 != NULL && cus2 != NULL); // pre-conditions
    size_t eu1 = cus1->elements_used,
           eu2 = cus2->elements_used;
    if (eu1 != eu2)
        return eu1 < eu2 ? -1 : 1;  // FIXME: Check order

    if (cus1->mem == NULL || cus2->mem == NULL) {
        return cus1->elements_used - cus2->elements_used;   // FIXME: Check order
    }

    return memcmp(cus1->mem, cus2->mem, cus1->elements_used);
}

void cuStr_rotate(cuStr *cus, int shift, unsigned n)
{
    bool go_left;

    if (cus->elements_used < 2)
        return;

    if (n > cus->elements_used)
        n = cus->elements_used;

    /* Rotating 0 or 1 characters of the total sequence makes no sense because
     * the result would be the same as no rotatation at all for any value of
     * 'shift'
     */
    if (n < 2)
        return;

    if (shift < 0) {
        go_left = 1;
        shift = -shift;
    } else {
        go_left = 0;
    }

    shift %= n;
    if (shift == 0)
        return;

    if (go_left)
        cuStr_rotate_lr(cus, -shift, n);
    else
        cuStr_rotate_lr(cus, shift, n);
}
