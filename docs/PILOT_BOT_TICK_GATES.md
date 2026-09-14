# The pilot bot's tick gates, its dispatch order, and the planner's entry

Packet `cc7-bottick`. Everything below is static evidence read from the Ghidra listing of
`battlestationspacific.exe` (project `C:/Users/sqz269/bsp.gpr`). No runtime measurement was taken,
and Ghidra was not mutated by this packet. Descriptive names are hypotheses.

This extends `docs/PLANE_BOT_CONTROL_WRITEBACK.md` (the sixteen-link chain) and
`docs/PILOT_BOT_PLAN_CONTROLS.md` (the axis arms). Where those two are corrected, the correction is
marked **CORRECTION** and carries the address that settles it.

---

## Headline

1. **The commit is reached through twelve gates, and after the twelfth it is unconditional.**
   From `0099AF4F` to `0099B0B9` there is no further branch that can skip `007B8C90`. Every join in
   that stretch lands on `0099B04D`, checked exhaustively below.
2. **The `dt` that reaches every downstream callee is not the frame delta.** `0099AD16` overwrites
   the incoming argument slot with the *accumulated* think interval, and that is the value later
   read at `0099AEF5`, `0099AF19` and `0099B05A`. So `009998A0`, `task->vtable[64h]`, `0099D300` and
   `0099BEE0` all receive `elapsed >= 0.09 s`, not the per-frame `dt`. **CORRECTION** to
   `docs/PLANE_BOT_CONTROL_WRITEBACK.md`, which calls it "dt from `[ESP+2Ch]`" without qualification.
3. **The short branch of `009998A0` can never produce a commit in the same tick.** Its predicate
   (`0099C270`) requires `unit[9C2h + idx*8] != 0`; the tick's last gate (`0099AF3B`) requires that
   same byte to be `0`. They are opposite conditions on the same byte, evaluated ~0x60 bytes apart.
4. **`0099D300` never reads the task's target entity.** `plan+270h` (= `task+274h`, the target) is
   referenced at exactly one address in the whole 1451-instruction body — `0099D337`, inside the
   entry gate. The desired heading arrives pre-computed in `plan+2C0h`, written by the per-kind arm
   `task->vtable[64h]` that runs immediately before it.

---

## ABIs

| address | name (hypothesis) | signature | return | evidence |
| --- | --- | --- | --- | --- |
| `0099ACD0` | `BSP_PilotBot_Tick` | `void __thiscall(bot, float dt)` | `RET 4` | `0099AD62`/`0099AD65`; arg at `[ESP+24h]` after `SUB ESP,1Ch` + `PUSH ESI` |
| `009998A0` | `BSP_PilotBot_Update` | `void __thiscall(task, float dt)` | `RET 4` | `009999B2`; arg at `[ESP+10h]` after three pushes |
| `0099D300` | `BSP_PilotBot_PlanControls` | `void __thiscall(plan, float dt)` | `RET 4` | `0099D3C2`, `0099D3BF`; `ECX = task+4` at the call sites `00999907`/`009999AA` |
| `0099C270` | *(branch predicate)* | `bool __thiscall(task)` | `RET 0` | `0099C290`/`0099C293` |
| `0074E230` | *(unit-state admits AI)* | `bool __thiscall(unit)` | `RET 0` | `0074E24C`/`0074E252` |
| `006DEEC0` | *(slot-state admits AI)* | `bool __thiscall(bot, int slot)` | `RET 4` | `006DEEDF`/`006DEEE7` |
| `0042A7E0` | *(unit-state is airborne-ish)* | `bool __thiscall(unit)` | `RET 0` | `0042A7F2`/`0042A7F8` |
| `007BB150` | *(publish a per-slot byte)* | `void __thiscall(unit, char)` | `RET 4` | `007BB16C` |

`bot+50h` is the plane unit (`0099ACD6`). `task+2F4h == plan+2F0h == the unit` — carried on trust
from `docs/PLANE_BOT_CONTROL_WRITEBACK.md`, and independently consistent here: `0099D3CB` reads
`plan+2F0h -> +72Ch` with the same `vtable[38h]` idiom that `0099AF5C` applies to `bot+50h -> +72Ch`.

---

## (1) The gate checklist for `0099ACD0`

`idx` below is always `(uint16)[00F876B8]`, loaded fresh at each use (`0099AD2B`, `0099AED4`,
`0099AF2C`, `0099B03B`). The `unit+9C0h` region is an array of 8-byte records indexed by it;
`+9C1h`, `+9C2h` and `+9C3h` within a record are all read by nearby code
(`0099B042`, `0099AF33`, `007BB15F`).

A host that wants the tick to reach the control commit at `007B8C90` must satisfy all twelve, in
this order:

| # | address | field | comparison | required value |
| --- | --- | --- | --- | --- |
| 1 | `0099ACD9` | `bot+50h` (the unit) | `TEST ECX,ECX` / `JZ` | non-null |
| 2 | `0099ACE1` | `unit+5Dh` (byte) | `CMP …,0` / `JNZ` | `0` |
| 3 | `0099ACEB` | `unit+60h` (byte) | `CMP …,0` / `JNZ` | `0` |
| 4 | `0099ACF5` | `unit+61h` (byte) | `CMP …,0` / `JNZ` | `0` |
| 5 | `0099AD05` | `(unit+9D4h)+61h` (byte) | `CMP …,0` / `JNZ` | `0` — and `unit+9D4h` must be non-null, there is no check before the dereference |
| 6 | `0099AD21`-`0099AD29` | `bot+70h + dt` | `FCOMIP` vs `[00D1F39C]`, `JBE` to the think path | `>= 0.09f` |
| 7 | `0099AE3F`-`0099AE46` | `006DEEC0(bot, 1)` | `TEST AL,AL` / `JNZ` continues | non-zero |
| 8 | `0099AE5F`-`0099AE66` | `0074E230(unit)` | `TEST AL,AL` / `JZ` bails | non-zero, i.e. `unit+900h ∈ {4,5,6,7}` |
| 9 | `0099AEC2`-`0099AEC4` | `EBX` = front task | `TEST EBX,EBX` / `JZ` | non-null, i.e. `bot+5Ch > 0` and `*(void**)(bot+58h) != 0` |
| 10 | `0099AF0A`-`0099AF10` | `bot+74h - dt` | `FLDZ` / `FCOMIP` / `JC` | `<= 0` (the countdown has expired) |
| — | `0099AF1C` | | | **`CALL 009998A0` — the whole plan is computed here** |
| 11 | `0099AF24`-`0099AF26` | `bot+50h` re-read | `TEST EAX,EAX` / `JZ 0099AFCC` | *not a production gate* — see below |
| 12 | `0099AF33`-`0099AF3B` | `unit[9C2h + idx*8]` (byte) | `CMP …,0` / `JZ` continues | `0`; otherwise `bot+74h = [00CE3854] = 3.0f` (`0099AF3D`) and the tick jumps to `0099B11C`, skipping the commit |

Gate 6's constant: `00D1F39C = EC 51 B8 3D = 0.09f`, so the planner runs at about **11 Hz**.
Gate 10's rearm constant: `00D7A260 = 00 00 80 BF = -1.0f`. Gate 12's suspend: `00CE3854 = 3.0f`.

### Gate 6 in detail — and the `dt` substitution

```
0099ad0f  FLD  [ESI+0x70]      ; acc
0099ad12  FADD [ESP+0x24]      ; acc + dt          <- [ESP+24h] is the incoming float argument
0099ad16  FSTP [ESP+0x24]      ; *** the argument slot now holds acc + dt ***
0099ad1a  FLD  [ESP+0x24]
0099ad1e  FST  [ESI+0x70]      ; bot+70h = acc + dt
0099ad21  FLD  [0x00d1f39c]    ; 0.09f
0099ad27  FCOMIP ST0,ST1       ; 0.09f vs (acc+dt)
0099ad29  JBE  0x0099ad68      ; taken when 0.09f <= acc+dt -> think
```
On the think path `0099AD75` zeroes `bot+70h` and `0099AD7E` accumulates `bot+80h += acc+dt`
(a lifetime total). Below threshold the tick takes `0099AD2B`-`0099AD65`: a call to `0099A9E0`
with two pointers into a 28-byte-stride array at `bot+84h`
(`LEA ECX,[EAX*8+0] / SUB ECX,EAX` = `7*idx`, then `LEA …[ESI + ECX*4 + 0x84]`), indices from
`[00F876B8]` and `[00E0B6CC]`; then `RET 4`.

The slot `[ESP+24h]` is never written again. Its later reads are `0099AEF5 FLD [ESP+0x2c]`,
`0099AF19 FSTP [ESP]` (the argument to `009998A0`) and `0099B05A FLD [ESP+0x2c]` (the argument to
`0099BEE0`). The `+8` shift is the two callee-saved pushes `PUSH EDI` (`0099AE3A`) and `PUSH EBX`
(`0099AE7B`); `PUSH ECX` at `0099AF16` is cleaned by `009998A0`'s `RET 4`, and `PUSH EBP`
(`0099AFD9`) is popped at `0099B019` before `0099B05A`. So all three reads see `acc + dt`.

### Gate 11 is not a gate — **CORRECTION**

`docs/PLANE_BOT_CONTROL_WRITEBACK.md` lists `0099AF26` among "four production gates" that "skip
links 5-10 entirely". Neither half holds:

* `0099AF26 JZ 0x0099AFCC` lands at `0099AFCC`, which falls through `0099B003`, `0099B019`,
  `0099B04D` into the production block. It skips only the `unit+C58h` bookkeeping at
  `0099AF2C`-`0099AFC2`, which is the block that dereferences the now-stale `EAX`.
* Gates 11 and 12 are *after* `CALL 009998A0` at `0099AF1C`, so they cannot skip link 5. Only
  gates 9 and 10 precede it.

### Everything from `0099AF4F` to the commit is unconditional

Exhaustive join check on the branches in `0099AF4F`-`0099B049`:
`0099AF51`, `0099AF5A`, `0099AF8E`, `0099AFB1`, `0099AFBD` → `0099AFCC`;
`0099AF6B`, `0099AF74`, `0099AF7F` → `0099AFBF` → `0099AFCC`;
`0099AFDF`, `0099AFFF` → `0099B003`; `0099AFED` → `0099B001` → `0099B003`;
`0099B01A`, `0099B035`, `0099B039` → `0099B04D`; `0099B027` → `0099B031` → `0099B033` → `0099B04D`.
No edge in that range targets `0099B11A`, `0099B11C` or `0099B11D`. Therefore once gate 12 passes,
`007BB150`, `0099BEE0`, `0099BF30` and `007B8C90` all run.

The bookkeeping those branches guard is a countdown on `unit+C58h`: it is decremented by one
(`0099AFB6`) when `unit+C58h > 0`, `(unit+72Ch)->vtable[38h]()` is true, `unit+5Ch != 0`,
`007B9140(unit, 0)` is true, and some task in the vector answers `vtable[24h]()` truthfully;
otherwise `unit+C58h = 0` (`0099AFC2`). `0099B00C` publishes `unit+DF8h = EBP`, where `EBP` is `1`
unless some task's `vtable[50h]()` returned a value other than `1`. `007BB150` then writes
`unit+DF0h` from either `(EBP == 2)` or, when `EBP == 1` and `(unit+9D4h)+3D0h` names a different
unit, that unit's `[9C1h + idx*8]` byte (the slot is written at `0099B015` and `0099B049` and read
at `0099B04D`; both writes are byte-wide to the same post-`POP EBP` address).

### Gates 7 and 8 resolved

```
006deec0: MOV ECX,[ECX+0x50]                  ; ECX = bot -> unit
006deec3: MOV EAX,[ESP+0x4]                   ; slot index, always 1 from 0099AE3B
006deec7: MOV EAX,[ECX + EAX*4 + 0x1ac]       ; unit[1ACh + slot*4]  -> unit+1B0h for slot 1
006deece: CMP EAX,0x8 / JZ -> return 1
006deed3: PUSH EAX / CALL 00927f10 / TEST AL,AL / JNZ -> return 1 ; else return 0
00927f10: MOV EAX,[ESP+4] / MOV ECX,[0x00e188a8]
00927f1a: MOV EDX,[ECX + EAX*4 + 0x18cc] / MOV AL,[EDX+0x9] / RET 4
```
So gate 7 requires `unit[1B0h] == 8`, or the table entry `((*[00E188A8])[18CCh + unit[1B0h]*4])+9h`
to be non-zero. `unit+1ACh` is an array of small integers (a crew/station occupancy enum is the
obvious reading, and is **provisional** — the value `8` is not otherwise identified).

```
0074e230: MOV EAX,[ECX+0x900] ; CMP 7 / CMP 6 / CMP 4 / CMP 5 -> return 1, else 0
0042a7e0: MOV EAX,[ECX+0x900] ; CMP 4 / CMP 5 -> return 1, else 0
```
`unit+900h` is a small state enum. Gate 8 admits `{4,5,6,7}`. The post-commit check at `0099B0E8`
uses the narrower `{4,5}` test, OR'd with `unit+900h == 6` at `0099B0F1` — so `{4,5,6}` there.
What the values mean is **not established**; the pair of tests is consistent with `4`/`5` being two
airborne sub-states and `6`/`7` being take-off/landing, but that is a guess, not evidence.

### `bot+58h` and `bot+5Ch` — settled

They are the data pointer and element count of a vector of task pointers.

* `bot+5Ch` is a signed `int` count: `CMP dword ptr [ESI+0x5c],0` with `JBE`/`JZ` at `0099ADC7`,
  `0099AE6C`, `0099AE83`, `0099B11D`.
* `bot+58h` is `BotTask**`: `0099AE72`-`0099AE75` (`MOV ECX,[ESI+58h]` / `MOV EDI,[ECX]`) takes the
  front element as an object with a vtable (`0099ADE7 MOV EDX,[ECX]`).
* element size is 4: the end pointer is `LEA EDX,[ECX + EAX*4]` with `EAX = bot+5Ch`
  (`0099AF89`, `0099AFD4`, `0099AFAA`, `0099AFF8`), and the scan steps `ADD EDI,4`
  (`0099AFA7`, `0099AFF5`).
* `bot+6Ch` is the capacity, grown as `2n+2` at `0099A542`-`0099A565` — **taken on trust** from
  `docs/PLANE_BOT_CONTROL_WRITEBACK.md`; not re-read here.

The front task is taken twice: `EDI` before `0099A4C0` re-syncs the vector (`0099AE6C`-`0099AE79`)
and `EBX` after (`0099AE83`-`0099AE90`). `0099AE92 CMP EDI,EBX` detects a *changed* front task and
rearms `bot+74h = -1.0f` at `0099AEBD` after notifying the old one through `vtable[54h]`.

### `bot+7Ch` — **not settled**, and here is exactly how far I got

`bot+7Ch` is a byte, tested at `0099AD94` and cleared at `0099ADB7`. When set, the block
`0099AD9A`-`0099ADBB` runs `00999E40`, `00999EE0`, then `0099A170 BSP_Bot_InstallCommandTask`,
clears the flag and rearms `bot+74h = -1.0f`. (**CORRECTION**, minor: the writeback doc names only
`0099A170` in this block; there are three calls, and `00999EE0` runs again unconditionally at
`0099ADC2` regardless of the flag.)

Byte scans over the whole image, each with a positive control:

* `c6 ?? 7c ??` (`MOV byte ptr [reg+7Ch], imm8`, mod=01 forms `40 41 42 43 45 46 47`).
  **Positive control:** `c6 46 7c 00` matches `0099ADB7`, the known clear in `0099ACD0`, plus
  `0053A8BF` and `006DFC61`. In the `009xxxxx` band the only stores of a non-zero value are
  `009BDD30` and nothing else.
* `88 ?? 7c` (`MOV byte ptr [reg+7Ch], r8`) over all 56 mod=01 modrm bytes. In the `009xxxxx` band:
  six hits in `00973B20`-`00973E90`, one at `0099A91E`, and `009C1BA8`.

Two candidates survive, and neither is proven to be *this* bot:

1. **`009C1BA8`, in `BSP_BotStateMoveTo_Tick` (`009C18C0`)**:
   ```
   009c1b79  MOV ECX,0x1
   009c1b9b  MOV EDX,dword ptr [ESI]
   009c1b9d  CMP dword ptr [EDX + 0x10],0x0
   009c1ba1  JZ 0x009c1bab
   009c1ba5  MOV EDX,dword ptr [EAX + 0x10]      ; EAX = EDX from 009c1ba3
   009c1ba8  MOV byte ptr [EDX + 0x7c],CL        ; CL = 1
   ```
   `CL` is `1`: tracing backwards from `009C1BA8` to the nearest preceding write of `ECX`/`CL`
   gives `009C1B79 MOV ECX,1`, and nothing between `009C1B7E` and `009C1BA5` writes `ECX`.
   The same block first writes `EAX = (*ESI)+18h` with `[EAX+2BCh] = 0.0f`, `[EAX+2D0h] = 2`,
   `[EAX+2C4h] = 0.0f`, `[EAX+2CCh] = 1` — those are plan/task target-and-mode offsets, so
   `(*ESI)+18h` is very likely the plan and `(*ESI)+10h` plausibly the bot. **Plausible, unproven.**
2. **`009BDD30`**, a complete one-instruction orphan: `MOV byte ptr [ECX+7Ch],1 / RET`, bounded by
   `INT3` padding at `009BDD2D`-`009BDD2F` and `009BDD35`-`009BDD37`. Ghidra has no function here
   (the nearest preceding is `FUN_009BDC10`, whose body ends at `009BDC74`, so the scan's
   attribution is wrong — the usual nearest-preceding-function artefact). Its immediate neighbour
   `009BDD20 MOV [ECX+270h],arg / RET 4` sets a `+270h` field, and `+270h` is a **task** field
   (`009998A4 CMP byte ptr [ESI+0x270],0x2` with `ESI = task`). If the two stubs belong to the same
   class, `009BDD30` sets `task+7Ch`, not `bot+7Ch`. Orphan COMDAT stubs are grouped by the linker,
   not by class, so adjacency is weak. **Unresolved.**

What this does *not* rule out: a write through a SIB-indexed address, a write at a different
literal offset because the bot is a sub-object of a larger allocation, or a `REP MOVSD` over the
whole bot. Any of those produces no matching `disp8` and no xref. `0077D600
BSP_Entity_IssueCommand` was not examined here, so the writeback doc's open question about it
stays open.

---

## (2) `009998A0` — the call order, and the branch

**The published order was inverted, and the writeback doc's correction is confirmed.** The seed
runs first:

```
009998cc  PUSH EDI
009998cd  LEA EDI,[ESI + 0x4]        ; EDI = the plan = task+4
009998d0  MOV ECX,EDI
009998d2  CALL 0x0099b450            ; (1) SEED: reset the plan to the live stick position
009998d7  MOV ECX,ESI
009998d9  CALL 0x0099c270            ; the branch predicate, on the task
009998de  TEST AL,AL / JZ 0x00999912
```

`dt` is `[ESP+10h]` throughout (`PUSH ECX` + `PUSH ESI` + `PUSH EDI` = 12 below the argument). Every
callee is `RET 4`, so the slot is at the same offset for all four reads at `009998E4`, `009998FD`,
`00999941`/`0099996B`/`0099997E`/`00999991`/`009999A0`. Traced forwards from each `PUSH ECX`
reservation to its `FSTP [ESP]`; no callee leaves the stack unbalanced.

### True branch (predicate non-zero), `009998E2`-`0099990F`

```
009998e2  MOV EDX,[ESI]              ; the task's vtable
009998e8  MOV EAX,[EDX + 0x64]       ; slot 64h = index 25
009998f1  MOV dword ptr [ESI+0x2e4],0x0
009998fb  CALL EAX                   ; (2) task->vtable[64h](dt)   -- the per-kind arm
00999907  CALL 0x0099d300            ; (3) the shared five-axis planner, ECX = EDI = plan
```

### False branch (predicate zero), `00999912`-`009999B2`

A countdown on `task+308h` with period `task+304h` runs first:

```
00999912  FLD [ESI+0x308] ; …        ; ST0 = dt, ST1 = t308 after 00999920
00999924  FCOMI ST0,ST1
00999926  JC 0x009999b5              ; CF=1 when dt < t308  -> just decrement
   expired (dt >= t308):
0099992c  FSUBR [ESI+0x304]          ; ST0 = t304 - dt
00999934  FADDP / 00999936 FSTP [ESI+0x308]   ; t308 += t304 - dt
0099993c  CALL 0x0099b740            ; BSP_BotTask_AbandonIfStale, ECX = task
   not expired:
009999b5  FLD ST0 / FSUBP ST2,ST0 / FXCH / FSTP [ESI+0x308]   ; t308 -= dt
009999c1  JMP 0x00999945             ; skips 0099b740
```
(The two paths join at `00999945` with the same x87 depth: the not-expired path leaves `dt` in
`ST0`, which is exactly what `00999941 FLD [ESP+0x10]` supplies on the other path.)

Then:

```
00999950  MOV dword ptr [ESI+0x2e4],0xff
0099995a  CALL EAX                   ; task->vtable[64h](dt)
0099995c  MOV ECX,[ESI+0x2f4]        ; the unit
00999962  CMP byte ptr [ECX+0xc24],0 / JZ 0099997e
00999979  CALL 0x009fc7c0            ; ECX = task+314h, (dt)
0099998c  CALL 0x009fd0e0            ; ECX = task+38Ch, (dt)
0099999b  CALL 0x009a17d0            ; ECX = task, (dt)
009999aa  CALL 0x0099d300            ; the planner, ECX = EDI = plan
```

### The branch condition — settled

```
0099c270: MOV EAX,[ECX+0x2f4]                    ; the unit
0099c276: TEST EAX,EAX / JZ -> return 0
0099c27a: MOVZX ECX,word ptr [0x00f876b8]
0099c281: CMP byte ptr [EAX + ECX*8 + 0x9c2],0 / JZ -> return 0
0099c28b: MOV EAX,1 / RET
```

**`0099C270(task)` returns true iff `task+2F4h != 0` and `unit[9C2h + idx*8] != 0`.**
So `task+2E4h` is `0` when that byte is set and `0FFh` when it is clear.

This is the same byte that gate 12 of the tick (`0099AF33`) requires to be **zero**. Consequently,
**on any tick that reaches the commit at `007B8C90`, `009998A0` took the false branch and
`task+2E4h == 0FFh`**. The short branch's plan is computed and then thrown away by gate 12 in the
same tick. Caveat: the two reads are ~0x60 bytes apart with `009998A0`'s whole callee tree in
between, so a callee writing the byte would break the implication; I did not census writers of
`unit+9C2h+idx*8`.

A provisional reading that fits every use: `idx = [00F876B8]` is a local player/slot index and
`unit[9C2h + idx*8] != 0` means the unit is under human control for that slot — which is why the
tick then suspends itself for 3.0 s (`0099AF3D`) and why the AI runs a reduced arm. **Provisional**;
the byte's meaning was not recovered.

### The entry gate of `009998A0`

```
009998a4  CMP byte ptr [ESI+0x270],0x2 / JNZ 009998cc
009998ad  MOV EAX,[ESI+0x274] / TEST / JZ 009998cc
009998be  CMP byte ptr [EAX + ECX*8 + 0x9c2],0x0 / JNZ 009999b0   ; return, doing nothing at all
```
When `task+270h == 2` **and** the target `task+274h` is non-null **and** the target's own
`[9C2h + idx*8]` byte is set, `009998A0` returns without even seeding the plan. Note the exit at
`009999B0` correctly skips `POP EDI`, because `PUSH EDI` is at `009998CC`, after the gate.

---

## (3) `0099D300` — the entry, and what it does *not* read

I read `0099D300`-`0099D510` (the first 110 instructions) plus a complete offset census over the
whole 1451-instruction body. I did **not** read the arms; `docs/PILOT_BOT_PLAN_CONTROLS.md` covers
them and I take the following from it on trust, unverified here: the five 12-byte slot layout at
`plan+274h` stride `0Ch`; the yaw arm `0099E81A`-`0099EA46`; the pitch terminal law
`0099E68D`-`0099E752`; the stick-override block `0099D64A`/`0099D688`/`0099D6C6`; the speed-hold
arm `0099D8C1`; the power ceiling `0099DC8F`; the bank-target arm `0099DCA4`-`0099DD64`; and the
per-tick frame `0099D46E`-`0099D510`.

`ESI = plan` throughout, so `plan+26Ch = task+270h`, `plan+270h = task+274h`, `plan+2F0h = the unit`.

### Entry gate A — the neutral plan, `0099D309`-`0099D3C2`

All four conditions must hold to take it:

| address | field | required |
| --- | --- | --- |
| `0099D309` | `plan+26Ch` (byte) = `task+270h` | `== 2` |
| `0099D329` | `unit[9C2h + idx*8]` (byte) | `!= 0` |
| `0099D33D` | `plan+270h` = `task+274h`, the target entity | `!= 0` |
| `0099D345` | `target[9C2h + idx*8]` (byte) | `!= 0` |

`idx*8` is built here as `EAX+EAX` three times (`0099D323`-`0099D327`), then used as `EAX*1`.

It then writes a complete neutral plan and returns:

| store | slot field | value |
| --- | --- | --- |
| `0099D35A` / `0099D362` / `0099D369` | yaw desired `+284h` / active `+288h` / `+2D4h` | `0.0f` / `1` / `0` |
| `0099D36F` / `0099D377` / `0099D37E` | pitch desired `+29Ch` / active `+2A0h` / `+2D0h` | `0.0f` / `1` / `0` |
| `0099D384` / `0099D38C` / `0099D393` | roll desired `+290h` / active `+294h` / `+2CCh` | `0.0f` / `1` / `0` |
| `0099D399` / `0099D3A1` | power desired `+278h` / active `+27Ch` | `1.0f` (`00D7A24C`) / `1` |
| `0099D3A8` / `0099D3B0` / `0099D3B7` | brake desired `+2A8h` / active `+2ACh` / `+2D8h` | `0.0f` / `1` / `0` |

**The `active` field at slot+8 is a byte, not a dword** (`MOV byte ptr [ESI+0x288],0x1`, and the same
form at `0099D362`, `0099D377`, `0099D38C`, `0099D3A1`, `0099D3B0`, `0099D409`, `0099D41E`,
`0099D643`, `0099D681`, `0099D6BF`, and `MOV byte ptr [ESI+0x27c],CL` at `0099D8D7`). It is also
*read* as a byte at `0099D7CB`, `0099D817`, `0099D90A`, `0099D977`. `docs/PILOT_BOT_PLAN_CONTROLS.md`
and `docs/PLANE_BOT_CONTROL_WRITEBACK.md` both show it as a plain field without a width; the width
is settled here. I did not re-read `0099B450` to check the width of its zeroing store.

### Entry gate B — wings level at low airspeed, `0099D3C5`-`0099D42B`

```
0099d3cb  MOV EDX,[EAX+0x72c] / LEA ECX,[EAX+0x72c] / MOV EAX,[EDX+0x38] / CALL EAX
0099d3de  TEST AL,AL / JZ 0x0099d42b
0099d3f0  MOVSS XMM1,[0x00ce3d34]          ; 4.0f
0099d3f8  COMISS XMM1,[ECX+0x908]
0099d3ff  JBE 0x0099d42b                   ; taken when 4.0f <= unit+908h
```
When `(unit+72Ch)->vtable[38h]()` is true **and** `unit+908h < 4.0f`, the yaw and roll slots are
zeroed and marked active (`0099D401`-`0099D425`), exactly as in gate A but without pitch, power or
brake. Then, unconditionally:

* `0099D431` `unit+520h = 0` (byte)
* `0099D446` `plan+2ECh = [00E0E2EC]`, replaced by `[00E0E2F4]` at `0099D466` when
  `unit+900h ∈ {4,5}` (`0099D454`/`0099D459`)
* `0099D46E` onward is the per-tick frame the other doc already covers.

### The frame overwrites the `dt` argument too — **CORRECTION**

`docs/PILOT_BOT_PLAN_CONTROLS.md`'s frame table lists `[ESP+38h]`, `[ESP+28h]`, `[ESP+10h]`,
`[ESP+34h]`, `[ESP+1Ch]`, `[ESP+20h]`, `[ESP+30h]`, `[ESP+2Ch]` but not `[ESP+6Ch]`:

```
0099d4ea  FLD  [ESP+0x28]     ; 1 / max(unit+340h * 0.4, 1.0)
0099d4ee  FMUL [ESP+0x6c]     ; * dt
0099d4f2  FSTP [ESP+0x6c]     ; *** the dt argument slot is replaced by dt / max(s, 1.0) ***
```
`[ESP+6Ch]` is the incoming float argument: `SUB ESP,58h` + `PUSH ESI` + `PUSH EDI` + `PUSH EBX`
(`0099D3DA`) + `PUSH EBP` (`0099D3DB`) = `0x68`, so the argument at entry+4 sits at `[ESP+6Ch]`.
Every later arm that reads `[ESP+6Ch]` therefore sees a speed-scaled `dt`, and since that `dt` was
already the ~0.09 s think interval (headline 2), it is two substitutions away from a frame delta.

### How a heading reaches the planner — it is already there

An exhaustive census over the whole body:

* **`plan+270h`, the task's target entity, appears exactly once: `0099D337`, in gate A.**
  `0099D300` never dereferences the target, never reads a position, and computes no bearing.
* The heading target is read as a float from `plan+2C0h` at **`0099DEAF`** — one read, in the yaw
  base term that `docs/PILOT_BOT_PLAN_CONTROLS.md` attributes to `0099DE8A`.
* The speed target is read from `plan+2B4h` at `0099D8C6`, `0099DA52`, `0099DAEC`.
* The bank target is slewed through `plan+2BCh` (`0099D745`, `0099DD4E`, `0099DD64`, `0099E4E8`,
  `0099E512`).

So the target-to-heading conversion happens **before** `0099D300`, in `task->vtable[64h]`, which
both branches of `009998A0` call immediately beforehand (`009998FB`, `0099995A`). That is consistent
with `BSP_BotStateMoveTo_Tick` at `009C1B71`-`009C1B91` writing those same target/mode offsets on an
object it reaches as `(*ESI)+18h`.

### Which plan slots the planner writes

Desired-value stores over the whole body (`MOVSS [ESI + <desired>]`):

| slot | desired | writers inside `0099D300` |
| --- | --- | --- |
| power | `+278h` | `0099D399` (gate A), `0099D8CF` (speed hold), `0099DC8F` (ceiling) |
| yaw | `+284h` | `0099D35A` (gate A), `0099D401` (gate B), `0099D63B` (stick override), `0099EA3E` (the yaw law) |
| roll | `+290h` | `0099D384` (gate A), `0099D416` (gate B), `0099D6B7` (stick override) |
| pitch | `+29Ch` | `0099D36F` (gate A), `0099D679` (stick override), `0099E739` (the pitch law) |
| brake | `+2A8h` | `0099D3A8` (gate A), `0099D8DD` (speed hold) |

All five slots are written on every non-early-out pass, so the "simplest attack arm" is not a
distinct code path in `0099D300`: the shared planner always fills all five, and the per-kind arm
differentiates by what it wrote into `plan+2B4h`/`+2C0h` and the mode dwords beforehand.

### The four mode dwords — not assigned, deliberately

`0099D300` clears four dwords in gate A: `+2CCh`, `+2D0h`, `+2D4h`, `+2D8h`, written adjacent to
the roll, pitch, yaw and brake slots respectively. **I am not claiming that adjacency is the
mapping.** `docs/PILOT_BOT_PLAN_CONTROLS.md` assigns `+2CCh` to the heading and `+2D0h` to the bank
on the strength of their *consumers* (`0099DE8A`, `0099DCE0`), which is better evidence than store
order, and it does not mention `+2D4h` at all. The two readings conflict; the consumer-based one
should stand until someone traces all four reads. What is established here is only that all four
are dwords, all four are cleared at `0099D369`/`0099D37E`/`0099D393`/`0099D3B7`, and that `+2CCh`
and `+2D4h` are cleared again by gate B.

---

## What is *not* established

* **No runtime measurement.** Which gates a live or reconstructed host actually passes is unknown.
  Everything above is static.
* **What sets `bot+7Ch`.** Two candidates (`009C1BA8`, `009BDD30`), neither tied to this bot's
  class. The scans that produced them cover only `disp8` literal-offset stores of an immediate or a
  byte register; SIB-indexed writes, a sub-object at a different literal offset, and block copies
  are all invisible to them.
* **The meaning of `unit+9C2h + idx*8`**, and of `unit+9C1h`/`+9C3h` in the same 8-byte record. The
  player-control reading is a hypothesis that fits four use sites, not a recovered fact.
* **The meaning of `unit+900h`'s values** `4`, `5`, `6`, `7`, and of `unit+1ACh`'s value `8`.
* **`unit+5Dh`, `unit+60h`, `unit+61h`, `(unit+9D4h)+61h`, `unit+C24h`, `unit+520h`, `unit+908h`,
  `unit+C58h`, `unit+DF0h`, `unit+DF8h`** are named here only by the role they play in a gate.
* **Whether a callee of `009998A0` can change `unit[9C2h + idx*8]`** between `0099C281` and
  `0099AF33`. If one does, headline 3 fails. I did not census writers of that byte.
* **`0099D300`'s body between `0099D510` and `0099EA46` was not read in this packet**, only
  offset-censused. The arms are taken from `docs/PILOT_BOT_PLAN_CONTROLS.md` on trust, as listed
  above.
* **`00999E40`, `00999EE0`, `00999F50`, `009A17D0`, `009FC7C0`, `009FD0E0`, `0099B740`, `009CFF40`,
  `007B9140`'s tail, `00927F10`'s table at `[00E188A8]+18CCh`** were not read beyond what is quoted.
* **The width of `0099B450`'s zeroing stores to the `active` fields** was not re-checked against the
  byte width established at `0099D362`.
* The vtable slot index `64h` is a byte offset; the owning class of the task vtable and the set of
  kinds that override slot `64h` were not enumerated here.
