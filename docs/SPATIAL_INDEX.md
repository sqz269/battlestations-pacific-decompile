# The spatial index singleton, the cell key and the per-step re-bucket

Addresses: 0042E630, 0042D450, 0098BDB0, 0098BC70, 0098A310, 0098A3D0, 0098A750, 0098BA10,
0098A500, 0098A4C0, 0098A2C0, 0098B920, 0098A920, 00722C20, 0098B7D0, 00F8A0D8

Packet `cc2_spatial_index` (`spatial_index_rebucket` of docs/FIXED_STEP_FANOUT.md). The fan-out's
rows 3 and 4 (`0042E630` then `0098BDB0`, `00875E33`/`00875E3A`) are the executable's unimplemented
host `FixedStepFanout::refresh_moved_spatial_nodes`. This doc is that method's body plus the
attach/detach pair that produces the list it walks.

Everything below was read from the Ghidra listing of the named bodies; the pseudocode was not used
for any claim. Names are hypotheses, not recovered symbols. The grid mapping itself
(`0098AD60`, `0098ADD0`) belongs to docs/HIT_NARROWPHASE.md and is reused, not re-derived.

## Correction to docs/FIXED_STEP_FANOUT.md

| was | is | evidence |
| --- | --- | --- |
| "constructor `0042D450` (a `memset` and nothing else)" | the constructor also stores a vtable at `+0h`, zeroes `+4h`, `+80h` and `+16014h`, and the `memset` covers only the grid at `+84h` (`0x15F90` bytes) | `0042D461`, `0042D467`, `0042D476`, `0042D480`, `0042D453` |
| "walks the list at `[this+16014h]` ... for every node whose `+8h` byte is clear" (a *moved* list) | `+16014h` is the head of every **grid-registered root node**, not a moved list; nothing marks a node moved. `node+8h` is a static flag written once at attach time from `0098BA10`'s fourth argument, and a set flag means *never refresh this node* | `0098BA22` (`MOV byte ptr [ESI+8],AL` from arg4), `0098BDC0` |
| "computes a four-byte key from a transformed point" | the key packs **four cell indices** (the AABB's min and max cells), not one point; `0098A750` rebuilds the world AABB first and `0098AD60` is called twice, on `node+13Ch` and `node+148h` | `0098BCE3`..`0098BD1A` |
| the "spatial index" reading of the key is provisional | settled: `0098A310` writes the same packing into `node+3Ch` immediately after linking the node into exactly those cells, and `0098A3D0` unpacks `node+3Ch` to find the cells to unlink | `0098A3AE`..`0098A3C4`, `0098A3D3`..`0098A3E9` |

## The singleton

`0042E630` `BSP_SpatialIndex_GetSingleton`, `__cdecl void* (void)`, `RET`, body
`0042E630..0042E6DD`. Double-checked lazy singleton over the global `00F8A0D8` with an SEH frame
(`0042E638` pushes `00C5EB28`): test `[00F8A0D8]`, take the process lock object from
`00415350()` at `+10h` and enter it through `[00CE2218]` (`+18h` is its recursion count), re-test,
`operator new(0x16018)` at `00BF681B`, construct with `0042D450`, publish to `00F8A0D8`, register
the instance for shutdown through `00BD0C30` (`BSP_SingletonLifetime_Register`), leave through
`[00CE2210]`. A failed allocation publishes null (`0042E6A0`), which every caller then dereferences.
26 call sites image-wide.

`0042D450`, `__thiscall void* (this)`, `RET`, body `0042D450..0042D48D`. Stores the vtable
`00CE3CEC` at `+0h`, `0` at `+4h`, `memset(this+84h, 0, 0x15F90)`, `[this+80h] = 0`,
`[this+16014h] = 0`, returns `this`. The loose array at `+8h` is **not** cleared; its count at
`+80h` is, which is what makes the stale pointers unreachable.

### Layout, 0x16018 bytes

| offset | size | field | evidence |
| --- | --- | --- | --- |
| `+0h` | 4 | vtable `00CE3CEC` | `0042D461` |
| `+4h` | 4 | zero, never read by this packet's routines | `0042D467` |
| `+8h` | 0x78 | loose array, 30 node pointers | `0098BB32` writes `[index + count*4 + 8]`; `+80h` bounds the array |
| `+80h` | 4 | loose count | `0042D476`, `0098BB36` (`+1`), `0098A4EA` (`-1`) |
| `+84h` | 0x15F90 | `96h * 96h` cell heads, `dword` each, index `(x * 96h + z) * 4` | `0042D453`, `0098A356`, `0098A44A`; the stride between rows is `0x258` at `0098A49A` |
| `+16014h` | 4 | head of the doubly linked list of grid-registered root nodes | `0042D480`, `0098BB16`, `0098BDB1` |

`0x84 + 0x15F90 = 0x16014`, so the list head is the last dword of the allocation and there is no
padding. The loose array has no bounds check anywhere: a 31st oversized node would overwrite the
count at `+80h` and then walk into the grid.

## The node

The node is the collision proxy of docs/HIT_NARROWPHASE.md (`+4Ch` is its owning pose object,
`+D0h`/`+F8h` its shapes, `+FCh`/`+100h` its children, `+13Ch`/`+148h` its world AABB). The fields
this packet establishes:

| offset | field | evidence |
| --- | --- | --- |
| `+8h` | static flag; set means the per-step refresh skips this node | `0098BA22` writes arg4, `0098BDC0` tests it |
| `+0Ch..+3Bh` | four 12-byte cell links `{prev, next, owner}`, one per occupied cell | `0098A34C` (`node + 12*slot + 12`), `0098A46E`; four slots is `(3Ch-0Ch)/0Ch` and is exactly the 2x2 cap enforced at `0098BAD2` |
| `+3Ch` | cell key (see below) | `0098A3C4` writes, `0098BD1A` compares, `0098A3D3` unpacks |
| `+40h` | number of cells the node occupies; `0` means not in the grid | `0098A3AB` writes the slot count, `0098A4AA`/`0098A4B6` write 0 |
| `+44h` / `+48h` | prev / next in the index's root list | `0098BAFD`, `0098BB00`, `0098A53D`..`0098A554` |
| `+50h` | world matrix, 0x40 bytes, rows at `+50h/+60h/+70h/+80h` | `0098BC94` copies `pose+CCh`; `0098A787` reads rows 0..2 |
| `+90h` | inverse world matrix, 0x40 bytes | `0098BCD1` copies `pose+110h` |
| `+104h` | child array capacity | `0098B929` |
| `+108h` | parent node, or 0 for a root | `0098BAA9` (0 on the grid path), `0098B9A9` (the parent) |
| `+10Ch` / `+118h` | local AABB min / max | `0098A929`..`0098A959` |
| `+124h` | local centre | `0098A80F` transforms it |
| `+130h/+134h/+138h` | local half extents | `0098A76D`, `0098A7B5`, `0098A7DF` |
| `+154h` | frame stamp for the world AABB | `0098A761` against `[00E188A8]+648h`, the game frame counter of docs/GAME_ON_MOVE_MAP.md |
| `+158h` | attached flag | `0098BB1D` sets, `0098A558` clears, `0098A505` gates the whole detach |
| `+15Ch` | the node's slot in its parent's child array | `0098B999`, `0098A2D3` |

The third dword of a cell link (`node+14h`, `+20h`, `+2Ch`, `+38h`) is the owner back-pointer that
`0098ADD0` reads as `cell_node+8h`. **Producer unread**: neither `0098A310` nor `0098A3D0` writes
it, so it is filled in when the node object is constructed, outside this packet. The query treats
the value as a node (`+4Ch`, `+13Ch`, `+D0h`), which is the shape the link's owner must have.

## The cell key

`0098A750` rebuilds the world AABB, `0098AD60` maps its two corners to cells, and the four cell
indices are packed little-endian into one dword:

```
key = minCell.x | (minCell.z << 8) | (maxCell.x << 16) | (maxCell.z << 24)
```

Computed at `0098BD01`..`0098BD1A` (`SHL 8` / `ADD` three times, most significant term first) and
again, from the same operands, at `0098A3AE`..`0098A3C4`. Unpacked at `0098A3D3`..`0098A3E9`
(`MOVZX EBX,AH`, `SHR 10h`, `SHR 18h`, `AND 0FFh`).

Two consequences, both visible in the listing and neither guarded:

* The cell indices are **not clamped** here. `0098ADD0` clamps to `[0, 95h]` for its own walk
  (docs/HIT_NARROWPHASE.md step 3), but the re-bucket path does not, so a node whose AABB leaves
  the 150x150 grid packs an out-of-range or negative byte and `0098A310` indexes outside the grid.
  The world is expected to stay inside `+-75 * [00CE3D90]`.
* Only the low byte of each index survives, so the key is a faithful identity for in-range cells
  only.

The grid mapping itself is `cell = floor(p.x / [00CE3D90]) + 4Bh` on x and z, y not indexed
(docs/HIT_NARROWPHASE.md).

## The world AABB, `0098A750`

`__thiscall void (node)`, `RET`, body `0098A750..0098A8D7`. Guarded by the frame stamp: if
`[node+154h] == [[00E188A8]+648h]` it returns immediately (`0098A767`), so at most one rebuild per
node per frame however many callers ask.

| # | range | step |
| --- | --- | --- |
| 1 | `0098A77E` | stamp `node+154h` with the current frame |
| 2 | `0098A78E`..`0098A804` | zero a scratch vec3, then three `00722C20(scratch, row.x, row.y, row.z, extent)` calls with matrix rows `+50h`, `+60h`, `+70h` and extents `+130h`, `+134h`, `+138h` |
| 3 | `0098A80F` | `BSP_Vector3f_TransformAffinePoint(node+124h, &centre, node+50h)` |
| 4 | `0098A81A`..`0098A862` | write the centre to both `node+13Ch` and `node+148h` |
| 5 | `0098A868`..`0098A8CD` | `min -= scratch`, `max += scratch`, one axis at a time on the x87 stack |

`00722C20`, `__thiscall void (acc, float x, float y, float z, float s)`, `RET 10h`, body
`00722C20..00722C96`: `acc[i] += |v[i]| * s`, the absolute value taken by `AND 7FFFFFFFh` on the
float bits (`00722C36`, `00722C5B`, `00722C7D`). Summed over the three rows this is the standard
oriented-box to axis-aligned-box extent, so the world AABB is `centre*M +- sum_i |M_i| * e_i`.

The local box that feeds it is produced by `0098A920`, `__thiscall void (node, const float* min,
const float* max)`: it stores the two corners at `+10Ch`/`+118h` and derives
`extent = 0.5*max - 0.5*min` into `+130h` (the `0.5` is the double at `00D7A280`), with the centre
in the unread tail. `0098B7D0` is its caller for shape-built nodes: it unions the AABBs of the
shapes at `node+D0h` (`count` at `+F8h`, each shape's box at `shape+4h`).

## Register, `0098A310`

`__fastcall void (index /*ECX*/, node /*EDX*/, const int cellMin[2], const int cellMax[2])`,
`RET 8`, body `0098A310..0098A3CD`. Argument order from the site at `0098BD2D`..`0098BD40`
(`&cellMax` pushed first, `&cellMin` second) and the `RET 8`.

| # | range | step |
| --- | --- | --- |
| 1 | `0098A326` | `slot = 0` |
| 2 | `0098A328` | if `cellMin.x > cellMax.x`, skip the whole loop (`slot` stays 0) |
| 3 | `0098A340`..`0098A3A8` | for `x = cellMin.x .. cellMax.x`, for `z = cellMin.z .. cellMax.z`: `link = node + 0Ch + 0Ch*slot`; `link->next = heads[x*96h + z]`; `link->prev = 0`; if the old head exists, `oldHead->prev = link`; `heads[x*96h + z] = link`; `++slot` |
| 4 | `0098A3AB` | `node+40h = slot` |
| 5 | `0098A3AE` | `node+3Ch = key(cellMin, cellMax)` |

The slot order is x-major, z-minor, and `0098A3D0` reproduces it exactly; that shared order is what
lets the unregister find each link from the key alone. Nothing bounds `slot` to the four link
slots: a rectangle larger than 2x2 writes over `+3Ch` and `+40h` and past them. The only thing that
keeps that from happening is the caller's 2x2 test at `0098BAD2`, and `0098BC70` does **not** repeat
it (see the open questions).

## Unregister, `0098A3D0`

`__fastcall void (index /*ECX*/, node /*EDX*/)`, `RET`, body `0098A3D0..0098A4BE`.

| # | range | step |
| --- | --- | --- |
| 1 | `0098A3D3`..`0098A3E9` | unpack `node+3Ch` into the four cell indices |
| 2 | `0098A406`..`0098A430` | for each of the `node+40h` links: `link->prev->next = link->next`, `link->next->prev = link->prev`, each guarded by a null test |
| 3 | `0098A438` | if `minX > maxX`, store `0` into `node+40h` and return |
| 4 | `0098A460`..`0098A4A7` | walk the same rectangle in the same order; when `heads[x*96h+z] == link` replace the head with `link->next` |
| 5 | `0098A4AA` | `node+40h = 0` |

Step 2 fixes the neighbours, step 4 fixes the heads; a link that was a head has `prev == 0`, so only
step 4 can unhook it. `node+3Ch` is left holding the old key, which is harmless because `node+40h`
is now 0 and every reader gates on that.

## Refresh, `0098BDB0` and `0098BC70`

`0098BDB0` `BSP_SpatialIndex_RefreshMovedNodes`, `__thiscall void (index, float)`, `RET 4`, body
`0098BDB0..0098BDD5`. The float is never read. Walk: `node = [index+16014h]`; while `node`, if
`node+8h == 0` call `0098BC70(node)`; `node = node+48h`. The next pointer is taken from the node
**before** the callee can change it, and `0098BC70` never relinks the root list, so the walk is
stable.

`0098BC70`, `__thiscall void (node)`, `RET`, body `0098BC70..0098BD73`, recursive.

| # | range | step | gate |
| --- | --- | --- | --- |
| 1 | `0098BC77`..`0098BC85` | `pose = node+4Ch`; `BSP_EntityPose_RefreshWorld(pose)` | `pose+C8h == 0` |
| 2 | `0098BC8A`..`0098BC94` | `BSP_Matrix_Copy4x4X87(node+50h, pose+CCh)` | always |
| 3 | `0098BC99`..`0098BCBF` | reload `pose`, refresh it again, `BSP_Matrix_BuildOrthogonalScaledAffineInverse(pose+110h, pose+CCh)`, set `pose+10Ch = 1` | `pose+10Ch == 0` |
| 4 | `0098BCC4`..`0098BCD1` | `BSP_Matrix_Copy4x4X87(node+90h, pose+110h)` | always |
| 5 | `0098BCD8` | `0098A750(node)`, the world AABB | always |
| 6 | `0098BCE3`..`0098BCFC` | `0098AD60(&cellMin, node+13Ch)` and `0098AD60(&cellMax, node+148h)` | `node+40h != 0` |
| 7 | `0098BD01`..`0098BD1D` | pack the key and compare with `node+3Ch` | `node+40h != 0` |
| 8 | `0098BD1F`..`0098BD40` | `0098A3D0(GetSingleton(), node)` then `0098A310(GetSingleton(), node, &cellMin, &cellMax)` | key changed |
| 9 | `0098BD45`..`0098BD6C` | for each of the `node+100h` children at `node+FCh`, recurse into `0098BC70` | always |

Steps 1-5 and 9 run for every node reached; only a node with `node+40h != 0`, that is a node the
grid holds, is ever re-bucketed. Children are attached through the parent (`+108h` non-zero, `+40h`
still 0), so the recursion refreshes their matrices and boxes and never touches the grid for them.
The static flag at `+8h` is tested only by `0098BDB0`, so a static **root** is skipped entirely
while a static **child** of a moving root is still refreshed.

## Attach, `0098BA10`

`__thiscall void (index, node, parent, const float* matrix, char staticFlag)`, `RET 10h`, body
`0098BA10..0098BB4B`.

| # | range | step | gate |
| --- | --- | --- | --- |
| 1 | `0098BA22` | `node+8h = staticFlag` | always |
| 2 | `0098BA25`..`0098BA82` | the same four matrix steps as the refresh (`00414DB0`, `004134F0`, `00B63D50`, `004134F0`) | as above |
| 3 | `0098BA91`..`0098BAA4` | `0098B920(parent, node)`, then `node+158h = 1` and return | `parent != 0` |
| 4 | `0098BAA9`..`0098BAAF` | `node+108h = 0`; `0098A750(node)` | `parent == 0` |
| 5 | `0098BAB4`..`0098BACD` | `0098AD60` on `node+13Ch` and on `node+148h` | `parent == 0` |
| 6 | `0098BAD2`..`0098BAE8` | if `maxX - minX > 1` or `maxZ - minZ > 1`, go to step 8 | `parent == 0` |
| 7 | `0098BAEA`..`0098BB1D` | `0098A310(index, node, &cellMin, &cellMax)`; `node+44h = 0`; `node+48h = [index+16014h]`; if that head exists, `head+44h = node`; `[index+16014h] = node`; `node+158h = 1` | fits 2x2 |
| 8 | `0098BB2C`..`0098BB3D` | `index->loose[index->looseCount++] = node`; `node+158h = 1` | too big |

The `matrix` argument (third push) is **not read** by the callee; both live sites pass either null
or the owning pose's `+CCh`. Only the fits-2x2 branch links the node into the list at `+16014h`, so
the per-step refresh never sees an oversized node or a child: **the "moved list" is the list of
grid-registered roots, and membership is decided here, at attach time**.

Call sites:

| site | index | node | parent | matrix | static | meaning |
| --- | --- | --- | --- | --- | --- | --- |
| `006D3E28` | `0042E630()` | `[this+828h]` | 0 | 0 | **1** | a static prop: attached once, never refreshed |
| `00710B6D` | `0042E630()` | `this` (a unit part, `BSP_UnitPartInstance_Construct` reaches it) | `0070F7D0(this)` | `pose+CCh` | `1` when `pose->vtable[5Ch]` answers to any of `1Ch`, `1Bh`, `36h`, `44h`, else 0 | a part either becomes a child of its parent part or a root of its own |
| `0092B30C` | `0042E630()` | `entity+1C4h` | 0 | 0 | **0** | the moving entity case, refreshed every step |

## Detach, `0098A500`

`__thiscall void (index, node)`, `RET 4`, body `0098A500..0098A585`.

| # | range | step |
| --- | --- | --- |
| 1 | `0098A505` | `node+158h == 0` -> return, the node was never attached |
| 2 | `0098A511`..`0098A575` | `node+108h != 0` -> `0098A2C0(parent, node)`, the child case |
| 3 | `0098A51B`..`0098A563` | `node+40h == 0` -> `0098A4C0(index, node)`, the loose case |
| 4 | `0098A525`..`0098A554` | otherwise `0098A3D0(index, node)`, then unlink from `+16014h`: head fix, `prev+48h = next`, `next+44h = prev` |
| 5 | `0098A558`, `0098A57A`, `0098A56C` | every path clears `node+158h` |

`0098A4C0`, `__fastcall void (index /*ECX*/, node /*EDX*/)`, `RET`: linear search of the loose
array, swap with the last entry, `--count` (`0098A4E2`..`0098A4EA`). `0098A2C0`,
`__thiscall void (parent, child)`, `RET 4`: `--parent+100h`, move the last child into the slot
`child+15Ch`, fix that child's own `+15Ch`, `child+108h = 0`. Both are swap-removes, so neither
preserves order.

Two of the five detach sites can pass a null node (`00880CDF`, `00929D82` both reach `XOR EAX,EAX`
before the push) and `0098A500` dereferences it at `0098A505` with no null test. Whatever keeps that
from firing is in the callers, which this packet did not read.

The child path, `0098B920`, `__thiscall void (parent, child)`, `RET 4`: doubling append to the
parent's child array at `+FCh` (`count` `+100h`, `capacity` `+104h`, `operator new` / `_memcpy` /
`_free`), `child+15Ch = slot`, `child+108h = parent`, then `0098B530(parent, child)`, which
expands the parent's local AABB at `+10Ch` by the child's eight local corners pushed through the
child's world matrix and the parent's inverse (`0098B540`..`0098B599`). `0098B530`'s tail is
unread; the bound merge is the part this packet needed.

## Host table, one row per native call site

`this`/args as the listing sets them; `ret` is the callee's own cleanup.

| site | containing | callee | host method | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- | --- |
| `00875E33` | `00875BB0` | `0042E630` | `spatial_index_singleton` | none | `RET` | always |
| `00875E3A` | `00875BB0` | `0098BDB0` | `refresh_moved_spatial_nodes` | `EAX` of row above; `0.05f` unused | `RET 4` | always |
| `0098BDC8` | `0098BDB0` | `0098BC70` | `refresh_node` | `ECX = node` | `RET` | `node+8h == 0` |
| `0098BC85` | `0098BC70` | `00414DB0` | `pose_refresh_world` | `ECX = node+4Ch` | `RET` | `pose+C8h == 0` |
| `0098BC94` | `0098BC70` | `004134F0` | `node_copy_world_matrix` | `ECX = node+50h`; `pose+CCh` | `RET 4` | always |
| `0098BCA7` | `0098BC70` | `00414DB0` | `pose_refresh_world` | `ECX = node+4Ch` | `RET` | `pose+10Ch == 0` |
| `0098BCBF` | `0098BC70` | `00B63D50` | `pose_build_inverse` | `ECX = pose+110h`, `EDX = pose+CCh` | `RET` | `pose+10Ch == 0` |
| `0098BCD1` | `0098BC70` | `004134F0` | `node_copy_inverse_matrix` | `ECX = node+90h`; `pose+110h` | `RET 4` | always |
| `0098BCD8` | `0098BC70` | `0098A750` | `node_rebuild_world_bounds` | `ECX = node` | `RET` | always |
| `0098BCED` | `0098BC70` | `0098AD60` | `cell_of_point` | `ECX = &cellMin`, `EDX = node+13Ch` | `RET` | `node+40h != 0` |
| `0098BCFC` | `0098BC70` | `0098AD60` | `cell_of_point` | `ECX = &cellMax`, `EDX = node+148h` | `RET` | `node+40h != 0` |
| `0098BD1F` | `0098BC70` | `0042E630` | `spatial_index_singleton` | none | `RET` | key changed |
| `0098BD28` | `0098BC70` | `0098A3D0` | `unregister_node` | `ECX = index`, `EDX = node` | `RET` | key changed |
| `0098BD37` | `0098BC70` | `0042E630` | `spatial_index_singleton` | none | `RET` | key changed |
| `0098BD40` | `0098BC70` | `0098A310` | `register_node` | `ECX = index`, `EDX = node`; `&cellMin`, `&cellMax` | `RET 8` | key changed |
| `0098BD62` | `0098BC70` | `0098BC70` | `refresh_node` | `ECX = child` | `RET` | per child |
| `0098A7B0` | `0098A750` | `00722C20` | `accumulate_abs_scaled` | `ECX = &acc`; row `+50h`, `ext.x` | `RET 10h` | stamp differs |
| `0098A7DA` | `0098A750` | `00722C20` | `accumulate_abs_scaled` | `ECX = &acc`; row `+60h`, `ext.y` | `RET 10h` | stamp differs |
| `0098A804` | `0098A750` | `00722C20` | `accumulate_abs_scaled` | `ECX = &acc`; row `+70h`, `ext.z` | `RET 10h` | stamp differs |
| `0098A815` | `0098A750` | `004142E0` | `transform_local_centre` | `ECX = node+124h`; `&centre`, `node+50h` | `RET 8` | stamp differs |
| `0098BA32` | `0098BA10` | `00414DB0` | `pose_refresh_world` | `ECX = node+4Ch` | `RET` | `pose+C8h == 0` |
| `0098BA41` | `0098BA10` | `004134F0` | `node_copy_world_matrix` | `ECX = node+50h`; `pose+CCh` | `RET 4` | always |
| `0098BA59` | `0098BA10` | `00414DB0` | `pose_refresh_world` | `ECX = node+4Ch` | `RET` | `pose+10Ch == 0` |
| `0098BA70` | `0098BA10` | `00B63D50` | `pose_build_inverse` | `ECX = pose+110h`, `EDX = pose+CCh` | `RET` | `pose+10Ch == 0` |
| `0098BA82` | `0098BA10` | `004134F0` | `node_copy_inverse_matrix` | `ECX = node+90h`; `pose+110h` | `RET 4` | always |
| `0098BA92` | `0098BA10` | `0098B920` | `attach_as_child` | `ECX = parent`; `node` | `RET 4` | `parent != 0` |
| `0098BAAF` | `0098BA10` | `0098A750` | `node_rebuild_world_bounds` | `ECX = node` | `RET` | `parent == 0` |
| `0098BABE` | `0098BA10` | `0098AD60` | `cell_of_point` | `ECX = &cellMin`, `EDX = node+13Ch` | `RET` | `parent == 0` |
| `0098BACD` | `0098BA10` | `0098AD60` | `cell_of_point` | `ECX = &cellMax`, `EDX = node+148h` | `RET` | `parent == 0` |
| `0098BAF8` | `0098BA10` | `0098A310` | `register_node` | `ECX = index`, `EDX = node`; `&cellMin`, `&cellMax` | `RET 8` | fits 2x2 |
| `0098A525` | `0098A500` | `0098A3D0` | `unregister_node` | `ECX = index`, `EDX = node` | `RET` | grid member |
| `0098A563` | `0098A500` | `0098A4C0` | `remove_loose` | `ECX = index`, `EDX = node` | `RET` | `node+40h == 0` |
| `0098A575` | `0098A500` | `0098A2C0` | `detach_from_parent` | `ECX = parent`; `node` | `RET 4` | `node+108h != 0` |
| `0042E652` | `0042E630` | `00415350` | `singleton_lock` | none | `RET` | first call |
| `0042E68B` | `0042E630` | `00BF681B` | `operator new(0x16018)` | `0x16018` | `_cdecl` | still null |
| `0042E699` | `0042E630` | `0042D450` | `construct_index` | `ECX = block` | `RET` | allocation succeeded |
| `0042E6A7` | `0042E630` | `00415350` | `singleton_lock` | none | `RET` | still null |
| `0042E6B5` | `0042E630` | `00BD0C30` | `register_singleton_lifetime` | `ECX = lock owner`; instance | — | still null |
| `0042D46E` | `0042D450` | `00BF79F0` | `memset` | `this+84h`, `0`, `0x15F90` | `_cdecl` | always |

## Coverage

| routine | coverage |
| --- | --- |
| `0098BDB0`, `0098BC70`, `0098A310`, `0098A3D0`, `0098A750`, `0098BA10`, `0098A500`, `0098A4C0`, `0098A2C0`, `0042E630`, `0042D450`, `00722C20` | complete |
| `0098B920` | complete for the append; `0098B530` is a contract |
| `0098A920` | partial: `0098A920..0098AA13` read (the box store and the half extents); `0098AA13..0098AA7A`, the centre, unread |
| `0098B530` | partial: `0098B530..0098B5DC` read (the eight-corner walk's setup); the merge tail unread |
| `0098B7D0` | partial: `0098B7D0..0098B8F2` read (the shape-box union); the tail unread |

## Open questions

* The 2x2 cap is tested at attach (`0098BAD2`) but not at re-bucket (`0098BC70` step 8 calls
  `0098A310` with whatever rectangle the new AABB spans). A node that is attached small and then
  grows, or rotates so that its AABB spans three cells on an axis, overruns the four link slots.
  Either the game's proxies never do that or a guard exists somewhere this packet did not read.
* The link owner back-pointer (`node+14h/+20h/+2Ch/+38h`) has no writer in these routines.
* `index+4h` is zeroed by the constructor and read by nothing here.
* The vtable at `00CE3CEC` was not followed; the index is destroyed through the lifetime registry.
* No run-time evidence was taken. The refresh runs every fixed step in the executable's own path,
  so a run log could confirm the claim that a static root is skipped; that is left for the
  integrator, who owns `bsp_game.exe`.
