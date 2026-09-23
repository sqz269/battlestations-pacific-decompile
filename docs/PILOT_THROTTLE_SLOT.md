# The throttle slot: why the fighters stall, and the speed-hold multiplier

Addresses: 0099D300, 0099D4EE, 0099D79F, 0099D7CB, 0099D87E, 0099D8C1, 0099D924, 0099DBBF,
0099B450, 007B4ED0, 009BEE30, 009BF9AA.

Packet `cc9_throttle_slot`. Every name is a hypothesis, not a recovered symbol.

## 1. The image's throttle path in 0099D300

The frame is `SUB ESP,58h; PUSH ESI; PUSH EDI`, then `PUSH EBX; PUSH EBP` at `0099D3DA` after the
early exit. From there the depth is 68h, so `[ESP+6Ch]` is the stack argument (`frame+4`), the
think dt. `tools/stack_frame_walk.py` loses the frame after its first indirect call and resets
its depth after the early epilogue; corrected for both, every `[ESP+6Ch]` below is that one slot.

| site | what |
| --- | --- |
| `0099D438`-`0099D466` | `plan+2ECh` (the slew rate) = `[00E0E2EC]` (0.24), or `[00E0E2F4]` (0.1) in flight states 4 and 5 |
| `0099D479`-`0099D4B3` | `dtScale = max(unit+340h * 0.4 (00CE65D0, double), 1.0)`; `unit+340h` is the CheatTurbo multiplier |
| `0099D4EE`/`0099D4F2` | **`[frame+4] = (1 / dtScale) * dt`** |
| `0099D79F`-`0099D7AF` | `plan+2D8h != 0` (speed mode) jumps to `0099D8C1` |
| `0099D7B5`-`0099D7C5` | mode 0 only: skips to `0099D8C1` unless `plan+2ECh > [00E0E2F4]` |
| `0099D7CB`-`0099D87E` | mode 0 only: `[frame+4] = max(|(active ? desired : current) - current|)` over the throttle slot (`+274h`/`+278h`/`+27Ch`) and the air-brake slot (`+2A4h`/`+2A8h`/`+2ACh`) |
| `0099D884`-`0099D8BF` | mode 0 only: `plan+2ECh` from that pending distance |
| `0099D8C1`-`0099D8EB` | mode 1 with `plan+2B4h < 0.001`: throttle 0.001, air brake, `+2D8h = 0` |
| `0099D924`-`0099DBC3` | the demand: `increment = interp(-6.94, -2, 6.94, 2, error)`, times 0.6 when positive, **times `[frame+4]`** (`0099DBBF`), added to the slot's seed |

So in speed mode the increment is scaled by the think interval, and in mode 0 by the pending
distance.

**`0099B450`** (the per-think seed, `0099B456`-`0099B470`) sets the throttle slot's `current`
(`+274h`) and `desired` (`+278h`) both to the live `unit+9F0h` and clears `active` (`+27Ch`). So
the pending distance is 0 unless a task writer re-activated the slot this think.
The same seed also sets `+2B4h` = `[plan+2F4h]+190h` (`0099B503`/`0099B511`, float; the
host takes it as TravelSpeed * NewTravelSpeedMul) and `+2D8h = 1` (`0099B542`, dword). So every
plane is in speed mode each think unless its task writes mode 0.

**The writers of the throttle slot** are the aim tick's head-on arm (`007B4ED0`), the maneuver
tail, attackrun, the dive-bomb states, and the follow law's hold arm (`009BEE30` at
`009BF9AA`-`009BF9C8`: throttle, air brake, `+2D8h = 0`). The moveto (`009C18C0`) writes none.

## 2. The host, traced

`src/plane_ai_control.cpp`'s `pilot_plan_throttle_0099d300` multiplies the increment by
`in.pending`, and the host fed it `|desired - current|` of the throttle slot while active, else 0,
**in both modes**. After the seed that is 0 in speed mode, so the speed hold never moves the
throttle.

Run TR (`local\TR_9000.log`, throttle wiring on, `throttle_trace` lines for the Yorktown
flight), Yorktown leader:

| phase | mode | desired speed | throttle slot | pending | output | speed | altitude |
| --- | --- | --- | --- | --- | --- | --- | --- |
| aim, not head-on | 1 | 1908-1937 | cur = des = 0.630, inactive | 0 | 0.630 | 50.77-50.79 | 199 to 185 |
| moveto, after aim | 1 | 83.33 | cur = des = 0.630, inactive | 0 | 0.630 | 50.79-50.85 | 180 to 122, -4.8 m per think, to the water |

The 0.630 is what the head-on arm left. **The divergent term is `in.pending` in speed mode**: the
image scales by dt there, and the host by a distance that is always 0. The host's follow also
lacks the hold arm's direct-throttle write (`009BF9AA`), which is reconstructed in
`src/plane_follow_hold.cpp` but not wired. That is a second, separate hole, and it is not
addressed here.

## 3. The binding (`kPilotThrottleSlotBound`, off by default; see section 6)

* In speed mode, pending is the think dt, since dtScale is 1.0 because CheatTurbo is not
  modelled.
* In mode 0, pending is the maximum of the throttle and air-brake pending distances.

## 4. Predictions, written before runs S0/S1/S1T

All runs are USN04 at the E2 parameters. S0 is this tree with the fix off. S1 turns on the fix.
S1T is the fix plus `kDogfightThrottleBound` plus the trace.

1. **S1 moves many rows.** Every plane in speed mode now has a working speed hold. Before, its
   throttle stayed where the last writer left it, usually 1.0. Speeds converge on each task's
   desired speed:
   * moveto 83.3 m/s;
   * the dive-bomb approach's value;
   * follow members' speeds.
   Expect changed paths, timings and dive-bomb rows for most aircraft. That is a broad change,
   explained by one term.
2. **S1T.** After aim the Yorktown leader's throttle climbs back from 0.63 at up to about
   2 × 0.6 × dt per think. It regains speed toward 83 m/s and does not drown. `.-2` in follow
   runs the host's fly-to arm in speed mode, so it recovers the same way.

## 5. Results

All runs are USN04 at the E2 parameters, from binaries built in this tree. The trace switch is
inert: V0 and S0 match, and so do V1 and S1, on every summary figure.

| run | log | fix | other switches | bomb drops | kill credits | plane water contacts | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S0 | `local\S0_9000.log` | off | none | 29 | 15 | 8 | failed at 228.06 s |
| S1 | `local\S1_9000.log` | on | none | 0 | 18 | 19 | none |
| S1T | `local\S1T_9000.log` | on | `kDogfightThrottleBound`, trace | 0 | 17 | 21 | none |
| S1A | `local\S1A_9000.log` | on | `kAimDiveTailBound` | 0 | 19 | 19 | failed at 342.04 s |
| V0 / V1 | `local\V0_9000.log` / `local\V1_9000.log` | off / on | trace (adds D3A Val #1.1) | 29 / 0 | as S0 / S1 | 8 / 19 | as S0 / S1 |

**Prediction 1 was right about breadth and wrong about the outcome.** The fix does not make
the Vals fly to their speeds and bomb. It stops every dive-bomb release. The chain for D3A Val
#1.1, from `val_trace`, first pass:

| state | V0: speed in, speed out, throttle | V1: speed in, speed out, throttle |
| --- | --- | --- |
| attackrun (7BCh), mode 0 | 66.7, 76.5, slot not written | same |
| flyabove (778h), mode 1, wants 63-66 | 76.6, 109.4, 1.0 (frozen) | 76.6, 63.0, cut to 0.1 |
| turndown (79Ch), mode 1, wants 34.5 | 109.5, 112.3, 1.0 (frozen) | 62.8, 52.3, 0.001 |
| aimdive (734h), mode 0 | 113.4, 133.9, 1.0 carried over | 52.8, 53.9, 0.001 carried over |
| release | at 288-345 m, 132-134 m/s | none; aimdive leaves at 450 m, 530 m out |

Before the fix, the host's dives ran on a throttle of 1.0 that no speed hold could move. With
the fix, flyabove and turndown hold their desired speeds, as the image does. Then two mode-0
states in this host write no throttle, so the cut carries through the dive:

* **aimdive.** The image writes throttle `+278h`/`+27Ch` and air brake `+2A8h`/`+2ACh` at
  `009C5F37` and `009C605F`-`009C6079`, just before `+2D8h = 0` at `009C6080`. The host has
  them behind `kAimDiveTailBound`, which is off because releases fell 23 to 4
  (`docs/AIMDIVE_RESPONSE.md`).
* **goaway.** Its throttle and air-brake shaping `009C4C0C`-`009C4CA7` is not modelled.

S1A binds the aimdive tail on top of the fix, and still drops nothing. The other moved rows,
namely the ship AI, gunnery, air ops, mission phase and dialogs, follow from the Lexington no
longer being bombed: S0 fails the mission at 228 s when it is lost, while S1 runs on into
phase 2 with two more squadrons launched. Ship AA kill deltas are also coupled through the
shared random stream.

**Prediction 2 was half right.** In S1T the Yorktown leader no longer stalls. After aim the
speed hold drives its throttle to 1.0 and it flies at 105 m/s. But it and `.-2` still reach the
water, at |v| 105.7 and 100.5, inside aim. Aim's speed arm asks for (distance - follow distance)
+ target speed (`src/dogfight_task.cpp`, about 1850 here, so full throttle), and its pitch tracks
a target Val that is itself gliding into the sea at 35-55 m/s. So the fighters' loss in S1T is
downstream of the Val regression, not of the throttle wiring. That is not proven.

The dogfight moveto doc's MB hypothesis (`docs/DOGFIGHT_MOVETO.md` section 6) is confirmed. The
state the direct-throttle arm leaves behind is the throttle value itself, 0.630. The speed-mode
commands that follow cannot clear it because their multiplier was 0.

## 6. Decision

* **The fix lands switched off**: `kPilotThrottleSlotBound = false`, with the measurement in
  the comment. The read is image-faithful and every moved row traces to it, through the aimdive
  and goaway throttle writers this host lacks. Switching it on today would take USN04 from 29
  releases to none.
* **Order of work.** Bind the goaway throttle shaping `009C4C0C`-`009C4CA7`, and resolve the
  aimdive tail's saturated pitch command (`docs/AIMDIVE_RESPONSE.md` section 4). Then re-take
  S0 against S1.
* **The throttle wiring stays off** (`kDogfightThrottleBound`). Both fighters still drown in S1T.
* The per-think re-seed's cause, as given in the `kDogfightThrottleBound` comment, was wrong. That
  comment now names the multiplier.

## 7. Step 4, read-only

* **`00956EE0`** is the base `vtable[1FCh](gunFire)`. It is an effect toggle.
  * With the flag set, a definition at `class(+538h)+ECh` and nothing at `unit+670h`, it creates
    one point effect on the node at `unit+4A4h` with an identity matrix (`008687C0`).
  * With the flag clear, it stops the effect's children and flags it dead (`00867B10`, byte
    `+9 = 1`).
* **`007CA5F0`** calls `00956EE0`, then does the same for an array.
  * The definition is at `class+234h`. The matrices are at `class+238h`, 40h bytes apart, and
    `class+23Ch` holds the count.
  * The live references are appended to `unit+A30h`, with the count at `+A34h`.
  * Clearing releases them from the end.
  * These are visual muzzle effects. Neither routine touches flight or throttle state.
  * Ghidra lists no direct callers of `007CA5F0`, since it is reached through the vtable.
* The weapon-slot class at `unit+974h` was not reached.
* Ledger names, both provisional: `00956EE0` BSP_Plane_SetGunFireEffect and `007CA5F0` BSP_Plane_SetGunFireEffects.

## Correction, 2026-09-23 (packet cc9_dive_throttle)

* Section 5 names aimdive and goaway as the mode-0 states that write no throttle in this host.
  Aimglide belongs on that list too. The image writes throttle, air brake and `+2D8h = 0` at
  009C5663-009C567F, and this host did none of it, so aimglide ran in the per-think speed mode.
  All three are now read, in `docs/DIVE_THROTTLE.md`.
