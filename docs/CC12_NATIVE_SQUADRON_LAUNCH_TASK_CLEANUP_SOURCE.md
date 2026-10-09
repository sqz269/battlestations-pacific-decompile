Primary compiled review: Registered task cleanup public body is185 bytes/65 instructions. Its complete12-function514-byte/187-instruction object and68/36/36-byte EH sections were reviewed. Member then base cleanup runs on a compatible endpoint-cleanup failure; only base remains armed once normal member cleanup begins. Each guard is disarmed before its normal child call. Current C++ termination on cleanup failure during unwind remains qualified.

Normal MSVC Win32 build 2026-10-09T17:59:46.563985+00:00 to 2026-10-09T18:00:05.036630+00:00 passed three existing checks. The shared receipt pins97 Source/build inputs and four artifacts, replays all19 prior reviewed objects unchanged and captures two unchanged existing pool-provider objects plus the two new objects (23 total). Core contains27 selected positive roots, including the two existing getter overloads; this helper uses the raw-manager overload only. New public roots are absent from the application map. No Native ABI, selected production binding, startup or gameplay credit. The candidate section below is an immutable worker-time snapshot; its Source88 build artifacts are historical.

# Squadron launch task cleanup: qualified Source candidate

The candidate composes the admitted endpoint-loop, observer-member and tick-base
cleanup providers through explicit actual-storage references. It writes task
profile `00D08AE4`, captures the endpoint once for the optional loop, then runs
member cleanup and base cleanup. Member cleanup receives the actual endpoint
cell and performs its own later read. Separate guards preserve that order on
Source exceptions, and each guard is disarmed before its normal child begins.

This is an unregistered, uncompiled Source candidate for primary review. The
worker changes only its header, implementation, this document and report. No
CMake, ledger or Ghidra mutation, new consumer, build, test or probe is included.
There is no new Source admission, Original ABI, startup or gameplay credit.

## Explicit interface and binding conditions

```cpp
void cleanup_native_squadron_launch_task_007f1e70(
    void* actual_task_base,
    volatile std::uint32_t& actual_task_profile_00,
    NativeObserverOwnerStorage& actual_member_owner_20,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell_34,
    NativeObserverLifetime& retained_lifetime,
    NativePendingEntityOwners& actual_pending_owners,
    NativePendingEntityProducerAccess& actual_pending_access,
    void* volatile& pending_flag_publication,
    void* volatile& pending_manager_publication);
```

All inputs must identify one actual task and its real service domains. The base
pointer identifies Task; the profile reference is Task+0; the existing 16-byte
observer prefix is Task+20h; the endpoint cell is both Task+34h and member+14h.
The function assumes those identities and does not calculate or validate them.
It introduces no 38h task type, 18h member type, offset-derived cast, allocation,
copied owner/cell, initialization or production adopter.

A captured endpoint is also the raw receiver for the admitted Source70 loop.
Its typed observer prefix alone does not establish that loop's wider backing:
the live signed count at +3CCh, inline entries beginning +3D0h, final byte +3ECh,
and each selected entity's required fields and providers must remain usable.
There is no fabricated capacity or implicit entity class. Actual pending owners
must be initialized and shared; producer access must map actual entity identities
to their live fields and real current providers.

Member cleanup borrows the existing owner/cell and retained observer lifetime.
Base cleanup receives the same Task base and actual pending registry/manager
publication references. Publication values can change, while their cell
addresses, service/domain identities and all reached storage remain valid.
Keep these bindings alive through callbacks, recursive work and failure cleanup,
using the existing child contracts. No private substitute domain is created.

## Normal order and two distinct endpoint reads

1. Write `00D08AE4` through the volatile task-profile reference.
2. Read the volatile endpoint cell once into the outer captured pointer.
3. Construct the base guard, then the member guard, both initially armed.
4. If the outer capture is nonnull, call Source70 on that pointer with the real
   pending owners/access. Its unused EDX placement parameter is `0u`.
5. Disarm the member guard **before** normal Source88 member cleanup. Pass the
   actual cell reference; Source88 stamps its member profile and rereads that
   cell. A change during the endpoint loop is therefore visible to this child.
6. Disarm the base guard **before** normal Source84 base cleanup, passing the
   same Task base and actual publication references. Its unused EDX placement
   parameter is also `0u`.

A null outer capture skips only Source70. Member and base cleanup still run.
Neither guard caches endpoint or publication *values* for its later cleanup:
the member guard retains the actual cell reference; the base guard retains the
actual publication references and Task pointer. The extra Source arguments,
ordinary C++ call ABI and explicit unused placement values do not preserve
Native incoming register values or hidden-register behavior.

## Guard lifetime and Source exception policy

The guards are private, noncopyable objects. Their constructors only retain
the actual inputs and arm private Source booleans; construction and disarming
are nonthrowing. The base guard is constructed first so normal C++ unwinding
reaches the member guard first.

| Failure | Armed cleanup reached under current C++ unwinding |
| --- | --- |
| Optional Source70 loop throws | Member guard invokes Source88 using the actual cell. If that cleanup returns, base guard invokes Source84. |
| Normal Source88 member cleanup throws | Member guard is already disarmed; base guard invokes Source84. Source88 itself owns its approved unregister/owner-destruction failure policy. |
| Normal Source84 base cleanup throws | Both guards are already disarmed; neither normal child is retried. |
| Any guard-invoked cleanup throws | The guard destructor is explicitly `noexcept`, so current C++ termination policy applies. Later cleanup is not guaranteed after termination. |

The public function deliberately lacks `noexcept`. There is no new catch,
exception translation, fallback callback, rollback or guaranteed final field
write. Source88 already disarms its own guard before normal owner destruction,
whose provider frees the current array on its Source catch path; this outer
composition does not repeat that destruction. Source84 retains its admitted
policy, including no newly added local unwind release. None of these Source
choices establishes Original double-exception or frame-handler behavior.

## Accepted Native evidence

The complete accepted `007F1E70..007F1ED2` body is 99 bytes / 26 operations:
SHA-256 `c87c2a32db8f6b74332559fed6393c4501089bef0af42b1ffec175617c664731`.
The primary gate and accepted readiness report cover its profile-before-capture
order, optional endpoint call, member/base calls and final plain RET. The
worker replays only this owned body's identity against the original executable.
The 99-byte count is not a claim about emitted Source size.

The Native state stores are full DWORD one, then **one byte** zero, then full
DWORD `FFFFFFFF`. The byte write preserves the current upper 24 bits, including
possible child/alias effects. Native receiver formation precedes its following
state store. The final saved-chain read precedes the ESI pop. Those exact state,
register, frame and alias facts stay in the accepted Native evidence; this C++
composition supplies no FS frame or byte-state emulation.

The accepted task EH audit covers 119 code bytes / 42 operations and 140 selected
data bytes across three related handlers, actions and the shared adapter. For
this cleanup, descriptor `DBFE28` selects map `DBFE18` with records
`(-1,C8F3D0)` and `(0,C8F3D8)`. Both actions reload current `[EBP-10h]`; one tails
to base cleanup, the other adds 20h and tails to `006569A0`. The later five-byte
tail audit proves `006569A0 -> 00653390`. This completes the direct target
relationship without proving interpreter selection, EBP origin, late saved-word
identity, exception dispatch or failure-during-unwind semantics.

The approved Source guards retain explicit actual references and their own
booleans. They do not follow arbitrary mutable Native frame words. No register,
flag, stack, EH-layout, callable-profile, final-profile, EAX-result or binary
replacement claim follows from this composition.

## Verification and remaining primary work

The report pins the three current provider headers/implementations, relevant
Native/EH receipts and the admitted Source88 build receipt. All 88 input
identities from that build match the current checkout. The report records a
compact identity-replay digest and counts, referring to the immutable receipt
instead of copying its object graphs or earlier input arrays.

That prior normal build ran at `2026-10-09T17:23:29Z..17:23:47Z`, passed three
existing checks and recorded 19 whole objects / 22 positive Core public roots.
It predates this candidate. The primary must register/build the candidate and
review all emitted code, new EH sections/relocations and actual Core definition
and application map. Actual production task binding and Original ABI/gameplay
validation remain separate obligations.
