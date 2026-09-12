# The torpedo: swim, homing and the two vtable overrides

Addresses: 00855B00, 00855A90, 00855F00, 00857480, 00856BB0, 008568E0, 008561F0, 00521370,
0085E880, 004193E0

The shared tick `006E1300`, the air flight `006E1510` and the commit `006E1010` the torpedo inherits
are in `docs/BOMB_FAMILY_TICK.md`. The class family and the record sizes are in
`docs/PROJECTILE_KINDS.md`, the descriptor fields in `docs/WEAPON_CLASS_DESCRIPTOR.md`. This
document is what MTorpedo overrides and the swim model it reaches through those overrides.

## The three overrides, and which table each is in

MTorpedo replaces four slots across three vtables. `ghidra xrefs` does not see data references from
outside functions, so all four were read from `.rdata` bytes.

| table | address | slot | base | MTorpedo | what it is |
| --- | --- | --- | --- | --- | --- |
| tick element | `00D0C35C` | `+0Ch` | `006E1010` | `00855B00` | the commit |
| shot interface | `00D0C378` | `+28h` | `006E6450` | `008568E0` | water entry |
| shot interface | `00D0C378` | `+2Ch` | `006E2570` | `00855F00` | the water query the tick branches on |
| shot interface | `00D0C378` | `+38h` | `006E2910` | `008561F0` | not an override in behaviour |
| class descriptor | `00CFA56C` | `+0Ch` | `006E9890` | `00855A90` | finalise, caches two derived fields |
| entity | `00D0C3E8` | `+1A0h` | `006E27F0` | `00857480` | the water advance |

`008561F0` and `006E2910` have identical two-instruction bodies (`MOV AL,[ECX+148h]; RET`), so the
"override" is a duplicated inline accessor and changes nothing.

`00CFA56C` is the MTorpedo class-descriptor table, not a tick table: its `+1Ch` slot holds
`00856420`, the create `docs/PROJECTILE_KINDS.md` identified. It begins immediately after the
`MFlakBullet` string at `00CFA560`, which is why the packet brief's reading of it as a torpedo table
needed the `.rdata` check.

## `00855B00` — the commit override

`__thiscall(this = record+3E4h)`, `RET`, body `00855B00`-`00855B22`. Coverage: **complete**.

```
006E1010(this)                                   ; the base commit, unchanged
if ([this+0BCh] != 0)                            ; record+4A0h
    004134F0(this+0C0h, [this+0BCh] + 74h)       ; record+4A4h <- that object's local matrix
```

So the torpedo adds one thing to the shared commit: it mirrors the local matrix of whatever object
sits at `record+4A0h` into `record+4A4h`. The producer of `record+4A0h` is unread, and the consumer
of `record+4A4h` is not in anything this packet read; the natural reading is the launching tube or a
wake node whose pose the torpedo has to track, but that is a hypothesis.

`004134F0`'s destination is `ECX`: `00857518` copies a stack matrix into `record+74h` with the stack
pointer pushed, which settles the direction for both call sites.

## `00855F00` — the water query

`__thiscall(shot)`, `RET`, body `00855F00`-`00855F46`, no Ghidra function.
Coverage: **complete**.

```
if (record+0C8h == 0) 00414DB0(record)
if (2 * record+47Ch > record+100h) return 2
return record+354h != 0
```

`record+47Ch` is the swim depth, the field the Lua binding `008A2710`
`BSP_LuaBinding_SetTorpedoSwimDepth` writes (`docs/PROJECTILE_KINDS.md`), reached here as
`[shot+16Ch]`. `record+100h` is the world height and `record+354h` is `shot+44h`, the base's
underwater flag.

Both `006E1300` and `006E1060` test the answer with `TEST EAX,EAX; JNZ`, so `2` and `1` select the
same slot and the extra value is produced but never distinguished. Nothing this packet read consumes
it.

## `00855A90` — the class finalise hook

`__thiscall(classDesc)`, `RET`, body `00855A90`-`00855AF5`, no Ghidra function.
Coverage: **complete**.

```
ok = 006E9890(classDesc)                                       ; the base slot
classDesc+60h  = classDesc[+0E4h] * classDesc[+54h] * 0.6f     ; WaterTravelSpeed * FlyTime, 00CEFF98
classDesc+0ECh = sqrt(2 * classDesc[+0E0h] * 9.81)             ; MaxFall, 00CF9058, sqrt at 00BF7030
return ok && classDesc+0ECh > 0.0f
```

`classDesc+ECh` is the terminal speed of a fall of `MaxFall` metres, and it is what the water entry
compares the torpedo's vertical velocity against. The depth charge's `classDesc+F0h`, which
`docs/WEAPON_CLASS_DESCRIPTOR.md` records as "computed, not read" and attributes to
`LIBCRT_unmatched_00BF7030`, is the same formula over its own `MaxFall` at `+F4h`.

`classDesc+60h` is the run distance implied by the class: speed times fly time, times `0.6f`. What
reads it is outside this packet.

## `008568E0` — water entry

`__fastcall(shot)`, body `008568E0`-`00856B4x`. Coverage: **partial: read for the two limits, the
two branches and their effect ids; the effect construction from `0078D1B0` on is a contract.**

```
if (shot[+44h]) return
shot->vtable[+20h]()
speed = <length>
if (speed > classDesc[+0DCh] || shot[+0Ch] < -classDesc[+0ECh]) {
    if (classDesc[+24h]) spawn ExplWaterEfx at the surface point 0078D1B0 gives
    if (*(00E188A8)+1FE4h == 2) 006E6450(shot) else 00926D90(record)
} else {
    006E6450(shot)                                   ; sets +44h, swaps the trail effects
    if (classDesc[+0D8h]) spawn WaterSplashEfx       ; the torpedo's WaterSplashEfx
}
```

`classDesc+DCh` is `MaxWaterHitVel` and `classDesc+ECh` is the derived fall speed `00855A90`
cached, so the torpedo breaks up if it hits the water too fast **or** was released above `MaxFall`.
`006FD660` applies the identical rule to the depth charge with `+ECh` and `+F0h`. In world mode 2
the torpedo enters water instead of dying, so a replay or a spectator world keeps it alive.

## `00857480` — the swim advance

`__thiscall(record, float dt)`, `RET 4`, raw listing from `00857480`, no Ghidra function.
Coverage: **partial: `00857480`-`00857679` complete; the wake and sound tail from `00857679` is
unread beyond its two gates and the `008E6430` key.**

1. `0085748A` `record+488h += dt`. The run timer; nothing this packet read consumes it.
2. `008574A0` When `record+46Ch == 10000.0f` (`00D0C310`, tested with the
   `UCOMISS`/`LAHF`/`TEST AH,44h`/`JP` equality idiom `docs/TICK_ELEMENT_OVERRIDES.md` records),
   the pose is refreshed and `00521370(record+0ECh, &pitch, &record+46Ch)` seeds the commanded
   heading from the hull's own world yaw. `record+46Ch` is the commanded heading in radians and the
   sentinel is its uninitialised value.
3. `008574F0` The local matrix is copied to the stack, its forward row's vertical component is
   decayed and the basis is rebuilt:

   ```
   m.forward.y = m.forward.y * (1 - 3.0f * dt)      ; 00D7A2B0, a true double 3.0
   0085DC80(&m)
   004134F0(record+74h, &m)
   ```

   So a torpedo that enters the water nose-down levels out with a time constant of a third of a
   second. Unlike the bomb's blend this one does scale with `dt`.
4. `00857531` `00856BB0(record, dt)`, the steering step below.
5. `00857536` The local matrix is read back, because the steering rotated it, and the velocity is
   damped anisotropically:

   ```
   f    = m.forward
   par  = f * (v . f)
   perp = v - par
   v    = par * (1 - record+474h * dt) + perp * (1 - record+478h * dt)
   ```

   `record+474h` is the axial drag and `record+478h` the lateral drag. There is no thrust term here
   and none in `00856BB0`, so `WaterTravelSpeed` (`classDesc+E4h`) does not reach the swim: the
   torpedo coasts on the velocity it entered the water with, shedding lateral motion faster or
   slower than axial motion depending on the two coefficients.
6. `00857679` The tail is gated on the byte `00E0C978` and on `[[00F88C30]+130h]`, then calls
   `008E6430(0Eh, record)` and reads `record+470h`. `008E6430` is
   `BSP_GameplayModifiers_ProductForUnit`, so `0Eh` keys a wake or sound modifier. Unread past
   `008576D6`.

## `00856BB0` — steering: acquisition, homing, depth and heading

`__thiscall(record, float dt)`, `RET 4`, body `00856BB0`-`00857155`.
Coverage: **partial: complete except the candidate walk's list plumbing `00856C28`-`00856C63`, whose
container is a contract.**

### Acquisition

- `00856BC0` The whole homing block is gated on `classDesc[+0F0h] > 0.0f`
  (`HomingHorzTurnSpeed`). A class with no horizontal homing speed never scans and never takes a
  target; control drops straight to the depth and heading controllers.
- `00856BD6` `record+518h` is a countdown and `record+514h` the re-scan interval. When
  `dt >= record+518h` the scan runs and `record+518h += record+514h - dt`; otherwise `00856F5F`
  does `record+518h -= dt` and the scan is skipped.
- `00856BFC` The scan needs `record+3BCh`. It detaches the current observer with
  `006952A0(target, record+4FCh)`, then walks the list at `[[[record+3BCh]+54h] -> 008053C0]+0DE8h`
  through node `+4h` with the candidate at `[[node+8h]+4h]`. A candidate qualifies when
  `candidate->vtable[+5Ch](8)` is true. Among the qualifiers it keeps the one with the smallest
  `|candidate+FCh - record+FCh|^2`, measured with `004193E0`, which returns a **squared** length.
  The winner is stored at `record+510h` and bound with `00694A60(candidate, record+4FCh)`, so the
  pointer is cleared for the torpedo when the target dies.

### The two homing turns

- `00856D90` `00521370(record+0ECh, &ownPitch, &ownYaw)` decomposes the hull's world forward row.
  `00521370` is `__fastcall(row, float* outPitch, float* outYaw)`: the `EDX` output is
  `asin(clamp(row.y))` and the stack output is an `atan2`.
- `00856DCA` `d = normalize(target+FCh - record+FCh)` through `00419510`, then
  `00521370(&d, &targetPitch, &targetYaw)`, and `00856E25` sets `record+46Ch = targetYaw`. The
  commanded heading becomes the target's bearing even when the turn below is skipped.
- `00856E2D` The turns need `classDesc[+0F4h] > 0.0f` (`HomingVertTurnSpeed`),
  `target->vtable[+5Ch](8)` true a second time and `00852820(target)` false.
- `00856E6C` **The pitch turn.** `e = BSP_Math_SubtractWrappedAngle(ownPitch, targetPitch)`, clamped
  into `[-HomingVertTurnSpeed * dt, +HomingVertTurnSpeed * dt]` by two compares, then
  `0085E880(m, m.row0, e)` where `m = record+74h` and row 0 is the hull's right axis. `0085E880`
  (`RET 8`) rotates the matrix about the given axis and ends in `0085DC80`.
- `00856EE8` **The yaw turn.** A second `BSP_Math_SubtractWrappedAngle` call, negated, clamped into
  `[-HomingHorzTurnSpeed * dt, +HomingHorzTurnSpeed * dt]`, then `0085E880(m, m.row1, -e)` where row
  1 is the hull's up axis, and the function returns from `0085713F` without running either
  controller below.

  **Both calls read the same two arguments.** Taking `E` as `ESP` at `00856E6C`, the first pushes
  `[E+0Ch]` and `[E+8h]`; `0085E880` is `RET 8`, so `ESP` is `E-8` at `00856EE8` and the second
  pushes `[E-8+1Ch] = [E+0Ch]` and `[E-8+10h] = [E+8h]`. Those two slots are the `EDX` outputs of
  the two `00521370` calls, the hull's own pitch at `[E+0Ch]` (`00856D9B` computes `ESP+10h` after
  one push) and the target's pitch at `[E+8h]` (`00856E0C` computes `ESP+0Ch` after one push). The
  yaw outputs live at `[E+14h]` and `[E+10h]` and neither is read here. So the horizontal homing
  turn is driven by the **pitch** error, not by a bearing error. The reading is provisional only in
  what it means, not in what the listing says: it is reproduced as read in
  `src/bomb_torpedo_tick.cpp`, and the practical effect is small, because `record+46Ch` already
  carries the target bearing into the heading controller on every step the homing turn is skipped.

### Depth keeping

Reached from `00856F83`, that is on every path except the one where both homing turns fired.

```
surface = 0078CF20([[00E188A8]+19F0h], world.translation.x, world.translation.z)
a       = record+484h * surface - record+480h * (world.translation.y - record+47Ch)
v.y    += a * dt
v.y     = clamp(v.y, -5.0f, +5.0f)                ; 00CFBC84 and 00CE3850
```

`0078CF20` is `BSP_GameWorld_SampleWaterHeight`, so the controller holds the torpedo at
`record+47Ch` relative to the local wave surface, scaled by the ratio of the two gains. It is
proportional with no damping term; the clamp on `v.y` is what keeps it bounded.

### Heading

`00857061`, immediately after the depth block.

```
yaw  = atan2(world.forward.x, world.forward.z)             ; 00BF701A, the CRT atan2
e    = -BSP_Math_SubtractWrappedAngle(yaw, record+46Ch)
k    = (00E0C978 && [[00F88C30]+13Ch]) ? 008E6430(0Fh, record) : 1.0f
rate = classDesc[+0E8h] * pi / 180 * k                     ; HeadingTurn, 00CE3D28 / 00CE3D20
0085E880(m, m.row1, clamp(e, -rate, +rate) * dt)
```

`HeadingTurn` is therefore authored in **degrees per second**, and `008E6430` key `0Fh` is a
gameplay modifier on it. This is the yaw control that actually runs on the common path, and the
homing block feeds it by writing the target bearing into `record+46Ch`.

## Record fields

The torpedo record is `51Ch` bytes; `468h` of it is the bomb record. The tail:

| offset | type | meaning | native |
| --- | --- | --- | --- |
| `+46Ch` | float | commanded heading, `10000.0f` until seeded | `00857490`, `00856E25` |
| `+470h` | float | a wake parameter | `008576C6` |
| `+474h` | float | axial drag coefficient | `00857603` |
| `+478h` | float | lateral drag coefficient | `008575C4` |
| `+47Ch` | float | swim depth | `008A2710`, `00855F26`, `00856FE4` |
| `+480h` | float | depth gain | `00856FF2` |
| `+484h` | float | surface gain | `00856FF8` |
| `+488h` | float | underwater run time | `0085748A` |
| `+4A0h` | ptr | the object whose local matrix the commit mirrors | `00855B08` |
| `+4A4h` | 4x4 | that mirror | `00855B16` |
| `+4FCh` | observer | the homing target binding | `00856C12` |
| `+510h` | ptr | the homing target | `00856C09` |
| `+514h` | float | re-scan interval | `00856BEE` |
| `+518h` | float | re-scan countdown | `00856BD6` |

`+47Ch` is the only one with a known producer. `00856420` allocates `51Ch` bytes and memsets them,
`00856050` writes only subobject vtables, and `008568E0` writes none of these, so the producer of
the four coefficients and the scan interval is outside everything this packet read. With all of them
zero the torpedo would coast undamped at its entry velocity, never correct its depth and re-scan
every step, which the game does not do, so a producer exists.

## Host table

| site | callee | name | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `00855B03` | `006E1010` | `commit_base` | `record+3E4h` | - | none |
| `00855B1C` | `004134F0` | `mirror_local_matrix` | `record+4A4h`; `[record+4A0h]+74h` | - | `record+4A0h` non-null |
| `00855A95` | `006E9890` | `finalise_base` | `classDesc` | `bool` in `AL` | none |
| `00855AC4` | `00BF7030` | `sqrt` | `2 * MaxFall * 9.81` | `float` | none; CRT |
| `00855F15` | `00414DB0` | `refresh_world_pose` | `record` | - | byte `record+0C8h` clear |
| `008574C9` | `00414DB0` | `refresh_world_pose` | `record` | - | heading unseeded, byte clear |
| `008574DD` | `00521370` | `direction_to_pitch_yaw` | `record+0ECh`; `&pitch`, `&record+46Ch` | - | heading unseeded |
| `00857513` | `0085DC80` | `orthonormalize` | the stack matrix | - | none; **contract** |
| `0085751F` | `004134F0` | `set_local_matrix` | `record+74h`; the stack matrix | - | none |
| `00857531` | `00856BB0` | `steer` | `record`; `dt` | - | none |
| `0085753B` | `004134F0` | `read_local_matrix` | the stack matrix; `record+74h` | - | none |
| `008576A4` | `008E6430` | `gameplay_modifier` | `0Eh`, `record` | `float` | `00E0C978` and `[[00F88C30]+130h]`; **contract** |
| `00856C1C` | `006952A0` | `unbind_target` | `target`; `record+4FCh` in `EDX` | - | a target is held; **contract** |
| `00856C31` | `008053C0` | `recon_slot` | `[record+3BCh]+54h` | ptr | scanning; **contract** |
| `00856C63` | `[cand->vtable+5Ch]` | `target_is_homing_kind` | candidate; `8` | `bool` in `AL` | per candidate |
| `00856D24` | `004193E0` | `distance_squared` | a stack `float[3]` | `float` in `ST0` | a target is held |
| `00856D59` | `00694A60` | `bind_target` | candidate; `record+4FCh` in `EDX` | - | the candidate wins; **contract** |
| `00856D9F` | `00521370` | `direction_to_pitch_yaw` | `record+0ECh`; `&ownPitch`, `&ownYaw` | - | a target is held |
| `00856E02` | `00419510` | `normalize` | the target delta | - | same |
| `00856E14` | `00521370` | `direction_to_pitch_yaw` | the delta; `&targetPitch`, `&targetYaw` | - | same |
| `00856E4F` | `[target->vtable+5Ch]` | `target_is_homing_kind` | target; `8` | `bool` in `AL` | `HomingVertTurnSpeed > 0` |
| `00856E5F` | `00852820` | `target_suppresses_homing` | target | `bool` in `AL` | same; **contract** |
| `00856E7E` | `00438B10` | `subtract_wrapped_angle` | `ownPitch`, `targetPitch` | `float` in `ST0` | same |
| `00856EE3` | `0085E880` | `rotate_about_right` | `record+74h`, `record+74h`; axis, angle | - | same |
| `00856EFA` | `00438B10` | `subtract_wrapped_angle` | the same two slots | `float` in `ST0` | same |
| `00857149` | `0085E880` | `rotate_about_up` | `record+74h`; `record+84h`, angle | - | reached from the homing tail and the heading tail |
| `00856F7A` | `0085E880` | `rotate_about_right` | `record+74h`; `record+74h`, `ownPitch` | - | a target but no vertical turn |
| `00856FC6` | `0078CF20` | `sample_water_height` | `[[00E188A8]+19F0h]`; `x`, `z` | `float` in `ST0` | not returned from the homing tail |
| `00857081` | `00BF701A` | `atan2` | `forward.x`, `forward.z` | `float` in `ST0` | same; CRT |
| `00857092` | `00438B10` | `subtract_wrapped_angle` | `yaw`, `record+46Ch` | `float` in `ST0` | same |
| `008570B8` | `008E6430` | `gameplay_modifier` | `0Fh`, `record` | `float` | `00E0C978` and `[[00F88C30]+13Ch]` |

## Coverage

| routine | coverage |
| --- | --- |
| `00855B00`, `00855A90`, `00855F00` | complete |
| `00857480` | partial: `00857480`-`00857679` complete, the wake tail unread |
| `00856BB0` | partial: complete except the candidate list's container `00856C28`-`00856C63` |
| `008568E0` | partial: the two limits and both branches; the effect construction is a contract |
| `00521370`, `004193E0` | complete |
| `0085E880` | partial: `RET 8`, rotates about the given axis and ends in `0085DC80`; the rotation body is unread |
| `0085DC80`, `008053C0`, `00694A60`, `006952A0`, `00852820`, `008E6430` | `contract: unread` |

## Open questions

- The producer of `record+474h`, `+478h`, `+480h`, `+484h`, `+514h` and `+4A0h`. `008A2710` is the
  only writer found for any of the tail, and it writes `+47Ch` alone.
- Whether the yaw turn at `00856EE8` reading the pitch slots is a defect in the original. The
  argument slots are established above; what is not established is whether the resulting behaviour
  was ever noticed, since `record+46Ch` carries the bearing into the heading controller anyway.
- What consumes the value `2` from `00855F00` and the cached range at `classDesc+60h`. Neither
  appears in anything this packet read.
- Whether the arming delay `record+45Ch` is ever set to something other than zero for a torpedo;
  `docs/BOMB_FAMILY_TICK.md` establishes it as a time, so there is no arming **distance** in the
  tick.
