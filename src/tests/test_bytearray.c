#include <stdio.h>
#include "../bytearray.h"
#include "../types.h"

void test_bytearray(void)
{
    bytearray *ba;
    BYTE arr[] = { '1', '2', '\0', 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

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

    ba_destroy(&ba);
    ba_destroy(&ba);
}

void look_and_say(const char *seed_str, int terms)
{
    bytearray *ba1, *ba2, *src, *dest, *tmp;

    if (!seed_str || *seed_str == '\0')
        return;

    ba1 = ba_new(-1);
    ba2 = ba_new(-1);
    if (!ba1 || !ba2)
        goto end;

    ba_set_from_cstr(ba1, seed_str);
    src = ba1; dest = ba2;

    printf("%s\n", seed_str);

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
        printf("%s\n", ba_cstr(dest));

        tmp = src; src = dest; dest = tmp;
    }

end:
    ba_destroy(&ba1);
    ba_destroy(&ba2);
}
