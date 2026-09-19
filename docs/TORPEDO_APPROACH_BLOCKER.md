# USN01's torpedo aircraft fly into the sea, and that is the blocker

Packet `cc8_torpedo_approach_blocker`. Addresses: `009D3420` (the approach update), `009D3489`
(the altitude-floor store and its gate), `009D4850` (the per-tick arm), `009D4A70` (the cruise
profile), `009D15F0` (the aim tick), `009D3210` (the engaged predicate), `009D4923` (the
manual-release test this host hard-codes to false), `009C8920` (the dive bomb's cruise profile, the
analogue that does write `+398h`).

The stream arrived here from `docs/PLANE_SQUADRON_HOST.md` section 4.2, which established what the
blocker is **not**: it is not the squadron's member array. `007C0D90` has exactly one caller and
`007EEF30` exactly one, so the release-order broadcast is unreachable until an aircraft has already
requested a release.

## 1. The finding

**All five Mavs fly into the water at about 141 m/s, roughly eleven seconds into their run.** The
log line is the same for each of them:

```
plane water contact: unit=Mav1 alt=-4.13 water=0.00 |v|=141.12 state 7 -> 6 (007CB7F0 tail 007CB92C)
plane water contact: unit=Mav2 alt=-5.05 water=0.00 |v|=140.95 state 7 -> 6
plane water contact: unit=Mav3 alt=-5.88 water=0.00 |v|=141.07 state 7 -> 6
plane water contact: unit=Mav4 alt=-5.05 water=0.00 |v|=140.96 state 7 -> 6
plane water contact: unit=Mav5 alt=-0.01 water=0.00 |v|=141.05 state 7 -> 6
```

Every symptom the stream has been chasing is downstream of that one fact, and none of them is a
separate defect:

| symptom | why |
| --- | --- |
| `approach ticks=124`, `arm_ticks=124` against `stage_ticks=3000` | `run_torpedo_task_arm_009d4850` returns before its counter when `command_target_plus_one == 0`; the aircraft is in the water by then |
| `min == last == 3928.5 m`, "never closes" | it closed 330.7 m (`ordered Mav1 range 4259.2 -> 3928.5 m ... heading error 1.785 -> 0.000 rad`) and then stopped, because it was dead |
| `aim_ticks=0` | the in-range latch wants `range < 2200 m` and the aircraft never gets there |
| `release_arm_009D2287=0`, `timer_009FA3A0` never on | the aim state never runs |
| `requests_007BBBA0=0`, `C20h_left=0`, `issue gate 007EEF40` unevaluated | nothing ever requests a release |

The heading error going to **0.000 rad** is worth stating separately: the aircraft was steering
correctly at the target. This is not a navigation failure.

## 2. The proximate cause: a commanded run-in altitude of 12 metres

The run's own descent census, on Mav1:

```
torpedo Mav1  descent census n=1    base=12.00 (74h=0.00 78h=12.00) commanded=12.00 live_alt=800.0 pitch_demand=-1.0472 pitch=0.0000
torpedo Mav1  descent census n=51   base=12.00 (74h=0.00 78h=12.00) commanded=12.00 live_alt=703.7 pitch_demand=-1.0472 pitch=-0.6198
torpedo Mav1  descent census n=101  base=12.00 (74h=0.00 78h=12.00) commanded=12.00 live_alt=272.3 pitch_demand=-1.0472 pitch=-1.0218
dive probe Mav1  alt=62.0 spd=140.52 pitch=-1.0396 path=-1.0312
plane water contact: unit=Mav1 alt=-4.13
```

The commanded altitude is **12 m**, the pitch demand is `-1.0472 rad`, which is exactly `-pi/3`, and
the aircraft holds it from 800 m all the way down. It is flying the command it was given.

`docs/TORPEDO_APPROACH_UPDATE.md` records that the aim tick reads `+78h + +74h` at `009D1631` as the
floor on the commanded altitude. Here that sum is `0.00 + 12.00`:

* `+78h` is `12.0`, `TorpReleaseAlt` from the `SPNormal` robots row
  (`kTorpReleaseAltSPNormal` in `include/bsp/torpedo_release_spawn.hpp`). That is a **release**
  altitude and is the right value for what it is.
* `+74h` is **0.00**, and that is the defect.

## 3. Why `+74h` is zero: a gated store the host's substitution cannot pass

> **WITHDRAWN.** This section's conclusion - that the zero in `+74h` is the defect - does not
> survive reading `009C89CE`. See the Correction section below before using anything here. The
> mechanism it describes is accurate; what is withdrawn is calling it the root cause.

`009D3489`-`009D34AE`, already reconstructed in `src/torpedo_approach_update.cpp`:

```
009d3489  FLD   float ptr [ECX + 0x398]
          FCOMIP against the double 100.0 at 00D7A220
009d34ae  JBE   ... skips the store
```

so `alt_floor_74 = ctl->+398h` **only when `+398h < 100`**. The host feeds that field
`kPilotTorpedoCruisingAltDefault = 500.0f` (`read_control_block` in `src/game_hosts_units.cpp`,
whose own comment says it is passing `Pilot/Torpedo/CruisingAlt`, the `+394h` key). 500 is not below
100, so the store never runs, `+74h` keeps the zero the constructor left, and the band collapses to
the 12 m release altitude.

**The substitution has no producer behind it.** `009D4A70`, the cruise profile the host cites, only
**reads** the field - `009D4B57 FLD float ptr [EAX + 0x398]` - and the whole routine contains no
store to `+398h` (its only float store in that range is `009D4BA7 MOVSS [ESI+484h],XMM0`, the engage
distance). So `+398h` is filled by something else, and the host guessed the wrong key for it.

## 4. Where to look next, and the bound on the negative

> **SUPERSEDED.** `009C89CE`, which this section names as the next step, has since been read, and
> what it says withdrew section 3. The census below and its warning about `0079CD26` still stand;
> the recommendation at the end of the section does not. Section 8.5 carries the live open list.

An exhaustive census of stores to `+398h` (`python tools/store_census.py 0x398`) finds **40** sites.
Two matter:

* `009C89CE MOVSS` in `009C8920 BSP_BotTaskDiveBomb_UpdateCruiseProfile` - the dive bomb's cruise
  profile, which is the direct structural analogue of the torpedo's `009D4A70` and **does** write the
  field. Start here: whatever tuning key it takes is very likely the same key the torpedo path wants,
  and `docs/DIVE_BOMB_TASK.md` may already name it.
* `0079CD26 MOVSS` in `FUN_0079CBD0`, which `docs/PLANE_SQUADRON.md` establishes is a **different
  object** - the `0x410` block allocated at `0079CC2D`, not the receiver the approach reads. Do not
  adopt it; it is the same trap that packet bounded for `+390h`.

The census covers twelve store forms in disp8 and disp32; it does not cover a 16-byte
`MOVAPS`/`MOVUPS` store reaching `+398h` from a lower offset, and `0079CD08`..`0079CD36` shows that
this field family is written one lane at a time on at least one class.

**The test that settles it**: any `+398h` below 100 m makes the store fire and the band becomes
`that value + 12 m`, which is a torpedo run-in altitude rather than a dive into the sea. The fix is
one field in `read_control_block`, and it must come from the producer, not from picking a number
that passes the gate.

## 5. Two things that are not the fix

**`009D4923`.** `docs/TORPEDO_TASK_ARM.md` section (1) step 8 is the manual-release passthrough, and
it runs `007BBBA0(unit)` only when `task+424h > 0`, `task+3FCh != 0`, the object embedded at
`unit+72Ch` answers its `vtable[+38h]` (that call is `009D4923`) **and the state is none of `aim`
`+710h`, `attackrun` `+6B4h`, `prepare` `+740h`**. USN01's Mavs spend all 124 of their arm ticks in
`attackrun`, so step 8 is skipped by its own state test whatever `009D4923` answers. Replacing the
hard-coded `false` is still worth doing and is not by itself a route to the first drop here.

**The two thresholds**, which the packet brief conflated:

| test | routine | threshold | USN01 |
| --- | --- | --- | --- |
| the task-level engaged predicate | `009D3210` | `task+484h * 2.2` at `009D324F` = 4840 m | satisfied at 4174.3 m |
| the in-range latch `+131h`, attackrun to aim | `009D3420`, `009D35D4`..`009D361E` | `range < approach+8Ch`, the run prints `8Ch=2200.0` | never, `min = 3928.5` |

## 6. Retraction

An earlier draft of this document predicted that the arm stopped at 124 ticks because the Mavs'
command row stopped being `current`, so `command_target_plus_one` went to zero on its own. **That is
wrong and is struck.** The command target does go to zero, and `run_torpedo_task_arm_009d4850`'s
first line does return on it, but the cause is the water contact, not a stale order. The mechanism
was inferred from the host sources before the log was read; reading the log is what corrected it.

## 7. Status

Sections 1 to 3 are measured, and measured twice on two different binaries.

| | `local/sqn_after_usn01.log`, on `b00c3cd24` | `local/tap_before_usn01.log`, on `b882aa1d4` |
| --- | --- | --- |
| water contacts | all five Mavs, `alt` -4.13 / -5.05 / -5.88 / -5.05 / -0.01, `\|v\|` 140.95 to 141.12 | identical, to the centimetre and the hundredth of a metre per second |
| Mav1 descent census | `base=12.00 (74h=0.00 78h=12.00) commanded=12.00 live_alt=800.0 pitch_demand=-1.0472` | identical |
| Mav1 approach | `ticks=124 replans=13 aim_ticks=0 engage 8Ch=2200.0 min=3928.5` | identical |
| Mav1 issue stage | `stage_ticks=3000 waiting=3000 issues=0` | identical |
| pilot attack | `ordered=5 range_first_mean=4174.3 closed_mean=368.1` | identical |
| `EXITCODE` | 0 | 0 |

`b882aa1d4` is this branch with `origin/main` merged, so it carries the dive bomb's `cmd+2CCh`
planner change and this lineage's plane-squadron wing; neither moves any of these numbers. Both runs
are `--frames 3200 --press-start-frame 30 --menu-select USN01 --mission-frames 3000
--mission-frame-seconds 0.05`, and `query session` was `console Active` before each.

Section 4 is a census plus a recommendation and is **not** yet a reading of `009C8920`. Nothing is
fixed: no code in this packet has changed, and `local/tap_before_usn01.log` is the same-binary
before column for whoever makes the change.

## Correction to section 3, made before the fix was attempted: `+398h` is a cruising altitude

Section 4 named `009C89CE` in `009C8920 BSP_BotTaskDiveBomb_UpdateCruiseProfile` as the place to
find out what `+398h` really carries. Reading it weakens section 3's root-cause claim, so the claim
is corrected here rather than acted on.

```
009c8977  CALL 0x0042e740                  ; the tuning singleton -> EAX, kept in EBP at 009c8996
009c897c  FLD  float ptr [0x00ce5380]
009c898b  FSTP float ptr [ESP + 0x4]
009c8994  FLDZ
009c8998  FSTP float ptr [ESP]
009c899b  CALL 0x00bd2f10                  ; the (0, 00CE5380) helper, a jitter
009c89a0  FADD float ptr [EBP + 0x4cc]     ; + tuning+4CCh
009c89ad  FSTP float ptr [ESP + 0x10]
009c89b6  COMISS / 009c89bf CMP byte [EDI+3aah],0   ; the dirty-byte guard
009c89ce  MOVSS dword ptr [EDI + 0x398],XMM0
009c89d6  MOV  byte ptr [EDI + 0x3ad],0x1
```

So on the dive-bomb path `+398h` is **tuning+4CCh plus a jitter**, and `tuning+4C0h..+4D8h` is the
dive bomb's own altitude block - the rows `src/game_hosts_units.cpp` reads as
`kPilotDiveBombCruisingAlt = 1300.0f` and its neighbours, per `docs/GAME_TUNING_SINGLETON.md`. That
is a **cruising** altitude, of the order of 1300 m, and it is nowhere near below the `100.0` the gate
at `009D3489` wants.

* **was** (section 3): the host feeds `+398h` the wrong key, 500 instead of something under 100, so
  the `009D3489` store never fires and `alt_floor_74` wrongly stays 0.
* **is**: `+398h` looks like a cruising altitude on the one path whose producer is located, so a
  value above 100 is probably the normal case and `alt_floor_74 = 0` is probably the normal outcome.
  A commanded band of `0 + 12 = 12 m` is then a plausible torpedo **release** altitude rather than a
  defect - torpedoes are dropped low. The host may still be passing the wrong key, and that is now
  **unproven either way**, because `009D4A70` writes nothing to `+398h` and no producer on the
  torpedo path has been located at all.
* **evidence**: the listing above; `009D4B57 FLD float ptr [EAX + 0x398]` is a read.

**What this does not change.** Section 1 stands entirely: the five aircraft fly into the sea at
about 141 m/s and every downstream symptom follows from that. Section 2 stands as a measurement: the
commanded altitude is 12.00 m, the pitch demand is exactly `-1.0472 rad` and it is held from 800 m
to the water.

**What it moves the question to.** If 12 m is the right place to be going, then the defect is the
way the aircraft goes there. `-1.0472` is `-pi/3` to four decimals, held constant over more than 700
metres of descent - that is the shape of a **clamp**, not of a computed slope, and the same census
line prints `drop_angle=0.4014` and `climb_1ec=0.1854`, neither of which the aircraft flies. The
aircraft also never flares: `pitch` tracks `pitch_demand` to `-1.0396` at 62 m and the next sample
is under the water.

So the next packet should start from the descent law rather than from the altitude field:
`docs/TORPEDO_MOVETO_TICK.md`, whose own title says the move-to tick commands a glide slope,
`docs/TORPEDO_RUN_IN_DESCENT.md`, and whatever writes the pitch demand the census prints. The
question to answer first is why the demand is a constant `-pi/3` instead of the glide slope those
documents reconstruct, and what should level the aircraft off at the band.

## 8. Handoff: everything the next worker needs, by address

The worktree and branch are handed over as they stand -
`J:\PROG\battlestations-pacific-decompile-cc8-plane-squadron` on `agent/cc8-plane-squadron`, built at
`b882aa1d4` (this branch with `origin/main` merged), with the before-run log already in `local/`.
Nothing needs rebuilding to start.

### 8.1 The before column is already taken and is complete

`local/tap_before_usn01.log`, `EXITCODE=0`, `frames_presented=3199 loop_finished=1`. Command:

```
./tools/run_game.ps1 -Log local\tap_before_usn01.log -WaitSeconds 3000 -- --frames 3200 `
  --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
```

`query session` was `console Active` before it. Section 7's table shows it reproduces the
`b00c3cd24` run to the centimetre, so it is a valid before column for any change made on this branch.
A USN01 run takes roughly forty minutes when three agents are sharing the machine, which they were.

### 8.2 The release chain, with the callsite counts that bound it

Established in `docs/PLANE_SQUADRON_HOST.md` section 4.2 and repeated here because it is what stops
a reader re-deriving it. All from `python tools/callsite_census.py`, which is exhaustive over rel32:

| routine | callsites | what that means |
| --- | --- | --- |
| `007BBC00` | **0** | not a function: the store `ADD dword [ECX+C20h],EBX` inside `007BBBA0`, past every early exit |
| `007BBBA0` | **34** | every one a release decision - `009FA3D0` in `BSP_ReleaseTimer_Tick`, `009D4956` in `BSP_BotTaskTorpedo_TickArm`, `009D26F8`/`009D2938`/`009D29CB` in the torpedo prepare states, `009C5777`/`009C60F1`/`009C88C4` in the dive-bomb states |
| `007C0D90` | **1** | `007CEA8D` in `007CE040 BSP_PlaneTickElement_FixedStep`, reached only when `unit+C20h > 0` |
| `007EEF30` | **1** | `007C0F01`, inside `007C0D90` |

So the release-order broadcast is unreachable until an aircraft has already requested a release. It
is what a plane that has just dropped tells its flight-mates. `009D2287` is what arms the release
timer, and it runs only after the aim stage - which is why section 1's water contact ends the chain
before it starts.

### 8.3 `009D4923`, and where it lives in this host

The image: `docs/TORPEDO_TASK_ARM.md` section (1) step 8, the call on the object **embedded at**
`unit+72Ch` through its `vtable[+38h]`.

The host: `TorpedoArmBinding::manual_release_requested` in `src/game_hosts_units.cpp`, which is

```cpp
bool manual_release_requested(void*) override {
    // (unit+72Ch)->vtable[38h] at 009D4923.
    record("Unit::device_requests_release", "009d4923");
    return false;
}
```

Replacing the `false` with the image's test is worth doing on its own terms. Section 5 is why it is
not a route to the first drop for USN01: step 8 runs `007BBBA0` only when the state is none of
`aim`, `attackrun` or `prepare`, and these aircraft spend all 124 arm ticks in `attackrun`.

### 8.4 The edit that is still owed, and the file it waits on

`docs/PLANE_SQUADRON_HOST.md` section 6 writes out one applicable edit to
`TorpedoReleaseOrderBinding` in `src/game_hosts_units.cpp`, left unmade because that file was leased
elsewhere for the whole of the packet that found it:

* `is_flight_member` (torpedo ordnance) and `controlled(int)` / `controlled_unit_count()` become a
  walk over `plane_squadron_registry().find_by_member_unit(index_of_slot(slot_))`'s `member_units`,
  which is `ctl+3D0h` under `ctl+3CCh`. A caller in no squadron answers a count of 0, which is
  `007EE891`'s own arm.
* `in.force_flag_378 = false` becomes that record's `force_flag_378`, seeded `true` after
  `007F2D1E`.

**The lease is still held.** `python tools/bsp.py lease check src/game_hosts_units.cpp` answers
`agent/cc8-dive-bomb:cc8_dive_bomb_goaway until 2026-09-19T14:35`, although that worker has reported
the file released. Check it again rather than trusting either statement, and take it only when the
check says it is free.

### 8.5 Open, by address

* ~~**The pitch demand.**~~ **RESOLVED by section 9, which supersedes this line.** The demand is a
  correct saturation of `009FB800`'s dive arm, not a clamp bug; the defect is the zero range pair
  the attackrun tick's step 4 hands `009FBA50`, which kills the glide bias and commands 12 m from
  800 m. The remaining work is reading `009D07B0` step 4's real argument setup. **That is the
  packet.**
* `ctl+398h` on the torpedo path has **no located producer**. `009D4A70` only reads it, at
  `009D4B57`. Whether `read_control_block` passes the right key is unproven either way; the
  Correction section says why the first answer was withdrawn.
* `009D3489`'s gate and `alt_floor_74` are understood but not settled: a `+398h` above `100.0`
  leaves `+74h` at zero, and on the one path whose producer is located that is the normal case.
* `009D4923`, above.
* `src/game_hosts_units.cpp`'s `TorpedoReleaseOrderBinding`, 8.4.

### 8.6 The two retractions in this document

Both are this packet's own, and both were caused by reasoning ahead of the evidence. They are listed
together so a reader does not have to find them.

1. **Section 6**: the arm was predicted to stop because the command row went stale. It stops because
   the aircraft is in the water. Inferred from the host sources before the log was read.
2. **The Correction section**: `+398h` was named as the root cause before `009C89CE` was read, and
   reading it withdrew the claim. Named as the next step in the same commit that depended on it.

## 9. The cause, located: the attackrun tick passes a zero range pair to `009FBA50`

This supersedes section 8.5's "find the producer of the pitch demand". The producer is found, the
`-pi/3` is not a clamp bug, and the defect is one call site in this host.

### 9.1 The `-pi/3` is a correct saturation, not a stray constant

`pitch_command_009fb800` (`src/plane_flight.cpp`), the dive arm at `009FB96E`..`009FBA4D`:

```
limit = max(class_drop_angle * kPlaneAngleLimitScale, kPlaneDiveAngleFloor)
t     = clamp(-weighted / drop_dist, 0, reference)
return -min(class_drop_angle * t, limit)
```

`class_drop_angle` is the `0.4014` the census prints, so `drop_angle` **is** being used - the arm
simply saturates. With the aircraft 788 m above its commanded altitude, `t` clamps to `reference`,
`class_drop_angle * t` exceeds `limit`, and the arm returns `-limit`. That is `-pi/3` to four
decimals and it is the arm doing exactly what it should when told to lose 788 m at once.

So the question is not "why is the demand a constant" but "why is the aircraft told to be at 12 m
while it is at 800 m and 4 km out".

### 9.2 The answer: `span` is zero because the call site says so

The attackrun tick's step 4, in `src/game_hosts_units.cpp`:

```cpp
binding.command_altitude_and_throttle(
    nullptr, ap.alt_margin_78 + ap.alt_floor_74,
    0.0f, 0.0f, 0.0f);          //  <-- range_low and range_high
```

The binding behind it does run the right chain, `009FBA50` then `009FB800`, and `009FBA50` biases
the base by `span * scale * class+518h` where `span = max(range_high - range_low, 0)`. With both
range arguments zero the span is zero, the bias is dead, and the commanded altitude is the bare
12 m. The binding's own comment already says it - "with span at zero the gain is unreachable anyway"
- it was written as a description of the state of things rather than read as the defect.

**The contrast that proves it is a defect rather than the native's shape.** The move-to tick's own
glide slope, about four hundred lines further down the same file, passes a real pair:

```cpp
cin.range_low  = ap.elapsed_134 >= 15.0f ? ap.speed_late_7c : ap.speed_early_80;
cin.range_high = distance;
```

That path produces a sloping descent, and `docs/TORPEDO_MOVETO_TICK.md`'s title says so. It never
runs for these aircraft: they are in `attackrun`, not `moveto`, and the run's own census confirms it
- **there is not one `glide census` line in the log**, only `descent census` lines. The mission's
aircraft are flown entirely by the step-4 call above.

### 9.3 What to read next, and the one thing not to assume

`009D07B0`, the attackrun tick, steps 1 to 5. `docs/BOT_TASK_STATES.md`'s "The torpedo run" records
step 4 as "altitude `approach->+78h` + `approach->+74h` through `009FBA50`" but this document has
not read what that step passes as the **two range arguments**, and the host's zeros are a
placeholder rather than a reading. That is the whole of the remaining work: read `009D07B0` step 4's
argument setup from the listing, pass the real pair, and the glide slope the move-to path already
produces becomes available to the run-in.

Do **not** assume the move-to pair is the right answer for the attackrun tick. `009D48CF` fills the
move-to ranges from `approach+74h`/`+78h` and `+7Ch`/`+80h`, and `docs/TORPEDO_APPROACH_UPDATE.md`
establishes that `+7Ch` and `+80h` are commanded **speeds** selected by the approach clock, which is
already an odd thing for a range slot; whatever `009D07B0` passes is its own and has to come from
its own listing.

### 9.4 Why this probably also explains the dive bomber

`agent/cc8-dive-bomb` reports its aircraft ditching at -1.12 m with pitch bottoming at -0.9795 rad,
and is binding a goaway climb-out that feeds `plane_climb_angle_1ec` - the very field this census
prints and does not fly. The climb arm of `009FB800` is the mirror of the dive arm above and takes
its bias from the same `009FBA50` span. If that path's call site also passes a zero range pair, the
climb saturates or collapses for the same reason and the binding cannot work. Worth checking on that
side before concluding the climb-out itself is wrong.

## Correction from packet `cc8_torpedo_descent_law`: section 8.5's first open item is closed, and the aircraft no longer ditch

The pitch demand's producer is `009FB800`, and the reason it sat at `-1.0472` was not the image.

* **was** (8.5, and the Correction to section 3): "`-1.0472 rad` held constant over more than 700 m
  of descent ... Find its producer", with `-1.0472` presumed to be what the image commands here.
* **is**: `-1.0472` is `-DEG(60)`, the dive cap at `00D05AAC`, and this host saturated it by feeding
  `009FB800` the commanded altitude as its second argument. That argument is `009FBA50`'s **fourth**
  - a dimensionless ramp in `[0.35, 0.8]` that `009D0A63` computes - and it is the clamp on `t`. With
  12.0 in it, `DropAngle * t` reached 4.82 rad against a 1.0472 cap at every altitude error over
  about 80 m, which is the whole 800 m descent. The image commands at most `DropAngle * 0.8` =
  0.321 rad here.
* **evidence**: `docs/TORPEDO_DESCENT_LAW.md`; `009FBA50`'s decompiler output ends
  `FUN_009fb800(param_2,param_5)`.

Section 2's second half is also corrected. The commanded altitude is **not** a flat 12 m in the
image: `009D0951`-`009D0A92` fills all four of `009FBA50`'s arguments and the range term
`max(range_90 - releaseDist, 0) * scale * tan(DropAngle)` is a glide slope that reaches 12 m at the
release distance. The three zeros this host passed for the last three arguments killed it. The
measurement `base=12.00 (74h=0.00 78h=12.00)` stands; `commanded=12.00` was the host's own doing.

`+74h = 0` is therefore no longer a suspect. It is the **bottom** of a slope, not a target the
aircraft is told to reach at once, so whether `ctl+398h` clears the `100.0` gate at `009D3489` is
now a question about the last few metres of a run-in rather than about a 4 km dive. The second and
third items of 8.5 stay open on those terms.

**Section 1 is superseded by measurement.** After the fix, on the same binary and the same
placement, `plane water contact` does not appear in the log at all, the approach runs 399 ticks
instead of 124, and the in-range latch at 2200 m closes for two of the five aircraft. The run table
is in `docs/TORPEDO_DESCENT_LAW.md`.
