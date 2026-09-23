# The follow law's speed: the two fly-to stores the host skipped

Addresses: 009BEE30 (fly-to stores 009BFD0F-009BFD1C), 0099D300 (readers 0099D79F, 0099D8C1-0099D8EB,
0099D924-0099D970), 009C1FD0 (abort-path stores 009C204C-009C2059).

Packet `cc9_follow_speed`. Follows `docs/PLANE_FOLLOW_HOLD_ARM.md` and
`docs/FOLLOWER_ATTACK_HANDOVER.md` section 11, which found that the host's fly-to binding writes
the desired speed but not the two stores after it.

## 1. The stores

`009BEE30`'s fly-to arm ends its speed command with three stores. `EBX` is the pilot plan,
loaded at `009BFCE3 MOV EBX,[EAX+18h]` with `EAX = [EDI] = approach`.

```
009BFD0F  FSTP float ptr [EBX+2B4h]      ; desired speed (float)        host: already written
009BFD15  MOV  byte ptr  [EBX+2B0h],0    ; byte                          host: added here
009BFD1C  MOV  dword ptr [EBX+2D8h],1    ; dword                         host: added here
```

Capstone over the disk bytes agrees on all three operand widths.

## 2. Their readers

A disp32 census over the plane-AI range (`scan-bytes`, `--limit 4000`, results filtered to
`007Bxxxx` and `0099xxxx`-`009Fxxxx`) was run for every load and compare form: `80`, `38`,
`8A`, `0F B6` against `+2B0h`, and `83`, `8B`, `39`, `3B`, `85` against `+2D8h`. The store
forms `C6`, `88`, `C7` and `89` match dozens of sites in that range, so these byte patterns do
occur. Exactly two reads turn up, both in `0099D300 BSP_PilotBot_PlanControls`:

* **`0099D79F MOV ECX,[ESI+2D8h]`**, the speed-command mode. `0099D7A5 TEST ECX,ECX / 0099D7AF
  JNZ 0099D8C1`: zero takes the ordinary throttle path at `0099D7B5`. At `0099D8C1`, `CMP ECX,1 /
  JNZ 0099D8F5` sends any value except 1 to the join. Mode 1 compares the desired speed
  (`0099D8C6 COMISS XMM0,[ESI+2B4h]`, `XMM0` = dword `[00D7A23C]`). A desired speed below that
  cut value writes the throttle cut and air brake and clears `+2D8h` (`0099D8CF`-`0099D8EB`).
  Otherwise `0099D8CD JBE 0099D924` enters the **speed-demand arm**, past the flight-state test
  at `0099D8FD`, which admits only state 5. An airborne plane is in state 7. So a desired speed
  reaches the throttle only when `+2D8h` is 1. The host already models this reader
  (`pilot_plan_throttle_0099d300`) and already carries the field, named
  `plane_air_brake_mode_2d8`. By its reader it is the speed-command mode; the field was not
  renamed, because its other writers live in other packets' hunks.
* **`0099D924 CMP byte ptr [ESI+2B0h],0`**, inside the demand arm. Zero runs
  `0099D92D`-`0099D970`:

  ```
  v = InterpolateClamped(-TrgSpeedCorrMinPitch, TrgSpeedCorrSpeedMul, 0, 1.0, pitch)
  plan+2B8h = max(plan+2B8h, v)                 ; 00415550 BSP_Math_MaxFloatByRef
  ```

  `EBX` is `0042E740()+538h` (`0099D47F`/`0099D487`), so `EBX+B0h/B4h/B8h` are
  singleton `+5E8h/5ECh/5F0h`, `Pilot/General/TrgSpeedCorrMinPitch` DEG(70),
  `TrgSpeedCorrSpeedMul` 2.6 and `TrgSpeedCorrMulDecay` 0.8. `pitch` is `[ESP+10h]`, written at
  `0099D4DC` from `unit+C64h`. The stack depth is equal at both sites: between them the only
  stack effects are `PUSH ECX` plus `CALL 007B4ED0` (RET 4), and calls to `0042B2F0`, `0099BA10`
  and vtable `+38h`, all RET 0. The `1.0` is the `FLD1` left on the x87 stack at `0099D764`.
  Earlier in the planner (`0099D75C`-`0099D79A`), `plan+2B8h` decays toward 1.0 by
  `TrgSpeedCorrMulDecay * [ESP+6Ch]`. The demand arm then divides its speed term by it
  (`0099D9A3 FDIV [EBP]`, `EBP = plan+2B8h`). So `+2B0h = 0` lets a nose-down plane raise its
  speed-error divisor toward 2.6, and a non-zero byte skips that raise. The host field is named
  `plane_trg_speed_corr_off_2b0` after this reader. The host's throttle rule substitutes 1.0 for
  `plan+2B8h`, so nothing in the host reads the new byte yet.

`009C1FD0` writes the same trio on its abort path (`009C204C` `+2B4h` = `classDesc+190h`,
`009C2052` `+2B0h` = 0, `009C2059` `+2D8h` = `ECX`), which is not the law path and is not
modelled here.

## 3. The host change

`GameUnitsHost::Impl::run_follow_law_009bfee0_009bee30` now follows `plane_desired_speed_2b4`
with `plane_trg_speed_corr_off_2b0 = 0` and `plane_air_brake_mode_2d8 = 1`, behind
`kPlaneFollowFlyToSpeedStores`. The flag exists only to take same-binary pairs; section 6 records
which value lands.

## 4. Predictions, written before runs C and B were launched

Controls A and D were already running when this section was written. Their configurations are
fixed, and nothing below was changed after reading them.

* **B against A (default configuration: mode pinned, placement on).** In E1 nothing enters
  `follow` (`follow law` lines = 0), so the fly-to binding never runs and the stores never
  execute. Prediction: every per-unit row identical, apart from the known non-deterministic
  `ship avoidance search refills` counter. If B differs from A anywhere else, the stores are
  reached by a path this packet does not know.
* **C against D (E2 configuration).** The eight wing members that start in `follow` now reach
  the demand arm with the fly-to law's speed: the leader's speed on station, up to
  `TurboMultiplier` times it at `GoodPositionDist`. Predictions:
  1. The follow-only members no longer lose speed from their seed. Their `|v|` at any water
     contact is at or above the seed (66.67 / 69.44), and most of the eight do not drown in
     `follow`.
  2. They close on their leaders: the `ordered ... closed` figure rises toward the leader's.
  3. Members that reach the 2080 m latch enter `flyabove` and release, so releases rise from
     D's value. The `movieval` pair should now attack.
  4. The fly-over and `done` behaviour of members that already reached `flyabove` in E2 is not
     predicted to change. The `done` placement snap of section 11 is still there.
* **Falsifier.** If C's follow-only members still drown below their seed speed, the demand arm
  is not what they lack. The next candidate is the HOLD arm's direct throttle law, which a member
  inside 100 m of its station takes in the image.

## 5. Runs

All four use USN04 with `--frames 9200 --press-start-frame 30 --menu-select USN04
--mission-frames 9000 --mission-frame-seconds 0.05`. Each runs from its own copied binary under
`local\binX\`, built from this branch with the configuration toggled by `local\cfg.py`.

| run | configuration | binary | log | releases | water contacts | `#3.1\|.-2` / `#7.1\|.-2` transitions | `follow law` rows |
| --- | --- | --- | --- | --- | --- | --- | --- |
| A | main, pinned, placement on | `local\binA` | `local\A_default_nostores.log` | 30 | 16 | 7 / 13 | 0 |
| B | A + stores | `local\binB` | `local\B_default_stores.log` | 30 | 16 | 7 / 13 | 0 |
| D | main + both E2 edits | `local\binD` | `local\D_e2cfg_nostores.log` | 20 | 24 | 23 / 12 | 39 |
| C | D + stores | `local\binC` | `local\C_e2cfg_stores.log` | 20 | 24 | 23 / 12 | 39 |
| Ct | C + a log-only trace every 25 ticks | `local\binCt` | `local\Ct_e2cfg_stores_trace.log` | 20 | 24 | 23 / 12 | 39 |

Releases come from the `summary mission dive-bomb task` row, and the other columns from
`local\e2_digest.ps1`. The digests are `local\<run>_digest.txt`.

* **A reproduces E1 and D reproduces E2 line for line.** The one exception is
  `gunnery torpedo_drop drops`, which reads 12 against the old runs' 0. The gunnery-host fix
  that has since landed makes that counter cumulative. So the controls are the measured
  baselines.
* **B equals A.** The digest and all 84 `summary mission` rows are identical, excluding the
  known non-deterministic `ship avoidance search refills`. So are every `follow law`,
  `water contact`, `release census`, `db aim exit` and `divebomb` row. Prediction B holds:
  nothing in the default configuration reaches the stores.
* **C equals D, and predictions 1-3 fail.** The digest, all 84 summary rows and all 39
  `follow law` rows are identical. The eight follow-only members drown exactly as in E2. The
  falsifier fires, but not in the direction section 4 expected. The HOLD arm's throttle law is
  not the missing piece either; see below.

**Why C equals D: run Ct.** Ct's summary rows equal C's, so the trace is log-only. All 516 trace
rows read:

```
mode2d8=1  thr=1.000/1.000  act=0  state=7
```

The stores do set the speed mode to 1, which D never did. But the throttle slot is already at
1.0, current and desired, on every row of every follow-only member in both runs. The demand arm
asks for more speed than the aircraft has (`want` 104-130 m/s against `v` 20-68 m/s), and full
throttle is what it already has. There is nothing for the stores to change.

What kills the members is the **pitch**. `movieval|.-3`, whose altitude equals its commanded
altitude to within 17 m at tick 26:

```
n=1   ownY= 725.0 cmdalt= 725.4 pitch= 0.190 v=67.79
n=26  ownY= 727.3 cmdalt= 744.4 pitch= 0.698 v=67.80
n=76  ownY= 805.2 cmdalt= 821.6 pitch= 0.698 v=48.05
n=251 ownY=1021.3 cmdalt=1035.9 pitch= 0.698 v=19.87
n=326 ownY= 821.8 cmdalt=1103.2 pitch= 0.698 v=47.32
n=501 ownY=  38.8 cmdalt=1218.3 pitch= 0.698 v=51.63
```

`Zuiho-class01_sqn09|.-2` repeats the pattern from 125 m. The commanded altitude climbs about
33 m/s behind the leader. The pitch command is pinned at the class climb angle (0.698 rad) from
the first ticks. Speed bleeds to about 20 m/s, the aircraft stalls and falls, and the command
never lets the nose down again. Over all 516 rows the pitch command takes three values: +0.698
(308), -1.047 (187) and about 0 (6). That is bang-bang. It is the host's own substitution: the
fly-to binding turns the commanded altitude into a pitch through `009FB800` with the commanded
altitude as its own reference. The image instead calls `009F9ED0` (`RET 8`, altitude error and
distance) at `009BFC21`, whose body is unread (`docs/PLANE_FOLLOW_LAW.md` §5.2, the host
comment at the call).

So the members lack a **pitch law that respects airspeed**, not a speed command. The HOLD arm's
throttle rule (`include/bsp/plane_follow_hold.hpp`) would not help: it also ends at full
throttle, and a member this far behind never takes the hold arm. It was not wired.

## 6. Decision

* **The stores land**, `kPlaneFollowFlyToSpeedStores = true`, on the default configuration.
  They are the image's own sequence, B is identical to A, and Ct proves they reach the demand
  arm (`mode2d8=1`, which D never had). The landing build's `.text` section is byte-identical
  to run B's binary. Only `.rdata` differs, as it does between any two links.
* **The E2 configuration does not land.** C against A fails every section 8 criterion: 8 new
  water contacts, releases 20 against 30, and transitions of 23 and 12 against the 4-7 range.
  Both E2 edits are reverted again: `control_mode_370` is pinned, and dive-bomb follow
  placement is on.
* **The next piece is `009F9ED0`.** Read its body, and bind the fly-to arm's pitch through it
  instead of the `009FB800` substitute. Then re-take C against D with the same parameters. The
  `follow trace` row stays in the source at `kFollowTraceEvery = 0`, which compiles it out; set
  it to 25 to take the trace again.

## 7. Phase A guards, partial (analysis only)

`tools/x87trace.py` over `009BFEE0` with `tools/callee_effects_009bfee0.json` puts the first
guard at x87 depth 5. There `e = |base+24h + base+0Ch|`, and `009C08B5`-`009C08C7 JA` takes the
bit-8 regime when `e > 0.05 * p` (double `[00D7A270]`). `base+0Ch` is `V`, the cross-track
offset (`009C018D`). `base+24h` is last written at `009C040B`, `009C04AC`, `009C0957`,
`009C09AB` or `009C0AB4`, each time a lateral displacement built from Phase A's turn model:

* `base-8h` is the horizontal speed (`sqrt` at `009C029F`).
* `ω` is `block+0Ch`/`+10h` (`SmallPlaneTurnMul` / `LargePlaneTurnMul`, picked by vtable
  `+5Ch(10h/16h)`) times `classDesc+270h`.
* `base-20h = classDesc+18Ch / ω` is a turn radius.
* The terms `-r sin θ` and `r (1 - cos θ)` (`base+18h`, `base+1Ch`) are the displacement after
  turning through `θ = A ± π/2`.

So `e` reads as the cross-track error left after the planned turn. `p` is the top of the x87
stack, carried through the iterative loop `009C07D6`-`009C0861` (which jumps back to
`009C07D6`); its identity is **not established**. The twins at `009C0B96`/`009C0BC1` were not
read.
