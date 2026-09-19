# The aim tick commands a dive, and one constant read at the wrong width said it commanded a climb

Packet `cc8_torpedo_release_timer`, owner `agent/cc8-plane-squadron`.

Addresses: `009D1D54`, `009D1D5A`, `009D1D62`, `009D1D66`, `009D1D6E`, `009D1D82`, `009D1DA8`,
`009D1DD2`, `009D1DED` (the release threshold `F34`); `009D1E32`, `009D1E39`, `009D1E49`,
`009D1E5A`, `009D1E6A`, `009D1E7E`, `009D1E8E`, `009D1E98`, `009D1E9A`, `009D1EA6`, `009D1EAE`,
`009D1EB0`, `009D1EBC`, `009D1EC4`, `009D1EC8`, `009D1ECA`, `009D1EDD`, `009D1EE5` (the commanded
pitch and its clamp); `009D19A0`, `009D19A4` (the run-time arm); `009D1360`, `009D13A9`, `009D13BF`
(the run time and the fall lead); `009D2038`, `009D2052`, `009D20B4`, `009D20C4`, `009D210E`,
`009D2165`, `009D21AC`, `009D21F2`, `009D2209`, `009D2215`-`009D2231`, `009D2272`, `009D2287`
(the five-flag chain and the arm). Constant: `00D21318`.

**The finding.** `009D1EDD` writes a **descent** command, and the host was making it a climb. The
clamp's lower bound at `00D21318` is the float `0xBFB2B8C3` = **-1.3962634 rad = -DEG(80)**;
`include/bsp/torpedo_aim_tick.hpp` carried `0.05625f`, which is the **double** sitting at the same
eight bytes. Both instructions that load it are four-byte. With a lower bound of `+0.05625` every
descent the aim tick asked for became a 3.2-degree climb, which is why USN01's torpedo bombers, once
they reached the aim state alive, climbed from 488.9 m to 559.9 m over the target instead of coming
down to the 25-to-40 metre release band.

## 1. The clamp, from the bytes

```
009d1e8e  FLD   float ptr [ESP + 0x3c]      ; F34, the height above the altitude floor
009d1e98  FCHS                              ; NEGATED
009d1e9a  FDIV  float ptr [ESP + 0x50]      ; / max(range - 1200, desc+188h * denom)
009d1e9e  FSTP  float ptr [ESP + 0x50]      ; p
009d1ea2  FLD   float ptr [ESP + 0x50]      ; ST0 = p
009d1ea6  FLD   float ptr [0x00d21318]      ; ST0 = LO, ST1 = p          <- FOUR-BYTE LOAD
009d1eac  FCOMIP ST0,ST1                    ; LO vs p, pop
009d1eae  JBE   0x009d1ebc                  ; taken when LO <= p -> the high test
009d1eb0  MOVSS XMM0,dword ptr [0x00d21318] ; else take LO               <- FOUR-BYTE LOAD
009d1eba  JMP   0x009d1eda
009d1ebc  FLD   double ptr [0x00d057e0]     ; ST0 = HI = 0.8726646, ST1 = p
009d1ec2  FXCH                              ; ST0 = p, ST1 = HI
009d1ec4  FCOMIP ST0,ST1                    ; p vs HI, pop
009d1ec8  JBE   0x009d1ed4                  ; taken when p <= HI -> keep p
009d1eca  MOVSS XMM0,dword ptr [0x00d05b40] ; else take HI = 0.8726646
009d1ed4  MOVSS XMM0,dword ptr [ESP + 0x50] ; p
009d1edd  MOVSS dword ptr [EAX + 0x2bc],XMM0
009d1ee5  MOV   dword ptr [EAX + 0x2d0],0x1
```

`00D21318`: `u32 = 0xBFB2B8C3`, float `-1.3962634`, qword-as-double `0.05625002828`. `DEG(80)` is
`1.3962634` exactly, so the authored value is **-80 degrees**. `009D1EA6` is `FLD float ptr` and
`009D1EB0` is `MOVSS`; neither is eight-byte, and the double form is never loaded anywhere.

Note the contrast one instruction later: the **high** bound really is stored in both widths, the
double `0.8726646304` at `00D057E0` for the `FLD double ptr` and the float `0.8726646` at
`00D05B40` for the `MOVSS`. That is how the compiler emits a constant used at both widths, and it is
the pattern every correctly-transcribed constant in `include/bsp/torpedo_aim_tick.hpp` shows.

So the whole rule is

```
margin      = max(unitAltitude - altitudeFloor, -0.5)               009D1D5A, 009D1D82
turnFactor  = max(1 - 1.6 * |turn|, 0.1)                            009D1D66, 009D1DA8
F34         = margin * turnFactor                                   009D1DD2, 009D1DED
pitchDen    = max(range - 1200, desc+188h * denom)                  009D1E5A, 009D1E7E
cmd+2BCh    = clamp(-F34 / pitchDen, -DEG(80), +DEG(50))            009D1E98 .. 009D1ECA
cmd+2D0h    = 1                                                     009D1EE5
```

`F34` is the aircraft's height **above** the release floor, so `-F34` is negative whenever the
aircraft is high and the command is a dive whose steepness falls as the aircraft descends and as the
range closes. It reaches zero at the floor. That is a flare, and it is the aim state's own descent -
the one `docs/TORPEDO_DESCENT_LAW.md` said did not exist.

## 2. How the wrong width survived: the audit that finds it

Every constant in `include/bsp/torpedo_aim_tick.hpp` was re-read from the image at both widths and
checked against the width of the instruction that loads it. The header's own convention is the tell:
a constant needing a width check carries **two addresses** (`00CEC9E0 / 00CE69D0` for -0.5,
`00D7A3A0 / 00D7A2F0` for 0.1, `00CE3928 / 00CE3CB4` for -0.1, `00D057E0 / 00D05B40` for 0.8726646)
or a **named site** proving which width was seen (`00CF1440, 009D204C`; `00CE3D40, 009D2104`;
`00CE4D70, 009D2002`; `00D7A2B0`; `00CE3D48, 009D1D66`). `kPitchClampLo` carried a bare address and
no site. It is the **only** one in the file that did, and it is the only one that is wrong.

Every other constant checks out: `00CE685C` float 40 and `00CE89CC` float 25 (the altitude gate),
`00CE5444` float 80 / `00CEB4D4` float 50 / `00CE5380` float 15 (the cone), `00CE38C8` float 30 and
`00CE3850` float 5 (the land and water floors), `00D7A24C` float 1 (the ground arm), `00CE3854`
float 3, `00CE398C` float 0.3490659, `00D7A260` float -1, `00CE89D8` float 2.2, `00CE69CC` float
-0.2, `00CE3958` float 2, `00CE3800` float 0.5, `00CE69C8` float 0.3, `00D06874` float 1.4,
`00D06BB4` float 1.6, `00CE74F8` float 0.8, `00CE7804` float 0.4, `00CE380C` float 1.5, `00D05B40`
float 0.8726646; and the double-only ones `00CE3D28` pi, `00CE3D20` 180, `00CF1440` 80, `00CEC390`
0.006, `00CE3D40` 0.8, `00CEC160` 1.2, `00CF0058` 72, `00CE4D70` 200, `00D7A2B0` 3, `00CE3D48` 1.6,
`00D7A280` 0.5, `00CEC9E0` -0.5, `00D7A3A0` 0.1, `00CE3928` -0.1, `00D057E0` 0.8726646 each match a
`double ptr` load at the site named beside them.

## 3. The five-flag release chain, and which flag was the blocker

`009D202C`-`009D2233`. `009D2215`-`009D2231` tests four byte flags and then `TEST BL,BL`; any clear
flag jumps to `009D228B`, so the arm at `009D2239` needs **all five**, while the countdown at
`009D228F` is re-tested at `009D228B` and needs only the cone flag.

| # | site | flag | on USN01 before this fix |
| --- | --- | --- | --- |
| 1 | `009D2038` | `!state+24h` | not the blocker |
| 2 | `009D2052` | `envelope > range` **and** `range + 80 > approach+A0h` | first half false while the range is long; second half trivially true, because `approach+A0h` is 0 |
| 3 | `009D20C4` | `Interp(0.4, 40, 1.0, 25, t) > unitAltitude`, so **25 to 40 metres** | **false, by two orders of magnitude**: the aircraft held 488.9 to 559.9 m |
| 4 | `009D2165` | `unit+C64h > max(range * 0.006 - 0.8, -0.1)`, computed only while `\|unit+C68h\| < 0.8` | not the blocker |
| 5 | `009D2209` | `cone(t) > \|turn\|`, `Interp(0.3, 80deg, 0.8, 50deg, t)` under 0.8 s and `Interp(0.8, 50deg, 1.6, 15deg, t)` over | open at aim tick 1 (`cone_open=1`, `F18=0.0148`) and closing later |

Flag 3 is the blocker, and flag 3 is exactly what the pitch clamp decides. The arm itself then needs
`1.0 > groundHeight` at `009D2272` (satisfied over water), refuses to re-arm on `state+0Ch`, and
starts the countdown at **zero** at `009D2282`, so the drop lands on the next aim tick.

## 4. `009D1360`, the run time, and why it does not run here

`009D19A0` calls it only when `turn_room > range`, with
`turn_room = max(obj+268h, obj+26Ch) * sin(min(|turn|, 1.2)) + speed + 200`: a commit hook that
fires once the aircraft is inside its own turning circle.

**`run_time_009D1360 = 0` in every run so far is a host substitution, not a measurement.**
`src/game_hosts_units.cpp` passes `in.turn_radius_268 = in.turn_radius_26c = 0.0f`, so the host's
`turn_room` is `0 * sin(.) + speed + 200` = about 281 m whatever the aircraft is doing, and the gate
can only open inside 281 m of the target. Nothing has been established about when the image opens
it. `docs/TORPEDO_DESCENT_LAW.md`'s statement that this gate "never opens" is withdrawn on the same
grounds.

What it computes, from `009D1360`-`009D14F3`: `+A0h = fallTime * v0`, the horizontal distance the
torpedo covers while falling (`007BCC80` gives the fall time from the release altitude, `009D13A9`
stores the product); then the water run, decelerating from `v0` to `007BCFA0`'s water speed at
`[00CF1440]` = 80 per second over the `[00D7A280]` = 0.5 mean-speed distance, and `+98h` = `+9Ch`
plus the total. `009D3D12` reads `+98h` as the bias of the engagement estimate at `+F8h`.

**A zero `+A0h` does not block the release.** Flag 2's second half is `range + 80 > +A0h`, which a
zero satisfies. So the hook staying cold costs accuracy in the engagement estimate, not the drop.

## Corrections

### To `src/game_hosts_units.cpp`, the comment above `plan_state.pitch_target_2bc`

* **was**: "The value is `clamp(-f34 / den, 0.05625, 0.872665)` and `f34` is the aircraft's height
  ABOVE the altitude floor, so the quotient is negative whenever the aircraft is high and the clamp
  floors it at 0.05625 rad. It is a nose-up floor and a pull-up, not a descent command: nothing in
  the aim tick brings a torpedo bomber down."
* **is**: the band is `[-DEG(80), +DEG(50)]`. The quotient being negative when the aircraft is high
  is the **point**: the aim tick is the descent that takes a torpedo bomber from the in-range latch
  to the release band, and it flares as the height above the floor closes.
* **evidence**: section 1.

### To `docs/TORPEDO_AIM_TICK.md` section (4)

Its row for `009D1DD6`-`009D1EE5` reads `clamp(-F34 / max(range - 1200, obj+188h * denom), 0.05625,
0.872665)`. The lower bound is `-1.3962634`. Appended to that document.

### To `docs/TORPEDO_DESCENT_LAW.md` section "Validation"

* **was**: "**The aim state commands no altitude.** `009D15F0`'s callee list contains neither
  `009FBA50` nor `009FB800` ... So when the `2200 m` latch closes the descent command stops ...
  Something between the latch and the band has to bring them the last 400 m, and it is not the chain
  this packet fixed."
* **is**: the callee observation is correct and the conclusion drawn from it is not. The aim tick
  does not command an **altitude**; it writes the **pitch** directly at `009D1EDD` with mode
  `cmd+2D0h = 1`, bypassing `009FBA50`/`009FB800` entirely. That write is the descent, and the thing
  that brings them the last 400 m is this packet's constant.
* **evidence**: section 1.

### To `docs/TORPEDO_RUN_IN_DESCENT.md` section 5 and `docs/PLANE_POSE_THROTTLE_ALTITUDE.md` section 4

Both call the aim tick's `plan+2BCh` write a nose-up floor or a pull-up. It is a dive command.
Appended to both.

## ABI

| address | prototype | cleanup |
| --- | --- | --- |
| `009D15F0` | `void __thiscall(this, float dt)` | `RET 4`, body `009D15F0`-`009D2379` |
| `009D1360` | `void __fastcall(this)` | `RET`, body `009D1360`-`009D14F3` |

## Coverage

| routine | coverage |
| --- | --- |
| `009D1D44`-`009D1EE5`, the threshold and the pitch | complete |
| `009D202C`-`009D2233`, the five flags, and the arm at `009D2239`-`009D2287` | complete |
| `009D1360` | read for its two outputs and their producers; the `007BCC80` fall-time curve and the `007BCFA0` water speed are `contract: unread` |
| `009D19A0`, the run-time arm | complete |

## Uncertainty

* `obj+268h`/`+26Ch`, the turn radii in `turn_room`, are not traced to their producers, so the
  statement that `009D1360` is a commit hook is a reading of the compare, not of the radii.
* `desc+188h` in `pitchDen` is `MaxSpd` by `docs/TORPEDO_AIM_TICK.md`; this packet did not re-read
  it.

## Validation

`local/reltimer_after_usn01.log` against `local/descentlaw_full_usn01.log`, one commit apart on this
branch, same USN01 command line, `query session` = `console Active`, `exit_code=0
frames_presented=3199 loop_finished=1`.

| Mav1, from the aim state's first tick | before | after |
| --- | --- | --- |
| altitude at aim tick 1 / 51 / 101 | 488.9 / 484.8 / 504.4 m, **climbing** | 488.9 / 340.8 / **115.8** m, descending |
| range 90h at the same ticks | 2196.6 / - / - | 2199.2 / 1881.7 / 1485.9 m |
| `\|v\|` | 81.15 / 72.61 / 68.00 | 81.15 / 100.18 / 110.08 |
| `cone_open`, `\|delta\|_F18` | 1 at tick 1, 0 by tick 301 | **1 throughout**, `F18` 0.0095 to 0.0224 |
| state transitions per aircraft | 193, oscillating attackrun/aim/goaway | **2 to 3** |
| `plane water contact` | none; held 489-560 m over the target | five, `alt` -0.02 to -0.26, `\|v\|` 99.3 to 103.4 |
| releases | 0 | 0 |

**The clamp is proved.** The aim tick's command reverses sign in the run exactly as the listing says
it should: the same aircraft, on the same trajectory into the same state, climbs 15 m in 100 ticks
before the fix and descends 373 m in 100 ticks after it. The oscillation also stops - 2 transitions
instead of 193 - because the aircraft no longer climbs out of its own approach.

**It is not yet a drop, and the reason is now three host substitutions, not the image.** The
aircraft reach the surface about 1.3 km short of the target, so they are never simultaneously low
enough for `009D20C4` and close enough for the rest of the chain. All three substitutions sit in one
block of `src/game_hosts_units.cpp`, which is leased to `agent/cc8-dive-bomb`; the edits are queued
for that file's next window.

| substitution | native | what it breaks |
| --- | --- | --- |
| `in.pitch_scale_188 = 1.0f` | `desc+188h`, `MaxSpd`, **69.44 m/s** on this aircraft (`009D1E64 FLD [EAX+188h]`) | `009D1E7E` takes `pitchDen = max(range - 1200, desc+188h * denom)`. Inside 1200 m of range the first term is negative, so this product **is** the denominator: 208 upward with `MaxSpd`, 3.0 upward with 1.0. With 1.0 the commanded pitch saturates at `-DEG(80)` for any height above the floor over about 4 m, and the aircraft goes vertical inside 1200 m |
| `in.pitch_div_1ac = 1.0f` | `desc+1ACh` (`009D1E39 FDIV [EDX+1ACh]`) | the nose-down damper `denom = 3 + (-unit+C64h / desc+1ACh) * sel`, which is what shallows the command as the dive steepens |
| `in.turn_radius_268 = in.turn_radius_26c = 0.0f` | `obj+268h`, `obj+26Ch` | `009D19A0`'s commit hook, section 4 |

The fourth quantity to check when that file opens is where the aircraft **enters** the aim state:
it arrives at the 2200 m latch at 489 m, while the attack run's own glide slope commands 383 m there
(`12 + (2200 - 450) * 0.5 * 0.4245`). The aircraft is 106 m above its own slope because it starts
the run at 800 m and lags; whether the image starts it lower is a placement question this packet did
not open.

## Known downstream, not this packet's to fix: the post-drop broadcast has no members to issue to

`agent/cc8-ai-squadron` reports that the squadron registry's `member_units` is filled for air-ops
launches and never for scene-row squadrons: the scene pass clears it and fills only names and spawn
indices, and the `resolve_member_units` its header promises does not exist. USN01's Mavs are scene
rows, so `find_by_member_unit` answers no record for them and the release-order binding merged at
`b2be05c68` - `controlled_unit_count` = `ctl+3CCh`, `controlled(index)` walking `ctl+3D0h` - sees
zero members.

It does **not** block arming the timer at `009D2287`, the countdown at `009FA3A0` or the first
release request through `007BBBA0`. It blocks `007EEF30`'s post-drop broadcast, which is what an
aircraft that has just dropped tells its flight-mates. The write-back is being made in
`src/game_hosts_scene_contents.cpp` and the squadron host by that agent; those files are not touched
here. If a run of this packet's reaches a drop before that lands, the broadcast is recorded as
blocked by this and nothing is inferred from its absence.
