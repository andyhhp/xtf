/**
 * @file arch/arm/include/arch/config.h
 *
 * A Linux-style configuration list.
 */
#ifndef XTF_ARM_CONFIG_H
#define XTF_ARM_CONFIG_H

#define XTF_VIRT_START 0x40000000

#if defined(CONFIG_ENV_arm64)
#define CONFIG_ARM              1
#define CONFIG_ARM_64           1
#define CONFIG_64BIT            1
#define ENVIRONMENT_DESCRIPTION "ARM64"
#undef CONFIG_ENV_arm64
#elif defined(CONFIG_ENV_arm32)
#define CONFIG_ARM              1
#define CONFIG_ARM_32           1
#define CONFIG_32BIT            1
#define ENVIRONMENT_DESCRIPTION "ARM32"
#undef CONFIG_ENV_arm32
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
