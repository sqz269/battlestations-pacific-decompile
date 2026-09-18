# Nothing in the pitch path stops the plunge, and the host is faithful to it

Addresses: 0099DC9E, 0099DCB2, 0099DCB8, 0099DCC8, 0099DCD0, 0099DCD4, 0099DCD6, 0099DCE0,
0099DCE7, 0099DCF1, 0099DD04, 0099DD0F, 0099DD19, 0099DD23, 0099DD2C, 0099DD35, 0099DD42,
0099DD46, 0099DD4A, 0099DD4C, 0099DD4E, 0099DD54, 0099E3D1, 0099E490, 0099E496, 0099E49A,
0099E4AA, 0099E4C2, 0099E4CC, 0099E4DC, 0099E4E4, 0099E4E8, 0099E4FA, 0099E4FE, 0099E512,
009FB800, 009FB96E, 007C4810, 00CE3DE0, 00D1F3D8, 00CEDF5C, 00CE398C, 00D05AAC.

Packet `cc8_attack_run_descent`, owner `agent/cc8-torpedo-run-in`, on main `103a67984` merged.

**Negative result, and it retires a hypothesis.** Nothing in the planner's pitch path stops a
60-degree dive for an aircraft that is level and on heading, in the image. The host's measured
`-1.07` rad is faithful to the native's own chain, constant for constant, so "the host dives wrong"
is not the explanation for the 800 m plunge. No code changed.

## 1. The descent profile, as an equation with every constant sourced

For an attack run commanded to altitude `A` from altitude `h`, with the aircraft level and on
heading:

```
err       = min(A, Ceiling - 50) - h                                  ; 009FB849
weighted  = err * ((A + 1) * 0.5)                                     ; 009FB858
t         = clamp(-weighted / DropDist, 0, A)                          ; 009FB9B8
demand    = -min(DropAngle * t, max(DropAngle * 1.6, DEG(60)))         ; 009FB96E
target    = min(demand, heldPitch + inc)          [pitch mode 2]       ; 0099DD4C
target    = max(target, PitchTurnMaxPitch - 2.5 * (1 - q))             ; 0099E4FE
  where q = clamp(|bank| / class+25Ch TurnRoll, 0, 1)
            * InterpolateClamped(PitchTurnHdgRange1, 0, PitchTurnHdgRange2, 1, |hdgErr|)
```

| constant | source | value |
| --- | --- | --- |
| `DEG(60)`, the dive cap | `00D05AAC` | 1.0471976 |
| `1.6`, the cap scale | `00CE3D48` | 1.6 |
| `DropDist` | `Pilot/General/DropDist` | 200 |
| `DropAngle` | `desc+1F0h`, authored | 0.610865 (TBD), 0.4014 (Mav) |
| `2.5`, the floor depth | `00CE3DE0`, a double | 2.5 |
| `PitchTurnMaxPitch` | `Pilot/General`, authored | `DEG(6)` = 0.10472 |
| `PitchTurnHdgRange` | `Pilot/General`, authored | `DEG(25)`, `DEG(50)` |
| mode 2's demotion speed | `00D1F3D8` | 2.7778 m/s = 10 km/h |
| mode 2's increment endpoints | `00CEDF5C`, `00CE398C` | `DEG(5)`, `DEG(20)` |

**Every one of these is an image constant or an authored row. None is a host substitution.**

## 2. Pitch mode 2 limits a climb, not a dive

`0099DC9E`-`0099DD54`, with the jump senses taken from the listing:

```
0099dc9e  MOV EAX,[ESI + 0x2d0]          ; the pitch mode
0099dcb2  JZ  0099dd94                   ; mode 0 skips the whole pitch path
0099dcb8  CMP EAX,0x2
0099dcc8  CALL EDX                       ; unit vtable+38h, the cached speed
0099dcd0  FCOMIP ST0,ST1                 ; [00D1F3D8] = 2.7778 vs the speed
0099dcd4  JBE 0099dce0                   ; 2.7778 <= speed -> stay in mode 2
0099dcd6  MOV dword ptr [ESI + 0x2d0],1  ; below 10 km/h -> DEMOTE to mode 1
0099dce7  JNZ 0099dd54                   ; mode 1 -> the else branch
0099dcf1  FLD float ptr [ECX + 0xc84]    ; unit+C84h, the held pitch
0099dd35  CALL 00419010                  ; inc, from the speed against DEG(5)..DEG(20)
0099dd42  FADD float ptr [ESP + 0x14]    ; sum = inc + held
0099dd46  FLD  float ptr [ESP + 0x10]    ; the commanded target
0099dd4a  FCOMIP ST0,ST1
0099dd4c  JBE 0099dd62                   ; commanded <= sum -> KEEP the commanded
0099dd4e  FSTP float ptr [ESI + 0x2bc]   ; else plan+2BCh = sum
```

So mode 2 gives `plan+2BCh = min(commanded, heldPitch + inc)`. A dive command is very negative and
`sum` is the held pitch plus a positive increment, so `commanded <= sum` holds and **the dive passes
through untouched**. Mode 2 is a rate limiter on the way *up*, and it stops mattering below 10 km/h,
which no flying aircraft reaches.

`docs/PLANE_FLIGHT.md`'s note that this host "does not model that arm" is therefore harmless for a
descent: modelling it would change nothing about a dive.

## 3. The nose-up floor cannot reach a 60-degree dive

`0099E490`-`0099E512` is the floor, and `0099E4FE` is the instruction that decides:

```
0099e496  FLD  float ptr [ESP + 0x1c]     ; |bank|
0099e49a  FDIV float ptr [ECX + 0x25c]    ; / class+25Ch TurnRoll
0099e4aa  FCOMIP ST0,ST1                  ; 1.0 vs the fraction
0099e4cc  FMUL float ptr [ESP + 0x40]     ; q = fraction * the heading ramp
0099e4dc  FMUL double ptr [0x00ce3de0]    ; * 2.5
0099e4e4  FSTP float ptr [ESP + 0x40]     ; floor = PitchTurnMaxPitch - 2.5 * (1 - q)
0099e4e8  FLD  float ptr [ESI + 0x2bc]    ; the target
0099e4fa  FCOMIP ST0,ST1                  ; floor vs target
0099e4fe  JBE  0099e508                   ; floor <= target -> keep the target
0099e500  MOVSS XMM0,dword ptr [ESP + 0x40]  ; else take the floor
0099e512  MOVSS dword ptr [ESI + 0x2bc],XMM0
```

`plan+2BCh = max(target, floor)`. With `PitchTurnMaxPitch` = `DEG(6)`:

| aircraft state | `q` | floor |
| --- | --- | --- |
| level, on heading | 0 | **-2.395 rad** |
| half bank, half heading error | 0.25 | -1.770 rad |
| hard bank past `DEG(50)` of heading error | 1 | **+0.105 rad** |

**A `-1.047` rad dive clears the level floor by 1.35 radians.** The floor only bites in a hard
banked turn well off heading, which is what it is for - it stops a bot spiralling into the sea while
turning, not a bot diving at its target. `src/plane_ai_control.cpp`'s
`pilot_pitch_demand_0099e490` computes exactly this, with the same `2.5` and the same authored
`PitchTurnMaxPitch`.

## 4. So the host is faithful, and the hypothesis is retired

Put together: the attack-run tick commands 12 m, `009FB800` saturates at `DEG(60)`, mode 2 lets a
dive through, and the floor sits 1.35 rad below it. **The image's own chain produces the same
`-1.047` rad the host measured**, and the host's reconstruction of every stage matches it.

`docs/TORPEDO_GLIDE_THROTTLE_WIRING.md` section 5 called the 800 m plunge "unexplained" and asked
for "the native's own answer for why an attack run from 800 m does not plunge". **The answer is
that it does plunge**, on this chain, and there is no fix to make in the descent law.

What that leaves, stated narrowly:

* **The divergence is not in the pitch path.** Anyone looking for a descent-rate law, a glide angle
  from `class+518h`, or a speed-dependent pitch limit on this path will not find one: sections 1 to
  3 are the whole of it.
* **What differs at the bottom is the water arm**, which `docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`
  bound the entry to and left as a contract. The image's aircraft arrives at the sea in the same
  attitude; what happens next is `007DC205`-`007DC68C`, unread.
* **The remaining question is upstream and it is about altitude, not pitch.** The aim tick's floor
  at `009D1631` is 30 m over land and 5 over water, and its release gate at `009D20B4` wants 25 to
  40 m, so the design expects an aircraft that is already low when it aims. USN01 authors its
  torpedo bombers at 800, 700 and 400 m and `docs/TORPEDO_ENGAGED_TEST.md` showed they are engaged
  from the first tick. Either the image plunges them too, or a state this reconstruction has not
  read brings them down first - and the only descent command anywhere in the torpedo chain is the
  attack-run tick's own.

## The fix location

**There is none in the files this packet can claim.** The descent law is correct at every stage, so
there is nothing to change in `src/plane_ai_control.cpp` or `src/plane_flight.cpp`. The next change
that could matter is the water arm `007DC205`-`007DC68C` behind the flight-state gate, which needs
reading before it can be bound, and that is a packet rather than a fix.

## ABI

* `0099D300` `BSP_PilotBot_PlanControls`, as recorded; the pitch path is `0099DC9E`-`0099E554` and
  `ESI` is the plan throughout.
* `009FB800` `BSP_PilotBot_CommandPitchFromAltitude`, as recorded.
* `007C4810`, the mode-2 increment's inner call, is **unread**; only its position in
  `0099DD35`'s argument list was taken.

## Uncertainty

* `007C4810` and therefore mode 2's exact increment. It does not affect a dive.
* `[EBX+0x88]` at `0099E4C2`, which the host models as the heading ramp on the authority of
  `docs/PILOT_PLANNER_PITCH_ROLL.md`; this packet did not re-derive it.
* Whether `unit+C84h`, the held pitch, has a producer. Not chased, for the same reason as `007C4810`.
* What the image does with an aircraft that reaches the sea in a 60-degree dive.

## Host methods

**None.** Reading only, and the reading confirms the existing reconstructions.

## Corrections

None. Both `pilot_pitch_demand_0099e490` and `pitch_command_009fb800` match the listing.

## no_ghidra_function

None.

## Validation

No run: nothing changed. The numbers this packet reasons against are
`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`'s `-1.0472` rad demand and 141.7 m/s water arrival, and
they are now explained rather than merely measured.

## Follow-up packets

1. **The water arm `007DC205`-`007DC68C`**, which is what the image does at the bottom of the dive
   and the only thing left that differs.
2. **Whether any state outside the torpedo chain descends an ordered aircraft** before its task
   takes over. If none does, the image plunges them too and the 800 m start is simply what the
   mission authors.
