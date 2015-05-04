/**
 * @file include/arch/x86/desc.h
 *
 * %x86 segment descriptor infrastructure.
 */

#ifndef XTF_X86_DESC_H
#define XTF_X86_DESC_H

#include <xtf/types.h>
#include <xtf/compiler.h>
#include <xtf/macro_magic.h>

/** 8 byte user segment descriptor (GDT/LDT entries with .s = 1) */
struct __packed seg_desc32 {
    union {
        /** Raw backing integers. */
        struct {
            uint32_t lo, hi;
        };
        /** Common named fields. */
        struct {
            uint16_t limit0;
            uint16_t base0;
            uint8_t  base1;
            unsigned type: 4;
            unsigned s: 1, dpl: 2, p: 1;
            unsigned limit: 4;
            unsigned avl: 1, l: 1, d: 1, g: 1;
            uint8_t base2;
        };
        /** Code segment specific field names. */
        struct {
            uint16_t limit0;
            uint16_t base0;
            uint8_t  base1;
            unsigned a: 1, r: 1, c: 1, x: 1;
            unsigned s: 1, dpl: 2, p: 1;
            unsigned limit: 4;
            unsigned avl: 1, l: 1, d: 1, g: 1;
            uint8_t base2;
        } code;
        /** Data segment specific field names. */
        struct {
            uint16_t limit0;
            uint16_t base0;
            uint8_t  base1;
            unsigned a: 1, w: 1, e: 1, x: 1;
            unsigned s: 1, dpl: 2, p: 1;
            unsigned limit: 4;
            unsigned avl: 1, _r0: 1, b: 1, g: 1;
            uint8_t base2;
        } data;
    };
};

/* GDT/LDT attribute flags for user segments */

/* Common */
#define SEG_ATTR_G      0x8000 /**< Granularity of limit (0 = 1, 1 = 4K) */
#define SEG_ATTR_AVL    0x1000 /**< Available for software use */
#define SEG_ATTR_P      0x0080 /**< Present? */
#define SEG_ATTR_S      0x0010 /**< !System desc (0 = system, 1 = user) */
#define SEG_ATTR_A      0x0001 /**< Accessed? (set by hardware) */

#define SEG_ATTR_COMMON 0x8091 /**< Commonly set bits (G P S A) */

#define SEG_ATTR_DPL0   0x0000 /**< Descriptor privilege level 0 */
#define SEG_ATTR_DPL1   0x0020 /**< Descriptor privilege level 1 */
#define SEG_ATTR_DPL2   0x0040 /**< Descriptor privilege level 2 */
#define SEG_ATTR_DPL3   0x0060 /**< Descriptor privilege level 3 */
#define SEG_ATTR_CODE   0x0008 /**< Type (0 = data, 1 = code)    */
#define SEG_ATTR_DATA   0x0000 /**< Type (0 = data, 1 = code)    */

/* Code segments */
#define SEG_ATTR_D      0x4000 /**< Default operand size (0 = 16bit, 1 = 32bit) */
#define SEG_ATTR_L      0x2000 /**< Long segment? (1 = 64bit) */
#define SEG_ATTR_C      0x0004 /**< Conforming? (0 = non, 1 = conforming) */
#define SEG_ATTR_R      0x0002 /**< Readable? (0 = XO seg, 1 = RX seg) */

/* Data segments */
#define SEG_ATTR_B      0x4000 /**< 'Big' flag.
                                *    - For %ss, default operand size.
                                *    - For expand-down segment, sets upper bound. */
#define SEG_ATTR_E      0x0004 /**< Expand-down? (0 = normal, 1 = expand-down) */
#define SEG_ATTR_W      0x0002 /**< Writable? (0 = RO seg, 1 = RW seg) */

/* Macro magic to expand symbolic SEG_ATTR names into a constant */
#define _GDTE_ATTR0()       (0)
#define _GDTE_ATTR1(x)      (SEG_ATTR_ ## x)
#define _GDTE_ATTR2(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR1(__VA_ARGS__))
#define _GDTE_ATTR3(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR2(__VA_ARGS__))
#define _GDTE_ATTR4(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR3(__VA_ARGS__))
#define _GDTE_ATTR5(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR4(__VA_ARGS__))
#define _GDTE_ATTR6(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR5(__VA_ARGS__))
#define _GDTE_ATTR7(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR6(__VA_ARGS__))
#define _GDTE_ATTR8(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR7(__VA_ARGS__))

#define GDTE_ATTR(...) VAR_MACRO(_GDTE_ATTR, __VA_ARGS__)


#define _INIT_GDTE(base, limit, attr) { { {                           \
     .lo = (((base) & 0xffff) << 16) | ((limit) & 0xffff),            \
     .hi = ((base) & 0xff000000) | ((limit) & 0xf0000) |              \
           (((attr) & 0xf0ff) << 8) | (((base) & 0xff0000) >> 16)     \
     } } }

/** Initialise an LDT/GDT entry using a raw attribute number. */
#define INIT_GDTE_RAW(base, limit, attr) _INIT_GDTE(base, limit, attr)
/** Initialise an LDT/GDT entry using symbol attributes. */
#define INIT_GDTE_SYM(base, limit, ...)  _INIT_GDTE(base, limit, GDTE_ATTR(__VA_ARGS__))

/** Long mode lgdt/lidt table pointer. */
struct __packed desc_ptr64 {
    uint16_t limit;
    uint64_t base;
};

/** Protected mode lgdt/lidt table pointer. */
struct __packed desc_ptr32 {
    uint16_t limit;
    uint32_t base;
};

#if defined(__x86_64__)

typedef struct desc_ptr64 desc_ptr;
typedef struct seg_desc32 user_desc;

#elif defined(__i386__)

typedef struct desc_ptr32 desc_ptr;
typedef struct seg_desc32 user_desc;

#else
# error Bad architecture for descriptor infrastructure
#endif

#endif /* XTF_X86_DESC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
