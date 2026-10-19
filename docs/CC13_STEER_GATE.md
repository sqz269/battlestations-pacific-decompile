# CC13 steer gate: the gun controller's commanded direction (packet cc13-steergate)

Addresses: 009FC7C0 (gate 009FCB70-009FCCB4, init 009FC829-009FC871, tail 009FCE5A-009FCE69),
009FABE0 and its 28 call sites (table in section 2), 0099B630, 009A1A20, 0099D0A0, 009FAAD0.
Files: `include/bsp/cc13_steer_gate.hpp`, `src/cc13_steer_gate.cpp`, `src/game_hosts_units.cpp`.
Switches (both OFF): `bsp::kBotCommandedDirection68Bound`, `bsp::kSteerGateCommandedDirectionBound`
(the second `static_assert`s the first). Names are hypotheses. Status: reconstructed, build-tested;
no game run was made for this packet.

## 1. The gate in the image

`009FC7C0` (`__thiscall(gun = task+314h, float dt)`, `RET 4`), run from `BSP_PilotBot_Update`
at `00999979` after the task arm, while unit+C24h is set:

1. `009FC829`-`009FC855`: gun+68h, +6Ch, +70h are each compared with 0 (`UCOMISS`, `LAHF`,
   `TEST AH,44h`, `JP`). Only when all three compare equal (a NaN counts as non-zero) does
   `009FC857` call `0042D7E0` and copy row 2 of the unit's world matrix (+20h..+28h, the
   forward) into +68h..+70h (`009FC85C`-`009FC871`).
2. `009FCB70`-`009FCB92`: u = world lead delta / d, each component `FDIV` then `FSTP` float.
3. `009FCB96`-`009FCBAF`: dot = (+6Ch * u.y + +68h * u.x) + +70h * u.z, `FSTP` float;
   `009FCBB3`-`009FCBBB`: `FLD1`/`FSUBRP`, 1 - dot, `FSTP` float to `[ESP+1Ch]`.
4. `009FCCA0`-`009FCCAB`: `FLD [ESP+1Ch]`, `FLD gun+40h`, `FCOMPI`, `JBE` skips the steer
   (so an unordered compare fails). `009FCCB1`: `COMISS 0, +4Ch`, `JB` skips while +4Ch > 0.
   The pass arm runs `009FA7E0` and `009F9FC0` (the fine aim) and sets +49h.
5. `009FCE5A`-`009FCE69`: `MOVSS` of xmm0 to +70h, +6Ch, +68h and +40h. xmm0 is 0 on every
   path into `009FCE26` (`XORPS` at `009FC7C3`, `009FC85F`, `009FC9FE`, `009FCC2D`,
   `009FCDA6`, `009FCE07`, `009FCE23`; the `1.0` loaded at `009FCD1B` is overwritten at
   `009FCDA6` before the join). The two early returns to `009FCED9` (gun+0 == 0,
   unit+2F4h == 0) skip the clear.

So +68h is a per-think value: a state tick that runs before the gun tick stores it, the gate
reads it, the tail clears it. When no state stored one, the gate measures against the nose.

**Before the first 009FABE0.** The gun constructor `009FAAD0` (`009FAAD0`-`009FAB58`) writes
+28h..+50h and +74h but never +68h..+70h. With zeroed memory the first tick therefore takes the
forward; this packet did not check whether the owning task's allocation is zeroed (open). The
host's `GameUnitSlot` is value-initialised for each spawn (slots are appended at
`host.slots.push_back`, never reused), so the host's first read is all zero, i.e. the forward.

**Precision.** The gun tick runs on the main thread (24-bit precision control,
docs/CC13_FPU_PRECISION.md), so each x87 step rounds to float; the C++ uses float operations in
the image's order. At 53 bits the dot's two sums and the 1 - dot would round once more each.

## 2. Every writer of gun+68h

A rel32 scan of the whole image (`tools/image_ref_scan.py 009FABE0`, which covers undefined
code too) finds 28 `CALL 009FABE0` and no absolute references. Every caller is a bot state
tick; each loads ECX = `[approach+1Ch]` (the gun block) and stores the cone to `[approach+1Ch]+40h`
just before. The other +68h writers are the gun tick itself (`009FC862` init, `009FCE64` clear);
the remaining disp8 +68h stores in the bot ranges (`tools/store_census.py 0x68`) are to approach
objects, not the gun block.

| call | state tick | heading argument | pitch argument | cone | host |
|---|---|---|---|---|---|
| `007B4D72` | rocket go-towards 007B4980 | local | 0099B630 | +678h | no task |
| `007B5237` | rocket attackrun 007B4F60 | 009A1A20 | 0099B630 | +40h store at 007B5214 | no task |
| `007B575B` | rocket aim 007B5430 | 009A1A20 | local | +678h | no task |
| `007B65B1` | rocket goaway 007B6240 | 009A1A20 | state+20h | +674h | no task |
| `009A38FC` | depth-charge attackrun 009A3770 | 009A1A20 | 0099B630 | +670h | DC `apply_plan` |
| `009A3F14` | depth-charge goaway 009A3E70 | second 009A3CF0 | 0099B630 | +674h | DC `apply_plan` |
| `009A747C` | dogfight attackrun 009A71E0 | 009A1A20 | 0099B630 | +670h | not hosted: cone not delivered |
| `009B08E7` | land line 009B0300 | local | 0099B630 | +66Ch | not hosted: cone not delivered (Angle_Prepare 0) |
| `009B1239` | land standby 009B0FE0 | 009A1A20 | 0099B630 | +66Ch | not hosted, same |
| `009B5993` | level-bomb goaway 009B5760 | local (= plan+2C0h) | 0099B630 | +674h | hosted |
| `009BCB46` | move-on-path circle 009BCA50 | 009A1A20 | 0099B630 | +670h | not hosted: cone not delivered |
| `009BFD59` | follow fly-to 009BEE30 | 009A1A20 | 0099B630 | +670h | hosted |
| `009C1B5B` | moveto 009C18C0 | 009A1A20 | 0099B630 | +670h | hosted (strafe, level-bomb, dive-bomb, torpedo and generic moveto) |
| `009C25AD` | moveto task 009C2430 | 009A1A20 | 0099B630 | +670h | hosted |
| `009C285F` | moveto circle 009C26D0 | 009A1A20 | 0099B630 | +670h | hosted |
| `009C4471` | dive-bomb attackrun 009C4220 | 009A1A20 | 0099B630 | +670h | hosted |
| `009C4E58` | dive-bomb goaway 009C4A40 | state+1Ch | 0099B630 | +674h | hosted |
| `009C705D` | dive-bomb flyabove 009C62B0 | local | 0099B630 | +66Ch | not hosted: cone not delivered |
| `009C9872` | retreat moveto 009C9310 | local (= plan+2C0h) | 0099B630 | +670h | hosted |
| `009C993D` | retreat enter zone 009C98C0 | 009A1A20 | 0099B630 | +66Ch | hosted |
| `009C9A56` | retreat leave 009C9990 | local (= plan+2C0h) | 0099B630 | +66Ch | hosted |
| `009CAC62` | strafe go-towards 009CA870 | local (= plan+2C0h) | 0099B630 | +678h | hosted |
| `009CB085` | strafe attackrun 009CADB0 | 009A1A20 | 0099B630 | +670h | hosted |
| `009CB4E0` | strafe aim 009CB1B0 | 009A1A20 | local atan2 (009CB2A2) | +678h | hosted |
| `009CBE5B` | strafe goaway 009CBB30 | 009A1A20 (plan+2CCh = 1) | state+20h | +674h | hosted |
| `009D0B02` | torpedo attackrun 009D07B0 | 009A1A20 | 0099B630 | +670h | hosted |
| `009D114A` | torpedo goaway 009D0F10, arms 1-2 | state+18h | [approach+8]+1ECh | +674h | hosted |
| `009D11E4` | torpedo goaway 009D0F10, arm 3 | state+18h | 0099B630 | +670h | hosted |

19 of the 28 sites are hosted. The 9 that are not either have no host task (rocket) or sit
in states whose cone the host does not deliver, so their gate stays shut whatever +68h holds
(the cone is 0 and `0 > 1 - dot` holds only through rounding). The dogfight attackrun's cone
`009A744E`/`009A7459` (Angle_MoveTo) is a separate gap: `DogfightConeState` names it but the
host's dogfight gate uses Angle_Strafe in aim only.

The two argument helpers:

* `0099B630` (`__thiscall(cmd)`, ST0): cmd+2D0h != 0 ? cmd+2BCh : unit+C64h. Host:
  `GameUnitsHost::Impl::commanded_pitch_0099b630` (plan_state.pitch_mode_2d0, pitch_target_2bc,
  plane_pitch_angle_c64).
* `009A1A20` (`__fastcall(ECX = cmd)`, ST0): cmd+2CCh == 2 ? cmd+2C0h : `00438AA0(unit heading,
  0099D0A0(cmd, unit+C68h))`. Host: `commanded_heading_009a1a20`, which builds the 0099D0A0
  inputs the way the planner's roll arm does (class rates, `caps_rate_at_one` for kinds 10h/16h,
  the four HdgDiffCalc tunings) but with the raw bank and the class TurnRoll (`0099D19E` reads
  class+25Ch itself; the leader's +260h is chosen by the planner at `0099DF9A`, not by 0099D0A0).
  Only the strafe goaway reaches this arm among the hosted sites (it leaves plan+2CCh = 1);
  the census counts it as `heading_fallbacks`.

`009FABE0` itself: `bot_commanded_direction_009fabe0` runs the existing
`bsp::cc13_seg68::bot_set_direction_from_heading_pitch_009fabe0` (re-verified against the
bytes: 00CE3830 and 00CE3828 hold the float values of pi/2 and 2pi as doubles, the
`FLDZ`/`FCOMPI`/`JBE` at `009FABF9` adds 2pi only when 0 > a, the length is inverted only when
positive) with `00412E20` = `native_projection_tangent_00412e20` and `00419440` as the float
length. Its third GunHost member (`009FA620`) is never called by 009FABE0 and aborts.

## 3. The host

* **Container.** `GameUnitSlot::gun_direction_68[3]`, beside `gun_cone_40`. Filled only by
  `GameUnitsHost::Impl::gun_direction_009fabe0` / `gun_direction_from_plan_009fabe0` at the
  19 hosted sites, each placed after the host's own plan heading and pitch writes for that tick
  (the image steers before its cone store; several host ticks write the cone first), and only
  when `kBotCommandedDirection68Bound`.
* **Clear.** At the gun tick's tail with the cone (`009FCE5A`-`009FCE64`), in the
  `kTaskGunControllerAllTasksBound` block after `df_gun_tick_009fc7c0`. The host clears every
  think, also for units whose gun tick did not run (C24h clear, not eligible); the image
  keeps +68h and +40h until a gun tick that reaches `009FCE26` (see uncertainties). Nothing
  else moves or clears it: the slot is never reused, the plane state machines only write it,
  and the dogfight arm never writes it, so a dogfight unit always reads zero (the forward).
* **Read.** `df_fine_aim_009f9fc0`: when any component is non-zero (as `009FC829`), the world
  lead delta is rebuilt from the unit-frame lead through the normalised world rows that
  `df_local` projected on (SUBSTITUTION, labelled: the host keeps the lead in the unit frame),
  u = delta / d per component, and `steer_gate_one_minus_cos_009fcb96` gives the gate input.
* **OFF identity.** Every store, the clear, the census and the summary lines are inside
  `if constexpr (bsp::kBotCommandedDirection68Bound)`; the gate change is inside
  `kSteerGateCommandedDirectionBound`. The `record(...)` calls at the five sites that had them
  become `done(...)` only when the store is on. With both OFF the build is behaviourally and
  log-identical.
* **Prediction mode.** `kBotCommandedDirection68Bound` ON with the gate OFF stores and counts but
  still gates on the forward: the run is the OFF trajectory, and the census reports which
  verdicts the direction would flip. Lines: `  steer gate <unit> writes= gates= to_pass= to_block=
  steer_ticks=`, `  steer gate site <009FABE0 call> writes= gates= to_pass= to_block=`, and
  `summary mission steer gate: direction_writes= heading_fallbacks= direction_gates= to_pass=
  to_block= gate_reads_direction=`.

## 4. Composition with kDogfightFireEnvelopeBound

Both live in 009FC7C0 but at different points. The envelope (`df_fire_envelope_009fc7c0`)
decides the fire request and whether 007B96D0 asks for a re-aim; the re-aim (`009FCDD0`
`CMP [ESI+49h],0`) only runs when this tick's steer gate passed. The host passes `reaim` into
`df_fine_aim_009f9fc0`, which returns before the re-aim when the gate fails, and the gate's
cone is the one this packet changes. So with both ON the re-aim follows the commanded-direction
gate, as the image does; with the envelope OFF the re-aim is not modelled and only the steer
changes. Neither switch reads the other's state. Built with the full `./scripts/build.ps1`, all
with `kDogfightFireEnvelopeBound` ON (its value on main): store/gate OFF/OFF, ON/OFF
(prediction mode) and ON/ON. No build was made with the envelope switch OFF.

## 5. Expected ON effect (no runs made)

The cones are radians compared with 1 - cos: Angle_Prepare DEG(0) never passes, Angle_MoveTo
DEG(2) passes within 15.2 degrees, Angle_GoAway DEG(10) within 34.4 degrees, Angle_Strafe
DEG(15) within 42.4 degrees (planeglobals.lua:453-456).

The structural change: OFF, a steer turns the nose onto the lead, so the forward-based gate
keeps passing while the target lasts (the steer holds itself open). ON, the gate is anchored
to the state's commanded direction and shuts once the lead leaves the cone around it, handing
the sticks back to the state. The fine aim writes plan+2D4h = 0 and the yaw/pitch sticks, so
every steer tick is a tick the state's own heading did not fly.

Current-main OFF logs (steer_ticks from `summary mission task gun cones`):

* **USNOS 4489** (`cc9-physics1/local/p23e_off_os.log`). Strafe aim 3969 ticks; in every strafe
  squadron steer_ticks equals aim ticks except Midway_sqn30 (aim 360, steer 336). In aim with
  no aircraft found, the gun's lead is the aim point (009CB4B1) and +68h is the direction to that
  same point (009F9E40 heading and the atan2 pitch from the same position), so 1 - dot is about
  0 and the gate passes on every aim tick with +4Ch <= 0: unchanged, at most +24 at
  Midway_sqn30. Strafe goaway (4682 ticks) and moveto/follow show no steers OFF; ON the goaway
  direction is the turn-ahead heading (0099D0A0 arm), so no change expected unless an enemy
  aircraft sits ahead. The per-unit lines sum to 4480 of the 4489: strafe aim accounts for
  3936 (of 3969 aim ticks), and Essex_sqn21 and Essex_sqn21|.-III, dive bombers whose only cone state was moveto
  (moveto=1751, flyabove=21), for 544. Those 544 moveto steers are the USNOS rows ON changes;
  see BSM09 for the moveto mechanism.
* **BSM01 1125** (`cc9-lua52/local/l69r_b1c_main_ref.log`): strafe aim 1125 = steer 1125. Same
  as USNOS aim: no change expected; goaway 2270 ticks, no steers OFF.
* **BSM09** (`cc9-physics1/local/p23j_off_b9_def.log` 1729; the later `p24_off_b9.log` 1125):
  no strafe task. Dive-bomb squadrons steer in **moveto** (e.g. Zuikaku_sqn05 cone_ticks =
  moveto ticks = 1545, steer 125; Yorktown_sqn02 1987/131). ON, the moveto gate is the 15.2
  degree cone around the bearing to the target and the 009FB800 cruise pitch: steer runs on
  escorts end once the fighter leaves that cone, so expect fewer, shorter steers, fewer
  fire_ticks, and planes holding course to the target (earlier arrival in the flyabove/aimglide
  chain, so AA exposure moves). Torpedo moveto/attackrun the same, goaway 34.4 degrees.
* **USN21 808, IJN08 131, USN04 10, USN01 1**: the same moveto/goaway logic on fewer steers;
  small changes.

**Goaway rows (for cc9).** The states whose gate changes in goaway are the level-bomb goaway
`009B5993`, the dive-bomb goaway `009C4E58`, the strafe goaway `009CBE5B`, the torpedo goaway
`009D114A`/`009D11E4` and the depth-charge goaway `009A3F14`. OFF, a goaway plane that finds an
enemy aircraft within 34 degrees of its nose can be held on it by the self-sustaining steer; ON,
only while the aircraft stays within 34 degrees of the away heading, i.e. the goaway turn cannot
be captured. Expect equal or shorter goaway durations where a steer was active. The logs do not
split steers by state for these (the `task gun` lines print only planes with bursts, and the
level bombers print none); run prediction mode for the per-site to_block counts.

## 6. Uncertainties

* Gun ticks the image skips (C24h clear, or the early returns) keep the previous +68h and +40h
  in the image; the host clears both every think. A state change in such a think would let the
  image's next gun tick read a stale direction once.
* Whether the task allocation that holds the gun block is zeroed (the image's first-tick value
  when no state wrote +68h) was not checked.
* The world lead delta is rebuilt from the unit-frame lead (rounding differs from the image's
  float subtraction at `009FCAE5`-`009FCB09`).
* `[approach+8]+1ECh` (torpedo goaway arms 1-2) is taken as the host's `plane_climb_angle_1ec`.
* 0099D0A0's planner-filled inputs (pitch command slot) are read at state-tick time.
* The ON effect sizes are reasoning from the OFF logs, not measurements.
