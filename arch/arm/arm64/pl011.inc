#include <arch/pl011.h>

/*
 * PL011 UART initialization
 * xb: register which contains the UART base address
 * c: scratch register number
 */
.macro pl011_uart_init xb, c
    mov   x\c, #PL011_WLEN_8
    str   w\c, [\xb, #PL011_UARTLCR]
    ldr   x\c, =(PL011_TX_ENABLE | PL011_ENABLE)
    str   w\c, [\xb, #PL011_UARTCR]
.endm

/*
 * PL011 UART wait UART to be ready to transmit
 * xb: register which contains the UART base address
 * c: scratch register number
 */
.macro pl011_uart_ready xb, c
80: ldrh  w\c, [\xb, #PL011_UARTFR]
    tst   w\c, #PL011_FR_BUSY           /* Check BUSY bit */
    b.ne  80b                            /* Wait for the UART to be ready */
.endm

/*
 * PL011 UART transmit character
 * xb: register which contains the UART base address
 * wt: register which contains the character to transmit
 */
.macro pl011_uart_transmit xb, wt
    strb  \wt, [\xb]
.endm

/*
 * PL011 UART transmit string
 * Clobbers: x1
 */
.macro console_puts
81:
    pl011_uart_ready x23, 1
    ldrb   w1, [x0], #1
    cbz    w1, 82f
    pl011_uart_transmit x23, w1
    b      81b
82:
    /* Send carriage return at the end */
    mov   w1, 0x0D
    pl011_uart_transmit x23, w1
.endm
