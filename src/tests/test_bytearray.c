#include <stdio.h>
#include "test_bytearray.h"
#include "../types.h"

#ifndef NDEBUG
void test_bytearray(void)
{
    bytearray *ba, *ba2;
    int r;
    static const char *result[] = { "FAILED", "Ok"};
    const BYTE arr[] = { '1', '2', '\0', 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    ba = look_and_say("3", 10);
    if (!ba) {
        printf("ba memory allocation failure. Aborting tests\n");
        return;
    }
    r = ba_strcmp_cstr(ba, "132113213221133112132123222113") == 0;
    printf("Byte array, look-and-see algorithm: %s\n", result[r]);

    ba_destroy(&ba);
    printf("Byte array, ba_destroy(): %s\n", result[ba == NULL]);
    ba = ba_new(0);
    if (!ba) {
        printf("ba_new() failed. Aborting tests\n");
        return;
    }
    printf("Byte array, ba_isfull() %s\n", result[ba_isfull(ba)]);
    ba_destroy(&ba);
    ba = ba_new(-1);
    if (!ba) {
        printf("ba_new() failed. Aborting tests\n");
        return;
    }
    printf("Byte array, ba_isfull() (test 2) %s\n", result[!ba_isfull(ba)]);

    ba_set(ba, arr, sizeof (arr) / sizeof (arr[0]));
    ba2 = ba_new(-1);
    ba_set(ba2, arr, sizeof (arr) / sizeof (arr[0]));
    printf ("Comparing two byte arrays for equality: %s\n", result[ba_cmp(ba, ba2) == 0]);
    ba_clear(ba2);
    printf ("Comparing two byte arrays for equality: %s\n", result[ba_cmp(ba, ba2) != 0]);
    ba_destroy(&ba2);

#if 0
    ba = ba_new(0);
    if (ba) {
        if (ba_isfull(ba)) {
            printf("Byte array is full\n");
        }
        printf(" 1) ba can hold %zu elements\n", ba_max_elements(ba));
        printf(" 2) ba set to '%s'\n", ba_cstr(ba));
        ba_set_from_cstr(ba, "12");
        ba_shrinktofit(ba);
        ba_set_chunksize(ba, 1);
        printf("2a) chunk size set to 1 (exact)\n");
        printf("2b) ba can hold %zu elements\n", ba_max_elements(ba));
        printf("2c) ba set to \"%s\"\n", ba_cstr(ba));
        printf("2d) size (strlen) is %zu\n", ba_size(ba));
        printf("Setting new string:\n");
        ba_set_from_cstr(ba, "Hello");
        printf(" 3) ba can hold %zu elements\n", ba_max_elements(ba));
        printf(" 4) ba set to \"%s\"\n", ba_cstr(ba));
        printf(" 5) size (strlen) is %zu\n", ba_size(ba));
        printf("Shrink:\n");
        ba_shrinktofit(ba);
        ba_set_chunksize(ba, 12);
        printf("5a) chunk size set to %zu (chunked)\n", ba_chunksize(ba));
        printf("5b) ba can hold %zu elements\n", ba_max_elements(ba));
        ba_append_cstr(ba, " world");
        printf("Appended string:\n");
        printf(" 6) ba can hold %zu elements\n", ba_max_elements(ba));
        printf(" 7) ba set to \"%s\"\n", ba_cstr(ba));
        printf(" 8) size (strlen) is %zu\n", ba_size(ba));
        printf("Shrink:\n");
        ba_shrinktofit(ba);
        printf(" 9) ba can hold %zu elements\n", ba_max_elements(ba));
        printf("10) ba set to \"%s\"\n", ba_cstr(ba));
        printf("11) size is %zu\n", ba_size(ba));
        printf("Setting from array:\n");
        ba_set(ba, arr, sizeof (arr) / sizeof (arr[0]));
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("ba as a string is \"%s\"\n", ba_cstr(ba));
        printf("size is %zu\n", ba_size(ba));
        printf("Hex dump:\n");
        ba_hexdump(stdout, ba, 4);
        printf("Byte at index 7 is %02x\n", ba_at(ba, 7));
        printf("Clearing:\n");
        ba_clear(ba);
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("Shrinking:\n");
        ba_shrinktofit(ba);
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("Appending array (instead of set):\n");
        ba_append(ba, arr, sizeof (arr) / sizeof (arr[0]));
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("ba as a string is '%s'\n", ba_cstr(ba));
        printf("size is %zu\n", ba_size(ba));
        printf("Hex dump:\n");
        ba_hexdump(stdout, ba, 4);

        ba_clear(ba);
        ba_shrinktofit(ba);
        ba_printf(ba, "%d", -12345678);
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("ba as a string is \"%s\"\n", ba_cstr(ba));
        printf("size is %zu\n", ba_size(ba));

        printf("printf append:\n");
        ba_printf_append(ba, ".%d ABCDEFGHIJKLMNOPQRS", 7654321);
        printf("ba can hold %zu elements\n", ba_max_elements(ba));
        printf("ba as a string is \"%s\"\n", ba_cstr(ba));
        printf("size is %zu\n", ba_size(ba));
        printf("-----------------------\n");


    }
#endif
    ba_destroy(&ba);
    ba_destroy(&ba);    // destroy a second time (this must be valid)
}

bytearray *look_and_say(const char *seed_str, int terms)
{
    bytearray *ba1, *ba2, *src, *dest, *tmp;

    if (!seed_str || *seed_str == '\0')
        return NULL;

    ba1 = ba_new(-1);
    ba2 = ba_new(-1);
    if (!ba1 || !ba2)
        goto end;

    ba_set_from_cstr(ba1, seed_str);
    src = ba1; dest = ba2;

    //printf("%s\n", seed_str);

    for (int i = 0; i < terms; i++) {
        int count = 1;
        char search_char = ba_at(src, 0);
        ba_clear(dest);
        for (int j = 1; search_char != '\0'; j++) {
            char ch = ba_at(src, j);
            if (ch == search_char)
                count++;
            else {
                ba_printf_append(dest, "%d%c", count, search_char);
                count = 1;
                search_char = ch;
            }
        }
        //printf("%s\n", ba_cstr(dest));

        tmp = src; src = dest; dest = tmp;
    }
    return src;

end:
    ba_destroy(&ba1);
    ba_destroy(&ba2);

    return NULL;
}
#endif // NDEBUG
