#ifndef XEN_PUBLIC_IO_CONSOLE_H
#define XEN_PUBLIC_IO_CONSOLE_H

struct xencons_interface
{
    char in[1024];
    char out[2048];
    uint32_t in_cons, in_prod;
    uint32_t out_cons, out_prod;
};
typedef struct xencons_interface xencons_interface_t;

#endif /* XEN_PUBLIC_IO_CONSOLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
