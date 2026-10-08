# Native entity registry owner Source readiness

The registry lifetime is now bounded enough for a **concrete ordinary Source owner
design**. That owner is not implemented, and real entity/factory wiring remains
unready. This packet adds two evidence documents; Source changes and reconstruction
credit are **0**. No C++/CMake changes, build, test, probe, entry execution or Ghidra
mutation occurred.

## Scope and whole Native evidence

Started from accepted main `9235c33285b99ed5568438761e8ff8345f98b73a` in the isolated Astra worktree.
Read-only batches verified project `bsp`, `/battlestationspacific.exe` at
`C:/Users/sqz269/bsp.gpr`. Address/range leases were expanded before investigating
supporting lifetimes, CRT code/table and exception data. The complete installed PE
SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Nominated whole body | Bytes | Instructions | Actual identity |
|---|---:|---:|---|
| `00928080..009280C4` | 69 | 19 | SceneRegistry constructor, raw8 / profile D19284 |
| `00928240..009282FC` | 189 | 51 | SceneRegistry getter, publication F899FC |
| `00924100..00924144` | 69 | 19 | Pending-init lock constructor, raw8 / profile D190C0 |
| `00924810..009248CC` | 189 | 51 | Pending-init lock getter, publication F899E4 |
| `00926BE0..00926C7D` | 158 | 49 | Same-root-pointer producer into owner F899CC |

Those five are **674 bytes / 189 instructions**. Expanded evidence retains
21 complete owner/support/routing spans plus 16 separate EH handler/funclet spans:
**1,678 bytes / 467 instructions**.
Every whole span matches both the installed PE and freshly read Ghidra memory.
The complete 4,016-byte / 1,004-cell CRT pointer table also matches both.
The linked [report](../reports/cc12_native_entity_registry_owner_source_readiness.json) supplies all extents, calls, hashes and ABI details.

## Separate lock ownership and exact ordering

The two constructors receive ECX=actual eight-byte owner and return that same
pointer in EAX, with no stack arguments. Each enters EH state0, stamps its own
profile, creates a real `BD1860` tracked critical section, then publishes section
`+4`. Constructor failure unconditionally clears its own FC/E4 publication and
stamps base `CE3818` through `927A60` / `923640`; it does not release the untouched
section preimage, free the owner or unregister it.

Each getter returns its first nonzero publication read on the fast path. On the
slow path it captures the **first** canonical manager's `+10` section, enters it
and increments depth, double-checks its own cell, allocates raw8 and constructs.
After leaving allocation EH state1 it publishes the owner, calls the manager
getter **again**, then reloads the publication as the registration argument.
It decrements the captured section depth before Leave and reloads publication
after unlocking for the slow-path result.

On constructor failure, the getter frees the captured allocation and then releases
the manager guard. On registration failure, the owner/publication remain and only
the guard unwinds. These are separate actual `F899FC` and `F899E4` owners; existing
destroy/kill lock `F899E8` / `9248D0` is not a substitute.

Whole scalar destructors `9285F0` and `925660` are each 55 bytes. They stamp the
correct profile, release actual owner `+4` through `41CC80`, test flag bit0, clear
publication unconditionally, stamp `CE3818`, optionally free raw8, then return
the captured owner address with RET4. The three-byte post-free continuations
missing from Ghidra at `92861E` / `92568E` are present in the PE and live bytes.
There is no publication-identity guard or unregister call. Literal Native profiles
are identity data, not callable Source vtables.

## Pending-init producer, ownership and teardown

`926BE0` takes the actual entity root in ECX and gets the **E4** section. It captures
the current sentinel, previous link and sentinel `+4` address. A local cell at
entry ESP-24 holds the same incoming root pointer for `924B10`'s third argument.
After node allocation, `9267F0` increments the current `F899CC+8` count **before**
the two link stores. It writes captured sentinel.previous, then rereads node.previous
and writes that node's next. The captured section is decremented before Leave.

There is no entity dereference, flag gate or deduplication in this body. Its only
EH action is guard release: if node creation succeeds and count growth throws,
the allocated unlinked node is **not freed** by this producer. A Source port must
preserve that behavior. Scene construction's outer FC lock and this inner E4 lock
must retain their distinct identities and order.

The actual static owner is **F899CC**, sentinel pointer **F899D0**, count
**F899D4**. Its initializer is verified `CD39A0..CD39C4` (37 bytes), selected by
CRT cell `CE30CC`. It creates/self-links a 12-byte sentinel, publishes the pointer,
zeros count, preserves owner `+0` and sentinel payload `+8`, then registers fixed
`CDF4C0` with `_atexit`. Allocation failure does not publish/register; registration
failure retains the initialized list and its returned status.

`CDF4C0` tail-jumps to whole `924A70..924AB7` (72 bytes) on the same CC owner.
Teardown detaches the ring and zeros count, frees payload nodes in next-link order,
then frees the sentinel and clears head. It never deletes the borrowed entities.
The saved Ghidra body omits the final twelve bytes; the retained complete PE/live
span includes them. An intact finite ring and quiescent process shutdown are required.

## Canonical raw ID pair and actual CRT registration

The verified ID initializer is **CD3A70..CD3ACA** (91 bytes), selected by table
cell **CE3120**. `CD3A8C` and `CD3AA0` are interior immediate-load sites, not entries.
It calls actual `951660` on raw54 primary `F89A08` with `(first=0,count=1000h)`,
then adjacent raw54 alternate `F89A5C` with `(first=1000h,count=1000h)`.
It registers **CDF500**, not CDF4D0.

`CDF500..CDF546` (71 bytes) destroys alternate first, then primary. Whole plain
destructor `9516B0..9516C0` (17 bytes) captures slots `+4C`, stamps D19B88 and
frees slots through BF6989 → BF65AC. It neither frees the static header nor clears
dangling slot fields. The existing Source scalar deleter with flags=0 offers the
same admitted field/free effect through its typed Source profile and new C++ ABI.

Both ID wrappers have a primary-only state0 cleanup: initializer failure after
primary construction destroys primary; normal teardown marks state0 while destroying
alternate, then marks state-1 before the explicit primary call. The initializer's
state0 remains active through registration. No alternate rollback is invented.

The full physical `__cinit` body `BFBC47..BFBCD8` walks `[CE2734,CE36E4)` upward
in DWORD steps, skipping zeros and calling each nonzero initializer after successful
C initialization. Individual initializer return values are ignored. Consequently
pending-init registration precedes ID-pair registration, and successful exit
callbacks run ID teardown before pending-list teardown. Existing `CD3940` remains
its separate accepted 37-byte kill-owner initializer.

**Routing correction:** whole `CDF4D0..CDF4F2` addresses owner `F899C0`, calls
`925BD0`, frees/clears `F899C4`; it is not ID-pair teardown. The later physical
continuation also corrects its truncated saved function membership. No Ghidra edit
or new function-count credit was taken for any recovered CRT entry/continuation.

CC, E4, FC and the raw A8-byte ID pair lie in the PE `.data` virtual tail beyond
its raw file bytes. Their initial zero state is loader zero-fill, corroborated by
Ghidra memory, not a fabricated disk-byte preimage.

## Exception evidence and Source boundary

Seven exact FuncInfo headers (`19930522`, flags=1), nine unwind-map entries and
sixteen complete handler/funclet spans are retained. All seven have no catch map.
They establish the local constructor/getter/producer and CRT cleanup actions above.
Static stack graphs visit every instruction in all 21 owner/support/routing spans,
with balanced ordinary returns and explicit callee-cleanup assumptions. This does
not execute FH3, reconstruct hardware-fault transport, or close earlier entity EH.
The deeper count/length-error graph remains explicitly qualified accepted library
reuse, with its earlier proof retained alongside the new complete count body.

Existing actual providers cover raw allocation/free, critical sections, canonical
manager publication/registration, sentinel/list teardown, payload nodes/count growth,
and raw ID table leaves. `GameNativeStringProcess` owns the permanent actual
`01090AA0` manager cell. `NativePendingEntityLock` supplies the concrete **E8** pattern.
The current singleton deletion switch lacks **D190C0** and **D19284** cases.
Current process pending owners cover A8/B4 only; no CC owner or canonical ID pair
producer exists. `ObjectHandleTables` is only a borrowed current-field view.

Current app `GameMissionLuaHost` deduplicates IDs in a semantic pending vector;
its host-mediated `925F20` InitAll binding is not the raw CC ring consumer.
Current entity construction still returns metadata and semantic units. Those cannot
be cast into real roots or mirrored into a second registry and called authoritative.

## Cohesive ordinary Source design and remaining frontier

A future bounded implementation should own, together:

1. One permanent process storage with CC raw0C list, adjacent primary/alternate
   raw54 headers, and distinct E4/FC publication cells; private construction,
   stable addresses and no duplicate implicit teardown.
2. Explicit once-only CD39A0/CD3A70 startup in the proven order, real fixed
   noncapturing `std::atexit` callbacks on those same bytes, exact failure retention
   and reverse teardown. No external opaque lifecycle callback or second manager.
3. Both complete lock lifetimes and direct Source deletion-switch cases bound to
   their actual process cells; use existing real allocation/section/manager providers.
4. Complete raw926BE0 and actual current-field ID aliases. Borrow a genuine entity
   root from its future producer, preserve count-before-link and exception behavior,
   and keep all consumers on one authority.

That is a reviewable design, **not an implemented owner or a ledger status change**.
Application migration remains gated by real root/descriptor allocation, world and
hierarchy publication/withdrawal, raw InitAll, functional entity profiles and the
unresolved SceneNode/GameEntity/full-root exception paths. No tiny getter, header
clear, fake profile, scratch root or mirrored semantic registry closes those gates.

## Artifact verification and limits

Nineteen complete historical worker objects are retained: seventeen match unique
whole members of the 1,905-member core archive; two application objects are correctly
absent. The evidence includes 276 selected complete function extents and 27 API
groups, full sections/symbols/ordered relocations and actual tool files. The current
Source/dependency copies match 3,251 rehashed physical pin records and
4,029 Source inventory records. These are not fresh consumed-input preimages.

The private singleton dispatcher has a complete 3,109-byte extent: 2,972 code bytes
plus 137 bytes of compiler switch tables/alignment. The local reader now keeps that
data and all relocations separately; four data labels are referenced by MOVZX/JMP
operands and all seven pointer-table entries target decoded code starts. No trailing
table bytes are claimed to be instructions. The original reader is also retained.

Evidence: `local/cc12_native_entity_registry_owner_source_readiness_evidence.zip`
(57,828,923 bytes / 3,410 entries), all CRCs and member hashes verified.
ZIP SHA-256: `e9060a02e1327840cfb69a180bc75b71bcefd77c2de251a55d16577cb75ab30e`.
Manifest SHA-256: `f253c89a1e47c42e25e50f439023354f877e2652eebaecd61c6624d4b79209ff`.
The final document/report are produced after sealing and intentionally outside the ZIP.
No final-link/COMDAT, loaded-target, native ABI compatibility or gameplay proof is claimed.

## Primary review

The integrator rehashed 3,409 artifacts and all 3,410 ZIP entries, checked
3,251 physical pins and 4,029 Source inventory records, and independently
decoded all 37 Native spans against the installed PE. It replayed all 21
ordinary stack/state graphs, checked the complete 1,004-cell CRT table and
seven FuncInfo/nine unwind entries, and compared 276 historical whole COFF
extents and their ordered relocations from 19 objects/17 unique Core members.
Receipt `local/cc12_entity_registry_readiness_primary_review/receipt.json`:
SHA-256 `0172b98199c02111e4c089017f8bb78d07bc9aad9f6a5c9cdbf106aa79ae4617`.
The cohesive ordinary owner design is accepted for a later implementation;
no Source, build, runtime, entity-parent or ledger credit is added here.
