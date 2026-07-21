Nested SVM VMLOAD/VMSAVE Test
=============================

This test exercises the architectural VMLOAD and VMSAVE failure cases
that are reachable from an hvm64 L1 guest using Xen nested SVM.

What It Verifies
----------------

The test verifies the distinct VMLOAD and VMSAVE error classes that are
reachable from this hvm64 harness:

* VMLOAD/VMSAVE with EFER.SVME clear.
* VMLOAD/VMSAVE executed at CPL > 0.
* VMLOAD/VMSAVE executed with malformed VMCB physical addresses in RAX.

How The Verification Functions Work
-----------------------------------

``test_main()`` supplies the VMLOAD and VMSAVE-specific matrices
to the runner and checks that Xen reports the same exceptions that
the AMD architecture defines for the same preconditions, within the
limits of an hvm64 long-mode harness.

``svm_negative_check_cases()`` toggles EFER.SVME as required
by each subtest, dispatches the instruction in kernel or user
context, and restores the original EFER value afterwards.

``stub_vmload()`` and ``stub_vmsave()`` execute the instructions
directly in L1 with caller-supplied RAX values and record any
faults via the XTF exception-table helpers.
