#ifdef LINKER_HEADER

#ifndef __riscv
# error Bad architecture to link with
#endif

#if __riscv_xlen == 32
OUTPUT_FORMAT("elf32-littleriscv")
#elif __riscv_xlen == 64
OUTPUT_FORMAT("elf64-littleriscv")
#else
# error Bad __riscv_xlen to link with
#endif

OUTPUT_ARCH(riscv)
ENTRY(_start)

#endif /* LINKER_HEADER */
