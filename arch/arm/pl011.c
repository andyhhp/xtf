/**
 * @file arch/arm/pl011.c
 *
 * ARM PrimeCell UART PL011.
 */
#include <arch/pl011.h>
#include <arch/barrier.h>

static inline uint32_t pl011_read(const volatile void *addr)
{
    uint32_t val;
#ifdef CONFIG_ARM_64
    asm volatile("ldr %w0, [%1]\n" : "=r" (val) : "r" (addr));
#else
    asm volatile("ldr %1, %0\n": "+Qo" (*(volatile uint32_t*)addr), "=r" (val));
#endif
    rmb();
    return val;
}

static inline void pl011_write(uint32_t val, volatile void *addr)
{
    wmb();
#ifdef CONFIG_ARM_64
    asm volatile("str %w0, [%1]\n" : : "r" (val), "r" (addr));
#else
    asm volatile("str %1, %0\n": "+Qo" (*(volatile uint32_t*)addr) : "r" (val));
#endif
}

void pl011_putc(char c)
{
    uint32_t busy;
    do {
        busy = pl011_read(PL011_REG(PL011_UARTFR));
    } while(busy & PL011_FR_BUSY);

    pl011_write((uint32_t)(unsigned char)c, PL011_REG(PL011_UARTDR));
}

void pl011_puts(const char *s, size_t len)
{
    for(; len > 0; len--, s++)
    {
        char c = *s;
        pl011_putc(c);
    }
}

void pl011_init(void)
{
    /* 8-N-1 */
    pl011_write(PL011_WLEN_8, PL011_REG(PL011_UARTLCR));
    /* Enable UART TX */
    pl011_write((PL011_TX_ENABLE | PL011_ENABLE), PL011_REG(PL011_UARTCR));
}

void pl011_console_write(const char *buf, size_t len)
{
    pl011_puts(buf, len);
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
