#include <xtf/types.h>

static void siftdown(void *base, size_t _size,
                     int (*compar)(const void *, const void *),
                     void (*swap)(void *, void *),
                     long start, long stop)
{
    long size = _size, root, child;

    for ( root = start; root * 2 + size < stop; root = child )
    {
        child = root * 2 + size;

        if ( (child + size < stop) &&
             (compar(base + child, base + child + size) < 0) )
            child += size;

        if ( compar(base + root, base + child) >= 0 )
            return;

        swap(base + root, base + child);
    }
}

void heapsort(void *base, size_t nmemb, size_t _size,
              int (*compar)(const void *, const void *),
              void (*swap)(void *, void *))
{
    long size = _size, total = size * nmemb, idx;

    for ( idx = (nmemb/2 - 1) * size; idx >= 0; idx -= size )
        siftdown(base, _size, compar, swap, idx, total);

    for ( idx = total - size; idx > 0; idx -= size )
    {
        swap(base, base + idx);
        siftdown(base, _size, compar, swap, 0, idx);
    }
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
