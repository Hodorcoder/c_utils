#ifndef CU_INCLUDE_STRING_H
#define CU_INCLUDE_STRING_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "types.h"

#define CUSTR_RESIZE_UP_EXACT       (1U << 0)
#define CUSTR_RESIZE_UP_CHUNKED     (1U << 1)

#ifndef CUSTR_DEFAULT_CHUNK_SIZE
#   define CUSTR_DEFAULT_CHUNK_SIZE    255
#endif
#define CUSTR_DEFAULT_INITIAL_MEM      CUSTR_DEFAULT_CHUNK_SIZE

typedef struct cuStr {
    size_t max_elements;
    size_t elements_used;
    char *mem;
    unsigned resize_flags;
    unsigned chunk_size;
} cuStr;

cuStr *cuStr_new(int sz);
cuStr *cuStr_copy(const cuStr *cus);
void cuStr_set_chunksize (cuStr *cus, unsigned sz);
size_t cuStr_chunksize(const cuStr *cus);
void cuStr_set_resize_strategy(cuStr *cus, unsigned flags);
void cuStr_destroy(cuStr **cus);
bool cuStr_isfull(const cuStr *cus);
size_t cuStr_max_elements(const cuStr *cus);
size_t cuStr_len(const cuStr *cus);
const char *cuStr_cstr(const cuStr *cus);
cuStr *cuStr_clear(cuStr *cus);
cuStr *cuStr_shrinktofit(cuStr *cus);
cuStr *cuStr_set(cuStr *cus, const char *str);
cuStr *cuStr_set_fromarray(cuStr *cus, const char *from, unsigned len);
cuStr *cuStr_append(cuStr *cus, const char *str);
cuStr *cuStr_append_array(cuStr *cus, const char *arr, unsigned len);
int cuStr_printf(cuStr *cus, const char *format, ...);
int cuStr_printf_append(cuStr *cus, const char *format, ...);
void cuStr_hexdump(FILE *f, cuStr *cus, int bytesperline);
char cuStr_at(const cuStr *cus, unsigned pos);
int cuStr_strcmp(const cuStr *cus1, const cuStr *cus2);
int cuStr_strcmp_cstr(const cuStr *cus, const char *s);
int cuStr_cmp(const cuStr *cus1, const cuStr *cus2);
void cuStr_rotate(cuStr *cus, int shift, unsigned n);

#endif /* CU_INCLUDE_STRING_H */
