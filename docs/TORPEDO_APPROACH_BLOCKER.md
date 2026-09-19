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
