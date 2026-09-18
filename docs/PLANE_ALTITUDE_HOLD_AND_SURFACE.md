# The climb gain is derived, not absent, and the water line is in the arm itself

Addresses: 007C4850, 007C4BC5, 007C4BCA, 007C4BD0, 007C4BE4, 007C4BE9, 007C4BFB, 007C4C00,
007C4C08, 007C4C0E, 007C4C14, 007C4CF8, 007C4CFD, 007C4D03, 007D98F0, 007D9360, 009FB800,
007CC2F0, 007CC523, 007CC542, 007CC54C, 007CC55A, 007CC562, 007CB7F0, 007CB92C, 007C1430, 0074E210, 007CE040, 007DCDD0, 007DC830, 007DCCF0,
0078CF20, 00CEFF98, 00CF8858, 00D06850, 00D05A28, 00D7A280, 00CE3910.

Packet `cc8_plane_altitude_hold_and_surface`, owner `agent/cc8-torpedo-run-in`, on `6af4b0b53`.

Two image behaviours `docs/TORPEDO_RUN_IN_DESCENT.md` named as the reason an aircraft commanded to
12 m flies to -400 m and stays there. Neither is what it looked like.

## 1. `desc+1ECh` has a producer, and `docs/PLANE_FLIGHT.md` is refuted

**The deciding instructions are `007C4C08`-`007C4C14`.**

```
007c4bc5  CALL 0042e740                 ; the tuning singleton
007c4bca  FLD  float ptr [EAX + 0x24c]  ; Dynamics/SpdMultipliers/LevelFlight (1.8)
007c4bd0  FMUL float ptr [ESI + 0x184]  ; * desc+184h StallSpd
007c4be4  CALL 007d98f0
007c4be9  FSTP float ptr [ESI + 0x1e4]  ; desc+1E4h
007c4bf0  FLD  float ptr [ESI + 0x18c]  ; desc+18Ch TravelSpeed
007c4bfb  CALL 007d98f0
007c4c00  FSTP float ptr [ESI + 0x1e8]  ; desc+1E8h
007c4c08  FLD  float ptr [ESI + 0x1e4]
007c4c0e  FMUL double ptr [0x00ceff98]  ; 0.6
007c4c14  FSTP float ptr [ESI + 0x1ec]  ; desc+1ECh = desc+1E4h * 0.6
```

`docs/PLANE_FLIGHT.md` says of `009FB800`'s climb arm: *"`classDesc+1ECh` has no producer... So the
field is zero for every shipped class... an AI plane pitches down toward a lower cruising altitude
but never pitches up toward a higher one through this routine."* **That is wrong.** The field is
**derived at class load, not authored**, which is why a scan for a Lua key or a loader store finds
nothing: its only writer in the plane range is inside `007C4850`, the same routine that derives
`desc+50Ch` as `Accel / MaxSpd²` (`docs/PLANE_POSE_THROTTLE_ALTITUDE.md` section 2). An exhaustive
`store_census` over offset `1ECh` returns 42 sites across the image and exactly one of them,
`007C4C14`, is a plane descriptor.

`007D98F0` is a **bisection for the steepest sustainable climb angle**:

```
low = 0.0; high = 1.39626                      ; 00CF8858 float and 00D05A28 double, DEG(80)
if (1.39626 - 0.0 <= 1.745e-4) return 0.0      ; 00D06850, DEG(0.01); never taken
do {
    mid = (high + low) * 0.5                   ; 00D7A280, the double 0.5
    007D9360(mid, probeSpeed, 0.04f, 10)       ; 00CE3910 is 0.04f, and 9.0f follows it
    if (probeSpeed <= result) low = mid; else high = mid;
} while (high - low > 1.745e-4);
return low;
```

`007D9360` is a 1330-byte **trial integration** - it copies matrices through `004134F0` and
`0085DEA0` and runs the flight law - which answers with the speed the aircraft still has after ten
steps of 0.04 s. Asking whether that is at least the probe speed is asking the **sign of `dv/dt` at
the probe speed**, and the search returns the steepest climb angle for which it is not negative.

So `desc+1E4h` is the steepest climb the aircraft can hold at `LevelFlight · StallSpd`, `desc+1E8h`
the same at `TravelSpeed`, and the climb arm's gain `desc+1ECh` is **0.6 of the first**.

### Bound, with the predicate substituted at its address

`max_sustainable_climb_angle_007d98f0` in `src/plane_flight.cpp` keeps the native's bounds, its
`0.5` midpoint, its `DEG(0.01)` tolerance and its acceptance test. **SUBSTITUTION, labelled**:
`007D9360` is not reproduced. Its ten steps test the sign of `dv/dt`, and this host's own law gives
that sign in closed form, so the predicate is

```
thrust - drag(v) - AccelCheatMul * 9.81 * sin(angle) >= 0
```

with `thrust = Accel · throttle`, `drag(v) = (Accel / MaxSpd²) · v²` - the same `desc+50Ch` the
loader derives - and the trial throttle taken as full, because `007D9360`'s own throttle is unread.

For USN01's `Mav` row (`Accel` 6, `MaxSpd` 69.44, `StallSpd` 19.4, `LevelFlight` 1.8) the probe
speed is 35.0 m/s, the drag there is 1.52 m/s², and the answer is `asin(4.48 / 14.715)` = **0.3091
rad**, so `desc+1ECh` = **0.1855 rad**, 10.6 degrees.

That is enough to recover the overshoot. At -400 m against a 12 m target the climb arm gives
`min(0.1855 · 12, max(0.1855 · 1.6, DEG(40)))` = **0.698 rad**, a 40-degree climb, where before it
gave exactly zero.

## 2. The water line is inside the free-flight arm, and the gate is the flight state

**The deciding instruction is `007CB92C`'s tail, `BSP_Plane_SetFlightState(6)`.**

The brief's premise was that `ctl+FCh` selects the arm. It does not. A `store_census` over `FCh`
shows `ctl+FCh` written by each of the three law entry points **on entry** - `007DC841` in
`007DC830 FreeFlightStep`, `007DCD24` in `007DCCF0 GroundRollLaw`, `007DCDDC` in `007DCDD0
WaterSurfaceLaw` - so it is a tag the arm sets, not the selector.

The selector is `unit+900h`, the flight state, through `0074E210`'s `unit+900h == 7` at
`007CEC3F`. And what changes it is inside the **free-flight arm itself**:

```
007cc523  CALL 0078CF20                  ; h = the sea under the aircraft, from unit+0FCh/+104h
007cc542  FLD  float ptr [EDI + 0x194]   ; SwimHeight
007cc548  FADD float ptr [ESP + 0x44]    ; + h
007cc54c  FADD float ptr [EDI + 0x508]
007cc55a  FCOMIP ST0,ST1                 ; the line against unit+100h
007cc55e  JBE  007cc567                  ; line <= altitude: no contact
007cc562  CALL 007cb7f0
```

and `desc+508h` is derived by the same `007C4850`:

```
007c4cf8  FLD  float ptr [EDI + 4]
007c4cfb  FCHS
007c4cfd  FSUB float ptr [ESI + 0x194]    ; SwimHeight
007c4d03  FSTP float ptr [ESI + 0x508]    ; desc+508h = -[EDI+4] - SwimHeight
```

**The two `SwimHeight` terms cancel.** The real line is `h - [EDI+4]`, and `[EDI+4]` is the model
bound, so the test is the aircraft's **lowest point** touching the water.

`007CB7F0` then, at `007CB92C`:

```
state = unit+900h
if (state != 6) {
    if (session mode != 2)  route message 0C3h carrying the old state
    else if (state == 7 || state == 4 || state == 5)  BSP_Plane_SetFlightState(6)
}
```

So a plane that touches the water leaves free flight for **state 6**, `0074E210` answers false, and
`007CE040` stops selecting `007CC2F0` and selects the surface arm `007CBA50` instead. Above that,
`007CB7F0` also raises a `"powerlost"` effect and, under a chain of conditions that includes
`unit+1B0h < 8` and `BSP_UnitInstance_IsAliveAndVisible`, calls `0090F6C0(unit, 3)`.

### Bound, with two substitutions at their addresses

The free-flight arm now samples the sea and applies the test, and on a contact it sets the flight
state to 6, which is what `007CB7F0`'s tail does. **SUBSTITUTION, labelled, twice**: `[EDI+4]` is
the model bound this host does not carry, so it stands at zero and the test is the aircraft's
**origin** crossing the sea rather than its belly - a metre or two for a torpedo bomber; and
`007CB7F0`'s `"powerlost"` effect, its `0090F6C0(unit, 3)` call and the `0C3h` session message are
**contracts, unread**.

What happens after contact is honest rather than invented: the host's `surface_007cba50` binding is
a counter, so the aircraft stops being integrated at the water instead of continuing to -400 m. The
surface law `007CBA50` and the water arm `007DC205`-`007DC68C` inside `007DB680` are **unread**, and
`docs/PLANE_FLIGHT_CORE_LAW.md` already records the second as such. No ad-hoc altitude clamp was
added.

## ABI

* `007D98F0`, `float __thiscall(desc, float probeSpeed)`, body `007D98F0`-`007D99B6`.
* `007D9360`, `__fastcall(outSpeed /* ECX */, ? /* EDX */, float angle, float probeSpeed, float
  step, int steps)`, body `007D9360`-`007D9892`. **Unread beyond its head and its role.**
* `007CB7F0`, `void __fastcall(unit)`, body `007CB7F0`-`007CB9D2`.
* `007C4850` `BSP_PlaneClass_DeriveFlightConstants`, body `007C4850`-`007C4D86`; this packet reads
  `007C4BC5`-`007C4C14` and `007C4CF8`-`007C4D03`.

## Uncertainty

* `007D9360`'s body, and therefore the exact quantization of `desc+1E4h`. The closed-form predicate
  agrees with it in sign by construction, not in its last digits.
* `007D9360`'s trial throttle, taken as full.
* `[EDI+4]`, the model bound in the water line.
* The surface law `007CBA50` and the water arm `007DC205`-`007DC68C`: an aircraft in state 6 is not
  integrated at all here.
* `007CB7F0`'s damage and effect paths.
* `desc+1E8h`, the `TravelSpeed` climb angle, is derived but has no reader in anything read so far.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `max_sustainable_climb_angle_007d98f0` | `src/plane_flight.cpp` | `007D98F0` | binding, predicate `007D9360` **substituted** |
| `desc+1E4h` and `desc+1ECh` at spawn | `src/game_hosts_units.cpp` | `007C4BE9`, `007C4C14` | binding |
| `pin.class_climb_angle` | `src/game_hosts_units.cpp` | `009FB800`'s `class+1ECh` | binding |
| the water line and the state change | `src/game_hosts_units.cpp` | `007CC562`, `007CB92C` | binding, `[EDI+4]` **substituted** |
| `SwimHeight` row key | `src/game_hosts_lua.cpp` | `desc+194h`, `007D2413` | binding |

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/PLANE_FLIGHT.md`, "`009FB800`, the pitch command from an altitude error". Its
  `classDesc+1ECh` paragraph is refuted: the field is derived at `007C4C14`, not absent, and the
  climb arm does command a climb.
* `docs/TORPEDO_RUN_IN_DESCENT.md` section 4, which repeated that claim as the first of its two
  reasons for the -400 m overshoot, and section 1's last paragraph.
* `docs/PLANE_FREE_FLIGHT_PHYSICS.md` gains the water line: the free-flight arm `007CC2F0` carries
  its own exit test at `007CC523`-`007CC562`, which no section of that doc records.

## no_ghidra_function

None. `007D98F0`, `007D9360`, `007CB7F0`, `007C4850`, `007CC2F0` and `007C1430` all have Ghidra
functions.

## Validation

`tools/run_game.ps1`, 3200 frames, `--mission-frames 3000` at `0.05` s, from this worktree at
`6af4b0b53`. The before column is the same source with the climb gain forced to zero and the water
test switched off by a temporary compile-time constant, removed before the commit.

**These baselines are not comparable with `docs/TORPEDO_RUN_IN_DESCENT.md`'s.** The merge of main
that produced `6af4b0b53` moved USN01 a long way: the ordered aircraft now start 1488 m from their
targets rather than 4117 m, and the mission scores **zero hits and zero damage in both columns**,
where the descent packet measured 68 and 660.0. Nothing in this packet caused that and nothing here
can, since both columns share it.

### USN01

| | before | after |
| --- | --- | --- |
| `desc+1ECh`, derived | 0.1854 rad (computed, unused) | **0.1854 rad (used)** |
| torpedo drops | **5** | **0** |
| water-entry breakups | 0 | 0 |
| `swims_started` | 0 | 0 |
| torpedo hits, torpedo damage | 0, 0 | 0, 0 |
| plane arm, free flight | 60000 | 48189 |
| plane arm, **surface** | **0** | **11811** |
| water contacts | 0 | **5** |
| lowest altitude reached | **-401.2 m** | **-5.8 m** |
| `distance_moved` | 186434 m | 145331 m |
| `pose_rotations` | 10463 | 2870 |
| mission hits / kills / damage | 0 / 0 / 0.0 | 0 / 0 / 0.0 |

**The water line works and the aircraft stop at the sea.** Five aircraft cross it and take the
`007CB92C` transition from state 7 to state 6, `007CE040` selects the surface arm 11811 times where
it selected it zero times before, and the lowest altitude any aircraft reaches goes from -401.2 m to
-5.8 m. The -400 m hole `docs/TORPEDO_RUN_IN_DESCENT.md` measured is closed.

**The derived climb gain is confirmed and is not what recovers the dive.** The census prints
`climb_1ec=0.1854` in both columns - it is computed either way, and the switch only decides whether
`009FB800` reads it - and 0.1854 rad is the analytic answer for the `Mav` row to four places. But it
never acts, because the aircraft never climbs: `009FB800`'s demand stays at the `DEG(60)` cap for
the whole descent and the aircraft reaches the water before the flare.

**And that is the regression: the drops go from 5 to 0.** The trace says why:

| n | commanded | live altitude | pitch demand |
| --- | --- | --- | --- |
| 1 | 12.00 | 800.0 | -1.0472 |
| 51 | 12.00 | 686.6 | -1.0472 |
| 101 | 12.00 | 194.9 | -1.0472 |
| - | - | **-4.80, `\|v\|` 141.50 m/s** | water contact |

The aircraft arrive at the sea doing **141.5 m/s**, twice their `MaxSpd` of 69.44, because a
60-degree dive held for 800 m outruns the drag. Before this packet they carried that speed through
the surface to -400 m and released there; now they stop at the surface, in state 6, and this host's
`surface_007cba50` is a counter, so they are not integrated further and never reach a release.

**Neither column drops a torpedo that swims, and the after column is the more correct of the two.**
A release at -230 m was never going to swim; an aircraft that ditches at the water is what the image
does with one that arrives there.

### USN02

168 hits, 3 kills, 20721.4 damage, `swims_started = 44`, and **0 water contacts**, so the new branch
never executed. Its numbers are not comparable with the previous packet's for the same merge reason.

### Still no swim, and the next gate by address and value

**The next gate is step 4's throttle half, which `docs/TORPEDO_RUN_IN_DESCENT.md` left unbound.**
`docs/BOT_TASK_STATES.md` records it: the same step that commands the altitude also commands a
throttle from `InterpolateClamped([00D7A2F0] 0.1, [00CF6560], [00CE7804] 0.4, [00CE74F8], .)` over
the height margin `[00D1F8D0] - unitY`, clamped against `min(approach+90h, [00CF0DD8])`, with the
second argument `approach+7Ch` above `approach+134h >= [00CF3F20]` and `approach+80h` below. The
native cuts the throttle as the aircraft descends, and `007D9050`'s whole thrust term is gated at
`007DB76C` on the latched throttle exceeding `0.01f`, so a cut throttle is what keeps the dive from
running away. This host flies the dive at full throttle and arrives at twice `MaxSpd`.

The aim state cannot save it either: `009D20B4`'s release gate lies between 25 and 40 m, and an
aircraft crossing that band at 141 m/s is inside it for 0.1 s, less than the 0.09 s pilot think
interval plus its own jitter. Slowing the run-in is what puts the release gate back in reach.

## Follow-up packets

0. **Step 4's throttle half of `009D07B0`.** The aircraft arrive at the sea at twice `MaxSpd`
   because this host flies the dive at full throttle. `007D9050`'s thrust is gated at `007DB76C` on
   the latched throttle exceeding `0.01f`, so the native's throttle cut is what keeps the dive in
   hand, and it is what puts `009D20B4`'s 25 to 40 m release gate back within reach. This is the
   gate.
1. **The surface law `007CBA50` and the water arm `007DC205`-`007DC68C`.** A plane in state 6 is
   not integrated at all in this host. This is what a ditched aircraft does.
2. **`007D9360`**, which would retire the substituted predicate and give `desc+1E4h` exactly.
3. **`007CB7F0`'s damage path**, `0090F6C0(unit, 3)` and the `"powerlost"` effect.
4. **The model bound `[EDI+4]`**, which is the difference between the belly and the origin in the
   water line.


## Correction from packet `cc8_torpedo_throttle_cut`: follow-up 0 is void

Appended, not rewriting the sections above.

Follow-up 0 names "step 4's throttle half of `009D07B0`" as the gate, on the reasoning that the
native's throttle cut is what keeps a 60-degree dive in hand. **There is no throttle cut.**
`009D0A6B` stores step 4's interpolation as `009FBA50`'s `scale` argument, which is read only when
`span > 0`, and both range arguments from this call site are `approach+90h`, so it is discarded.
No routine in the torpedo chain writes the plan's throttle slot.

The measured arrival at 141.5 m/s therefore has a different cause, and the Validation section's
reading of it stands otherwise: the aircraft enter `attackrun` at their 800 m spawn altitude,
because this host ticks only the `aim` and `attackrun` states and never `moveto`, whose `009C18C0`
would command `Pilot/Torpedo/CruisingAlt`. `docs/TORPEDO_THROTTLE_CUT.md`.
