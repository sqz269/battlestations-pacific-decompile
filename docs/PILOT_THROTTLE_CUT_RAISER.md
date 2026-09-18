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
