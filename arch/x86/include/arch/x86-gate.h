/**
 * @file arch/x86/include/arch/x86-gate.h
 *
 * %x86 Gate Descriptor infrastructure.
 */

#ifndef XTF_X86_GATE_H
#define XTF_X86_GATE_H

#include <xtf/lib.h>

/** 8-byte gate - Protected mode IDT entry, GDT task/call gate. */
struct __packed x86_gate32 {
    union {
        struct {
            uint32_t lo, hi;
        };
        struct {
            uint16_t offset0;
            uint16_t selector;
            unsigned int param_count:5, _r0:3;
            unsigned int type:4, s:1, dpl:2, p:1;
            uint16_t offset1;
        };
    };
};

static inline void pack_x86_gate32(
    struct x86_gate32 *g,
    unsigned int type, unsigned int sel, uint32_t offset,
    unsigned int dpl, unsigned int count)
{
    g->offset0     = offset & 0xffff;
    g->selector    = sel;
    g->param_count = count;
    g->_r0         = 0;
    g->type        = type;
    g->s           = 0;
    g->dpl         = dpl;
    g->p           = 1;
    g->offset1     = (offset >> 16) & 0xffff;
}

/** 16-byte gate - Long mode IDT entry, GDT call gate. */
struct __packed x86_gate64 {
    union {
        struct {
            uint64_t lo, hi;
        };
        struct {
            uint16_t offset0;
            uint16_t selector;
            unsigned int ist:3, _r0:5;
            unsigned int type:4, s:1, dpl:2, p:1;
            uint16_t offset1;
            uint32_t offset2;
            uint32_t _r1;
        };
    };
};

static inline void pack_x86_gate64(
    struct x86_gate64 *g,
    unsigned int type, unsigned int sel, uint64_t offset,
    unsigned int dpl, unsigned int ist)
{
    g->offset0  = offset & 0xffff;
    g->selector = sel;
    g->ist      = ist;
    g->_r0      = 0;
    g->type     = type;
    g->s        = 0;
    g->dpl      = dpl;
    g->p        = 1;
    g->offset1  = (offset >> 16) & 0xffff;
    g->offset2  = (offset >> 32) & 0xffffffffu;
    g->_r1      = 0;
}

#if defined(__x86_64__)

typedef struct x86_gate64 env_gate;

#elif defined(__i386__)

typedef struct x86_gate32 env_gate;

#else
# error Bad architecture for Gate infrastructure
#endif

static inline void pack_gate(
    env_gate *g,
    unsigned int type, unsigned int sel, unsigned long offset,
    unsigned int dpl, unsigned int other)
{
#if defined(__x86_64__)
    pack_x86_gate64
#else
    pack_x86_gate32
#endif
        (g, type, sel, offset, dpl, other);
}

static inline void pack_task_gate(env_gate *g, unsigned int selector)
{
    pack_gate(g, 5, selector, 0, 0, 0);
}

static inline void pack_intr_gate(
    env_gate *g, unsigned int sel, unsigned long offset,
    unsigned int dpl, unsigned int other)
{
    pack_gate(g, 14, sel, offset, dpl, other);
}

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
