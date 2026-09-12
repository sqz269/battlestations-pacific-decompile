# Landscape class predicate and path party gate

Addresses: 004F1360, 007B38D0 (Party producer evidence only).

`scene_landscape_is_kind_of_004f1360(dynamic_class_id, requested_class)` returns
the exact native EAX value of the Landscape class predicate: 1 for requested
class 44h, 1, 0, or the actual dynamic field at entity+C4h; otherwise 0.
The caller must first establish Landscape vtable dispatch. This does not
construct a native entity or reinterpret arbitrary records as Landscapes.

| Routine | Body and original ABI | Coverage |
| --- | --- | --- |
| 004F1360 | 004F1360..004F1386; ECX entity, stack class, EAX 0/1, RET4 | Complete predicate, using an explicit C++ value interface rather than native pointer/ABI access. |
| 007B38D0 | 007B38D0..007B3912; ECX entity, no stack arguments, RET | Partial evidence only: 007B38D8..007B38FC holder-kind check, Party lookup and entity+54h write. Base setup at 007B38D3 and path load at 007B390C are consumed boundaries, not reconstructed here. |

## Concrete dispatch

The existing scene registration table maps `Landscape`, class44h, to creator
004F1460. At 004F14CE it calls constructor 004F11C0. That constructor stores
vtable00CEA090 at 004F1272 and class44h to entity+C4h at 004F1338. ESI retains
the constructor's incoming ECX across both stores. The four bytes at
00CEA0EC (vtable+5Ch) are `60 13 4F 00`, resolving the predicate to 004F1360.
The other reference to that vtable is 00882900; no claim about that routine's
construction/lifecycle is made by this packet.

004F1360's entire body is 39 bytes, has no gaps and calls no base predicate.
EAX comes from `[ESP+4]`. It compares 44h, 1, zero and finally `[ECX+C4h]` in
that order. The false arm zeros EAX and returns at 004F137C; the true arm sets
EAX=1 at 004F137F and returns at 004F1384. Both RET instructions pop four bytes.
The existing `entity_class_ids` table already describes the same ancestor set;
this source adds the exact address-named callable boundary without changing it.
The existing correct Ghidra name `BSP_Landscape_IsKindOf` is preserved.

0041CCD0 obtains the immediate parent at 0041D06D and 0041D07B, loads that
parent's vtable+5Ch at 0041D084 and invokes it with 44h at 0041D089. Only an
AL!=0 answer permits storing the same immediate parent at zone+10h. The
report marks this as an indirect call whose concrete Landscape target is
proved by the constructor/vtable bytes, not by a direct call-graph edge.

## The separate party gate

0041D1E0 reads its entity argument into EDI at 0041D1F9, passes it to
007AC9D0 at 0041D201 and compares `[EDI+54h]` with 2 at 0041D206.
Filtering the full listing for EDI confirms its next write is 0041D284, after
this gate. EAX's path-interface result is copied to EBX at 0041D20A; it is
not the object whose +54h field is read. 007AC9D0 preserves the callee-saved
EDI register and returns entity+1E4h when the entity accepts class47h.

The field producer is Path setup 007B38D0: ESI captures the actual entity;
for a nonnull entity+C0h holder with kind1, 007B38F4 finds `Party` (literal
00CE5804), 007B38F9 reads property+0Ch and 007B38FC writes entity+54h. The
PathPoints loader's source-holder kind1 is a separate enum and must not be
supplied to the party gate. Installed `universe/library/global.enums` lines
1705..1710 declares Allied=0, Japanese=1, Neutral=2. Integration should pass
the retained record's resolved `party` value to this gate.

## Verification and limits

All Ghidra batches used the configured read-only wrapper, which verifies
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. No Ghidra writes.
The report records build, installed-parent checks and a manifested isolated
x86 native-byte comparison. No native Landscape creator, full Path initializer,
geometry association runtime or original-game behavior is reconstructed here.

Win32 Release build and the existing reconstructed_math test passed. The
isolated manifested probe copied the exact 39-byte native body and compared
six queries covering the fixed classes, dynamic equality and false result;
all returned the same EAX value as the reconstructed callable. The parser
reported no errors on installed Marshall and verified these visitation IDs:

| Scene ID | Authored name | Authored class ID | Predicate query44h |
| --- | --- | --- | --- |
| 1 | Landscape 03 | 44h | 1 |
| 93 | Landscape 04 | 44h | 1 |
| 99 | Landscape 05 | 44h | 1 |
| 105 | Landscape 06 | 44h | 1 |

All 21 AvoidZone Path entities explicitly author `Party = E Party : Neutral`.
These IDs match the actual scene-retention runtime log from
`reports/game_scene_zone_paths.json`. The report-call checker passed all four
rows, identifying 0041D089 as an indirect call that needs the vtable evidence
above. The probe log is `local/scene_landscape_class_probe.log`.
