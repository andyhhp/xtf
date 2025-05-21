/*
 * Xen x86 public cpuid interface
 */

#ifndef XEN_PUBLIC_ARCH_X86_CPUID_H
#define XEN_PUBLIC_ARCH_X86_CPUID_H

/*
 * For compatibility with other hypervisor interfaces, the Xen cpuid leaves
 * can be found at the first otherwise unused 0x100 aligned boundary starting
 * from 0x40000000.
 *
 * e.g If viridian extensions are enabled for an HVM domain, the Xen cpuid
 * leaves will start at 0x40000100
 */

#define XEN_CPUID_FIRST_LEAF 0x40000000

/*
 * Leaf 1 (0x40000x00)
 * EAX: Largest Xen-information leaf. All leaves up to an including @EAX
 *      are supported by the Xen host.
 * EBX-EDX: "XenVMMXenVMM" signature, allowing positive identification
 *      of a Xen host.
 */
#define XEN_CPUID_SIGNATURE_EBX 0x566e6558 /* "XenV" */
#define XEN_CPUID_SIGNATURE_ECX 0x65584d4d /* "MMXe" */
#define XEN_CPUID_SIGNATURE_EDX 0x4d4d566e /* "nVMM" */

/*
 * Leaf 3 (0x40000x02)
 * EAX: Number of hypercall transfer pages. This register is always guaranteed
 *      to specify one hypercall page.
 * EBX: Base address of Xen-specific MSRs.
 * ECX: Features 1. Unused bits are set to zero.
 * EDX: Features 2. Unused bits are set to zero.
 */

/*
 * Leaf 5 (0x40000x04)
 * HVM-specific features
 * Sub-leaf 0: EAX: Features
 * Sub-leaf 0: EBX: vcpu id (iff EAX has XEN_HVM_CPUID_VCPU_ID_PRESENT flag)
 * Sub-leaf 0: ECX: domain id (iff EAX has XEN_HVM_CPUID_DOMID_PRESENT flag)
 */
#define XEN_HVM_CPUID_VCPU_ID_PRESENT  (1u << 3) /* vcpu id is present in EBX */
#define XEN_HVM_CPUID_DOMID_PRESENT    (1u << 4) /* domid is present in ECX */

#endif /* XEN_PUBLIC_ARCH_X86_CPUID_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
