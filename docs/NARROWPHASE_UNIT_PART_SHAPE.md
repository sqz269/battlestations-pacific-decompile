# The unit-part collision shape: who builds it, what it selects, and what the host must offer

Addresses: 00723D60 00723AA0 007238E0 00722B20 0085CAD0 0085D020 0085BF90 00723B70 00712440
007135C0 0087BCC0 0098AC20 0098AC82 00CFD768 00CFD7B8 0092D1F0 00937C90 00727310 0070F6F7
0071067E 00710A2C 00711046 00712685

Packet `cc7_narrowphase_unit_part_shape`, **read-only in Ghidra** (no renames, comments, function
creation, prototypes or saves). Every descriptive name below is a hypothesis, not a recovered
symbol. `docs/HIT_HULL_SEGMENT.md`'s two chains, `docs/HIT_NARROWPHASE.md`'s record table and shape
interface, `docs/PART_DAMAGE_REACHABILITY.md`'s kind table, `docs/GEOM_MESH_RESOURCE.md`'s payload
decode and `docs/SHIP_HIT_RECORD.md`'s rules R1 and R4 are cited, not restated. This installation is
**modded** (BSPRM/AlterBSP); every data file read is listed with its mtime in §6.

## 0. The answer

The chain is complete natively and there is no missing native link. Every step:

| # | site | step |
| --- | --- | --- |
| 1 | `0087BDF5`/`0087BDFB` | `model = *(*(unit+354h) + 50h)` |
| 2 | `0087BE3C`/`0087BE4B` | `lod = unit->vtable[190h](1.0f or DAT_00CED9E0)`; the constant is picked by `unit->vtable[5Ch](1Bh)` at `0087BE0D` |
| 3 | `0087BE3A`/`0087BE48`/`0087BE52` | `partSet = model->vtable[8h](lod)`, `this` = the model. **This is the peer boundary** (§5) |
| 4 | `0087BE58` (and `0087BE9B`) | `007135C0(instance /*ECX, operator new(1ACh) at 0087BE1C*/; unit, partSet)` |
| 5 | `007135C0` | stores the unit at `+4Ch`/`+164h`, the part set at `+160h`, installs node vtable `00CFD7B8` |
| 6 | `007136F2` | `00712440(instance)` — guarded by the non-null unit test at `007136E3` |
| 7 | `00712440` | per `{geometry, transform}` pair in the vector `[instance+160h]+40h..+44h`, builds a `28h`-byte shape with vtable `00CFD768` (`00712685`), files it into the list at `+18Ch` or `+198h`, and for the `+18Ch` ones publishes the shape address into `instance+D0h + instance+F8h*4` and bumps `+F8h` |
| 8 | `0098AC82` | the spatial query's per-entity narrowphase calls `shape->vtable[0]` over `entity+D0h[0 .. entity+F8h)` — the array step 7 filled |
| 9 | `00724510` -> `00723E90` -> `00723D60` | the element scan; `record+30h = element+4h`, `record+34h = element+8h` |

So `docs/HIT_HULL_SEGMENT.md`'s §6 follow-up 1 ("Whoever fills `[node+160h]+40h` is the next step")
resolves to a single virtual call, `model->vtable[8h]`, and the geometry it yields is **the
`GeomMesh` itself** (§2). Nothing between the parser and the consumer is missing; what the
reconstruction lacks is the host object, not native understanding.

## 1. The class that owns vtable `00CFD768`

`00CFD768` is written into `[reg]` at **five** sites, not the two Ghidra's `xrefs` reports
(`00711046`, `00712685`). A `scan-bytes` census of `68 d7 cf 00` finds three more — `0070F6F7`,
`0071067E`, `00710A2C` — all of them `C7 00 68 D7 CF 00` / `C7 01 68 D7 CF 00`
(`MOV dword ptr [EAX],0CFD768h` / `[ECX]`) inside code Ghidra has **not** defined as a function.
`bsp.py`'s "enclosing candidate" attribution (`FUN_0070F4D0`, `FUN_00710630`, `FUN_007109E0`) is the
nearest preceding function, not the owner; `disasm` refuses all three ("no function starts here").
They are listed under `no_ghidra_function` in the report as **byte-level decodes only** — their
extents were not established.

The bytes around each of the three decode as the same field set the constructed shape has:

```
00710672  d9 58 04   FSTP [EAX+4]        00710a20  d9 59 04   FSTP [ECX+4]
00710675  d9 41 08   FLD  [ECX+8]        00710a23  d9 42 08   FLD  [EDX+8]
00710678  89 50 1c   MOV  [EAX+1Ch],EDX  00710a26  89 41 1c   MOV  [ECX+1Ch],EAX
0071067b  d9 58 08   FSTP [EAX+8]        00710a29  d9 59 08   FSTP [ECX+8]
0071067e  c7 00 ...  MOV  [EAX],0CFD768h 00710a2c  c7 01 ...  MOV  [ECX],0CFD768h
00710684  d9 41 0c   FLD  [ECX+0Ch]      00710a32  d9 42 0c   FLD  [EDX+0Ch]
00710687  8b 51 20   MOV  EDX,[ECX+20h]  00710a35  8b 42 20   MOV  EAX,[EDX+20h]
```

so all five are constructors or assignments of one `28h`-byte value type. `00712440` assembles it on
its own frame at `local_28..local_4`, which pins the layout:

| offset | written at | meaning |
| --- | --- | --- |
| `+00h` | `00712685` | vtable `00CFD768` |
| `+04h`, `+08h`, `+0Ch` | `local_24/20/1C` | a float3 seeded from `DAT_00F87574/78/7C` (instruction addresses not read; the decompiled body is the source) |
| `+10h`, `+14h`, `+18h` | `local_18/14/10` | a second float3 from the same three cells; `BSP_CollisionShape_SetBounds(&local_40,&local_34)` overwrites the pair with what `00723170` computed |
| `+1Ch` | `local_c` | the owning collision node — read back at `00724514 MOV EAX,[ECX+1Ch]` |
| `+20h` | `local_8` | `local_5c[1]`, the pair's second word: the transform |
| `+24h` | `local_4` | `*local_5c`, the pair's first word: **the geometry** — read back at `00724517 MOV ECX,[ECX+24h]` |

The shape is not heap-allocated on its own. `BSP_CollisionShapeList_BuyNode(list, list->next, &shape)`
copies it into a list node and the published pointer is `listNode + 8`, so the `entity+D0h` array
holds interior pointers into two STL lists (`instance+18Ch`, `instance+198h`).

The filter that decides which list: `00712440` searches the `10h`-byte group records at
`instance+16Ch..+170h` **from index 1** for a pointer equal to the pair's transform. Found -> list
`+198h` and **not** published into `+D0h`; not found -> list `+18Ch` **and** published into `+D0h`
with `+F8h` incremented. `docs/NATIVE_MODEL_GROUP_SELECTION.md` already owns the `+168h`/`+16Ch`
group vector and `00713380` that builds it.

`instance+D0h` has `0Ah` slots (`docs/HIT_NARROWPHASE.md`); `00712440` does not bound-check `+F8h`
against `0Ah`. **Not read here**: whether anything else caps it.

## 2. The geometry is the `GeomMesh`, and `00723D60`'s selection

`shape+24h` is the `GeomMesh` object `docs/GEOM_MESH_RESOURCE.md` decodes. Three offsets agree and
no third object sits in between:

* `00723D60` walks `[geom+0Ch, geom+10h)` with stride `2Ch`; the parser reserves the element vector
  through `00726D80(mesh+8h, n)`, and an MSVC `vector` whose proxy is `+8h` has begin `+0Ch` and
  end `+10h`.
* `007238E0` reads `element+0Ch` as the mesh (`00723928 MOV EDI,[ESI+0Ch]`), then `mesh+2Ch` and
  `mesh+30h` (`0072392B MOV EAX,[EDI+2Ch]`, `0072392E ADD EDI,28h`, `00723935 MOV ECX,[EDI+8]`) and
  divides the span by 6 (magic `2AAAAAABh` at `0072393A`); the parser builds `mesh+28h` through
  `007262A0` with **6-byte** records, three `u16` per triangle.
* `007239A4 MOV ESI,[ESI+4]` takes the vertex array from `mesh+18h+4`, and the leaf indexes it by
  `v * 12`; the parser fills `mesh+18h` with three floats per vertex through `004215D0`.

### `00723D60`, instruction by instruction

`__thiscall(geom /*ECX*/; const float3* from, const float3* to, float3* outHit)`,
body `00723D60-00723E86`, `RET 0Ch` at `00723E79` and `00723E84` (three stack arguments, from the
cleanup, not the decompiler).

1. `00723D63`..`00723D8D` copy `*to` into the frame as a local far endpoint.
2. `00723D85`..`00723DB5` compute `count = ([geom+10h] - [geom+0Ch]) / 2Ch` (`IMUL 2E8BA2E9h` /
   `SAR EDX,3`), zero when begin is null, and stash it in the incoming `to` slot.
3. `00723D93` seeds the result cell with `EBX = 0`. `00723DBB`/`00723DBF` `CMP`/`JLE` on the count
   exits to `00723E7E MOV EAX,EBX` — a **signed** test, so a negative count returns null too.
4. The loop runs `i = 0 .. count-1` (`EDI`) with the byte offset `EBP` advancing `2Ch`
   (`00723E61`). Each iteration re-derives the count and calls `00bf6713` if `i` is out of range —
   an STL debug check, not part of the rule.
5. `00723E06` calls `00723AA0` with `ECX = [geom+0Ch] + EBP` and the three pushes
   `(from, &localFarEnd, outHit)` — push order `EBX`(outHit), `EAX`(&localFarEnd), `EDX`(from) at
   `00723DFD`/`00723E02`/`00723E05`.
6. On `AL != 0`, `00723E35`..`00723E58` copy the three floats the callee wrote into `outHit` into
   the **local far endpoint** and store `[geom+0Ch] + EBP` into the result cell.
7. `00723E6E` returns the result cell.

**The selection rule: every element is tested, in index order, against a segment whose far end has
already been pulled back to the closest hit so far; the element that survives is the last one that
accepted, which is therefore the closest.** There is no distance comparison and no bounding-volume
reject anywhere in `00723D60` — the shortening *is* the ordering. A tie (a later element reporting a
hit at exactly the shortened endpoint) is resolved in favour of the later element, because
`00723AA0` is asked again with the already-shortened `to`.

### What the test below it needs, and what that means for the host

`00723AA0` (`RET 0Ch` at `00723B56`/`00723B5E`) does not test the element's bounds first. Its whole
`3Ch`-byte frame is one segment-query record that it hands down by address:

| offset | filled by | contents |
| --- | --- | --- |
| `+00h`, `+0Ch` | `00722B20` at `00723B00` (`ECX = &+18h`, `EDX = &+24h`, pushes `&+00h`, `&+0Ch`) | two derived float3s, contents unread |
| `+18h` | `00723AAB`..`00723AC5` | a copy of `from` |
| `+24h` | `00723ACF`..`00723AFA` | a copy of `to` |
| `+30h` | `007238E0` | the hit point; read back at `00723B31`..`00723B4C` into the caller's `outHit` |

`00723B23` calls `007238E0(element /*ECX*/; &record, 0, 0, triangleCount)` where the count is
`([element+18h] - [element+14h]) >> 1` (`00723B05`..`00723B15`). The two zeros are the root node
index and the first ordinal.

`007238E0` is a bounded-volume walk, not a flat loop:

* `007238EB`..`0072390C`: `0085CAD0([element+20h] + node*12, [element+24h] + node*12, record+18h,
  record+24h)`; a false return exits at `00723913`. `node*12` is built as `n + n*2`, `+ itself`,
  `+ itself` at `007238F8`/`00723900`/`00723906`.
* `0072391E CMP EBP,14h` / `00723922 JG`: more than `14h` ordinals in the node recurses
  (`00723A2E`, not read here); otherwise the leaf loop runs.
* Leaf, `007239C0`..`00723A17`: for each ordinal, `word[ordinals + i*2]` indexes `mesh+2Ch` at
  `idx*6`, the three `int16` there index the vertex array at `v*12`, and `0085D020` is called with
  the three vertices and `record+30h` as the output. On a hit `00723A02`..`00723A11` copy
  `record+30h` into `record+24h` — **the leaf shortens the same `to` the outer scan shortens**, and
  sets the hit byte.

So the closest triangle wins inside an element and the closest element wins across elements. The
`+20h`/`+24h` AABB arrays are an accelerator built at load time — the parser zeroes both cells
(`007273D3`, `007273D7`) and frees them as heap pointers — so **a host that brute-forces every
ordinal of every element returns the same element**, provided that tree is conservative. The
builder (`00725380`, called at `00727A76`) was **not read**, so "conservative" is assumed, not
proven. The host therefore needs the mesh's triangles and vertices, not only per-element bounds;
per-element bounds alone would pick the wrong element whenever two elements' boxes overlap along
the ray, which is the normal case for `fizika` segments of one hull.

### A refutation

`docs/HIT_HULL_SEGMENT.md` §4 and `include/bsp/hit_hull_segment.hpp`'s
`kGeomElementOffRejectScalar` call `element+24h` "a scalar `0085BF90` turns into the squared-radius
reject", and leave `+20h` unread. **Both cells are pointers.** `00723B70`'s head is
`00723B7E MOV ECX,[EBP+24h]` / `00723B81 MOV EDX,[EBP+20h]` / `00723B84 PUSH ECX` /
`00723B8D CALL 0085BF90` with `ECX` = the sphere centre, so `0085BF90(centre, element+20h,
element+24h)` returns a squared distance to the box those two *address*; and `007238E0` indexes
both by `node*12`. They are the min-corner and max-corner arrays of the per-element tree.

## 3. How a shape enters the segment query

`docs/HIT_NARROWPHASE.md` already records that `0098AC20` iterates `entity+F8h` shape pointers in
the array at `entity+D0h` and calls `shape->vtable[0](from, to, record)` through
`0098AC70 MOV ECX,[EBP]` / `MOV EDX,[ECX]` / `MOV EDX,[EDX]` / `0098AC82 CALL EDX`. §1 shows
`00712440` is what fills that array. The two halves meet exactly:

* the query's "entity" is the collision node — the `1ACh` unit-part instance `007135C0` builds;
* the record's `+0h` is `entity+4Ch` (`0098AC88 MOV EAX,[EDI+4Ch]`, then `00470370`), and
  `007136E5`/`007136EB` load `instance+164h` into `instance+4Ch`. So `record+0h` is the **unit**
  that `0087BE55` pushed, which is why `00826F10` can dispatch on it.

**Registration is per unit-part instance, not per model node and not per element.** One instance
carries up to `0Ah` shapes and each shape carries one `{GeomMesh, transform}` pair. A unit whose
hull mesh has seven elements (`Farragut_1934.MMOD`'s first chunk) still presents **one** shape for
that mesh; the seven elements are inside it, and `00723D60` chooses among them. Nothing per-element
is ever offered to the broadphase.

## 4. Do `element+8h` and `0092D1F0`'s `0..19` share a space?

**Yes — the engine has no remap anywhere, and the data agrees. This is a verdict from the consumer's
arithmetic plus a file census, not from a native conversion step, because there is none.**

The consumer, `0092D1F0` (`__thiscall(parts /*ECX*/; uint index, const float dir[3], float damage)`,
`RET 0Ch`), uses **one** index for both arrays:

```
0092d20a: MOV EDI,dword ptr [ESP + 0x3c]        ; index = record+34h, via 00826F10
0092d210: CMP byte ptr [EDI + EBX*0x1 + 0x34c],0x0
0092d218: JL  0x0092d2d8                        ; a negative gate byte bails
0092d243: MOVSS XMM0,dword ptr [EAX + EDI*0x4]  ; parts+310h[index], the health
```

The producer of `+34Ch`, `00937C90`, indexes it by the **name suffix**:

```
00937ff5: PUSH 0x14 / 00937ff9: CALL 0x004a8f10   ; parts+310h resized to 20 floats
00938054: CALL 0x004f9a80                          ; sprintf(buf, "fizika_%02d", i)
        FUN_00b6f9a0(nodes, &name)                 ; the model-node lookup
        *(parts + 0x34c + i) = 0xFF                ; i is the %02d suffix
        if (nodes is empty)  keep 0xFF
        else                 *(parts + 0x34c + i) = (char)groupSlot++
```

So `+34Ch` is indexed by `NN` and *holds* a compacted group slot; the native itself distinguishes
the two numbers and the damage path uses `NN`. Therefore `record+34h`, i.e. `element+8h`, must be
`NN` for `0092D1F0` to hit the right segment, and no code converts it on the way.

`element+8h` is the bare `u32` the `GeomMesh` parser reads after the element name (`007273AC`,
stored at `007273DF`) — authored data, not a load-time counter. The file census below shows the
authored numbering matches the `fizika_NN` node names one for one in this installation:

| file (this installation) | mtime | `fizika_NN` node names present | decoded element indices (`docs/GEOM_MESH_RESOURCE.md`) |
| --- | --- | --- | --- |
| `models/ships/us/Farragut_1934.MMOD` | 2024-07-13 11:24:44 | `00..03` | `0,1,2` + `3` over two chunks |
| `models/ships/japan/yamato.mmod` | 2024-07-13 11:23:20 | `00..10` | `1,2,5,6,0,4,3` + `7,8,9,10` |
| `models/ships/us/Colorado.MMOD` | 2024-07-13 11:24:44 | `00..01` | `0,1` |
| `models/ships/us/North_Carolina.mmod` | 2024-07-13 11:24:54 | `00..01` | `0,1` |
| `models/ships/us/Essex.mmod` | 2024-07-13 11:24:44 | `00..01` | `0` in the one decoded chunk |
| `models/ships/us/farragut.MMOD` | 2024-07-13 11:25:18 | `00..02` | not decoded |
| `models/ships/japan/super_yamato.mmod` | 2024-07-13 11:23:12 | `00..10` | not decoded |

Two things this settles and one it does not:

* **Settled: the count and the range agree exactly** in every file where both sides are known, and
  `yamato.mmod`'s `1,2,5,6,0,4,3` proves `element+8h` is **not** a sequential counter assigned in
  element order — the elements are stored out of index order inside one chunk.
* **Settled: every value is inside `0..19`**, so `0092D1F0`'s `0x14`-slot arrays can hold them, and
  `00937C90`'s `fizika_%02d` loop over `0..19` covers them.
* **Not settled: the gap case.** Nothing in this installation authors a `fizika_NN` set with a hole,
  so the data cannot separate "the suffix" from "a per-kind counter assigned in node order". They
  coincide for every ship here. Since the native offers no remapping step, a model with a hole would
  damage the wrong segment under the counter hypothesis — which is an argument, not a proof.

## 5. The host prescription

The reconstruction's gap is one host object. `src/game_hosts_gunnery.cpp`'s `SegmentBinding`
currently answers `shape_count` with `1` and implements `shape_trace_segment` as an oriented-box
slab test that ends in `record.shape_kind = 0x0A; record.hull_segment = kDirectHitHullSegment`
(`src/game_hosts_gunnery.cpp:1495` and `:1533..1582`). That is a faithful model of the three
hard-coded shapes `docs/HIT_NARROWPHASE.md` tabulates and of nothing else, which is why R1 at
`00826F62` skips `00826F6B..0082757B` on every shot.

What has to change, and nothing else does:

1. **Per unit, hold one `UnitPartShape` per `{GeomMesh, transform}` pair.** Native
   `00712440`; the pair vector is `[instance+160h]+40h..+44h`. Each shape needs the mesh's
   `GeomMeshResourcePayload` (`include/bsp/geom_mesh_resource.hpp`: `elements` with `kind`,
   `node_index` and `triangle_ordinals`, plus `triangles` and `vertices`) and the node's world and
   inverse-world matrices (native `node+50h` and `node+90h`, pushed at `0072452C`/`00724525`).
   A host that models only the hull may hold exactly one such shape per unit.
2. **`shape_count(entity)` returns that shape count**, not `1` — native `entity+F8h`. Keeping `1` is
   correct only while the host holds one shape.
3. **`shape_trace_segment(entity, slot, from, to, record)` runs `00724510`'s body**: transform
   `from` and `to` by the shape's inverse-world matrix (native `00723E90` at `00723E99`/`00723EBC`
   through `004142E0`), call `trace_closest_element_00723d60` (this packet) with a tester that runs
   the element's triangle ordinals against the segment, transform the returned hit point back by the
   world matrix, and then call the existing `apply_segment_hit_00723e90`
   (`include/bsp/hit_hull_segment.hpp`) with `GeomElementHit{ element.kind, element.node_index }`.
   **On a miss it must write nothing** — `00723F1B`..`00723F24` leaves the record untouched, and
   `00470470` has already set `+30h`/`+34h` to `-1`.
4. **Do not filter by kind in the host.** `record+30h` must be whatever `element.kind` says; R4's
   `0Dh` test at `00826FAC` is the consumer's job, and a `body`/`underwater` element hit legitimately
   yields a non-`0Dh` kind with `-1` never appearing again.
5. **Feed the parts object from the same numbering.** `00937C90` sizes `parts+310h` to 20 and writes
   `parts+34Ch + NN`; the host's gate byte array must be indexed by the mesh element's `node_index`
   for R4 to reach `0092D1F0` (`part_index_reach_0092d1f0` in `include/bsp/hit_hull_segment.hpp`
   already models the gate).

With those five, `record+30h` becomes `0Dh` on a `fizika` element and `record+34h` a value in
`0..19`, which is exactly what R1 and R4 need.

## 6. Peer territory, and the smallest thing it must expose

Steps 1..3 of §0 are the model, scene and resource systems. This packet read them and did not lease
or reconstruct them. The boundary is one virtual call:

> **`model->vtable[8h](lodValue)` at `0087BE52`**, where `model = *(*(unit+354h) + 50h)`
> (`0087BDF5`, `0087BDFB`), returns a refcounted "selected set" whose `+40h..+44h` vector holds
> 8-byte `{ GeomMesh*, transform* }` pairs. `00711080` releases it by decrementing `+4h` through
> `[00CE2220]` and dispatching slot 0 at zero.

The smallest contract peer territory has to expose for this host is therefore: **for a unit and a
LOD value, the list of `{ decoded GeomMesh payload, node world transform }` pairs of the selected
set, plus the node's world and inverse-world matrices.** Nothing else in §5 needs the scene graph.
The group filter of §1 needs one more bit per pair — whether the pair's transform appears in any
group record at index `>= 1` — and a host that models only the hull can hard-code that bit false.

`00723170`, the routine that produces each shape's local bounds from the geometry before
`BSP_CollisionShape_SetBounds`, was **not read**; a host that answers `entity_bounds` from its own
hull box (as `src/game_hosts_gunnery.cpp` does) does not need it.

Data files read, all under `I:/SteamLibrary/steamapps/common/Battlestations Pacific/models/ships`,
read-only, byte scan for the literal `fizika_NN` only: the seven `.MMOD`/`.mmod` files tabulated in
§4 plus `PACK3_Colorado_1941.mmod` (2024-07-13 11:24:56), `Yamato1945.mmod` (2024-07-13 11:22:10)
and `colorado1944.MMOD` (2024-07-13 11:25:14), which contain no `fizika_` name at all.

## 7. What is proven, and what is assumed

Proven from the listing and the bytes in this packet:

* `00CFD768` is stored at five sites; three of them are in code Ghidra has not defined, and Ghidra's
  `xrefs` reports only the two that are.
* The shape is a `28h`-byte value type whose `+1Ch`/`+20h`/`+24h` are the node, the transform and
  the geometry, built by `00712440` and copied into an STL list node at `+8h`.
* `00712440` reaches its source pairs through `[instance+160h]+40h..+44h` and publishes only the
  shapes whose transform is absent from every group record at index `>= 1` into `instance+D0h`.
* `007135C0` is called with `(unit, model->vtable[8h](lod))` at `0087BE58` and `0087BE9B`.
* `00723D60` tests every element with a progressively shortened far end and returns the last
  acceptor; it has no distance comparison and no bounds reject.
* `00723AA0`'s frame is one `3Ch`-byte segment record whose `+30h` the leaf writes and whose `+24h`
  the leaf shortens; `007238E0` prunes by the `element+20h`/`+24h` arrays and leaf-tests at most
  `14h` ordinals per node.
* `element+20h` and `element+24h` are pointers, refuting `kGeomElementOffRejectScalar`.
* `0092D1F0` uses one index for both `parts+34Ch` and `parts+310h`, and `00937C90` writes
  `parts+34Ch + NN` where `NN` is the `fizika_%02d` suffix.
* In this installation, the `fizika_NN` node names and the decoded element indices agree in count
  and range on every file where both are known.

Assumed or partial:

* **`00722B20` is unread** — the two derived float3s at record `+00h`/`+0Ch` are named, not decoded.
* **`0085CAD0`, `0085D020`, `0085BF90` bodies are unread.** They are used as a segment/AABB test, a
  segment/triangle test and a point-to-AABB squared distance from their argument shapes and their
  callers' use of the result, not from their code.
* **`007238E0`'s recursion arm (`00723A2E`..`00723A9B`) is unread**, so "the tree is conservative"
  and therefore "brute force is equivalent" is an assumption.
* **`00725380`, the tree builder, is unread.**
* **`model->vtable[8h]` and everything above it is described, not read.** The `1ACh` instance's
  `+160h` object has no name here beyond "selected set".
* **The gap case in §4 is untestable on this installation's data.**
* `instance+D0h`'s `0Ah`-slot cap is not enforced by `00712440`; nothing was found that enforces it.

## 8. Follow-up packets

1. **`00722B20`, `0085CAD0`, `0085D020`.** The three primitives a host must reimplement to trace a
   `fizika` element exactly rather than equivalently. `0085D020` decides the tie behaviour of §2.
2. **`00725380` and the per-element AABB tree.** Whether it is conservative, and its node layout —
   the one thing standing between "brute force is equivalent" and a proof.
3. **`model->vtable[8h]` and the selected set.** Peer territory: the class at `[instance+160h]`, who
   allocates it, and how its `+40h` pairs are built from the loaded `GeomMesh` resources.
4. **`007238E0`'s recursion arm** and how a node index maps to its children.
5. **A gap model.** If any installation authors `fizika_NN` with a hole, §4's last open question
   closes immediately; a `.MMOD` with an authored index above the node-name count would too.
6. **`00723170`** — the per-shape local bounds, if a host ever needs the native broadphase box.
