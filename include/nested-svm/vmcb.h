/* Shared minimal VMCB definitions for nested-SVM tests. */
#ifndef XTF_NESTED_SVM_VMCB_H
#define XTF_NESTED_SVM_VMCB_H

#include <xtf/types.h>

struct vmcb_seg {
    uint16_t sel;
    uint16_t attr;
    uint32_t limit;
    uint64_t base;
};

/* VMCB 0x060: Virtual Interrupt Control to inject virtual (INTR) interrupts */
typedef union v_intr_ctrl {
    uint64_t bytes;
    struct v_intr_ctrl_fields { /* V_INTR_CTRL */
        uint64_t v_tpr         :  8;  /* 0:7 - virt Task Priority Register */
        uint64_t vIRQ_pending  :  1;  /* 8   - vIRQ pending */
        uint64_t vGIF          :  1;  /* 9   - vGIF value */
        uint64_t reserved_1    :  1;  /* 10  - Reserved */
        uint64_t vNMI_pending  :  1;  /* 11  - vNMI pending */
        uint64_t vNMI_blocking :  1;  /* 12  - vNMI blocking */
        uint64_t reserved_2    :  3;  /* Reserved */
        uint64_t vIRQ_prio     :  4;  /* 16:19 - vIRQ Priority */
        uint64_t v_ign_tpr     :  1;  /* 20    - vIRQ ignore TPR */
        uint64_t reserved_3    :  3;  /*  Reserved */
        uint64_t vIRQ_masking  :  1;  /* 24 - vIRQ Masking */
        uint64_t vGIF_enabled  :  1;  /* 25 - vGIF enable/disable */
        uint64_t vNMI_enabled  :  1;  /* 26 - vNMI enable/disable */
        uint64_t reserved_4    :  3;  /* Reserved */
        uint64_t x2AVIC_enable :  1;  /* 30 - x2AVIC enable/disable */
        uint64_t AVIC_enable   :  1;  /* 31 - AVIC enable/disable */
        uint64_t vector        :  8;  /* 32:39 - Virtual Interrupt Vector */
        uint64_t reserved_5    : 24;  /* 40:63 - Reserved */
    } __attribute__((packed)) fields;
} v_intr_ctrl_t;

typedef union intercept_insns_00c {
    uint32_t bytes;
    struct intercept_insns_vec_00c_fields {
        uint32_t intr        : 1; /*  0 - INTR (physical maskable interrupt) */
        uint32_t nmi         : 1; /*  1 - NMI */
        uint32_t smi         : 1; /*  2 - SMI */
        uint32_t init        : 1; /*  3 - INIT */
        uint32_t vintr       : 1; /*  4 - VINTR (virtual maskable interrupt) */
        uint32_t cr0         : 1; /*  5 - CR0 bit writes other than TS or MP */
        uint32_t rd_idtr     : 1; /*  6 - IDTR read */
        uint32_t rd_gdtr     : 1; /*  7 - GDTR read */
        uint32_t rd_ldtr     : 1; /*  8 - LDTR read */
        uint32_t rd_tr       : 1; /*  9 - TR read */
        uint32_t wr_idtr     : 1; /* 10 - IDTR write */
        uint32_t wr_gdtr     : 1; /* 11 - GDTR write */
        uint32_t wr_ldtr     : 1; /* 12 - LDTR write */
        uint32_t wr_tr       : 1; /* 13 - TR write */
        uint32_t rdtsc       : 1; /* 14 - RDTSC */
        uint32_t rdpmc       : 1; /* 15 - RDPMC */
        uint32_t pushf       : 1; /* 16 - PUSHF */
        uint32_t popf        : 1; /* 17 - POPF */
        uint32_t cpuid       : 1; /* 18 - CPUID */
        uint32_t rsm         : 1; /* 19 - RSM */
        uint32_t iret        : 1; /* 20 - IRET */
        uint32_t intn        : 1; /* 21 - INTn */
        uint32_t invd        : 1; /* 22 - INVD */
        uint32_t pause       : 1; /* 23 - PAUSE */
        uint32_t hlt         : 1; /* 24 - HLT */
        uint32_t invlpg      : 1; /* 25 - INVLPG */
        uint32_t invlpga     : 1; /* 26 - INVLPGA */
        uint32_t ioio        : 1; /* 27 - IOIO_PROT */
        uint32_t msr         : 1; /* 28 - MSR_PROT */
        uint32_t task_switch : 1; /* 29 - TASK_SWITCH */
        uint32_t ferr_freeze : 1; /* 30 - FERR_FREEZE */
        uint32_t shutdown    : 1; /* 31 - Shutdown events */
    } __attribute__((packed)) fields;
} intercept_insns_vec_00c_t;

typedef union intercept_insns_vec_010 {
    uint32_t bytes;
    struct intercept_insns_vec_010_fields {
        uint32_t vmrun     : 1; /*  0 - VMRUN */
        uint32_t vmmcall   : 1; /*  1 - VMMCALL */
        uint32_t vmload    : 1; /*  2 - VMLOAD */
        uint32_t vmsave    : 1; /*  3 - VMSAVE */
        uint32_t stgi      : 1; /*  4 - STGI */
        uint32_t clgi      : 1; /*  5 - CLGI */
        uint32_t skinit    : 1; /*  6 - SKINIT */
        uint32_t rdtscp    : 1; /*  7 - RDTSCP */
        uint32_t icebp     : 1; /*  8 - ICEBP */
        uint32_t wbinvd    : 1; /*  9 - WBINVD/WBNOINVD */
        uint32_t monitor   : 1; /* 10 - MONITOR/MONITORX */
        uint32_t mwait     : 1; /* 11 - MWAIT/MWAITX unconditionally */
        uint32_t mwait_mon : 1; /* 12 - MWAIT/MWAITX monitor conditional */
        uint32_t xsetbv    : 1; /* 13 - XSETBV */
        uint32_t rdpru     : 1; /* 14 - RDPRU */
        uint32_t efer      : 1; /* 15 - EFER write */
        uint32_t cr0       : 1; /* 16 - CR0 write */
        uint32_t cr1       : 1; /* 17 - CR1 write */
        uint32_t cr2       : 1; /* 18 - CR2 write */
        uint32_t cr3       : 1; /* 19 - CR3 write */
        uint32_t cr4       : 1; /* 20 - CR4 write */
        uint32_t cr5       : 1; /* 21 - CR5 write */
        uint32_t cr6       : 1; /* 22 - CR6 write */
        uint32_t cr7       : 1; /* 23 - CR7 write */
        uint32_t cr8       : 1; /* 24 - CR8 write */
        uint32_t cr9       : 1; /* 25 - CR9 write */
        uint32_t cr10      : 1; /* 26 - CR10 write */
        uint32_t cr11      : 1; /* 27 - CR11 write */
        uint32_t cr12      : 1; /* 28 - CR12 write */
        uint32_t cr13      : 1; /* 29 - CR13 write */
        uint32_t cr14      : 1; /* 30 - CR14 write */
        uint32_t cr15      : 1; /* 31 - CR15 write */
    } __attribute__((packed)) fields;
} intercept_insns_vec_010_t;


typedef union intercept_insns_vec_014 {
    uint32_t bytes;
    struct intercept_insns_vec_014_fields {
        uint32_t invlpgb_all    : 1; /*  0 - All INVLPGB insns */
        uint32_t invlpgb_ill    : 1; /*  1 - Illegally specified INVLPGBs */
        uint32_t invpciid       : 1; /*  2 - INVPCID */
        uint32_t mcommit        : 1; /*  3 - MCOMMIT */
        uint32_t tlbsync        : 1; /*  4 - TLB SYNC */
        uint32_t buslock        : 1; /*  5 - BUSLOCK */
        uint32_t hlt_no_pending : 1; /*  6 - HLT with no pending virq */
    } __attribute__((packed)) fields;
} intercept_insns_vec_014_t;

struct vmcb {
    uint16_t intercept_read_cr;
    uint16_t intercept_write_cr;
    uint16_t intercept_read_dr;
    uint16_t intercept_write_dr;
    uint32_t intercept_exceptions;
    intercept_insns_vec_00c_t intercept_insns_vec_00c;
    intercept_insns_vec_010_t intercept_insns_vec_010;
    intercept_insns_vec_014_t intercept_insns_vec_014;
    uint8_t  _pad_018[0x03C - 0x018];
    uint16_t pause_filter_threshold;
    uint16_t pause_filter_count;
    uint64_t iopm_base_pa;
    uint64_t msrpm_base_pa;
    uint64_t tsc_offset;
    uint32_t asid;
    uint8_t  tlb_control;
    uint8_t  _pad_05d[3];
    /* 0x060: Virtual Interrupt Control to inject virtual interrupts */
    v_intr_ctrl_t v_intr_ctrl;
    uint64_t int_state;
    uint64_t exitcode;
    uint64_t exitinfo1;
    uint64_t exitinfo2;
    uint64_t exit_int_info;
    uint64_t np_enable;
    uint8_t  _pad_098[0x0a8 - 0x098];
    /* 0x0a8: Event Injection */
    uint64_t event_inj;
    uint64_t h_cr3;
    uint8_t  _pad_0b8[0x400 - 0x0b8];
    struct vmcb_seg es;
    struct vmcb_seg cs;
    struct vmcb_seg ss;
    struct vmcb_seg ds;
    struct vmcb_seg fs;
    struct vmcb_seg gs;
    struct vmcb_seg gdtr;
    struct vmcb_seg ldtr;
    struct vmcb_seg idtr;
    struct vmcb_seg tr;
    uint8_t  _pad_4a0[0x4cb - 0x4a0];
    uint8_t  cpl;
    uint32_t _pad_4cc;
    uint64_t efer;
    uint8_t  _pad_4d8[0x548 - 0x4d8];
    uint64_t cr4;
    uint64_t cr3;
    uint64_t cr0;
    uint64_t dr7;
    uint64_t dr6;
    uint64_t rflags;
    uint64_t rip;
    uint8_t  _pad_580[0x5d8 - 0x580];
    uint64_t rsp;
    uint8_t  _pad_5e0[0x5f8 - 0x5e0];
    uint64_t rax;
    uint8_t  _pad_tail[0x1000 - 0x600];
};

#define VMCB_CHECK(field, offset) \
    _Static_assert(__builtin_offsetof(struct vmcb, field) == (offset), \
                   "VMCB layout mismatch: " #field)
VMCB_CHECK(intercept_insns_vec_00c,   0x000c);
VMCB_CHECK(intercept_insns_vec_010,   0x0010);
VMCB_CHECK(intercept_insns_vec_014,   0x0014);
VMCB_CHECK(asid,                      0x0058);
VMCB_CHECK(v_intr_ctrl,               0x0060);
VMCB_CHECK(exitcode,                  0x0070);
VMCB_CHECK(es,                        0x0400);
VMCB_CHECK(gdtr,                      0x0460);
VMCB_CHECK(idtr,                      0x0480);
VMCB_CHECK(tr,                        0x0490);
VMCB_CHECK(efer,                      0x04d0);
VMCB_CHECK(cr4,                       0x0548);
VMCB_CHECK(cr3,                       0x0550);
VMCB_CHECK(cr0,                       0x0558);
VMCB_CHECK(rflags,                    0x0570);
VMCB_CHECK(rip,                       0x0578);
VMCB_CHECK(rsp,                       0x05d8);
VMCB_CHECK(rax,                       0x05f8);
_Static_assert(sizeof(struct vmcb) == 0x1000, "VMCB size != 4 KiB");
#undef VMCB_CHECK

/* VMCB exit codes (Exception-vector exit-code slots) */
#define VMEXIT_READ_CR0            0x000
#define VMEXIT_READ_CR2            0x002
#define VMEXIT_READ_CR3            0x003
#define VMEXIT_READ_CR4            0x004
#define VMEXIT_READ_CR8            0x008
#define VMEXIT_WRITE_CR0           0x010
#define VMEXIT_WRITE_CR2           0x012
#define VMEXIT_WRITE_CR3           0x013
#define VMEXIT_WRITE_CR4           0x014
#define VMEXIT_WRITE_CR8           0x018
#define VMEXIT_READ_DR0            0x020
#define VMEXIT_READ_DR1            0x021
#define VMEXIT_READ_DR2            0x022
#define VMEXIT_READ_DR3            0x023
#define VMEXIT_READ_DR4            0x024
#define VMEXIT_READ_DR5            0x025
#define VMEXIT_READ_DR6            0x026
#define VMEXIT_READ_DR7            0x027
#define VMEXIT_WRITE_DR0           0x030
#define VMEXIT_WRITE_DR1           0x031
#define VMEXIT_WRITE_DR2           0x032
#define VMEXIT_WRITE_DR3           0x033
#define VMEXIT_WRITE_DR4           0x034
#define VMEXIT_WRITE_DR5           0x035
#define VMEXIT_WRITE_DR6           0x036
#define VMEXIT_WRITE_DR7           0x037
#define VMEXIT_EXCP_BASE           0x040
#define VMEXIT_EXCP_DE             0x040 /* vector 0, #DE */
#define VMEXIT_EXCP_DB             0x041 /* vector 1, #DB */
#define VMEXIT_EXCP_BP             0x043 /* vector 3, #BP */
#define VMEXIT_EXCP_OF             0x044 /* vector 4, #OF */
#define VMEXIT_EXCP_BR             0x045 /* vector 5, #BR */
#define VMEXIT_EXCP_UD             0x046 /* vector 6, #UD */
#define VMEXIT_EXCP_NM             0x047 /* vector 7, #NM */
#define VMEXIT_EXCP_DF             0x048 /* vector 8, #DF */
#define VMEXIT_EXCP_TS             0x04a /* vector 10, #TS */
#define VMEXIT_EXCP_NP             0x04b /* vector 11, #NP */
#define VMEXIT_EXCP_SS             0x04c /* vector 12, #SS */
#define VMEXIT_EXCP_GP             0x04d /* vector 13, #GP */
#define VMEXIT_EXCP_PF             0x04e /* vector 14, #PF */
#define VMEXIT_EXCP_MF             0x050 /* vector 16, #MF */
#define VMEXIT_EXCP_AC             0x051 /* vector 17, #AC */
#define VMEXIT_EXCP_MC             0x052 /* vector 18, #MC */
#define VMEXIT_EXCP_XF             0x053 /* vector 19, #XF */
#define VMEXIT_EXCP_CP             0x055 /* vector 21, #CP */
#define VMEXIT_EXCP_HV             0x05c /* vector 28, #HV */
#define VMEXIT_EXCP_VC             0x05d /* vector 29, #VC */
#define VMEXIT_EXCP_SX             0x05e /* vector 30, #SX */
#define VMEXIT_INTR                0x060
#define VMEXIT_NMI                 0x061
#define VMEXIT_SMI                 0x062
#define VMEXIT_INIT                0x063
#define VMEXIT_VINTR               0x064
#define VMEXIT_CR0_SEL_WRITE       0x065
#define VMEXIT_IDTR_READ           0x066
#define VMEXIT_GDTR_READ           0x067
#define VMEXIT_LDTR_READ           0x068
#define VMEXIT_TR_READ             0x069
#define VMEXIT_IDTR_WRITE          0x06a
#define VMEXIT_GDTR_WRITE          0x06b
#define VMEXIT_LDTR_WRITE          0x06c
#define VMEXIT_TR_WRITE            0x06d
#define VMEXIT_RDTSC               0x06e
#define VMEXIT_RDPMC               0x06f
#define VMEXIT_PUSHF               0x070
#define VMEXIT_POPF                0x071
#define VMEXIT_CPUID               0x072
#define VMEXIT_RSM                 0x073
#define VMEXIT_IRET                0x074
#define VMEXIT_SWINT               0x075
#define VMEXIT_INVD                0x076
#define VMEXIT_PAUSE               0x077
#define VMEXIT_HLT                 0x078
#define VMEXIT_INVLPG              0x079
#define VMEXIT_INVLPGA             0x07a
#define VMEXIT_IOIO                0x07b
#define VMEXIT_MSR                 0x07c
#define VMEXIT_TASK_SWITCH         0x07d
#define VMEXIT_FERR_FREEZE         0x07e
#define VMEXIT_SHUTDOWN            0x07f
#define VMEXIT_VMRUN               0x080
#define VMEXIT_VMMCALL             0x081
#define VMEXIT_VMLOAD              0x082
#define VMEXIT_VMSAVE              0x083
#define VMEXIT_STGI                0x084
#define VMEXIT_CLGI                0x085
#define VMEXIT_SKINIT              0x086
#define VMEXIT_RDTSCP              0x087
#define VMEXIT_ICEBP               0x088
#define VMEXIT_WBINVD              0x089
#define VMEXIT_MONITOR             0x08a
#define VMEXIT_MWAIT               0x08b
#define VMEXIT_MWAIT_COND          0x08c
#define VMEXIT_XSETBV              0x08d
#define VMEXIT_RDPRU               0x08e
#define VMEXIT_EFER_WRITE_TRAP     0x08f
#define VMEXIT_CR0_WRITE_TRAP      0x090
#define VMEXIT_CR1_WRITE_TRAP      0x091
#define VMEXIT_CR2_WRITE_TRAP      0x092
#define VMEXIT_CR3_WRITE_TRAP      0x093
#define VMEXIT_CR4_WRITE_TRAP      0x094
#define VMEXIT_CR5_WRITE_TRAP      0x095
#define VMEXIT_CR6_WRITE_TRAP      0x096
#define VMEXIT_CR7_WRITE_TRAP      0x097
#define VMEXIT_CR8_WRITE_TRAP      0x098
#define VMEXIT_CR9_WRITE_TRAP      0x099
#define VMEXIT_CR10_WRITE_TRAP     0x09a
#define VMEXIT_CR11_WRITE_TRAP     0x09b
#define VMEXIT_CR12_WRITE_TRAP     0x09c
#define VMEXIT_CR13_WRITE_TRAP     0x09d
#define VMEXIT_CR14_WRITE_TRAP     0x09e
#define VMEXIT_CR15_WRITE_TRAP     0x09f
#define VMEXIT_INVLPGB             0x0a0
#define VMEXIT_INVLPGB_ILLEGAL     0x0a1
#define VMEXIT_INVPCID             0x0a2
#define VMEXIT_MCOMMIT             0x0a3
#define VMEXIT_TLBSYNC             0x0a4
#define VMEXIT_BUS_LOCK            0x0a5
#define VMEXIT_IDLE_HLT            0x0a6

/* Feature-specific exits. */
#define VMEXIT_NPF                 0x400
#define VMEXIT_AVIC_INCOMPLETE_IPI 0x401
#define VMEXIT_AVIC_NOACCEL        0x402
#define VMEXIT_VMGEXIT             0x403

/* Host/software and error exits. */
#define VMEXIT_SW                  0xf0000000ull
#define VMEXIT_INVALID             (~0ull) /* -1 */
#define VMEXIT_BUSY                (~1ull) /* -2 */
#define VMEXIT_IDLE_REQUIRED       (~2ull) /* -3 */
#define VMEXIT_INVALID_PMC         (~3ull) /* -4 */

/* Get the vmexit reason */
static const char __used *vmexit_reason(uint64_t exitcode)
{
    switch (exitcode) {
    case VMEXIT_READ_CR0:            return "READ_CR0";
    case VMEXIT_READ_CR2:            return "READ_CR2";
    case VMEXIT_READ_CR3:            return "READ_CR3";
    case VMEXIT_READ_CR4:            return "READ_CR4";
    case VMEXIT_READ_CR8:            return "READ_CR8";
    case VMEXIT_WRITE_CR0:           return "WRITE_CR0";
    case VMEXIT_WRITE_CR2:           return "WRITE_CR2";
    case VMEXIT_WRITE_CR3:           return "WRITE_CR3";
    case VMEXIT_WRITE_CR4:           return "WRITE_CR4";
    case VMEXIT_WRITE_CR8:           return "WRITE_CR8";
    case VMEXIT_READ_DR0:            return "READ_DR0";
    case VMEXIT_READ_DR1:            return "READ_DR1";
    case VMEXIT_READ_DR2:            return "READ_DR2";
    case VMEXIT_READ_DR3:            return "READ_DR3";
    case VMEXIT_READ_DR4:            return "READ_DR4";
    case VMEXIT_READ_DR5:            return "READ_DR5";
    case VMEXIT_READ_DR6:            return "READ_DR6";
    case VMEXIT_READ_DR7:            return "READ_DR7";
    case VMEXIT_WRITE_DR0:           return "WRITE_DR0";
    case VMEXIT_WRITE_DR1:           return "WRITE_DR1";
    case VMEXIT_WRITE_DR2:           return "WRITE_DR2";
    case VMEXIT_WRITE_DR3:           return "WRITE_DR3";
    case VMEXIT_WRITE_DR4:           return "WRITE_DR4";
    case VMEXIT_WRITE_DR5:           return "WRITE_DR5";
    case VMEXIT_WRITE_DR6:           return "WRITE_DR6";
    case VMEXIT_WRITE_DR7:           return "WRITE_DR7";
    case VMEXIT_EXCP_DE:             return "EXCP_DE";
    case VMEXIT_EXCP_DB:             return "EXCP_DB";
    case VMEXIT_EXCP_BP:             return "EXCP_BP";
    case VMEXIT_EXCP_OF:             return "EXCP_OF";
    case VMEXIT_EXCP_BR:             return "EXCP_BR";
    case VMEXIT_EXCP_UD:             return "EXCP_UD";
    case VMEXIT_EXCP_NM:             return "EXCP_NM";
    case VMEXIT_EXCP_DF:             return "EXCP_DF";
    case VMEXIT_EXCP_TS:             return "EXCP_TS";
    case VMEXIT_EXCP_NP:             return "EXCP_NP";
    case VMEXIT_EXCP_SS:             return "EXCP_SS";
    case VMEXIT_EXCP_GP:             return "EXCP_GP";
    case VMEXIT_EXCP_PF:             return "EXCP_PF";
    case VMEXIT_EXCP_MF:             return "EXCP_MF";
    case VMEXIT_EXCP_AC:             return "EXCP_AC";
    case VMEXIT_EXCP_MC:             return "EXCP_MC";
    case VMEXIT_EXCP_XF:             return "EXCP_XF";
    case VMEXIT_EXCP_CP:             return "EXCP_CP";
    case VMEXIT_EXCP_HV:             return "EXCP_HV";
    case VMEXIT_EXCP_VC:             return "EXCP_VC";
    case VMEXIT_EXCP_SX:             return "EXCP_SX";
    case VMEXIT_INTR:                return "INTR";
    case VMEXIT_NMI:                 return "NMI";
    case VMEXIT_SMI:                 return "SMI";
    case VMEXIT_INIT:                return "INIT";
    case VMEXIT_VINTR:               return "VINTR";
    case VMEXIT_CR0_SEL_WRITE:       return "CR0_SEL_WRITE";
    case VMEXIT_IDTR_READ:           return "IDTR_READ";
    case VMEXIT_GDTR_READ:           return "GDTR_READ";
    case VMEXIT_LDTR_READ:           return "LDTR_READ";
    case VMEXIT_TR_READ:             return "TR_READ";
    case VMEXIT_IDTR_WRITE:          return "IDTR_WRITE";
    case VMEXIT_GDTR_WRITE:          return "GDTR_WRITE";
    case VMEXIT_LDTR_WRITE:          return "LDTR_WRITE";
    case VMEXIT_TR_WRITE:            return "TR_WRITE";
    case VMEXIT_RDTSC:               return "RDTSC";
    case VMEXIT_RDPMC:               return "RDPMC";
    case VMEXIT_PUSHF:               return "PUSHF";
    case VMEXIT_POPF:                return "POPF";
    case VMEXIT_CPUID:               return "CPUID";
    case VMEXIT_RSM:                 return "RSM";
    case VMEXIT_IRET:                return "IRET";
    case VMEXIT_SWINT:               return "SWINT";
    case VMEXIT_INVD:                return "INVD";
    case VMEXIT_PAUSE:               return "PAUSE";
    case VMEXIT_HLT:                 return "HLT";
    case VMEXIT_INVLPG:              return "INVLPG";
    case VMEXIT_INVLPGA:             return "INVLPGA";
    case VMEXIT_IOIO:                return "IOIO";
    case VMEXIT_MSR:                 return "MSR";
    case VMEXIT_TASK_SWITCH:         return "TASK_SWITCH";
    case VMEXIT_FERR_FREEZE:         return "FERR_FREEZE";
    case VMEXIT_SHUTDOWN:            return "SHUTDOWN";
    case VMEXIT_VMRUN:               return "VMRUN";
    case VMEXIT_VMMCALL:             return "VMMCALL";
    case VMEXIT_VMLOAD:              return "VMLOAD";
    case VMEXIT_VMSAVE:              return "VMSAVE";
    case VMEXIT_STGI:                return "STGI";
    case VMEXIT_CLGI:                return "CLGI";
    case VMEXIT_SKINIT:              return "SKINIT";
    case VMEXIT_RDTSCP:              return "RDTSCP";
    case VMEXIT_ICEBP:               return "ICEBP";
    case VMEXIT_WBINVD:              return "WBINVD";
    case VMEXIT_MONITOR:             return "MONITOR";
    case VMEXIT_MWAIT:               return "MWAIT";
    case VMEXIT_MWAIT_COND:          return "MWAIT_COND";
    case VMEXIT_XSETBV:              return "XSETBV";
    case VMEXIT_RDPRU:               return "RDPRU";
    case VMEXIT_EFER_WRITE_TRAP:     return "EFER_WRITE_TRAP";
    case VMEXIT_CR0_WRITE_TRAP:      return "CR0_WRITE_TRAP";
    case VMEXIT_CR1_WRITE_TRAP:      return "CR1_WRITE_TRAP";
    case VMEXIT_CR2_WRITE_TRAP:      return "CR2_WRITE_TRAP";
    case VMEXIT_CR3_WRITE_TRAP:      return "CR3_WRITE_TRAP";
    case VMEXIT_CR4_WRITE_TRAP:      return "CR4_WRITE_TRAP";
    case VMEXIT_CR5_WRITE_TRAP:      return "CR5_WRITE_TRAP";
    case VMEXIT_CR6_WRITE_TRAP:      return "CR6_WRITE_TRAP";
    case VMEXIT_CR7_WRITE_TRAP:      return "CR7_WRITE_TRAP";
    case VMEXIT_CR8_WRITE_TRAP:      return "CR8_WRITE_TRAP";
    case VMEXIT_CR9_WRITE_TRAP:      return "CR9_WRITE_TRAP";
    case VMEXIT_CR10_WRITE_TRAP:     return "CR10_WRITE_TRAP";
    case VMEXIT_CR11_WRITE_TRAP:     return "CR11_WRITE_TRAP";
    case VMEXIT_CR12_WRITE_TRAP:     return "CR12_WRITE_TRAP";
    case VMEXIT_CR13_WRITE_TRAP:     return "CR13_WRITE_TRAP";
    case VMEXIT_CR14_WRITE_TRAP:     return "CR14_WRITE_TRAP";
    case VMEXIT_CR15_WRITE_TRAP:     return "CR15_WRITE_TRAP";
    case VMEXIT_INVLPGB:             return "INVLPGB";
    case VMEXIT_INVLPGB_ILLEGAL:     return "INVLPGB_ILLEGAL";
    case VMEXIT_INVPCID:             return "INVPCID";
    case VMEXIT_MCOMMIT:             return "MCOMMIT";
    case VMEXIT_TLBSYNC:             return "TLBSYNC";
    case VMEXIT_BUS_LOCK:            return "BUS_LOCK";
    case VMEXIT_IDLE_HLT:            return "IDLE_HLT";
    case VMEXIT_NPF:                 return "NPF";
    case VMEXIT_AVIC_INCOMPLETE_IPI: return "AVIC_INCOMPLETE_IPI";
    case VMEXIT_AVIC_NOACCEL:        return "AVIC_NOACCEL";
    case VMEXIT_VMGEXIT:             return "VMGEXIT";
    case VMEXIT_SW:                  return "SW";
    case VMEXIT_INVALID:             return "INVALID";
    case VMEXIT_BUSY:                return "BUSY";
    case VMEXIT_IDLE_REQUIRED:       return "IDLE_REQUIRED";
    case VMEXIT_INVALID_PMC:         return "INVALID_PMC";
    default:                         return "UNKNOWN";
    }
}

/* Function to print all bits of v_intr_ctrl_t */
static inline void print_v_intr_ctrl(const char *file, int line,
                                     const struct vmcb *vmcb,
                                     const char *prefix)
{
    struct v_intr_ctrl_fields ctrl = vmcb->v_intr_ctrl.fields;

    printk("%s:%d: rax:%lx IRQ-ctrl", file, line, vmcb->rax);
    if (prefix)
        printk(" %s", prefix);
    printk(":");

    /* Print a list of the bits which are set: */
    if (ctrl.vIRQ_prio)
        printk(" prio:%u", ctrl.vIRQ_prio);
    if (ctrl.vector)
        printk(" vec:%x", ctrl.vector);
    if (ctrl.vGIF_enabled)
        printk(" vGIF:%u", ctrl.vGIF);
    if (ctrl.vIRQ_pending)
        printk(" IRQ:pending");
    if (ctrl.vNMI_enabled)
        printk(" NMI:enabled");
    if (ctrl.vNMI_pending)
        printk(" NMI:pending");
    if (ctrl.vNMI_blocking)
        printk(" NMI:blocking");

    if (ctrl.v_tpr)
        printk(" tpr=%u", ctrl.v_tpr);
    if (ctrl.v_ign_tpr)
        printk(" ign_tpr");
    if (ctrl.vIRQ_masking)
        printk(" IRQ:masking");
    printk("\n");
}

#endif /* XTF_NESTED_SVM_VMCB_H */
