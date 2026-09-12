# Hit narrowphase: the segment query, the hit record's producer and the flak burst

Addresses: 0098ADD0, 0098AC20, 0085CAD0, 0098AD60, 00722B20, 0098B370, 0098B130, 0098C630,
0098C510, 00470470, 00470350, 00470370, 004704B0, 004704E0, 004706D0, 00470740, 0087FEC0,
0087FF80, 00929B80, 00904470, 0084BAD0, 00922F30, 00922F80, 00922FD0, 009263C0, 00925F20,
0070C210

This packet closes the open items of `docs/PROJECTILE_IMPACT.md`: the routine that packet left as
`0098ADD0`'s "narrowphase, `contract: unread`", the unattributed writers of `projectile+5Ch` and
`+5Dh`, and the flak detonation `0070C210`. It also settles the producer side of the hit record
whose consumer-side layout is in `docs/UNIT_HIT_PATH.md` ("The hit record"), so rule 4 of
`docs/WORKER_VERIFICATION_CHECKLIST.md` is now satisfied for every field except the part-hit array.

## `0098ADD0` is the spatial index's segment query, not a narrowphase

`0098ADD0` (`0098ADD0`-`0098B129`, `RET 14h` at `0098B127`) is

```
__thiscall bool SpatialIndex::QuerySegment(SpatialIndex* this,
                                           const float from[3],   // arg1
                                           const float to[3],     // arg2
                                           void* excludeEntity,   // arg3
                                           HitRecord* record,     // arg4
                                           int kindFilter)        // arg5
```

Five stack arguments from the callee cleanup `RET 0x14`; `this` is in ECX. The prologue proves the
argument order: `MOV EDX,[ESP+0x4c]` at `0098ADD3` is `arg2` (entry ESP + 8) and its three floats
are copied into the local at `[ESP+0x30]`; `MOV ECX,[ESP+0x60]` at `0098AE01` is `arg1`. Every caller
below matches that shape.

It is a **broadphase plus a nearest-hit reduction**, and the only geometry it owns is the AABB
rejection. The actual geometry is one virtual call per collision shape, two levels down.

| order | site | step |
| --- | --- | --- |
| 1 | `0098ADE1`..`0098AE14` | copy `to` into a mutable local (`local_24`), then `00722B20(from, to, &segMin, &segMax)` — the segment's AABB |
| 2 | `0098AE21`, `0098AE2E` | `0098AD60(&cellMin, &segMin)` and `0098AD60(&cellMax, &segMax)` — the two grid cells |
| 3 | `0098AE33`..`0098AE60` | clamp both cell indices to `[0, 95h]` |
| 4 | `0098AE70`..`0098AFD6` | walk the inclusive cell rectangle of the grid at `this+84h`; each cell is a singly linked list, node `+4h` is the next node and node `+8h` is the entity |
| 5 | `0098AFDC`..`0098B11x` | then the same test over the unbucketed array at `this+8h`, count at `this+80h` |
| 6 | — | return the "anything hit" byte (`[ESP+0x13]`) |

`0098AD60(int out[2], const float p[3])` is the cell mapping:
`out[0] = (int)floor(p.x / [00CE3D90]) + 4Bh`, `out[1] = (int)floor(p.z / [00CE3D90]) + 4Bh`. The
grid is `96h * 96h` cells of `dword` heads at `this+84h`, indexed `(x * 96h + z) * 4`; `4Bh` is the
half-width, so the 150x150 grid is centred on the world origin and **y is not indexed at all**.
`00722B20` is the component-wise min/max of two float3s.

Per entity the filter chain is, in the order the listing tests it:

| test | site | meaning |
| --- | --- | --- |
| `kindFilter < 1 \|\| entity[+4Ch] == 0 \|\| entity[+4Ch]->vtable[5Ch](kindFilter)` | `0098AECD` | the caller's class filter; `0` means "no filter" |
| `excludeEntity != entity` | `0098AEE4` | pointer compare, **not a bitmask** (see the correction below) |
| six float compares against `entity+13Ch`..`+150h` | `0098AEEE`..`0098AF60` | entity AABB (`min` at `+13Ch/+140h/+144h`, `max` at `+148h/+14Ch/+150h`) versus the segment AABB |
| `0085CAD0(&entity[13Ch], &entity[148h], from, to)` | `0098AF6C` | the exact segment-versus-AABB test |
| `0098AC20(entity, from, to, exclude, record, kindFilter)` | `0098AF8C` | the per-entity narrowphase |

On a hit the local `to` is replaced by `record+8h..+10h` (`0098AF95`), so the segment shortens to the
nearest hit found so far and the record always ends up holding the **closest** hit. This is the only
ordering rule; the grid is walked in index order and nothing is sorted.

`0085CAD0` (`0085CAD0`-`0085CDAF`, `RET 8h`) is `__fastcall(boxMin, boxMax, from, to)`: a
six-axis separating-axis test between the AABB and the segment treated as a degenerate box (three
face axes, then the three cross products of the box diagonal with the segment direction). `00D7A280`
is the `0.5` it halves every extent with. ECX and EDX at the call site come from the
`LEA ECX,[ESI+0x13c]` and `LEA EDX,[ESI+0x148]` that the preceding AABB compares leave behind
(`0098AF00`, `0098AF06`), which is why the decompiler prints only two arguments.

### Correction to `docs/PROJECTILE_IMPACT.md`

That doc's host table calls `0098B370`'s third argument, and `owner->vtable[B0h]`, a *collision
mask*. `0098ADD0` compares it with `!=` against the entity pointer it pulled out of the cell list
(`0098AEE4`, `CMP` of two pointers) and passes it unchanged to `0098AC20`, which compares it against
child entity pointers the same way (`0098ACE3`). It is an **entity to exclude**, so
`owner->vtable[B0h]` returns the shooter's collision root rather than a bit set. The doc's other
entry, `0098ADD0 is an AABB walk`, is right; "twelve callers ... 0042ee60" is wrong, the caller in
that range is `0042EF90`.

## `0098AC20` — the per-entity narrowphase

`0098AC20` (`0098AC20`-`0098AD5D`, `RET 14h` at `0098AD5B`) has the same signature with the entity
in ECX instead of the index. It owns no geometry either:

| order | site | step |
| --- | --- | --- |
| 1 | `0098AC4B`..`0098ACBE` | for each of the `entity+F8h` shape pointers in the array at `entity+D0h` (`0Ah` slots, `+D0h`..`+F4h`): `shape->vtable[0](from, to, record)`, ECX = the shape |
| 2 | `0098AC88` | on a hit: shorten the local `to` to `record+8h..+10h`, set the byte, and `00470370(record, entity[+4Ch])` |
| 3 | `0098ACC0`..`0098AD4E` | when `kindFilter == 0` only, recurse into the `entity+100h` children at `entity+FCh`, each guarded by `exclude != child` and `0085CAD0` |

The shape call is `MOV ECX,[EBP]; MOV EDX,[ECX]; MOV EDX,[EDX]` at `0098AC70`: slot `0` of the
shape's own vtable, `__thiscall` with three stack arguments and no caller cleanup.

**`record+0h` is `entity[+4Ch]`, not the collision node.** `00470370` is
`HitRecord::SetEntity(record, e)` (`*record = e`), and the same `entity+4Ch` is the object the kind
filter calls `vtable[5Ch]` on. So the node in the grid is a collision proxy and `+4Ch` is its owning
mission entity — which is exactly the `record+0h` that `00926E80` queues and `009239A0` dispatches on.

### The shape interface, `shape->vtable[0]`

Three implementations are in the program, all with the same tail; the rest are `contract: unread`
because no run reaches them and the vtables were not enumerated.

| shape | body | geometry | record writes |
| --- | --- | --- | --- |
| `0087FEC0` | `0087FEC0`-`0087FF7x` | `[this+3D0h]->vtable[3Ch](from, scaledEnd)` | `+8h..+10h` the hit point, `+0h` the entity, `+30h = 0Ah`, `+34h = -1` |
| `0087FF80` | `0087FF80`-`0087FFFx` | `[this+8Ch]->vtable[3Ch](from)`, entity is `this-344h` | the same four |
| `00929B80` | `00929B80`-`00929C7x` | transforms both endpoints by `this+B0h`, `0085CDB0` against a physics-library box from `DYN_physics_00C31F90`, hit point back through `this+70h`; entity is `this-1A4h` | the same four |

`MOV dword ptr [reg+30h], 0Ah` occurs at exactly three addresses in the whole program (`0087FF5D`,
`0087FFDE`, `00929C5E` — byte search `C7 46 30 0A 00 00 00`, and no match for the EAX/ECX/EDI
encodings), so `+30h` is a shape-kind code and `0Ah` is the code these three share. **None of the
three ever writes a hull segment index or a part hit**: all three store `-1` into `+34h`.

## The hit record, producer side

The record is `54h` bytes (`00926E80` copies that much; the blast vector strides `54h`). Its class
lives at `00470350`..`004707A0`:

| routine | body | contract |
| --- | --- | --- |
| `00470470` | `00470470`-`004704Ax` | `HitRecord::Reset`: `+0h`, `+4h`, `+38h`, `+3Ch`, `+40h`, `+44h`, `+48h`, `+4Ch`, `+50h` to `0`, the byte `+2Ch` to `0`, and **`+30h` and `+34h` to `-1`** |
| `00470350` | `00470350`-`0047036x` | `HitRecord::SetShot(shot)`: `+4h = shot`, and when non-null `+14h = shot->vtable[54h]()` |
| `00470370` | `00470370`-`0047043x` | `HitRecord::SetEntity(e)`: `+0h = e` |
| `004704B0` | `004704B0`-`004704Dx` | `HitRecord::~HitRecord`: `free(+3Ch)` — the part-hit array is heap-owned |
| `004704E0` | `004704E0`-`0047050x` | `HitRecord::OwningUnit()`: `[+4h]->vtable[108h]()` then `+ACh`, the object `00470510` and `004705C0` scale by |
| `004706D0` | `004706D0`-`0047073x` | `HitRecord::WorstPart(&index)` |
| `00470740` | `00470740`-`0047079x` | `HitRecord::MaxPartDamage()` |

| offset | writer | meaning |
| --- | --- | --- |
| `+0h` | `00470370` from `0098AC20` (`0098ACB4`), `0098C510` (`0098C570`), the three shapes | the hit entity, `node[+4Ch]` |
| `+4h` | `00470350` from `0084BF00` (`0084C0C3`), `00904470` (`009044F4`), `0084BAD0` (`0084BBB1`) | the shot: `shotInterface[+0CCh]` on the segment path, the caller's shot on the blast path |
| `+8h`..`+10h` | the shape `vtable[0]`, or `0084BAD0` (`piVar3[2..4]`) on the blast path | the hit position; the blast path stores the burst centre |
| `+14h` | `00470350` | `shot->vtable[54h]()` — the hull damage base. **This is the only damage field a direct segment hit gets.** |
| `+18h`..`+20h` | `00904470` | the blast centre (blast path only) |
| `+24h` | `00904470` | the blast **radius**; `004705C0` divides the part distance by it |
| `+28h` | `0084BAD0` | the part damage base, the caller's `*param_3` |
| `+2Ch` | `00904470` and again `0084BAD0` | the ignore-falloff byte |
| `+30h` | the shape `vtable[0]`; `-1` from `00470470` | shape-kind code, `0Ah` for the three known shapes |
| `+34h` | the shape `vtable[0]`; `-1` from `00470470` | hull segment index; **all three known shapes write `-1`** |
| `+38h`, `+3Ch`, `+40h`, `+44h` | `contract: unread` | source record, part-hit array, count, capacity |
| `+48h` | `008778BF`, `00877A24` (consumer) | the damage the handler applied |
| `+4Ch`, `+50h` | `00470470` only | no other writer found |
| `+54h` | `00926F48` in the queued copy | the impact direction `00926E80` appends |

So `docs/UNIT_HIT_PATH.md`'s `+24h` ("part falloff range (divisor)") is the explosion radius and its
`+2Ch` ("ignore falloff") is the explosion's own flag: the falloff `1 - distance/radius` in
`004705C0` is **blast falloff**, and a direct hit never reaches it because a direct hit leaves
`+28h` at `0`.

### Worst-part selection, `004706D0`

```
float WorstPart(HitRecord* r, int* outIndex) {
    *outIndex = -1; float best = 0.0f;
    for (unsigned i = 0; i < r->partHitCount; ++i) {       // +40h
        float d = BSP_HitRecord_PartDamage(r, <ST0 armour>, i);
        if (best < d) { *outIndex = i; best = d; }
    }
    return best;
}
```

The seed is `0.0f` and the compare is strict, so a part whose damage is zero or negative is never
selected and the index stays `-1`; the first entry wins a tie. `00470740` is the same loop without
the index. Callers: `0084F049` and `0047A6A9`, both unread.

### The blast path, `00904470` and `0084BAD0`

`0084BAD0(centre, &radius, &damage, ignoreFalloff, sourceEntity, shot)` is `__fastcall` with the
centre in ECX and `&radius` in EDX. It skips everything when world mode `[00E188A8+1FE4h] == 2`.

| order | site | step |
| --- | --- | --- |
| 1 | `0084BB0x` | `00428800(radius, damage)` — a debug or effect contract |
| 2 | `0084BB3x` | `00904470(vector, centre, radius, ignoreFalloff, shot, sourceEntity)` |
| 3 | `0084BBA0`..`0084BC2x` | per gathered record: `+28h = damage`, `SetShot(shot)`, `+8h..+10h = centre`, `+2Ch = ignoreFalloff`, and `BSP_DeferredEntityEventQueue_Push` **unless `record+0h == sourceEntity`** |
| 4 | `0084BC3x` | destroy every record (`004704B0`) and free the vector |

`00904470` is the gather: `exclude = sourceEntity->vtable[B0h]()`, then
`0098C630(spatialIndex, centre, radius, exclude, vector)` and, per `54h`-byte element, `SetShot`,
`+18h..+20h = centre`, `+24h = radius`, `+2Ch = ignoreFalloff`.

`0098C630` (`RET 10h`) is the **sphere** sibling of `0098ADD0` over the same grid, de-duplicating
nodes with a frame stamp at `node+4h` against `[00E188A8+648h]`; `0098C510` (`RET 14h`) resets a
temporary record, sets `+0h = node[+4Ch]`, calls **`node->vtable[0Ch](centre, radius, vector,
&record)`** and appends through `0098C460` on a hit, then recurses over the node's children. That
`vtable[0Ch]` override is where a unit's part-hit array must be built; it is `contract: unread`.

## `projectile+5Ch` and `+5Dh` are scene-node flags, not projectile fields

Both bytes belong to the mission-entity / scene-node base the projectile derives from at offset `0`
(the same base whose `+44h` is the sibling link and `+48h` the child head, which
`docs/PROJECTILE_IMPACT.md` already attributes). Neither the spawn `0072F830` nor the constructor
`006E7B00` writes them.

| byte | writers | meaning |
| --- | --- | --- |
| `+5Ch` | `00922F30` (set `1`), `00922F80` (clear), `00922FD0` (clear), `009263C0` (clear), `00925F20` at `009261D0`/`00926207` (set/clear inline), `009274DA` (clear) | the node's **active** flag |
| `+5Dh` | `00922FE4`, `009263FD`, `009274CE`, `009272EF` — always `1`, always beside `+5Eh = 1` | the node's **torn-down** flag |

Byte search: every `MOV byte ptr [reg+5Ch], imm8` encoding (`C6 40/41/42/43/45/46/47 5C`) and the
same seven for `+5Dh`. Only `C6 46 5C`, `C6 47 5C`, `C6 41 5C`, `C6 40 5C`, `C6 46 5D` matched, and
the game-range hits are the functions above.

* `00922F30(node, arg)`: when `+5Eh` is clear and `+5Ch` is clear, set `+5Ch = 1`, call
  `vtable[68h]`, recurse over the children — **Enable**.
* `00922F80(node, arg)`: the mirror with `vtable[6Ch]` — **Disable**.
* `00922FD0(node)`: `+5Eh = 1`, `+5Dh = 1`, `+5Ch = 0`, `+6Ch = 1`, recurse, `vtable[84h]` — **Kill**.
* `009263C0(node)`: `+5Dh = 1`, `+5Eh = 1`, `+5Fh = 1`, `+5Ch = 0`, `vtable[80h]` — the deferred
  removal; `+5Eh`/`+5Fh` are the two bytes `BSP_DeferredEntityEventQueue_Drain` requires clear.

So `006E64EF`'s gate `[proj+5Ch] != 0` is "the projectile node is still enabled" — the sweep stops
the moment anything disables or kills the node, which is what `BSP_MissionEntity_Kill` at
`0084BE00` does on impact. `006E6BB0`'s gate `[proj+5Dh] == 0` is "the node has not been torn down",
so a projectile that is being destroyed never re-binds to an owner.

## The flak burst, `0070C210`

`__thiscall(projectile, ...)`, body `0070C210`-`0070C366`, sole caller `0070C370`
`BSP_FlakProjectile_TickAdvance` at `0070C3B5` (the fuse expiry) and `0070C79B` (a proximity hit).
It is **not** a proximity search of its own: it is the detonation, and the damage is one call to the
shared explosion `0084BAD0`.

| order | site | step |
| --- | --- | --- |
| 1 | `0070C217` | `[proj+C8h] == 0` -> `00414DB0` refresh the world pose |
| 2 | `0070C226`..`0070C247` | `damage = 00BD2F10(classDesc[+B4h], classDesc[+B8h])` with `ECX = 1` |
| 3 | `0070C266`..`0070C280` | `0084BAD0(ECX = proj+FCh, EDX = classDesc+70h, &damage, 1, [proj+238h], projectile)` |
| 4 | `0070C285`..`0070C325` | when `classDesc[+2Ch]` is non-null: `BSP_PointEffect_CreateFromPosition(&effect, [00E188A8+19ECh], definition, proj+FCh, 0, 0)`, set `effect+9h = 1`, release the reference |
| 5 | `0070C327`..`0070C350` | when `proj+198h` is non-null: `004842C0(proj+198h, proj+FCh)` `BSP_PointEffect_SetWorldPoint` |
| 6 | `0070C355` | `BSP_MissionEntity_Kill(1)` |

The burst rule, with every value's site:

| quantity | value | site |
| --- | --- | --- |
| centre | `projectile+FCh`, the world translation | `LEA EBX,[ESI+0xfc]` at `0070C274`, ECX at `0070C27E` |
| radius | `classDesc[+70h]` | `MOV EDX,[ESI+0x174]; ADD EDX,0x70` at `0070C266`/`0070C27B` |
| damage | `Random(classDesc[+B4h], classDesc[+B8h])` | `0070C22C`, `0070C23E`, `CALL 00BD2F10` at `0070C247`, stored by `FSTP [ESP+0xc]` at `0070C253` and passed by address |
| ignore falloff | `1` | `PUSH 0x1` at `0070C26E` |
| excluded entity | `projectile+238h`, the owner | `MOV EDI,[ESI+0x238]` at `0070C257`, `PUSH EDI` at `0070C26D` |
| shot | the projectile itself | `PUSH ESI` at `0070C26C` |

`00BD2F10` calls `BSP_RandomThreads_GetState` and `00BD2E60`, so it is the random helper; the two
class-descriptor floats are a range. **A flak burst therefore damages every part inside
`classDesc[+70h]` at full strength** — the ignore-falloff flag makes `004705C0`'s
`1 - distance/radius` collapse to `1.0f` whenever it is still positive — and the owner is the only
entity spared.

## Host table

One row per native call site the reconstruction models.

| site | callee | host method | this / arguments | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `0098AE14` | `00722B20` | `segment_bounds` | from; to, &min, &max | — | always |
| `0098AE21`, `0098AE2E` | `0098AD60` | `cell_of_point` | &out; point | — | twice |
| `0098AE8x` | grid `this+84h` | `cell_head` | index; cellX, cellZ | node | per cell |
| `0098AECD` | `entity[+4Ch]->vtable[5Ch]` | `entity_is_kind` | owner; kindFilter | bool | `kindFilter >= 1` and `+4Ch` non-null |
| `0098AF6C` | `0085CAD0` | `segment_overlaps_box` | boxMin, boxMax; from, to | bool | after the six AABB compares |
| `0098AF8C` | `0098AC20` | `test_entity` | entity; from, to, exclude, record, kind | bool | after the coarse test |
| `0098AC82` | `shape->vtable[0]` | `shape_trace_segment` | shape; from, to, record | bool | per shape slot |
| `0098ACB4` | `00470370` | `record_set_entity` | record; `entity[+4Ch]` | — | on a shape hit |
| `0098ACF0` | `0098AC20` | `test_entity` | child; the same five | bool | `kindFilter == 0` only |
| `0084C0C3` | `00470350` | `record_set_shot` | record; `shotInterface[+0CCh]` | — | `0084BF00` setup |
| `0098C57x` | `node->vtable[0Ch]` | `node_overlap_sphere` | node; centre, radius, vector, record | bool | blast path |
| `009044F4` | `00470350` | `record_set_shot` | record; shot | — | per gathered record |
| `0084BBB1` | `00470350` | `record_set_shot` | record; shot | — | per gathered record |
| `0084BBCx` | `00926E80` | `queue_hit` | record, direction | — | `record+0h != sourceEntity` |
| `0070C221`, `0070C261` | `00414DB0` | `refresh_world_pose` | projectile | — | `[proj+C8h] == 0` |
| `0070C247` | `00BD2F10` | `random_in_range` | `classDesc[+B4h]`, `classDesc[+B8h]` | float | always |
| `0070C280` | `0084BAD0` | `apply_explosion` | centre; &radius, &damage, `1`, owner, projectile | — | always |
| `0070C2EC` | `008685E0` | `spawn_burst_effect` | out; world effect manager, definition, centre, `0`, `0` | — | `classDesc[+2Ch]` non-null |
| `0070C350` | `004842C0` | `move_attached_effect` | `proj+198h`; centre | — | `proj+198h` non-null |
| `0070C35x` | `BSP_MissionEntity_Kill` | `kill_projectile` | `1` | — | always |

## The callers of `0098ADD0`

Fifteen call sites in twelve containers (`python tools/bsp.py ghidra xrefs 0098add0`). Every one
passes the singleton from `0042E630 BSP_SpatialIndex_GetSingleton` in ECX.

| site | container | from / to | exclude | kind | read |
| --- | --- | --- | --- | --- | --- |
| `0098B3EB` | `0098B370` `BSP_SpatialIndex_SweepSegments` | the single from/to pair | caller's | `0` | yes |
| `0098B41F` | the same | each `18h`-stride pair until one hits | caller's | `0` | yes |
| `0042F044` | `0042EF90` | `(pos + delta) * 0.5` and `this+A4h` | `0` | `0` | yes: on a hit it snaps `this+A4h`..`+ACh` to `record+8h`..`+10h`, a move-and-slide |
| `0072CE91` | `0072CDD0` | the caller's point and a local end built from `target[+FCh]` plus `[00D7A370]` | `0` | `44h` | yes: a line of sight; success also requires `record[+0h][+54h] == shooter[+54h]`, otherwise it retries through `0098B130` |
| `00547857` | `00547480` | EDI and a stack float3 | `[this+3F0h]->vtable[20h]()` | `0` | call shape only; the container is `contract: unread` |
| `0043BA7E` | `0043B9C0` | — | — | — | no |
| `00452D49` | `00452BD0` | — | — | — | no |
| `009043D4`, `009043F6` | `009043A0` | — | — | — | no |
| `00904434`, `0090445B` | `00904400` | — | — | — | no |
| `00894B66` | `008949D0` | — | — | — | no |
| `0095787E` | `00957740` | — | — | — | no |
| `00957DA0` | `00957BD0` | — | — | — | no |
| `004302D4` | no Ghidra function | — | — | — | no |

## Coverage

| routine | coverage |
| --- | --- |
| `0098ADD0`, `0098AC20`, `0085CAD0`, `0098AD60`, `00722B20` | complete |
| `00470470`, `00470350`, `00470370`, `004704B0`, `004704E0`, `004706D0`, `00470740` | complete |
| `00922F30`, `00922F80`, `00922FD0`, `009263C0` | complete |
| `00925F20` | partial: only the `+5Ch` enable/disable block `009261B5`-`0092620F` |
| `00904470` | complete |
| `0084BAD0` | complete for the record fill and the queue gate; `00428800` is a contract |
| `0098C630`, `0098C510` | complete; `node->vtable[0Ch]` and `0098C460` are contracts |
| `0087FEC0`, `0087FF80`, `00929B80` | complete; their `vtable[3Ch]` geometry is a contract |
| `0070C210` | complete; the five effect and kill callees are contracts |
| `0098B130` | `contract: unread`, named only as `0072CDD0`'s fallback |

`contract: unread`, in the order a later packet should take them:

1. the shape `vtable[0]` implementation that writes a real hull segment index into `record+34h`, and
   the `node->vtable[0Ch]` override that allocates `record+3Ch` and counts `record+40h`. The three
   shapes found here never write either, so the producer of the part-hit array of
   `docs/UNIT_HIT_PATH.md` is still unattributed and that one table row stays consumer-derived;
2. `record+38h`, the source record `007BBCF0` copies into `unit+800h`;
3. `0098B130`, the widened query `0072CDD0` falls back to;
4. the nine unread callers above;
5. `0084F020` and `0047A6A0`, the two `WorstPart` callers.
