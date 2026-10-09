Primary compiled review: Registered observer cleanup uses explicit actual storage/cell/lifetime references. Its115-byte public body disarms the guard before normal destruction; the complete285-byte/seven-function object and both52/36-byte EH sections were reviewed. Original FS/frame/register ABI, interpreter policy, production task binding and gameplay remain unproved.

Normal MSVC Win32 build 2026-10-09T17:23:29.234687+00:00 to 2026-10-09T17:23:47.076256+00:00 passed three existing checks. The shared receipt captures88 inputs,19 whole objects and22 unique public Core roots. All16 prior objects and the existing67-function observer provider are byte-identical, including EH payloads and indexed relocations. New public roots are absent from the application map; no consumer or forced retention was added. The candidate section below is an immutable worker-time snapshot; its older build artifacts are historical.

# Observer member cleanup: qualified Source candidate

`cleanup_native_observer_member_00653390` composes the existing observer methods
through three explicit references: the actual `NativeObserverOwnerStorage`
prefix, its separate volatile endpoint-pointer cell, and the retained
`NativeObserverLifetime`. It stamps `00CF6494`, captures the endpoint once,
conditionally unregisters that captured endpoint against the actual owner,
and destroys the owner. A private armed guard performs owner destruction if
unregister throws. The guard is disarmed before normal destruction begins,
so a failure in normal destruction is never retried.

This is an unregistered, uncompiled Source candidate for primary review. The
worker changes only the header, implementation, this document and its report.
There is no worker CMake, ledger or Ghidra change, new consumer, build, test or
probe. No Source admission, Original ABI, startup or gameplay credit is added.

## Interface and actual storage

The public function is:

```cpp
void cleanup_native_observer_member_00653390(
    NativeObserverOwnerStorage& actual_member_owner,
    NativeObserverOwnerStorage* volatile& actual_endpoint_cell,
    NativeObserverLifetime& retained_lifetime);
```

`NativeObserverOwnerStorage` is the existing 16-byte prefix: volatile profile
DWORD followed by the actual edge pointer, count and capacity. The new function
introduces no encompassing 24-byte member, task type, copied owner, copied cell,
storage initialization or task/member address conversion. The endpoint cell is
outside this prefix's contract. Its actual identity, backing and association
with the member remain caller obligations. A nonnull captured pointer must
identify the actual already-adjusted first endpoint, not a proxy or metadata.

The retained lifetime is a distinct Source service object. It borrows the
existing manager domain, mutable lock and dispatch publication cells, and real
observer services. It is not reconstructed from either endpoint's address.
Current `GameObserverRuntime` already retains such a service and its publication
cells; that existing domain does not establish a binding for this selected raw
member or task. Keep the same actual owner/endpoint identities, allocated array
spans, live dispatch/section backing, publication-cell addresses and service
lifetime valid through reached calls and failure cleanup. No validity guard or
corruption-recovery behavior is added by this candidate.

## Source schedule and failure paths

| Phase | Candidate behavior |
| --- | --- |
| Initial mutation | Write the owner's volatile profile DWORD `00CF6494` before the endpoint-cell read. The value is an opaque Native numerical profile, not a callable Source vtable. |
| Endpoint selection | Read the actual volatile endpoint cell once and retain that value for the null check and optional unregister call. There is no later cell reread or clearing. |
| Unregister phase | Arm a noncopyable private guard borrowing the actual owner and retained lifetime. For a nonnull captured endpoint call `unregister_pair_006952a0(*captured_endpoint, actual_member_owner)`. |
| Normal destruction | Disarm the guard before calling `destroy_callback_owner_00695870(actual_member_owner)`. A null endpoint still reaches this call. |
| Unregister exception | The armed guard calls owner destruction once. If that succeeds, the original C++ exception continues unwinding. |
| Cleanup exception during unwind | The guard destructor is explicitly `noexcept`; a throwing owner destruction invokes current C++ termination policy. This is an approved Source policy, not a recovered Native double-exception rule. |
| Normal destruction exception | The guard is already disarmed. The existing child handles its own Source cleanup and propagates; the candidate does not retry it. |

The public function deliberately lacks `noexcept`. The guard constructor and
disarm operation only manipulate private Source references/state and are
nonthrowing. It is armed after endpoint capture and covers only the conditional
unregister phase. The Source guard is not Native local-this storage, an FS
registration, or a recovered EH state slot. No extra catch, exception conversion,
endpoint reset, array reset, owner allocation/deletion or final-profile guarantee
is introduced. Hardware faults, arbitrary frame aliases and invalid object
lifetimes are outside this C++ composition contract.

## Existing children and the no-retry boundary

Current `unregister_pair_006952a0` acquires the shared lock, uses the separately
nested pair lookup, invalidates dispatch slots, decrements the found edge's
reference count and removes/deletes it when zero. A missing edge returns. The
actual endpoint addresses are used by lookup and removal. This Source method
does not itself destroy the callback owner when it throws.

Current `destroy_callback_owner_00695870` writes profile `00CE3CD4`, takes outer
and nested locks, captures count, optionally detaches all captured edges, then
frees the current array after unlocking. Its catch path also frees the current
array and rethrows. It leaves array pointer/count/capacity bytes as they stand
and does not free the owner. Therefore repeating destruction after this normal
child call begins can operate on retained freed-pointer bytes; disarming before
the call is essential.

Child behavior remains qualified Source policy. Guards use current Win32
locking; array free reaches the current CRT; validation uses the existing
return-capable SDK route. Canonical edge profile `00CF7E64` uses the admitted
Source deletion implementation; other profiles use services, and the current
game service rejects unsupported profiles by throwing. This candidate neither
expands those bindings nor proves Native virtual-target, register, CRT or
exception equivalence.

## Accepted Native evidence and limits

The accepted complete owner body `00653390..006533E7` is 88 bytes / 24 operations,
with SHA-256
`83a09c9e3d3c3f6006cf2d3aab805129a3f0d58cb21584f49c720c963dc737ea`.
Its direct sequence stamps member profile `00CF6494`, captures current member
`+14h`, writes full state zero, optionally calls unregister with the captured
endpoint/member register pair, resets state to `-1`, and calls owner destruction.
The Source interface explicitly separates the typed prefix from the external
cell instead of deriving `+14h` from an invented larger C++ object.

The primary separately accepted all 18 code bytes / four operations and 44
data bytes across handler `00C7AF48`, descriptor `00DA7B14`, map `00DA7B0C` and
action `00C7AF40`. The action loads current ECX from current `[EBP-10h]` and
tails `00695870`; its two owned instructions do not write stack or arithmetic
flags. Framework selection, the interpreter's EBP basis and original failure
policy remain unproved. The primary expressly approved the Source guard policy
above with those limits. No worker Native handler or interpreter body was opened.

The candidate is ordinary C++ composition. It does not preserve Native FS/frame
layout, local saved-word aliases, volatile-register results, flags, stack argument
placement, body size or instruction identity. The 88-byte Native count is not an
emitted Source size. Its `void` result carries no common EAX guarantee. Existing
Source84 and Source70 interfaces likewise keep their extra-argument and provider
policies separate from Native ABI admission; they do not bind this function into
a raw task or make the new Source guard an Original EH implementation.

The report pins current actual provider excerpts, both primary gates, the
accepted Native88 record, current observer audits, and all 80 inputs from the
latest accepted Source build receipt. That prior registered build and its three
existing checks predate this candidate. No compiled object or application
behavior is established here. The primary will register/build and inspect the
entire emitted object, its new EH data and relocations, actual Core definition,
and application map before Source admission. Production binding and Original
ABI/gameplay validation remain separate work.
