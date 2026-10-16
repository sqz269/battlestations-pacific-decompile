# CC13 bomb drop: 006E4D50 BSP_MBombPlatform_Drop, full body

Packet cc13-bombdrop. Addresses: 006E4D50-006E56EF. Source: `src/cc13_bombdrop.cpp`,
`include/bsp/cc13_bombdrop.hpp` (`bsp::cc13_bombdrop::bomb_platform_drop_006e4d50`), built in
`bsp_core`, not bound into `bsp_game`. Status: reconstructed from the listing and bytes,
build-tested only; no fixture, not game-validated. Names are hypotheses.

## ABI

`__thiscall(ECX = rack, [ESP+4] char use_given, [ESP+8] float angle, [ESP+0Ch] float spread)`,
`RET 0Ch`, no return value. SEH frame with handler `00C8278B`; EH state 0 covers only the
launch-effect handle (`006E4E5F`-`006E4E8A`). Rack vtable slot `+1D8h` (`00CF9880` single rack,
`00CF9AF0` multiple rack), reached from `00727E30 BSP_Gun_FireIfReady`.

## What the body does, in order

1. `006E4D7B`-`006E4EAD` launch effect. When the definition `[rack+3F8h]` has an effect at `+0Ch`:
   AddRef the effect (`effect+4`, `InterlockedIncrement`), `008687C0(&handle, [owner+4A4h], effect,
   &identity4x4, 1, 0, 0)` with owner `[rack+3F0h]`; a non-null handle gets byte `+9 = 1`;
   `owner->vtable[200h](0, 0)`; release the handle (`InterlockedDecrement(handle+4)`, `vtable[0]` at 0).
2. `006E4EAF`-`006E4ED6` the first child in the list `[rack+48h]` (next `+44h`) that answers
   `vtable[5Ch](2Ah)`. With none, skip to step 13. Only one round is dropped per call.
3. `006E4EDB`-`006E4FBF` cone. With `use_given`, `rack+400h = angle`, `rack+404h = spread`. Otherwise
   the throw `s = [rack+3F8h]+4h`. When the owner answers `vtable[5Ch](0Fh)` and has a bot
   (`owner+DF4h`), and its slot `owner+1B0h` is 8 or `00927F10(slot)` is true, then
   `s = 00999B70(bot, [[rack+3F8h]+34h]) * s`. Draw 1 gives `rack+400h`. Draw 2 goes through
   `00412E20` (tangent) into `rack+404h`. The calls are lazy, as in the image: `00927F10` only runs
   for slot != 8, and `00999B70` only when the test passes.
4. `006E4FC5`-`006E5045` `D = (t cos a, -t sin a, 1)` from `rack+400h`/`404h`. FSIN and FCOS work on
   the float angle, and each result is rounded to float.
5. `006E503F`-`006E5111` `w = [rack+3F8h]+8h`, `v = owner->vtable[34h](buf)`, `len = sqrt(vx²+vy²+vz²)`
   (CRT `_CIsqrt` `00BF7030`, called as `std::sqrt`), `inv = len > 0 ? 1/len : 0`, `n = inv*v`.
6. `006E50E3`-`006E51EE` drift at `rack+4ECh`: `(nz, 0, -nx)`, scaled by draw 3, plus draw 4 times `n`.
7. `006E51F1`-`006E525A` refresh the child's pose (`00414DB0` when byte `+C8h` is 0), then
   `mover = child->vtable[108h]()`. A kind-33h child with `[[child+314h]+E0h] <= 0` instead gets
   `D = (1, 1, 1)` and zero drift.
8. `006E525E`-`006E528A` fire-time gate. If `now - rack+480h > 0.30000001192092896` (extended against
   the double), `rack+480h = now`. Otherwise `mover+45h = 0`.
9. `006E529C` `mover->vtable[34h](&D, rack+4ECh)`. That is `006E1F00` for MBomb/MTorpedo
   (docs/CC13_VER_20.md).
10. `006E529E`-`006E5419` `mover+8h..10h += [[rack+3F4h]+DCh] * rack+DCh..E4h` (after refreshing the
    rack pose). Then the round `[mover+CCh]` is shown through a direct call of
    `cc13_seg33a::bomb_projectile_set_visible_006e0b40(round, 1)`. The rack copies the round's word
    `+174h` into `+49Ah` and `+460h` into `+49Ch`, refreshes the round, then copies the round's pose
    `+CCh..108h` into `+4A0h..4DCh` and `+318h..320h` into `+4E0h..4E8h`.
11. `006E541F`-`006E5515` paratrooper branch, taken when byte `rack+4F8h` is set and the child answers
    kind 31h. The landing point `rack+4FCh..504h` is copied to a local and gets draw 5 on x and draw 6
    on z. Then `007AB230(child, &local)` runs, and `rack+4FCh..504h += [rack+494h] * rack+508h..510h`.
12. `006E551B`-`006E55FE` `mover+1Ch = [owner+1B0h]`. The stamp is cleared (0) when all of these hold:
    session `[[00E188A8]+1FE4h]` is 0, the player `[00E188D8]` exists, `[[00E188A8]+1ED4h]` answers
    kind 4Eh, and `mover+ACh` is the player. For a kind-18h player it can instead be one of
    `player+3D0h[i]`, `i < [player+3CCh]`, with entries past index 4 reading null. Then
    `009555A0(owner, mover)`, refresh the child, and either `rack->vtable[224h]()` when
    `rack+484h > 0` (signed) or `rack+4F8h = 0`.
13. `006E5605`-`006E5633` ammo warning: if `00923810(rack, 1)` answers kind 0Fh,
    `00984BA0([00F8A0C4], [ancestor+9D4h])`. The ancestor is fetched again for the call.
14. `006E5638`-`006E56C4` drop interval. If the ancestor answers kind 0Fh: `last = plane+E8Ch`,
    `plane+E8Ch = 0`, `dt = (float)(now - last)`. When the plane is its squadron's `+3D0h`, its slot
    `+1B0h > 7` (unsigned), `dt > 0` and `last > 0`, it calls
    `0090E670([[00E188A8]+21A0h], [plane+E90h], plane, dt)`.
15. `006E56C9` `rack+490h = rack+48Ch`.

## Shared-RNG draws (00BD2F10, in call order)

00BD2FC0 is not called. Every draw goes through `BombDropHost::random_uniform_float_range_00bd2f10`
with a `DrawSite` tag. Draws 1-4 are taken only when a kind-2Ah child exists. Draws 1-2 are taken
only when `use_given == 0`. Draws 5-6 are taken only in the paratrooper branch.

| # | call | ECX (stream) | lo, hi | use |
|---|------|--------------|--------|-----|
| 1 | `006E4F91` | 1 | `0`, `pi` (float `00D7A264`) | cone angle `a` -> `rack+400h` |
| 2 | `006E4FB1` | 1 | `0`, `s` (throw, after ThrowMul) | cone spread; `tan` via `00412E20` -> `rack+404h` |
| 3 | `006E513C` | 1 | `-w`, `w` (`w = [rack+3F8h]+8h`) | side drift, scales `(nz, 0, -nx)` |
| 4 | `006E51AC` | 1 | `(float)(-w*0.5)` (`00D7A280`), `(float)(w*0.30000001192092896)` (`00CE3DC8`) | along-velocity drift, scales `n` |
| 5 | `006E5479` | 0 | `-50`, `50` (`00CECA0C`, `00CEB4D4`) | paratrooper landing x offset |
| 6 | `006E549E` | 0 | `-50`, `50` | paratrooper landing z offset |

## Callees the host supplies (BombDropHost)

| address | current name | ABI | does |
|---------|--------------|-----|------|
| `00BD2F10` | BSP_Random_UniformFloatRange | ECX stream, (float lo, float hi), RET 8, ST0 | the draws above |
| `00CE221C`/`00CE2220` | InterlockedIncrement/Decrement (IAT) | stdcall | effect handle refcounts |
| `008687C0` | BSP_PointEffect_CreateWithParentMatrix | ECX out, EDX parent, 5 stack args, RET 14h | launch effect (reconstruction: src/point_effect_owner.cpp) |
| `00927F10` | BSP_PartySlot_IsAiHeld | stdcall(slot), RET 4 | AI-held slot test |
| `00999B70` | BSP_PilotBot_ThrowMul | thiscall(bot, record), RET 4, ST0 | PilotBot throw multiplier |
| `00414DB0` | BSP_EntityPose_RefreshWorld | thiscall(entity) | world pose refresh (child, rack, round) |
| `007AB230` | BSP_MParatrooper_SetLandingPos | thiscall(para, float*), RET 4 | landing target |
| `009555A0` | BSP_Owner_RegisterProjectile | thiscall(owner, mover), RET 4 | owner registers the round |
| `00923810` | BSP_EntityAncestor | thiscall(entity, levels), RET 4 | ancestor (called 4 times) |
| `00984BA0` | BSP_WarningManager_DispatchAmmoTypeChannel | thiscall(manager, squadron), RET 4 | "ammoType" warning channel |
| `0090E670` | FUN_0090E670 | thiscall(table, slot <= 7, entity, float), RET 0Ch | per-slot record of `(entity+174h id, (float)(int)dt)` through `0090D9D0`; purpose provisional |
| vtable `+5Ch` | is-kind | thiscall(kind), RET 4 | kinds 2Ah, 0Fh, 33h, 31h, 4Eh, 18h |
| owner `+200h` | unknown | thiscall(0, 0) | called after the launch effect; meaning not established |
| owner `+34h` | world velocity | thiscall(out*) -> float* | drift basis |
| child `+108h` | mover | thiscall() -> mover | the round's +310h launch sub-object (per CC13_VER_20) |
| mover `+34h` | launch | thiscall(D*, drift*) | `006E1F00` for MBomb/MTorpedo |
| rack `+224h` | follow-up | thiscall() | `006E3E70` (single, via `00CF98CC`), `006E5AB0` (multiple, via `00CF9B3C`) |
| handle `+0` | destroy | thiscall() | last-reference release |
| globals | `[00F876A4]` time, `[00E188A8]` game, `[00E188D8]` player, `[00F8A0C4]` warnings | | |

Called directly, not through the host:
- `cc13_seg33a::bomb_projectile_set_visible_006e0b40`, through `seg33a_host()`.
- `native_projection_tangent_00412e20`, the naked original-byte helper.

## Reuse decisions

- `plane_release_ver_cc13::rack_throw_extent_006e4f30` is not called. It takes `slot_ai_held` and
  `throw_mul` precomputed, which would run `00927F10`/`00999B70` where the image does not. Its
  `rack_drop_cone_006e4f91` applies `tan` itself, but `D` here reads the stored `rack+404h`, which is
  already the tangent (and is the caller's value under `use_given`). Both files are also in
  `bsp_game`, not `bsp_core`. The arithmetic here agrees with both models.
- `gameplay_seg33_cc13` models rack state (`SingleRackState`). The step-12 slot `+224h` is its
  `rack_attach_next_round_006e3e70` for the single rack. It is invoked as a virtual through the host.
- `game_hosts_units.cpp` `lb_rack_drop_006e4d50` and `run_rack_bomb_drop_006e4d50` are a
  PLACEHOLDER for this routine. They predict an impact point and call the gunnery release. They do
  no bomb creation through the rack child/mover, none of this routine's dispersion draws, no launch
  effect, no owner registration, no ammo warning and no paratrooper branch.

## Uncertainty

- FSIN/FCOS/`_CIsqrt` run in x87 extended precision. The C++ uses `double` and rounds to float at
  every point where the image stores a float, so results can differ in the last bit in rare cases.
  The FPU precision-control word (D3D may set 24-bit) is not modelled.
- The meaning of owner `vtable[200h]`, `0090E670`/`0090D9D0`, and the kinds 33h/4Eh/18h is not
  established. The code reproduces the calls and tests only.
- Objects are raw image layouts. A host with its own structs needs image-shaped objects or an adapter.
