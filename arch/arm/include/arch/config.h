/**
 * @file arch/arm/include/arch/config.h
 *
 * A Linux-style configuration list.
 */
#ifndef XTF_ARM_CONFIG_H
#define XTF_ARM_CONFIG_H

/*
 * On MMU less system, when using XTF as dom0 we need to know the load address
 * as it may differ depending on the target. Allow specifying the load address
 * on the command line when invoking make using:
 * CONFIG_LOAD_ADDRESS=<address>
 */
#if defined(CONFIG_LOAD_ADDRESS)
#define XTF_VIRT_START CONFIG_LOAD_ADDRESS
#else
#define XTF_VIRT_START 0x40000000
#endif

#if defined(CONFIG_ENV_64le)
#define CONFIG_ARM_64           1
#define CONFIG_64BIT            1
#define CONFIG_LE               1
#define ENVIRONMENT_DESCRIPTION "ARM64 LE"
#elif defined(CONFIG_ENV_32le)
#define CONFIG_ARM_32           1
#define CONFIG_32BIT            1
#define CONFIG_LE               1
#define ENVIRONMENT_DESCRIPTION "ARM32 LE"
#else
#error "Bad environment"
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
