/**
 * Minimal C logic
 */

#if defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
# define COND(_32, _64) _64
#else
# define COND(_32, _64) _32
#endif

const char test_title[] = "Hello from RISC-V" COND("32", "64") "\n";

char __attribute__((section(".bss.page_aligned"))) stack[4096];

void test_main(void)
{
#if defined(__riscv)
    register unsigned long nr asm ("a7");
    register unsigned long a0 asm ("a0");
    register unsigned long a1 asm ("a1");
    register unsigned long a2 asm ("a2");

    nr = 18;                        /* __HYPERVISOR_console_io */
    a0 = 0;                         /* CONSOLEIO_write */
    a1 = sizeof(test_title);        /* len */
    a2 = (unsigned long)test_title; /* ptr */
    asm volatile ("ecall"
                  : [nr] "+r" (nr), [a0] "+r" (a0),
                    [a1] "+r" (a1), [a2] "+r" (a2)
                  :
                  : "memory");

    unsigned int reason = 0;        /* SHUTDOWN_poweroff */

    nr = 29;                        /* __HYPERVISOR_sched_op */
    a0 = 2;                         /* SCHEDOP_shutdown */
    a1 = (unsigned long)&reason;    /* ptr */
    asm volatile ("ecall"
                  : [nr] "+r" (nr), [a0] "+r" (a0), [a1] "+r" (a1)
                  :
                  : "memory");
#endif
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
