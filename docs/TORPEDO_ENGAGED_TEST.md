# The engaged test is five clauses and no hysteresis, and USN01 starts inside it

Addresses: 009D3210, 009D3213, 009D321C, 009D322B, 009D323A, 009D3243, 009D3249, 009D324F,
009D3255, 009D3259, 009D325B, 009D325F, 009D4A70, 009D4030, 009D41C4, 009D4053, 007B8AD0,
00D05AC8.

Packet `cc8_torpedo_engaged_test`, owner `agent/cc8-torpedo-run-in`, on main `78af19721` merged.

**The answer to the mission question is no**, so this packet stops where the lead's condition says
to stop: the attack-run branch keeps its zero range pair and nobody invents a slope for it. No host
code changed, and `src/game_hosts_units.cpp` was never claimed - the dive-bomb worker held it
throughout and did not need to release it.

## 1. The whole predicate, in 22 instructions

`009D3210 BSP_BotTaskTorpedo_IsEngaged`, `bool __fastcall(task)`, Ghidra body
`009D3210`-`009D3265`. The body is short enough to give entire:

```
009d3213  CMP  byte ptr [ESI + 0x529],0x0
009d321a  JNZ  009d325f                      ; the in-range latch -> engaged
009d321c  CMP  dword ptr [ESI + 0x4c4],0x0
009d3223  JZ   009d325b                      ; no engage target -> not engaged
009d3225  MOV  EAX,dword ptr [ESI + 0x404]
009d322b  CMP  dword ptr [EAX + 0x370],0x2
009d3232  JZ   009d325f                      ; pilot control mode 2 -> engaged
009d3234  MOV  ECX,dword ptr [ESI + 0x3fc]
009d323a  CALL 007b8ad0                      ; unit+9D8h == 0
009d3241  JZ   009d325b                      ; has a follow target -> not engaged
009d3243  FLD  float ptr [ESI + 0x488]       ; the live range
009d3249  FLD  float ptr [ESI + 0x484]       ; the engage distance
009d324f  FMUL double ptr [0x00d05ac8]       ; * 2.2
009d3255  FCOMIP ST0,ST1
009d3259  JA   009d325f                      ; engage * 2.2 > range -> engaged
009d325b  XOR  EAX,EAX                       ; else not engaged
```

`00D05AC8` is `00 00 00 a0 99 99 01 40`, the double **2.2**.

**There is no hysteresis, no timer, no heading gate, no altitude gate and no aim-command clause.**
The only latch is `task+529h`, the in-range latch, and it pushes the answer toward *engaged*, never
away from it. Nothing in this predicate can hold an aircraft in move-to except the three negative
clauses: no engage target, a follow target, or a range beyond `2.2 * engageDistance`.

`src/torpedo_task_arm.cpp`'s `torpedo_engaged_009d3210` already had all five clauses and the right
senses. This packet confirms it whole rather than changing it.

## 2. The two range fields, and why no census was needed

`task+484h` is the engage distance and `task+488h` the live range; the host records them as the
same storage as `approach+8Ch` and `approach+90h`.

`task+484h`'s producer is already established and printed at run time. The USN01 log's own line
reads:

```
009D4A70 sets the engage distance task+484h=2200.0 (Pilot/Torpedo/AttackDist)
```

and `src/bot_tasks.cpp`'s `kTorpedoCruiseProfile` names the same row, `kTorpedoAttackDist` = 2200.
So an image-wide census over `8Ch` - which returns 230 rows, far too common an offset to classify -
would add nothing: the value is authored, its writer is named, and the run prints it.

## 3. The mission question, with numbers

| quantity | value |
| --- | --- |
| engage distance, `task+484h` | **2200 m**, `Pilot/Torpedo/AttackDist` |
| the scale at `009D324F` | **2.2** |
| the engaged threshold | **4840 m** |
| USN01 ordered aircraft, `range_first_mean` | **1490.8 m** |
| USN01 `worst_closed` | 0.0 m, so no aircraft ever exceeded its start range |

**1490 m is well inside 4840 m, so the fifth clause is true on the first arm tick and stays true
for the whole run.** `009D4030` returns the approach state only when this predicate is false, at
`009D41C4` and `009D4053`, so the ordered bombers go to `attackrun` immediately and never occupy
`moveto`.

**The native does the same.** Every input is the image's own: the authored 2200, the image's 2.2,
and a range the mission's own placement fixes at 1490 m. There is no host substitution anywhere in
the chain, so there is nothing to correct.

`docs/TORPEDO_GLIDE_THROTTLE_WIRING.md`'s next gate is therefore closed with a negative: the
engaged inputs are right, the glide slope and the throttle arm are correctly wired and correctly
inert on this mission, and the reason is the mission's placement rather than a defect.

## 4. What this leaves

The glide slope is not wasted work - it is simply unreachable on USN01. Two things follow, and both
are for someone else to weigh:

* **A mission whose torpedo bombers launch beyond 4840 m would fly it.** Nothing else needs to
  change for that to happen. Whether any shipped mission does is not established here.
* **The 800 m plunge stays unexplained.** The attack-run tick commands 12 m from 800 m and
  `009FB800` saturates at its `DEG(60)` cap, and `docs/TORPEDO_THROTTLE_CUT.md` proved the
  attack-run site passes a zero range pair, so the slope's shape is not available there. The
  native's own answer for why an attack run from 800 m does not plunge is still unread, and
  inventing one is what this packet declines to do.

## ABI

`009D3210` `BSP_BotTaskTorpedo_IsEngaged`, `bool __fastcall(task)` with the result in `AL`, Ghidra
body `009D3210`-`009D3265`, 22 instructions, complete. `task+3FCh` is the unit, `task+404h` the
pilot control block.

## Uncertainty

* Whether any shipped mission places torpedo bombers beyond `2.2 * AttackDist` of their targets.
* `task+4C4h`, the engage target, and `task+529h`'s own producer, which are the two clauses this
  packet did not chase because neither can hold an aircraft in move-to.

## Host methods

**None.** Reading only, and the reading confirms the existing binding rather than changing it.

## Corrections

None. `src/torpedo_task_arm.cpp`'s reconstruction of this predicate is correct as written.

## no_ghidra_function

None. `009D3210` and `007B8AD0` both have Ghidra functions.

## Validation

No run: nothing changed, and `docs/TORPEDO_GLIDE_THROTTLE_WIRING.md`'s USN01 run already supplies
the two numbers the question turns on, `range_first_mean = 1490.8` and the log's own
`task+484h=2200.0`.

## Follow-up packets

1. **Why an attack run from 800 m does not plunge in the image.** This is the live question, and
   the answer is not in the move-to path.
2. **A mission survey**: whether any placement puts torpedo bombers beyond 4840 m of their targets,
   which would exercise the glide slope as wired.
