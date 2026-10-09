# Squadron launch task scalar deletion: qualified Source candidate

The new cdecl Source function captures the actual task pointer, invokes the
admitted nine-input task cleanup, then reads the actual caller flags byte once.
Mask `0x01` selects current `std::free` on the captured task. Both normal paths
return the retained opaque pointer value without dereferencing it. The result
may already refer to freed storage and does not establish a live object.

This is an unregistered, uncompiled candidate. Only its header, implementation,
this document and report change. There is no worker CMake, ledger or Ghidra
change, build, test, probe, consumer or new admission/Original ABI credit.

## Actual inputs and late flag read

`scalar_delete_native_squadron_launch_task_007f1ee0` retains the cleanup function's
nine explicit inputs in the same order, followed by
`volatile std::uint8_t& actual_deletion_flags_low_byte`. It returns `void*`, is
explicitly `__cdecl`, and deliberately lacks `noexcept`.

The nine cleanup inputs are the actual Task base, volatile Task+0 profile cell,
Task+20h observer-owner prefix, actual Task+34h/member+14h endpoint cell, retained
observer lifetime, actual shared pending owners and producer access, and actual
pending registry/manager publication references. The candidate passes those
identities directly to `cleanup_native_squadron_launch_task_007f1e70`; it does
not derive offsets, copy owner/header/service storage or invent a task type.

The flags reference names actual caller-supplied low-byte backing. Its value is
not captured at entry. Cleanup may change it; the single volatile read occurs
only after cleanup returns normally. The predicate is exactly
`(late_flags & 0x01u) != 0u`, meaning bit zero/mask one. Other bits have no effect.
No flags write, full-word read, fake flags receiver or callback is added.

The actual flags cell must remain valid through this late read. All current
task-cleanup backing and service/domain requirements remain caller conditions.
If deletion is selected, the captured pointer must still be eligible for the
current Source CRT's `free`, with compatible allocation provenance and no
prior incompatible release. The wrapper allocates no replacement object or
heap/lifetime domain and adds no validation or recovery behavior.

## Source behavior and failure boundary

| Phase | Behavior |
| --- | --- |
| Capture | Retain `actual_task_base` as the task identity used by cleanup, optional free and the returned value. |
| Cleanup | Invoke the actual admitted Source99 with all nine real inputs. Its member/base guards and qualified C++ exception policies remain in force. |
| Flags | On normal child return only, read the actual volatile byte once. |
| Optional free | If mask `0x01` is set, call current `std::free(captured_task)`. |
| Return | Return the retained pointer value without dereferencing it, including after free. |

If cleanup throws, terminates or otherwise does not return normally, the wrapper
does not reach the flags read or its free call. It adds no catch, finally-style
free, retry, exception translation or `noexcept` promise. It does not modify the
admitted task cleanup's own failure handling.

The existing tick-subnode scalar helper is useful context for late flag
selection and opaque return semantics. Its naked fastcall interface is a
different Source contract. This candidate calls the real nine-input HLL cleanup
normally; it does not force that function into a Native ECX-only call or declare
a raw-call stub.

## Complete raw body and incomplete saved listing

Root accepted the full raw `007F1EE0..007F1EFD` window: **30 bytes / 11 operations**,
SHA-256 `61c4f6a149a32ebf0082d04b56f3bc34fdd1f78355cd94a3711f810ff5095d18`.
The current saved listing has **10 starts** and omits
`007F1EF5: ADD ESP,4` after the Native `00BF65AC` free call. The worker verifies
all owned raw bytes against the original executable and the accepted gate;
it does not call the saved listing complete.

The Native wrapper saves ESI, captures ECX, calls `007F1E70` without pushed
arguments, then tests the current low byte of its original flags argument.
The free branch pushes current ESI, calls `00BF65AC`, and executes the raw ADD.
Both branches move current ESI to EAX, restore ESI and end in `RET 4`. Child
preservation and stack balance remain requirements. The Source wrapper instead
retains its explicit task pointer and forwards actual storage/service references;
it does not reproduce the Native register, flagword, stack/frame or alias ABI.

The indexed and accepted metadata name remains the correct compiler-generated
`CG_scalar_deleting_dtor_007f1ee0`; the Native free label remains `_free`. The
candidate's C++ symbol is separate. No name, flow, NoReturn property, listing,
function extent or Ghidra project is changed. The omitted start alone does not
establish the exact cause or any NoReturn property. Native allocator/free and
exception behavior remain separate from the current Source CRT policy.

## Verification and primary review

The report pins the actual Source99 header/implementation and its admitted
Source97 build receipt, relevant scalar-readiness evidence and tick-scalar
context. All 97 current build-input identities replay; prior compiled artifacts
remain historical rather than evidence for this candidate. That normal build
ran `2026-10-09T17:59:46Z..18:00:05Z` and passed three existing checks.

The primary must register/build the candidate and inspect the entire emitted
object, calls/imports, sections/relocations, actual Core definition and application
map. No Source instruction-size, register/flag, Original ABI, allocation-domain,
production binding or gameplay equivalence is claimed by this packet.
