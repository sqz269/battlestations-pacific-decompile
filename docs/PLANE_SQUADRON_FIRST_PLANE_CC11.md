# Actual squadron first-plane pointer (cc11)

`native_plane_squadron_first_plane_007ed010` reconstructs the complete ordinary
getter through ONE genuine live pointer cell. It reads the SAME member slot
published by the existing exact sorted producer; the returned plane identity
can feed the existing Source leader provider. This closes a conditional Source
leaf, not native class dispatch, world ownership or a larger caller.

| Entry | End exclusive | Bytes / instructions | Coverage / original convention |
| --- | --- | --- | --- |
| `007ED010` | `007ED017` | 7 / 2 | Complete: ECX=squadron; EAX=[ECX+3D0h]; plain RET |

The whole body is `8B 81 D0 03 00 00 C3`, PE/live SHA256
`87d3c6e97fd5be315d08d84d87574053eb97cec9453657bc98938747fd16dd25`.
There are zero native CALLs, absolute globals or direct caller xrefs. Padding and
neighbor `007ED020` are excluded. Primary defined the exact body with supported
locked tools, saved and exported it; see
[the definition receipt](../reports/cc11_plane_squadron_first_plane_definition.json).
Live `FUN_007ed010` covers `007ED010..007ED016` inclusive with two instructions.
Primary refreshed the lookup index after defining and exporting this body.

The borrowed view stores the actual squadron identity and a REFERENCE to its
already-live nullable pointer cell at +3D0h. Its pure factory checks nonnull root,
4-byte alignment, backing >=3D4h, address wrap and exact cell address. It performs
no represented value read, allocation, callback, native call, profile lookup or
cached pointer translation. Caller-provided placement/liveness/coherence remains
a required contract; address checks cannot establish object lifetime. Invalid
Source placement throws logic_error, qualifying native fault paths separately.

The operation then performs ONE volatile pointer read. It does not read count,
check membership or manufacture a default. Null is returned unchanged. A retained
nonnull pointer is returned even when count is zero; this does not prove that the
returned object is alive. Its later use needs the consumer's own actual lifetime
and field domain. `plane_squadron_flight_leader(PlaneSquadronEntity&)` has an added
count guard and semantic backing, so it cannot implement this native getter.
Concurrent mutation, structural reentry, invalid/fault paths, enclosing class,
constructor, arena and game bindings remain unbound. The public const-void-pointer
return preserves identity through a new C++ view/cdecl interface, not an original
ECX/class ABI entry.

The only observed entry xref is raw profile DATA `00D088FC`: `00D087C0+13Ch`
contains `007ED010`. Actual constructor instruction `007F2CAD` stamps
`00D087C0` into the squadron root. Both regions match PE/live. This slot is distinct
from sorted insertion `007ED0D0`, which has no table xref. Raw image profile words
remain uncallable Source tokens; no class/profile/table adapter is introduced.

The real generic weapon-message preflight in `00721A40` loads its owner's +34h
identity; `00721A53..00721A65` tests that receiver, loads its profile/+13Ch target,
then calls EDX at `00721A63`. This is an INDIRECT call, not a direct call row to
this getter. Its target is `007ED010` only for the actual `00D087C0` receiver
profile. The subsequent returned-object dead-byte check and world/owner identities
remain external. Current `gameunit_apply_set_command_00721a40` Source in
`src/cruise_command.cpp` projects the later 5Ch arm; it does not close or adopt
this native preflight. No whole preflight/weapon caller or profile dispatch claim
is made by this leaf.

One unique ignored fixture compiles FOUR fresh TUs: new reader, unchanged exact
sorted producer, unchanged Source leader provider, and the probe. Genuine live
Source-shaped squadron/plane cells have static offset assertions and full backing
guards. A reader view is bound before the exact Source sorted producer runs ONCE
per invocation, publishing index 0 before existing indices 1/4. The unchanged,
unrelocated original7B and new Source reader both return that SAME published plane.
The current Source leader and final adopter consume its actual live +9D8h cell.
The fixture then sets count to zero with the pointer retained and later publishes
null into the SAME slot. Both readers observe those values without modifying any
backing/guards. These controlled setup stores are Source instrumentation, not
observed native retirement, reentry or lifetime behavior. All shaped objects stay
alive throughout. Unexercised higher tuning/controller services throw rather than
supply defaults; no cruise/constructor or weapon-preflight body is invoked.

The final strict `/W4 /WX /fp:strict /O2 /MD` Win32 run passed 14 explicit checks.
Only original7B reader code executes: zero instruction changes, CALL bridges or
table relocations. Profile/constructor bytes are pinned as DATA evidence only.
The Source COFF reader loads its view at +00, cell-reference address at +04,
ONE actual pointer at +07 and returns at +09. No count/CMP/CALL/default is emitted.
The full factory COFF performs address-only checks and stores root/reference at
+34/+36; no cell value is read on its admitted path.

Recipe: `local/cc11_plane_squadron_first_plane_build.ps1`; schema-1 manifest:
`local/cc11_plane_squadron_first_plane_manifest.json`, SHA256
`a20b15e1bbe5c5682151c2ee20ac0d1dfabbf0a5c04c3b372b01ce61869da77d`.
All 36 Source/recipe/support inputs and three selected toolchain executables match
before/after; all 28 quoted project headers also match actual compiler includes.
Current primary core `f1ed060c4d20b2a461b92cf35d3691d80d02daf113b08b53bc4dd5dab999da43`
and Lua/zlib were frozen with source-before=copy=source-after before a root rebuild.
The executable is PE32/I386 with embedded manifest 24/1/asInvoker. An initial
same-case pass had pinned Hostx64 binaries while vcvars32 selected Hostx86; the
final fresh rebuild pins the actual selected tools. Counts are not aggregated;
no cases were added. Old 24/31/165/136 and earlier families were not rerun or changed.

General `007ED260` live reindex remains a separate unfinished dependency at this
handoff. Its five-member scans can read beyond the five-word local stack table:
old +9D0 values `[0,1,2,3,4]` reach odd5 at ESP+24 (incoming return word), and
`[0,2,1,4,3]` reach even6 at ESP+28 (following caller word). This packet adds no
candidate guard, replacement reindex, geometry or numeric kernel. Primary owns
that native frame/read contract. Full build/integration, CMake/ledgers/Ghidra
annotations and actual ABI/world/game validation also remain primary-owned.
