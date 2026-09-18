# The throttle one-shot is raised by the desired-speed setter, and it works in flight

Addresses: 009C1850, 009C1877, 009C187A, 009C1895, 009C189A, 009C18A0, 009C18A7, 009C18B6,
0099D300, 0099D756, 0099D769, 0099D78F, 0099D79A, 0099D7A5, 0099D7AF, 0099D8C1, 0099D8C6,
0099D8CD, 0099D8CF, 0099D8EB, 0099D8FD, 0099D904, 0099D90A, 0099D911, 0099D924, 0099D95C,
0099D96B, 0099D970, 0099D977, 0099D99E, 0099D9A3, 0099DC31, 0099DC6B, 0099DC7A, 0099DC8F,
009C18C0, 009BECD0, 007C47F0, 00D7A23C.

Packet `cc8_pilot_throttle_cut_raiser`, owner `agent/cc8-torpedo-run-in`, on `9b9f05d45` plus main.

Two results. The raiser is found, and **`docs/PILOT_BOT_THROTTLE_DEMANDS.md`'s correction was
itself too strong**: the throttle demand arm does work in flight. Only the `0.6` cap is ground
handling. I reported the opposite to the lead and am retracting it here.

## 1. The census

`tools/store_census.py 0x2d8`, image-wide: **101 writers**, of which 55 are immediate stores and
**26 store `1`**. The rest store `0` or a register. So the field is raised widely, across the pilot
control range `007B4xxx` and the whole bot-task range, and the four torpedo-chain sites are all
**clears**:

| site | routine | value |
| --- | --- | --- |
| `009D0AC5` | `BSP_BotStateTorpedoAttackRun_Tick` | 0 |
| `009D105B`, `009D1182` | `BSP_BotStateTorpedoGoAway_Tick` | 0 |
| `009D1F6B` | `BSP_BotStateTorpedoAim_Tick` | 0 |
| `0099D8EB`, `0099DC6B` | `BSP_PilotBot_PlanControls` | 0, the planner spending the one-shot |

That the torpedo states only ever clear it is itself the evidence the lead asked for: **no torpedo
state raises the one-shot.** The raising is done elsewhere, by the state that is actually moving
the aircraft.

## 2. The raiser, and it is unconditional

**The deciding instructions are `009C189A` and `009C18A7`.**

`009C1850 BSP_BotStateMoveTo_SetDesiredSpeed` is the `vtable+1Ch` slot that `009C18C0`'s step 2
calls with the planar distance to the target:

```
009c1877  MOV   EDX,dword ptr [ESI + 0x4]     ; the approach
009c187a  MOV   EDI,dword ptr [EDX + 0x18]    ; approach+18h, the command block
009c1895  CALL  009becd0                      ; the speed, from 007C47F0's value and the distance
009c189a  FSTP  float ptr [EDI + 0x2b4]       ; cmd+2B4h = the desired speed
009c18a0  MOV   byte ptr [EDI + 0x2b0],0x0
009c18a7  MOV   dword ptr [EDI + 0x2d8],0x1   ; cmd+2D8h = 1
009c18b6  RET   0x4
```

**There is no condition.** Every call raises the field, and the same routine writes `cmd+2B4h`
immediately above it. That closes the loop the previous packet left open: `plan+2B4h` is not an
abstract threshold, it is **the desired speed**, and `0099D8C6`'s `COMISS 0.001f, plan+2B4h` asks
whether the bot has been told to stop.

So the pair `(+2B4h, +2D8h)` is one command: *here is the speed I want, act on it once*.

## 3. The retraction: the demand arm is not ground-only

**The deciding instruction is `0099D8CD`.**

```
0099d8c6  COMISS XMM0,dword ptr [ESI + 0x2b4]  ; 0.001f vs the desired speed
0099d8cd  JBE   0x0099d924                     ; a real speed -> jump INTO the demand block
0099d8cf  MOVSS dword ptr [ESI + 0x278],XMM0   ; a stop -> cut the engine
```

`0099D924` is **past** the flight-state test at `0099D8FD`. So when the one-shot is armed and the
desired speed is at or above `0.001`, control enters the demand block **without the state gate**,
runs the seed at `0099D977`, the speed ratio at `0099D99E`/`0099D9A3`, the accumulate and the
clamp, and writes the throttle and air brake at `0099DC31` and `0099DC4B`.

`docs/PILOT_BOT_THROTTLE_DEMANDS.md` section 1 says both ground arms are behind `unit+900h == 5`.
That is right for the **cap** at `0099DC8F`, which `0099D911 JZ 0099DC7A` reaches only inside the
state branch. It is **wrong for the demand** at `0099DC31`, which has this second entry.

So a flying bot with a desired speed does get a throttle demand, and the correct statement is:

| arm | reachable in flight? |
| --- | --- |
| `0099D399` centred stick | yes, on its own per-slot gate |
| `0099D8CF` engine cut | yes, when the desired speed is below 0.001 |
| `0099DC31` the demand | **yes**, via `0099D8CD`, whenever the one-shot is armed with a real speed |
| `0099DC8F` the 0.6 cap | no, ground only |

**And that reverses what I told the lead.** I said wiring the throttle arm would not move the dive
numbers because nothing raised the field. Both halves were wrong: the move-to speed setter raises it
unconditionally, and the demand arm it leads to runs in flight. Wiring the arm **will** move them.

## 4. The reference the demand divides by

`0099D769 LEA EBP,[ESI + 0x2B8]`, so `EBP` is `plan+2B8h` for the rest of the body, and
`0099D9A3 FDIV float ptr [EBP]` divides `007D99C0`'s forward speed by it. `plan+2B8h` is written
just before, at `0099D78F` and `0099D79A`, and again at `0099D970` as
`max(InterpolateClamped(...), plan+2B8h)` from the `0099D924` entry. So it is a **reference speed**
the planner maintains, and the demand's input is a speed ratio: measured over reference.

That names the last unknown of the demand's two ends. The middle, `0099D9A3`-`0099DBAB`, is still
untranscribed and is the packet's remaining follow-up.

## 5. Not a Lua or script path

The lead's stop condition does not apply. The raiser is native code in the move-to state's own
speed slot, called from the move-to tick, and the other 25 raising sites are all in the pilot
control and bot-task ranges. No property reader before `00BD6830` names this offset, and the
kamikaze gate `007EDA90` is not among the writers.

## ABI

* `009C1850` `BSP_BotStateMoveTo_SetDesiredSpeed`, `void __thiscall(this, float distance)`,
  `RET 4`, Ghidra body `009C1850`-`009C18B8`. `ESI` is the state, `ESI+4h` the approach and
  `approach+18h` the command block.
* `0099D300` `BSP_PilotBot_PlanControls`, as recorded; `EBP` is `plan+2B8h` from `0099D769`.

## Uncertainty

* `009BECD0` and `007C47F0`, the two routines that compute the desired speed. Unread.
* The demand's increment chain, `0099D9A3`-`0099DBAB`.
* `plan+2B0h`, the byte `009C18A0` clears and `0099D924` tests.
* The other 25 sites that store `1` were classified by their enclosing routine's name and value
  only, not read. Several are in `BSP_PlaneBot_ApproachBaseStep` (`009B2C58`),
  `BSP_PlaneBot_TaxiStep` (`009CDC7F`) and `BSP_BotStateDepthChargeAim_Tick` (`009A4556`), which
  fits landing, taxi and attack paths, but that is inference from names.
* Whether the command block at `approach+18h` and the plan the planner holds are one record. Every
  offset in this chain agrees, and this packet did not prove it.

## Host methods

**None.** Reading only. The arm's binding waits with the glide-slope wiring.

## Corrections

Appended to the doc it amends, and verified present there.

* `docs/PILOT_BOT_THROTTLE_DEMANDS.md` sections 1, 2 and 3. Its gate analysis is right for the cap
  and wrong for the demand, which `0099D8CD` reaches past the state test; and its closing claim
  that wiring the arm will not move the dive numbers is withdrawn.

## no_ghidra_function

None. `009C1850`, `0099D300`, `009BECD0` and `007C47F0` all have Ghidra functions.

## Validation

No run: reading only.

## Follow-up packets

1. **The increment chain** `0099D9A3`-`0099DBAB`, so the pure arm loses its input parameter.
2. **`009BECD0` and `007C47F0`**, the desired speed itself.
3. **The 25 other raising sites**, if any of them matters to a torpedo bomber.


## The increment chain, read (packet `cc8_pilot_throttle_increment`)

Appended to this doc rather than given its own, because it completes section 4's
"remaining follow-up".

`0099D99E`-`0099DBC7` is a **proportional speed controller**, and every constant on the path is
below.

```
0099d99e  CALL 007d99c0                        ; the measured forward speed
0099d9a3  FDIV  float ptr [EBP]                ; / plan+2B8h
0099d9b5  FSTP  float ptr [ESP + 0x14]         ; the ratio
0099da52  FLD   float ptr [ESI + 0x2b4]        ; the desired speed 009C189A wrote
0099da58  FSUB  float ptr [ESP + 0x14]         ; error = desired - ratio
0099da97  CALL  EAX                            ; unit vtable+38h
0099daa1  CALL  0042b2f0                       ; over the vec3 at unit+AE0h..AE8h
0099daa6  FSUBR float ptr [ESP + 0x44]         ; vtable38() - that
0099dabe  FMUL  double ptr [0x00ce3d88]        ; * 20.0
0099dac4  FMUL  float ptr [ESP + 0x6c]         ; * pending
0099dac8  FSUBR double ptr [ESP + 0x54]        ; error -= the correction
0099db29  FCOMIP                               ; the dead band, four conditions
0099db56  JBE   0099dbcb                       ;   all four -> SKIP the increment
0099db65  CALL  00415510                       ; plan+2ECh = min(plan+2ECh, 0.16)
0099db9e  CALL  00419010                       ; Interp(-6.9444, -2.0, +6.9444, +2.0, error)
0099dbb1  FMUL  double ptr [0x00ceff98]        ; positive side only, * 0.6
0099dbbf  FMUL  float ptr [ESP + 0x6c]         ; * pending
0099dbc3  FADD  float ptr [ESP + 0x18]         ; accumulate onto the seed
```

**The sign convention**, which is what the arm's behaviour turns on: `0099DA58` is
`desired - measured`, so a bot flying **too fast** produces a **negative** error, the interpolation
returns a negative increment, the demand falls, and `0099DC46`'s `max(0, -d)` turns it into air
brake. Too slow gives a positive increment and throttle. The error enters the map in **metres per
second** and its endpoints are `00D1F3DC` = -6.9444 and `00D0686C` = +6.9444, which are -25 and
+25 km/h, onto `00CE7D7C` = -2.0 and `00CE3958` = +2.0. So the controller saturates at 25 km/h of
error, well inside a cruise mistake.

**The clamp order** is: interpolate first, scale the positive side by `0.6`, scale by `pending`,
accumulate, and only then clamp the accumulated demand into `[-1, 1]` at `0099DBF4` and split it at
`0099DC2C`/`0099DC46`. The `[-1, 1]` clamp is on the demand, never on the increment.

**There is no frame-time factor.** `[ESP+0x6c]`, which multiplies both the correction and the
increment, is written at `0099D7E8`-`0099D81E` as `|slot value - plan+274h|`: the slot's `desired`
when its `active` byte is set, its `current` otherwise, less `current`, then made positive. So it is
**how far the slot has already been asked to move**, and it is exactly zero for a slot whose
`desired` equals its `current` - which is every slot straight out of `0099B450`'s reset. A host that
wires this arm and sees no throttle movement should look there first.

**The dead band**, `0099DB29`-`0099DB56`, skips the whole increment when four conditions hold at
once: `|error| <= 0.83333` (the double at `00D09450`), `ratio >= 1.0`, `desired/|ratio| <= 1.5`
(`00CE380C`) and `0.5` (`00CE3800`) `> desired/|ratio|`. The last two together are a narrow window,
so the band is tight.

**`plan+2B8h` is a scale, not a reference speed.** `docs/PILOT_THROTTLE_CUT_RAISER.md` section 4
calls it "a reference speed". For the subtraction at `0099DA58` to be dimensionally sound with an
error in m/s, `plan+2B8h` has to be near-dimensionless, so "the scale the measured speed is divided
by" is the accurate phrasing. Its own producer, `0099D756`-`0099D79A` and `0099D970`, is unread.

**Still substituted**: `0099DA97`'s unit `vtable+38h` call and the vec3 at `unit+AE0h..AE8h` that
`0042B2F0` reduces. `docs/PILOT_PLANNER_PITCH_ROLL.md`'s note 9 already lists that vec3's producer
as open. The pure function takes the correction term as an input and says so.

`pilot_plan_throttle_0099d300` now takes the measured speed, the scale, the desired speed, the
pending magnitude, the correction and the dead-band flag, and derives the increment itself.


## The two substitutions, resolved as far as the image allows (packet `cc8_pilot_throttle_correction`)

### The vtable slot is a trivial getter, and its field has no displacement writer

`0099DA66 MOV EAX,[EDX+0x38]` with `EDX = [plan+2F0h]` is the **unit's primary vtable**, slot
`+38h`. A byte scan for the known `+3Ch` thunk `0074E1E0` finds **nine** plane vtables
(`00D000AC`, `00D00344`, `00D05F5C`, `00D06674`, `00D0695C`, `00D0BABC`, `00D19D64`, `00D1A03C`,
`00D1A314`), the same nine `docs/PLANE_UNIT_TICK.md` counts, and the dword four bytes before each -
slot `+38h` - is **`007B8E60` in all nine**.

`007B8E60` is two instructions:

```
007b8e60  FLD float ptr [ECX + 0xb1c]
007b8e66  RET
```

So `unit->vtable[38h]` is **`unit+B1Ch`**, a cached float, not a computed speed.
`docs/TORPEDO_APPROACH_UPDATE.md` line 266 already carries this slot as `unit_speed_vtable38`, an
interface; this names its implementation.

**And `unit+B1Ch` has no displacement writer.** An exhaustive `store_census` over `B1Ch` returns
five sites and not one is in the unit or plane range - they are a joystick device, `deflateEnd` and
`_tr_init`. The same is true of the vector: a census over `AE0h` returns **three**, none of them a
unit. `0042B2F0 BSP_Vector3_LengthFloatThreshold`, which reduces it, is a 3D length: `0042B2F6` reads `[ECX+8]`, squares and sums
the three components and compares against the double `1e-10` at `00CE3820` before the root.

The caveat that has to travel with a negative census: MSVC emits `disp32` for offsets this large, so
a direct store would appear, but a store through a base register already offset into the object
would not. What is established is that **no literal `[reg+0B1Ch]` or `[reg+0AE0h]` store exists in
the image**.

If both fields are in fact never written, the correction term `(unit+B1Ch - |unit+AE0h|) * 20 *
pending` is identically **zero** and the arm's error is simply `desired - measured/scale`. That is
falsifiable at run time by printing both fields once the arm is wired, and it is the cheapest way
to retire the substitution.

### The dead band, and a correction to the section above

`0099DB1F`-`0099DB56`, with the jump senses read rather than assumed:

```
0099db29  FCOMIP ST0,ST1            ; |error| vs 0.83333 (the double at 00D09450)
0099db2d  JA 0099db58               ; |error| >  0.83333  -> run the increment
0099db37  COMISS XMM0,XMM1          ; 1.0 (00D7A24C) vs ratio
0099db3a  JA 0099db58               ; 1.0 > ratio         -> run the increment
0099db42  COMISS XMM0,[00ce380c]    ; desired/|ratio| vs 1.5
0099db49  JA 0099db58               ; > 1.5               -> run the increment
0099db53  COMISS XMM1,XMM0          ; 0.5 (00CE3800) vs desired/|ratio|
0099db56  JBE 0099dbcb              ; 0.5 <= it           -> SKIP the increment
```

So the increment is skipped when **all four** hold:

| condition | constant |
| --- | --- |
| `\|error\| <= 0.83333` | `00D09450`, a double |
| `ratio >= 1.0` | `00D7A24C` |
| `desired / \|ratio\| <= 1.5` | `00CE380C` |
| `desired / \|ratio\| >= 0.5` | `00CE3800` |

**Correction to the section above.** It lists the fourth as "`0.5` `> desired/|ratio|`", which is
the wrong way round: `0099DB56` is `JBE`, so the skip needs `0.5` to be **at or below** the ratio.
The band is therefore the sensible one - a small error, the aircraft at or above its scaled speed,
and the desired-to-measured proportion inside `[0.5, 1.5]` - rather than the near-impossible window
the wrong sense implied.

`desired/|ratio|` is formed at `0099DAEC`-`0099DAFF`: `FLD plan+2B4h` then `FDIV [ESP+0x38]`, where
`[ESP+0x38]` is `|ratio|` from `0099DAD0`-`0099DAE6`.
