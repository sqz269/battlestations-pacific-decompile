# The bomb-family tick: flight, water entry, dive and ignition

Addresses: 006E1300, 006E1060, 006E1010, 006E1510, 006E27F0, 006E2800, 006E2570, 006E6900,
006FCD20, 006FD660, 0080A000, 008561F0, 006E2910

The class family, the record sizes and the vtable tables are in `docs/PROJECTILE_KINDS.md`; the
descriptor fields and their Lua keys are in `docs/WEAPON_CLASS_DESCRIPTOR.md`; the shell's own tick,
the sweep `0084C430` and the impact chain are in `docs/PROJECTILE_IMPACT.md`. The torpedo's
overrides are in `docs/TORPEDO_TICK.md`. This document is the tick the bomb, the depth charge, the
rocket and the torpedo share, and the two per-kind motion models that hang off it.

`006E1300`, `006E1060` and `006E1010` were `contract: unread` in `docs/PROJECTILE_KINDS.md` before
this packet; the coverage table below replaces that line.

## The record is the bullet record with 1A0h inserted

Every field `006E1300` touches that the bullet tick also touches is at the bullet's offset plus
`1A0h`, which is exactly how far the shot interface moved (`+170h` to `+310h`). The pairs are:

| field | MBullet | bomb family | native |
| --- | --- | --- | --- |
| shot interface | `+170h` | `+310h` | `006E1344` loads `[ESI-0D4h]` |
| class descriptor | `+174h` | `+314h` | `006E133B` loads `[ESI-0D0h]` |
| flight time | `+1C4h` | `+364h` | `006E147C` |
| sweep extra | `+1C8h` | `+368h` | `006E1418` |
| current snapshot | `+1D0h` | `+370h` | `006E101A` |
| previous snapshot | `+1DCh` | `+37Ch` | `006E1017` |
| owner | `+238h` | `+3D8h` | `006E1400` |
| tick element | `+244h` | `+3E4h` | the vtable tables |

The fields ahead of the insertion point keep their bullet offsets: the sweep-enable byte is `+5Ch`
in both (`006E138D` reads `[ESI-388h]`, and `3E4h - 388h = 5Ch`), the pose-valid byte is `+C8h`
(`006E13F9` reads `[EDI+0C8h]`) and the world translation is `+FCh`.

Tick-element-relative fields, which the bullet's tick never reads:

| offset from `+3E4h` | record | meaning | native |
| --- | --- | --- | --- |
| `+2Ch` | `+410h` | launch delay, negative until the drop is released | `006E130B` |
| `+34h` | `+418h` | the 4x4 matrix the commit caches | `006E1055` |
| `+64h` | `+448h` | that matrix's translation row (`34h + 30h`) | `006E102C` |
| `+74h` | `+458h` | active flag | `006E1331` |
| `+78h` | `+45Ch` | arming countdown, gates the collision sweep | `006E139B` |

## `006E1300` — the tick, in full

`__thiscall(MBombTickElement* this, float step)`, `RET 4`, body `006E1300`-`006E1500`. `this` is
the tick element at `record+3E4h`; every record field is reached by a negative displacement.
Coverage: **complete**.

1. `006E130B` If `[this+2Ch] < 0.0f`, `[this+2Ch] += step` and return. The compare is
   `XORPS XMM1,XMM1; COMISS XMM1,XMM0; JBE`, so it is a strict `0 > value` test. Nothing else in
   the tick runs while a bomb is still attached to its rack.
2. `006E1331` If the byte `[this+74h]` is clear, return.
3. `006E133B` `dt = classDesc[+5Ch] * step`. `+5Ch` is the class time scale, a constructor-only
   field defaulting to `1.0f` (`docs/WEAPON_CLASS_DESCRIPTOR.md`, `00D7A24C`). The scaled value
   overwrites the argument slot at `[ESP+20h]`, so every later step in the tick uses it.
4. `006E1344` `mode = shot->vtable[+2Ch]()` with `ECX = record+310h`. For the bomb, the depth charge
   and the rocket that slot is `006E2570`; the torpedo overrides it with `00855F00`.
5. `006E137D` `record->vtable[+19Ch](dt)` when `mode == 0`, `record->vtable[+1A0h](dt)` otherwise.
   The two slots are the whole motion model and they are what differs per kind:

   | kind | entity vtable | `+19Ch` (air) | `+1A0h` (water) | `+1A4h` |
   | --- | --- | --- | --- | --- |
   | MBomb | `00CF9438` | `006E1510` | `006E27F0` | `006E2800` |
   | MDepthCharge | `00CFBA80` | `006E1510` | `006FCD20` | `006E2800` |
   | MRocket | `00D090E8` | `0080A000` | `006E27F0` | `006E2800` |
   | MTorpedo | `00D0C3E8` | `006E1510` | `00857480` | `006E2800` |

   Read from `.rdata` at `00CF95D4`, `00CFBC1C`, `00D09284` and `00D0C584`. The rocket's and the
   torpedo's tables end at `+1A4h`: the dword after it is `42480000h` in the rocket's case and the
   start of the string `Homing...` in the torpedo's, so neither has a `+1A8h` slot.
6. `006E138D` When the byte `record+5Ch` is set:
   - `006E139B` If `[this+78h] >= 0.0f`, `[this+78h] -= dt` and the sweep is skipped this step. The
     comparison is against `00D7A218`, whose four bytes are zero. This is the arming delay, and it
     is a **time**, not a distance: nothing in the tick measures travel.
   - Otherwise `006E13C7` `record->vtable[+1A4h](&flagA, &flagB)`, then the sweep. `006E13D5` forces
     both flags to zero when `*(00E188A8)+1FE4h == 2`.
   - `006E13F9` Refresh the world pose through `00414DB0` when `record+C8h` is clear.
   - `006E145B` `0084C430` with eleven dwords, `RET 2Ch`, matching the bullet's call at `006E657F`:
     the previous snapshot `record+37Ch..384h` and the refreshed world translation `record+FCh..104h`
     by value, the shot interface `record+310h` (null-guarded by the `NEG/SBB/AND` idiom at
     `006E13ED`), the owner `record+3D8h`, the class descriptor `record+314h`, the global
     `00F87574` and `record+368h`, plus `CL = flagA` and `DL = flagB`.
7. `006E1464` When `*(00E188A8)+1FE4h != 2`: `flightTime = record+364h += dt`; if
   `flightTime > classDesc[+54h]` (`FlyTime`, default `00D7A248` = `FLT_MAX`), then
   - `006E1495` when `classDesc[+D4h]` (`TimeOutEfx`) is non-zero, `0042D7E0` for the world matrix,
     `00427EB0` for the refreshed position, then `0084B6F0(classDesc+D4h, position)`;
   - `006E14D2` `00696350(record, 0)` then `00926D90(record, 2)`, and return.
8. `006E14EA` `006E6900(record+310h, dt)`.

`006E2800` (`006E2800`-`006E280E`, `RET 8`, no Ghidra function) is
`void __stdcall(char* a, char* b) { *a = 1; *b = 1; }`. All four kinds use it, so the bomb family
passes `CL = DL = 1` outside world mode 2 and `CL = DL = 0` inside it. The bullet instead passes
`CL = (worldMode != 2)` and `DL = 1` (`docs/PROJECTILE_IMPACT.md`), so the bomb family is the only
one that suppresses the second flag in world mode 2.

`006E2570` (`006E2570`-`006E2578`, `RET`, no Ghidra function) is
`bool __thiscall(shot) { return shot[+44h] != 0; }`. `+44h` of the shot interface is the underwater
flag `docs/PROJECTILE_KINDS.md` already identified, set by the water-entry hook `vtable[+28h]`.

`006E6900` (`006E6900`-`006E6935`) is `__thiscall(shot, float dt)`: `shot[+94h] -= dt`, and once it
goes negative and `shot[+C8h]` is non-zero it calls `BSP_Observer_UnregisterPair` and clears
`shot[+C8h]`. That is the shooter's own-fire grace period draining; the pointer it releases is the
owner the sweep passes at `record+3D8h`.

## `006E1510` — the shared air flight

`__thiscall(record, float dt)`, `RET 4`, body `006E1510`-`006E163D`. Coverage: **complete**.
Used by the bomb, the depth charge and the torpedo for `+19Ch`; the rocket replaces it.

1. `006E151C` When the byte `classDesc[+20h]` (`NoGravity`) is clear:

   ```
   v.y -= 9.81 * dt                    ; 00CF9058, double 9.8103800773621
   v.x += a.x * dt                     ; a = record+348h..350h
   v.y += a.y * dt
   v.z += a.z * dt
   ```

   `v` is `record+318h..320h`. The three acceleration terms are inside the same branch, so a class
   with `NoGravity` set gets no acceleration at all, not just no gravity.
2. `006E1590` `inv = |v| > 0 ? 1/|v| : 0`, using `00419440` on a stack copy of `v`.
3. `006E15EC` The forward row of the local matrix is blended toward the velocity's vertical
   direction and nothing else changes:

   ```
   m.forward.y = 0.9f * m.forward.y + 0.1f * (inv * v.y)     ; 00D7A390, 00D7A3A0
   ```

   `m` is `record+74h`; `[ECX+20h]`, `[ECX+24h]` and `[ECX+28h]` with `ECX = record+74h` are rows
   `0`, `1` and `2`'s... in fact `+20h` is row 2, the forward axis, and `+24h`/`+28h` are its `y`
   and `z`. The `x` and `z` writes at `006E160D` and `006E1618` store the values just loaded, so
   they are no-ops the compiler emitted from a whole-vector assignment.
4. `006E1634` `0085DC80(record+74h)` rebuilds an orthonormal basis around the changed row. The same
   routine is the tail of `0085E880`, the axis rotation the torpedo uses.

The blend is a fixed-coefficient exponential filter with no `dt` term, so it is only frame-rate
independent because the tick element runs on the fixed `0.05f` step
(`docs/TICK_ELEMENT_OVERRIDES.md`). A bomb needs about seven steps, a third of a second, to close
most of the gap between its release attitude and its velocity direction.

`006E27F0` (one byte, `RET 4`, no Ghidra function) is the bomb's and the rocket's water advance: it
does nothing. Once a bomb is in the water its velocity is frozen; only the place-pose slot still
moves it.

## `006E1060` — the place pose

`__thiscall(this = record+3E4h, float step)`, `RET 4`, body `006E1060`-`006E1290`.
Coverage: **partial: `006E1236`-`006E127D`, the water-trail branch, is unread.**

`006E1067` rescales the step by `classDesc[+5Ch]` exactly as the tick does, and `006E1075` applies
the same negative launch-delay gate. Then:

- `006E108C` When `[this+74h]` is set, the same `shot->vtable[+2Ch]` query picks
  `record->vtable[+194h]` (mode zero) or `[+198h]` (otherwise) with the scaled step. This is the
  position integration pair, one slot below the tick's velocity pair, matching the bullet's
  `[+114h]`/`[+118h]` against its `[+11Ch]`/`[+120h]` (`docs/TICK_ELEMENT_OVERRIDES.md`).
- `006E10CB` The query runs a second time. On mode zero, and only when `record+338h` is non-null:
  `006E0CD0(effect, &lastPoint)`; `004842C0` sets the effect's world point to `record+FCh`;
  `d = worldTranslation - lastPoint`; and when `|d|^2 > 1.0e-4f` (`00D7A268`) the entity's world
  matrix is copied to the stack, its forward row is replaced with `d`, `0085DC80` re-orthonormalises
  it, its translation row is set to the world translation, and `0053D9C0` hands it to the effect.
  That is the in-air trail: a billboard oriented along the step's own displacement.
- `006E1236` The non-zero mode takes `record+334h` instead. Unread.

`0084C430`'s host row and the impact chain below it are `docs/PROJECTILE_IMPACT.md`'s; nothing in
`006E1300` calls the impact routine directly.

## `006E1010` — the commit, and the pose shift register

`__thiscall(this = record+3E4h)`, `RET`, body `006E1010`-`006E105F`. Coverage: **complete**.

```
record+37Ch..384h  <- record+370h..378h        ; previous <- current
record+370h..378h  <- [this+64h..6Ch]          ; current  <- the cached matrix's translation
if (record+C8h == 0) 00414DB0(record)
004134F0(this+34h, record+0CCh)                ; cache <- the world matrix
```

`004134F0`'s destination is `ECX` and its source is the pushed pointer; `00857518` in the torpedo
swim settles the direction, where the stack matrix is copied into `record+74h`. So the commit is a
three-stage shift register and the `current` snapshot the next sweep reads is the translation as of
the **previous** commit, not this frame's. The bullet's commit `006E7D50` takes `current` straight
from `record+FCh` instead (`docs/TICK_ELEMENT_OVERRIDES.md`), so the bomb family's sweep segment is
one commit longer than the bullet's.

## The depth charge: `006FCD20` and `006FD660`

`006FCD20` is `__thiscall(record, float dt)`, `RET 4`, body `006FCD20`-`006FD0C5`.
Coverage: **partial: the per-target block `006FCE8D`-`006FCF8B` is read only for its transform
calls; the tail after `006FD0AA` is unread.**

**Dive.** `006FCD28` reads `k = record+468h` and negates it once:

```
v.x += -k * v.x * dt
v.y += (-k * v.y - 9.81) * dt      ; 00CF9058 again, gravity is not switched off under water
v.z += -k * v.z * dt
```

Gravity stays on and the drag is linear on all three axes, so the charge settles at a terminal sink
rate of `-9.81 / k` metres per second. The producer of `record+468h` is **unread**: the create
`006FD210` only allocates `474h` bytes and memsets them, and neither `006FC960` nor the water-entry
hook writes it. `DiveSpeed` (`classDesc+DCh`) is the obvious source, as `k = 9.81 / DiveSpeed` makes
the terminal rate equal to it, but no code was found that computes it.

**Proximity scan.** `006FCDBD` gates the rest on `*(00E188A8)+1FE4h != 2` and on
`record+100h < -5.0f` (`00CFBC84`), so nothing is searched above five metres of depth. The list is
`[[record+30h]+7Ch]` walked through node `+4h` with the entity at node `+8h`. For each entity both
poses are refreshed, `distSq = |target+FCh - record+FCh|^2` is formed inline at `006FCE59`, and the
entity is skipped when `distSq > 625.0f` (`00CFBC80`, so a 25 m radius). Inside the hit branch
`00B63D50` builds the target's inverse transform into `target+110h` once (guarded by the byte
`target+10Ch`) and `004142E0` maps the charge's position into the target's local frame; what the
local point is then compared against, from `target+538h` on, is unread.

**Detonation.** `006FCFA6` fires when `record+100h < -|record+470h|` (the sign is stripped with
`AND 7FFFFFFFh`). `record+470h` is the charge's own detonation depth; its producer is unread, and
`DiveMinDepth`/`DiveMaxDepth` at `classDesc+E0h`/`+E4h` are the candidates. The blast itself is the
row `docs/EXPLOSION_RADIAL_DAMAGE.md` already records for `006FD032`: `0084BAD0` with `record+FCh`,
the blast descriptor `[record+314h]+70h`, a radius randomised by `00BD2F10(classDesc+B4h,
classDesc+B8h)` at `006FCFF9`, the flag `0`, the owner `[record+3D8h]` and the projectile. Then
`0078D1B0` projects the position onto the water surface, `record+C8h` is forced to 1 and
`record+10Ch` to 0.

`006FD660` (no Ghidra function, `006FD660`-`006FD743`, `RET 4`) is the depth charge's water-entry
hook, the shot interface's `vtable[+28h]`:

```
vtable[+20h](1.0f)
if (shot[+44h]) return
speed = 0042B2F0(shot+8h)
if (speed > classDesc[+ECh] || shot[+0Ch] < -classDesc[+F0h]) -> the break-up branch at 006FD746
006E6450(shot, ...)                                  ; the base water entry, sets +44h
if (classDesc[+E8h]) spawn WaterSplashEfx at record+CCh
```

`classDesc+ECh` is `MaxWaterHitVel` and `classDesc+F0h` is the field
`docs/WEAPON_CLASS_DESCRIPTOR.md` records as "computed, not read". `00855A90` shows what computes
it: `sqrt(2 * MaxFall * 9.81)`, the speed a body reaches after falling `MaxFall` metres. So the
second test is a drop-height limit expressed as an impact speed. The break-up branch at `006FD746`
is unread.

## The rocket: `0080A000`

No Ghidra function; raw listing `0080A000`-`0080A56x`, `RET 4`, SEH frame at `00C90928`.
Coverage: **partial: read to `0080A26A`; the trail block from `record+47Ch` on is unread.**

The rocket is the only kind that replaces the **air** slot, and it uses `006E27F0` (the no-op) for
water, so `0080A000` is its entire motion model.

**Ignition.** `record+494h` is the ignited flag and `record+490h` the timer.

- Before ignition (`0080A06E`): `pre = min(dt, classDesc[+E0h] - record+490h)` is the slice of the
  step still inside `IgnitionDelay`, and `post = dt - pre` the slice after it. The motor lights
  (`record+494h = 1`, then `shot->vtable[+3Ch]()`, which is `00809F20` for the rocket against
  `006E1CD0` for every other kind) when `min(record+490h, post)` is non-zero, or when `pre` is zero.
- `0080A100` `record+490h = pre + record+490h - min(record+490h, post)`.
- `0080A124` `v.y -= 9.81 * pre` and then `v.y += 9.81 * min(record+490h, post)`. The class's
  `NoGravity` flag is **not** consulted on this path. The second term cancels gravity again over the
  post-ignition slice, so a lit rocket is weightless while `record+490h` still has credit in it;
  whether that was the intent is not recoverable from the listing.

**Thrust.** `0080A14E` runs only once ignited:

```
f = record+0CCh row 2                     ; the world forward axis
spd = v . f
if (classDesc[+D8h] > spd) {              ; VMax
    spd' = min(spd + classDesc[+DCh] * dt, classDesc[+D8h])   ; Acceleration, then VMax again
    v += f * (spd' - spd)
}
```

So the rocket accelerates only along the axis it is pointing, only up to `VMax`, and the test is on
the axial component rather than the speed, so a rocket knocked sideways keeps accelerating forward.

## Host table

One row per native call site in the routines this packet read. `this` is `ECX`; `step` is always
the value already scaled by `classDesc[+5Ch]`.

| site | callee | name | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `006E1361` | `[[record+310h]+2Ch]` | `query_water_mode` | `record+310h` | `int` | byte `record+458h` set |
| `006E138B` | `record->vtable[19Ch]` / `[1A0h]` | `advance_in_air` / `advance_in_water` | `record`; `step` | - | the query's answer |
| `006E13D3` | `record->vtable[1A4h]` | `query_sweep_flags` | `record`; two `char*` | - | byte `record+5Ch` set, arming expired |
| `006E1407` | `00414DB0` | `refresh_world_pose` | `record` | - | byte `record+0C8h` clear |
| `006E145B` | `0084C430` | `sweep_step_segment` | eleven dwords, `RET 2Ch`; `CL`, `DL` | - | same; **contract**, `docs/PROJECTILE_IMPACT.md` |
| `006E14A9` | `0042D7E0` | `world_matrix` | `record` | ptr | timeout and `classDesc[+D4h]` set |
| `006E14BA` | `00427EB0` | `world_position_refreshed` | `record`; matrix`+20h` | ptr | same |
| `006E14C7` | `0084B6F0` | `spawn_timeout_effect` | `classDesc+0D4h`; position in `EDX` | - | same; **contract** |
| `006E14D2` | `00696350` | `expire_projectile` | `record`; `0`, `EDX = 0` | - | flight time over `FlyTime` |
| `006E14DB` | `00926D90` | `release_projectile` | `record`; `2` | - | same; **contract** |
| `006E14F4` | `006E6900` | `advance_owner_grace` | `record+310h`; `step` | - | not expired |
| `006E10A1` | `[[record+310h]+2Ch]` | `query_water_mode` | `record+310h` | `int` | byte `record+458h` set |
| `006E10C9` | `record->vtable[194h]` / `[198h]` | `place_pose_air` / `place_pose_water` | `record`; `step` | - | the query's answer |
| `006E1101` | `006E0CD0` | `trail_last_point` | `record+338h`; `float[3]*` | - | `record+338h` non-null; **contract** |
| `006E1129` | `004842C0` | `effect_set_world_point` | `record+338h`; `record+0FCh` | - | same |
| `006E11E5` | `0085DC80` | `orthonormalize` | the stack matrix | - | segment longer than `1e-4`; **contract** |
| `006E122F` | `0053D9C0` | `effect_set_world_matrix` | `record+338h`; the stack matrix | - | same |
| `006E1058` | `004134F0` | `cache_world_matrix` | `record+418h`; `record+0CCh` | - | none |
| `006E15BE` | `00419440` | `vector_length` | the stack copy of `v` | `float` in `ST0` | none |
| `006E1634` | `0085DC80` | `orthonormalize` | `record+74h` | - | none; **contract** |
| `006FCDD8` | `00414DB0` | `refresh_world_pose` | `record` | - | byte `record+0C8h` clear |
| `006FCEB0` | `00B63D50` | `build_inverse_transform` | `target+110h`; `target+0CCh` | - | byte `target+10Ch` clear |
| `006FCED3` | `004142E0` | `transform_point` | `record+0FCh`; out, `target+110h` | - | inside the radius |
| `006FCFF9` | `00BD2F10` | `random_blast_radius` | `classDesc+0B4h`, `classDesc+0B8h`; `ECX = 1` | `float` | at detonation depth |
| `006FD032` | `0084BAD0` | `radial_blast` | `record+0FCh`, `classDesc+70h`; radius, `0`, owner, record | - | same; `docs/EXPLOSION_RADIAL_DAMAGE.md` |
| `006FD08C` | `0078D1B0` | `project_to_water_surface` | `[[00E188A8]+19F0h]`; out, position, `record+370h` | ptr | same |
| `006FD686` | `0042B2F0` | `impact_speed` | `shot+8h` | `float` | `shot+44h` clear |
| `006FD6BF` | `006E6450` | `enter_water` | `shot`; the step | - | within both limits |
| `006FD715` | `00440490` | `effect_arg_pack` | `classDesc+0E8h`, world matrix | - | `WaterSplashEfx` set; **contract** |
| `0080A0FE` | `[[record+310h]+3Ch]` | `on_ignition` | `record+310h` | - | the motor lights this step |
| `0080A166` | `00414DB0` | `refresh_world_pose` | `record` | - | ignited, byte clear |

## Coverage

| routine | coverage |
| --- | --- |
| `006E1300` | complete |
| `006E1010` | complete |
| `006E1510` | complete |
| `006E27F0`, `006E2800`, `006E2570`, `006E6900` | complete |
| `006E1060` | partial: `006E1236`-`006E127D`, the water-trail branch, unread |
| `006FCD20` | partial: `006FCE8D`-`006FCF8B` read only for its transform calls; after `006FD0AA` unread |
| `006FD660` | partial: the break-up branch `006FD746`-`006FD89A` unread |
| `0080A000` | partial: read to `0080A26A`; the trail from `record+47Ch` unread |
| `0084C430`, `0084BF00`, `0084BC60` | reused from `docs/PROJECTILE_IMPACT.md` |
| `006E0CD0`, `0085DC80`, `00419440`, `0084B6F0`, `00696350`, `0078D1B0`, `0042B2F0` | `contract: unread` |

## Open questions

- The producer of the depth charge's `record+468h` drag coefficient and `record+470h` detonation
  depth. Neither the create, the constructor nor the water-entry hook writes them, and the record is
  memset to zero, so a path outside this packet must. With both at zero the charge free-falls and
  never detonates, which the game clearly does not do.
- Why the rocket adds gravity back over `min(record+490h, post)` after ignition. It reads as a
  cancellation with a credit that drains, but the intent is not recoverable statically.
- `record+348h..350h`, the acceleration `006E1510` adds inside the gravity branch, has no producer
  in anything this packet read.
