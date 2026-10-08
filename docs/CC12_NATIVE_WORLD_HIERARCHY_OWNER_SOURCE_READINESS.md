# Native World hierarchy owner Source readiness (CC12)

The actual World/entity attachment and placement path remains **unready**.
This packet adds **Source 0, production-ready owners 0, registered ready packets
0**. It recommends one bounded Source packet containing the two complete,
call-free intrusive unlink leaves, without claiming a World/entity producer or
integration. All five Native bodies and the placement guard unwind dependency
are now physically qualified. No Source, CMake, tests, build, fixture execution,
game execution, ledger or saved Ghidra program was changed.

The worker baseline is `8c5e4353c1f8dd9366b9d307fb6c382b3a35b6ea`. Main was
`86da7c2800ad6bd959031c09ae86a5db7f81a387` at capture. The prior registry Source candidate
`cf1970bb9` remains distinct from accepted main admission. The worker core was
built before the latest main merge and is explicitly historical. The registry
Source was subsequently accepted into main at `a4ba85f1f`; its primary receipt
is `907f9662c05bfd543877f523134522b8c2bada25714f80b4f59527acd69b8d2a`.
This closes that separate registry Source admission, while the World owner
frontier described here remains unready.

## Whole physical evidence

Fresh exports from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, match the complete installed PE bytes and live
Ghidra memory. Every exported instruction start matches independent x86
decoding: **703 bytes / 241 instructions** across five functions. There are no
missing continuations in these five extents.

| Native entry | Bytes | Instructions | SHA256 |
| --- | ---: | ---: | --- |
| `009037F0` | 89 | 34 | `a78634c872f6a342bbab2641f25a04679590a448431ae363b6a45fd3cd480689` |
| `009258F0` | 267 | 88 | `887cc8a74fa4d6673d8b7019b7295d31e51728b5fb54b3188b7d3538c2b6e10a` |
| `00928860` | 181 | 61 | `60de4981acea7f7b8fd69b6d555b7ef1d4c845741009f473de569e960505164a` |
| `00903F30` | 83 | 29 | `1eb60de632316963b95f9385620d97fe53bb5188cd20ac3cc593f988489e4dc4` |
| `00924710` | 83 | 29 | `5662da33dd4d146824cdadff4440b6b95f0c20d14d1293674b9bff17bfbe6d31` |

The approved EH expansion adds handler `00CA6FE8` (10B/2), cleanup
`00CA6FE0` (8B/2), FuncInfo `00DDA418` (36B) and unwind map `00DDA410` (8B).
The retained report contains the full extent/transfer records; the sealed
family contains every instruction and the whole original PE.

## Physical owners and distinct list layouts

`World+4` and `World+8` point to two separately allocated **12-byte** intrusive
headers, laid out as `first+0, last+4, count+8`. Their nodes are the actual
entities: `+34/+38` form the all-entities chain; `+40/+44` form the root/sibling
chain. A non-null hierarchy parent owns the corresponding child header
directly at `parent+48/+4C/+50`.

These are distinct from the **0x61 (97)** embedded category roots beginning at
World `+18`, each with `count+0, head+4, tail+8` and separate allocated nodes.
Category 1 is World `+24`. Publishing World `+4/+8` does not construct or admit
that category root, the World `+0C` triple, or their callers.

Current `GameWorldHost::build_entity_chains_009037f0` clears and fills C++
vectors with indices. `construct_world_object_004cb030` returns a
`WorldObjectLayout` projection with an opaque sentinel token; `run_world_construct`
delegates creation and postconstruction to abstract host methods. Current
Source searches find no concrete `WorldConstructHost` allocation implementation.
Those paths do not allocate the actual 0x4BC World or its raw intrusive headers.

## Complete Native schedules

**009037F0.** ECX is the real World. The first stacked argument contributes its
low byte; the second word is unused but both words are consumed by RET8. It
zeros World `+4A8`, writes byte `+4A4`, allocates 0x0C through `00BF681B`, writes
header `+4,+0,+8` in that order, and publishes the result at World `+4` before
the second allocation. The second header is initialized in the same order and
published at `+8`; null-return arms remain explicit. EAX is the second allocation
result on ordinary return. There is no local EH cleanup. A first allocation
throw preserves the earlier two World writes; a second allocation throw also
preserves the first allocation/publication and leaves World `+8` at its preimage.
Existing malloc/new-handler Source services do not manufacture the World owner.

**009258F0.** ECX is the actual entity; stack arguments are hierarchy parent,
World, matrix, and RET0C consumes all three. A nonzero byte `+BC` returns before
publication. Otherwise it writes entity `+30` and immediately dereferences
World `+4`, appending through `+34/+38` and updating the raw header. Only then
does it write `+3C` and append through `+40/+44` to the actual parent's
`+48/+4C/+50` header, or to the reloaded World `+8` when parent is null.

If `+B4` or `+B8` is nonzero, it captures both, clears `+B4` then `+B8`, and calls
whole `009245A0(entity, oldB8, oldB4)`. It calls existing raw `004134F0` with ECX
`entity+74`, stacked matrix pointer, RET4. It captures the child head at `+48`,
clears bytes `+C8/+10C`, calls `0042ED50` on every actual child, reloading each
child's `+44` after that callback, and writes byte `+BC=1` **last**. There is no
local rollback. The latch cannot justify ignoring the nonzero pair/child arms.
Current live callers are `00928860`, `004B2030`, and `00929C80`; older sole-caller
and two-caller comments are stale and were not mutated in this read-only packet.

**00928860.** ECX is the entity with the same three stack arguments and RET0C.
It calls `00928240`, captures the returned registry's raw section at `+4`, builds
the local guard with profile `00CE37FC`, enters the non-null section, and then
increments section depth `+18`. It captures old entity `+30`, compares it with
the incoming World once, and preserves that decision across callbacks. State 0
is armed at `009288AF`, after successful lock entry/increment and the comparison.
If changed and old World is nonzero, it reloads the actual entity vptr and calls
slot `+134` before attachment. After attachment it uses the original changed
decision and current entity `+30`; if nonzero it reloads the actual vptr and calls
slot `+130`. Thus a latched attachment does not make the outer placement call a
no-op. A non-null captured section's current depth is decremented before
`LeaveCriticalSection`; the original FS chain is restored on ordinary return.

FuncInfo `00DDA418` has magic `19930522`, one state, no try/catch map, flags 1.
State 0 maps to -1 through `00CA6FE0`, which computes ECX=`EBP-14h` and tails to
existing `00411EE0`. That provider captures guard `+4` before writing its profile,
preserves the stored section pointer, decrements its current `+18` counter, then
leaves it; a null section skips decrement/Leave. This closes the Native guard
dependency graph and supports an ordinary C++ cleanup contract. It does not
establish raw Native FH3 frame, SEH, hardware-fault, or invalid-pointer parity.

**00903F30 / 00924710.** Each is a complete 83B/29 call-free body. ECX is the
actual 12-byte intrusive header; one stacked actual entity pointer is consumed
by RET4 and remains in EAX. They differ only in entity member displacements
`+34/+38` versus `+40/+44`. With both links zero, the sole skip condition is
**signed count > 1**. Counts 1, 0 and negative values take the mutation path,
whose DWORD decrement wraps. The body updates the previous next link or header
first, reloads the actual next link, updates its previous link or header last,
clears entity next **before** previous, then decrements count. ECX and all
nonvolatile registers survive; EDX is volatile. Exact reload order, overlap,
arithmetic flags and no added null/bounds guard belong to this raw contract.

## Current Source and existing providers

`chain_unlink_00903f30` now has the accepted signed `count > 1` fix, but remains
a `std::vector`/handle projection with bounds checks. It is not either raw leaf.
The retained historical core object still emits **JLE** after `CMP count,1`
(function offset 0x32), proving it predates the fix. That old object cannot be
used as current build evidence; no build was run by this audit.

Current `attach_unit_instance` writes a detached state projection and delegates
the deferred pair/matrix to abstract host calls. It neither performs the two
raw list splices nor invokes the declared child-notification method. Current
`place_unit_instance` similarly uses host callbacks and is `noexcept`, without
the original ordinary exception guard. Neither supplies the actual receiver,
current vptr providers, lifetime or full Native attachment behavior.

Existing `copy_native_camera_matrix_004134f0` is a complete raw ECX destination,
stack-source RET4 implementation. Its historical physical core member is
103B/35 with no relocations and the accepted Native hash. Reuse it; no new x87
packet is needed. The existing ordinary `00411EE0` guard provider is physically
present in core (31B/12 with the real Leave import), distinct from Native's
25-byte original and from the owning-section release `0041CC80`.

Existing `004B7EC0` Source is 13B/6 and matches its historical physical member;
it clears three words of fresh storage. Its ledger currently says standalone
fixture admission reopened. Existing `004C2D30` Source tails to raw clear70;
its final admission is pending that clear's regate. Neither is missing Source,
and neither independently supplies the complete World owner. Do not substitute
the header13 initializer for 009037F0's distinct whole allocation/write schedule.

Seven historical object modules were inspected; six complete core archive
members match those exact physical objects. The seventh is the game-target
World host object and is absent from core as expected. Whole archive bytes,
member offsets, relocations, compiler/read records and current Source are
retained. These establish physical provenance and staleness boundaries, not
fresh compilation or current consumed-input identity.

## Concrete next Source proposal and owner frontier

Propose **`cc12_native_entity_intrusive_unlink_source`**, containing only whole
`00903F30` and `00924710` in new `native_entity_intrusive_unlink.hpp/.cpp`, with
a dedicated doc/report and Root-owned CMake integration. The raw MSVC Win32
facade uses ECX header, an unused EDX formal and one stacked entity pointer;
it returns that pointer in EAX and RET4. These leaves have no allocator,
callbacks, external data or unresolved callees. Preserve all 83 bytes' behavior,
including reloads, signed guard, wrap, flags and clear order. Validate with the
normal Win32 build and fresh whole COFF/member evidence. Do not fabricate a
World/entity fixture or claim a production caller. Root must register and
review the proposal; this report awards no ready/source count itself.

The **first unclosed production owner is still 004CB030 and its real allocation,
construction, publication and lifetime**, before 009037F0 can serve an actual
World. Existing prior evidence qualifies its 126B/39 body and genuine 80B/23
caller witness; it does not admit its complete dependencies. Close the reopened
header callback gates, actual `004C3080` sentinel allocation/teardown,
`00BF7CD1` iterator and partial-array cleanup, constructor EH, actual profile
lifetime, and game `+19CC` publication. Then establish actual entity/parent
phases and independently close whole `009245A0`, recursive `0042ED50`, and the
current class's `+134/+130` callbacks. Empty trees, a no-op deferred callback,
projection casts and manual `entity+30` writes cannot replace these contracts.

## Retained family and limits

Evidence directory: `local/cc12_native_world_hierarchy_owner_source_readiness_evidence/`.
ZIP: `J:\PROG\battlestations-pacific-decompile-cc12_wake_append_source_ast\local\cc12_native_world_hierarchy_owner_source_readiness_evidence.zip` (24752539 bytes), SHA256
`a99c88fb20788477adde9c0cecd9f3e4742eda561f96cbc4d6782ce49c85a5cf`. Manifest: 150 files; ZIP:
151 entries. Every payload hash and ZIP CRC passed. The 89
physical input pins, current Source snapshots, complete PE, fresh exports,
historical archive/objects and verification scripts are retained together.
The tracked doc/report are outside the archive to avoid a checksum cycle.

This is static Native recovery and Source-readiness review. It adds no build,
fixture, ABI execution, Native entry execution, game startup or gameplay proof.

Primary review accepted the readiness findings after independently rehashing
all 150 retained artifacts, 151 ZIP entries and 89 physical input pins, checking
the seven complete Native spans against the installed PE, and reviewing the
34 selected current Source inputs and seven historical object modules. The
primary receipt is `local/cc12_world_hierarchy_owner_primary_review/receipt.json`,
SHA256 `9672e0b1f3f80c8f6ac2321e7b6c8b2861c1c4b01e901138997ad8f607283cbd`.
The two unlink leaves are admitted for a bounded Source implementation next;
this review itself adds no reconstructed-function or ready-owner credit.
