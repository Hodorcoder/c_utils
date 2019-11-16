#include <stdio.h>
#include "test_string.h"
#include "../types.h"

#ifndef NDEBUG
void test_bytearray(void)
{
    cuStr *cus, *cus2;
    int r;
    static const char *result[] = { "FAILED", "Ok"};
    const char arr[] = { '1', '2', '\0', 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    cus = look_and_say("3", 10); // with seed of "3", results in A006715
    if (!cus) {
        printf("cus memory allocation failure. Aborting tests\n");
        return;
    }
    r = cuStr_strcmp_cstr(cus, "132113213221133112132123222113") == 0;
    printf("Byte array, look-and-see algorithm: %s\n", result[r]);

    cus2 = cuStr_copy(cus);
    printf("cuStr_copy #1: %s\n", result[cuStr_cmp(cus, cus2) == 0]);

    cuStr_destroy(&cus2);
    cuStr_clear(cus);
    cuStr_shrinktofit(cus);
    cus2 = cuStr_copy(cus);
    printf("cuStr_copy #2: %s\n", result[cuStr_cmp(cus, cus2) == 0]);

    cuStr_destroy(&cus);
    cuStr_destroy(&cus2);

    printf("cuStr_destroy(): %s\n", result[cus == NULL]);
    cus = cuStr_new(0);
    if (!cus) {
        printf("cuStr_new() failed. Aborting tests\n");
        return;
    }
    printf("cuStr_isfull() %s\n", result[cuStr_isfull(cus)]);
    cuStr_destroy(&cus);
    cus = cuStr_new(-1);
    if (!cus) {
        printf("cuStr_new() failed. Aborting tests\n");
        return;
    }
    printf("cuStr_isfull() (test 2) %s\n", result[!cuStr_isfull(cus)]);

    cuStr_set_fromarray(cus, arr, sizeof (arr) / sizeof (arr[0]));
    cus2 = cuStr_new(-1);
    cuStr_set_fromarray(cus2, arr, sizeof (arr) / sizeof (arr[0]));
    printf ("Comparing for equality: %s\n", result[cuStr_cmp(cus, cus2) == 0]);
    cuStr_clear(cus2);
    printf ("Comparing for equality: %s\n", result[cuStr_cmp(cus, cus2) != 0]);



    cuStr_set(cus, "ab");
    cuStr_rotate(cus, -3, cus->elements_used);
    printf("Rotate test 1: (result %s) %s\n", cuStr_cstr(cus), result[cuStr_strcmp_cstr(cus, "ba") == 0]);

    cuStr_set(cus, "hello");
    cuStr_rotate(cus, -2, 3);
    printf("Rotate test 2: (result %s) %s\n", cuStr_cstr(cus), result[cuStr_strcmp_cstr(cus, "lhelo") == 0]);

    cuStr_set(cus, "abc");
    cuStr_printf_append(cus, "def");
    cuStr_rotate(cus, 2, 3);
    printf("Rotate test 3: (result %s) %s\n", cuStr_cstr(cus), result[cuStr_strcmp_cstr(cus, "bcadef") == 0]);

    cuStr_set(cus, "abc");
    cuStr_rotate(cus, 9, cus->elements_used);
    printf("Rotate test 4: (result %s) %s\n", cuStr_cstr(cus), result[cuStr_strcmp_cstr(cus, "abc") == 0]);

#if 0
    cuStr_destroy(&cus);
    cus = cuStr_new(0);
    if (cus) {
        if (cuStr_isfull(cus)) {
            printf("Byte array is full\n");
        }
        printf(" 1) cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf(" 2) cus set to '%s'\n", cuStr_cstr(cus));
        cuStr_set(cus, "12");
        cuStr_shrinktofit(cus);
        cuStr_set_chunksize(cus, 1);
        printf("2a) chunk size set to 1 (exact)\n");
        printf("2b) cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("2c) cus set to \"%s\"\n", cuStr_cstr(cus));
        printf("2d) size (strlen) is %zu\n", cuStr_len(cus));
        printf("Setting new string:\n");
        cuStr_set(cus, "Hello");
        printf(" 3) cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf(" 4) cus set to \"%s\"\n", cuStr_cstr(cus));
        printf(" 5) size (strlen) is %zu\n", cuStr_len(cus));
        printf("Shrink:\n");
        cuStr_shrinktofit(cus);
        cuStr_set_chunksize(cus, 12);
        printf("5a) chunk size set to %zu (chunked)\n", cuStr_chunksize(cus));
        printf("5b) cus can hold %zu elements\n", cuStr_max_elements(cus));
        cuStr_append(cus, " world");
        printf("Appended string:\n");
        printf(" 6) cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf(" 7) cus set to \"%s\"\n", cuStr_cstr(cus));
        printf(" 8) size (strlen) is %zu\n", cuStr_len(cus));
        printf("Shrink:\n");
        cuStr_shrinktofit(cus);
        printf(" 9) cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("10) cus set to \"%s\"\n", cuStr_cstr(cus));
        printf("11) size is %zu\n", cuStr_len(cus));
        printf("Setting from array:\n");
        cuStr_set_fromarray(cus, arr, sizeof (arr) / sizeof (arr[0]));
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is \"%s\"\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));
        printf("Hex dump:\n");
        cuStr_hexdump(stdout, cus, 4);
        printf("Byte at index 7 is %02x\n", cuStr_at(cus, 7));
        printf("Clearing:\n");
        cuStr_clear(cus);
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("Shrinking:\n");
        cuStr_shrinktofit(cus);
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("Appending array (instead of using set):\n");
        cuStr_append_array(cus, arr, sizeof (arr) / sizeof (arr[0]));
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is '%s'\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));
        printf("Hex dump:\n");
        cuStr_hexdump(stdout, cus, 4);

        cuStr_clear(cus);
        cuStr_shrinktofit(cus);
        cuStr_printf(cus, "%d", -12345678);
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is \"%s\"\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));

        printf("printf append:\n");
        cuStr_printf_append(cus, ".%d ABCDEFGHIJKLMNOPQRS", 7654321);
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is \"%s\"\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));
        printf("-----------------------\n");

        printf("printf append:\n");
        cuStr_printf_append(cus, "%s", "TUVWXYZ");
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is \"%s\"\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));
        printf("-----------------------\n");

        printf("Force memory dealloc:\n");
        cuStr_clear(cus);
        cuStr_shrinktofit(cus);
        printf("cus can hold %zu elements\n", cuStr_max_elements(cus));
        printf("cus as a string is \"%s\"\n", cuStr_cstr(cus));
        printf("size is %zu\n", cuStr_len(cus));
        printf("-----------------------\n");

    }
#endif
    cuStr_clear(cus);
    cuStr_shrinktofit(cus);
    cuStr_destroy(&cus);
    cuStr_destroy(&cus);    // destroy a second time (this must be valid)
    cuStr_destroy(&cus2);
}

cuStr *look_and_say(const char *seed_str, int terms)
{
    cuStr *cus1, *cus2, *src, *dest, *tmp;

    if (!seed_str || *seed_str == '\0')
        return NULL;

    cus1 = cuStr_new(-1);
    cus2 = cuStr_new(-1);
    if (!cus1 || !cus2)
        goto end;

    cuStr_set(cus1, seed_str);
    src = cus1; dest = cus2;

    //printf("%s\n", seed_str);

    for (int i = 0; i < terms; i++) {
        int count = 1;
        char search_char = cuStr_at(src, 0);
        cuStr_clear(dest);
        for (unsigned j = 1; search_char != '\0'; j++) {
            char ch = cuStr_at(src, j);
            if (ch == search_char)
                count++;
            else {
                cuStr_printf_append(dest, "%d%c", count, search_char);
                count = 1;
                search_char = ch;
            }
        }
        //printf("%s\n", cuStr_cstr(dest));

        tmp = src; src = dest; dest = tmp;
    }
    cuStr_destroy(&dest);
    return src;

end:
    cuStr_destroy(&cus1);
    cuStr_destroy(&cus2);

    return NULL;
}
#endif // NDEBUG
