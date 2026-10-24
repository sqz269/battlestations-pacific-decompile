# October 24 render and pool identifier audit

This follow-up audits the baseline `unknown_no_identifier` groups assigned by the tail
integrator: 158 rows in `src/render_fx_cc13.cpp` and 92 in `src/native_material_pools.cpp`.
The baseline is `515837c10` / `local/output/oct24_wire_before.json`. No C++ source,
Ghidra analysis, host wiring, or game process is changed.

## Result

| Group | Baseline unknown | Verified body mappings | Metadata-only corrections |
|---|---:|---:|---:|
| Render shared shapes | 158 | 127 | 31 |
| Native material / GUI / bush pool engine | 92 | 92 | 0 |
| Total | 250 | 219 | 31 |

`config/wire_identifiers_render_pools.json` uses schema version 1 and maps each address to
an actual executable body identifier in its recorded source. These are existing shared
algorithms, not 219 newly written bodies. Every mapping includes the exact table shape
or trait/address contract and preserves existing ABI/storage/ownership qualifications.
The 219 addresses are unique, disjoint from the 31 corrected rows, and do not duplicate
the integrator's high/low identifier manifests at validation time.

## Render mappings

Each render mapping joins an actual `RoutineRow` address/Shape/stride_or_offset/extra row
to its shared C++ algorithm. The parameters remain part of the evidence; mapping does not
supply a new native wrapper. Source bodies and header shape contracts were inspected;
no table accessor such as `find_routine_row` is used as an implementation identifier.

| Actual C++ algorithm | Mapped addresses |
|---|---:|
| `array_clear_count_down` | 9 |
| `array_clear_via_resize` | 1 |
| `array_destroy_count_down` | 1 |
| `array_destroy_via_resize` | 12 |
| `array_free_data` | 1 |
| `array_pop_back_destroy` | 3 |
| `array_push_back_via_reserve` | 23 |
| `array_reserve_min_one` | 4 |
| `array_resize` | 7 |
| `com_reference_count` | 4 |
| `com_slot_release` | 5 |
| `owned_slot_delete` | 4 |
| `pointer_delete_with` | 1 |
| `pooled_string_release` | 3 |
| `ref_slot_release` | 4 |
| `slab_all_slots_free` | 9 |
| `slab_has_no_free_slot` | 9 |
| `slab_pop_free_slot` | 9 |
| `slab_push_free_slot` | 9 |
| `slab_stamp_index` | 9 |

The array models preserve row-specific capacity rules, fill policy, element stride, and
owner/field adjustments. Slab helpers preserve the recorded slot size and 32-slot metadata
layout. Ref/COM/owned/string helpers preserve the selected ownership operation and require
the corresponding host/service contract. They retain their existing new/delete versus native
allocator and original register-ABI qualifications; this audit does not broaden them.

## Native pool mappings

Fourteen GUI trait rows in `include/bsp/native_gui_widget_pools_cc13.hpp` explicitly bind
construct/allocate/free/trim/destroy/init-block addresses to pool constants. The C++ file
explicitly instantiates each trait's `NativeGuiWidgetPool<T>` methods, which delegate to
`initialize_pool`, `allocate_slot`, `return_slot`, `trim_empty_slabs`, `destroy_pool`, and
`initialize_slab`. Their implementations operate on the native 38h pool header, real
critical section and recursive depth, slab free-index stack/count, table capacity and
first-free index. The trait constants remain authoritative.

The BushGroup instance is bound by `include/bsp/terrain_bush_pool_cc13.hpp`: five ordinary
methods and the existing free thunk are among the 92 unknown rows. Its recorded comparison
matches the GUI pool engine with its distinct slot/slab constants. The free thunk maps
to target `free` semantics while retaining its global-load/JMP ABI qualification; no new
ABI wrapper is claimed. Two other rows are the actual `initialize_slab<NativeMaterialPool>`
and `initialize_slab<NativeMaterialParameterPool>` algorithms, documented at the common
initializer in the C++ source.

| Member / algorithm | Mapped addresses |
|---|---:|
| `allocate` | 15 |
| `destroy` | 15 |
| `free` | 16 |
| `initialize` | 15 |
| `initialize_slab` | 16 |
| `trim` | 15 |

Canonical pool storage, allocator-list domain, companion lifetime, distinct native globals,
thread-lock behavior, payload ownership and failure behavior remain the existing contracts.
No native ABI compatibility or runtime/game validation was added by the manifest.

## Thirty-one table-only rows

The following rows have a table description but no callable implementation binding or
executable dispatcher for that row. Generic `rd32`/`wr32` field helpers, an unrelated
constant/no-op body, or a table lookup are insufficient. In particular, the three record-at
shapes require distinct base/offset handling, and locked getters require the native critical
section/depth protocol; a bare load cannot stand in for those operations. Forward rows lack
a recorded executable binding even if their native target behavior is known.

All 31 whole-function records were corrected through the lease-checked
`bsp.py ledger add-function --json-file ... --replace` batch command to
`metadata_only_no_function_body_verified_oct24`. Each previous
`reconstructed_build_tested` status is saved as `previous_status`. Source, name, original
ABI and all other existing fields remain identical; original evidence is retained and
an address/shape-specific explanation is appended. They stay visible as missing-body
work in the integrator's target policy, with no reconstruction credit.

| Address | Table shape | Row contract |
|---|---|---|
| `00b34d60` | `kConstructEmptyPair` | `PooledString_ConstructEmpty_Unreferenced`; stride/offset `0x0`, extra `0x0` |
| `00b3cd30` | `kGetDword` | `D3D9Surface_GetSampleCount`; stride/offset `0x24`, extra `0x0` |
| `00b3ce20` | `kSetDword` | `D3D9Surface_SetSampleCount`; stride/offset `0x24`, extra `0x0` |
| `00b3ce40` | `kReturnZero` | `D3D9Texture2D_GetKind`; stride/offset `0x0`, extra `0x0` |
| `00b3cf60` | `kReturnConstant` | `CubeTexture_GetKind`; stride/offset `0x0`, extra `0x1` |
| `00b3cf70` | `kGetDword` | `CubeTexture_GetReportedWidth`; stride/offset `0x24`, extra `0x0` |
| `00b3cf80` | `kGetDword` | `CubeTexture_GetReportedHeight`; stride/offset `0x28`, extra `0x0` |
| `00b3d080` | `kReturnConstant` | `VolumeTexture_GetKind`; stride/offset `0x0`, extra `0x2` |
| `00b3ff70` | `kGetByte` | `NativeShaderReflectionOwner_GetByte74_Unreferenced`; stride/offset `0x74`, extra `0x0` |
| `00b40050` | `kRecordAt` | `ShaderEffect_GetPassRecord`; stride/offset `0x40`, extra `0x144` |
| `00b400b0` | `kGetByte` | `MaterialEffect_GetByte13C_Unreferenced`; stride/offset `0x13C`, extra `0x0` |
| `00b405d0` | `kRecordAt` | `Float4Block_At_Unreferenced`; stride/offset `0x10`, extra `0x4` |
| `00b47c00` | `kForward` | `VertexDeclarationPool_ReturnRawSlotUnwind`; stride/offset `0x0`, extra `0x0` |
| `00b47c10` | `kForward` | `HardwareLayout_ReturnRawSlot_Unreferenced`; stride/offset `0x0`, extra `0x0` |
| `00b47c80` | `kRecordAt` | `VertexDeclaration_ElementAt_Unreferenced`; stride/offset `0x14`, extra `0xC` |
| `00b488d0` | `kForward` | `HardwareLayoutPool_AllocateRawSlotStub_Unreferenced`; stride/offset `0x0`, extra `0x0` |
| `00b48cc0` | `kGetDword` | `D3D9LogicalVertexStream_GetFlags`; stride/offset `0x60`, extra `0x0` |
| `00b48d80` | `kGetDword` | `D3D9LogicalIndexStream_GetFlags`; stride/offset `0x10`, extra `0x0` |
| `00b48d90` | `kGetDword` | `D3D9LogicalIndexStream_GetCount`; stride/offset `0x14`, extra `0x0` |
| `00b48da0` | `kGetDword` | `D3D9LogicalIndexStream_GetFormat`; stride/offset `0x18`, extra `0x0` |
| `00b49960` | `kForward` | `LogicalVertexPool_ReturnSlotUnwind`; stride/offset `0x0`, extra `0x0` |
| `00b49970` | `kForward` | `LogicalIndexStreamPool_ReturnSlotUnwind`; stride/offset `0x0`, extra `0x0` |
| `00b4b480` | `kNoOp` | `PhysicalIndexBuffer_NoOpSlot10`; stride/offset `0x0`, extra `0x0` |
| `00b4b520` | `kNoOp` | `PhysicalVertexBuffer_NoOpSlot10`; stride/offset `0x0`, extra `0x0` |
| `00b4ec30` | `kConstructEmptyPair` | `PodPair_ZeroInit`; stride/offset `0x0`, extra `0x0` |
| `00b4ec40` | `kNoOp` | `PostEffectOwner_NoOpSlot0C`; stride/offset `0x0`, extra `0x0` |
| `00b54cc0` | `kNoOp` | `BloomOwner_NoOpSlot0C`; stride/offset `0x0`, extra `0x0` |
| `00b5df90` | `kLockedGetDword` | `LockedObject_GetField40`; stride/offset `0x40`, extra `0x48` |
| `00b5dfc0` | `kLockedGetDword` | `LockedObject_GetField3C`; stride/offset `0x3C`, extra `0x48` |
| `00b5dff0` | `kLockedGetByte` | `LockedObject_GetByte44`; stride/offset `0x44`, extra `0x4C` |
| `00b5e020` | `kLockedGetByte` | `LockedObject_GetByte45`; stride/offset `0x45`, extra `0x4C` |

`00b4b480` was one of the original outside-partition implementation records in
`docs/OCT24_TAIL_OUTSIDE.md`. This follow-up refines that baseline classification:
the outside recorded-only subtotal becomes 54 and the existing-implementation-record
subtotal becomes 255; the four partial bodies and three compiler projections remain.
The original 316 outside count is unchanged as a partition statistic.

## Validation and limits

- Valid JSON schema; 219 unique mapping addresses; 219 + 31 exhausts the assigned 250 rows.
- Every simple identifier matched an actual C++ definition using the integrator's current
  `DEF_RE`, including templated `NativeGuiWidgetPool<T>::` qualifiers; no table accessor is mapped.
- No duplicate addresses against the high/low manifest files present during validation.
- Strict field preservation for all 31 ledger corrections; unchanged shard rows identical;
  no added or deleted reconstruction record keys. Index refreshed after edits; diff check passed.
- No C++ change, added test, build, fixture execution, native ABI check, or game run.

The wiring report is explicitly a source/file-granular approximation. A shared identifier
being used in a reachable source file does not prove that every native instantiation or
parameter specialization is installed or executable. These mappings resolve body identity;
they do not turn the source-level report into runtime wiring or game-validation proof.
The integrator must rerun its final board/readiness reports after combining all lanes.
