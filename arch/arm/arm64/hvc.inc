/*
 * Print a string using Xen debug console
 * Clobbers: x0, x1, x2, x3, x16
 */
.macro console_puts
    mov   x2, x0
    mov   x1, #0
97: ldrb  w3, [x2, x1]
    add   x1, x1, #1
    cbnz  w3, 97b
    mov   x0, #CONSOLEIO_write
    mov   x16, #__HYPERVISOR_console_io
    hvc   #XEN_HYPERCALL_TAG
.endm
