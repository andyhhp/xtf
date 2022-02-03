#ifdef LINKER_HEADER

#if defined(__arm__)

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

#elif defined(__aarch64__)

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

#else
# error Bad architecture to link with
#endif

ENTRY(_start)

#endif /* LINKER_HEADER */
