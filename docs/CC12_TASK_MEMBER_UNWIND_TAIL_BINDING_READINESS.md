# CC12 task member unwind tail and Source binding readiness

`006569A0..006569A4` is exactly one five-byte relative jump to `00653390`.
Fresh Ghidra body, bytes and listing agree with Root's complete-body gate and
the Original PE. This closes the direct tail relationship between the task's
member unwind target and the accepted normal member cleanup entry. It adds
no Source provider, Native frame-handler policy or ABI equivalence.

Baseline: `572b6612cf6c2c8092922f75a8b104d408a8e374`. Only this document and
its [report](../reports/cc12_task_member_unwind_tail_binding_readiness.json)
are added. The lease covers only `006569A0`; no child, action, descriptor,
table, interpreter or neighboring Native body is newly opened.

## Complete five-byte evidence

| Property | Verified value |
| --- | --- |
| Bytes | `E9 EB C9 FF FF` |
| Body and instruction count | `006569A0..006569A4`, one five-byte instruction |
| Signed relative displacement | `FFFFC9EB`, or -13845 |
| Target calculation | `006569A5 - 3615h = 00653390` |
| SHA-256 | `2f04b813babd5ec30228de397ecf4627834322753638268e1a989dab00a41ad5` |
| Existing metadata | `FUN_006569a0`, displayed `undefined(void)` |

The jump redirects EIP. It adds no CALL, RET, pushed return word, stack
adjustment, data access, general-register write, arithmetic-flag write, FS
registration or local EH frame. The target receives the current ECX, EDX,
other general registers, ESP, EBP, flags and existing stack/frame backing.
The body establishes no independent return value or downstream preservation
guarantee. Its displayed prototype does not recover those contracts.

Each live command uses the repository client's project/program verification:
project `bsp`, program `/battlestationspacific.exe`, x86 little-endian 32-bit,
image base `00400000`. Live and snapshot function counts remain 64,729. The
whole Original image hash and selected five file-backed bytes were checked.
No Ghidra name, signature, comment, listing, flow flag or saved project changed.

## Relationship to the earlier EH audit

The [earlier EH audit](CC12_SQUADRON_LAUNCH_TASK_EH_HANDLER_READINESS.md),
published in `33d6755762a7a8833d426ebc0c0f0049660c1705`, correctly recorded
different physical targets: normal cleanup calls `00653390`; the member
unwind action tails to `006569A0`. Its report recorded no equivalence. Both
older files are unchanged, with their exact Git-version content rechecked.
The new evidence supplies the missing direct edge from the second address
to the first; it does not rewrite the earlier evidence domain.

The accepted action reloads current stored-this from EBP-10h and adds 20h.
The normal owner uses its current preserved ESI plus 20h. Those inputs need
not identify the same member when aliases or preceding children changed
saved frame backing. The five-byte jump forwards whichever current values
reach it. A common downstream address proves neither equal inputs nor equal
Native exception dispatch, state interpretation, frame validity, child
failure behavior or unwind completion. `C07991` and the member cleanup's
`C7AF48` handler remain unopened by this packet.

## Actual current Source interfaces and member backing

The accepted [88-byte cleanup contract](CC12_OBSERVER_ENDPOINT_CLEANUP_ABI_READINESS.md)
already establishes: stamp `DWORD[M+0]=00CF6494`; capture current endpoint E
from `M+14h`; optionally call `006952A0` with E and M; then always call
`00695870` on normal continuation. There is no endpoint reload after the
state-zero store, no test of M+10h and no clearing of M+14h. That contract is
reused without reopening its Native body.

Both concrete Source operations are methods of `NativeObserverLifetime`:

| Method | Explicit Source receiver storage |
| --- | --- |
| `unregister_pair_006952a0` | `NativeObserverOwnerStorage& first`, `NativeObserverOwnerStorage& callback_owner` |
| `destroy_callback_owner_00695870` | `NativeObserverOwnerStorage& owner` |

The implicit Source `this` is the retained lifetime service, separate from M.
These methods accept actual owner references. They do not accept a whole
18h-byte member as an untyped address or an invented member view, and they
are not direct implementations of the Native ECX/EDX entry interface.

`NativeObserverOwnerStorage` is exactly a 10h-byte prefix:

| Member offset | Actual Source field or required external backing |
| --- | --- |
| `M+0` | `volatile uint32_t native_vtable_00` |
| `M+4` | `NativeObserverEdgeStorage** edges_04.data_00` |
| `M+8` | `uint32_t edges_04.count_04` |
| `M+C` | `uint32_t edges_04.capacity_08` |
| `M+10` | Separate member byte; not read by accepted cleanup88 |
| `M+11..13` | Outside the prefix; no field or initialization invented |
| `M+14..17` | Separate actual endpoint-pointer cell, outside the prefix |

A later provider still needs the actual M prefix, the actual external M+14h
cell and, on the nonzero arm, the actual already-adjusted first-endpoint
base E. It must preserve the accepted late capture after the profile write.
A copied endpoint object, private empty array or copied pointer cell does
not establish the same identities. Source lookup compares edge endpoint
pointers with the addresses of the supplied owner references, and removal
mutates their actual arrays. Under accepted task placement, M is Task+20h
and M+14h is Task+34h; that arithmetic creates neither storage nor ownership.
This packet declares no full member type, view, producer or binding API.

## Borrowed lifetime and Source exception boundary

`NativeObserverLifetime` retains a copied `SoundLifetimeAccess` borrower,
references to the actual E198E0 lock and E198E4 dispatch publication cells,
and an `ObserverLifetimeServices&`. The access can reference the existing
semantic fixture domain or the actual 01090AA0 manager publication cell.
Its identity comparison uses the cell/object address, not its current
pointer value. Equal values in distinct cells are not the same domain.

The existing application path supplies `GameObserverRuntime::lifetime()`.
Its constructor uses `GameSingletonHost::sound_lifetime()`, which borrows
the host's actual manager publication, and its own stable lock/dispatch
cells and service implementation. The host retains this context through
raw manager drain. Member and endpoint backing, edge arrays, selected
dispatch storage, services and publication cells must remain valid through
cleanup; registration and removal must use the same actual identities and
domain. Observer cleanup must finish before shutdown frees the dispatch
owner. E198E4 alias bits deliberately survive that drain and do not prove
a live owner. No selected task-to-runtime binding is supplied here.

There is no automatic owner destruction in `NativeObserverOwnerStorage`:
the prefix has no constructor, destructor or owned sidecar. Source
`unregister_pair_006952a0` releases its captured section guard when a C++
exception leaves it, but does not automatically invoke owner destruction.
The concrete `destroy_callback_owner_00695870` stamps `00CE3CD4`, takes
outer and nested count-query guards, optionally detaches edges, then frees
the current array pointer. Its catch path also frees the current array and
rethrows after guard unwinding. It neither frees the owner nor clears the
retained pointer/count/capacity bytes.

Any later automatic Source cleanup must therefore distinguish a failure
before owner destruction begins from a failure inside `00695870`, whose
Source catch already performs array cleanup. Replaying it after that failure
can act on retained freed-pointer bytes. Adding a guard that calls it on an
unregister failure would be an explicit new Source exception policy until
the relevant Native handler contract is established. Neither state-store
labels nor this tail jump prove that policy. No guard, retry, Native FS
adapter, raw frame thunk or provider is added by this audit.

## Current evidence and validation boundary

The current scalar-deletion primary receipt covers 80 Source inputs and four
build artifacts, all independently hash-matched here. It records the normal
Win32 build ending `2026-10-09T16:28:29Z`, three existing checks, sixteen
whole objects and eighteen positive public Core roots. The prior scalar
candidate is now admitted as 39 Source bytes / 13 instructions with its
cleanup REL32 and current CRT-free import relocation. These are existing
build results, not execution or Source credit for the five-byte tail.

The older 67-input receipt stays historical: 66 current inputs still match;
`CMakeLists.txt` changed during the later admission. Its old artifact pins
are not presented as current. Separately pinned observer Source excerpts
support this interface audit without claiming a composed member provider.
Fresh selected Native verification, current pins, unchanged historical EH
files and `git diff --check` are the validation performed here. No new build,
test, probe, Source implementation, CMake, ledger or Ghidra mutation occurs.
