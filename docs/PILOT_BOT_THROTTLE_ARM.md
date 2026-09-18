# What sets a bot plane's throttle: the planner, in four places

Addresses: 0099D300, 0099D305, 0099D399, 0099D3A1, 0099D8C1, 0099D8C6, 0099D8CF, 0099D8D7,
0099D8DD, 0099DC2C, 0099DC31, 0099DC82, 0099DC8A, 0099DC8F, 0099D700, 0099B450, 0099B45E,
0099B590, 0099B608, 0099BC00, 007BB6E0, 007BB83A, 00415550, 00CE3D30, 00D7A24C.

Packet `cc8_torpedo_moveto_tick` follow-up, owner `agent/cc8-torpedo-run-in`, on `792ec7b1e`.
Reading only.

`docs/TORPEDO_THROTTLE_CUT.md` and `docs/TORPEDO_MOVETO_TICK.md` both closed with the same open
question: **what sets a bot plane's throttle?** Neither the attack-run tick, the aim tick nor the
move-to tick writes it, and `0099B450` only reseeds the slot from the live value. The answer is
that no *task* arm writes it - **the planner does**.

## 1. The slot, and the census

`docs/PILOT_PLAN_SLOT_PIPELINE.md` gives the layout: base `plan+274h`, stride `0Ch`, fields
`{ current, desired, active }`, and slot 0 is the throttle. So the throttle's `desired` is
**`plan+278h`** and its `active` byte is **`plan+27Ch`**.

An exhaustive `store_census` over offset `278h` returns 65 sites image-wide. Six are in the pilot
bot's own range:

| site | routine | what |
| --- | --- | --- |
| `0099B45E` | `BSP_PilotBot_SeedPlanSlots` | the per-think reseed from the live value |
| `0099B608` | `BSP_PilotBot_SeedPlanSlotsHeld` | the second seeder: it calls `0099B450` then rewrites all five `desired` halves from the live axes |
| `0099D399` | `BSP_PilotBot_PlanControls` | the centred-stick arm |
| `0099D8CF` | `BSP_PilotBot_PlanControls` | the speed-hold arm |
| `0099DC31` | `BSP_PilotBot_PlanControls` | the floor |
| `0099DC8F` | `BSP_PilotBot_PlanControls` | the **ceiling, 0.6** |

The rest are other objects that happen to have a `+278h`, the torpedo command block among them:
`009D0AA4`, `009D103D`, `009D115D` and `009D1F4A` are the `cmd+278h = 1.0` flag the attack-run,
go-away and aim ticks write, which is a different record.

**`ESI` is the plan.** `0099D305 MOV ESI,ECX` takes the `this` pointer into `ESI` and never
reloads it; `0099D700 FSTP [ESI+0x2C4]` writes the bank target, which
`docs/PILOT_PLANNER_PITCH_ROLL.md` already attributes to the plan. And the discriminator that
settles it beyond doubt is the pair: `0099D399` writes `[ESI+0x278]` and `0099D3A1` immediately
writes `[ESI+0x27C]`, the slot's own `active` byte, which only the plan record has.

## 2. The four arms

**The centred-stick arm, `0099D34F`-`0099D3B7`.** Entered when the byte at
`[[ESI+0x270] + EAX + 0x9C2]` is non-zero. It writes every slot at once: yaw, pitch, roll and air
brake `desired` to `0.0` with their `active` bytes raised and their modes cleared, and the
**throttle to `[00D7A24C]` = 1.0**. A centred stick and full power.

**The speed-hold arm, `0099D8C1`-`0099D8DD`.**

```
0099d8c1  cmp    ecx, 1
0099d8c4  jne    0099d8f5
0099d8c6  comiss xmm0, [esi + 0x2b4]
0099d8cd  jbe    0099d924
0099d8cf  movss  [esi + 0x278], xmm0     ; throttle desired
0099d8d7  mov    byte [esi + 0x27c], cl  ; active = 1
0099d8dd  movss  [esi + 0x2a8], xmm3     ; air brake desired
```

A mode of 1 and a demand above the threshold `plan+2B4h` set the throttle to the demand and the air
brake alongside it. `plan+2B4h` is one of the fields `0099B450` writes, so the threshold is reset
every think.

**The floor, `0099DC2C`-`0099DC31`.** `00415550 BSP_Math_MaxFloatByRef` over two stack slots, and
the result goes straight into `plan+278h`.

**The ceiling, `0099DC82`-`0099DC8F`.**

```
0099dc82  movss  xmm1, [0x00ce3d30]      ; 0.6
0099dc8a  comiss xmm0, xmm1
0099dc8d  jbe    0099dc9e
0099dc8f  movss  [esi + 0x278], xmm1     ; pin to 0.6
```

**So the native does cut a bot's throttle, to `0.6`**, and it does it in the planner rather than in
any task state. That is the throttle cut `docs/TORPEDO_THROTTLE_CUT.md` went looking for in step 4
of the attack-run tick and correctly did not find there.

## 3. Why this host's planes fly at full throttle

`src/game_hosts_units.cpp`'s `plan_yaw_0099d300()` reconstructs `0099D300`'s **yaw, roll and pitch**
arms and nothing else. The throttle slot is never written, so `pilot_reset_plan_0099b450` reseeds it
from `plane_live_throttle` every think, that field is constructed `1.0f` and nothing moves it, and
`pilot_evaluate_plan_slots_0099bc00` slews a constant into the command block. The aircraft therefore
holds full throttle for its whole life, which is what
`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md` measured as a 60-degree dive arriving at the sea at
141.5 m/s, twice `MaxSpd`.

The path from the slot to the engine is already whole: `0099BC00` slews `desired` into the command
block, `007BB6E0`'s `cmd+14h == 0` branch at `007BB83A` quantises `cmd[3]` into `unit+9F0h`, and
`cc8_plane_pose_throttle_altitude` bound the thrust term that reads the latched copy. **Only the
producer is missing.**

## ABI

* `0099D300` `BSP_PilotBot_PlanControls`, `__thiscall(plan)`, Ghidra body `0099D300`-`0099EBAB`.
  `ESI` is the plan from `0099D305` onward.
* `0099B590` `BSP_PilotBot_SeedPlanSlotsHeld`, `__thiscall(bot)`, `RET 0`, Ghidra body
  `0099B590`-`0099B629`. Its ledger record from `cc2_pilot_controls` already has the rule: it calls
  `0099B450` at `0099B591`, then rewrites only the five `desired` halves from the live axes, raises
  all five `active` bytes and zeroes the four modes. `0099B608` is its throttle store. That record
  also says "no Ghidra function", which is **stale**: there is one.
* `00415550` `BSP_Math_MaxFloatByRef`, two by-reference floats.

## Uncertainty

* **The demands themselves.** `XMM0` at `0099D8C6` and `0099DC8A`, and the two stack slots
  `00415550` compares, were not traced to their producers. This packet establishes *where* the
  throttle is written and *what bounds it*, not what it is computed from.
* The gate of the centred-stick arm, the byte at `[[ESI+0x270] + EAX + 0x9C2]`.
* `ECX`'s mode value at `0099D8C1`, and `plan+2B4h`'s meaning beyond being a reset field.
* When `0099B590` runs instead of `0099B450`; its body is already recorded by
  `cc2_pilot_controls`.
* The 59 census sites outside the pilot-bot range were classified by their enclosing routine's name
  and range, not read.

## Host methods

**None.** Reading only.

## Corrections

Appended to the docs they amend, and verified present there.

* `docs/TORPEDO_THROTTLE_CUT.md`, its second open question and its section 3, which says "no
  routine in the torpedo chain writes it" - true, and now completed: the planner writes it.
* `docs/TORPEDO_MOVETO_TICK.md` section 6, the same question.

## no_ghidra_function

None. `0099D300`, `0099B450`, `0099B590`, `0099BC00`, `007BB6E0` and `00415550` all have Ghidra
functions.

## Validation

No run: reading only, no behaviour changed.

## Follow-up packets

1. **The throttle demands.** Trace `XMM0` at `0099D8C6` and `0099DC8A` and the two `00415550`
   operands to their producers, then bind the arm. That is what would let a bot plane fly at
   anything other than full power, and it is the missing half of every dive this reconstruction has
   measured.
2. **When `0099B590` runs**, the second seeder whose body `cc2_pilot_controls` already read.
3. **The centred-stick arm's gate**, `unit+9C2h` indexed by `EAX`.


## Correction from packet `cc8_pilot_throttle_demands`: the 0.6 is a ground cap

Appended, not rewriting the sections above.

This doc's census, its four sites, and its proof that `ESI` is the plan - the `desired` write at
`0099D399` followed immediately by the slot's own `active` byte at `0099D3A1` - all stand. **Its
headline does not.**

`0099D8FD CMP dword ptr [EDX + 0x900],0x5` and `0099D904 JNZ 0099DC9E` put **both** the demand at
`0099DC31` and the ceiling at `0099DC8F` inside `unit+900h == 5`, a ground-roll state. A plane in
free flight jumps straight past them. So the `0.6` is a **taxi cap**, not an in-flight throttle
cut, and section 2's "the native does cut a bot's throttle, to `0.6`" is wrong about when.

Section 3's attribution is withdrawn with it. The host's planes hold full throttle because nothing
writes the slot, but so do the image's flying bots: the only throttle write a plane in state 7 can
reach is the one-shot at `0099D8CF`, gated on `plan+2D8h == 1`, which sets `0.001f` - below the
`0.01f` thrust gate at `007DB76C`, so the engine goes off - and clears the field at `0099D8EB`.

**The question therefore moves to what raises `plan+2D8h` to 1.** Every torpedo-chain site read so
far writes zero into it (`009D0AC5` in the attack-run tick, `009D1F6B` in the aim tick). Until the
raiser is found, wiring the arm will not move the dive numbers.
`docs/PILOT_BOT_THROTTLE_DEMANDS.md`.
