#include <xtf/types.h>

#include <arch/x86/config.h>

#include <xen/xen.h>

#ifdef CONFIG_ENV_pv
/* Filled in by head_pv.S */
start_info_t *start_info = NULL;
#endif

void arch_setup(void)
{

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
