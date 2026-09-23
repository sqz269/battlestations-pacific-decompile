# Anti-aircraft target assignment: host against image

Addresses: `00864FE0` (BSP_UnitGunneryAi_Tick), `00863990`, `00862820`, `008633D0`, `00864D90`,
`00727BD0` (BSP_GunneryTargetRanks_Build), `00729BC0`, `005459E0`, `00729B90`, `008FBE00`
(AAGunnerBot slot `1Ch`), `008FBFC0` (AAFlakBot slot `1Ch`), `0085A9A0`, `007F60A0`, `00730A20`,
`0072F6E0` (BSP_Gun_TargetShotDecision), `00729670`.

Packet `cc9_aa_targeting`, 2026-09-23. It asks whether the host's AA target assignment is
faithful, from two measured oddities in the dive-bomb packets. The host code is
`src/game_hosts_gunnery.cpp`, with the gunnery pass `GunneryPassBinding` over the reconstructed
rule table `src/unit_gunnery_pass.cpp`.

## 1. The image's selection law

Most of this was read earlier, in `docs/UNIT_GUNNERY_PASS.md` section 5 and
`docs/GUNNERY_CANDIDATE_ORDER.md`. It is restated here with this packet's additions marked
**new**.

- **Cadence.** `00864FE0` runs its body once each `GlobalConfig+88h` seconds; the host's
  `think_time` is 2.000 s. Nothing is re-evaluated between bodies.
- **Candidate set, per category `i`.**
  - The recon sweep takes the side's contact list at `[recon+DE8h]`.
  - `00863990` requires `00862820`: the target is alive, active and ranked, and passes the
    per-(category, class) permission byte and the class arms.
  - `008633D0` applies the category's plane/ship mask.
  - The 3D distance must be under the unit's per-category range at `unit+430h+i*4`.
  - `00862440` is the unit-AI "untouchable" gate, and `00864D90` is the visibility cache.
- **Order.** The insert sorts by rank, from `00727BD0`'s inversion of the twelve authored
  preference rows, and by distance. The walk visits the lowest rank first and the nearest first
  within a rank. `00863A4F` adds **100.0** to a plane's distance when it follows nothing
  (`[target+9D8h] == 0`). The director's fire target and newest command target are appended
  unsorted, so they are tried before the sweep.
- **Assignment `00865773`.** For each gun of the category, it walks the candidates and takes the
  first that passes:
  - `005459E0`: weapon kind `[[gun+3F4h]+80h]` is **5 or 6**. That field is the device row's
    Function id, so the kinds are FLAK and LIGHTARTILLERYFLAK (**new**, read from the listing).
    For those kinds a plane nearer than `00729B90` is skipped. `00729B90` is `MinRange` at `+58h`
    of the first ammunition class, or of the second (`+74h+7Ch`) for kind 6.
  - `00729BC0`: rejects a null target and a kind-5 target with `[t+54h]==2`. It rejects beyond
    the ammunition's `[proj+60h]`, then dispatches on projectile kind to one of six bot slots and
    returns `slot->vtable[1Ch](t)`.
- **The two AA bots' `vtable[1Ch]` (new).**
  - `AAGunnerBot` (vtable `00D180A0`, slot `1Ch` = `008FBE00`) is the category-1 ship mount at
    `gun+390h`. If the target answers `IsKindOf(4)`, it refuses when the round's
    `max(DamageMax +B0h, BlastDamageMax +B8h) <= target class Armour +4Ch`. Then it runs
    `0085A9A0`.
  - `AAFlakBot` (vtable `00D18238`, slot `1Ch` = `008FBFC0`) is category 5/6 at `gun+394h`, and
    runs `0085A9A0` alone.
  - `0085A9A0` transforms the gun-to-target direction (target `+FCh` minus the gun node's
    `+120h`) into the gun's frame. It takes `00521370`'s pitch and yaw and returns
    `BSP_GunPlatform_AnglesInFireWindow 007F60A0(-0.0 - yaw, pitch)`; `00D7A208` is `-0.0f`.
  - Both bots then ask the gun's `vtable[1D4h]`, which is `00730A20`: `ADD ECX,424h` and a jump
    to `0072F6E0`. That answers 1 while `gun+42Ch` is zero. Otherwise it caches per target, for
    a randomised lifetime, the negation of a line-of-fire predicate `00729670` built on the
    object at `gun+42Ch`. That object's installer is **contract: unread**.
- **Hysteresis.** There is none beyond the order. Each body re-walks from scratch; a gun keeps
  its target only if that target is still the first acceptable candidate. `00727F10` re-sends
  the same target and `00728000` clears a gun that found none.

## 2. Host against image, term by term

| term | image | host before this packet | status |
| --- | --- | --- | --- |
| cadence | `GlobalConfig+88h` | `think_time` 2.000 s | faithful |
| contact list | the side's recon list `[recon+DE8h]` | every enemy unit whose recon level is not `none` | substitution, recorded earlier (`docs/RECON_SLOT_OBJECT.md`) |
| class gate, mask, rank | `00862820`, `008633D0`, `00727BD0` | the same routines | faithful |
| category range | 3D distance under `unit+430h+i*4` | aim point to aim point (position plus class `Height`) under `category_ranges` | faithful to within the height offset |
| no-follow penalty | +100.0 when `[target+9D8h]==0` | never applied (`target_lacks_follow_target = false`) | **differs, not bound**: the plane follow target is on `src/game_hosts_units.cpp`, outside this packet |
| untouchable gate `00862440` | unit-AI `+1D4h` | always false | contract unread, unchanged |
| visibility `00864D90` / `00864680` | a line-of-sight test with a cache | always visible, with a random TTL | unimplemented, unchanged |
| order and walk | rank, then nearest, director targets first | the reconstructed rule table | faithful |
| flak minimum air range `005459E0`/`00729B90` | kind 5/6, `MinRange` | flag keyed on category 7 (TORPEDO), minimum 0 | **differs, bound** for kind 5; kind 6's second ammunition is not loaded, so its minimum stays 0 |
| gun range `[proj+60h]` | per round | `max_range`, the derived engagement range | faithful |
| AA gunner armour test `008FBE00` | `max(DamageMax, BlastDamageMax) <= Armour` refuses | none | **differs, bound** |
| fire window `0085A9A0` | the target must lie in the gun's authored fire window | none | **differs, bound**, in the hull frame and the shared muzzle point the host's aim already uses (the host builds no gun node) |
| line of fire `0072F6E0` | true while `gun+42Ch` is 0 | none, so true | installer unread, unchanged |
| hysteresis | none beyond the order | none | faithful |

The bound terms sit behind `kAaTargetingBound` in `src/game_hosts_gunnery.cpp`, in
`bind_aa_acceptance`.

## 3. Evidence 1: why Fletcher-class02 "re-targeted" and two Kates lived

The goaway-turn packet's logs were deleted with its worktree. The pair was rebuilt exactly: main
`dd4f591b3` (control) against the goaway branch head `adb4d47cd` (treatment), each with only this
packet's gunnery instrumentation added. That instrumentation is behaviour-neutral: every traced
run matched its untraced twin on every simulation line. It is gated by `BSP_AA_TRACE_UNIT` and
`BSP_AA_TRACE_TARGET`. Runs used `--frames 5000 --press-start-frame 30 --menu-select USN04
--mission-frames 4800 --mission-frame-seconds 0.05`, with `BSP_AA_TRACE_UNIT=Fletcher-class02`
and `BSP_AA_TRACE_TARGET="B5N Kate #6.1"`. The logs are `local\orig_ctl.log` and
`local\orig_turn.log` in this worktree.

The goaway doc's numbers reproduce exactly: `deaths` 10 -> 8, and Fletcher-class02 has 247 -> 185
shots and nearest 96 -> 35. In the control it kills `B5N Kate #6.1` at 221.56 s and `#6.1|.-2` at
228.51 s.

**Fletcher-class02's assignment did not change.** Every AA assignment it made is identical in
both runs through 215.31 s, target for target and metre for metre. At 215.31 s both runs move
all six AAMACHINEGUN guns (960 m) and all four LIGHTARTILLERYFLAK guns (1500 m) onto
`B5N Kate #6.1`, at 558 m and 17 m up. The candidate dump shows why, and it is the image's
order: the Kates rank 2 in both categories and the Vals rank 3, and #6.1 is the nearest rank-2
contact.

**What differs is whether the guns fire.** The kills are single volleys from the two
dual-purpose guns 128 and 129:

| run | 216.11-216.16 s | 221.21 s | outcome |
| --- | --- | --- | --- |
| control | gun 129, then gun 128, at 490/486 m | guns 128 and 129 at 148 m | direct 220 -> 88, blast -> 29.1, direct: dead at 221.56 s |
| treatment | guns 128 and 129, then gun 128 again | no shot, and none until 233.86 s | #6.1 ends with 147 hp, #6.1\|.-2 untouched |

Fletcher-class02's own ship-AI rows (heading, rudder, throttle) are identical for the whole run,
and so are its assignments. The first line of its trace that differs is **162.31 s**: gun 128
fires at `B5N Kate #2.1|.-2` in the treatment only, with the same target at the same range
(1007 m). From there the two dual-purpose guns' barrel and reload phase differ, and at 221.21 s
one run has a loaded pair and the other does not.

**The coupling is the gunnery host's shared random stream.** `GameGunneryHost::Impl` draws every
`00BD2F10` stand-in from one generator (`random_range_00bd2f10`). Those draws are the visibility
cache's TTL, the fire stagger, hull damage and blast damage. The goaway turn changes the Vals'
paths from tick 2400 (120 s), which is the first line of the whole log that differs. The
mission's gunnery totals are identical through 145 s and differ by 150 s:

| at 150 s | assigns | shots | hits | damage |
| --- | --- | --- | --- | --- |
| control | 2750 | 1415 | 28 | 6096.7 |
| treatment | 2748 | 1414 | 28 | 6094.3 |

The same 28 hits with a different damage sum is a shifted random stream: the same events drew
different numbers. Two fewer assignments means other ships met the Vals as candidates at other
times, so the number of draws before any given tick changed. Every later stagger draw for every
gun moved with it, and Fletcher-class02's gun 128 fired at 162.31 s in one run only. **No changed
input flipped Fletcher-class02's choice.** The changed input is the draw count of a random
stream the two engagements share.

In kind this is faithful. The image's `00BD2F10` is one process-wide generator shared with
everything, including the goaway re-roll, so the image couples unrelated engagements at least as
tightly. The host's separate gunnery stream is, if anything, a narrower coupling.

## 4. Evidence 2: the goaway Vals are engaged, and never hit

Traced on current main (`8d417666b` plus this packet, switch off), USN04 at 9000 mission frames:
`--frames 9200 ... --mission-frames 9000`, `BSP_AA_TRACE_TARGET="D3A Val"`, log
`local\trace2_e9000.log`. That log matches the untraced `local\ctl_e9000.log` on all 57 per-unit
rows.

| aircraft | assignments | gun-ticks held | angle refusals | rounds fired at it | nearest shot | hits taken |
| --- | --- | --- | --- | --- | --- | --- |
| D3A Val #3.1\|.-2 | 3857 | 158137 | 72641 | 1834 | 706 m | 0 |
| D3A Val #7.1 | 2353 | 96473 | 43552 | 1437 | 726 m | 0 |
| D3A Val #7.1\|.-3 | 1657 | 67697 | 28633 | 892 | 640 m | 0 |
| D3A Val #7.1\|.-2 | 817 | 33497 | 17251 | 332 | 576 m | 0 |

**The premise that no ship's AA engages them is wrong.** Target selection gives them to guns
thousands of times, and 4495 rounds are fired at the four aircraft. None lands. The refused
commanded verticals reach 75.8-89.9 degrees, so these are steep, high shots.

The envelope. At their goaway standoff, 1400-1575 m out from their ordered ship and 900-1000 m
up, the slant range is about 1700-1860 m. That is beyond every AAMACHINEGUN range in USN04 (960 m
on most mounts, 1600 m on some Northampton mounts) and beyond LIGHTARTILLERYFLAK's 1500 m. It
is inside FLAK's 2000 m category range, but only Lexington, Yorktown and Northampton-class05
carry FLAK. So the ordered ship mostly cannot reach them there. The shots come when their
cycling carries them within 576-1288 m of some ship. **The selection is not what keeps them
alive; the rounds miss.** That is the aim and projectile path, `docs/AA_VERTICAL_WINDOW.md`
section 5's superelevation among the suspects, and it is outside this packet.

## 5. Predictions for the bound terms, written before the switch-on run

The switch-off build counts what the bound terms would refuse without applying them (USN04, 4800
frames, `local\obs_usn04.log`, identical to `local\ctl_usn04.log` on every simulation line):
`window_rejects=5214 armour_rejects=0 min_range_skips=136`.

- **Armour:** nothing moves. Every AA round's best damage exceeds every aircraft's armour here.
- **Fire window:** AAMACHINEGUN and FLAK guns whose first candidate lies outside their authored
  window take the next candidate or clear. So fewer early assignments, and more of them held by
  guns that can bear.
- **Flak minimum range:** FLAK guns skip planes inside `MinRange`. That is 136 evaluations, few.
- **Unchanged:** LIGHTARTILLERYFLAK is not bound, so Fletcher-class02's kill guns 128/129 keep
  their rule. Deaths could not be predicted by unit: every change reaches the shared random
  stream within seconds (section 3).

## 6. Runs

| log | tree | switch | what |
| --- | --- | --- | --- |
| `local\ctl_usn04.log` | main `8d417666b`, unchanged | - | control, USN04 4800 |
| `local\obs_usn04.log` | this packet | off, observing | identical to control, 0 simulation lines differ |
| `local\bound_usn04.log` | this packet | **on** | treatment |
| `local\ctl_e9000.log`, `local\trace2_e9000.log` | main / this packet | - / off | USN04 9000, identical per-unit rows |
| `local\orig_ctl.log`, `local\orig_turn.log` | `dd4f591b3` / `adb4d47cd` + instrumentation | off | evidence 1 |

**Switch on against the control, USN04 4800.** The bound run counts `window_rejects=11748
armour_rejects=0 min_range_skips=308`. The first line that differs is the first assignment at
110 s: 8 assignments in the control, 0 bound. At 115 s it is 86 against 18, with the same 7 shots
and the same first shot at 113.50 s. From there:

| | control | bound |
| --- | --- | --- |
| `queued_hits` / `hull` | 107 / 70 | **268 / 101** |
| `total_damage` | 16080.6 | 17036.4 |
| `deaths` | 9 | **14** |

Five aircraft that live in the control die bound, and no death is lost:

| unit | control | bound |
| --- | --- | --- |
| movieval\|.-2 | lives | 198.56 s, Northampton-class01 |
| B5N Kate #6.1\|.-3 | lives | 222.26 s, Lexington-class01 |
| B5N Kate #6.1 | lives | 222.46 s, Lexington-class01 |
| D3A Val #1.1\|.-2 | lives | 227.51 s, Northampton-class02 |
| B5N Kate #8.1\|.-2 | lives | 232.21 s, Northampton-class05 |

33 of 47 per-unit rows differ.

## 7. Decision

**The switch lands false.** The direction is what the window term predicts: fewer guns hold
targets they cannot bear on, more rounds land, and more aircraft die. But the moved numbers cannot
be explained term by term. The first assignment differs at 110 s. After that, every change feeds
the shared random stream (section 3), so no per-unit outcome can be attributed to one term. Two
of the bound terms also carry substitutions: the fire window in the hull frame with a shared
muzzle, and the unloaded kind-6 second ammunition. The measured magnitude, 2.5 times the direct
hits, is too large to land on a substitution.

What is established regardless:
- **The candidate selection is faithful** in the terms that matter to both oddities: rank,
  nearest, cadence and no hysteresis.
- **Fletcher-class02 never re-targeted.** The two Kates' fate is a shared random stream's draw
  count.
- **The goaway Vals are engaged and missed.**
- **Three real divergences are recorded and bound behind the switch:** the 005459E0 kind 5/6 key
  (the host used category 7, TORPEDO), the fire-window acceptance, and the AA gunner armour test.

## 8. Open

- **The fire window needs the per-gun node frame.** `0085A9A0` transforms by `[gun+3Ch]+110h`
  from `[gun+3CCh]+120h`. `docs/GUN_MOUNT_POSITIONS.md` records that this process builds no model
  node. Binding the window faithfully needs that frame first.
- **Why 4495 rounds at the goaway Vals never hit.** This is the aim and projectile path.
- **The +100 no-follow penalty (`00863A4F`)** needs the plane's `+9D8h` follow target from
  `src/game_hosts_units.cpp`.
- **Kind 6's second ammunition (`+74h+7Ch`)** for `00729B90`.
- **`gun+42Ch`'s installer**, the line-of-fire predicate behind `0072F6E0`.
- **A shared-stream control for pairs.** Any behavioural pair that changes an aircraft's path
  will move unrelated gunnery outcomes through the random stream. A per-gun or per-unit stream
  would make such pairs readable, but it would be less faithful than the image's single
  generator.

## Correction from docs/RANDOM_STREAMS.md (packet cc9_rng_streams, 2026-09-23)

- **Section 3's "one process-wide generator shared with everything" is imprecise.** `00BD2F10`
  selects its state by the caller's ECX from two MT19937 streams per registered thread
  (`00BD2ED0`). Every gunnery consumer draws on **stream 1**, and so do the gun bots, the pilot
  bots and the goaway enter (`009C49FC`). The goaway tick's two draws (`009C4D13`, `009C4D33`)
  pass `ECX = EBX`, not read here. Stream 0 is a separate stream used mostly by the `004xxxxx`
  band. The coupling claim stands for stream 1: the gameplay consumers share one sequence, so a
  behaviour change anywhere shifts every later gunnery draw. That stream is also reseeded from
  wall-clock milliseconds when the HUD movie camera is destroyed (`0079A260`), so the image's own
  runs are not reproducible.
- **Section 2's "bound for kind 5" row is now landed.** The kind 5/6 minimum-air-range term is
  on by default (`kAaMinRangeBound = true`). Its measured pair, with the per-consumer stream
  option, moved exactly the eight predicted Yorktown-class01 FLAK rows and nothing else
  (`docs/RANDOM_STREAMS.md` section 3).
