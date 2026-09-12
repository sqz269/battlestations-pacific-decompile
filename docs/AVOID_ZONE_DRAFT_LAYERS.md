# Avoid-zone draft physics inputs

Packet `orch6_avoid_zone_draft_layers`, worker `agent/orch6-zone-wrappers`.
Read-only Ghidra: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Names are descriptive hypotheses. The C++ interfaces are not binary replacements.

The tail of `00424D00` reduces actual ship depth settings into `{float key,
uint32 mask}` pairs and calls `00423C50` for each pair. It does **not** insert
draft groups. `00423C50` requires an exact existing group, partitions its zone
polygons, extrudes each partition and creates static Dyn collision bodies.
The original scene scan and its zone/group producers remain unchanged.

This corrects the old “registers draft keys” language in
`AVOID_ZONE_GEOMETRY.md` and the proposed per-group segment-tree interpretation.
Those were explicitly unread-tail hypotheses. The outputs found here are the
manager's body-pointer vector at `+5Ch` and copied-hull list at `+6Ch`; neither
is a new source group or source zone.

## Implemented scope

| Native body or fragment | Evidence and implementation |
| --- | --- |
| `00424D00..00425487` | Complete containing body and call attribution checked; only the tail's tuning reads, ordered pair reduction and key conversion are added. No full rebuild replacement. |
| `004223B0..00422432` | Complete valid-container push-back semantics with `std::vector`; native allocator/debug iterator/SEH ABI is not reproduced. |
| `00423C50..00424671` | Entire listing and dependencies at the caller boundary read. **Partial projection**: exact group selection, source corner snapshot, extrusion of explicit native partition indices and body/shape descriptor stores. |
| `00837DE0..00837DFA` | Existing `ship_tuning_block_offset` reused; no duplicate selector. |

The partial stages cannot create a collision body by themselves. Remaining
inputs are the actual `004F6F20` partition result, its ordered index arrays,
the `00C5DF30` Dyn hull and `00C32D20` result, the manager-owned copied hull
handle and the real world at `(*00E188A8)+18h`. No substitute partitioner,
hull, physics world or default ship depths are provided.

## Tuning producer and pair reduction

`0083B5E0` selects `ShipGlobals["AvoidZoneDepthsSingle"]` for the record at
settings `+80h`, then `ShipGlobals["AvoidZoneDepthsMulti"]` for `+F0h`.
The keys are flat entries directly under `ShipGlobals`, **not** nested
`AvoidZoneDepths.Single/Multi`. The pointer selection and literal addresses
are `00841B81/87` and `00841B8E/94`. Both records are `70h` bytes of integers.
The existing `AvoidZoneDepthSlot` constants are reused.

At `00424DDF`, `00424C40` returns the settings object. `MOV ECX,EAX` at
`00424DF0` supplies it to `00837DE0` at `00424DF6`. That six-instruction
selector tests `(*00E188A8)+1FE4h`: zero selects `+80h`, any nonzero selects
`+F0h`. The new settings reader copies only the following proven fields and
requires those bytes to have already been produced by the real loader.

| Order | Class key `[1]` | Record offset | Integer producer store | Mask | Append call |
| --- | --- | --- | --- | --- | --- |
| 1 | BattleShip | `28h` | `00841BEE` | `10h` | `00424E19` |
| 2 | MotherShip | `00h` | `00841CA6` | `20h` | `00424EBE` |
| 3 | Destroyer | `08h` | `00841D5D` | `40h` | `00424F7C` |
| 4 | TBoat | `10h` | `00841E11` | `80h` | `0042503A` |
| 5 | LargeLandingShip | `20h` | `00841F85` | `200h` | `004250FB` |
| 6 | CargoShip | `30h` | `0084203D` | `400h` | `004251BC` |
| 7 | LightCruiser | `38h` | `008420F5` | `800h` | `0042527D` |
| 8 | HeavyCruiser | `40h` | `008421AD` | `1000h` | `0042533E` |
| 9 | Submarine | `5Ch` | `00842431` | `2000h` | `004253FF` |

Each class access is followed by index `1` through `00B67720`, then integer
conversion `00B66290`, before its store. The second depth in each surface
ship's pair is not read here. Neither SmallLandingShip (`18h`) nor MiniSub
(`48h`) is read by this tail, and no `100h` mask is inserted.

The first pair is appended unconditionally. Subsequent integers are converted
with `CVTSI2SS`, then compared to the stored **float** keys. An equal key ORs
the mask in place; a miss appends in the order above. This is not sorting or
integer deduplication: under nearest rounding, integers `16777216` and
`16777217` merge. Current MXCSR rounding is preserved by the intrinsic.
Before the `00423C50` call, `00425459` uses `CVTTSS2SI` to convert the float
back to a signed layer. For example, an `INT_MAX` tuning integer can round to
float `2147483648`, which converts to integer-indefinite `INT_MIN` when the
SSE invalid exception is masked. The implementation retains these instructions.

The currently installed `scripts/datatables/shipglobals.lua:210..237` reduces
to Single `{0,3EF0h}`, or Multi `{3,1E30h},{1,20C0h}` in that order. These are
observations, not defaults stored in C++. The native byte/literal reads and
script hash are recorded in the JSON report.

`004223B0` is `__thiscall(vector in ECX, element pointer on stack)`, both
returns `RET4`. Native vector offsets `+4/+8/+Ch` are begin/end/capacity;
the element stride is eight. Available space calls `0041C7E0` with destination
in ECX, count one in EDX and four stack words (`RET10h`), copying the element's
two words. Growth goes through `00421A50` then `00420F80`, which snapshots
both words before reallocation and grows capacity by 1.5 where sufficient.
The C++ vector performs the same append for valid inputs; capacity, allocator,
invalid-parameter callbacks and exception-frame identity are outside the claim.

## Exact group and hull input

`00423C50` receives manager in ECX and stack `(int layer, uint32 mask)`,
`RET8`. `00423C7C` calls existing `004120D0`, but `00423C81` immediately
compares the result's `+10h` key to the requested layer and returns on a
different key. The fallback group is not used for physics. Empty selected
groups also return normally. An entirely empty manager table is native-invalid;
the checked C++ selector throws in that case.

Within the exact group, `+4/+8` supplies the native zone-pointer slots/count.
Each zone supplies corner pointers/count at `+0/+4`. Unsigned count below
three skips it. `00423CDE` appends the first eight bytes (`x,z`) of each
corner, preserving native order and leaving the source zone intact. This
reuses `AvoidZoneNativeStorage`; it does not manufacture a second group type
or recompute native corner records.

At `00423D08`, `004F6F20` receives the point-vector address and float bits
`3C8EFA35` from `00CE3984`. Its body calls a multi-stage polygon partition
algorithm; those stages remain unreconstructed. `004F4130` returns the
number of 16-byte index-vector records at partitioner `+48h/+4Ch`.
`004F62C0` copies the selected vector by value. The extrusion function takes
that **explicit ordered index vector**; supplying a fan or the whole polygon
as a replacement would change native behavior and is not part of this packet.

Index vectors shorter than three are skipped and the loop continues. Other
indices are checked by native iterator/range validations; the C++ interface
throws for invalid indices instead of representing them as a clear result.
It rejects unrepresentable Win32 array sizes. Source points are copied twice
per index, then Y is overwritten at `00423FA8/00423FE4` with `-500/+500` from
`00CE3980/00CE397C`. The temporary resize's uninitialized fill words are never
used in the resulting points after these stores.

The mean sums one copy's X and Z in index order, rounding to binary32 after
each addition. `0042405B` converts the count through x87; `00424079..85`
divides and stores the means. Every point then subtracts mean X, double
positive zero from Y (`00D7A258`), and mean Z. The reconstructed x87 helpers
retain the one-operation/store schedule. World center is `(meanX,0,meanZ)`.
No area-weighted centroid, winding change or depth-dependent Y extent appears.

## Native ownership sequence beyond the projection

1. `0042414E`: construct a temporary eight-byte Dyn hull handle with
   `00C5DF30(point_count, local_points)`. Its constructor zeros both words,
   then calls `00C5DEB0`; that allocates a `68h` internal record and invokes
   `00C5DDD0`. Hull generation itself is unresolved.
2. `0042415F`: `00C32D20` reads the first handle pointer's `+4` dword. A result
   below four destroys the temporary handle (`00C37450`), frees local points
   and indices, and continues to the next piece. The exact producer meaning
   of this count is not needed or invented by the partial interface.
3. Accepted hulls append to manager `+6Ch` list. `004241D3` calls `0041C0F0`
   with sentinel, previous tail, and temporary handle. It allocates a `10h`
   node, writes next/previous at `+0/+4`, and invokes `00C40F50` at node `+8`.
   That copies the eight-byte handle and, when nonnull, allocates another
   `68h` record and calls `004039D0` to copy its contents. The link stores at
   `004241E3/004241E9` make this node the list's new tail. List counter/growth
   call `00420030` and allocator failure semantics remain native dependencies.
4. The shape descriptor's `+14h` points to **node+8**, the retained copied
   hull handle. It is not a pointer to the temporary handle or source zone.
   `00424204..004243DB` initializes the shape with `+00=0,+04=0,+08=mask,
   +0C=0,+10=4,+14=retained handle`, then an identity 3x4 transform at `+18`.
   Offset names preserve unproven filter/material meanings. Type four is
   selected by `00C5C940` for its `00C57F50` shape constructor.
5. Body descriptor stores use mass/inertia one, identity transform translated
   by `(meanX,0,meanZ)`, zero velocities/damping/listener/owner, both speed
   bounds `1000`, flags bit zero set, and one shape. The existing
   `DynBodyDescriptor` defaults match these stores. Its semantic `shape_count`
   does not replace the native vector's actual shape pointer. The returned
   `AvoidZoneDraftBodyInputs` carries the shape separately for that reason.
6. `00424472`: call `Dyn_World_CreateBody` with ECX from `(*00E188A8)+18h` and
   the body descriptor. `00C5D580` owns body allocation and calls the shape
   attach routine for each pointer. Append the returned pointer to manager
   `+5Ch` vector, using fast-path store or `00420830` at `004244CF`.
7. `_sprintf` at `004244F5` builds `aimap%d` from the post-append body count,
   then `0042453C` constructs a temporary string and the last body index is
   validated. **No name assignment or name-setting call exists in this
   listing.** The temporary string is destroyed; the hull handle, points,
   indices and partitioner are released. Source zones remain borrowed.

Neither retained-hull production nor world creation/retention is implemented
by the descriptor projection. It requires a real retained handle and rejects
null. There is no no-op host implementation, dummy physics output, or claim
that a descriptor is an actual body. Manager teardown ordering and native
allocation-failure rollback are outside the packet.

## Assembly and verification boundaries

`00424D00` saves manager ECX through EBP and `[ESP+14h]`; at `00425420` it
reloads EBX, and `00425460` restores ECX before every physics call. No assumed
callee-preserved EAX or inferred return value supplies the manager. It returns
with plain `RET`. `00423C50` saves its manager at `[ESP+60h]` while the layer
argument is pushed, later read at balanced-stack `[ESP+5Ch]`; its temporary
hull is separately at `[ESP+60h]` after stack restoration.

Ghidra's `_free` no-return assumptions create misleading decompiler returns.
Every one of the nine `00423C50` listing gaps and the tail gap at
`00425472..00425474` contains `83 C4 04` (`ADD ESP,4`), verified read-only.
They continue into cleanup/loops; they are not early returns. The nine-byte
gap `00424D47..00424D4F` is unreachable alignment skipped by the preceding
jump. No Ghidra flow repairs or annotations were applied. Fragment entry
addresses are inside their existing containing functions, not new functions.

`scripts/build.ps1` passed MSVC Win32 Release and the existing two CTests.
One ignored `/MANIFEST:EMBED` fixture passed eight checks: installed-value
reduction, wide-integer merge/indefinite conversion, both record selections,
exact-group miss, square extrusion, short-piece preservation, descriptors and
partition parameter bits. No tracked tests were added. This is build and
fixture evidence, not native whole-function differential, ABI compatibility,
runtime physics integration or game validation. Call rows are checked in
`reports/avoid_zone_draft_layers.json`.
