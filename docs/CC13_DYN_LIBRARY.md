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
  unreferenced function. They were **not defined**, because defining them is a Ghidra mutation of a scale the
  lead should decide. One consequence: the index attributes call sites inside those gaps to the preceding
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

## Tag regeneration

`tools/build_tag_ledger.py` builds `block_dyn` only from the `00C30930` range of `rtti_middleware.json`, and
it skips addresses that have ledger names. A regeneration therefore drops these 285 rows. The names survive
in `config/names`, and the rows can be restored from this commit. `write_tags` was not used.

## Tools (worktree-local, not committed)

`local/dy1_xscan.py` (rel32 + absolute scan for given targets), `dy1_refs.py` (scan for a range plus RTTI),
`dy1_dec.py`, `dy1_classify.py`, `dy1_final.py` (merge, then `ledger add-name`), `dy1_tags.py` and
`dy1_gaps.py` (undefined starts after INT3 padding).

## Handoff

Done: all 286 names are in the ledger and in Ghidra, the 285 tags are written, and the index is rebuilt.
Open, for the lead:
1. Decide whether to define the about 665 unreferenced functions Ghidra lacks in this segment
   (`local/dy1_gaps.py` lists them). Defining them would raise the raw count. They are dead code, so they
   could instead be bulk-tagged as dead once defined.
2. The 15 game-side raw functions listed under Result.
3. The two 10h-element pools (SAPPair and SAPEndpoint) are told apart by address order only.
