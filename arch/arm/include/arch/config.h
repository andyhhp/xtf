/**
 * @file arch/arm/include/arch/config.h
 *
 * A Linux-style configuration list.
 */
#ifndef XTF_ARM_CONFIG_H
#define XTF_ARM_CONFIG_H

#include <arch/mm.h>

#if defined(CONFIG_ENV_mmu64le)
#define CONFIG_ARM_64           1
#define CONFIG_64BIT            1
#define CONFIG_LE               1
#define CONFIG_MMU              1
#define ENVIRONMENT_DESCRIPTION "ARM64 LE MMU"
#elif defined(CONFIG_ENV_32le)
#define CONFIG_ARM_32           1
#define CONFIG_32BIT            1
#define CONFIG_LE               1
#define ENVIRONMENT_DESCRIPTION "ARM32 LE"
#else
#error "Bad environment"
#endif

#ifdef CONFIG_MMU
#define XTF_VIRT_START VA_START
#else
#define XTF_VIRT_START 0x40000000
#endif

#endif /* XTF_ARM_CONFIG_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
