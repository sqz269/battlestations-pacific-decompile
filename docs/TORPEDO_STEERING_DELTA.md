# Torpedo steering delta: the heading conventions, and what really blocks the drop

Addresses: 009D1699, 009D1679, 009D15F0, 009D1500, 0074E260, 006DFD60, 007CFD20,
007CFD78, 007D03C1, 00D05F20, 00438B10, 009D3586, 009D35C0, 0099DEB8, 007C1900,
009D2175, 009D21AC, 009D21F2, 009D2209, 009D2239, 009D2287, 009D229D, 009D0D90.

Packet `cc8_torpedo_steering_delta`. The packet was written on the hypothesis that the
steering delta at `009D1699` sits at 2.28-2.34 rad because of a convention mismatch: a
compass-versus-mathematical angle, a pi or pi/2 offset, a sign flip, or the delta
measured against a heading the planner does not steer. **The listing refutes all four.**
The aim tick and the pilot planner subtract against the same field, with the same zero
direction and the same sense, and the reconstruction of both already carries it. What
blocks the drop is a gate that is much tighter than the packet assumed, plus an
aim/goaway cycle that reverses the commanded heading roughly every 1.7 s.

## The convention table

Every heading in the chain is the compass angle measured from `+Z` toward `+X`, i.e.
`atan2(dx, dz)`, expressed in the source as `pi/2 - atan2(dz, dx)`. There is no second
convention anywhere on the path.

| Field | Producer | Zero direction | Sense | Range |
|---|---|---|---|---|
| `unit+C6Ch`, the plane heading | `007C1900`, flatten then `pi/2 - atan2(flat_fz, flat_fx)` | `+Z` | toward `+X` | `(-pi, pi]`, via `00438B10` |
| `unit+1050h`, the ship hull heading | `00826C56`, `atan2` over world row 2 (docs/SHIP_AI_RUDDER_HOP.md) | `+Z` | toward `+X` | `atan2` principal |
| `approach+94h`, the bearing to the target point | `009D35C0`, `pi/2 - atan2(dz, dx)`, single `2*pi` add when negative | `+Z` | toward `+X` | `[0, 2*pi)` |
| `plan+2C0h`, the commanded heading | `009D1D16` from `009D1BCC` = `WrapAdd(vtable50(), delta)` | `+Z` | toward `+X` | `(-pi, pi]` |
| `f10h`, the steering delta | `009D1699` = `00438B10(approach+94h, vtable50())` | n/a | positive = turn toward `+X` | `(-pi, pi]` |

The `[0, 2*pi)` range of `approach+94h` against the `(-pi, pi]` range of the heading is
harmless: `00438B10` wraps the difference, so the two ranges cannot produce an offset.

## Rule 1: vtable slot 50h is the plane's own heading, not the hull heading

This is the instruction that decides the packet.

```
0074e260  d9 81 6c 0c 00 00   fld dword ptr [ecx + 0xc6c]
0074e266  c3                  ret
```

`0074E260` is a two-instruction leaf with no Ghidra function. It occupies **slot 50h** in
nine vtables. One of them is `00D05F20`, and `BSP_PlaneUnitInstance_Construct` (`007CFD20`)
installs that vtable into the object at `007CFD78`. So on an aircraft, slot 50h returns
`unit+C6Ch`. The same vtable is stored again at `007D03C1`, which is inside
`007D03A0 CG_vector_deleting_dtor_007d03a0`, the destructor re-installing its own class
vtable; that corroborates the class but is not a second constructor install.

The ship override of the same slot is `006DFD60 BSP_UnitInstance_GetHullHeading`,
`FLD dword ptr [ECX+1050h] / RET`, which occupies slot 50h in nine other vtables
(`00CF90B0`, `00CFA778`, `00CFB738`, `00CFC3D0`, `00CFFA30`, `00D01630`, `00D09678`,
`00D0BF80`, `00D0C648`); `00D09678` is installed at `0081ED80`, inside
`0081ED40 BSP_UnitVehicleBase_Construct`, and again at `0081F3C2`, whose enclosing body
this packet did not establish.

Both install-site attributions above were wrong in this packet's first draft, which read
them off `bsp.py lookup`; that reports the nearest preceding function, not containment.
`tools/verify_report_calls.py` resolved the real enclosing bodies and is what corrected
them. Ghidra's own body range then confirmed the rest: `BSP_PlaneUnitInstance_Construct`
is `007CFD20`-`007D0344`, so `007CFD78` is inside it and `007D03C1` is past its end,
which settles the correction independently.

Four of the nine vtables carrying `0074E260` are attributed to named aircraft
constructors, each storing the vtable 13 to 15 bytes into its body:

| Vtable | Store site | Constructor | Ghidra body |
|---|---|---|---|
| `00D00070` | `0074E0FF` | `BSP_ReconPlaneUnitInstance_Construct` | `0074E0F0`-`0074E15E` |
| `00D00308` | `0074E2DD` | `BSP_LargeReconPlaneUnitInstance_Construct` | `0074E2D0`-`0074E33C` |
| `00D05F20` | `007CFD78` | `BSP_PlaneUnitInstance_Construct` | `007CFD20`-`007D0344` |
| `00D0BA80` | `0084C92D` | `BSP_SmallReconPlaneUnitInstance_Construct` | `0084C920`-`0084C98C` |

The other five are the same shape with unnamed constructors: `00D06638` at `007D772F` in
`FUN_007D7720` (`007D7720`-`007D778E`), `00D06920` at `007DD9BF` in `FUN_007DD9B0`
(`007DD9B0`-`007DDA1E`), and `00D19D28`, `00D1A000`, `00D1A2D8` at `00951B6F`, `00951C4F`
and `00951D2F`. Every containment above is Ghidra's body range, not `lookup`'s nearest
preceding function.

RTTI is stripped in this executable: the dword before each vtable base is zero or unrelated
data, and following it as a Complete Object Locator yields no class name. The vtable bases
above were therefore recovered from the `mov dword ptr [reg(+disp8)], imm32` stores in
`.text` whose immediate lands in `.rdata` (3311 of them), taking for each occurrence the
largest such immediate at or below it. Under that rule both getters land on slot `0x50`
exactly, which is itself a check on the method: a wrong base would not put eighteen
occurrences of two functions on one slot index.

`include/bsp/plane_ai_control.hpp:226` already recorded the plane override for the planner
side. This packet confirms it independently from the vtable bases and extends it to the aim
tick, where it had not been established.

## Rule 2: the aim tick and the planner subtract the same field

`BSP_BotStateTorpedoAim_Tick` (`009D15F0`, raw listing, no Ghidra function):

```
009d15f7  mov  edi, [esi+4]          ; the approach block
009d161a  movss xmm0, [edi+0x94]     ; the bearing
009d1622  movss [esp+0x1c], xmm0
009d166b  mov  ecx, [edi+4]          ; the aircraft
009d166e  mov  eax, [ecx]
009d1670  mov  edx, [eax+0x50]       ; -> 0074E260 on a plane
009d1679  call edx                   ; ST0 = unit+C6Ch
009d167b  fstp [esp+0x20]
009d167f  fld  [esp+0x20]
009d1683  sub  esp, 8
009d1686  fstp [esp+4]               ; second argument
009d168a  fld  [esp+0x24]            ; = old [esp+0x1c], the bearing
009d168e  fstp [esp]                 ; first argument
009d1691  call 0x438b10
009d1699  fstp [esp+0x10]            ; F=10h, the steering delta
```

The `sub esp, 8` shifts every following `[esp+n]`, so `[esp+0x24]` after it is the
pre-subtraction `[esp+0x1c]`, which `009D1622` set to `approach+94h` and nothing overwrites
in between. `00438B10` ends `ret 8` at `00438B4C` and `00438B7B`, so the callee cleans both
arguments and `[esp+0x10]` at `009D1699` is a frame slot, not a shifted one.

`BSP_Math_SubtractWrappedAngle` is `FLD [ESP+4] / FSUB [ESP+8]` at `00438B10`-`00438B14`,
so it is `first - second`. The delta is therefore

```
f10h = WrapPi(approach+94h - unit+C6Ch)
```

and the planner's yaw base term at `0099DEB8` is `WrapPi(plan+2C0h - unit+C6Ch)`. Same
minuend convention, same subtrahend field. Since `009D1BCC` writes
`plan+2C0h = WrapAdd(unit+C6Ch, f10h)`, the planner's heading error and the aim tick's delta
are the same number by construction. **Nothing is lost at any hop.**

## Rule 3: the cone gate is 15 degrees for the whole run-in

`009D2209` is the flag the packet is about. Its argument order, read off the push slots:

```
009d2175  fcompi st(1)                 ; f0c_time against 0.8 (00CE74F8)
  near branch, 009d217d-009d21ac:  00419010(0.3, 80.0, 0.8, 50.0, f0c)
  far  branch, 009d21c3-009d21f2:  00419010(0.8, 50.0, 1.6, 15.0, f0c)
009d21b1  fmul qword [0xce3d28]        ; * pi
009d21b7  fdiv qword [0xce3d20]        ; / 180
009d2209  fcompi st(1) / jbe           ; gate = cone_rad > |delta|
```

Both argument orders match `src/torpedo_aim_tick.cpp:312-320` exactly; the pair is not
reversed. `00CE3D28` is the same `pi` that `00438B51` uses as the wrap's upper bound.

The consequence the packet brief did not have: the cone is **80 degrees only below 0.3 s to
target, 50 degrees at 0.8 s, and 15 degrees at and beyond 1.6 s.** For every part of a
run-in that is more than 1.6 s from the target the gate demands the nose within
15 degrees = 0.262 rad, not "a few tens of degrees". The measured `|delta|` of 2.28 rad is
8.7 times the gate.

## ABI

| Routine | Convention | Arguments | Result |
|---|---|---|---|
| `0074E260` | `__thiscall`, ECX = unit | none | ST0 = `unit+C6Ch`, plain `RET` |
| `006DFD60` | `__thiscall`, ECX = unit | none | ST0 = `unit+1050h`, plain `RET` |
| `00438B10` | callee-cleaned, two `float` on the stack | `[ESP+4]` minuend, `[ESP+8]` subtrahend | ST0, `RET 8` |
| `009D15F0` | `__thiscall`, ECX = bot state | one stack argument | `RET 4` |
| `009D1500` | `__thiscall`, ECX = approach block | none | ST0 = `+90h / speed` |

## Host methods

| Host method | Native | State |
|---|---|---|
| `AimTickBinding::unit_heading_vtable50` | `009D1679` -> `0074E260` | **wrong producer**, see Contract |
| `PlaneMotion::refresh_attitude_007c1900` | `007C1900` | correct, writes `plane_heading_c6c` |
| `PilotBot::plan_yaw_0099d300` | `0099D300`, `0099DEB8` | correct, subtracts `plane_heading_c6c` |
| `TorpedoApproach::bearing_009d3586` | `009D35C0` | correct |

## Contract: the one host divergence at this hop

`src/game_hosts_units.cpp:3403`, inside `AimTickBinding`, binds the native's
`call [eax+50h]` to `owner_.pose_heading_radians(unit_)`, which is
`atan2(motion.pose_row2[0], motion.pose_row2[2])` — the producer of the **ship** field
`unit+1050h`. On a plane that slot is `0074E260` and the value is `unit+C6Ch`, which this
host already computes and caches as `unit_.plane_heading_c6c` for the planner.

The edit, for whoever holds the file (it was leased to `agent/cc8-recon-binding` until
2026-09-18T12:23 for the whole of this packet's turn):

```cpp
float unit_heading_vtable50() override {
    // 009D1679 calls slot 50h; on 00D05F20, the vtable
    // BSP_PlaneUnitInstance_Construct installs at 007CFD78, that slot is
    // 0074E260 FLD [ECX+0C6Ch] / RET.  Not the ship override 006DFD60.
    return heading_;   // set from unit_.plane_heading_c6c, not pose_heading_radians
}
```

and at `src/game_hosts_units.cpp:3484`, `binding.heading_ = unit_.plane_heading_c6c;`.

**Do not expect this to move the delta.** Both expressions reduce to `atan2(fx, fz)` over
the same forward row: `007C1900` flattens the pose by rotating it about `(-fz, 0, fx)/h` by
`-pitch`, which preserves the horizontal direction with a positive factor, so
`pi/2 - atan2(flat_fz, flat_fx)` equals `atan2(fx, fz)`. The fix is for faithfulness and
for the near-vertical latch that `007C1900` has and a bare `atan2` does not: the native
leaves the previous heading in place when the forward axis is near vertical, and
`pose_heading_radians` instead returns a heading that swings freely there.

## Validation

USN01, this worktree, commit before any source change, 3000 mission frames at 0.05 s.
No source change was made, so there is no after-run to compare: the packet's verdict is
that the reconstruction on this path is already faithful.

`summary mission pilot attack`: ordered 5, range first mean 4827.3 m, range last mean
920.9 m, heading error first mean 2.188 rad, heading error last mean 0.918 rad.

Per aircraft at the first aim-complete (`009D236E`), all five on the `|delta|` clause:

| Aircraft | first true at aim tick | F18 = \|delta\| | F0C = time to target | F14 = range | cone at that F0C | releases |
|---|---|---|---|---|---|---|
| Mav1 | 276 | 2.2811 | 2.2792 | 1267.52 | 0.262 rad | 0 |
| Mav2 | 267 | 2.2801 | 2.2319 | 1239.16 | 0.262 rad | 0 |
| Mav3 | 276 | 2.3390 | 2.2537 | 1252.22 | 0.262 rad | 0 |
| Mav4 | 271 | 2.2808 | 2.2722 | 1263.31 | 0.262 rad | 0 |
| Mav5 | 275 | 2.2906 | 2.2905 | 1274.31 | 0.262 rad | 0 |

`release_arm_009D2287 = 0`, `timer_on = 0`, `timer_fires = 0` on all five, so the cone flag
never set and the chain never reached `009D2287`.

The aim/goaway cycle, same run:

| Aircraft | goaway enters | range peak in goaway | break off `+24h` |
|---|---|---|---|
| Mav1 | 96 | 697.4 | 700.0 |
| Mav2 | 91 | 702.3 | 700.0 |
| Mav3 | 90 | 697.0 | 700.0 |
| Mav4 | 87 | 766.5 | 700.0 |
| Mav5 | 98 | 700.4 | 700.0 |

Four of the five never clear the 700 m break-off distance while in goaway, so goaway
re-enters at once. Ninety entries in 150 s is one reversal of the commanded heading every
1.7 s. `summary mission plane motion` for the same run: `pose_rotations=12799` of 60000
steps, `heading_change=29.086 rad`, `yaw_plans=6495`. The five ordered aircraft account for
1299 yaw plans each. A nose that is commanded to reverse every 1.7 s cannot converge into a
15-degree cone, whatever the delta's convention.

USN02, same tree and same settings, `local/usn02_before.log`, `loop_finished=1`,
`exit_code=0`. It exercises none of this path: `summary mission plane motion` reads
`distance_moved=0.00 m pose_rotations=0 thinks=0 commits=0 yaw_plans=0`, and
`summary mission pilot attack` reads "no unit was ever ordered at a target the yaw arm
could plan for". So USN02 cannot show a regression in the aim tick either way, and with no
source change there is nothing for it to regress.

## Corrections

To append to `docs/TORPEDO_AIM_TICK.md`, not to rewrite: the frame table's `F=10h` entry
should name the subtrahend as `unit+C6Ch` reached through vtable slot 50h `0074E260`, not
the hull heading `unit+1050h` / `006DFD60`. The two are different overrides of one slot and
only the ship one was previously recorded.

To append to `docs/TORPEDO_FIRST_RELEASE.md`: the "Validation" section's reading of the
2.28-2.34 rad delta as a convention signature does not survive the listing. The delta is
correct; the gate it is tested against is 15 degrees rather than a few tens of degrees, and
the aircraft's commanded heading reverses about every 1.7 s.

## no_ghidra_function

| Address | Inclusive end | What it is |
|---|---|---|
| `0074E260` | `0074E266` | `BSP_PlaneUnitInstance_GetHeading`, `FLD [ECX+0C6Ch] / RET`, 7 bytes, slot 50h of the plane vtables |
| `009D15F0` | `009D2377` | `BSP_BotStateTorpedoAim_Tick`, already recorded by the aim-tick packet |

`006DFD60`, `007CFD20`, `00438B10` and `009D1500` all have Ghidra functions and names.

## Uncertainty

- The nine vtables that carry `0074E260` are asserted to be aircraft classes on the strength
  of one of them, `00D05F20`, being installed by `BSP_PlaneUnitInstance_Construct`, and on
  `unit+C6Ch` being a field only the plane attitude block owns. The other eight are not
  individually attributed. Partial.
- `F0C` of about 2.27 s at a range of 1263 m implies a speed slot near 556 m/s, which is not
  an aircraft speed. `009D1500` is `approach+90h / (+7Ch or +80h)`, and the clamp at
  `009D3445` against the pilot control block's `+39Ch` ceiling has no modelled input in this
  host, so the raw class speed survives. It does not change the cone, which is already at
  its 15-degree floor for any plausible speed at this range, but it does let the countdown
  gate `kCountdownGate 3.0 > f0c` pass when the true value would fail it. Open.
- The per-tick census the packet asked for was not produced. `--trajectory-csv` is ship-only:
  `refresh_row` is never called on the plane path, so all 3001 rows for each Mavis carry the
  spawn position and heading `0.0`. Producing it needs `src/game_hosts_units.cpp`, which was
  leased elsewhere for this packet's whole turn. The figures above come from the summary
  census, which is per-run rather than per-tick.

## Follow-up packets

1. **The aim/goaway hysteresis. This is the gate now, by address and value.**
   `BSP_BotStateTorpedoGoAway_IsComplete` (`009D3150`, body `009D3150`-`009D31A5`,
   `__fastcall(state)`, sole caller `009D4030 BSP_BotTaskTorpedo_TransitionRule`) returns
   `state+24h < approach+90h`: goaway ends only once the range exceeds the break-off
   distance. In the USN01 before-run `state+24h` is 700.0 on all five aircraft, from
   `0042E740+438h`, and `approach+90h` peaks inside goaway at 697.4, 702.3, 697.0, 766.5 and
   700.4. Four of the five never satisfy it, yet goaway is still entered 87-98 times each,
   so the aircraft are leaving goaway by some route other than this predicate and
   re-entering at once. Find that route in `009D4030` and establish whether the native has a
   hysteresis band this host drops. Until the commanded heading holds for more than 1.7 s,
   no cone of any width will close.
2. **The approach speed ceiling.** Model `ctl+39Ch` so `009D3445` clamps `+80h` and rescales
   `+7Ch`, then re-measure `F0C` and the `009D229D` countdown gate.
3. **The plane row refresh.** Call `refresh_row` on the plane motion path so
   `--trajectory-csv` covers aircraft; every future plane packet needs it.
4. **The yaw turn numerator.** `scratch.turn_num` is hardcoded to `0.0f` at the planner
   binding, so `0099E888`'s coordinated-turn block never runs and the yaw demand collapses to
   zero whenever the bank exceeds `tuning+80h`. `yaw_turn_numerator_0099e69b` is reconstructed
   and unused.
