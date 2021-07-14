/**
 * @file arch/arm/include/arch/pl011.h
 *
 * ARM PrimeCell UART PL011.
 */
#ifndef XTF_ARM_PL011_H
#define XTF_ARM_PL011_H

#include <xtf/types.h>

#define PL011_REG(reg)      (volatile void *)(CONFIG_PL011_ADDRESS + reg)

/* UART register offsets */
#define PL011_UARTDR        0x00    /* Data register. */
#define PL011_UARTRSR       0x04    /* Receive status register (Read). */
#define PL011_UARTECR       0x04    /* Error clear register (Write). */
#define PL011_UARTFR        0x18    /* Flag register (Read only). */
#define PL011_UARTILPR      0x20    /* IrDA low power counter register. */
#define PL011_UARTIBRD      0x24    /* Integer baud rate divisor register. */
#define PL011_UARTFBRD      0x28    /* Fractional baud rate divisor register. */
#define PL011_UARTLCR       0x2C    /* Line control register. */
#define PL011_UARTCR        0x30    /* Control register. */
#define PL011_UARTIFLS      0x34    /* Interrupt fifo level select. */
#define PL011_UARTIMSC      0x38    /* Interrupt mask. */
#define PL011_UARTRIS       0x3C    /* Raw interrupt status. */
#define PL011_UARTMIS       0x40    /* Masked interrupt status. */
#define PL011_UARTICR       0x44    /* Interrupt clear register. */
#define PL011_UARTDMACR     0x48    /* DMA control register. */

/* UARTFR bits */
#define PL011_FR_BUSY       (1 << 3) /* Transmit is not complete. */

/* UARTCR bits */
#define PL011_ENABLE        (1 << 0) /* UART enable. */
#define PL011_TX_ENABLE     (1 << 8) /* Transmit enable. */

/* UARTLCR bits */
#define PL011_WLEN_8        (3 << 5) /* 8bits word length. */
#define PL011_PEN           (1 << 1) /* Parity enable. */

#ifndef __ASSEMBLY__
void pl011_init(void);
void pl011_console_write(const char *buf, size_t len);
void pl011_putc(char c);
void pl011_puts(const char *s, size_t len);
#endif

#endif /* XTF_ARM_PL011_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
