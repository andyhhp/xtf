/**
 * @file arch/x86/include/arch/x86-tss.h
 *
 * %x86 Task State Segment infrastructure.
 */

#ifndef XTF_X86_TSS_H
#define XTF_X86_TSS_H

#include <xtf/types.h>
#include <xtf/compiler.h>

struct __packed hw_tss32 {
    uint16_t link; uint16_t _r0;

    uint32_t esp0;
    uint16_t ss0; uint16_t _r1;

    uint32_t esp1;
    uint16_t ss1; uint16_t _r2;

    uint32_t esp2;
    uint16_t ss2; uint16_t _r3;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es; uint16_t _r4;
    uint16_t cs; uint16_t _r5;
    uint16_t ss; uint16_t _r6;
    uint16_t ds; uint16_t _r7;
    uint16_t fs; uint16_t _r8;
    uint16_t gs; uint16_t _r9;
    uint16_t ldtr; uint16_t _r10;
    uint16_t t; uint16_t iopb;
};

struct __packed hw_tss64 {
    uint16_t link; uint16_t _r0;

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint64_t _r1;

    uint64_t ist[7]; /* 1-based structure */

    uint64_t _r2;

    uint16_t t;
    uint16_t iopb;
};

#define X86_TSS_INVALID_IO_BITMAP 0x8000

#if defined(__x86_64__)

typedef struct hw_tss64 hw_tss;

#elif defined(__i386__)

typedef struct hw_tss32 hw_tss;

#else
# error Bad architecture for TSS infrastructure
#endif

#endif /* XTF_X86_TSS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
