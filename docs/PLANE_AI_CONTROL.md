# What writes an AI-flown plane's control inputs (packet `cc7_plane_ai_control`)

Addresses: `007B8C90`, `007BB6E0`, `007BB920`, `007CFD20`, `007D1360`, `00519520`, `00519BB0`,
`0099ACD0`, `0099B450`, `0099BB40`, `0099BC00`, `0099BEE0`, `0099BF30`, `0099D300`, `0042E740`.

Worker `agent/cc7-plane-ai-control`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was **read-only** for this packet: no rename, no comment, no
prototype, no tag, no write lock, no save. Every descriptive name below is a hypothesis, not a
recovered symbol.

## Headline: the result is positive — a plane AI exists

**`0099ACD0 BSP_PilotBot_Tick` is the AI.** It is one of exactly two producers of a plane unit's
pilot command block, the other being the local player's HUD view. `docs/PLANE_UNIT_TICK.md`'s
note that no plane AI is named anywhere in the ledger is **stale**: `docs/PILOT_COMMAND_PATH.md`
(worker `agent/cc2-pilot-command-path`, 2026-09-12) had already found and named both producers.
This packet's contribution is the *complete writer census* that proves the set is closed, a
correction to the census method, and the recovery of one steering arm.

The practical caveat is in [What this does not give you](#what-this-does-not-give-you): the
plumbing from "desired axis values" to "the flight law moves the plane" is fully recovered, but
the *production* of the desired values — the per-task steering laws inside `0099D300` — is only
partly recovered, so this is **not yet enough to fly an AI plane**.

## The two blocks and their fields

Two consecutive blocks on the plane unit. Offsets and the game's own field names are from
`docs/PILOT_COMMAND_PATH.md`, which recovered the names from the Lua property-bag reader
`007D5D20` (each `00BD6830` call pushes `{2, &field}` then `{0, "name"}`, so the key string
following a field pointer is that field's name). I did not re-derive the names; I did re-derive
every writer.

### The command block, `unit+9FCh`..`unit+A14h` — what a producer writes

Six dwords plus a pending byte. `007B8C90` is the setter, verified instruction by instruction:

```
007b8c90: MOV EAX,[ESP+4]          ; the six-dword source
007b8c96: MOV [ECX+9FCh],EDX       ; cmd[0]
007b8c9f: MOV [ECX+A00h],EDX       ; cmd[1]
007b8ca8: MOV [ECX+A04h],EDX       ; cmd[2]
007b8cb1: MOV [ECX+A08h],EDX       ; cmd[3]
007b8cba: MOV [ECX+A0Ch],EDX       ; cmd[4]
007b8cc3: MOV [ECX+A10h],EAX       ; cmd[5], three request bytes + one untouched
007b8cc9: MOV byte [ECX+A14h],1    ; pending
007b8cd0: RET 4
```

`void __thiscall(unit, const void* cmd)`, `RET 4`.

| dword | offset | axis | range |
| --- | --- | --- | --- |
| `cmd[0]` | `+9FCh` | yaw | `[-1, 1]` |
| `cmd[1]` | `+A00h` | pitch | `[-1, 1]` |
| `cmd[2]` | `+A04h` | roll | `[-1, 1]` |
| `cmd[3]` | `+A08h` | power | `[0, 1]` |
| `cmd[4]` | `+A0Ch` | air brake | `[0, 1]` |
| `cmd[5]` | `+A10h` | three request bytes at `+14h`/`+15h`/`+16h` of the buffer | — |
| — | `+A14h` | pending flag, set to 1 | — |

### The control block, `unit+9E4h`..`unit+9FAh` — what the quantiser produces

`007BB6E0 BSP_Plane_QuantizeControlAxes` consumes the command block and writes these; the flight
law reads them. Names from `007D5D20`'s `control` group (`00D05DAC`):

| offset | name | from |
| --- | --- | --- |
| `+9E4h` | `yawInput` | `cmd[0]` |
| `+9E8h` | `pitchInput` | `cmd[1]` |
| `+9ECh` | `rollInput` | `cmd[2]` |
| `+9F0h` | `pwrInput` | `cmd[3]` |
| `+9F4h` | `airBrakeInput` | `cmd[4]` |
| `+9F9h` | `turboInput` | byte |
| `+9FAh` | `gunFire` | byte |

`+9F8h` is not in the `control` group. **`include/bsp/plane_flight.hpp` labels `+9E4h` as roll
and `+9ECh` as yaw; those two are swapped**, and its `kLiveAux` is the air brake. That correction
is `docs/PILOT_COMMAND_PATH.md`'s, and I did not change the header — it is leased elsewhere and
its numeric offsets are right, only the two labels are wrong.

## The writer census

Method, and why it matters: the brief warns that a census which guesses one access encoding will
see one writer where there are three. So rather than grep for an instruction form, I scanned
`.text` for the **disp32 byte pattern of each offset** — which is encoding-independent, since any
`mod=10` access to `[reg+0x9FC]` contains the bytes `FC 09 00 00` whatever the opcode — and then
validated each hit by linear-sweeping from every known function start and keeping the instruction
that actually contains that displacement. Script kept out of the repo (scratchpad only).

**The method's own trap, which I walked into.** Capstone reports an x87 memory store
(`fstp dword ptr [esi+0x9E4]`) as a *read* of that operand, so the first pass classified all nine
of `007BB6E0`'s control-block stores as reads and would have reported the quantiser as a
non-writer. Counting `fst`/`fstp`/`fist`/`fistp`/`fbstp` as writes fixed it. Re-running the whole
census with the fix added thirteen writes and **no new command-block writer**, which is the
result that matters.

Residual blind spots, stated rather than hidden: the sweep anchors on Ghidra function starts, so
a body with no Ghidra function is swept from the previous start and can desync. I checked every
disp32 site the sweep did not consume; **none falls in the plane range (`007Bxxxx`-`007Dxxxx`) or
the bot range (`0099xxxx`-`009Axxxx`)** — they are coincidental byte sequences in unrelated code
and in the CRT. A writer reaching the block through a *pre-offset pointer* would also escape the
scan, so I enumerated every site that takes the block's address: `&unit+9FCh` is taken exactly
twice, at `006339C4` (an `esp`-relative stack frame, unrelated) and at `007BB982` inside
`BSP_Plane_CommitPilotCommand` — the consumer, not a producer.

### Writers of the command block `+9FCh`..`+A14h`

Complete. Every entry below is every function in the image containing an instruction that writes
any of those seven offsets.

| address | name | fields | what it is |
| --- | --- | --- | --- |
| `007B8C90` | `BSP_Plane_SetPilotCommandBlock` | all seven | **the setter** — see its two callers below |
| `007BB920` | `BSP_Plane_CommitPilotCommand` | `+A08h`, `+A0Ch`, `+A14h` | the quantiser's own tail; consumer, clears pending |
| `007CFD20` | `BSP_PlaneUnitInstance_Construct` | all seven | construction-time init |
| `007D1360` | `BSP_Plane_ApplyControlStateMessage` | `+9FCh`..`+A10h` (not `+A14h`) | **the network / replay apply**, plane vtable `+18Ch` |

Not the plane's struct — a different class with a field at a coincident offset, each confirmed by
what the enclosing function does: `004ECA30` and `004EB9B0` (scene header properties, callers
`004F1D70 BSP_SceneRecord_ApplyHeaderProperties`, string refs `Fog`/`Sun`/`CloudLights`),
`0062C2C0` (`esp`-relative), `00818EA0`, `0081F980`, `00834A70 BSP_UnitInstance_UpdatePropellerSpray`,
`00834CC0 BSP_UnitInstance_UpdateSternWave`. `0081ED40 BSP_UnitVehicleBase_Construct` writes
`+9FCh`/`+A10h`/`+A14h`; whether that is the plane's own base-class constructor or a sibling
layout I did not establish — either way it is construction, not steering.

### The setter's callers — exactly two

`python tools/bsp.py ghidra callers 007b8c90` returns two, and `docs/PILOT_COMMAND_PATH.md`
independently confirms the count by a `.text` scan for `E8` displacements landing on `007B8C90`
(one call site each, no vtable entry):

* `00519520 BSP_PlanePilotView_BuildPlayerCommand` at `00519818` — **the local player**, reached
  from the pilot HUD screen's per-frame update `00519BB0` (vtable `00CEC578` slot `+38h`).
* `0099ACD0 BSP_PilotBot_Tick` at `0099B0B9` — **the AI**, `PilotBot` vtable `00D1F348` slot
  `+0Ch`. `0099B0B1 MOV ECX,[ESI+50h]` is the plane unit and `LEA EAX,[ESP+10h]` the six-dword
  buffer.

### Subtraction

Player (`00519520`) and network/replay (`007D1360`) and construction (`007CFD20`) and the
quantiser's own commit (`007BB920`) are all accounted for. **What remains is `0099ACD0`, and it
is the AI.** There is no third steering producer, no Lua path that writes these fields directly,
and no other vtable slot reaching the setter.

Writers of the *control* block `+9E4h`..`+9F4h` are consistent with that and add no steering
producer: `007BB6E0` (the quantiser, the only law-carrying writer), plus resets and limits
(`007B8C30`, `007C11E0`, `007C1430`, `007C5F60`, `007CA3F0`, `007CAF10`, `007CC7A0`, `007DA380`),
construction `007CFD20`, and the two network paths `007D0B80` / `007D1360`.

## The AI's control-writing step

`0099ACD0` is `void __thiscall(bot, float dt)`, `RET 4`, raw body `0099ACD0`-`0099B1A3`, **no
Ghidra function**. The plane unit is `bot+50h`. `docs/PILOT_COMMAND_PATH.md` has the full step
table; the command-production window is `0099AE6C`-`0099B1A3`, and it produces no command at all
on four paths (no task, a still-positive `bot+74h` timer, the `unit+9C2h` suppression, or a null
unit).

The chain, every link of which I verified from the listing:

```
0099D300 BSP_PilotBot_PlanControls(task+4h, dt)   ; writes the five slots' `desired` halves
0099B450 BSP_PilotBot_SeedPlanSlots(task+4h)      ; reseeds every `current` half from the live block
0099BEE0(task+4h, buf, 4.0f, dt)                  ; -> 0099BC00 -> 0099BB40 x5, then 3 request bytes
0099BF30(task+4h, buf)                            ; band repair
007B8C90(unit, buf)                               ; commit; unit+A14h = 1
007BB6E0 BSP_Plane_QuantizeControlAxes            ; command block -> control block
007B9770 BSP_Plane_LatchControlInput              ; control block -> unit+BB0h.. for the rate law
```

### The plan slots

Five `{current, desired, active}` triples, base `task+274h`, stride `0Ch`. The slot order is
**not** the command order; the permutation is real and I confirmed all five legs from the listing
of `0099BC00` (the `LEA ECX,[EDI+n]` that selects the slot, and the `MOVSS [ESI+m]` that stores
the result):

| slot | base | `LEA ECX` | seeded from | axis | store | cmd | clamp |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | `+274h` | `EDI+0` | `unit+9F0h` | power | `0099BC5D [ESI+0Ch]` | `cmd[3]` | `[0, 1]` |
| 1 | `+280h` | `0099BC56 EDI+0Ch` | `unit+9E4h` | yaw | `0099BCB0 [ESI+0]` | `cmd[0]` | `[-1, 1]` |
| 2 | `+28Ch` | `0099BCA9 EDI+18h` | `unit+9ECh` | roll | `0099BD02 [ESI+8]` | `cmd[2]` | `[-1, 1]` |
| 3 | `+298h` | `0099BCFB EDI+24h` | `unit+9E8h` | pitch | `0099BD55 [ESI+4]` | `cmd[1]` | `[-1, 1]` |
| 4 | `+2A4h` | `0099BD4E EDI+30h` | `unit+9F4h` | air brake | `0099BD76 [ESI+10h]` | `cmd[4]` | `[0, 1]` |

The clamp constants are `00D7A24C = 1.0f` and `00D7A260 = -1.0f`, read from the image. NaN is not
filtered by either clamp: both comparisons are `COMISS`/`FCOMIP` whose unordered result takes the
branch that passes the value through.

### The slew limiter `0099BB40`

`float __thiscall(slot, float rate, float dt)`, `RET 8`, body `0099BB40`-`0099BBF8`. Read from
the listing, x87 stack tracked by hand because the whole body is x87:

```
delta = slot->desired - slot->current          ; 0099BB5A FSUBP ST2,ST0
step  = rate * dt                              ; 0099BB65 FMUL
if (step > |delta|)   return slot->desired;    ; 0099BB89 FCOMI, JBE at 0099BB8D
sign = (delta < 0) ? -1 : (delta > 0 ? +1 : 0) ; 0099BBAB / 0099BBCE COMISS, FILD
return slot->current + sign * step;            ; 0099BBBE / 0099BBE9 FADDP
```

Two details the prior write-up did not state and that a reimplementation gets wrong by default:
the `delta == 0` case loads the integer `0` at `0099BBDB` and returns `current` **exactly**, and a
NaN `delta` takes `JBE` at `0099BBAE` and then fails `JA` at `0099BBD9`, so it also yields sign
`0` and returns `current` — NaN never propagates a step. It does **not** write back; the new
value only reaches the command buffer, so `current` is authoritative only because `0099B450`
reseeds it from the live block every step.

The rate is `4.0f` (`00CE3D34`) for all five axes, pushed by `0099B0A0`.

### The three request bytes, `0099BEE0`

After `0099BC00` returns, `0099BEE0` writes three bytes into the sixth dword:

```
0099BF06: buf+16h <- bot+2DCh
0099BF0F: buf+15h <- bot+2E5h
0099BF18: buf+14h <- bot+2E4h
```

Only `buf+14h` survives: it lands at `unit+A10h`, whose byte tests at `007BB7B9` and `007BB7ED`
are its only readers. `buf+15h` and `buf+16h` land at `unit+A11h`/`+A12h`, and
`docs/PILOT_COMMAND_PATH.md`'s `.text` scan for those two displacements finds no reader at all.
**The bot's `turboInput` and `gunFire` requests do not reach `unit+9F9h`/`+9FAh` by this path.**
Where they do reach it, if anywhere, is unestablished.

## One recovered steering arm: the banked-turn yaw blend

`0099D300 BSP_PilotBot_PlanControls` is `void __thiscall(bot, float dt)`, `RET 4`, body
`0099D300`-`0099EBAB` (6.3 KB, many per-task-state arms). `docs/UNIT_COMMAND_PRODUCERS.md` marks
it "only the control-override clamp and the slot protocol reconstructed". I recovered one further
arm, the late yaw write at `0099EA3E`, because it is the arm that would actually turn an aircraft.

Shared setup, `0099D46E`-`0099D506`:

```
EBX   = 0042E740 BSP_GameTuning_GetSingleton() + 538h     ; a plane tuning block
ECX   = bot+2F0h                                          ; the plane unit
[ESP+28h] = 1.0f / max(unit+340h * <double 00CE65D0>, 1.0f)
[ESP+10h] = unit+C64h
[ESP+34h] = unit+C68h ;  [ESP+1Ch] = |unit+C68h|
[ESP+20h] = sin(unit+C68h)   ; 0099D4FA FSIN
[ESP+30h] = cos(unit+C68h)   ; 0099D504 FCOS
```

`unit+C68h` is taken as an **angle in radians** — `FSIN`/`FCOS` of it, its magnitude interpolated
against tuning thresholds, and its sign selecting `±1.1f` in the repair `0099BF30`. That it is
specifically the **bank angle** is an inference from those three uses plus the algebra below, not
a recovered fact. `unit+C64h` is unidentified beyond being gated `> 0`.

The arm, `0099E881`-`0099EA4D`:

```
turn = 0
if (unit+C64h > 0) {                                  ; 0099E88E COMISS, JBE skips the term
    turn = clamp( unit+C64h
                  / ( [bot+2F4h]+1B0h * sin(unit+C68h) * EBX[+9Ch] ),
                  -1.0f, +1.0f )                      ; 0099E92D FDIVP, 0099E939 -> 00415690
}
t    = min(1.0f, 00419010 BSP_Math_InterpolateClamped(EBX[+7Ch], 0.0f, EBX[+80h], 3.0f,
                                                      |unit+C68h|))   ; 0099E8C8, 0099E8E7
yaw  = clamp( t * turn + (1 - t) * base, -1.0f, +1.0f )   ; 0099E94E FSUBRP, 0099E98D -> 00415620
slot1.desired = yaw ; slot1.active = 1                    ; 0099EA3E / 0099EA46
```

So the yaw command is a **blend, weighted by how hard the aircraft is banked, between a base yaw
demand and a bank-derived turn term** — a turn-coordination rudder law. `base` is `[ESP+18h]`,
set at `0099E884` from code I did not read. `EBX[+7Ch]`, `EBX[+80h]` and `EBX[+9Ch]` are authored
tuning constants at `tuning+5B4h`, `+5B8h` and `+5D4h`; I did not read their values.
`[bot+2F4h]` is a pointer distinct from the unit at `bot+2F0h` and is **unidentified** — that
alone stops this arm from being implementable.

`coverage: partial`. The pitch arm at `0099E739`, the roll arm at `0099D6B7`, the power arm at
`0099DC8F` and the air-brake arm at `0099D8DD` were **not read**.

## What this does not give you

The packet asked whether an AI exists and what its control-writing step is. Both are answered.
But for the gameplay goal behind the packet — planes that close on a fleet instead of flying
straight — this is **not sufficient**, and the gap should not be papered over:

* The commit pipeline (slot → slew → clamp → permute → commit) is recovered and implemented.
* The *desired* values that go into it come from `0099D300`'s per-task arms, of which one of five
  axes is recovered and its inputs are not fully identified.
* `0099ACD0` only produces a command when it has a task, and the tasks come from the bot task
  system (`docs/BOT_TASKS.md`, `docs/BOT_TASK_STATES.md`). Nothing here creates a task.

I did **not** invent a steering law to close that gap. A fabricated one would make the gunnery
numbers look validated when they are not.

## Status of every claim here

* **Exported / read from the listing**: `007B8C90`, `0099BB40`, `0099BC00` (all five legs),
  `0099BEE0`, the `0099D300` setup block and yaw arm, the float constants.
* **Reconstructed** (`src/plane_ai_control.cpp`): the slew limiter, the per-axis clamp and
  permutation, the request bytes, the commit.
* **Build-tested**: partially. `src/plane_ai_control.cpp` compiles clean with MSVC Win32,
  `cl /c /EHsc /W4 /std:c++17 /Iinclude`, no warnings. It is **not registered in
  `CMakeLists.txt`** — that file is leased by the integrator — so it has not been built inside
  the project's CMake build and `scripts/build.ps1` does not cover it. **The integrator must add
  `src/plane_ai_control.cpp` to `CMakeLists.txt`.**
* **Fixture-tested / game-validated**: no.
* **Carried from prior work, not re-derived**: the field *names* from `007D5D20`, the
  `0099ACD0` step table, the `007D1360` two-pass structure, the `0099BF30` repair.
* **Inferred, not proved**: that `unit+C68h` is a bank angle; that `0081ED40`'s writes are the
  plane's base-class construction.
