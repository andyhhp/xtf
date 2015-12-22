#ifndef XTF_FRAMEWORK_H
#define XTF_FRAMEWORK_H

/* To be implemented by each arch */
void arch_setup(void);
void test_setup(void);

/* Single line summary of execution environment. */
extern const char *environment_description;

#endif /* XTF_FRAMEWORK_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
