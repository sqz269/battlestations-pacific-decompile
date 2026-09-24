# Sampler diagnostic exception policy, CC10

The revised private helper allows the debuggee's own handlers to decide unowned
first-chance exceptions by continuing with `DBG_EXCEPTION_NOT_HANDLED`.
Second-chance exceptions close the owned process. Entry/marker addresses,
primary-thread loader stages and active DR0/DR1 traps retain their strict
protocol checks. This follows the documented
[ContinueDebugEvent exception semantics](https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-continuedebugevent).

The shared policy records the complete exception record fields, requested native
and WOW64 contexts, bounded code/stack windows, current mapped regions and the
original helper's existing loaded-module hash records. Read/query failures are
explicit. These are observations, not a reconstructed caller stack or full
architectural/XSTATE preservation claim. Diagnostics perform no context writes.

Two owned x86 TLS fixtures execute before their PE entry point. The handled
`C0000008` case reaches its own SEH handler, then passes entry restoration,
both hardware stops, stack/code/debug-register comparisons and ordinary exit0.
The unhandled case produces first and second chance, restores the entry patch
and closes diagnostically with exit125. Both inner and outer jobs reach zero
active processes. The final shared-policy fixture run passed; earlier failures
are retained, including a job-accounting retirement race corrected by a bounded
poll in the fixture helper.

The fixtures use x86 KernelBase exception dispatch. They do not reproduce the
original run01 native64 ntdll cause, additional-thread exceptions or every
unowned debug notification. The new original helper was compiled but not run
for this packet; it inherits the reviewed admission, profile-backup, owned-job,
entry restoration and capture boundaries. Controller wiring and any further
original attempt require separate concrete review. The initial
[inconclusive original attempt](NATIVE_SAMPLER_ORIGINAL_CAPTURE_CC10.md) remains
unchanged evidence and supplies no sampler value.

`reports/native_sampler_exception_policy_cc10.json` pins all109 frozen artifacts,
the compiled64-bit helper, shared source and both final fixture receipts. No
original launch/attach, installed-game edit, game startup or sampler-input proof
is claimed by this packet.
