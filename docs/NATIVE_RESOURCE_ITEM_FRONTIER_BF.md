# Native resource-item ownership frontier BF

The next ready packet is **B86930/B86990, 58 original bytes**, with a finite
`D631C0` deleting-profile provider. The existing `BD30E0` implementation already
captures the current profile and calls the required provider for slot04 with
flags1. Its concrete source does not supply the missing fallback scalar body
or any of the thirteen other observed registered parser-result profiles.

The [report](../reports/native_resource_item_frontier_bf.json) preserves complete
inclusive spans, matching PE/live hashes, original ABI, producer and register
provenance, direct/indirect calls, unwind maps and explicit partial boundaries.
`verify_report_calls.py` passed **100 direct rows, zero failures**. Twenty
indirect rows retain their actual operands and are not mechanically resolved
by that check. Every live batch used the target-verifying `bsp.py ghidra`
route: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, bridge8089.
The installed PE SHA256 remained
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

This is read-only discovery. There are no source, build, fixture, native ABI,
Ghidra mutation, game or UI results. Address-backed source status was checked
in this worker checkout; it is distinct from previous typed payload success.

## Immediate fallback packet

| Entry | Complete inclusive span | Bytes | Coverage / current source |
| --- | --- | ---: | --- |
| Fallback construct | `B86930-B86945` | 22 | Complete; named, no implementation |
| Fallback scalar delete | `B86990-B869B3` | 36 | Complete; named, no implementation |
| Shared deleting thunk | `BD30E0-BD30ED` | 14 | Complete existing `native_ref_counted.cpp`; requires provider |
| Shared ref-base destroy | `BD30F0-BD30F6` | 7 | Complete existing naked Win32 body |
| General item-base construct | `B868B0-B868C5` | 22 | Complete read; named, no implementation; separate dependency |
| General item-base destroy | `B86890-B8689A` | 11 | Complete read; named, no implementation; separate dependency |

`B86930` receives ECX raw storage, captures it in EAX, writes `CEB130` at00,
writes count1 at04, then writes `D631C0` at00 and returns EAX with plainRET.
The observed allocation is **eight bytes**, with no hidden pool word or owned
payload. Neither this constructor nor its scalar calls the general item-base
helpers `B868B0/B86890`. Those are not prerequisites of this fallback packet.

`B86990` saves ESI and captures ECX in it. It stamps `D5C104`, calls the
existing `BD30F0` at `B86999`, then tests bit0 of the flags byte at `ESP+8`.
When set, it pushes that same ESI and calls `BF65AC` at `B869A6`, followed by
`ADD ESP,4`. It returns the original pointer in EAX, restores ESI, and uses
RET4 even if the pointer was freed. Count04 is untouched; there is no scalar
null test, decrement, payload cleanup, or local FH3 frame. The current live
listing includes the complete returning-free continuation and has zero gaps.

`BD30E0` is also complete: nullable ECX, no stack inputs, plainRET. For a
nonnull receiver it reads `[ECX]`, then that table's current04 entry, pushes1,
and calls it. The source `NativeRefCountedDeleteCalls` interface is explicit
dependency injection with a changed C++ ABI. Add a concrete provider resolving
the captured `D631C0` identity to `B86990`; do not turn this interface into a
claim that arbitrary resource profiles are implemented. There is no second
reference decrement in either the thunk or its provider.

The source allocation/free boundary already exists:
`singleton_lifetime_allocate` supplies the documented `BF681B` service, and
`singleton_lifetime_free` supplies `BF65AC`. For this raw prefix, native and
host allocation extents are both8. Retain CRT identity and failure contracts;
do not port allocator, new-handler or exception internals. The native producer
calls `BF681B` directly, not `BF55BE`. The latter is an already known thunk to
the same allocation entry, not a pool allocation.

Claim only `B86930/B86990` and new
`include/bsp/native_resource_fallback_item.hpp`,
`src/native_resource_fallback_item.cpp`, its doc and report. A deleting-only
provider for an already formed object needs the36-byte scalar; including the
22-byte constructor makes the58-byte formation/deletion packet. Use a raw
eight-byte prefix separate from host virtual interfaces, preserve current
profile reads, ordered stores, flag handling and the same-pointer result.
Recheck leases and use the strict Win32 build and relevant existing checks.
No general resource destructor, cache, array or hierarchy integration belongs
in that packet. Unknown profiles remain an explicit external contract.

## Producer, append and real release paths

There is exactly one live incoming call to `B86930`: `B7EA81` in `B7E970`.
The unknown-type path pushes8 at `B7EA68`, calls `BF681B` at `B7EA6A`, and
cleans that one argument with `ADD ESP,4` at `B7EA6F`. It moves EAX into ECX
before construction. The result is captured in ESI before child payload skip;
`B7EAA6/B7EAA7` pushes the same pointer into the current resource's slot0C.
Only child-handle cleanup follows. Allocation-null instead supplies a null
item to the same append route; no safe native null-item teardown follows.

Freshly matched `B87AA0` stores that raw pointer in primary data[count] and
increments count without AddRef. `71BB40` calls this base append before
classification and has no reference-count operation in its complete body.
Classification helper internals remain external. This retains one release
obligation per primary array entry, including each occurrence of a duplicate.

`B88430` reloads primary data at10 and count at14. EBP captures each item.
EBX is loaded from `CE2220`, identified in the installed PE imports as
`KERNEL32.dll!InterlockedDecrement`. `B8847A` calls it on EBP+4 with no null
check. Only a returned zero reaches `B88487`, with ECX=EBP and the item's
current slot0. `D631C0` makes this `BD30E0 -> current04(1) -> B86990`.
It is the primary item that carries a vtable and count. The separate hierarchy
record's first DWORD is a numeric parent, as established by the sibling
[hierarchy frontier](NATIVE_RESOURCE_HIERARCHY_FRONTIER_BE.md).

All three live incoming `B88430` routes are recorded and checked:
`B88763` calls it from the base scalar wrapper; `71886B` tail-jumps after
concrete classification-array cleanup; `C84F73` tail-jumps from construction
unwind. `B86990` itself has no live direct callers and one data reference,
`D631C4`. The report also retains every live incoming reference to `B86890`.
Direct/data xrefs cannot enumerate every possible indirect release caller
globally; this is the observed resource route, not an exhaustive virtual-call
analysis of the executable.

## Exceptional lifetime

`B7E970` handler `CC1FE3` loads info `DFB134`, whose two-state map is `DFB124`.
State1→0 invokes `CC1FD8`: free the raw allocation saved at `[EBP-2C]` through
`BF65AC`. Its native `POP ECX; RET` at `CC1FE1..CC1FE2` is missing from the
stored body. State0→-1 invokes `CC1FD0`, releasing the child handle atEBP-34
through `BE9ED0`. State1 covers fallback construction and is reset to0 before
skip and append. The map does not release the already constructed item when
skip or append throws. Adding transactional item rollback would add behavior.

`B88430` handler `CC25F9` uses info `DFBA64` and map `DFBA44`. Its four actions
are hierarchy backing `CC25EE -> B87B40(resource+1C)`, primary backing
`CC25E3 -> B87B20(resource+10)`, name `CC25D8 -> 41DD20(resource+8)`, and base
`CC25D0 -> BD30F0(resource)`. There is no per-item progress action or retry in
that map. The primary integrator owns the arrays; hierarchy fields/pool and
manager/cache remain separate contracts. This audit does not reconstruct
`B88430` or broaden the sibling packets.

## Other fallback virtual slots

The complete36-byte `D631C0..D631E3` table has nine observed entries. Only the
first two belong to the immediate deleting-profile packet.

| Offset / target | Complete inclusive body | Observed meaning and ABI |
| --- | --- | --- |
| 00 / `BD30E0` | `BD30E0-BD30ED` | Current04(1) deleting thunk |
| 04 / `B86990` | `B86990-B869B3` | Scalar described above |
| 08 / `B86860` | `B86860-B86865` | EAX=current DWORD109021C; RET |
| 0C / `B86950` | `B86950-B86977` | AL membership of stack token among109021C/220/224; RET4 |
| 10 / `6F9D20` | `6F9D20-6F9D24` | EAX0; ignore two stack words; RET8 |
| 14 / `B86870` | `B86870-B86875` | EAX=current DWORD1090228; RET |
| 18 / `B86880` | `B86880-B86882` | Empty RET10h; no result guarantee |
| 1C / `6F9D30` | `6F9D30-6F9D32` | EAX0; RET |
| 20 / `B868A0` | `B868A0-B868A2` | Empty RET4; no result guarantee |

These seven nondeleting bodies have no current Ghidra function definitions.
The report records exact raw spans with `no_ghidra_function`, not inferred
signatures or invented field semantics. `B86950` does not read its owner.
Raw startup `CD82C0..CD82C9` sets ECX=109021C and tail-jumps `B86A00`;
that initialization/type-token domain is external. Runtime token values were
not observed. A future full append/classification binding needs the actual
token domain in addition to the two-slot deleting profile.

## Thirteen separate registered result profiles

The known registration documents identify eight manager/application and five
game parser targets. Each producer and the table prefix below was freshly
byte-matched, as was each complete outer scalar. The allocation extent and
construction-installed profile are established independently of the parser
singleton's own table. The delegated payload helpers and their mutations,
nested releases, full exception handling and parser-result lifetime are not
collectively audited here. These13 observations are not proof that later or
indirect registration cannot add another parser.

| Type / parse entry | Allocated bytes | Installed result table | Slot04 scalar | Direct cleanup dependency |
| --- | ---: | --- | --- | --- |
| Mesh / `B947A0` | 10h | `D63738` | `B93B40` | `B93910` |
| SkinedMesh / `B94850` | 10h | `D6375C` | `B93B60` | `B93910` |
| SkinedMeshAnimation / `B92F20` | 18h | `D63700` | `B92FA0` | `B92EC0` |
| MatrixIndexedMesh / `B94900` | 10h | `D63780` | `B93B80` | `B93910` |
| Camera / `B8B240` | 18h | `D632DC` | `B8B2C0` | `B8AA60` |
| GroupParams / `B8EB50` | 0Ch | `D634B0` | `B8EBC0` | `B86890` |
| AnimationChannels / `B8A910` | 14h | `D6328C` | `B8A760` | `B8A680` |
| Bone / `B8A990` | 2Ch | `D632B8` | `B8AF10` | `B8A8A0` |
| Aux / `71B5C0` | 54h | `CFD8FC` | `71ACB0` | `71AC40` |
| ZoneDesc / `719240` | 40h | `CFD888` | `718790` | Inline SSO, then `B86890` |
| Note / `719000` | 28h | `CFD860` | `718710` | Inline SSO, then `B86890` |
| GeomMesh / `727A90` | 50h | `CFDBD8` | `7269E0` | `726960` |
| ConvexObject / `6FAF00` | 2Ch | `CFB6A4` | `6F9DD0` | `6F9D70` |

Every listed table has `BD30E0` at00; every listed scalar has its own native
identity. No address-backed native implementation or reconstruction entry
was found for these13 scalar bodies in this checkout. The cleanup dependencies
are consumed by address; their full bodies and nested ownership remain
external to this audit. Existing typed Mesh/Note/GroupParams results and the
input/shader/text deleting providers remain complete at their own interfaces.
They do not implement these raw parser-result providers.

The three Mesh wrappers put the local mesh at08 and the serialized prefix at0C,
retain mesh+4, then release the temporary mesh reference with zero dispatch.
SkinedMesh and MatrixIndexedMesh replace the initial `D63738` table with their
own final tables. Their three scalars call the same `B93910`; the concrete
nested mesh profile, pool and field destruction still need source bindings.

General base `B868B0` writes count1 and `D631A0`. Aux, GeomMesh and ConvexObject
reach it through `71ABB0`, `7268D0` and `6F9CD0` respectively; their other
constructor helpers remain external. Other result producers call it directly.
The GroupParams scalar performs only `B86890` and conditional outer free: no
cleanup of08 occurs in that scalar. This is useful for a later finite deletion
packet but does not close parse slot20 or general result ownership.

Note and ZoneDesc conditionally free native SSO storage, reset size0,
capacity15 and the first inline byte, call `B86890`, and conditionally free
the outer object. Their respective size/capacity/data offsets are
1C/20/0C and2C/30/1C. This does not authorize porting STL algorithms or using
an ordinary host `std::string` as native storage.

Twelve three-byte `ADD ESP,4` instructions after returning frees are absent
from eleven of these scalar listings. The report preserves each inclusive
tail range and matched bytes; the Note and plain Mesh scalars currently have
complete listings. Raw handler definitions and the allocation-cleanup tail
are also recorded. All repairs, definitions, comments, names, saved analysis
and any source implementation are left to the primary integrator.

The complete local delivery, including capture scripts, matched binaries,
decoded listings, current source lookups and report verification, is retained
under ignored `local/resource-item-frontier-bf/`. Its
`whole-local-manifest.json` enumerates and hashes every other file under this
worktree's entire `local/`, including the SQLite index, so discovery survives
the worker handoff.
