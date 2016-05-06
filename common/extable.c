/**
 * @file common/extable.c
 *
 * Exception table support.
 */
#include <xtf/lib.h>
#include <xtf/extable.h>

extern struct extable_entry __start_ex_table[], __stop_ex_table[];

const struct extable_entry *search_extable(unsigned long addr)
{
    const struct extable_entry *start = __start_ex_table,
        *stop = __stop_ex_table, *mid;

    while ( start <= stop )
    {
        mid = start + (stop - start) / 2;

        if ( addr == mid->fault )
            return mid;
        else if ( addr > mid->fault )
            start = mid + 1;
        else
            stop = mid - 1;
    }

    return 0;
}

static int compare_extable_entry(const void *_l, const void *_r)
{
    const struct extable_entry *l = _l, *r = _r;

    if ( l->fault == r->fault )
        return 0;
    else if ( l->fault > r->fault )
        return 1;
    else
        return -1;
}

static void swap_extable_entry(void *_l, void *_r)
{
    struct extable_entry tmp, *l = _l, *r = _r;

    tmp = *l;
    *l = *r;
    *r = tmp;
}

void sort_extable(void)
{
    heapsort(__start_ex_table,
             __stop_ex_table - __start_ex_table,
             sizeof(__start_ex_table[0]),
             compare_extable_entry,
             swap_extable_entry);
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
