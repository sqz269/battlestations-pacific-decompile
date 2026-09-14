# `task->vtable[64h]` — the per-kind arm that fills `plan+2C0h` (packet `cc7-heading-arm`)

Addresses: `009998FB`/`0099995A` (the two call sites), the vtable slot `00D1FCD4` and its thirteen
siblings, `009AF510` and `009AE8A0` (two arms), `009AC190` (the state tick that produces the
heading), `009ABBC0`, `009AC0B0`, `009F9980`, `009F9CE0`, `009FB200`, `009ABAB0`, `009AD720`,
`009ADBF0`, `009AEBE0`, `009AB740`. Constants `00CE3830`, `00CE3828`, `00D7A264`, `00CE3DC0`,
`00CE38B8`, `00CE3D10`, `00CE3814`, `00CE74F8`, `00CE380C`, `00D7A2B0`, `00CE3938`, `00D7A208`,
`00D7A220`, `00D7A24C`.

Worker `cc7-heading-arm`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was **read-only**: no rename, no comment, no prototype, no
tag, no write lock, no save, no lease, no git; no existing file in this repository was edited —
this document is the only thing written. Every descriptive name below is a hypothesis.

## Headline

1. **`task->vtable[64h]` is not itself the heading producer.** It is a thin per-kind arm that
   runs the kind's own bookkeeping and then dispatches one virtual call:
   `state = task->+310h; state->vtable[0Ch](dt)`. The heading is written by that **state tick**.
   Proven at `009AE90F`-`009AE91E` (DropKamikaze) and `009AF539`-`009AF545` (Kamikaze).
2. **The heading is a bearing to a *cached* world point, not to a live entity.** The state reads
   the target position through a virtual getter (`009ABBC0`) that just copies three floats out of
   the task's own memory. Those floats were latched **once, at task construction**, from the
   commanded entity's world position (`009FB32A`-`009FB345`). Nothing in the tick path
   re-reads the entity, so a moving ship's bearing is not re-derived per tick by this arm.
3. **The arm I traced belongs to `Kamikaze/gotowards`, not to `DropKamikaze/attackrun`.**
   The `.rdata` string that precedes a vtable does **not** name it: the vtable at `00D1FB84`
   (whose slot `0Ch` is `009AC190`) sits immediately after the `DropKamikaze` state-name block but
   is installed at `009AEE13` on the **Kamikaze** task's state at `ctx+25Ch`, registered as
   `"Kamikaze/gotowards"` (`009AEE4A`/`009AEE4B`, name string `00D1FD1C`). `DropKamikaze/attackrun`
   is a different state (`ctx+1ACh`, vtable `00D1FBEC`, tick `009ACFC0`).

---

## (0) Object model, so the offsets below are unambiguous

Three distinct objects are involved. Naming them is necessary because the existing docs use
"task" and "bot" for more than one of them.

| name here | what it is | how it is reached |
| --- | --- | --- |
| `T` — the **pilot task** | `009998A0`'s `this`; the element at the front of the pilot bot's list (`*[bot+58h]`, `0099AE8C`) | `0099AF17 MOV ECX,EBX` |
| `plan` | the embedded plan, **`T+4`** | `009998CD LEA EDI,[ESI+4]`; `0099D300`/`0099B450` both take it as `this` |
| `ctx` — the **bot task** | a sub-object at **`T+3F8h`** carrying the state machine | `009ADC20 LEA EDI,[ESI+0x3f8]`, `009AE8A9 LEA ECX,[ESI+0x3f8]` |

`ctx`'s own layout, from its root constructor `009F9CE0` (which installs vtable `00D21C74`):
`+0` vtable, `+4` = the unit, `+8` = `unit+538h` (vehicle class descriptor), `+0Ch` = `unit+9D4h`,
`+10h` = `unit+DF4h`, `+14h` = a per-team record, `+18h`/`+1Ch`/`+20h` = 0.

`009F9980` — `void __thiscall(ctx, T*)`, `RET 4` — wires `ctx` back to the pilot task, and this is
the single fact that ties the two halves of the chain together:

```
009f9988  LEA EDX,[EAX + 0x4]        ; EAX = T
009f998f  MOV [ECX + 0x18],EDX       ; ctx->+18h = &T->plan   (= T+4)
009f9992  LEA EDX,[EAX + 0x314]
009f999d  MOV [ECX + 0x1c],EDX       ; ctx->+1Ch = T+314h
009f99a0  MOV [ECX + 0x20],EAX       ; ctx->+20h = T+38Ch  (EAX was ADDed at 009f9998)
```

`T+314h` and `T+38Ch` are exactly the two sub-objects `009998A0` ticks at `00999979` and
`0099998C`, which is an independent confirmation that `009F9980`'s argument is `009998A0`'s `this`.
Called at `009ADC7C` (`MOV ECX,EDI` = ctx, `PUSH ESI` = T).

**So `ctx->+18h` is the plan, and every `+2C0h`/`+2CCh`/`+2D8h` store a bot state makes through
`[[state+4]+18h]` lands on the same object `0099D300` reads.** A state's owner back-pointer is at
`state+4` (`009AEDFC MOV [EAX+4],ESI` with `EAX = ctx+25Ch`, `ESI = ctx`).

## (1) The vtable slot, and the thirteen arms

Every pilot-task vtable shares the run `0099B6F0, 0099B700, 0099B710` at `+30h`/`+34h`/`+38h`.
Scanning `.rdata` for that 12-byte run finds **14 vtables**; slot `64h` is the **last** entry of
each (the vtables are `68h` bytes long — for `00D1FC70` a string begins at `00D1FCD8`).

| vtable base | `[base+64h]` | identification |
| --- | --- | --- |
| `00D05708` | `007B4160` | base/default; string `"not defined"` follows the vtable |
| `00D05918` | `007B7C40` | not identified |
| `00D1F738` | `009A66C0` | `BSP_BotTaskDepthCharge_Tick` (Ghidra name); `"dogfight/avoid_t…"` follows |
| `00D1F9B0` | `009AB1C0` | not identified; `"_autoaim"` follows |
| `00D1FC70` | `009AE8A0` | **DropKamikaze** (`009ADC41`); `"Kamikaze/moveto"` follows |
| `00D1FD40` | `009AF510` | **Kamikaze** (`009AEF41`); near `BSP_BotTaskKamikaze_UpdateCruiseProfile` |
| `00D20210` | `009B8B50` | not identified |
| `00D20910` | `009BD990` | not identified; `"circle (moveto)"` follows |
| `00D20B68` | `009C3950` | not identified |
| `00D20BE0` | `009C3950` | same arm as the previous one |
| `00D20E18` | `009C8790` | not identified |
| `00D20F60` | `009CA130` | not identified |
| `00D210E0` | `009CD170` | not identified |
| `00D213C8` | `009D4850` | not identified |

Ghidra has a defined function at only one of these (`009A66C0`); the rest fall in unanalysed gaps
and were read with `disasm-raw`. **I read two of the fourteen arms in full — `009AE8A0` and
`009AF510`. The other twelve were not read**, so the "arm ends in a `state->vtable[0Ch](dt)`
dispatch" shape is established for two kinds, not for all.

### ABI

`void __thiscall(T*, float dt)` — `ECX = T`, `dt` as one 4-byte stack argument, callee cleans it.
Both call sites reserve the slot with `PUSH ECX` and fill it with `FSTP [ESP]`
(`009998F3`/`00999952`); `docs/PILOT_BOT_TICK_GATES.md` establishes that every `009998A0` callee is
`RET 4`. The state tick it dispatches to has the same shape: `009AC7B9 RET 0x4`.

### `009AF510` — the Kamikaze arm, in full

```
009af510  FLD [ESP+4]                 ; dt
009af518  LEA ECX,[ESI + 0x3f8]       ; ctx
009af521  CALL 0x9af0a0               ; ctx->Update(dt)
009af52b  MOV ECX,ESI
009af530  CALL 0x9af310               ; T::<kind bookkeeping>(dt)
009af539  MOV ECX,[ESI + 0x310]       ; the current state
009af53f  MOV EAX,[ECX]               ; its vtable
009af541  MOV EDX,[EAX + 0xc]         ; slot 0Ch = Tick
009af545  FSTP [ESP] / CALL EDX       ; state->Tick(dt)
```

`009AE8A0` (DropKamikaze) is the same shape: `CALL 0x9ade00` on `T+3F8h`, then
`009AE90F`-`009AE91E` `state = [T+310h]; state->vtable[0Ch](dt)`. `T+310h` is set in the task
constructor — `009ADC6C MOV [ESI+0x310],ECX` with `ECX` = `T+4D0h` or `T+50Ch`, which are
`ctx+D8h` and `ctx+114h`, the two states `009AB740` registers as `"moveto (DropKamikaze)"` and
`"follow (DropKamikaze)"`.

## (2) `009AC190` — `Kamikaze/gotowards`, the state that produces the heading

`void __thiscall(state, float dt)`, `RET 4`, 433 instructions, body `009AC190`-`009AC7B9`.
`ESI = state`, `state+4 = ctx`. It calls `LIBCRT_atan2` (`00BF701A`), `BSP_Math_AddWrappedAngle`
(`00438AA0`), `BSP_Math_SubtractWrappedAngle` (`00438B10`), `BSP_Math_InterpolateClamped`,
`BSP_EntityPose_RefreshWorld` (`00414DB0`).

Identification: vtable `00D1FB84` (slot `0Ch` = `009AC190`, the function's only data xref is
`00D1FB90`); installed at `009AEE13` on `ctx+25Ch`; `ctx+25Ch` is registered under the name string
`00D1FD1C` = `"Kamikaze/gotowards"` at `009AEE4A`-`009AEE52` (`EAX` still holds `ESI+0x25c` from
`009AEDF6`). The same state class is also constructed at `009ABE37`, inside `009ABBE0`, whose only
Ghidra-known caller is `009AD920` — a second DropKamikaze task constructor. **Which name it carries
there was not determined.**

### Stack-frame normalisation (read this before trusting any `[ESP+n]` below)

`tools/stack_frame_walk.py` mis-tracks this function: it assumes indirect calls pop nothing.
Three indirect calls on the straight-line path each pop 4 (`009AC1A7` after `PUSH 0x17`,
`009AC1C8` after `PUSH EAX`, `009AC2F5` after `PUSH EAX`), so the walker's depth is 4/8/12 too
large from those points on. Cross-check that fixes it: `009AC1C3 LEA EAX,[ESP+0x38]` and
`009AC1E0 FLD [ESP+0x38]` must name the same slot, and they do only under the corrected depths.

Corrected entry-relative frame offsets (`frame = literal − depth`, depth 80 after the prologue):

| frame | holds | written at | later reused for |
| --- | --- | --- | --- |
| `-24 … -16` | the target position `{x,y,z}` (out-param) | `009AC1C8` (callee) | re-filled at `009AC2F5` into `-12 … -4` |
| `-36` | `dx = target.x − self.x` | `009AC1ED` | `009AC2CD` overwrites it with `dist*0.2f` |
| `-28` | `dz = target.z − self.z` | `009AC1FB` | — |
| `-48` | `d`, the 3-D range | `009AC204` | `009AC2D1` overwrites it with `|headingError|` |
| `-44` | raw `atan2` | `009AC23A` | `009AC2EA` (a double), then `009AC38F`, `009AC3B2` |
| `-68` | **`h`, the heading target** | `009AC248` / `009AC25C` | not overwritten before the store |
| `-56` | the value that reaches `plan+2C0h` | `009AC275`, `009AC3F1` | `009AC443` (the elevation) |
| `-60` | heading error, then `self.y − target.y` | `009AC290`, `009AC301` | — |
| `-64` | the turn-distance estimate | `009AC33D` | — |

Frame `-56` has exactly two writers before the store at `009AC40B`, and I traced the store
backwards to them rather than forwards from the first producer.

### The expression

```
009ac1bc  MOV ECX,[ESI+4]            ; ctx
009ac1bf  MOV EDX,[ECX] / [EDX]      ; ctx->vtable[0]
009ac1c3  LEA EAX,[ESP+0x38]         ; &out   (frame -24)
009ac1c8  CALL EDX                   ; ctx->GetTargetPosition(&out); returns &out in EAX
009ac1cd  MOV EDI,[ctx+4]            ; the unit (self)
009ac1d0  if (self->+C8h == 0) BSP_EntityPose_RefreshWorld(self)
009ac1e0  FLD [frame -24] / FSUB [EDI+0xfc]  / FSTP [frame -36]   ; dx = target.x - self.x
009ac1f1  FLD [frame -16] / FSUB [EDI+0x104] / FSTP [frame -28]   ; dz = target.z - self.z
009ac228  FLD [frame -28]            ; ST1 <- dz
009ac231  FLD [frame -36]            ; ST0 <- dx
009ac235  CALL 0x00bf701a            ; _CIatan2: ST0 = atan2(ST1, ST0) = atan2(dz, dx)
009ac23a  FSTP [frame -44]
009ac23e  FLD [frame -44]
009ac242  FSUBR double [0x00ce3830]  ; ST0 = 1.5707963705062866 - atan2(dz, dx)
009ac248  FSTP [frame -68]
009ac24c..009ac25c  if (h < 0) h += 6.2831854820251465   ; double [0x00ce3828]
009ac275  MOVSS [frame -56],XMM0     ; XMM0 = [frame -68] = h
...
009ac3f9  MOV EDX,[ESI+4]            ; ctx
009ac400  MOV EAX,[EDX+0x18]         ; the plan  (= T+4)
009ac405  MOVSS XMM0,[frame -56]
009ac40b  MOVSS [EAX + 0x2c0],XMM0   ; plan->headingTarget
009ac41b  MOV dword [EAX + 0x2cc],2  ; plan->headingMode = 2
```

So, for this arm:

```
h = wrap2pi( PI_2 - atan2(target.z - self.z, target.x - self.x) )
plan->+2C0h = h                                  (normal case)
            = AddWrappedAngle(h, PI)             (the reverse/abort case, below)
plan->+2CCh = 2
```

* **Units: radians.** `PI_2` is the double `0x3FF921FB60000000` = `1.5707963705062866`, which is
  the *float* `π/2` widened, not the double `π/2`; the wrap constant `0x401921FB60000000` =
  `6.2831854820251465` is likewise `2 × (float π)`; the reverse offset `00D7A264` is the float
  `π` = `3.14159274f`. That matters only to someone matching bit-for-bit, and it is the same
  pattern `docs/PILOT_BOT_PLAN_CONTROLS.md` records for the `π/6` in the depth-charge arm.
* **It is an absolute heading**, not relative: the current heading is subtracted only later, by the
  planner's yaw base term (`SubtractWrappedAngle(plan+2C0h, unit+C6Ch)` at `0099DEB8`), and by this
  state at `009AC28B` for its own turn-distance estimate.
* **Convention.** `h = 0` points along `+Z` (`dz>0, dx=0` → `atan2 = π/2` → `h = 0`); `h = π/2`
  points along `+X`. Wrapped to `[0, 2π)`. `self` is the unit's world position at
  `unit+FCh/+100h/+104h` = `{x, y, z}`; only `x` and `z` enter the bearing, so it is a **ground
  bearing**, horizontal, with the vertical difference used separately (see the reverse case).
* **`atan2` argument order.** `00BF701A` is a thunk (`MOV EDX,0xe15c20; JMP 0x00c07ee0`), so its
  identity rests on Ghidra's naming plus two local facts: it consumes two x87 registers and
  produces one (the stack is empty before `009AC228` and empty again after `009AC23A`), and the
  second use in the same function, `009AC43E`, computes `atan2(target.y − self.y, range)` — an
  elevation angle — which is only physically sensible with `ST1` as the numerator. That is the
  `_CIatan2` convention (`FPATAN` = `atan(ST1/ST0)`). **`dz` is loaded first, so it is the
  numerator.**
* **x87 ambiguities settled by bytes:** `009AC389 = D8 E9` → `FSUBR ST(0),ST(1)`, i.e. `ST0 =
  ST1 − ST0`; `009AC35A = DD D8` → `FSTP ST(0)`; `009AC403 = D9 E0` → `FCHS`. No `D8 C8+i` /
  `DC C8+i` pair occurs in the traced region.

### The reverse case (`h + π`)

The state latches a "turn away and re-attack" flag in `state+1Dh`.

```
BL          = unit->vtable[5Ch](0x17) && unit->+C24h == 0        (009ac1a7 .. 009ac1b6)
range       = max(d, 10.0f)                     -> state+18h     (009ac208 .. 009ac22c)
err         = SubtractWrappedAngle(h, unit->vtable[50h]())        (009ac28b)
|err|       = err > 0 ? err : (-0.0f - err)                       (009ac29e .. 009ac2b0)
dy          = self.y - target.y                                   (009ac2fc, 009ac301)
turnDist    = ((BL ? 1.2f : 0.8f) * |err| + 3.0) * classDesc[188h] (009ac329 .. 009ac33d)

if (state+1Dh != 0)                       ; already turning away
    if (range <= turnDist + 50.0) -> take the +PI path
    else state+1Dh = 0 and write h unchanged
else
    m = min((dy - 100.0) * (BL ? 1.5f : 1.0f), turnDist)
    if (m > range) and (BL ? (range*0.2f <= dy) : true)
        state+1Dh = 1; take the +PI path
```

`unit->vtable[50h]` is the heading getter that `docs/PILOT_BOT_PLAN_CONTROLS.md` resolves to
`unit+C6Ch`; `classDesc = ctx+8 = [unit+538h]`, and `classDesc+188h` is a float (its meaning is not
recovered — the shape `k*|Δheading| + c` scaled by it reads as "ground distance needed to complete
the turn", which is a hypothesis, not a recovered formula). I did **not** verify that `009AC2F5`'s
second `GetTargetPosition` call returns the same point as the first; it is the same virtual on the
same object, so it does unless a callee in between mutates the cache.

### What else this state writes into the plan

| address | store | value |
| --- | --- | --- |
| `009AC40B` | `plan+2C0h` float | the heading above |
| `009AC41B` | `plan+2CCh` dword | `2` |
| `009AC42B` | `plan+2E8h` float | `1.2f` (`00CE3814`) |
| `009AC674` | `plan+2BCh` float | a bank command from `BSP_Math_InterpolateClamped` (`00419010`) at `009AC640`; inputs not traced |
| `009AC67C` | `plan+2D0h` dword | `1` |
| `009AC714` | `plan+278h` float | `1.0f` — the **throttle slot's `desired`** |
| `009AC71F` | `plan+27Ch` byte | `1` — the throttle slot's `active` |
| `009AC726` | `plan+2A8h` float | `0.0f` — the **air-brake slot's `desired`** |
| `009AC72E` | `plan+2ACh` byte | `1` — the air-brake slot's `active` |
| `009AC735` | `plan+2D8h` dword | `0` (speed-hold off) |
| `009AC74C` | `plan+2E4h` byte | `1` |

It does **not** write `plan+2B4h` (the speed target).

## (3) How a commanded target entity becomes those three floats

`009ABBC0` — `vec3* __thiscall(ctx, vec3* out)`, `RET 4`, returns `out` in `EAX` — is the whole
getter:

```
009abbc0  MOV EAX,[ESP+4]
009abbc4  FLD [ECX+0x4c] / FSTP [EAX]
009abbc9  FLD [ECX+0x50] / FSTP [EAX+4]
009abbcf  FLD [ECX+0x54] / FSTP [EAX+8]
```

`ctx+4Ch..+54h` is `ctx+30h + 1Ch..24h`, a sub-object constructed by `009FB200` (vtable
`00D21CB4`) with the commanded target as its argument:

```
009fb22d  EDI = arg                       ; the commanded entity
009fb23f  EAX=[EDI]; EDX=[EAX+0x5c]; PUSH 5; ECX=EDI; CALL EDX   ; entity->vtable[5Ch](5)
009fb24a  TEST AL,AL / JE 009fb252        ; fails the test -> treat as null
009fb26c  MOV [ESI+0x14],EAX              ; holder->+14h = the accepted entity   (= ctx+44h)
...
009fb31b  if (entity->+C8h == 0) BSP_EntityPose_RefreshWorld(entity)
009fb32a  FLD [EDI+0xfc]  / FSTP [ESI+0x1c]   ; = ctx+4Ch  <- entity.x
009fb337  FLD [EDI+0x100] / FSTP [ESI+0x20]   ; = ctx+50h  <- entity.y
009fb340  FLD [EDI+0x104] / FSTP [ESI+0x24]   ; = ctx+54h  <- entity.z
```

and, when there is no entity (`009FB36B`), from a global float3 at `00F87574`/`+4`/`+8` instead.
`009FB3E0` is a second function that re-loads the same three fields from that global.

The chain of ownership that delivers the entity: `BSP_BotTask_MakeDropKamikaze` (`009AEBE0`,
`ECX` = pilot bot, `EDX` = the target) → `BSP_BotTaskDropKamikaze_Construct` (`009ADBF0`) →
`0099C6F0` (the pilot-task base, kind `5`) and `009AD720(ctx, pilotBot, target)` → `009ABAB0` →
`009FB200(ctx+30h, target)`, which latches the position above. The Kamikaze family reaches the same
holder through its own constructor chain.

So the answer to "how a commanded target entity becomes the float in `plan+2C0h`" is:

```
entity  --(task construction, 009FB32A)-->  ctx+4Ch..54h (a fixed world point)
        --(009ABBC0)-->  out
        --(009AC1E0..009AC235)-->  atan2(out.z - self.z, out.x - self.x)
        --(009AC242..009AC25C)-->  h = wrap2pi(PI_2 - that)
        --(009AC40B)-->  plan+2C0h,  plan+2CCh = 2
```

## Contradictions and corrections to existing documents

* **`docs/PILOT_BOT_TICK_GATES.md`, "How a heading reaches the planner"** says the conversion
  happens "in `task->vtable[64h]`". That is true only at one remove: `vtable[64h]` dispatches to a
  bot-state tick and writes no heading itself. Anyone reimplementing `vtable[64h]` as a single
  function will not find the bearing there.
* **The same document's caveat on `task+2E4h`** (`0` on the true branch, `0FFh` on the false
  branch, set at `009998F1`/`00999950`) does **not** survive the arm. `009AE8A0` writes
  `T+4B4h = 0FFh` at `009AE8B2` and then copies it over the field at
  `009AE92D MOV [ESI+0x2e4],EAX` (`EAX = [ESI+0x4b4]`), *after* the vtable dispatch. For the
  DropKamikaze kind, `task+2E4h` is `0FFh` on **both** branches by the time `0099D300` runs, unless
  a callee changes `T+4B4h` in between. I did not check the other thirteen arms for this.
* **`docs/PILOT_BOT_PLAN_CONTROLS.md`** calls the `+2C0h` carrier "the bot" and gives the access
  form `[[ESI]+18h]` at `009A3845`. `009A383F`-`009A3845` really is `MOV EDX,[ESI]` /
  `MOV EAX,[EDX+0x18]`, so that transcription is correct for *that* site, but the carrier is the
  **plan**, `T+4` — the same object `0099D300` runs on — and in `009AC190` the access form is
  `[[ESI+4]+18h]` (`009AC3F9`/`009AC400`). Both forms exist; `ESI`'s binding at `009A383F` was not
  checked here.
* **`docs/PLANE_BOT_CONTROL_WRITEBACK.md`** records the vtable at `00D1F354` as unidentified and
  offers it as the lead for the `+64h` slot. It is **not** the task vtable. Its base is `00D1F31C`
  (a `"PilotBotCmd("` string ends at `00D1F31B`), `0099ACD0` sits at `+38h`, and it is `74h` bytes
  long — 29 entries, with no `+64h` function of the kind wanted. Its `this` is the **pilot bot**
  (`[this+50h]` = the unit, `[this+58h]` = the task list); the pilot task `T` is a separate object,
  `*[bot+58h]`, and its vtables are the fourteen listed in section (1).
* **The `+2C0h` producer census in `PILOT_BOT_PLAN_CONTROLS.md`** is not contradicted but is
  now named. Its fifth row, `009AC40D / 009AC41B (MOVSS form)`, is the store traced here: the
  instruction starts at `009AC40B` and `009AC40D` is two bytes in, and the mode store at
  `009AC41B` matches exactly. Its enclosing function `FUN_009ac190` is `Kamikaze/gotowards`.
  That row is also the one the census left without a function name.

## What is *not* established

* **Twelve of the fourteen `vtable[64h]` arms were not read.** Only `009AE8A0` (DropKamikaze) and
  `009AF510` (Kamikaze) were. The task-kind identification in section (1) is from the string that
  follows each vtable in `.rdata` plus one Ghidra name, and section (0)'s headline 3 is a worked
  example of why that inference is unreliable — treat every "not identified" row as unknown and the
  identified ones as provisional except `00D1FC70` and `00D1FD40`, which are pinned by the
  `MOV [ESI],imm32` in their constructors.
* **`DropKamikaze/attackrun` (`009ACFC0`, vtable `00D1FBEC+0Ch`) was not read.**
  `PILOT_BOT_PLAN_CONTROLS.md` summarises it as `bot->+2C0h = owner[+B8h]` — a stored heading
  copied out — which, if right, means that kind's heading is produced somewhere else again.
  The other five listed producers (`009A3ED3`, `009A42B3`, `009A7324`, `009AD566`, and
  `009A384C`) were not read here either.
* **Whether the cached target position is ever refreshed.** `009FB200` latches it at construction
  and `009FB3E0` reloads it from the global. I did not find a per-tick refresh from the entity, but
  I also did not exhaustively census writers of `ctx+4Ch`: the scans I ran cover `MOVSS`/x87 stores
  with a `disp8` of `0x4C` (`f3 0f 11 ?? 4c` full-binary, `d9 ?? 4c` full-binary) and a `disp32` of
  `0x444` (`f3 0f 11 ?? 44 04 00 00`). Integer `MOV` stores, `REP MOVSD`, SIB-indexed writes and
  stores through a pointer held in a register are all invisible to them. The claim "nothing in the
  tick path re-reads the entity" is therefore bounded by: I read `009AF510`, `009AF0A0`'s first
  ~60 instructions, `009ADE00` in full, and `009AC190` in full, and none of them writes it.
* **`entity->vtable[5Ch](n)`**, the type predicate used with `0x17` (self, in `009AC190`), `5`
  (the target, in `009FB200`) and `0xE` (in `009AF0A0`). Its meaning is not recovered.
* **`classDesc+188h`** (`ctx+8` = `[unit+538h]`), the scale in the turn-distance estimate, and
  `classDesc+18Ch`/`+394h`/`+398h` touched nearby.
* **`009AC0B0`'s tail.** It forms `target − self` on all three axes and feeds them to a magnitude
  at `009AC0FF`-`009AC10B`; I read to `009AC10B` and stopped, so "3-D range" is an inference from
  the three inputs, not from the arithmetic.
* **The bank command** at `009AC674` (`plan+2BCh`) and its mode `1` at `009AC67C`.
  `PILOT_BOT_PLAN_CONTROLS.md` records `+2D0h == 2` as "bank hold"; this state writes `1`, and what
  `1` selects was not traced.
* **`state+1Ch`, `state+20h`, `state+58h`** and the property names `"targetHdg"`,
  `"insideAttackRange"`, `"targetDist"`, `"attackDist"`, `"startDist"`, `"targetLock"`,
  `"currentAvoidFlag"` (`00D1FB08`-`00D1FB67`) were not matched to offsets. They are the
  registered property descriptors installed at `009ABC45`-`009ABDB6`; matching them would name
  these fields from the original source, and is the obvious next packet.
* **No runtime measurement.** Everything above is static.
