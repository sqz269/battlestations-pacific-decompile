# The projectile class family and the per-kind projectiles

Addresses: 006E8430, 006EA1C0, 006EA200, 006EA260, 006EA330, 006EA3A0, 006EA470, 006EA4F0,
006EA5E0, 006EA6B0, 006EA720, 006EA7F0, 006EA870, 0070CC30, 0070CAE0, 0070C370, 006E2C00, 006E2670,
00856420, 00856050, 006FD210, 006FC960, 0080ADE0, 0080AB40, 006E6410, 006E6450, 008A2710, 0089BB70

The descriptor, the registry and the field readers are in `docs/WEAPON_CLASS_DESCRIPTOR.md`; the
shell's flight, sweep and impact are in `docs/PROJECTILE_IMPACT.md`. This document is the class
family: which vtable each `Type` builds, what its `vtable[20h]` create allocates, and how a torpedo,
a bomb, a depth charge, a rocket and a flak shell differ from a shell.

## Reading the vtables

`ghidra xrefs` does not see data references outside functions, so every vtable here was read from
`.rdata` bytes. The layout is regular: a class name string follows the last slot of its vtable, so
`00CFA158` (slot `20h` of `00CFA138`) is followed by `MBullet` at `00CFA15C`, `00CFA460` by
`MArtilleryBullet`, and so on through `00CFA6A4` `MDummySubmarine`. Nine slots are in use:
`+0h` the scalar deleting destructor `00BD30E0`, `+18h` the Lua field reader, `+20h` the create.

## The family

Sub-type is the constant the constructor stores at `+8h`. Descriptor size is the allocation in
`006EA910`; instance size is the `operator_new` inside the create.

| Lua `Type` | native name | ctor | vtable | `+8h` | desc size | create (`+20h`) | reader (`+18h`) | instance size |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Bullet | MBullet | `006E8320` | `00CFA138` | `1` | `0D4h` | `006E8430` | `006E8770` | `284h` |
| Artillery | MArtilleryBullet | `006EA1C0` and inline at `006EAAB8` | `00CFA440` | `4` | `0D4h` | `006E8430` | `006E8770` | `284h` |
| Bomb | MBomb | `006EA260` | `00CFA4B0` | `9` | `0D8h` | `006E2C00` | `006E1BE0` | `468h` |
| Torpedo | MTorpedo | `006EA4F0` | `00CFA56C` | `0Ah` | `0FCh` | `00856420` | `008566B0` | `51Ch` |
| DepthCharge | MDepthCharge | `006EA3A0` | `00CFA508` | `0Bh` | `0F8h` | `006FD210` | `006FD400` | `474h` |
| DummyTarget | MDummyTarget | `006EA6B0` | `00CFA5CC` | `0Ch` | `0E4h` | `00700BE0` | `00701060` | unread |
| DummyKamikazePlane | MDummyKamikazePlane | `006EA7F0` | `00CFA63C` | `0Dh` | `0E0h` | `006FEF10` | `006FF170` | unread |
| DummySubmarine | MDummySubmarine | `006EA870` | `00CFA678` | `0Eh` | `0E0h` | `006FFDF0` | `00700050` | unread |
| Paratrooper | MParatrooper | `006EA720` | `00CFA604` | `0Fh` | `100h` | `007AC2C0` | `007AC780` | unread |
| Flak | MFlakBullet | `006EA470` | `00CFA53C` | `10h` | `0DCh` | `0070CC30` | `0070C030` | `298h` |
| Kamikaze | MKamikazePlane | `006EA200` | `00CFA478` | `11h` | `0D4h` | `006EA230` | `006E8770` | unread |
| Rocket | MRocket | `006EA330` | `00CFA4DC` | `12h` | `0E8h` | `0080ADE0` | `00809CB0` | `498h` |
| WaterMine | MWaterMine | `006EA5E0` | `00CFA59C` | `13h` | `0ECh` | `0085F310` | `0085F500` | unread |

The reader chain (`docs/WEAPON_CLASS_DESCRIPTOR.md`) gives the inheritance: MBullet is the root,
MArtilleryBullet and MKamikazePlane use its reader unchanged, MFlakBullet derives directly from it,
and MBomb sits between MBullet and the other eight.

Two sub-type consumers already named in `include/bsp/projectile_impact.hpp` gain their meaning:
`kProjectileSubTypeDecalVariant = 0x10` is the **flak** shell's decal branch in `0084BC60`, and
`kProjectileSubTypeFriendlyExempt = 0x11` is the **kamikaze plane**'s friendly-fire exemption in
`0084BF00`.

## What the sub-type gates

| consumer | site | rule |
| --- | --- | --- |
| launch bias | `006E85B4` | `4`..`7` lose `0.05 * 9.8103800773621` from `v.y` at spawn |
| world mode 2 | `0072FE0x` | `8`..`0Fh`, `12h`, `13h` return without spawning; `10h` flak and `11h` kamikaze still fire |
| ammo provider | `0073039D`-`00730476` | `0Ah` asks `00521E70(5)` and uses `gun+39Ch`; `0Bh` asks `(7)` and uses `gun+3A4h` or `gun+3A0h`; `10h` asks `(3)` and uses `gun+394h`; `4`..`7` ask `(4)` and use `gun+398h`; everything else asks `(2)` and uses `gun+390h` |
| last-fired stamp | `0095561B` | `[director + 1D0h + subType*4] = [00F876A4]`, so the director reserves at least `14h` sub-type slots |

## Two record layouts

MBullet and MFlakBullet share one layout: the **shot interface** subobject is at `+170h`
(vtable `00CF9D90` for the bullet, `00CFD570` for the flak), its self-pointer at `+23Ch`, and the
**tick element** at `+244h`. The create returns `projectile + 170h`.

The bomb family uses a larger record: the shot interface is at `+310h`, its self-pointer at `+3DCh`
(`006E2670` writes `param_1[0F7h] = param_1`) and the tick element at `+3E4h`. All four bomb-family
creates return `piVar5 + 0C4h`, which is `+310h` in bytes. The constructors also fill `+10h`,
`+24h`, `+170h` and `+1E4h` with further subobject vtables that this packet did not read.

| class | entity ctor | shot interface | tick element vtable | place pose (`+4h`) | tick advance (`+8h`) | slot `+Ch` |
| --- | --- | --- | --- | --- | --- | --- |
| MBullet | `006E7B00` | `+170h` `00CF9D90` | `00CF9D78` | `006E6750` | `006E6490` | `006E7D50` |
| MFlakBullet | `0070CAE0` | `+170h` `00CFD570` | `00CFD554` | `006E6750` | `0070C370` | `006E7D50` |
| MBomb | `006E2670` | `+310h` `00CF95E0` | `00CF93F0` | `006E1060` | `006E1300` | `006E1010` |
| MDepthCharge | `006FC960` | `+310h` `00CFBA10` | `00CFB9F8` | `006E1060` | `006E1300` | `006E1010` |
| MRocket | `0080AB40` | `+310h` `00D09078` | `00D09060` | `006E1060` | `006E1300` | `006E1010` |
| MTorpedo | `00856050` | `+310h` `00D0C378` | `00D0C35C` | `006E1060` | `006E1300` | `00855B00` |

So the bomb, the depth charge and the rocket run **the same tick** `006E1300` and the same pose
placement `006E1060`; they differ only in the destructor slot and in their descriptor fields. The
torpedo is the only one of the four that overrides slot `+Ch`, with `00855B00`. The flak is the only
kind that overrides the tick itself.

`006E1300`, `006E1060` and `00855B00` are `contract: unread`: this packet identified them as the
bomb family's tick slots from the vtable bytes but did not read their bodies.

## `006E8430` — the shell factory, in full

`__thiscall(classDesc, arg1, ownerId, float* pos, float* dir, char spawnFlag, char nearCamera,
int arg7)`, `RET 1Ch`, body `006E8430`-`006E86E4`, SEH frame. Seven stack arguments; the frame is
three entry pushes, `SUB ESP,50h` and three register pushes, so the return address sits at
`[ESP+68h]` and argument *n* at `[ESP + 68h + 4n]`.

| argument | slot | use |
| --- | --- | --- |
| 1 | `[ESP+6Ch]` | **never read**; `0072C004` pushes `0` |
| 2 | `[ESP+70h]` | read at `006E8516` into `+54h`, the owner id |
| 3 | `[ESP+74h]` | read at `006E851D`, the spawn position |
| 4 | `[ESP+78h]` | read at `006E8523`, the direction |
| 5 | `[ESP+7Ch]` | read at `006E86A0` into `+278h` |
| 6 | `[ESP+80h]` | read at `006E86A6` into `+1CCh` |
| 7 | `[ESP+84h]` | **never read**; `0072BFE3` pushes `gun[+41Ch]` |

That corrects `docs/PROJECTILE_IMPACT.md` step 9 of the factory:

| what | was | is | evidence |
| --- | --- | --- | --- |
| `+1CCh` and `+278h` | `+1CCh` the gun's spawn flag, `+278h` the near-camera flag, and `vtable[3Ch]` fires on the near-camera flag | `+278h` takes argument 5, the gun's spawn flag; `+1CCh` takes argument 6, the near-camera flag; `vtable[3Ch]` fires on argument 5 | `006E86A0`/`006E86B8` pair `[ESP+7Ch]` with `+278h` and `006E86A6`/`006E86BE` pair `[ESP+80h]` with `+1CCh`; the branch at `006E86C4` tests AL, which came from `[ESP+7Ch]`. On the caller side `0072BF10` pushes the constant `1` for argument 6 on its near-camera path (`0072BFC5`) and `0` on the normal path (`0072BFE8`), while argument 5 is `0072BF10`'s own fourth parameter |

The body, in order:

| order | site | step |
| --- | --- | --- |
| 1 | `006E8452`, `006E8462` | `operator_new(284h)` (size pushed at `006E844B`) then `memset 0`; a null result is tolerated and the constructor runs on address zero |
| 2 | `006E8478` | `006E7B00 BSP_ProjectileTickableEntity_Construct` |
| 3 | — | `+174h = classDesc` (this) |
| 4 | `006E8493`-`006E8511` | a scale-1 matrix on the stack (four `00D7A24C` = `1.0f` diagonal entries), then `00923870(0, [00E188A8+19CCh], &matrix)` with the projectile in ECX — the world entity registry that `GetShipTorpedoes` and `0089BB70` later walk |
| 5 | `006E8516`-`006E8590` | `+54h` owner id; the spawn position into `+A4h`, `+FCh`, `+1D0h` and `+1DCh`; `+C8h = 1`, `+10Ch = 0` |
| 6 | `006E8598`-`006E85F1` | `v = classDesc[+50h] * dir`, with `v.y -= 00D0DE84 * 00CF9058` when `classDesc[+8h]` is `4`..`7`; written to `+178h` and to `+94h` |
| 7 | `006E8625`-`006E8659` | `0085DC80()` at `006E8625`, the `[proj+48h]`/`+44h` child-node walk through `0042ED50` at `006E8642`, then `00414DB0` at `006E8659` when `+C8h` is clear |
| 8 | `006E8680`-`006E869E` | `0078CF20 BSP_GameWorld_SampleWaterHeight(pos.x, pos.z)`; `pos.y <= height` loads `vtable[28h]` at `006E869B`, otherwise `vtable[24h]` at `006E8696`, and the indirect call is at `006E869E` |
| 9 | `006E86B0`-`006E86CD` | `+1C8h = 0`, `+278h` and `+1CCh` from arguments 5 and 6, and `vtable[3Ch]` on argument 5 |
| 10 | `006E86D3` | returns `projectile + 170h` |

Step 8 is the water-entry branch, and it is a one-shot decision at the muzzle, not a per-step test.
The two modes are small:

- `vtable[24h]` = `006E6410`: if the shot's `+44h` flag is set, clear it, resume the effect at
  `+24h` and stop the children of the effect at `+28h`.
- `vtable[28h]` = `006E6450`: if `+44h` is clear, set it, stop the children of `+24h` and resume
  `+28h`.

So `+44h` of the shot interface (projectile `+1B4h`) is the underwater flag and `+24h`/`+28h` are
the air and water trail effects the descriptor's `TravelEfx` and `WaterTravelEfx` feed.

`0070CC30`, the flak create, is the same routine with `operator_new(298h)`, the flak entity
constructor `0070CAE0` and no launch bias: `0070CC9x` writes `speed * dir` into `+178h` with no
sub-type test, because `10h` is outside `4`..`7`.

## Per-kind differences

| kind | how it differs from a shell |
| --- | --- |
| Artillery | identical code path: same create, same reader, same tick. Only `+8h` differs, and that changes the launch bias (`006E85B4`), the ammo provider slot (4) and the director's stamp slot |
| Flak | own create `0070CC30` (`298h`-byte record, entity ctor `0070CAE0`, an extra subobject vtable `00CFD5B0` at `+24h`) and own tick `0070C370`, which runs the base tick `006E6490` first, then self-destructs when `classDesc[+54h] < flightTime` and runs a proximity search once `classDesc[+D8h] < flightTime`. Reader adds `MinRange`. Not suppressed in world mode 2 |
| Bomb | `468h`-byte record with the shot interface at `+310h`; create `006E2C00`, entity ctor `006E2670`, tick `006E1300`. Reader adds `TimeOutEfx`, so a bomb has a timeout explosion a shell does not |
| Depth charge | `474h` record, create `006FD210`. Shares the bomb tick. Its reader is the sinking model: `V0RandomFactor`, `DiveSpeed`, `DiveMinDepth` and `DiveMaxDepth` (both defaulting to `1.0`), plus `MaxWaterHitVel`, `MaxFall` and `WaterSplashEfx`. Ammo provider slot 7 |
| Torpedo | `51Ch` record, create `00856420` — the only create with **five** arguments and the only one that registers an observer pair (`00856594`-`008565D8`), which is the homing target binding. Shares the bomb tick but overrides slot `+Ch` with `00855B00`. Reader adds the swim model: `WaterTravelSpeed`, `HeadingTurn`, `MaxWaterHitVel`, `MaxFall`, `WaterSplashEfx` and the two homing turn speeds. Ammo provider slot 5 |
| Rocket | `498h` record, create `0080ADE0`, bomb tick. Reader adds `VMax`, `Acceleration`, `IgnitionDelay` and the `AntiAir` boolean, so a rocket accelerates after a delay instead of coasting from `V0` |
| Kamikaze | own create `006EA230` and an extra vtable slot `+24h` (`006EA240`); reader unchanged from MBullet. Exempt from friendly fire at `0084BF00` |
| Dummies, paratrooper, water mine | own creates and readers, all `contract: unread` |

## Torpedo entry points

`008A2710` `SetTorpedoSwimDepth(entity, depth)` is a Lua binding: it takes the entity pointer out of
argument 0 with `BSP_LuaTable_GetPtrField`, the number out of argument 1, and writes it to
**`torpedo + 47Ch`**. That is inside the `51Ch` torpedo record and outside the `468h` bomb record, so
the swim depth is a torpedo-only field.

`0089BB70` `KillTorpedoesFromUnit(unit)` walks the same world registry the create registers into:
head `*[[00E188A8+19CCh] + 8h]`, next link at node `+44h`, and for each node tests
`entity->vtable[5Ch](2Bh)` and `entity[+3BCh] == handle` before `BSP_MissionEntity_Kill(2)`. So
`2Bh` is the torpedo's kind id for the `vtable[5Ch]` predicate and `+3BCh` is the owner handle the
torpedo carries — the same field `GetShipTorpedoes` filters on in `docs/UNIT_WEAPON_DEVICES.md`.

## Host table

`create_projectile_006e8430` in `include/bsp/projectile_kinds.hpp` has one virtual per call site.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `006E8452`, `006E8462` | `operator_new`, `memset` | `allocate_zeroed` | `(284h)` -> storage | none; null is tolerated |
| `006E8478` | `006E7B00` | `construct_tickable_entity` | `(storage)` -> projectile | only when the allocation succeeded |
| `006E8511` | `00923870` | `register_in_world` | `(projectile; 0, registry, &matrix)` -> void | none |
| `006E8625` | `0085DC80` and the `+48h`/`+44h` walk | `refresh_child_poses` | `(projectile)` -> void | none |
| `006E8659` | `00414DB0 BSP_EntityPose_RefreshWorld` | `refresh_world_pose` | `(projectile)` -> void | `+C8h == 0` |
| `006E8680` | `0078CF20 BSP_GameWorld_SampleWaterHeight` | `sample_water_height` | `(x, z)` -> float | none |
| `006E869E` (slot loaded at `006E869B`) | shot `vtable[28h]` = `006E6450` | `set_water_mode` | `(shot; 0)` -> void | `pos.y <= water height` |
| `006E869E` (slot loaded at `006E8696`) | shot `vtable[24h]` = `006E6410` | `set_air_mode` | `(shot; 0)` -> void | otherwise |
| `006E86CD` | shot `vtable[3Ch]` = `006E6C20` | `on_spawn_flag` | `(shot)` -> void | argument 5 non-zero |

## Coverage

| routine | coverage |
| --- | --- |
| `006E8430` | complete for its argument handling, record writes, launch velocity and water-entry branch; the stack matrix build and the child-node walk are host contracts |
| `006EA1C0`, `006EA200`, `006EA260`, `006EA330`, `006EA3A0`, `006EA470`, `006EA4F0`, `006EA5E0`, `006EA6B0`, `006EA720`, `006EA7F0`, `006EA870` | complete (each is a vtable write plus `+8h` and one or two zeroed fields) |
| `0070CAE0` | complete |
| `0070CC30` | partial: read for its allocation size, entity constructor and the absence of the launch bias; the tail after `0085DC80` is unread |
| `006E2C00`, `00856420`, `006FD210`, `0080ADE0` | partial: allocation size, entity constructor and returned subobject only |
| `006E2670`, `00856050`, `006FC960`, `0080AB40` | partial: the subobject vtable writes only |
| `0070C370` | as `docs/PROJECTILE_IMPACT.md` has it: the two time comparisons; the proximity search `0070C4A4`-`0070C795` is unread |
| `006E6410`, `006E6450` | complete |
| `008A2710`, `0089BB70` | complete |
| `006EA230`, `00700BE0`, `006FEF10`, `006FFDF0`, `007AC2C0`, `0085F310`, `006E1300`, `006E1060`, `00855B00` | `contract: unread` |
