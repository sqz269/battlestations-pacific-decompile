# The four throttle arms, their gates, and the one a flying plane can reach

Addresses: 0099D300, 0099D305, 0099D309, 0099D316, 0099D329, 0099D33D, 0099D345, 0099D399,
0099D3A1, 0099D3A8, 0099D3B7, 0099D79F, 0099D7A5, 0099D7A7, 0099D7AF, 0099D7C5, 0099D7CB,
0099D7D6, 0099D7E0, 0099D8C1, 0099D8C6, 0099D8CF, 0099D8D7, 0099D8DD, 0099D8EB, 0099D8FD,
0099D904, 0099D90A, 0099D911, 0099D977, 0099D998, 0099D99E, 0099D9A3, 0099DBAB, 0099DBB1,
0099DBBF, 0099DBC3, 0099DBCB, 0099DBF4, 0099DC01, 0099DC2C, 0099DC31, 0099DC46, 0099DC4B,
0099DC6B, 0099DC7A, 0099DC82, 0099DC8A, 0099DC8F, 0099DC9E, 00415550, 00415690, 007D99C0,
007DB76C, 00D7A23C, 00D7A24C, 00D7A260, 00CE3D30, 00CEFF98, 00F876B8.

Packet `cc8_pilot_throttle_demands`, owner `agent/cc8-torpedo-run-in`, on `6f3a7f799`.

Follow-up 1 of `docs/PILOT_BOT_THROTTLE_ARM.md`, and it **corrects that doc's headline**. The four
writes are right and the census is right; the reading that "the native does cut a bot's throttle,
to 0.6" is wrong about *when*. Two of the four arms run only while the aircraft is on the ground.

## 1. The gate that changes the picture

**The deciding instructions are `0099D8FD` and `0099D904`.**

```
0099d8fd  CMP dword ptr [EDX + 0x900],0x5     ; EDX = plan+2F0h, the unit
0099d904  JNZ 0x0099dc9e                      ; not state 5 -> past BOTH ground arms
```

`unit+900h` is the flight state: 7 is free flight, 6 the water surface, and the ground-roll arm of
`007CE040` takes 4 or 5. So everything from `0099D90A` to `0099DC97` - which is where both
`0099DC31` and `0099DC8F` live - is **ground handling**. A flying plane jumps straight over it to
`0099DC9E`.

`docs/PILOT_BOT_THROTTLE_ARM.md` read the 0.6 at `0099DC8F` as an in-flight throttle cut and made
it "the finding of the day". It is a **taxi cap**. The correction is appended there.

## 2. The four arms, each with its own gate

### `0099D399`, the centred-stick arm. Not state gated.

Four conditions in series, all before any state test:

```
0099d309  CMP  byte ptr [ESI + 0x26c],0x2     ; plan+26Ch == 2
0099d310  JNZ  0099d3c5
0099d316  MOVZX EAX,word ptr [0x00f876b8]     ; the session slot index
0099d323  ADD EAX,EAX   (x3)                  ; EAX = index * 8
0099d329  CMP  byte ptr [ECX + EAX + 0x9c2],0 ; ECX = plan+2F0h, the unit
0099d331  JZ   0099d3c5
0099d337  MOV  ECX,dword ptr [ESI + 0x270]
0099d33d  CMP  ECX,EDI                        ; EDI is 0 from 0099D307
0099d33f  JZ   0099d3c5
0099d345  CMP  byte ptr [ECX + EAX + 0x9c2],0
0099d34d  JZ   0099d3c5
```

`unit+9C2h` indexed by `8 * [00F876B8]` is the same per-slot byte array the ordnance release reads
at `unit+9C3h[[00F876B8]*8]`, so this is a per-session-slot flag on two different units. When both
carry it and `plan+26Ch` is 2, the arm writes **every** axis: yaw, pitch, roll and air brake
`desired` to `0.0` with their `active` bytes raised and their modes cleared, and the throttle to
`[00D7A24C]` = **1.0**. A centred stick and full power.

### `0099D8CF`, the one-shot engine cut. Not state gated, and the only one a flying plane reaches.

```
0099d79f  MOV   ECX,dword ptr [ESI + 0x2d8]   ; plan+2D8h
0099d7a5  TEST  ECX,ECX
0099d7a7  MOVSS XMM0,dword ptr [0x00d7a23c]   ; 0.001f
0099d7af  JNZ   0099d8c1                      ; non-zero -> the arm, XMM0 = 0.001f
...
0099d8c1  CMP   ECX,0x1
0099d8c4  JNZ   0099d8f5
0099d8c6  COMISS XMM0,dword ptr [ESI + 0x2b4] ; 0.001f vs plan+2B4h
0099d8cd  JBE   0099d924
0099d8cf  MOVSS dword ptr [ESI + 0x278],XMM0  ; throttle = 0.001f
0099d8d7  MOV   byte ptr [ESI + 0x27c],CL     ; active = 1
0099d8dd  MOVSS dword ptr [ESI + 0x2a8],XMM3  ; air brake
0099d8e5  MOV   byte ptr [ESI + 0x2ac],CL
0099d8eb  MOV   dword ptr [ESI + 0x2d8],0x0   ; spend the one-shot
```

**`XMM0` is the constant, not a demand.** `0099D8C1` is reached only by the jumps at `0099D7AF` and
`0099D7C5`, and the instruction before `0099D8C1` is `0099D8BF JMP 0099D8F7`, which goes past it -
so the compared value and the stored value are the same `0.001f` that `0099D7A7` loaded. That is
**below the `0.01f` thrust gate at `007DB76C`**, so the arm switches the engine off.

`0099D8EB` clears `plan+2D8h`, so a task raises that field to 1 and the planner spends it once:
engine off, air brake out. This is the in-flight "slow down now" command, and it is the only
throttle write a plane in state 7 can reach besides the centred-stick arm.

### `0099DC31`, the ground demand. Inside state 5, slot already active.

One signed demand, seeded from the slot's own value and accumulated:

```
0099d977  CMP   byte ptr [ESI + 0x27c],0x0    ; the slot's active byte
0099d980  MOVSS XMM0,dword ptr [ESI + 0x278]  ; active -> desired
0099d98a  MOVSS XMM0,dword ptr [ESI + 0x274]  ; else current
0099d998  MOVSS dword ptr [ESP + 0x18],XMM0   ; the seed
0099d99e  CALL  007d99c0                      ; the forward speed
0099d9a3  FDIV  float ptr [EBP]               ; over a reference: a speed ratio
...
0099dbb1  FMUL  double ptr [0x00ceff98]       ; 0.6, on the positive branch only
0099dbbf  FMUL  float ptr [ESP + 0x6c]
0099dbc3  FADD  float ptr [ESP + 0x18]        ; accumulate onto the seed
0099dbf4  CALL  00415690                      ; clamp into [-1.0, 1.0]
0099dc2c  CALL  00415550                      ; throttle = max(0.001f, d)
0099dc31  FSTP  float ptr [ESI + 0x278]
0099dc46  CALL  00415550                      ; air brake = max(0.0f, -d)
0099dc4b  FSTP  float ptr [ESI + 0x2a8]
```

So **one axis drives both controls**: the positive part of the demand is the throttle, floored at
`0.001f`, and the negative part is the air brake. `0099DC01` forms the negation as `-0.0f - d`
through `00D7A208`, the same idiom the lift and drag terms use.

**PARTIAL**: the increment chain `0099D9A3`-`0099DBAB` is roughly 130 instructions and was not
transcribed. What is established is its two ends - a speed ratio from `007D99C0` at one, the
accumulate onto the seed at the other - and the `0.6` scale the positive branch takes at
`0099DBB1`.

### `0099DC8F`, the ground cap. Inside state 5, slot not active.

```
0099dc7a  MOVSS XMM0,dword ptr [ESI + 0x274]  ; the slot's CURRENT
0099dc82  MOVSS XMM1,dword ptr [0x00ce3d30]   ; 0.6
0099dc8a  COMISS XMM0,XMM1
0099dc8d  JBE   0099dc9e
0099dc8f  MOVSS dword ptr [ESI + 0x278],XMM1
```

A plane taxiing above 0.6 throttle is commanded back to 0.6.

## 3. What this means for the reconstruction's planes

`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md` measured aircraft diving at 60 degrees and arriving at
the sea at 141.5 m/s, twice `MaxSpd`, and `docs/PILOT_BOT_THROTTLE_ARM.md` attributed that to a
missing 0.6 cut. **That attribution is wrong.** A plane in free flight never reaches the ground
arms, so the native's flying bots also hold whatever throttle they have, and the only thing that
changes it is a task raising `plan+2D8h`.

So the question moves: **what raises `plan+2D8h` to 1?** An exhaustive census over that offset
returns writers across the bot-task range, but every torpedo-chain site read so far writes **zero**
- `009D0AC5` in the attack-run tick and `009D1F6B` in the aim tick both store `0`. The raiser was
not found in this packet, and until it is, a flying bot's throttle is whatever it was seeded with,
in the image as much as in this host.

That is a materially better answer than "the host is missing a cut", and it means the dive numbers
will **not** move when the arm is wired unless the raiser is found too.

## 4. The pure function

`pilot_plan_throttle_0099d300` in `src/plane_ai_control.cpp`, beside
`pilot_plan_roll_0099e2ba`. It takes the slot, the flight state, the two mode fields and the
already-evaluated centred-stick conditions, and returns which of the throttle and air brake were
written and to what. The three complete arms are exact; the ground demand takes its accumulated
increment as an **input**, because the chain that derives it is partial.

`XMM3`, the one-shot's air-brake value, is also **not traced**: `0099DC59` sets `XMM3` to `1.0`
from `00D7A24C` on the ground path, which is the only load of it this packet read, so the function
takes `1.0` and says so.

## ABI

`0099D300` `BSP_PilotBot_PlanControls`, `__thiscall(plan)`, Ghidra body `0099D300`-`0099EBAB`.
`ESI` is the plan from `0099D305`; `EDI` is zero from `0099D307` and is the source of every
"cleared" store in the centred-stick arm.

## Uncertainty

* The ground demand's increment chain, `0099D9A3`-`0099DBAB`.
* `XMM3` at `0099D8DD`.
* `[EBP]` at `0099D9A3`, the speed reference.
* `plan+2B4h`, the one-shot's threshold, and what raises `plan+2D8h` to 1.
* `plan+26Ch == 2` and the meaning of the `unit+9C2h` per-slot byte.
* `plan+270h` and `plan+2F0h` as two distinct unit pointers.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `pilot_plan_throttle_0099d300` | `src/plane_ai_control.cpp` | `0099D300`'s four arms | reconstruction, ground increment **partial** |

The binding into the units host waits for `src/game_hosts_units.cpp` to free, with the glide-slope
wiring, as the lead's packet says.

## Corrections

Appended to the doc it amends, and verified present there.

* `docs/PILOT_BOT_THROTTLE_ARM.md`. Its census, its four sites and its proof that `ESI` is the plan
  all stand. Its headline does not: the `0.6` at `0099DC8F` and the demand at `0099DC31` are both
  inside `unit+900h == 5`, so they are ground handling, and its section 3's attribution of the
  host's full-throttle dives to a missing cut is withdrawn.

## no_ghidra_function

None. `0099D300`, `00415550`, `00415690` and `007D99C0` all have Ghidra functions.

## Validation

No run: this packet adds a pure function with no caller. The census this arm will move is the one
the lead's packet reserves for the combined wiring commit, against
`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md`'s numbers as the before column.

## Follow-up packets

1. **What raises `plan+2D8h` to 1.** This is now the gate on the whole throttle question: without
   it no flying bot ever changes power, in the image or here.
2. **The ground increment chain**, `0099D9A3`-`0099DBAB`.
3. **`plan+26Ch` and the `unit+9C2h` per-slot byte**, which decide whether the centred-stick arm
   ever fires.
