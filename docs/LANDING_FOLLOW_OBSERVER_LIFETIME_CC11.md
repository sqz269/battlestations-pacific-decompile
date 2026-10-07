# Follow observer member lifetime, CC11

`cc11_land_follow_observer_lifetime` reconstructs two complete integer-only
ordinary bodies as direct conditional Source services. No whole Follow
constructor, approach/task destructor, private EH, callable image profile,
native ABI, arena, observer lifetime or game binding is supplied.

| Native body | End exclusive | Bytes | Full disk instructions | Original ABI |
|---|---|---:|---:|---|
| `006CDD70` | `006CDDC8` | 88 | 24 | ECX actual component; RET |
| `006CDDF0` | `006CDE0E` | 30 | 11 | ECX component; stack DWORD flags; EAX captured identity; RET4 |

All **118 bytes / 35 instructions** match the complete original PE and live
Ghidra bytes. The copied-body fixture preserves every instruction except four
natural CALL operands. Every `bsp.py ghidra bytes` request verifies configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The primary repaired
returning-free `ADD ESP,4` at `006CDE05..006CDE08` in
[the supported flow report](../reports/follow_callback_scalar_flow_recovery_cc11.json);
no worker Ghidra writes occurred. Bounds exclude alignment and neighbors.

The ordinary body stamps raw `CF8900` first at `6CDD8D`, then captures actual
component+14 once at `6CDD93`. A nonnull captured value is already the actual
`006952A0` **FIRST-endpoint identity** passed in ECX; component identity is EDX
at `6CDDA4`. There is no whole-unit assumption, endpoint-offset translation or
identity reload. `6CDDB3` then invokes complete `00695870` on the component,
including its real locking, detach, deletion and array-free effects. The facade
preserves all resulting profile/fields. It does not clear +14, reset headers,
restore `CF8900`, free the receiver or infer pointee lifetime.

The scalar body invokes the complete ordinary body at `6CDDF3`, then tests
flags' low byte bit0 at `6CDDF8`. If set, it real-frees the captured component
at `6CDE00`. It returns the original identity at `6CDE08`, even after free.
Source volatile flag capture retains post-destruction evaluation order. A
flag1 receiver must be a separate actual allocation in the existing Source
CRT domain; an interior Follow callback+18 must never be self-freed. No access
after receiver free or repeated destruction of freed backing storage is admitted.

`NativeLandFollowObserverCleanupView` borrows the same callback prefix at +0
and first-endpoint cell at +14. Its pure projection from the already completed
`NativeLandFollowEntryView` borrows callback+18/watched+2C references directly;
it performs no represented field read, call, allocation or translated cache.
Require coherent live >=18h component storage, same-address actual first-prefix
alias for nonnull +14, immutable view bindings and mandatory existing
`NativeObserverLifetime&` with its real manager/publications/locks/dispatch/
allocator/deletion-provider context. No new observer world or default provider
is constructed by the production API. Structural reentry, concurrency, faults,
invalid pointers and private-EH behavior remain outside the admitted domain.

Native constructor `009C2980` initializes the member prefix at state+18, zeros
data/count/capacity and watched+2C, sets byte+28=1, and stamps **derived
`CF89B4`** at `9C29CF`. Its full arithmetic/parameter construction remains
unbound here. Completed Follow entry `009BED80` publishes the first identity
at `9BEE0C` and registers at `9BEE15`; existing Source invokes genuine
`register_observer_pair_00694a60` with the actual first-prefix alias and same
callback+18. Actual ordinary approach destructor `9B2C80` passes
approach+108+18 at `9B2D15`; outer land destructor calls it at `9B3FBF`.
This proves native reachability, not whole caller Source/lifetime admission.
Task+404 remains a distinct retained-squadron identity and is not used here.

Raw `CF8900` has six original words (`6CDDF0`, `42B110`, `42B120`, `42B130`,
`42B140`, `6935C0`); derived `CF89B4` begins `6CEEB0`, `6CEED0`. Neither table
is made callable in Source. Unregister/detach do not dispatch owner +04/+08:
native `695399` / `695654` dispatch the **edge's slot0(flags1)** after removing
both endpoint entries. Genuine registration creates `CF7E64`; existing complete
`693CA0` only restamps that edge and optionally real-frees it. There is no
missing `CF8900` OnDetach callback in this canonical-pair path. Other edge
profiles still require the existing lifetime's real `delete_edge_virtual_00`
provider. The facade supplies no fake dispatcher or no-op callback.

One ignored fixture, `local/cc11_land_follow_observer_lifetime_probe.cpp`, uses
a contiguous actual-shaped Follow component and projects the existing entry
view's member references. It then runs **real registration twice on the same
canonical pair**. Unregister decrements references2 to1 and suppresses its
pending dispatch slot; complete callback-owner destruction must force detach
and delete that remaining edge. This catches an omitted cleanup tail. Flags0
retain the embedded receiver; flags1 use a separately CRT-allocated component
with null first identity. The same flow runs both full original bodies.
Constructor/entry arithmetic is not repeated or claimed executed by this probe.

Original observer CALLs use argument-only Source ABI bridges to the complete
existing lifetime operations; scalar's ordinary CALL targets the full copied
ordinary body, and free CALL targets genuine `singleton_lifetime_free`.
Private SEH instructions remain in the original copies, but only successful
normal flow is admitted; unmapped original handler/exception behavior is not
validated. Existing registration, locks, edge arrays, deletion and dispatch
suppression are real Source bodies. Unsupported profile/invalid-input fixture
providers reject those excluded domains rather than perform no-ops.

The fixture borrows explicit Source publication cells and an already-published
real OS recursive section. Its manager lookup branch is unexercised; these
instrumented cells are not proof of an actual runtime manager or owner lifetime.
Only inside the ignored probe, its CRT free import is traced before self free,
always forwarding the captured real CRT function and restoring the import.
Only captured observations/numeric identity are read afterward. Final
`CE3CD4` is observed from the real canonical-path provider, not assumed to be
retained `CF8900` or forced by the facade. No behavioral reentry is injected.

Two fresh strict MSVC Win32 TUs and the embedded `asInvoker` manifested probe
passed **43 checks in the explicitly owned ordinary Source observer domain**:
pair/refcount cleanup, pending suppression and published
size, final provider profile/fields, untouched first cell/padding, recursive
section restoration, flags0/1 and original-copy bridges. Source COFF shows
stamp before the one first-cell load, optional unregister before complete base
destruction, and scalar low-byte capture after it before optional real free.
Thirty-six TU/header/provider-source/pinned-library inputs are unchanged
pre/post. Support libraries were frozen from root's current beff-build before
its next build. Exact receipts are in
[the report](../reports/landing_follow_observer_lifetime_cc11.json).

Whole `9B2C80` still requires genuine shared-state `7B45F0` and MoveTo
`7B65E0` cleanup contracts. `7B4500` operates on 24-byte `CF5C94` elements;
existing `64B5F0` Source admits only 90h node storage and is not substituted.
Full outer task/base cleanup, raw profile/callback semantics, cached identities,
arena/CRT historical equivalence, observer/death lifetimes, private EH, original
ABI and gameplay remain unbound. Root owns CMake registration, full build,
independent review, annotations and integration; no tracked tests were added.

## Primary integration

Main `8ea38c617da5a2a4aa7b4811ffcac4defde5589d` passed the full Win32 build and all three existing CTests. Root independently reviewed all new code, both native bodies and the whole fixture, compiled three fresh actual TUs including the existing complete observer lifetime provider, verified39current Source/provider/header/fixture inputs (33actual includes), three current libraries and the original PE before/after linking, and checked the PE32 asInvoker manifest. All43ordinary Source/original-copy checks passed; all118native bytes also matched the fixture literals. Six native direct rows passed. Current emitted scalar code inlines the ordinary body; the post-base-destruction flag test remains verified, without native call-instruction or ABI equivalence. All runtime/manager/constructor/entry/private-EH/game qualifications above remain; no tracked tests were added.

## Active scalar provider reuse

Packet `cc11_land_follow_active_scalar_reuse` closes the normal provider mapping
for active profile `CF89B4` slot0, `CG_scalar_deleting_dtor_006ceeb0`.
The complete native body is `006CEEB0..006CEECE` exclusive: **30 bytes,
11 listed instructions, zero call gaps**. Disk and current live bytes match.
Root repaired only the returning-free `ADD ESP,4` continuation at
`006CEEC5..006CEEC8`; see
[the flow repair report](../reports/cc11_follow_active_scalar_flow_repair.json).
The original library name is retained.

Its bytes equal the previously completed `006CDDF0..006CDE0E` after normalizing
only the two four-byte rel32 CALL operands at body offsets `[4,8)` and
`[17,21)`. All other bytes remain identical. Actual targets also agree:
`006CEEB3` and `006CDDF3` call `006CDD70`; `006CEEC0` and `006CDE00` call
canonical CRT free `BF65AC`. Both execute component capture, complete ordinary
cleanup, the post-helper low-byte flags bit0 test, optional same-component free,
captured component return and `RET4`. The complete existing Source provider
`scalar_delete_native_land_follow_observer_006cddf0` therefore supplies this
normal contract unchanged. Header/source edits are comments only; no wrapper,
interface, code or ABI change was made.

The actual `CF89B4+0` word points to `006CEEB0`, whereas destructor profile
`CF8900+0` points to `006CDDF0`. The actual Follow constructor publishes
`CF89B4` at root+18h at `009C29CF`; this proves the profile connection, not
constructor execution, a complete callable class table, or receiver lifetime.
These numeric image profiles remain uncallable in the Source interface.

The incoming identity is the **component**, including callback18 when borrowed
inside a Follow root. This scalar has no root adjustment and does not route to
whole-root `009C2A60`. Flags0 cleans and returns that same component; flags1
requires a separately allocated actual CRT component and returns its now
dangling numeric identity. Flags1 never admits an interior callback18 free,
whole-root free, or root identity return. Embedded Follow cleanup still uses
the separate complete root/member paths. Mandatory actual observer lifetime,
manager/publications/locks, coherent storage, successful ordinary providers and
same-CRT allocation qualifications remain unchanged; private EH, image class
ABI, arena, full constructor/runtime lifetime and gameplay remain unbound.

Current evidence is read-only native equivalence plus static verification that
the header/source token streams and all historical report fields are unchanged.
The ignored receipt is
`local/cc11_land_follow_active_scalar_reuse_receipt.json`. The earlier 43-check
fixture and its support hashes above are historical results; they were not
rerun, relinked or expanded for this packet. The completed MoveTo/Follow callback
artifacts were left unchanged. No new Source execution or build is claimed.
