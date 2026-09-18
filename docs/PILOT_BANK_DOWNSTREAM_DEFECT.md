# The roll arm's downstream defect (packet `cc8_bank_downstream_defect`)

Addresses: `0099E17B`, `0099E180`, `0099E1CC`, `0099E1A9`-`0099E1D1`, `0099D479`-`0099D4B3`,
`0099D300`, `008A62A0`.

`docs/PILOT_BANK_COMMAND_INPUTS.md` ended with the true `caps_rate_at_one` measured and unlanded,
because applying it sent two USN01 aircraft 60 km off mission. This packet finds why. The input was
right; **one interpolation in the roll arm had its two `y` endpoints reversed**, which pinned the
bank limit at its tightest value and hid every other bank input behind it.

## Headline

1. **`0099E180` and `0099E17B` are not defective.** The sign of `C` and the soft zone were already
   correct. Re-read with the frame walked and the two ambiguous `FMUL` forms decoded from the
   bytes, the listing agrees with `docs/PILOT_PLANNER_PITCH_ROLL.md` §1b exactly.
2. **`0099E1CC` was.** The bank limit is
   `InterpolateClamped(Pitch/1, Roll/2, Pitch/2, Roll/1, pitchError)` — it **falls** from `Roll/2`
   to `Roll/1` as the pitch error grows. The reconstruction passed `Roll/1` as `y0` and `Roll/2` as
   `y1`, so it **rose** instead. With the shipped `{DEG(15), DEG(60)}` that clamped the bank target
   to 15° whenever the pitch was on target.
3. **The `dt_scale = 1.0f` pin is not a defect.** `unit+340h` is the `luaMW_SetCheatTurbo`
   multiplier, and `max(unit+340h · 0.4, 1.0)` is exactly `1.0` for any value at or below 2.5. No
   host change is needed.
4. **With the fix, the predicate helps instead of hurting.** Every ordered aircraft closes, and
   USN02 is untouched.

## (1) `0099E180`, the sign of `C` — confirmed, not changed

The frame, anchored on `0099E110 SUB ESP,0x14` and on `00419010`'s `RET 14h` restoring it:

```
0099e0c6  [E0+18h] = L0            ; the 0099D0A0 result
0099e0ce  C = (|L0| > 0.01f) ? L0 : (L0 < 0 ? -0.1f : +0.01f)     ; 00D7A238, 00D7A300, 00CE3CB4
0099e136  FST double [E0+5Ch] = h  ; the heading error kept as a DOUBLE, no pop
0099e17b  s = InterpolateClamped(-C, -0.3C, +C, +0.3C, h)
0099e180  FSUBR double [E0+5Ch]    ; ST0 = h - s          <- the double stored at 0099E136
0099e18d  FDIV float [E0+18h]      ; ST0 = (h - s) / C
0099e199  raw = ClampFloatByRef(that, -1.0f, +1.0f)       ; lo at E0+54h, hi at E0+50h
0099e19e  raw *= maxBank
```

`0099E180` is a **reverse subtract against a double copy of `h`**, not a divide; the divide is the
separate `FDIV` three instructions later. Both were already reconstructed correctly.

The soft zone's four endpoints hinge on two instructions whose Ghidra text is ambiguous, so they
were decoded from the bytes:

| address | bytes | meaning |
| --- | --- | --- |
| `0099E14C` | `DC C9` | `FMUL ST(1),ST(0)` — **not** `D8 C9` |
| `0099E168` | `DE CA` | `FMULP ST(2),ST(0)` |

`DC C9` is what makes the argument slots come out as `x0 = -C`, `y0 = -0.3C`, `x1 = +C`,
`y1 = +0.3C`. Under the `D8 C9` reading the third push would be `-0.3C²`, which is dimensionally
impossible — that is the check that settles it. `src/plane_ai_control.cpp` already had this right.

## (2) `0099E1CC`, the defect

```
0099e1a9  FLD [ESP+50h]  / 0099e1ad FSTP [ESP+10h]   ; x  = the pitch error
0099e1b1  FLD [EBX+74h]  / 0099e1b4 FSTP [ESP+0Ch]   ; y1 = TurnRollPitchLimitRoll/1
0099e1b8  FLD [EBX+70h]  / 0099e1bb FSTP [ESP+8]     ; x1 = TurnRollPitchLimitPitch/2
0099e1bf  FLD [EBX+78h]  / 0099e1c2 FSTP [ESP+4]     ; y0 = TurnRollPitchLimitRoll/2
0099e1c6  FLD [EBX+6Ch]  / 0099e1c9 FSTP [ESP]       ; x0 = TurnRollPitchLimitPitch/1
0099e1cc  CALL 00419010                              ; InterpolateClamped(x0,y0,x1,y1,x)
0099e1d1  FSTP [ESP+18h]                             ; Lc
```

`00419010`'s signature is `(x0, y0, x1, y1, x)` and it clamps between the two **`y`** endpoints in
whichever order they come, which is why an inverted pair produces a plausible-looking curve rather
than a visible error.

`EBX = GameTuning singleton + 538h`, and `include/bsp/game_tuning_singleton.hpp:402-403` fixes the
two names: `+5ACh` is `Roll/1` (`tuning+74h`) and `+5B0h` is `Roll/2` (`tuning+78h`). So the listing
says `y0 = Roll/2` and `y1 = Roll/1`: **the limit falls as the pitch error grows.**

The installed `planeglobals.lua` line 393 is
`["TurnRollPitchLimitRoll"] = { DEG(15), DEG(60) }`, commented
"ha ennyivel kisebb a target pitch, mint a jelenlegi, akkor... max ekkora rollal kanyarodik. hogy
elobb a pitch alljon be, es csak utana kanyarodjon" — *if the target pitch is this much below the
current one, it turns with at most this much roll, so that the pitch settles first and only then
does it turn.* That only reads as a falling curve, and it corroborates the listing independently.

`src/plane_ai_control.cpp` passed `roll_1` as `y0` and `roll_2` as `y1`:

```
correct :  Lc = 60° at pitch error 0, falling to 15° beyond 12°
was     :  Lc = 15° at pitch error 0, rising  to 60° beyond 12°
```

`L = max(Lc, |bank|)` clamped to `[-2, +2]`, and `plan+2C4h = clamp(raw, -L, +L)` at `0099E23E`.
With `Lc` stuck at 15° whenever the pitch was near its target — which is most of a cruise — `L` was
the tightest clamp in the arm. That is why `cc8_bank_command_inputs` measured `maxBank` moving from
56° to 70° with no effect at all: the 15° clamp downstream swallowed it. It is also why applying the
true rate cap was catastrophic. The cap shrinks the bank command, and with the ceiling already at
15° the aircraft could not turn at all.

**The fix is the argument order only**; no constant, no branch, no new input.

## (3) `unit+340h` and `dt_scale` — no change needed

```
0099d473  ECX = plan+2F0h = the unit
0099d479  FLD [ECX+340h]
0099d481  FMUL double [00CE65D0]          ; 0.40000000596 = the float 0.4f widened
0099d491  FLD [ESP+38h] ; FLD1 ; FCOMI ; JBE
          dtScale = max(unit+340h * 0.4f, 1.0f)
0099d4b3  FDIV                            ; slot 32 = 1 / dtScale
```

A byte scan for every store form against `+340h` gives five writers, and the two that reach a unit
are both in `FUN_008A62A0` (`008A6417`, `008A6436`), whose string constants are `luakod` and
`luaMW_SetCheatTurbo failed:` and whose store is gated on `vtable[5Ch](5)`. So `unit+340h` is the
**cheat turbo multiplier**, set only from Lua, and nothing in the plane constructor writes it.

`max(x · 0.4, 1.0)` is `1.0` for every `x <= 2.5`. The host's `rin.dt_scale = 1.0f` is therefore the
exact value in normal play, not an approximation, and it only diverges if someone sets turbo above
2.5. **No host change, and no reconstruction change.** The rule is recorded here rather than added
as code, because a pure function that is constant over the whole reachable input range would be
reconstruction for its own sake.

## Validation

All runs through `./tools/run_game.ps1`, `USN01`, 3000 mission frames at 0.05 s.

| | Mav1 | Mav2 | Mav3 | Mav4 | Mav5 |
| --- | --- | --- | --- | --- | --- |
| **A** baseline, closed | -3952.0 | 1367.6 | 563.2 | 927.4 | -514.0 |
| **B** predicate only, closed | **-64529.4** | 710.8 | -793.5 | -712.2 | **-57734.3** |
| **C** limit fix only, closed | 3651.7 | 3508.7 | 3300.4 | 4112.7 | 3839.3 |
| **D** both, closed | 4006.0 | 3961.1 | 3840.0 | 4219.8 | 4114.5 |

| run | closed_mean | worst_closed | range_last_mean | heading_error_last_mean |
| --- | --- | --- | --- | --- |
| A baseline | -321.6 m | -3952.0 m | 5148.9 m | 0.147 rad |
| B predicate only | -24611.7 m | -64529.4 m | 29439.1 m | 0.477 rad |
| C limit fix only | 3682.6 m | 3300.4 m | 1144.8 m | 2.439 rad |
| **D both** | **4028.3 m** | **3840.0 m** | **799.1 m** | 1.191 rad |

Attribution, one line each:

* **B against A** is the rate cap alone, and it is the regression the previous packet refused to
  land.
* **C against A** is the argument order at `0099E1CC` alone. Every aircraft goes from a mixed result
  to closing, and `worst_closed` turns positive: the 15° ceiling is gone, so the bank target is
  finally allowed to follow the heading error.
* **D against C** is the rate cap added on top, and it now *helps* — `closed_mean` 3682.6 to 4028.3
  and `worst_closed` 3300.4 to 3840.0. The same input that was catastrophic against the broken limit
  is an improvement against the correct one, which is the strongest evidence that the limit was the
  defect and the input was never the problem.
* **`heading_error_last_mean` rises** in C and D and that is expected, not a regression. The metric
  is sampled at the last frame, and in A the flight was still 5.1 km out and flying straight at the
  target; in D it is 800 m out and manoeuvring around it. Two of the five aircraft in D end at
  0.015 and 0.048 rad. Range closed is the metric that means what it says here.

`USN02` with both changes is **unchanged**: `candidates=3421`, `shots=734`, `first_shot=1.40 s`,
`hull=180`, `deaths=2`, `total_damage=18525.6`. No gunnery path reads the pilot roll arm.

## Contracts still not landed

`src/game_hosts_units.cpp` is held by another worker, so the host predicate stays a contract. It was
applied only to produce run D and then reverted; the file is untouched in this branch.

```cpp
// replaces the dead rin.small_turn_roll_limit = false
rin.scale.caps_rate_at_one =
    bsp::unit_is_kind_of(unit_.class_id, 0x10) ||   // MPlaneBomber
    bsp::unit_is_kind_of(unit_.class_id, 0x16);     // MLargeReconPlane
```

`rin.dt_scale = 1.0f` needs **no** change, per §3.

## Follow-up

1. **`heading_error_last_mean`.** With the flight now arriving, a terminal-geometry metric would say
   more than a last-frame heading error. That is a harness question, not a reconstruction one.
2. **The other `InterpolateClamped` call sites in `0099D300`** deserve the same endpoint audit that
   found this one: `0099E07E`, `0099E17B`, `0099E4CC` and `0099E5FC`. Only `0099E1CC` was wrong
   here, but the failure mode is silent by construction, because the routine clamps between its `y`
   endpoints in whichever order they arrive.
