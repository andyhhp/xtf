#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../common/heapsort.c"

void print_array(const char *pref, size_t n, int arr[n])
{
    size_t i;

    printf("%s{", pref);
    for ( i = 0; i < n; ++i )
    {
        printf("%d", arr[i]);
        if ( i != n - 1 )
            printf(", ");
    }
    printf("}\n");
}

int compar_int(const void *_l, const void *_r)
{
    const int *l = _l, *r = _r;

    return *l - *r;
}

void swap_int(void *_l, void *_r)
{
    int *l = _l, *r = _r, tmp;

    tmp = *l;
    *l = *r;
    *r = tmp;
}

bool test(size_t n, int arr[n])
{
    size_t i;
    int heap[n];

    memcpy(heap, arr, n * sizeof(arr[0]));

    print_array("Input: ", n, arr);

    heapsort(heap, n, sizeof(arr[0]), compar_int, swap_int);

    if ( n > 1 )
    {
        for ( i = 1; i < n; ++i )
            if ( heap[i - 1] > heap[i] )
            {
                print_array("  Failed! ", n, heap);
                return false;
            }
    }

    return true;
}

#define TEST(...)                               \
    ({ int _a[] = { __VA_ARGS__ };              \
        size_t _s = sizeof _a / sizeof *_a;     \
        if ( !test(_s, _a) )                    \
            return 1;                           \
    })

int main(void)
{
    bool success = true;
    TEST(1);
    TEST(1, 2);
    TEST(2, 1);
    TEST(3, 2, 1);
    TEST(4, 3, 2, 1);
    TEST(3, 2, 4, 1);
    TEST(3, 3, 4, 1);
    TEST(3, 3, 4, 1, 10);
    TEST(2, 3, 11, 1, 10);
    TEST(2, 3, 11, 1, 10, 4);

    printf("Testing array of 10000 random elements\n");

    int i, *arr = malloc(sizeof(*arr) * 10000);
    for ( i = 0; i < 10000; ++i )
        arr[i] = rand();

    heapsort(arr, 10000, sizeof(*arr), compar_int, swap_int);

    for ( i = 1; i < 10000; ++i )
        if ( arr[i - 1] > arr[i] )
        {
            printf("Failed\n");
            success = false;
            break;
        }
    free(arr);

    return !success;
}
