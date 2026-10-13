# What the image does to an aircraft that reaches the water

Addresses: `007CB7F0` (BSP_Plane_OnWaterContact), `007BC5B0`, `007BBFA0` (the plane's named-effect
handler, vtable `00D19D28 + 194h`), `007CBA50` (BSP_Plane_WaterSurfaceStep), `007DCDD0`
(BSP_PlaneFlight_WaterSurfaceLaw), `007CE040` (`007CE313`-`007CE3AC`, the depth kill), `006DF520`.

Packet `cc9_water_surface_law`, 2026-09-23. Ghidra was read, not written. Every pair has
`BSP_GUNNERY_RNG_STREAMS=1` on both sides.

## 1. The case

In the 9000-frame USN04 reference (`local\base2_e9000.log`, commit `635e7b27b`), eight aircraft
touch the water. Seven were already dead in the gunnery host when they touched. Only
D3A Val #1.1|.-4 was alive:

| unit | contact | mission time | health at the end | died |
| --- | --- | --- | --- | --- |
| D3A Val #1.1|.-4 | frame 4833 | 241.65 s | 190 | never |
| D3A Val #1.1|.-2 | frame 5125 | 256.25 s | 0 | 248.81 s |
| movieval|.-2 | frame 5285 | 264.25 s | 0 | 246.11 s |
| D3A Val #1.1 | frame 5416 | 270.80 s | 0 | 156.95 s |
| movieval | frame 5780 | 289.00 s | 0 | 210.51 s |
| D3A Val #5.1|.-4 | frame 6432 | 321.60 s | 0 | 295.65 s |
| D3A Val #5.1|.-2 | frame 6724 | 336.20 s | 0 | 295.90 s |
| D3A Val #5.1 | frame 7015 | 350.75 s | 0 | 265.21 s |

The live Val left its dive at 280.5 m at about frame 3163 and reached the water 83.5 s later at
68.28 m/s. That is a sink of about 3.4 m/s, a glide angle of 2.8 degrees and an up-axis height
`unit+E0h` of about 0.9988. It touched as a nearly level glide, not as a failed pull-out.

The host then set the flight state to 6. Its surface arm `007CBA50` is a counter, so the Val stopped
being integrated. It sat at the surface as a live, stationary AA target for the rest of the run
and drew most of the 291712 negative-vertical halvings.

## 2. The read

### 2.1 `007CB7F0`, the contact handler, from the listing

The Ghidra pseudocode names its first flag `MinWaterSpd != 0`. The listing says the opposite:
`007CB81A UCOMISS [desc+198h], 0.0 (00D7A218)`, `LAHF`, `TEST AH,44h`, `JP` then `MOV BL,1`. That sets
`BL` when **MinWaterSpd is zero**. Both readings give the same early return; they differ only in
how the later branch is named, so the listing is the one used here.

1. `AL = 007BC5B0(unit)`. If `AL` is false and `BL` is clear, meaning MinWaterSpd is non-zero,
   **return at `007CB9C2` with nothing done**: no effect, no award, no state change.
2. The `SA_OC` award `0090F6C0(unit, 3)` on the scoring object `[00E188A8]+21A0h`, only for a
   plane of kind `14h` in a player slot (`unit+1B0h < 8`) that is alive and slower than 8.0f
   (`00CE3918`) with `unit+9F0h < 0.1` (`00D05E28`). It is scoring only; `0090F6C0` grants
   `"SA_OC"` through `BSP_MissionScoring_GrantAward` and applies no damage.
3. With `BL` set (MinWaterSpd zero), an up-axis height `unit+E0h` above 0.8 (`00CE3D40`, double)
   skips to the tail. Otherwise, and in net modes other than 2, the handler raises `"powerlost"`
   (`00D059F8`) through `vtable[194h]`.
4. The tail `007CB92C`: state 7, 4 or 5 becomes 6, or the `0C3h` session message on the authority.

`007BC5B0` is true when any of these holds:
- `unit+61h` is set, in which case it returns `unit+9E2h`;
- the state is 7, 6 or 4, `unit+AA0h <= 0`, and either `unit+150h <= 0` (no health left) or the
  owning slot `unit+1ACh` is not 8 and `BSP_PartySlot_IsAiHeld` answers false (a human's plane).

**For an AI aircraft with health left and a non-zero MinWaterSpd, touching the water does
nothing.** This installation's D3A Val row (`VehicleClass[158]`) has `MinWaterSpd = 22.222221`
and `SwimHeight = 0.1`, so the live Val takes the early return at every contact. It stays in
state 7, and the free-flight arm keeps integrating it below the surface.

### 2.2 What `"powerlost"` is

`007BBFA0`, the plane's `vtable[194h]`, compares the name. For `"powerlost"` it stores 1000.0
(`00CE3804`) at `unit+C10h` and routes session message 102 (`007BA1C0`). This installation's
`planepartclasses.lua` names `'PowerLost'` as one of a shot-down aircraft's death modes, with
`PowerLostExplosionDelay = {4, 8}`. So raising it is a death. That handler was not read further.

### 2.3 `007CBA50` and `007DCDD0`, the state-6 arm

For an aircraft that does reach state 6 (a dead one, or one with MinWaterSpd zero):
- **Impact.** The impact test raises `"splash"` for a dying aircraft (`unit+5Dh` set) that
  arrives too fast, too steep or too banked.
- **The law.** `007DCDD0` is a buoyancy law. The listing reads `desc+198h` at `007DCDEE`
  (`MOVSS XMM1,[EAX+198h]`, `UCOMISS XMM1,XMM0` against zero, `LAHF`, `TEST AH,44h`,
  `JP 007DD349`), which takes the three-point hull
  sample only when MinWaterSpd is zero. It computes the depth below the swim line
  `water + SwimHeight - altitude` and a wave-surface normal. It calls `007DB680` with
  `ctl+FCh = 2` (the water arm `007DC205`-`007DC68C`, still partly unread at
  `007DC426`-`007DC64A`). It applies a buoyancy force along the normal through `007DAFD0`, a
  righting term when `ctl+94h` is set, and a velocity damping below StallSpd. It returns the depth.
- **Exits.** A depth above `Water/MaxDepth` (6.0) times a speed blend raises `"powerlost"`.
  When the plane rises 1.5 m (`00CE3D78`) above the line, the arm requests state 7 again.

### 2.4 `007CE040`, the depth kill

At `007CE313`-`007CE3AC`, in net modes other than 2 and with `unit+61h` clear, the plane tick
compares the altitude `unit+100h` with a limit. The limit is 30.0f (`00CE38C8`), or the float at
`+0Ch` of what `(unit+360h)->+160h->+0Ch->vtable[48h]()` returns when that chain exists. When the
altitude falls below minus the limit, it calls `BSP_MissionEntity_Kill(unit, 1)` (`007CE3A7`).

This is what ends an AI aircraft that flies into the sea. **The host lacks it**, which is why
planes used to fly to -400 m before the state-6 contact was added.

## 3. The missing term

The host sends every aircraft that touches the water to state 6 and freezes it there. The image
does that only for a dead aircraft, a human's aircraft, or a class with MinWaterSpd zero. A live
AI Val keeps flying under the surface, and the tick kills it once its origin is 30 m down, unless
its flight takes it back up first.

**Quantified on the reference log.** One aircraft, D3A Val #1.1|.-4, is affected. It went into
state 6 at 241.65 s instead of continuing its 3.4 m/s glide. At that rate it would pass -30 m
about 9 s later, near 250.6 s. From then until frame 9000 (450 s) it stayed a live target at the
surface, about 200 s of AA fire.

## 4. The binding (`kPlaneWaterContactGateBound`, units host water arm)

- **Contact gate.** `007CB7F0` step 1: the state change happens only when `007BC5B0` is true or
  the class's MinWaterSpd is zero.
  - **`007BC5B0`, SUBSTITUTED.** It is true when the gunnery host has the aircraft dead, which
    stands for health `unit+150h <= 0`. `unit+61h` has no writer, so it is taken as clear.
    `unit+AA0h` is not carried and is taken as 0. Every aircraft is treated as AI-held, because
    this process flies no human-piloted aircraft.
- **Depth kill.** `007CE3A7` runs for a state-7 aircraft after its water test, with the 30.0f
  limit and cause 1, through the gunnery host's kill funnel.
  - **SUBSTITUTION:** the scene-handle chain's limit is not carried, so this uses the image's own
    30.0f fallback.
  - **SUBSTITUTION:** the image runs the check at the top of every tick for every flight state.
    The host runs it after the free-flight step, for state 7 only. A state-6 aircraft is frozen at
    the surface and a ground aircraft never goes below -30 m, so this makes no difference here.
- **Not bound.** The water law, the state-6 motion and its exits are not bound (section 2.3). The
  seven dead aircraft keep the host's state 6.

## 5. Predictions, written before the pair

USN04, 9000 mission frames, with the switch off and then on.
- **The Val.** It stays in state 7 after 241.65 s, sinks, and is killed at -30 m. If it keeps its
  3.4 m/s sink, that is near 250 s, within 30 s of contact. `sunk_at` goes from -1 to about 250,
  and the Kill counts as a death credited to the funnel's last attacker. Deaths go 32 to 33.
- **Halvings.** Most of the 291712 halvings come after 241 s against the frozen Val. They fall by
  at least 200000.
- **Category 1.** Shots fall by several thousand and hits fall by several hundred, because the
  zero-damage friendly-hull hits around the Val stop. Damage dealt changes little.
- **The seven dead aircraft.** Their contact rows do not change: `007BC5B0` is true for them, so
  they still take state 6.
- **Dive-bomb rows.** No release or dive row moves. The Val's bombs were released at about
  159 s, long before the contact.
- Guns that stop firing at the Val retarget, so other category 1 rows move as a consequence.

## 6. The pair with the stream option on

The treatment is `local\wT_9000.log` and the control is `local\wC_9000.log`, both on this tree
(main `942f47987` plus this packet). The two logs are identical apart from the log path on
line 25. Every row is the same and `water depth kills=0`.

**The Val prediction failed, for a reason outside the binding.** With the per-consumer streams,
D3A Val #1.1|.-4 is shot down at 179.36 s, 62 s before it reaches the water. All eight contacts
are therefore dead aircraft (`local\contacts.py`). For a dead aircraft `007BC5B0` is true, and
state 6 is the image's own answer. The halvings are 10216 in this world, not 291712. The
live-Val case exists only with the shared stream, as in the reference runs.

**A second pair without the option, written before its runs.** This is labelled as a departure
from the standing rule, because the case under test does not occur with the option on. The
logs are `local\wC0_9000.log` and `local\wT0_9000.log`. Main has moved since `635e7b27b`, so
the prediction depends on who is alive at contact:
- If D3A Val #1.1|.-4 touches the water alive, as in the reference, the treatment logs `plane
  water contact ignored` for it. It stays in state 7, and it is killed at -30 m within 30 s, or
  it climbs away. The halvings then fall by more than 200000, and category 1 hits fall by several
  hundred.
- If it is dead by then, the two logs match, as they do with the option on.

## 7. Secondary: the artillery bot's aim point, and the barrel count

### 7.1 `006DF520` step 4, read from the listing

- **Refresh.** At `006DF6D7`-`006DF6F5` the refresh countdown `bot+B4h` falls by the step. When
  it goes negative, the bot takes a row `R = [00E19990] + bot+34h * 1Ch`. The row index is the
  skill (`LEA EAX,[ECX*8]`, `SUB EAX,ECX`, `ADD EAX,EAX` twice). `[00E19990]` is a BSS pointer
  filled at run time.
- **The call.** The countdown is reloaded from `R+14h`. At `006DF762`-`006DF76A` the bot calls
  `target = bot->vtable[44h]()`, then `target->vtable[100h](out, &(c,c,c), &[00F87574],
  R+18h, R+1Ch, R+20h, R+24h)`, where `c` is 0.6f (`00CE3D30`). The answer is stored at
  `bot+A8h..B0h`.
- **The aim point.** It is that section point transformed by the target's pose `+CCh`, plus the
  error offset `bot+90h..98h`. The offset steps toward `bot+84h..8Ch` at `step * 30.0`
  (`00CE7630`).
- **The lead.** `t = |aim - shooter| / (V0 * cos(elev) * [round+5Ch])`, where `elev` is half the
  clamped asin of `g*R/V0^2`. The aim point moves by `t` times the target's velocity. The
  vertical term is scaled by the constant at `00D7A258`.

The host aims the artillery bot at the Height-raised visibility point, so both the section
point and this lead are missing. Binding them needs `vtable[100h]` of the ship classes and the
`[00E19990]` table, and neither is read. USN04 has no enemy ship, so an artillery-bot pair
there moves nothing. **Not bound; the read above is the deliverable.**

### 7.2 The barrel count

`docs/GUN_MOUNT_POSITIONS.md` section 5 already has the image's rule. `gun+448h` is seeded at
`0072E71A` from `0072AB80(gunClass)`, `max(1, muzzle count)`, where the muzzles are the
model's `"fire"` nodes. The host's flatten counts `Bullet` records instead (`bn` in
`src/game_hosts_gunnery.cpp`). That is wrong both ways: a dual-purpose mount with two ammunition
records reads as two barrels, and a twin mount with one record reads as one. The fix needs the
model's `"fire"` node count, which the host does not load. **Not bound.**

### 7.3 The seven unread `00901C20` callers

Ghidra lists these seven call sites. This is not a census.
- `00526F27` in `00526A40`: no Ghidra caller.
- `005270FA` in `00526A40`: the same function's second call.
- `00640B15` in `00640A20`: called by `00642040 BSP_InGameHudMarkers_BuildUnitMarker`, so a
  HUD lead marker.
- `00902379` in `00902290`: called by `00957740 BSP_Aim_ResolveRayToWorldPoint`.
- `00957A00` in `00957740 BSP_Aim_ResolveRayToWorldPoint`: the player aim path.
- `00957CA6` in `00957BD0`: called by `00547480` and `00959C20 BSP_Unit_ApplyGunAimMessage`.
- `0070C680` in `0070C370 BSP_FlakProjectile_TickAdvance`: the flak round's own tick, which has
  no Ghidra function and was read from bytes.

## 8. The run without the option, and the decisions

**Control: no log.** The control `local\wC0_9000.log` died with 0xC0000005 at 12:26 local time,
right after the renderer init request, while the session was on Remote Desktop (`rdp-tcp#0`
active). As instructed, it was not retried.

**Treatment: `local\wT0_9000.log`, complete.** D3A Val #1.1|.-4 is shot down at 179.31 s on this
tree even without the option, 62 s before it reaches the water. Main has moved since
`635e7b27b`. All eight contacts now carry the new gate inputs, and all eight are dead aircraft:

| unit | MinWaterSpd | dead | up-axis height |
| --- | --- | --- | --- |
| D3A Val #1.1|.-4 | 22.222 | 1 | 0.9933 |
| D3A Val #1.1|.-2 | 22.222 | 1 | 0.9934 |
| movieval|.-2 | 22.222 | 1 | 0.9988 |
| D3A Val #1.1 | 22.222 | 1 | 0.9933 |
| movieval | 22.222 | 1 | 0.9933 |
| D3A Val #5.1|.-4 | 22.222 | 1 | 0.9933 |
| D3A Val #5.1|.-2 | 22.222 | 1 | 0.9934 |
| D3A Val #5.1 | 22.222 | 1 | 0.9933 |

No contact was ignored, and `water depth kills=0`. Neither new code path ran, so a control
would match it. The halvings are 10175 and `queued_hits` is 392, against 291712 and 1494 in
the `635e7b27b` reference. **The frozen live Val no longer occurs on current main.**

**Decisions.**
- **`kPlaneWaterContactGateBound`: landed, default true.** The rule is read from the listing. Its
  substitutions are labelled. It moves no row in either run on this tree, because every aircraft
  that reaches the water here is already dead, and state 6 is the image's answer for a dead one.
- **`kPlaneDepthKillBound`: landed, default true.** It is the image's only end for a live AI
  aircraft under the sea, and it did not fire here.
- In the `635e7b27b` reference, the gate would have sent the live Val on in free flight. Its fate
  there, a kill at -30 m or a climb away, was not measured.

## 9. Open items

- **Dead aircraft fly on.** In the reference, the seven dead aircraft touched the water 7 to
  114 s after their deaths. So the host keeps a shot-down aircraft in normal flight. The image
  gives it a death mode, `'Explosion'`, `'Spinning'` or `'PowerLost'`, with
  `ExplosionExplosionDelay = {0.6, 1.8}`, `SpinExplosionDelay = {4, 12}` and
  `PowerLostExplosionDelay = {4, 8}` in `planepartclasses.lua`. The `"explosion"` fuse at
  `007CBFF0` then ends it.
  - This is the end-of-aircraft term the host lacks. It is not the water.
  - It matters for AA: a dead aircraft's path still feeds other guns' target choices until it
    hits the water, even though the gunnery host marks it dead.
  - Read next: `007BBFA0`'s message 102 handler, and the death-mode selection.
- **The state-6 motion is unbound.** That is `007DCDD0` past the sampling, `007DB680`'s water arm
  (`007DC426`-`007DC64A` still unread), and `007CBA50`'s exits. A dead aircraft in state 6 is
  frozen at the surface.
- **The depth-kill limit.** The `(unit+360h)->+160h->+0Ch->vtable[48h]()` chain was not read; 30.0f
  stands in for it.
- The artillery section point, the barrel count and the seven intercept callers are in section 7.

## 10. The state-6 arm, read whole (packet `cc9_plane_water_law`, cc9-physics4, 2026-10-12)

Ghidra read only. Every routine below was read from the listing, with the pseudocode as a
cross-check; `tools/x87_dc_form_scan.py` over all three bodies finds one DC-form multiply,
`007DCF89 DC CA` (ST2 = ST2 * ST0), and it is transcribed as such. Rules in
`include/bsp/plane_water_law.hpp` / `src/plane_water_law.cpp`. Names are hypotheses.

| routine | what it is | coverage |
| --- | --- | --- |
| `0078D3D0` (RET 0Ch) | `n = normalize(b x a)`, `a = (1, h(x+1) - h(x-1), 0)`, `b = (0, h(z+1) - h(z-1), 1)` (`004F9B30` with EDX = b at `0078D4C0`): `normalize(-dhx, 1, -dhz)` over 2 m | complete |
| `007DCDD0`-`007DD45F` | the sample. MinWaterSpd zero: the origin and body points (8, 0, 0) / (-8, 0, 0) (`00CE3918`, `00CE3CC8`); a tip higher than the origin by `d`, `c = clamp(4d, 0, 1)` (double `00D7A328`), pulls the normal's horizontal part away from it by `max(cos(heading to tip - heading of n), 0) * |n_xz| * c`, headings `pi/2 - atan2(z, x)` wrapped to [0, 2pi). MinWaterSpd non-zero: the origin only. `depth = SwimHeight + h - y`; the normal is renormalised | complete |
| `007DC205`-`007DC68C` | the water band of `007DB680`, below | complete, `007DC426`-`007DC64A` included |
| `007DD48E`-`007DD9AA` | the tail: the LiftRotate kick, the `ctl+94h` nose-down kick, `ctl+8Ch = 0`, the sub-StallSpd angular damping | complete |
| `007DAFD0` (RET 14h) | an **angular** kick on the world angular velocity `ctl+24h`, not a force: `axis = normalize(dir x R(point))` (`unit+674h` when shorter than 0.001), and per component `w += step * accel * axis` until `w` reaches `1000 * accel * axis` | complete |
| `007DA3D0`-`007DA53F` | `007DA380`'s mode-2 factors: `outA = IC(MaxCtrlAngle, 1, MinCtrlAngle, 0, up.y) * authority`; `outB = YawControlFactor * (1, or below NormalYawControlSpd: IC(0, 0, 0.8 MaxYawControlSpd, throttle > 0 ? MaxYawControl : 1, v), or with throttle above MaxYawControlSpd: IC(1.25 MaxYawControlSpd, MaxYawControl, NormalYawControlSpd, 1, v))`; byte 0 | complete |
| `007CBC36`-`007CBD6D` | the lift-off byte: the plane stays while `-depth < desc+508h + 1.5`; above that, with `unit+770h`, it stays while any `desc+5BCh` hull point is under the water | complete |

**The water band (`ctl+FCh == 2`).** `fade = max(1 - DeadMeat * 0.5, 0)`, `s = 1 - clamp(unit+90Ch / 10, 0, 1)`:
- `L`: under the line, `clamp((depth + 1)^LiftDepthRatio, 1, LiftMax)`; above it,
  `max(0, depth / (desc+508h * LiftBeginDiveMul) + 1)^2`.
- `D = min(m * vy^2 * DecelSpeed, 6.0)` while `unit+ACCh < 0`, `m` 1 under the line and `L` above.
- `dyn+28h += AccelCheatMul * 9.81 * L * s * fade * n` (buoyancy along the normal);
  `dyn+10h += AccelCheatMul * 9.81 * D * s * fade * n` (the sink brake).
- Above a world speed of 0.01: `depth' = depth (+0.5 when MinWaterSpd != 0)`,
  `H = desc+508h` (`max(2, H)` when `depth' > 0`), `r = clamp((H + depth') / H, 0, 1.8)`; with
  `r > 0`, `r = 1 - (1 - r)^2` below 1, `dyn+34h += r * max(speed, MinDragSpd)^2 * WaterDecel *
  (vx, 0, vz) / speed` (the third fold carries it into `dyn+40h`), and
  `dyn+04h += clamp(-sgn(ctl+3Ch) * r * ctl+3Ch^2 * SideDragRatio, +-MaxSideDrag)`.

**The tail.** `f = (depth + 1)^LiftRotateDepthRatio` under the line, else
`max(0, depth / (desc+508h * LiftBeginDiveMul) + 1)^2`; times `|2 - R(0, cos WaterPitch,
sin WaterPitch) . n|^LiftRotateAngleRatio`, capped at LiftRotateMax; one `007DAFD0` kick of
`WaterRotAccel * f` about `n x R(v)`, `v = (0, cos WaterPitch, sin WaterPitch)`; since `0085E4D0`
turns the pose about `-w` (`include/bsp/plane_angular_velocity.hpp`), that turns `v` toward `n`.
With `ctl+94h`: a kick of `min(2|roll|, 1) * max(1 - |pitch|, 0) * IC(0, 3, 83.33, 16, horizontal
speed)` on the nose `(0, 0, 1)` against world down. Then below StallSpd,
`q = IC(0, 0, StallSpd, 1, ctl+44h)` and each component of `ctl+24h` loses `(1 - q^2) * step` of
itself, never past zero.

**`ctl+94h` has a producer after all.** `007D8180` (`ctl+94h = 1` on the authority, net modes 0
and 1) is called from `007CBC02` for a dying plane (`unit+5Dh`) whose byte is still clear, which
then sets `unit+C10h = 99.0` (`00D059A0`); also from `007CE1D9` (ECX = `unit+7A0h`), `007BBB01`
and `007D1449`, not read here. So on the water a wreck takes the nose-down kick and the core law's
second gravity term (`007DB9E7`-`007DBA2F`).

**`desc+508h`** is `-[desc+50h]+2Ch - SwimHeight` (`007C4CF8`-`007C4D03`): `[desc+50h]` is the
class model and `+28h..3Ch` its BoundingBox (min xyz, max xyz; `docs/UNIT_HULL_EXTENTS.md`), so it
is the belly's depth below the origin less the swim height.

## 11. The binding (`kPlaneWaterSurfaceLawBound`, OFF) and the predictions

`src/game_hosts_units.cpp`, `PlaneBinding::water_surface_step_007cba50`, in `007CBA50`'s order: the
pilot pass (the gate `0099AE5F` admits state 6) and `007C5AC0`; step 3 (`ctl+94h` and
`unit+C10h = 99.0` for a dying plane); the sample; `run_core_law_007db680` with `ctl+FCh = 2`
(the water band replaces the ceiling, no turbo, `ctl+94h` drives `007DB9E7`'s second gravity
term, `007DA380`'s mode-2 factors in the rate law, the tail on the angular velocity before the
pose turns); the lift-off byte; the drown test; the accumulator; the state-7 request. The class
fields are read once per class and logged (`plane water class ...`), `desc+508h` from the Mesh
BoundingBox.

SUBSTITUTIONS, labelled in the source: the `"splash"` effect (presentation) is not raised; the
hull list `desc+5BCh` and `unit+770h` are not carried, so the lift-off is the 1.5 m test alone;
vtable `+204h` is the world speed; `"powerlost"` is counted, not raised; the `C3h` request is
applied at once; the host keeps the body angular rates as its state, so the tail goes through
the pose rows to the world vector and back; `unit+674h` (the degenerate-axis fallback) is pose
row 0. The free-flight contact line keeps its origin-crossing substitution (`[EDI+4]` = 0, section
4); with `desc+508h` now known that line is a separate, unbound change.

**Can a floating wreck touch gameplay?** Every aircraft that reaches state 6 on the logs on disk is
dead (section 10's reach). Its kill was credited and its death mode chosen when it died; the
gunnery host's targeting and kill funnel skip a dead unit; and the water arm kills nothing (no
depth kill in state 6: a live state-6 plane has MinWaterSpd 0, so `unit+90Ch` never grows and its
buoyancy holds). The only live state-6 entry on disk is JM08's scripted "Movie Mavis".

**Predictions, written before the pair** (USN01 `p3_u1_z4`, a win; ESMP08 `l46_e8_orders2`):
- pair_diff verdict 1 (gameplay identical) on both; the win holds on USN01; ESMP08 stays as it is
  on main (a loss on 355b60167).
- Moved rows confined to the wreck: its pose after contact, `plane water law` lines (ON only).
  A wreck on the water has `DeadMeat > 2 s` at once, so `fade = 0`: no buoyancy and no sink brake;
  it sinks under gravity (and `007DB9E7`'s extra term) and leaves the log's surface rows.
- No `plane water lift-off` for a dead plane; `plane water powerlost` lines possible once a wreck
  passes the 6 m threshold, counted only.

## 12. The pair, and the flip (cc9-physics4, 2026-10-12)

Reference form, `--stop-after-end 40`, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`; OFF is the tree
build at `1809e5771`, ON its `pair_export` flip (`local\p4_wl`). Logs `local\p4w_{off,on}_{u1z4,e8w}.log`.

| Row | OFF | ON | pair_diff |
| --- | --- | --- | --- |
| USN01 `p3_u1_z4` | completed 1014.63 s, guard | completed 1014.63 s, guard | gameplay identical (90 deaths, 3965 hits, 19 water contacts) |
| ESMP08 `l46_e8_orders2` | no end (lost, as on main) | no end | gameplay identical (803 deaths, 10815 hits, 310 water contacts) |

What moved is the wrecks only: `007DA710` runs for state-6 aircraft (ESMP08 6.67M -> 19.37M calls),
their poses turn (`heading_change` 3440 -> 111118 rad over the mission: the nose-down kick and the
LiftRotate kick spin a sinking wreck), and every one of the 310 ESMP08 wrecks passes the drown test
(`plane water powerlost`, counted). No `plane water lift-off`. Class values as logged: Jill
`desc+508h` 1.744, Kate 1.417, Corsair 1.421, Zero 0.641.

Predictions against the result: verdict 1 on both, held; the USN01 win held; wrecks sink, held - and
further than predicted: with no depth kill for a dead plane in this host, a wreck sinks without
limit (one summary row reads a land-task plane at -40558 m). The image's depth kill would call
`BSP_MissionEntity_Kill(unit, 1)` on it at -30 m; on a unit that is already dead that is a
presentation question here (the host's kill funnel skips dead units), recorded as open.

**Flipped ON** by the verdict: the mechanism is the listing's and nothing scored moved.

**Answer to "can a floating wreck touch gameplay?"** Not on these runs: every death, hit, damage and
unit row is identical with the wrecks moving. Targeting skips dead units, the kill was credited at
death, and the water arm itself kills nothing.

## 13. The wreck's floor (`kPlaneWreckDepthRemovalBound`, OFF)

Section 12 left a wreck sinking without limit. The image has a floor: the depth kill `007CE313`-`007CE3AC`
runs at the top of every tick in every flight state, and below -30 m it calls `00926D90
BSP_MissionEntity_Kill(unit, 1)`. Kill returns only when `+5Fh` is already set; a shot-down plane reaches
`+5Fh` only through the explosion path (`007D0CFD`), so a PowerLost or delayed wreck is killed there: `+5Fh`
set and the entity queued on `00F899B4` for `009273A0`'s on-killed dispatch, which takes it out of the
world. The host already removes an exploded plane that way (`plane_death_removed`, skipped by the step),
so the binding sets that flag for a dead state-6 plane below -30 m (the 30.0f fallback, as at the state-7
gate) at the head of the arm. Predictions: gameplay identical (the wreck was already dead and credited);
`plane wreck depth kill` lines ON only, one per sinking wreck; no -40 km summary rows.

**The pair** (OFF = tree build at `67e99782c`, ON = its flip; `local\p4r_{off,on}_e8.log`): ESMP08 no end on both
sides (lost, as on main), gameplay identical (803 deaths, 10815 hits, 908 units). All 310 wrecks log `plane wreck
depth kill` and leave the world; the plane fixed step falls from 19.38M to 6.69M calls, near its level before the
water law, since sunk wrecks stop stepping. **Flipped ON.**
