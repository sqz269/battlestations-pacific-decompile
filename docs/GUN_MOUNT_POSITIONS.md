# Where a gun fires from: the mount point

Addresses: `00730160`, `0072F830`, `0072E6D0`, `007325A0`, `00732560`, `007327B0`,
`0072AB80`, `006FDD70`, `006E3DC0`, `006FE160`, `004181A0`, `004142E0`, `004134F0`,
`00413920`, `00B6DB70`, `00B63F10`, `0072BF10`.

`docs/GAME_EXECUTABLE.md` milestone 2t says a platform's mount point is a model node and
that every gun in the reconstruction fires from its unit's pose translation raised by the
class `Height`. Both halves are answered here. The native mount point is a **model node
plus a per-barrel local offset carried by the weapon class**; the class `Height` has
nothing to do with it.

`docs/GUN_DISPERSION.md` reads `00730160` and stops at `00730762`, labelling
`00730762..00730A1B` "the muzzle-position tail ... by call shape only". This document is
that tail. It reuses that document's frame-A convention verbatim.

Every name below is a hypothesis, not a recovered symbol. Evidence rows are in
`reports/cc7_gun_mount_positions.json`; the rules are in `include/bsp/gun_mount_positions.hpp`
and `src/gun_mount_positions.cpp`.

## 1. The answer in one line

```
muzzleWorld = TransformAffinePoint( gunClass->muzzleOffsets[ gun->nextFireBarrel ],
                                    gun->muzzleNode->worldMatrix )
```

with `gun->muzzleNode` the first non-null of the model nodes named `"barrel"` and
`"base"` and the model's own first node, and with the whole expression collapsing to
`gun->muzzleNode->worldMatrix.translation` when the class carries no muzzle offsets.

## 2. ABI, with the stack-cleanup evidence

| Routine | ABI | Cleanup evidence | Coverage |
| --- | --- | --- | --- |
| `00730160` `BSP_Gun_Fire` | `__thiscall(gun)(bool useExplicitThrow, float throwA, float throwB)` | `RET 0Ch` at `00730A1B` | partial: complete for `00730762..00730A0E`; `00730160..0073075E` is `docs/GUN_DISPERSION.md`'s |
| `0072F830` `BSP_Gun_SpawnShotAndEffects` | `__thiscall(gun)(const float* muzzleWorld, const float* muzzleLocal, const float* direction, int flag)` | `RET 10h` at `0073015A`; four pushes at `0073082D`-`0073083D` | partial: complete for the argument binding, the slot-`1E0h` call and both `0072BF10` sites |
| `0072E6D0` `BSP_Gun_SetupFromDescriptor` | `__thiscall(gun)(...)` | not read; only the node block is claimed | partial: `0072E93D..0072EE97` and `0072E6F5..0072E73A` |
| `007325A0` | `__thiscall(gunClassLoader)()` | `RET` at `007326D9`, no immediate | complete for the "fire" list copy; the two other branches from `007326DA` are unread |
| `0072AB80` | `__thiscall(gunClass)()`, returns `int` | `RET` at `0072ABAB` / `0072ABD1` | complete |
| `006FDD70` | same shape | `RET` at `006FDD7A` / `006FDD93` | complete |
| `006E3DC0` | `(float* out, const float* in)`, `ECX` unused | `RET 8` at `006E3DD8` | complete, from the raw listing; no Ghidra function |
| `006FE160` | `__thiscall(gun)(float* out, const float* in)` | `RET 8` at `006FE38B`; `MOV EAX,[ESP+78h]` at `006FE2F8` is `out`, `MOV EDI,[ESP+7Ch]` at `006FE170` is `in` | complete |
| `004181A0` | `__thiscall(vectorHeader)(int index)`, returns `begin + index*0Ch` | `RET 4` at `004181D8` | complete |

`0072F830`'s four arguments are settled by the four pushes at `0073082D`-`0073083D` and
the `RET 10h`, and each is bound by its read inside the callee: `[ESP+0ECh]` (`arg1`) at
`0072FEF0`, `[ESP+0F0h]` (`arg2`) at `0072F882`, `[ESP+0F4h]` (`arg3`) at `0072FE5C`,
`[ESP+0F8h]` (`arg4`) at `0072FD79`, all in the frame `ESP = entry-0E8h` that the
prologue `0072F845 SUB ESP,0CCh` plus the four register pushes produces.

## 3. The node: `gun+3CCh`, resolved once at setup

`BSP_Gun_SetupFromDescriptor` looks the mount up by **node name** against the model
hanging off `[gun+360h]+160h`:

| Site | Field | Source |
| --- | --- | --- |
| `0072E98A` | `gun+3BCh` | `[[gun+360h]+160h]+0Ch`, the model's own first node |
| `0072E9E2`-`0072E9F0` | `gun+3C4h` | `0071AD50(model, "base")` — the string at `00CFACD8` is `62 61 73 65 00` |
| `0072EA10`-`0072EA1C` | `gun+3C8h` | `0071AD50(model, "barrel")` — the string at `00CF70C4` is `62 61 72 72 65 6C 00` |
| `0072EE65`-`0072EE8F` | `gun+3CCh` | the first non-null of `+3C8h`, `+3C4h`, `+3BCh`; `0072EE6F` stores null first so "none" is the default |

`0071AD50` is a name lookup on the model's node table and is **contract: not read** — it
belongs to the model/scene packet. What is proven here is the call shape
(`PUSH <string>; MOV ECX,<model>; CALL 0071AD50` at `0072E9E2`/`0072EA15`, the result
stored straight into the gun) and the two names.

The node's world matrix is at `node+0F0h`, refreshed on demand:

```
007301C3  TEST byte ptr [ESI+5Ch],0x2
007301C8  JNZ  007301D1
007301CA  MOV  ECX,ESI
007301CC  CALL 00B6DB70          ; BSP_Transform_RefreshWorldMatrix
007301D1  ADD  ESI,0xF0
007301D7  MOV  ECX,0x10
007301DC  LEA  EDI,[ESP+0x48]
007301E0  MOVSD.REP ES:EDI,ESI   ; 10h dwords = the 4x4
```

`ESI` is `[gun+3CCh]` (`007301BD`, the only write to `ESI` before the copy; the other
`MOV ESI` sites in the body are at `0073026E`, `007304AF`, `0073089C`, `00730916`,
`007309C4`, all after it). The matrix layout is settled by its own uses inside this
function, not assumed: rows 0 and 1 are the dispersion plane (`[ESP+48h]`, `[ESP+58h]`),
row 2 is the shot direction (`0073022A`), and row 3 is the translation — proved by the
fallback branch reading `node+120h`, `+124h`, `+128h` at `007308C1`/`007308D6`/`007308ED`,
which is `node+0F0h + 30h`.

Between the copy and its use the matrix can be replaced:

```
007301F7  MOV  ECX,[EBP+0x3C]
007301FA  LEA  EDX,[ESP+0x48]   ; the node matrix
007301FE  PUSH EDX
00730201  MOV  EDX,[[ECX]+0x94]
00730207  LEA  EAX,[ESP+0x8C]
0073020E  PUSH EAX
0073020F  LEA  EAX,[ESP+0xD0]
00730216  PUSH EAX
00730217  CALL EDX              ; only when [[gun+3Ch]]+8Ch answered true at 007301F1
00730219  MOV  ECX,EAX
0073021B  CALL 00413920         ; BSP_Matrix_Multiply4x4
00730220  PUSH EAX
00730221  LEA  ECX,[ESP+0x4C]
00730225  CALL 004134F0         ; BSP_Matrix_Copy4x4X87, back into the node matrix
```

Three pushes serve two calls: `00413920` reads its own arguments at `[ESP+44h]` and
`[ESP+48h]` after `SUB ESP,40h`, which are the first two of those pushes, so the vtable
slot `94h` consumes exactly one (`RET 4`) and returns a matrix pointer in `EAX`. The
object at `gun+3Ch` and its slot `94h` are **contract: not read**; the packet's claim is
only that a successful slot-`8Ch` test routes the mount through a matrix that object
supplies. This is the seam where a recoil or animation pose would enter.

## 4. The per-barrel offsets: weapon class `+98h`

`gun+3F4h` is the weapon class descriptor. `+98h` is a `std::vector` header — `begin` at
`+9Ch`, `end` at `+0A0h`, elements `0Ch` bytes:

- `004181A0` returns `begin + index*0Ch` (`LEA EAX,[EDI+EDI*2]; LEA EAX,[ECX+EAX*4]` at
  `004181D0`), and range-checks against `(end-begin)/0Ch`.
- `0072AB80` and `006FDD70` both compute `(end-begin)/0Ch` with the magic `0x2AAAAAAB`
  and `SAR EDX,1`.
- `004142E0` `BSP_Vector3f_TransformAffinePoint` consumes one element, so an element is a
  float triple.

**Producer.** `007325A0` fills it at class load. It builds the `std::string "fire"`
(`00CE6798`, length 4, pushed at `007325CC` and `0073261A`), queries the model twice
through `00718870` with flags `0` and `1`, and on the first result copies a range into
the descriptor's vector:

```
00732689  MOV  EDI,[EAX+0x4C]
0073268F  LEA  EBP,[ESI+0x98]   ; the destination vector header
00732695  LEA  ESI,[EAX+0x44]   ; the source vector header
...
007326B4  PUSH EBP / PUSH EBX / PUSH EDI / PUSH ESI / PUSH EAX / PUSH ESI / PUSH ECX
007326BF  CALL 00732560
007326C4  ADD  ESP,0x1C          ; seven __cdecl arguments
```

`00732560` is a `std::copy`-shaped loop: `ADD ESI,0Ch` per step (`0073257B`) with
`004215D0` appending each element. So the descriptor's muzzle list is a **verbatim copy
of the element list of the model's `"fire"` node group**, element size `0Ch`.

`00718870` and the `"fire"` group's own layout belong to the model/resource packet and
are **contract: not read**. The boundary is exactly here: the gun side proves what the
vector contains and how it is indexed; what builds the `"fire"` group from the mesh is
someone else's packet.

`007327B0` `BSP_GunClass_ReadLuaFields` supplies the two salvo fields next to it:

| Field | Lua key | Site | Type |
| --- | --- | --- | --- |
| `class+0CCh` | `OneTimeBulletAmount` (`00CFE898`) | `00732ADB`/`00732AE4` | int |
| `class+0D0h` | `MultiBulletConeAngle` (`00CFE880`) | `00732B17`/`00732B1C` | float |

## 5. The shot, end to end

`gun+448h` is seeded at `0072E71A` from `0072AB80(gunClass)` — `max(1, muzzleCount)` —
and `gun+44Ch` is zeroed at `0072E72A`. `BSP_Gun_Fire`'s tail then, per shot:

| Step | Site | What |
| --- | --- | --- |
| 1 | `00730762`-`00730793` | `count = (class+0A0h - class+9Ch)/0Ch`; a null `begin` or a non-positive count jumps to `00730875`, the fallback |
| 2 | `00730799`-`007307A0` | `004181A0(class+98h, gun+44Ch)` -> the local-space mount point |
| 3 | `007307A5`-`007307CD` | copy that triple into the frame slot that becomes `arg2` |
| 4 | `007307C4`-`007307D3` | `004142E0(localOffset, out, nodeMatrix)` -> the world mount point |
| 5 | `007307D8`-`007307FC` | copy the result into the slot that becomes `arg1` |
| 6 | `0073082D`-`00730840` | `0072F830(arg1=world, arg2=local, arg3=direction, arg4=1)` |
| 7 | `007309ED`-`00730A0E` | `gun+44Ch = (gun+44Ch + 1) % gun+448h` |

The fallback at `00730899`/`00730913` re-reads `gun+3BCh` (**the model's first node, not
`gun+3CCh`**), refreshes it, zeroes the `arg2` triple and passes `node+120h..+128h` as
`arg1`. So a class with no `"fire"` nodes fires every barrel from the model origin. That
is the only case in which a battery shares one origin, and it is a data property of the
class, not a rule of the engine.

Inside `0072F830`, `arg1` is forwarded to `0072BF10` `BSP_Gun_CreateProjectile` as `pos`
and the slot-`1E0h` result as `dir` (both call sites, `00730042` and `00730076`);
`docs/PROJECTILE_IMPACT.md` independently records the resulting seven-argument
`classDesc->vtable[20h](0, ownerId, pos, dir, flag, nearCamera, gun[+41Ch])` at
`0072C006`. `arg2`, the local offset, is stored into the translation row of a matrix
copied from the template at `00E1AE30` (`0072F8B7`-`0072F8E4`, `[ESP+54h]+30h`): it
places the muzzle-flash effect in the node's own space.

## 6. Correction: gun vtable slot `1E0h` is the direction, not the origin

`docs/GUN_PLATFORM_ARC.md`'s vtable table calls slot `1E0h` "the unmodified muzzle
origin" and `006FE160` "how a salvo mount spreads its barrels". The `in` it receives is
the shot **direction**, and `006FE160` spreads the shot cone, not the barrels.

- **was:** slot `1E0h` base `006E3DC0` — "copies three floats; the unmodified muzzle origin".
- **is:** slot `1E0h` takes and returns a direction. The `in` pointer at the only call
  site is `0072F830`'s `arg3`, which `BSP_Gun_Fire`
  fills at `0073022A`/`0073023C`/`00730249` from **row 2 of the node matrix** and then
  perturbs with the dispersion cone at `00730654`..`0073075E`. The world position is a
  different argument (`arg1`) produced by step 4 above.

  `0072FE63`-`0072FE74` is the **only** dispatch of slot `1E0h` in `.text`. A byte scan
  covers every encoding: `FF 9x E0 01 00 00` (`CALL [reg+1E0h]`, six register forms, no
  matches), `FF 94 ?? E0 01 00 00` (the SIB form, no matches), and `8B ?? E0 01 00 00`
  plus `8B ?? ?? E0 01 00 00` (`MOV reg,[...+1E0h]` then an indirect call) — of whose 34
  hits `0072FE66` is the only one inside a gun body, the rest being `[ESP+1E0h]` locals
  and unrelated fields. Slot `1E0h` itself appears in nine vtables (`006E3DC0` has eight
  data xrefs, `006FE160` one), so the dispatch is shared, not the caller.
- **evidence:** three independent lines. (a) `docs/GUN_DISPERSION.md`'s own frame-A table
  already calls that triple "row 2, the direction the shot leaves on". (b) `006FE160`
  passes `in` to `00B63F10` `BSP_Matrix_BuildLookAt` as the look-at **target** with the
  eye pinned to the zeroed triple at `[ESP+38h]` (`006FE192`-`006FE19E`, `LEA EDX` at
  `006FE18A`), which is only meaningful for a direction. (c) the ring radius
  `class+0D0h` is the Lua key `MultiBulletConeAngle` (`00CFE880`, `00732B1C`) — an angle
  applied to a direction, not a length applied to a position.
- **effect:** none on `src/gun_platform_arc.cpp`'s arithmetic, which is correct as
  written; only the name `gun_ring_muzzle_origin_006fe160` and the doc prose are wrong.
  `006FE160`'s ledger evidence already says "builds a basis from the input direction".

## 7. Correction: `006FDD70` divides by `0Ch`, not `18h`

`docs/GUN_DISPERSION.md` section 5 records `006FDD70(gunclass)` as
`([+0A0h] - [+9Ch]) / 18h`.

- **was:** `/18h`.
- **is:** `/0Ch`.
- **evidence:** `006FDD83 MOV EAX,0x2AAAAAAB; 006FDD88 IMUL ECX; 006FDD8A SAR EDX,0x1`.
  The magic `0x2AAAAAAB` with a one-bit post-shift is division by 12; `/24` needs
  `SAR EDX,2`. `0072AB80` (`0072AB9A`) and `006FE160` (`006FE25D`) use the identical
  sequence on the same two fields, and `004181A0`'s indexing stride is `0Ch`
  (`004181D0 LEA EAX,[EDI+EDI*2]`).
- **effect:** the torpedo fan divides the tube spacing by `n-1` where `n` is the tube
  count; halving `n` halves the fan. `src/gun_dispersion.cpp` takes the count as an
  explicit argument, so the reconstruction is unaffected — only a host that derives the
  count from the raw byte span would be.

## 8. The class `Height` is not a mount height

`Height` is the unit **class** row's hull height at `class+0A8h` (`docs/GAME_EXECUTABLE.md`
lines 6686 and 7052; `include/bsp/ship_motion.hpp` comments it `class+A8h`). Its native
consumers are ship motion — draft and the keel list — and the hit test, which uses it as
a half-extent (`src/game_hosts_gunnery.cpp:1394`, `hull_height * 0.5f`). One gunnery path
does raise a point by it: `00864D90` builds the **aim point** as the target's position
plus `[target+538h]+0A8h`, so a shooter aims at the middle of a hull rather than its
waterline. Nothing on the firing side reads it.

`src/game_hosts_gunnery.cpp:1095`, `muzzle = {origin.x, origin.y + hull_height, origin.z}`,
therefore has no native counterpart: it is a placeholder that both raises the muzzle by a
whole hull height and gives every gun on a unit the same origin.

## 9. Is the muzzle origin per-barrel? Yes, and the ring is not it

Three separate per-shot spreads exist and only the first moves the origin.

| Spread | Where | Index | Count | Magnitude | Moves |
| --- | --- | --- | --- | --- | --- |
| the mount list | `00730799` | `gun+44Ch` | `(class+0A0h - class+9Ch)/0Ch` | the authored offset itself | the **position** |
| the salvo cone | `0072FEFB`..`00730056` | the pellet loop counter | `class+0CCh` `OneTimeBulletAmount` | `class+0D0h` `MultiBulletConeAngle` | the direction |
| the sub-type `27h` cone | `006FE160` | `gun+44Ch` | `(class+0A0h - class+9Ch)/0Ch` | `class+0D0h` | the direction |

So `gun+44Ch` is genuinely a barrel index, and it selects **both** the mount point and,
for sub-type `27h`, a direction offset keyed to the same barrel. The ring in `006FE160`
is a second-order effect on top of a mount point that already differs per barrel; it is
not the mechanism that spreads a battery. The salvo cone is already reconstructed as
`gun_multi_bullet_ring_0072fefb` in `include/bsp/gun_dispersion.hpp` and is not repeated.

## 10. Proven vs. assumed

**Proven from the listing.**
- `0072F830`'s four arguments and their meanings, from the `RET 10h`, the four pushes and
  each argument's read inside the callee.
- `gun+3CCh`'s three-way pick and the two node names, read from the bytes at `00CFACD8`
  and `00CF70C4`.
- `node+0F0h` is the world matrix; `node+120h` is its translation row; row 2 is the shot
  direction. Each from a use inside `00730160`, not from a matrix convention.
- The muzzle-offset vector's location, stride and indexing, from `004181A0`, `0072AB80`
  and `004142E0`.
- The producer of that vector: a verbatim copy of the `"fire"` group's element list.
- `class+0CCh` / `class+0D0h` and their Lua key names, from `007327B0`.
- The barrel index's round-robin advance.
- Both corrections in sections 6 and 7.

**Assumed or unread.**
- `0071AD50` is a lookup by node name: inferred from the call shape and the two string
  constants; the body was not read.
- `00718870` and the `"fire"` group's record layout: not read. The claim is only that the
  copied elements are `0Ch` bytes and are consumed as points.
- `[gun+3Ch]`'s vtable slots `8Ch` and `94h`: not read. Whether they carry recoil,
  animation or an attachment pose is open.
- `007325A0`'s second `"fire"` query (flag `1`, result in `EDI`) and its branches from
  `007326DA`: not read. The packet claims only the flag-`0` path.
- `0072E6D0` outside `0072E6F5..0072E73A` and `0072E93D..0072EE97`.
- Whether a gun's `"barrel"` node is animated by the elevation drive, which is what would
  make the mount point move with the gun rather than only with the hull. The matrix is
  read fresh every shot, so it can move; nothing here proves it does.
- No run-time evidence was taken. The path is reachable in `bsp_game.exe` only once a
  host supplies real node transforms, which is precisely what this packet says is missing.

## 11. Follow-up packets

To give each gun a real origin in the host, in order:

1. **`gun_mount_node_resolution`** — read `0071AD50` and the model's node table, and
   establish how `[gun+360h]+160h` reaches the model. Deliverable: a node lookup by name
   returning a world matrix. This is the one hard dependency; everything else is already
   proven. It sits inside the model/scene packet's territory and must be claimed there.
2. **`weapon_class_fire_group`** — read `00718870` and the `"fire"` node group so the
   per-barrel offsets can be loaded from data rather than authored by hand. Until then a
   host can supply the offsets itself; the indexing rule in
   `include/bsp/gun_mount_positions.hpp` is complete.
3. **`gun_mount_pose_override`** — read `[gun+3Ch]`'s slots `8Ch` and `94h`. Needed only
   if the mount must move with recoil or with the turret's own animation; without it the
   mount is the node's resting world pose.
4. **The host wiring itself** needs, per gun: the picked node's world matrix and the
   class's offset list. With those two, `gun_muzzle_world_position_00730762` and
   `gun_muzzle_direction_0073022a` give the origin and the unperturbed direction, and the
   height difference `h = aim.y - muzzle.y` that makes the recovered ballistic arc
   `00955630` differ from the `asin(gR/v^2)/2` pre-estimate becomes non-zero for the
   first time.

## 12. Coverage

| Routine | Coverage |
| --- | --- |
| `00730762`..`00730A0E` (the mount tail of `00730160`) | complete |
| `0072AB80`, `006FDD70`, `004181A0`, `006E3DC0` | complete |
| `006FE160` | complete for the argument binding and the ring; the arithmetic is `docs/GUN_PLATFORM_ARC.md`'s |
| `0072F830` | partial: argument binding, the slot-`1E0h` call and both `0072BF10` sites; the effect body `0072F891..0072FDF0` unread |
| `0072E6D0` | partial: `0072E6F5..0072E73A` and `0072E93D..0072EE97` |
| `007325A0` | partial: the flag-`0` "fire" copy; `007326DA..00732792` unread |
| `007327B0` | partial: `00732AB9..00732B1C` only |
| `0071AD50`, `00718870`, `[gun+3Ch]` slots `8Ch`/`94h`, `00B63F10`'s body | **contract: unread**, model/scene and resource packets |
