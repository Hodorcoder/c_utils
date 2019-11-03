#ifndef CU_INCLUDE_BYTEARRAY_H
#define CU_INCLUDE_BYTEARRAY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "types.h"

#define BA_RESIZE_UP_EXACT       (1U << 0)
#define BA_RESIZE_UP_CHUNKED     (1U << 1)
#define BA_RESIZE_DN_EXACT       (1U << 2)    /* TODO: Implement */
#define BA_RESIZE_DN_CHUNKED     (1U << 3)    /* TODO: Implement */

#ifndef CU_DEFAULT_BYTEARRAY_CHUNK_SIZE
#   define CU_DEFAULT_BYTEARRAY_CHUNK_SIZE    255
#endif
#define CU_DEFAULT_BYTEARRAY_INITIAL_MEM      CU_DEFAULT_BYTEARRAY_CHUNK_SIZE

typedef struct bytearray {
    size_t max_elements;
    size_t elements_used;
    BYTE *mem;
    unsigned resize_flags;
    unsigned chunk_size;
} bytearray;

bytearray *ba_new(int sz);
void ba_set_chunksize (bytearray *ba, unsigned sz);
size_t ba_chunksize(const bytearray *ba);
void ba_set_resize_strategy(bytearray *ba, unsigned flags);
void ba_destroy(bytearray **ba);
bool ba_isfull(const bytearray *ba);
size_t ba_max_elements(const bytearray *ba);
size_t ba_size(const bytearray *ba);
const char *ba_cstr(const bytearray *ba);
bytearray *ba_clear(bytearray *ba);
bytearray *ba_shrinktofit(bytearray *ba);
bytearray *ba_set_from_cstr(bytearray *ba, const char *str);
bytearray *ba_set(bytearray *ba, const BYTE *from, unsigned len);
bytearray *ba_append_cstr(bytearray *ba, const char *str);
bytearray *ba_append(bytearray *ba, const BYTE *bytes, unsigned len);
int ba_printf(bytearray *ba, const char *format, ...);
int ba_printf_append(bytearray *ba, const char *format, ...);
void ba_hexdump(FILE *f, bytearray *ba, int bytesperline);
BYTE ba_at(const bytearray *ba, size_t pos);
int ba_cstr_cmp(const bytearray *ba1, const bytearray *ba2);
int ba_cstr_cmp_to_cstr(const bytearray *ba, const char *s);

// TODO: FIXME: Implement memcmp functions (because mem can contain \0)

#endif /* CU_INCLUDE_BYTEARRAY_H */
