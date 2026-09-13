# Native pending entity owners

Addresses: `00CD3910`, `00CD3940`, `00CDF4A0`, `00CDF4B0`, `00924A50`, `00924A70`.
Contracts: `00926C80`, `00926D90`, `00925A00`.

This component supplies the actual adjacent raw owners at `00F899A8` and
`00F899B4`, explicit CRT initialization, and borrowed-node teardown. It does
not initialize either list implicitly or supply producers, cancellation,
event delivery, a replacement exit registry, or a private empty runtime queue.

| Owner | Untouched word | Sentinel pointer | Count | Node payload |
| --- | --- | --- | --- | --- |
| destroy | `F899A8` | `F899AC` | `F899B0` | borrowed entity pointer |
| kill | `F899B4` | `F899B8` | `F899BC` | borrowed entity pointer |

Each owner is 0Ch; each node is 0Ch with next/previous/payload at 0/4/8.
`00924B10` writes all three node fields from its three stack arguments, and
returns with `RET 0Ch`. Both producers append before the sentinel. The raw
types alias `NativeEffectDeletionNode` and `NativeEffectDeletionListStorage`,
whose layout and allocator ownership match exactly; there is no STL port.

| Routine | Native ABI and range | Coverage |
| --- | --- | --- |
| `CD3910` | no inputs, EAX `_atexit` result, `RET`; `[CD3910,CD3935)` | complete source |
| `CD3940` | no inputs, EAX `_atexit` result, `RET`; `[CD3940,CD3965)` | complete source |
| `CDF4A0` | no inputs; ECX=`F899A8`, tail jump; `[CDF4A0,CDF4AA)` | complete source |
| `CDF4B0` | no inputs; ECX=`F899B4`, tail jump; `[CDF4B0,CDF4BA)` | complete source |
| `924A50` | ECX unconsumed; EAX sentinel, `RET`; `[924A50,924A6A)` | complete through proven canonical source reuse |
| `924A70` | ECX raw list, `RET`; `[924A70,924AB8)` | complete through proven canonical source reuse; stored Ghidra flow partial |
| `926C80`, `926D90`, `925A00` | described below | contracts only; no new source implementation |

`CD3910`/`CD3940` allocate and self-link a sentinel, publish its address at
owner+4, publish count zero at owner+8, and call `_atexit` with the **actual**
shutdown identity `CDF4A0`/`CDF4B0`. The call's integer result is returned
unchanged. The `POP ECX` after each call consumes its single argument and
does not change EAX. Failed registration leaves the initialized owner intact.
If allocation throws, neither head nor count has been published, and no
registration occurs. The allocator word and the sentinel payload retain their
preimages. Reinitializing a live owner is not supported by a safety guard in
the native code and is not made into a no-op here.

`NativePendingEntityCrtRegistration` must bind the actual CRT registration
domain and dispatch each shutdown identity against these same owner bytes.
Its context and the owners must survive the eventual callbacks. A missing
callback is rejected before storage mutation as a source-interface binding
error. No default, success stub, or local callback stack exists in this module.

Teardown captures the first node, self-links the sentinel and zeros the count
before freeing any node. It reads the next pointer before each free, compares
against the current owner head, frees the sentinel last, and clears owner+4.
Payloads are never deleted. Owner+0 and the other pending list are untouched.
It requires an initialized intact finite ring, does not lock, and is not
idempotent on a null head. It does not deliver pending entity events.

## Source reuse and stored-flow limits

All eight complete spans (290 bytes) match the live verified `bsp` program
and installed executable. Full `924A50[26]` and `4C3200[26]` instructions are
identical after converting their relative CALL operands to actual target
addresses; both call `BF681B`. Full `924A70[72]` and `4C5940[72]` likewise
match and call `BF65AC` twice. Branch offsets, all other bytes, instruction
boundaries, and final RETs agree. This establishes reuse of the actual
canonical allocation/free implementation, beyond merely matching layouts.

`BF681B` retries malloc through the CRT new handler and throws on failure;
`BF65AC` is the free thunk. Canonical source delegates to
`singleton_lifetime_allocate`/`singleton_lifetime_free`, retaining that
allocation/free boundary. There is no separate pending-list allocator.

The retained Ghidra body of `924A70` ends at `924AAB`; `924A98`, `924AAC`,
and `924AB7` still have no containing function. Its complete original-byte
source proof includes both post-free continuations, loop, head clear, and RET.
The canonical `4C5940` body similarly ends at `4C597B`; its final RET is
`4C5987` (exclusive end `4C5988`). The earlier constructor report's inclusive
`4C5988` wording is an off-by-one boundary description. This packet performs
no Ghidra mutations and claims no complete stored-flow repair.

## Producer and cancellation contracts

These contracts refer to the actual raw owners, rather than a detached count
or substitute container. They are future integration requirements, not
behavior supplied by this component. All three routines obtain `9248D0`'s
lock owner, capture owner+4, enter its critical section when non-null and
adjust the explicit recursion word at section+18h. They use that captured
section to leave. Callback/SEH equivalence is outside this packet.

`926C80` (`__thiscall(entity,recurse)`, `RET 4`, last instruction `926D88`)
does nothing to the pending list when entity+60h is already set. Otherwise
it sets that byte before further work. If entity+70h is zero, it copies the
parent's +70h when parent+3Ch is non-null and parent+60h is set, or stores 1.
Only the **low byte** of recurse is tested at `926CF7`. When nonzero it walks
children through +48h/+44h, calls child slot+78h with the parent, and for a
nonzero AL result calls child slot+70h with 1. Immediately before that second
call it rereads parent+70h and clears child+70h if the parent value is zero;
the prior callback can change the parent. It then allocates a node via
`924B10(head,head->previous,&entity)` at `926D4E`, grows count by one via
`9267F0` at `926D5C`, and only then publishes both ring links at
`926D61`/`926D66`. The native argument-byte test is narrower than this branch's
existing `src/unit_damage.cpp` host projection's `recurse != 0`; that shared
source and its omitted conditional child-cause store are assigned to a root
follow-up.

`926D90` (`__thiscall(entity,cause)`, `RET 4`, last instruction `926E7B`)
maps cause 7 to stored cause 2 and retains the original argument for child
recursion. If +5Fh is clear it sets +5Fh first. When +60h is clear it writes
the mapped cause to +70h and calls entity slot+70h with 1. For original
cause 7 it writes +70h=2 again after this call. It recurses unconditionally
over children at +48h/+44h with the original cause, then allocates at
`926E40`, increments count at `926E4E`, and publishes the two tail links at
`926E53`/`926E58`. An existing +5Fh bypasses this whole block. In both
producers the flags are published before allocation; no rollback is inferred
from the normal path. `9267F0` checks the `3FFFFFFFh` maximum before count
growth, and its overflow exception path is not supplied by this component.

`925A00` (`__thiscall(entity)`, `RET`, `[925A00,925A8A)`) proceeds only if
the parent at +3Ch is null or the parent has +5Ch nonzero and +5Dh/+60h/+5Eh
all zero. Otherwise it releases the lock without changing entity flags or
either list. On success: if +5Fh is set, remove matching payload pointers
from the kill list with `781260` and clear +5Fh; clear +5Eh; if +60h is set,
remove matching pointers from the destroy list and clear +60h; finally clear
+5Dh. The +5Ch active byte is untouched. `781260` scans the entire ring,
unlinks/frees every matching node, and decrements count for each removal;
it never deletes the payload. Its omitted listing gap `7812D5[7]` is
`ADD ESP,4; ADD dword ptr [EBP+8],-1`, verified from live/disk bytes.
For both calls in the cancel path its `RET 4` consumes `&entity`.

These tail insertions and the existing drain's forward walk imply FIFO order,
but no frame/run-time claim is made here. See `docs/ENTITY_EVENT_QUEUES.md`
for the existing copy-clear-dispatch drain contract and its lack of a lock.

## Call evidence

| Site | Native callee | Original containing function | Operation |
| --- | --- | --- | --- |
| `CD3915`, `CD3945` | `924A50` | `CD3910`, `CD3940` | allocate sentinel |
| `CD392E`, `CD395E` | `BF6FF5` | `CD3910`, `CD3940` | actual CRT registration |
| `CDF4A5`, `CDF4B5` | `924A70` | `CDF4A0`, `CDF4B0` | tail jump to teardown |
| `924A52` | `BF681B` | `924A50` | raw allocation, `ADD ESP,4` |
| `924A93`, `924AA7` | `BF65AC` | `924A70` | node/sentinel free, `ADD ESP,4` |

Producer/cancel contract call sites are also recorded in the machine-readable
report, with exact containing function attribution. The four owner names are
hypotheses; no recovered symbols are claimed. Ghidra annotation requests are
left to the primary integrator because workers are read-only in Ghidra.

## Verification

`scripts/build.ps1` passed in MSVC Win32 Release, with both existing CTests
passing. The four native/source comparisons, source allocation-failure check,
and real CRT provider shutdown path passed. All 19 report call rows passed
live attribution checks; all eight seed ranges matched the installed PE.

The report retains the exact fixture, provider library, objects, generated
native bytes, commands, result logs and failures in a hashed local manifest.
One focused native fixture compares initialization and teardown for both
owners, successful/failed registration, and empty/three-node rings. It checks
free order, detached state before each free, preserved payload/owner preimages,
other-owner isolation, and no entity payload deletion. The instrumented source
uses the production owner/helper files with symbol rebinding only; allocation
and free still delegate to the actual canonical providers. A source-only
allocation failure verifies no publication or registration. A second path in
the same executable links the exact `bsp_core.lib` providers and runs their
actual wrappers through real `std::atexit` in reverse registration order.

This is a new C++ ABI, not a binary replacement. Original exception dispatch,
concurrency, corrupted rings, whole producer/cancel/drain behavior and game
validation remain unproved. No permanent test suite is added.
