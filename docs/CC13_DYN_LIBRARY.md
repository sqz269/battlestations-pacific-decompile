# Segment 0 (00401010-004138D0): the Dyn/Mit header code, identified (cc13-dyn1)

Worker cc13-dyn1, 2026-10-12, requested by cc9. The pass covers names and tags only: no C++, no edits to
cc9's Dyn reconstruction files (cc9-physics1 owns those), and no function definitions. The status
vocabulary is **exported and annotated**: names are in the ledger and in Ghidra, and tags are in
`config/tags`. Nothing here is reconstructed, build-tested or game-validated.

## Result

| | before | after |
|---|---|---|
| raw `FUN_` in 00401010-004138D0 with no ledger name and no tag | 301 | 15 |
| names recorded (`config/names/00400000.jsonl`, `00410000.jsonl`) and applied in Ghidra | | 286 |
| tag rows added (evidence id `cc13dy1`, `config/tags/00400000.jsonl`, `00410000.jsonl`) | | 285 |

The tags break down as 249 `block_dyn`, 35 `stl_instantiation` and 1 `crt_unmatched` (`_freea`).
`BSP_FormatString100_00412F80` is named but not tagged, because it is a game helper.

The 15 left raw are game-side and outside the Dyn/Mit block. They are 00410540, 00410610, 004106A0,
00410790, 00410820, 00410850, 00410B90 and 00411EB0 (bot-state registry neighbours: functor vtables at
00CE374C/00CE3784, forwarders through `*(this+8)`, a lock guard), 00412010 and 00412070 (a 2D grid
step and a bounds-checked lookup), 00412F40, and the vector destructors 004104E0, 00412CD0, 00412DC0 and
00412EF0. They were left for the owners of those areas.

## What segment 0 is

- **Dyn is in-house, not third-party.** The source paths `y:\mitengine\sources\physics\source\...`
  (`DynConvexMeshShape.cpp`, `DynBody.cpp`, `Containers/DynFixedVector.h`) and the `Mit` namespace place it in
  Mithis's own engine, as `reports/library_inventory/rtti_middleware.md` section 1 already says. There is no
  public source to align against, so every name is **descriptive and provisional**, built on the existing
  `DYN_<Class>_<Verb>_<ADDR>` and `Mit_PoolAllocator_<N>_<P>_<Verb>_<ADDR>` conventions. The disabled asserts
  name three container headers (`mitmaximizedarray.h`, `MitStlCloneVector.h`, `DynFixedVector.h`). They are not
  referenced, and no container below is tied to a specific header.
- The block is the **first linked object**. It holds Mit core math, the Dyn header instantiations (container
  templates, fixed-block pools, GJK helpers, profiler timer), two global `Mit::cPoolAllocator`s, then
  Dinkumware `std::string`/`std::_Allocate` instantiations and game code from about 00410240 on.
- **Most of it is unreachable.** 201 of the 286 named functions have no rel32 CALL/JMP and no absolute
  reference anywhere in the PE, and 42 more are reached only from unwind funclets. The image keeps
  unreferenced code: 007BDC30 and 007BDC60 are dead out-of-line copies too (`docs/CC13_PLANE_WANDER_RNG.md`).
- **Ghidra is missing about 665 more functions here.** That is the count of 16-aligned starts after INT3
  padding that are not function starts in the index (`local/dy1_gaps.py`). Of these, 97 lie below 00403610,
  388 in 00403610-00410240 and 180 above. A sample (00401030, `char_traits::assign`-shaped) is a real,
  unreferenced function. They were not defined in the first pass. The lead then approved defining them, and 717 are now
  defined (see "Defined bodies"). One consequence: the index attributes call sites inside those gaps to the preceding
  defined function. For example, the index lists "callers" 0040C520 and 0040C570 for 0040EE00, and both are
  wrong.

## Method

1. Took the 301 untagged, unnamed `FUN_` in the range from the index.
2. Ran `local/dy1_refs.py` over the PE on disk. It finds every E8/E9 rel32 to each start, and every
   absolute dword equal to it in every section: byte-step in `.text`, 4-aligned elsewhere. Each `.rdata`
   hit is resolved through RTTI (COL -> TypeDescriptor) to a vtable and slot. The positive control is
   DYN_BodyPool_Construct_00409170, which returns its known caller Dyn_World_Construct (two sites).
   Ghidra's own xrefs report nothing for the sampled dead functions (0040E050, 00404DC0).
3. Live-decompiled all 301 (`local/dy1_dec.py`) and classified the container shapes with
   `local/dy1_classify.py`: push_back with capacity `2c+2`, resize, reserve, constant reserve, set-capacity,
   copy-construct, assign, free-buffer, pool allocate/add-page/free-pages. The element size comes from the
   `operator new` multiplier. Everything else was read by hand, with assembly where the pseudocode dropped
   a return (`0040C520`).
4. Pool classes are attributed by element size and address order next to the already named constructors.
   The order is Body 88h (`00409170`), Motion C8h (`00409450`), Manifold E0h (`0040A1E0`), SAPPair 10h
   (`0040AC90`), the 44h pool, SAPEndpoint 10h (`0040B4A0`) and SAPProxy 50h (`0040B750`). The second set of
   seven at `0040DCF0..0040EC30` repeats that order. The two 10h pools are separated by order alone, which is
   provisional.

## Families

| family | addresses (examples) | evidence |
|---|---|---|
| Mit math | `Mit_Vec3_Cross_004015A0`, `_Normalize_004016D0`, `_RotateAroundAxis_004019A0`, `Mit_Mat33_MulVec(Transposed)`, `_Multiply_00402660`, `_Orthonormalize_00402870`, `_RotateAroundAxis_004029B0`, `Mit_Transform_InverseTransformPoint` x2, `Mit_Aabb_TransformToWorld_00403280`, `Mit_Aabb_GetCenter_00403510`, `Mit_Math_LineLineClosestParams_00404060`, `Mit_Math_MinAbsFloatByRef_0040C520`, `Mit_Mat44_Multiply_00413560` | all dead; register ABIs (EAX/ECX/EDX/ESI inputs) in each ledger record |
| Mit pools | `Mit_PoolAllocator_36_128_{Construct 00407590, DestroyGlobal 00407690, Allocate 004077F0, Free 00407960, PageConstruct 004084D0}`; `Mit_PoolAllocator_20_128_{Construct 0040F910, DestroyGlobal 0040FA10, Allocate 0040D860, PageConstruct 0040DA40}` | vftables `Mit::cPoolAllocator<36,128>` / `<20,128>`, registry `[00E188B4]`, slot stride 28h / 18h, pages 1504h / D04h, free stacks of 80h shorts |
| Dyn pools | `DYN_<Body,Motion,Manifold,SAPPair,SAPEndpoint,SAPProxy>Pool_{Allocate,AddPage,FreePages}`, `DYN_Pool68_*`, `DYN_Pool68_Release_0040B030`, `DYN_SAPProxyPool_Release_0040B880` | 999-element malloc pages (`0x21340`, `200000`, `0x36B00`, `16000`, `68000`, `80000`) |
| Dyn containers | `DYN_Vector<N>_{PushBack,Resize,Reserve,ReserveK,SetCapacity,SetCapacityK,CopyConstruct,Assign,Insert}`, `DYN_Vector_FreeBuffer`, `DYN_Vector_ConstructEmpty_0040AB80`, `DYN_Vector_Tidy_0040C720`, `DYN_VectorOfVectors_*`, `DYN_Array48/88_ReserveDiscard` | `{data, size, capacity}`; `<N>` is the element size in bytes |
| Dyn objects | `DYN_{TaskManager,World,Engine,CollisionSystem,ManifoldContainer}_Delete`, `DYN_ProfileNode_{Delete,Destruct,DeleteTree}`, `DYN_ConvexHull_{DeleteInternal,FreeInternalBuffers,ConstructCopyAt}`, `DYN_SAPBroadPhaseManager_DestroyAxes_00404250`, `DYN_TriangleShape_SetVertices_00405AB0`, `DYN_Shape_UpdateWorldAabb_00403CE0`, `DYN_Island_PushContactBodies_004070B0` | each wraps or inlines a named Dyn routine, or matches a layout in `docs/DYN_PROFILE_SCOPES.md` or the SAPBroadPhaseManager constructor `00C37030` |
| Dyn algorithms | `DYN_GeneralConvex_Simplex{TriangleRegion 00404DC0, EdgeRegion 00406510, FaceSide 00406860, FaceRegion 00406B70}`, `DYN_RadixSort_UInt32Key` x2, `DYN_RadixSort_FloatKey_0040BB10`, `DYN_EdgeHashMap_{Find,FindOrInsert,Clear,Destroy,CopyKeyValue,CopyValue}`, `DYN_ProfileTimer_{CalibrateFrequency 00407000, ElapsedSeconds 004041D0}`, `Mit_FormatString1024_0040CFC0` | double-precision GJK simplex layout (W +98h, support points +F8h/+158h); radix sort histograms; the edge map (two-short keys, 128 buckets) used by `DYN_ConvexHull_BuildVertexAdjacency_00C389C0` |
| STL / CRT | `BSP_LegacySboString_{ConstructFromCString,AssignCString,Tidy,Grow,CopyConstruct}`, `std_logic_error_{dtor,what}`, `STL_allocate_<N>B` x11, `STL_deallocate` x15, `STL_vector{8,12}_size`, `_freea` | Dinkumware shapes (SBO capacity 0Fh, `bad_alloc` overflow check); `_freea`'s 0DDDDh marker |

## Game-specific changes to Dyn

None can be seen in segment 0. Without source there is nothing to diff, and the header code is generic.
One behaviour matters for the host: `DYN_ProfileTimer_CalibrateFrequency_00407000` runs from a static
initializer (`00CC89E0`) and calls `Sleep(100)` between two `rdtsc` reads. That is a 100 ms start-up cost,
and it fixes the profiler's ticks-per-second in `[0109E9E8]`.

## Corrections to earlier records

- `reports/library_inventory/rtti_middleware.md` lists "allocators 00407960/004077f0" as Dyn dependencies.
  They are `Mit::cPoolAllocator<36,128>` Free and Allocate. Their callers are the convex-hull triangle
  builders (`00C51480`, `00C5CAE0`, `00C5D900`, `004060E0`).
- `0040FA10` (the `<20,128>` destroy) is tail-jumped from the stub at `00CD9250`, which is not a Ghidra
  function. `BSP_DynConvexShapePool_StaticDestroy` at `00CD9240` jumps to `00407D70`, the `<532,128>` destroy,
  which agrees with its name. The index's "caller 00CD9240" for `0040FA10` is a gap-attribution artefact.
- `0040D860` has callers `00407B71` (FindOrInsert) and a tail jump at `0040CD90`. That jump is code after
  `0040CBE0`, not in it.

## Tag regeneration (fixed, commit 29dada4f2)

`tools/build_tag_ledger.py` used to drop every kept tag row whose address has a ledger name. So a
regeneration lost these 285 rows, and the 411 `cc13ll1` rows from the Lua pass too. The tool now keeps a row
whose `source` is a `docs/` review pass for as long as the ledger name still equals the tag's name.

To check it, I regenerated into the worktree with both the old tool and the new one, diffed each against
HEAD, and then restored `config/tags`:
- the old tool loses 2196 rows, including all 285 of `CC13_DYN_LIBRARY` and all 411 of `CC13_EMBEDDED_LUA_LIBRARY`;
- the new tool loses 1500;
- the difference is exactly those 696 rows.

The other 1500 rows were already being dropped before this change. They are inventory rows that a review
superseded or that the reports no longer derive. Evidence ids are renumbered on regeneration (`cc13dy1` becomes
`eNNNN`), but their texts survive. The tag rows for the defined bodies (below) carry no ledger name, so they
survive the same way as `stl_shape` does.

## Defined bodies (follow-up, same day)

Following `docs/CC13_UNDEFINED_FUNCTION_SWEEP.md`, I defined **717 functions** in 00401010-004138D0 and
recorded them in `reports/cc13_dyn1_defined_functions.json`. The planner is `local/dy1_gapdef.py`.
- **Start evidence.** A start is 16-byte aligned, follows INT3 padding and is not already an entry (703
  candidates). I also took 23 `aligned_after_ret` starts: aligned code that directly follows a planned body's
  RET or JMP, which the descent from that body does not reach.
- **Body.** The body comes from a Capstone recursive descent bounded by the next entry. The descent fails on a
  conditional branch out of the gap, a decode failure or a table jump. `jmp reg` and `jmp [reg+disp]` are
  accepted as tail calls. The reached instructions must tile the body without overlap, and only INT3 may sit
  between the body end and the next entry.
- **Containment, checked two ways.** First, that padding test. Second, live `get_function_by_address` at the
  start, middle and last byte: four candidates were inside existing bodies (`004019A0`, `00404DC0`, `00405D80`,
  `0040C790`) and were skipped. Five more failed the descent (`00404E30`, `00406730`, `00406800`, `004086E0`,
  `0040F070`).
- **Writing.** I used `tools/ghidra_define_function.py` in batches of 80 under the write lock and a range lease.
  It made 716 new definitions; `0040D9D0` already existed. Afterwards, all 717 have a Ghidra body that starts and
  ends where planned (`local/dy1_verify.py`).
- **After.** `_free` (00BF6989, 00BF65AC) keeps `void __cdecl _free(void *)`, and the code after a `_free` call
  still decompiles (0040E050). One `snapshot --force` and an index rebuild followed.
- **References.** 696 of the 717 have no rel32 or absolute reference. The 21 that do are the CRT
  `memcpy_s`/`memmove_s` wrappers (00401060, 00401090), vtable slots (`00CE3744` slots 11-15 and 22, and
  table entries at 0117DB74 and 0117DEB4), a thunk target (0040D9D0), and a few `.text` dword matches.
- **What they are.** Mostly tiny: `char_traits` helpers, float wrappers around sin/cos/sqrt/abs/atan2,
  min/max, register-ABI vector and matrix inlines, and getters.
- **Tags:**
  - 520 below 00410240 have generic names and the tag `block_dyn` `DYN_header_<addr>` (medium confidence,
    evidence `cc13dy1h`). That is the first object's Dyn/Mit header-instantiation run; it includes the Mit core
    math wrappers. The class of each is not identified.
  - 8 are STL/CRT-shaped and tagged `stl_instantiation` `STL_inst_<addr>` (evidence `cc13dy1s`).
  - 189 above 00410240 are game-side and stay untagged.
- **Count.** Segment 0 now has 205 untagged `FUN_`: those 189, the 15 earlier ones, and 004138D0 (the next
  segment's first entry). The brief's image-wide untagged count is 14917.
- **Index.** The call sites inside the former gaps now resolve to their own functions.

## Tools (worktree-local, not committed)

`local/dy1_xscan.py` (rel32 + absolute scan for given targets), `dy1_refs.py` (scan for a range plus RTTI),
`dy1_dec.py`, `dy1_classify.py`, `dy1_final.py` (merge, then `ledger add-name`), `dy1_tags.py` and
`dy1_gaps.py` (undefined starts after INT3 padding).

## Handoff

Done: all 286 names are in the ledger and in Ghidra, the 285 tags are written, and the index is rebuilt.
The follow-up is also done: the tag-regeneration fix and the 717 defined bodies (sections above).
Open, for the lead:
1. The 9 skipped candidates (listed under "Defined bodies"). The four inside existing bodies suggest those
   Ghidra bodies (`004019A0`, `00404DC0`, `00405D80`, `0040C790`) span INT3 padding into further code. They
   may be truncated, or merged with a neighbour, and are worth a look with `disasm-raw`.
2. The 15 game-side raw functions listed under Result, plus the 189 game-side bodies defined above 00410240.
3. The two 10h-element pools (SAPPair and SAPEndpoint) are told apart by address order only.
