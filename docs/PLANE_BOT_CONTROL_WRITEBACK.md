# How a bot task's plan reaches the latched controls (packet `cc7_plane_bot_control_writeback`)

Addresses: `0099ACD0`, `009998A0`, `0099B450`, `0099D300`, `0099BEE0`, `0099BC00`, `0099BF30`,
`007B8C90`, `007CE040`, `007BB920`, `007BB6E0`, `007B9770`, `007CAF10`, `007CFD20`, `007DB680`,
`007DA710`, `0099A170`, `0099A4C0`, `0071BE40`.
Constants: `00D1F39C`, `00CE3854`, `00CE3D34`, `00CFD408`, `00D05998`, `00D7A250`, `00D7A24C`,
`00D7A260`. Vtable slot `00D1F354`.

Worker `cc7-plane-bot-control-writeback`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Ghidra was **read-only**: no rename, no comment, no
prototype, no tag, no write lock, no save, no lease, no git. Every descriptive name is a
hypothesis, not a recovered symbol.

## Headline

**The chain is not missing. It is complete in the image, and it was already published in
`docs/PLANE_AI_CONTROL.md` — but with the two middle links in the wrong order**, and that
particular error is the kind that would make a faithful reimplementation produce exactly the
symptom this packet was opened to explain (planned controls that never take effect).

`0099B450 BSP_PilotBot_SeedPlanSlots` runs **before** `0099D300 BSP_PilotBot_PlanControls`, not
after, and it writes **both** halves of every plan slot rather than only the `current` half. In
the documented order the seed would overwrite every `desired` the planner had just produced, and
the slew would then converge each axis onto the value it already had — a plane that holds its
stick wherever it is. See [Corrections](#corrections-to-existing-docs).

The second correction is smaller but affects every reader of the brief: the latched controls are
fields of the **unit**, not of the flight controller. `007DA72D MOV EDI,dword ptr [ESI + 0x8]`
makes `EDI` the unit, and every `+BB0h`/`+BB4h`/`+BB8h` in `007DA710` is `EDI`-based. Writing them
as `ctl+BB0h` is wrong by one indirection.

## The chain, one link per line

Each line is `address  name  ABI  what it does`. Every call site address below was read from the
Ghidra listing or from `ghidra xrefs`; none is inferred.

| # | address | name (hypothesis) | ABI | role |
| --- | --- | --- | --- | --- |
| 1 | `0071BE40` | `BSP_WeaponDirector_CurrentCommand` | `__thiscall(director)` | the unit's current command, the thing `0077D600` ultimately deposits |
| 2 | `0099A170` | `BSP_Bot_InstallCommandTask` | `__thiscall(bot)` | builds a task object for the command kind |
| 3 | `0099A4C0` | task-vector sync | `__thiscall(bot)` | re-syncs `bot+58h/+5Ch` against (1), calling (2) |
| 4 | `0099ACD0` | `BSP_PilotBot_Tick` | `void __thiscall(bot, float dt)`, `RET 4` | the tick; picks the front task and drives 5-9 |
| 5 | `009998A0` | `BSP_PilotBot_Update` | `void __thiscall(task, float dt)`, `RET 4` | seeds the plan, runs the task's own arm, then plans |
| 6 | `0099B450` | `BSP_PilotBot_SeedPlanSlots` | `void __thiscall(plan)`, `RET 0` | resets all five slots from the unit's live control block |
| 7 | `0099D300` | `BSP_PilotBot_PlanControls` | `__thiscall(plan, float dt)`, `RET 4` | writes the five `desired` fields |
| 8 | `0099BEE0` | `BSP_PilotBot_EvaluatePlanSlots` | `__thiscall(plan, void* cmdBuf, float rate, float dt)`, `RET 0Ch` | slews `current`->`desired` into `cmdBuf`, + 3 request bytes |
| 9 | `0099BF30` | `BSP_PilotBot_RepairCommandBands` | `__thiscall(plan, void* cmdBuf)`, `RET 4` | band repair on the buffer (body not read) |
| 10 | `007B8C90` | `BSP_Plane_SetPilotCommandBlock` | `__thiscall(unit, const void* cmd)`, `RET 4` | copies six dwords to `unit+9FCh`..`+A10h`, sets `unit+A14h = 1` |
| 11 | `007CE040` | `BSP_PlaneTickElement_FixedStep` | `__thiscall(element, ...)` | the plane's own fixed step; calls 12 then 14 |
| 12 | `007BB920` | `BSP_Plane_CommitPilotCommand` | `void __thiscall(unit)`, `RET 0` | gates on `unit+A14h`, calls 13, clears `unit+A14h` |
| 13 | `007BB6E0` | `BSP_Plane_QuantizeControlAxes` | `__thiscall(unit, const float* cmd)`, `RET 4` | 8-bit quantize `cmd[0..4]` into `unit+9E4h`..`+9F4h` |
| 14 | `007B9770` | `BSP_Plane_LatchControlInput` | `void __thiscall(unit)`, `RET 0` | copies `unit+9E4h`..`+9F4h` to `unit+BB0h`..`+BC0h` |
| 15 | `007DB680` | `BSP_PlaneFlight_CoreLaw` | `__thiscall(ctl, float dt)` | calls 16 at `007DB6A6` |
| 16 | `007DA710` | `BSP_PlaneFlight_ControlRateLaw` | `__thiscall(ctl, float dt)`, `RET 4` | reads `[ctl+8h]+BB0h/+BB4h/+BB8h` |

`RET` sizes for 7, 9, 13 and 16 were read from the caller rather than the epilogue: none of
`00999907`, `0099B0AC`, `007BB98B`, `007DB6A6` is followed by an `ADD ESP,n`, so each callee
cleans its own one stack argument. 4, 5, 6, 8, 12 and 14 have their `RET` read directly
(`0099AD65 RET 4`, `009999B2 RET 4`, `0099BF23 RET 0Ch`, `007BB998 RET`, `007B97D2 RET`); 6's
`RET 0` is from `009998D2 CALL 0099B450` followed by `MOV ECX,ESI` with no cleanup and no stack
argument set up.

## Link 4: `0099ACD0 BSP_PilotBot_Tick` — what it dispatches to, and when

`void __thiscall(bot, float dt)`, `RET 4`, body `0099ACD0`-`0099B1A5`, 367 instructions,
cyclomatic complexity 49. `bot+50h` is the plane unit (`0099ACD6 MOV ECX,dword ptr [ESI + 0x50]`).
Its only xref is `DATA` from `00D1F354`, a vtable slot — the tick is a virtual, and no direct
caller exists.

### Entry gates (return before any work)

```
0099acd9  TEST ECX,ECX                    JZ 0099b19f   ; bot+50h (the unit) null
0099ace1  CMP byte ptr [ECX + 0x5d],0x0   JNZ 0099b19f
0099aceb  CMP byte ptr [ECX + 0x60],0x0   JNZ 0099b19f
0099acf5  CMP byte ptr [ECX + 0x61],0x0   JNZ 0099b19f
0099ad05  CMP byte ptr [EAX + 0x61],0x0   JNZ 0099b19f  ; EAX = unit+9D4h, the owner/squadron
```

### The think-rate gate

```
0099ad0f  bot+70h += dt
0099ad21  FLD [00D1F39C]                  ; 3D B8 51 EC = 0.09f
0099ad27  FCOMIP ST0,ST1 / JBE 0099ad68   ; taken when 0.09f <= bot+70h
```

Below `0.09 s` of accumulation the tick takes the short path at `0099AD2B` (a peer/neighbour
update through `0099A9E0`, indices from the words at `00F876B8` and `00E0B6CC`) and returns at
`0099AD65`. So the planner runs at about **11 Hz**, not once per frame. At and above the
threshold, `0099AD75` zeroes `bot+70h` and `0099AD7E` accumulates `bot+80h`.

### Task acquisition

`bot+58h` is a task-vector data pointer, `bot+5Ch` its size, `bot+6Ch` its capacity
(`0099A542`-`0099A565` grows it as `2n+2` through `operator new` at `00BF55BE`).

* `0099ADAA CALL 0099A170 BSP_Bot_InstallCommandTask` when `bot+7Ch` is set (`0099AD94`), then
  `bot+7Ch = 0` and `bot+74h = -1.0f` (`00D7A260`).
* `0099AE7E CALL 0099A4C0` re-syncs the vector against `0071BE40`, and itself calls `0099A170`.
  `0099A170`'s callees are the fourteen task factories — `009AB570 MakeDogfight`,
  `009CD300 MakeStrafe`, `009C8C70 MakeDiveBomb`, `009D4E30 MakeTorpedo`, `009B9030 MakeLevelBomb`,
  `009AF720 MakeKamikaze`, `009AEBE0 MakeDropKamikaze`, `009A6970 MakeDepthCharge`,
  `007B7FD0 MakeRocket`, `009A2F40 MakeCloseToShip`, `009B41C0 MakeLand`, `009CA2B0 MakeRetreat`,
  `009BADB0 MakeStop` — plus `00521EA0 BSP_CommandTarget_ResolveObject` and
  `006BCD20 BSP_AirOps_GetBlock`.
* `0099AE83`-`0099AE90` then take the **front** task:
  `EBX = (bot+5Ch > 0) ? *(void**)bot+58h : NULL`.

### The four production gates

Each of these skips links 5-10 entirely:

| address | condition | target |
| --- | --- | --- |
| `0099AEC4` | `EBX == 0` — **the task vector is empty** | `0099B11C` |
| `0099AF10` | `bot+74h -= dt` is still `> 0` (`FLDZ`/`FCOMIP`/`JC`) | `0099B11A` |
| `0099AF3B` | `unit[0x9C2 + idx*8] != 0`, `idx = word@00F876B8`; also sets `bot+74h = [00CE3854] = 3.0f` at `0099AF45` | `0099B11C` |
| `0099AF26` | `bot+50h` became null during link 5 | `0099AFCC` |

The first is the one that matters for the reconstruction: with no task the bot writes nothing at
all, `unit+9FCh`.. keeps its previous contents, `unit+A14h` is never set, `007BB920` returns at its
own `unit+A14h` gate, and `007B9770` keeps latching a control block nobody has touched. The rate
law then sees three constants. `bot+74h = -1.0f` on every task install/change (`0099ADBB`,
`0099AEBD`, `0099AEED`) exists precisely so a fresh task clears the second gate immediately.

### The production block

```
0099af17  MOV ECX,EBX                     ; this = the front task
0099af1c  CALL 009998a0                   ; link 5
0099b075  LEA ECX,[ESP + 0x18]            ; &cmdBuf, a stack local
0099b07c  LEA EDI,[EBX + 0x4]             ; the plan object = task+4h
0099b080..0099b096  six dwords of cmdBuf zeroed
0099b09a  MOVSS [ESP + 0x28],XMM0         ; the power slot preset to 1.0f (00D7A24C)
0099b0a0  CALL 0099bee0                   ; link 8, rate = 00CE3D34 = 4.0f, dt from [ESP+2Ch]
0099b0ac  CALL 0099bf30                   ; link 9
0099b0b1  MOV ECX,[ESI + 0x50]            ; the unit
0099b0b9  CALL 007b8c90                   ; link 10
```

`EBX+4` is the plan object, and the `+4` shift is independently confirmed: `009998A0` reads
`task+2F4h` as an object with a `+C24h` byte (`0099995C`/`00999962`), `0099B450` reads `plan+2F0h`
as the unit, and `007BB7B1` reads `unit+C24h`. `task+2F4h == plan+2F0h == the unit`.

## Link 5: `009998A0` — the real order, and the task's own arm

`void __thiscall(task, float dt)`, `RET 4`, body `009998A0`-`009999C2`, 87 instructions.

```
009998a4  CMP byte ptr [ESI + 0x270],0x2     JNZ 009998cc
009998ad  MOV EAX,[ESI + 0x274]              JZ  009998cc      ; the task's target entity
009998be  CMP byte [EAX + ECX*8 + 0x9c2],0   JNZ 009999b0      ; suppressed -> do nothing
009998cd  LEA EDI,[ESI + 0x4]                                  ; the plan object
009998d0  MOV ECX,EDI
009998d2  CALL 0099b450                      ; SEED FIRST
009998d9  CALL 0099c270                      ; predicate on the task
009998de  TEST AL,AL / JZ 00999912
   true : 009998f1 MOV dword [ESI+2E4h],0      ; then task->vtable[64h](dt) at 009998fb
          00999907 CALL 0099d300 (ECX = EDI)   ; PLAN SECOND
   false: 00999912 timer arm on task+308h / task+304h
          0099993c CALL 0099b740 BSP_BotTask_AbandonIfStale
          00999950 MOV dword [ESI+2E4h],0FFh   ; then task->vtable[64h](dt) at 0099995a
          00999979 CALL 009fc7c0 (ECX = task+314h) when unit+C24h set
          0099998c CALL 009fd0e0 (ECX = task+38Ch)
          0099999b CALL 009a17d0 (ECX = task)
          009999aa CALL 0099d300 (ECX = EDI)   ; PLAN SECOND, on this branch too
```

Both branches reach `0099D300`, and on both the seed precedes it. **`task->vtable[64h]` is where
the task kind enters** — dogfight, strafe, dive bomb and the rest each override it, and it runs
before the shared five-axis planner.

## Link 6: `0099B450` writes both halves

`void __thiscall(plan)`, `RET 0`. `EAX = plan+2F0h` is the unit. Read straight from the listing:

| live field | -> `desired` | -> `prev`/`current` | -> `active` | axis |
| --- | --- | --- | --- | --- |
| `unit+9F0h` (`0099B456`) | `plan+278h` (`0099B45E`) | `plan+274h` (`0099B466`) | `plan+27Ch = 0` | power |
| `unit+9E4h` (`0099B476`) | `plan+284h` (`0099B47E`) | `plan+280h` (`0099B486`) | `plan+288h = 0` | yaw |
| `unit+9ECh` (`0099B494`) | `plan+290h` (`0099B49C`) | `plan+28Ch` (`0099B4A4`) | `plan+294h = 0` | roll |
| `unit+9E8h` (`0099B4B2`) | `plan+29Ch` (`0099B4BA`) | `plan+298h` (`0099B4C2`) | `plan+2A0h = 0` | pitch |
| `unit+9F4h` (`0099B4D0`) | `plan+2A8h` (`0099B4D8`) | `plan+2A4h` (`0099B4E0`) | `plan+2ACh = 0` | air brake |

It also clears `plan+26Ch` and `plan+270h`, zeroes `plan+2C4h`/`plan+2BCh`, and stores
`[plan+2F4h]+190h` into `plan+2B4h` (`0099B4F1`-`0099B517`). `plan+2F4h` is the vehicle class
descriptor per `docs/PILOT_BOT_PLAN_CONTROLS.md`.

So this is a **full reset of the plan to the aircraft's current stick position**, not a partial
reseed. Calling it after the planner would erase the plan.

## Links 7-8: the plan slots

Five 12-byte `{prev, desired, active}` records, base **`plan+274h`**, stride `0Ch`. Verified
stores: `0099EA3E MOVSS [ESI+284h],XMM0` (yaw `desired`) and `0099D6B7 MOVSS [ESI+290h],XMM0`
(roll `desired`), `ESI = plan` in both.

`0099BEE0 BSP_PilotBot_EvaluatePlanSlots(plan, cmdBuf, rate, dt)`, `RET 0Ch`:

```
0099bef7  LEA ECX,[ESI + 0x274]           ; the slot array
0099bf01  CALL 0099bc00                   ; (slots, cmdBuf, rate, dt) -> 0099BB40 slew x5
0099bf06  MOV AL, [ESI + 0x2dc]  -> [EDI + 0x16]
0099bf0f  MOV CL, [ESI + 0x2e5]  -> [EDI + 0x15]
0099bf18  MOV DL, [ESI + 0x2e4]  -> [EDI + 0x14]
```

`rate` is `[00CE3D34] = 40 80 00 00 = 4.0f` at the `0099ACD0` call site, so the bot's stick moves at
most 4 units/second and needs `0.5 s` to travel the full `[-1, +1]`.

**Which slot a plane-attack task uses: none in particular.** The slots are per-axis, not
per-task-kind, and `0099D300` re-plans all five on every tick that gets past the gates. The task
kind enters one level up, through `task->vtable[64h]` at `009998FB`/`0099995A`. The premise that a
plane-attack task selects a slot does not hold.

The three request bytes come from `plan+2DCh`, `plan+2E5h` and `plan+2E4h`. The `0`/`0FFh` that
`009998F1`/`00999950` write are at `task+2E4h`, which is `plan+2E0h` — a **different field**. I did
not establish any relation between them; the offsets are four apart and that is all I can say.

## Links 10-14: the command block and the latch

`007B8C90 BSP_Plane_SetPilotCommandBlock(unit, cmd)`, `RET 4`, body `007B8C90`-`007B8CD2`: six
dwords to `unit+9FCh`, `+A00h`, `+A04h`, `+A08h`, `+A0Ch`, `+A10h` (`007B8C96`-`007B8CC3`), then
`unit+A14h = 1` at `007B8CC9`. Its other callers are `00519818` in
`BSP_PlanePilotView_BuildPlayerCommand` (the human stick, same block) and `0060C22B`.

`007BB920 BSP_Plane_CommitPilotCommand(unit)`, `RET 0`, called at `007CE865`:

```
007bb923  CMP byte [ESI + 0x61],0     JNZ 007bb997   ; gate
007bb929  CMP byte [ESI + 0xa14],0    JZ  007bb997   ; gate: nothing pending
007bb932  EAX = [ESI + 0x900]; if EAX not in {7,6,4,5}: [ESI+0xa0c] = 1.0f   (00D7A24C)
007bb95c  if unit->vtable[5Ch](0x17) && [ESI+0xc24] == 0: [ESI+0xa08] = 1.0f
007bb982  LEA EAX,[ESI + 0x9fc]
007bb98b  CALL 007bb6e0
007bb990  MOV byte [ESI + 0xa14],0
```

`007BB6E0 BSP_Plane_QuantizeControlAxes(unit, cmd)`. Every axis takes the same shape:

```
n = ftol(cmd[i] * 127.0 + 128.5)              ; 00CFD408 = 127.0, 00D05998 = 128.5, via 00BF7420
n >= 0FFh   -> +1.0f
n <= 1      -> -1.0f   (00D7A250 = -1.0 double)
otherwise   -> (n - 128) / 127.0
```

| source | store | destination | axis |
| --- | --- | --- | --- |
| `cmd[0]` `007BB6E6` | `007BB72A FSTP` | `unit+9E4h` | yaw |
| `cmd[1]` `007BB730` | `007BB75D FSTP` | `unit+9E8h` | pitch |
| `cmd[2]` `007BB763` | `007BB798 FSTP` | `unit+9ECh` | roll |
| `cmd[3]` `007BB83A` | `007BB877 FSTP` (also `007BB824 MOVSS`) | `unit+9F0h` | throttle/power |
| `cmd[4]` `007BB87D` | after `007BB8AC` | `unit+9F4h` | air brake |

This is an 8-bit quantization — the bot's planned float survives the round trip only to
`1/127` precision. It matches the slot-to-command permutation in
`docs/PILOT_BOT_PLAN_CONTROLS.md` exactly: slot 1 (yaw) stores to `cmd[0]`, slot 3 (pitch) to
`cmd[1]`, slot 2 (roll) to `cmd[2]`.

`007B9770 BSP_Plane_LatchControlInput(unit)`, `RET 0`, called at `007CE96F` and at `007CB1F3`,
in full:

```
007b9770  FLD  [ECX + 0x9e4]   007b9783  FSTP [ECX + 0xbb0]   ; yaw
007b9789  FLD  [ECX + 0x9e8]   007b979c  FSTP [ECX + 0xbb4]   ; pitch
007b97a2  FLD  [ECX + 0x9ec]   007b97a8  FSTP [ECX + 0xbb8]   ; roll
007b97ae  FLD  [ECX + 0x9f0]   007b97ba  FSTP [ECX + 0xbbc]   ; throttle
007b97c6  FLD  [ECX + 0x9f4]   007b97cc  FSTP [ECX + 0xbc0]   ; air brake
007b9776  [+9f8h] -> 007b978f [+0bc8h]
007b977d  [+9fah] -> 007b97b4 [+0bc9h]
007b9795  [+9f9h] -> 007b97c0 [+0bcah]
```

`ECX` is the unit: the source offsets `+9E4h`.. are the plane unit's live control block
(`007CFD20` zeroes them at `007CFE92`-`007CFEB8`).

## The writers of `+BB0h`, `+BB4h`, `+BB8h` — a complete census of literal-displacement stores

`0BB0h` forces a `disp32` in every MSVC addressing form, so the four-byte little-endian tail
`b0 0b 00 00` appears verbatim in every such instruction.

**Positive control first**, because a byte-scan negative is worthless without one: the FSTP
pattern `d9 ?? b0 0b 00 00` matched `007B9783`, the store I had already read from the listing.
The scan is therefore live, not vacuous. `007B9783` encodes as `d9 99 b0 0b 00 00`.

| site | function | form | role |
| --- | --- | --- | --- |
| `007B9783` / `007B979C` / `007B97A8` | `007B9770 BSP_Plane_LatchControlInput` | `FSTP [ECX+n]` | the per-step latch |
| `007CB3C1` / `007CB3D4` / `007CB3E4` | `007CAF10 BSP_Plane_LimitAndResetControls` | `MOVSS [ESI+n]` | the override/reset path (`docs/PLANE_FLIGHT.md` line 148) |
| `007CFF99` / `007CFF9F` / `007CFFA5` | `007CFD20 BSP_PlaneUnitInstance_Construct` | `MOV [ESI+n],r32` | zero-init |

**Nothing else writes them on this class.** The other `+BB0h`/`+BB4h`/`+BB8h` stores in the image
— `00818970` (`008189B8`, `008189BE`, `00818ABE`, `00818AC4`, `00818B2D`, `00818B33`),
`0081ED40 BSP_UnitVehicleBase_Construct` (`0081F004`, `0081F00A`, `0081F010`),
`0081F3A0 BSP_UnitVehicleBase_Destruct` (`0081F675`, `0081F64E`, `0081F627`) and
`00822C20 BSP_UnitInstance_SEntityInit` (`00822FC4`) — belong to a **different class**. They treat
the fields as pointers (the vehicle base releases them as refs), and `007CFD20` does not call
`0081ED40`: its base constructor is `0095CC90 BSP_UnitGameObject_Construct`. The two layouts
coincide by offset only.

Read-only sites, for completeness: `007C9C23 MOVSS XMM0,[EDI+0BB0h]` in `FUN_007C95A0`,
`007D6B98` in `BSP_Plane_ReadPropertyBag` (the `yawF` property key pair), `007DA936` in the rate
law, `007DB6E0` in the core law.

`MOV [reg+0BB0h],imm32` (`c7 ?? b0 0b 00 00`) returned no matches — **and I did not establish a
positive control for that encoding, so that negative carries no information.** It is listed only
so nobody re-runs it and believes it.

## Where the latched controls actually live

`007DA710 BSP_PlaneFlight_ControlRateLaw(ctl, dt)`:

```
007da715  MOV ESI,ECX                     ; ESI = the flight controller (this)
007da72d  MOV EDI,dword ptr [ESI + 0x8]   ; EDI = the UNIT
007da732  FLD  [EDI + 0x838]
007da738  FADD [EDI + 0xbb8]              ; latched roll
007da8f1  MOVSS XMM0,[EDI + 0xbb4]        ; latched pitch
007da934  ...   [EDI + 0xbb0]             ; latched yaw
```

So the fields are `unit+BB0h/+BB4h/+BB8h`, reached through `ctl+8h`. This agrees with
`docs/PLANE_FLIGHT_CORE_LAW.md` line 51 (`ctl+8h` is the unit) and with
`include/bsp/plane_flight.hpp`, whose `plane_control_off` namespace comment says in as many words
"The pilot control block, unit+9E4h". Only the namespace *name* suggests a controller base.

## Corrections to existing docs

**1. `docs/PLANE_AI_CONTROL.md`, the chain block at lines 160-172 — the order of the first two
lines is inverted, and this one is behavioral.** It reads:

```
0099D300 BSP_PilotBot_PlanControls(task+4h, dt)   ; writes the five slots' `desired` halves
0099B450 BSP_PilotBot_SeedPlanSlots(task+4h)      ; reseeds every `current` half from the live block
```

The listing of `009998A0` is unambiguous: `009998D2 CALL 0099B450` precedes
`00999907 CALL 0099D300` on the true branch and `009999AA CALL 0099D300` on the false branch.
Implemented in the published order, the seed would run last and overwrite every `desired` the
planner produced, so the slew at `0099BB40` would find `delta == 0` on all five axes and the
aircraft would hold whatever stick it already had — the exact symptom this packet was opened for.

**2. Same line: "reseeds every `current` half" is wrong.** `0099B450` writes `desired` **and**
`prev` from the same live value and clears the `active` byte, on all five slots. It is a reset,
not a reseed. Table above.

**3. `docs/PLANE_FLIGHT.md` line 162 is stale and contradicts its own correction at lines
489-491.** Line 162 still reads "`classDesc+1B0h YawSpd` times the latched **roll** `unit+0BB0h`";
lines 489-491 of the same file correct exactly that, establishing `unit+BB0h` as `yawF`, the
latched yaw, on the evidence of `007D6BB2` pairing `unit+BB0h` with `00D05D30 'yawF'` and
`007B9770` copying `unit+9E4h` there. The table row was never updated.

**4. The packet brief's `kLatchedYaw ctl+BB0h` is wrong by one indirection** — `unit+BB0h` via
`ctl+8h`, per `007DA72D`. The header it cites does not make this error.

**5. The brief's premise "nothing currently connects the two" does not hold for the binary.**
Sixteen links, all present, all with call-site addresses above. If the reconstruction shows no
turning, the cause is at one of the four gates in `0099ACD0` or in the ordering error at (1), not
in a missing writer.

**6. Slot-base labelling.** `docs/PLANE_AI_CONTROL.md` and `docs/PILOT_BOT_PLAN_CONTROLS.md` both
give the slot base as `task+274h`. It is `plan+274h`, and `plan = task+4h`, so in task-relative
terms it is `task+278h`. Every individual offset those docs list (`+274h`, `+280h`, `+284h`, ...)
is correct **as plan-relative**; only the `task+` prefix is wrong. Confirmed by
`0099BEF7 LEA ECX,[ESI+274h]` with `ESI` the object `0099ACD0` computed as `LEA EDI,[EBX+4]`.

## What is *not* established

* **No runtime measurement was taken.** Which of the four gates in `0099ACD0` fires in the
  reconstructed host, and whether `bot+5Ch` is ever non-zero there, is unknown from this packet.
  Everything above is static.
* **Block copies and SIB-indexed writes to `+BB0h` are invisible to the census.** A `REP MOVSD`
  over the unit, or a `MOV [reg+reg*s+0BB0h]`, produces no `disp32` an offset scan can find and no
  xref. The census is complete for literal-displacement stores and for nothing else.
* **I did not verify that `007CE865` (commit) and `007CE96F` (latch) lie on a common path** inside
  `007CE040`. Both are unconditional `CALL` instructions and the commit precedes the latch in
  address order, but I did not trace the basic-block graph of a 1200-instruction function, so
  "commit then latch in the same step" is an inference from address order, not a proof.
* **The owning class of the vtable at `00D1F354` is unidentified.** Scans for code references to
  `00D1F310`, `00D1F318`, `00D1F320`, `00D1F328`, `00D1F330`, `00D1F338` and `00D1F340` as a
  vtable base all returned nothing, so I could not find the constructor that installs it. The slot
  is at `00D1F354`; the neighbouring entries are `00D1F340 = 006935C0`, `+4 = 0`,
  `+8 = 0099A850`, `+0Ch = 009995A0`, `+10h = 0071C480`, `+18h = 0099A830`, `+1Ch = 0072BC70`.
  (`0099ACD0` itself has one data xref only, from `00D1F354`.)
* **`0099BF30 BSP_PilotBot_RepairCommandBands` was not read.** It receives `(plan, cmdBuf)` and
  runs between the slew and the commit; whatever it does to the buffer is unmodelled here.
* **`0099D300`'s per-task-kind behaviour is untraced.** It is 1451 instructions with cyclomatic
  complexity 135; `docs/PILOT_BOT_PLAN_CONTROLS.md` covers its yaw and pitch arms. How the
  `task->vtable[64h]` arm of a specific kind (dogfight, strafe, dive bomb) changes what `0099D300`
  produces is not established — only that the vtable call happens first, at `009998FB` and
  `0099995A`.
* **The relation, if any, between `task+2E4h` (written `0`/`0FFh` at `009998F1`/`00999950`) and the
  three request bytes read from `plan+2DCh`/`plan+2E4h`/`plan+2E5h` is not established.** They are
  different offsets once the `plan = task+4h` shift is applied, and a plausible-looking match is
  not evidence.
* **Whether `0077D600 BSP_Entity_IssueCommand` is what sets `bot+7Ch`** (the flag that triggers
  `0099A170` at `0099ADAA`) was not traced. The link from the command to the task vector was taken
  through `0099A4C0` -> `0071BE40` -> `0099A170`, which is a different route.
* `unit+61h`, `unit+5Dh`, `unit+60h`, `unit+9D4h+61h`, `unit+900h`, `unit+C24h` and
  `unit+9C2h+idx*8` are named here only by the role they play in a gate. Their meanings were not
  independently recovered in this packet.
