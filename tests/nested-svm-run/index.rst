Nested SVM VMRUN
================

This test exercises a minimal AMD nested-SVM guest entry path on hvm64.

An L1 guest enables SVM, builds an L2 VMCB that reuses L1's page tables and
descriptor tables, mirrors the active TR and LDTR state into the VMCB, and
enters L2 with VMRUN.  The L2 payload runs on its own stack, writes a sentinel
into shared memory, and halts.

The test passes when L1 observes a HLT VMEXIT from L2 and the expected
sentinel value.

What It Verifies
----------------

The test verifies that Xen's nested-SVM implementation accepts a minimal but
architecturally valid L2 VMCB built by an L1 guest running in long mode.

More specifically, it verifies that:

* L1 can enable SVM and program the host-save area needed by VMRUN.
* L1 can populate an L2 VMCB with inherited control state, descriptor-table
	state, and system-segment state taken from the live L1 environment.
* VMRUN succeeds in entering L2 rather than failing immediately because of
	malformed guest state.
* L2 executes the supplied payload on its own stack, updates shared memory,
	and exits through HLT.
* L1 receives a VMEXIT with exit code VMEXIT_HLT and observes the sentinel
	value written by L2.

How The Verification Functions Work
-----------------------------------

The verification logic is split between the helpers that build a
valid VMCB and the final checks performed after returning from VMRUN.

``test_main()`` performs the end-to-end verification.  It enables SVM,
writes the host-save area address to MSR_VM_HSAVE_PA, builds the VMCB,
and enters L2 through ``svm_vmrun()``.

When execution returns to L1, the test checks two conditions: the VMEXIT
reason must be ``VMEXIT_HLT``, and the shared handshake value must match
``L2_SENTINEL``.  Both checks must pass for the test to report success.

``l2_entry()`` is the L2 payload.  It avoids using VMMCALL, because
in Xen's nested-SVM model that would unconditionally cause a VMEXIT
to L1.  Instead, it writes a known sentinel value into shared memory
and halts in a loop so that L1 sees a clean HLT exit reason.

``build_l2_vmcb()`` prepares the nested guest state.  It programs the
required intercepts, reuses L1's paging and descriptor-table state,
assigns the L2 RIP and stack, and copies LDTR and TR from the current
GDT into the VMCB.

The helper ``vmcb_set_seg_desc()`` translates an L1 selector into the
VMCB segment format, while rejecting selectors that are null, LDT-based,
or out of bounds for the current GDT limit.  This matters because VMRUN
consumes the VMCB's segment state directly, including the system
descriptors needed for long-mode execution.
