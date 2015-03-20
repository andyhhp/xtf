#ifndef XTF_CONSOLE_H
#define XTF_CONSOLE_H

#include <xtf/compiler.h>

#include <xen/event_channel.h>
#include <xen/io/console.h>

/* Console output callback. */
typedef void (*cons_output_cb)(const char *buf, size_t len);

/*
 * Register a console callback.  Several callbacks can be registered for usful
 * destinations of console text.
 */
void register_console_callback(cons_output_cb cb);

/*
 * Initialise the PV console.  Will register a callback.
 */
void init_pv_console(xencons_interface_t *ring,
                     evtchn_port_t port);

void printk(const char *fmt, ...) __printf(1, 2);

#endif /* XTF_CONSOLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
