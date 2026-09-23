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

(Filled in after the runs.)

## 6. Decision

(Filled in after the runs.)
