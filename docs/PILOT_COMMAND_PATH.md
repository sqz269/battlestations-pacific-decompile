# The two producers of the plane's pilot command block (packet `cc2_pilot_command_path`)

Addresses: `00519520`, `00519BB0`, `007BB9A0`, `007B8C90`, `007C2810`, `007C2880`, `007D0B80`,
`007D1360`, `007D5D20`, `0099ACD0`, `0099B450`, `0099BB40`, `0099BC00`, `0099BEE0`, `0099BF30`.

Worker `agent/cc2-pilot-command-path`, 2026-09-12 UTC. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was **read-only** for this packet: no rename, no comment, no
function creation, no prototype, no save. Every descriptive name below is a hypothesis, not a
recovered symbol. `docs/PILOT_CONTROLS.md` is the contract for the quantiser `007BB6E0` and the
command block `unit+9FCh`; `docs/PLANE_FLIGHT.md` for the latch `007B9770` and the rate law
`007DA710`; `docs/BOT_TASK_STATES.md` for the task states.

## Headline

**Both callers are found, and the axes have the game's own names.**

The bot's caller is `0099ACD0`, the pilot bot's think tick, and the player's is `00519BB0`, the
pilot HUD screen's per-frame update. Both were in undisassembled code, which is why
`docs/PILOT_CONTROLS.md` could not reach them: a `.text` scan of the disk image for `E8` branch
displacements landing on each target finds exactly one call site each, and no vtable entry.

```
PilotBot vtable[+0Ch] = 0099ACD0(bot, float dt)        ; no Ghidra function, 0099ACD0-0099B1A3
  -> 009998A0 BSP_PilotBot_Update(task, dt)            @0099AF1C
       -> 0099B450 BSP_PilotBot_SeedPlanSlots(task+4h) @009998D2   ; both halves <- unit+9E4h..
       -> 0099D300 BSP_PilotBot_PlanControls(task+4h)  @00999907, @009999AA ; desired + flags
  -> 0099BEE0(task+4h, cmd, 4.0f, dt)                  @0099B0A0   ; slew + clamp per axis
       -> 0099BC00(task+278h, cmd, 4.0f, dt)           @0099BF01
            -> 0099BB40(slot, 4.0f, dt) x5             @0099BC17, BC65, BCB7, BD0A, BD5A
  -> 0099BF30(task+4h, cmd)                            @0099B0AC   ; band repair and clamp
  -> 007B8C90 BSP_Plane_SetPilotCommandBlock(unit, cmd)@0099B0B9   ; -> unit+9FCh, +A14h = 1

HudPilotScreen vtable[+38h] = 00519BB0(screen, float dt) ; no Ghidra function, 00519BB0-00519D3D
  -> 00927F30 BSP_UnitInstance_IsLocalPlayerRole(unit, 1) @00519BF0 ; pilotplayer vs pilotai text
  -> 007BB9A0(unit)                                      @00519C9A ; the local-input gate
       false -> 00647300 BSP_InGameHudRoot_SetSpectatedUnit, 004CC460(24h, unit), return
       true  -> 005191B0(screen, dt)                     @00519CE8
                00519520 BSP_PlanePilotView_BuildPlayerCommand(screen, dt) @00519CF7
                  -> 007B8C90(screen+1Ch, cmd)           @00519818
                00519020(screen)                         @00519CFE
                00637620([00E198C4]+48h, 0.5f, 0.5f)     @00519D1B
                005484F0([00E198C4]+50h)                 @00519D29
```

Proof of the bot's link: `0099B0B1 MOV ECX,[ESI+50h]`, `0099B0B4 LEA EAX,[ESP+10h]`,
`0099B0B8 PUSH EAX`, `0099B0B9 CALL 007B8C90`. `ESI` is the routine's `this` throughout
(`0099ACD4 MOV ESI,ECX`, the only write to `ESI` in the whole listing), `ESI+50h` is the plane
unit, and `[ESP+10h]` is the same six-dword stack buffer `0099BEE0` filled at `0099B0A0`.

Proof of the player's link: `00519CF1 PUSH ECX`, `00519CF2 MOV ECX,ESI`, `00519CF4 FSTP [ESP]`,
`00519CF7 CALL 00519520`, with `ESI` the screen (`00519BC9 MOV ESI,ECX`).

## The axes: the game's own names

`007D5D20` reads the plane unit's control block back out of its Lua property bag, field by field,
through `00BD6830 BSP_LuaReader_ReadField(reader, key_kind, key_value, field_type, void* dest)`
(`RET 10h`, `docs/GUI_LUA_READER.md`). Each call pushes two eight-byte records - `{2, &field}`
then `{0, "name"}` - so the key string that follows a field pointer **is that field's name**:

| offset | recovered name | pointer site | key site | key string |
| --- | --- | --- | --- | --- |
| `unit+9E4h` | `yawInput` | `007D69BA` | `007D69D6` | `00D05D94` |
| `unit+9E8h` | `pitchInput` | `007D6A28` | `007D6A44` | `00D05D7C` |
| `unit+9ECh` | `rollInput` | `007D69F1` | `007D6A0D` | `00D05D88` |
| `unit+9F0h` | `pwrInput` | `007D6983` | `007D699F` | `00D05DA0` |
| `unit+9F4h` | `airBrakeInput` | `007D6A5F` | `007D6A7B` | `00D05D6C` |
| `unit+9F9h` | `turboInput` | `007D6ACD` | `007D6AE9` | `00D05D58` |
| `unit+9FAh` | `gunFire` | `007D6A90` | `007D6AB2` | `00D05D64` |

The group is opened as `control` (`00D05DAC`) by `00BD8E20 BSP_LuaReader_EnterKey` at `007D696F`.
`unit+9F8h` is **not** in the group.

The next group, `dynamics` (`00D05D4C`, opened at `007D6B02`), names the latched copies, and the
permutation matches the latch `007B9770` exactly, which is an independent confirmation of both
readings:

| offset | recovered name | key site | latched from |
| --- | --- | --- | --- |
| `unit+BB0h` | `yawF` | `007D6BB2` | `+9E4h` |
| `unit+BB4h` | `pitchF` | `007D6C57` | `+9E8h` |
| `unit+BB8h` | `rollF` | `007D6B7B` | `+9ECh` |
| `unit+BBCh` | `pwrF` | `007D6C20` | `+9F0h` |
| `unit+BC0h` | `airBrakeF` | `007D6BE9` | `+9F4h` |

Three further independent checks agree:

* `0099BC00` clamps the two axes that become `+9F0h` and `+9F4h` to `[0, 1]` and the three that
  become `+9E4h`, `+9E8h` and `+9ECh` to `[-1, 1]`. Power and an air brake are exactly the two
  unipolar controls an aeroplane has.
* the plane unit constructor seeds only `+9F0h`, with `1.0f` (`007CFEB0`) - full power.
* `007BB954` forces the command that becomes `+9F4h` to `1.0f` whenever `unit+900h` is not one of
  `{4, 5, 6, 7}`, the airborne set: brakes fully on when the plane is not flying.

So **`docs/PLANE_FLIGHT.md`'s roll/yaw labels on `+9E4h` and `+9ECh` are swapped**, and its
"fifth axis" is the air brake. See the corrections section.

## The bot's path, as a rule table

`0099ACD0` is `__thiscall(bot, float dt)`, `RET 4`, raw body `0099ACD0`-`0099B1A3`, **no Ghidra
function**. It is `PilotBot` vtable `+0Ch`: the table base is `00D1F348` (the vptr stored at
`0099A744` and `0099A8AD`), `00D1F354` holds `0099ACD0`, and the same table's `+4h` is
`009995A0`, inside `read_PilotBot_parameters_009973B0`. The prologue is
`SUB ESP,1Ch; PUSH ESI; MOV ESI,ECX`, with `PUSH EDI` at `0099AE3A` and `PUSH EBX` at
`0099AE7B`, so the frame is `28h` deep from `0099AE7B` on and `[ESP+2Ch]` is the `dt` argument.

`coverage: partial` - `0099ACD0`-`0099AE6B` is a task-list maintenance pass that was not read.
The command-production window `0099AE6C`-`0099B1A3` is complete:

| step | site | rule |
| --- | --- | --- |
| unit | `0099ACD6` | `unit = bot+50h`; a null unit exits at `0099B19F` |
| task | `0099AE6C`-`0099AE94` | `edi` = the first element of the task vector `bot+58h`/count `bot+5Ch`, then `0099A4C0(bot, edi_prev)`, then `ebx` = the first element again; a changed head runs the `0099AE96`-`0099AEBD` teardown |
| no task | `0099AEC2` | `ebx == 0` jumps to `0099B11C`: **no command this step** |
| force | `0099AECA`-`0099AEE3` | when `[ebx+274h]` is set and `[[ebx+274h] + [00F876B8]*8 + 9C2h]` is clear, `bot+74h = -1.0f` |
| timer | `0099AEF2`-`0099AF10` | `bot+74h -= dt`; a still-positive `bot+74h` jumps to `0099B11A`: **no command this step** |
| think | `0099AF1C` | `009998A0 BSP_PilotBot_Update(ebx, dt)` - the task state machine |
| suppress | `0099AF21`-`0099AF4A` | when `[unit + [00F876B8]*8 + 9C2h]` is set, `bot+74h = 3.0f` (`00CE3854`) and jump to `0099B11C`: **no command this step** |
| counter | `0099AF53`-`0099AFC2` | gated on `unit+C58h > 0`, `unit+72Ch`'s `vtable[+38h]`, byte `unit+5Ch` and `007B9140(unit, 0)`: a task whose `vtable[+24h]` is true decrements `unit+C58h` (`0099AFB6`), otherwise it is zeroed (`0099AFC2`). It does **not** gate the command |
| mode | `0099AFCC`-`0099B001` | `mode = 1`; each task's `vtable[+50h]` is called in order and the first result that is not `1` becomes `mode` and stops the walk |
| publish | `0099B00C` | `unit+DF8h = mode`; the scratch byte becomes `mode == 2` |
| peer | `0099B01A`-`0099B049` | when `mode == 1` and `[[unit+9D4h]+3D0h]` exists and differs from `unit+9D4h`, the scratch byte becomes `[that + [00F876B8]*8 + 9C1h]` |
| flag | `0099B055` | `007BB150(unit, scratch)`: `unit+DF0h = scratch ? unit+9C3h[[00F876B8]*8] : 0` |
| buffer | `0099B05A`-`0099B09A` | a six-dword stack buffer at `[ESP+10h]`, zeroed, then `[ESP+1Ch]` (index 3, the power command) set to `1.0f` from `00D7A24C` |
| evaluate | `0099B0A0` | `0099BEE0(ebx+4, buffer, 4.0f, dt)`; the rate `4.0f` is `00CE3D34` and `dt` comes from `[ESP+2Ch]` |
| repair | `0099B0AC` | `0099BF30(ebx+4, buffer)` |
| **commit** | `0099B0B9` | `007B8C90(unit, buffer)`: the six dwords land in `unit+9FCh`..`+A10h` and `unit+A14h` becomes `1` |
| tail | `0099B0BE`-`0099B113` | gated on `unit+72Ch`'s `vtable[+38h]` being false, the task's `vtable[+38h]` and `vtable[+30h]` being true, and `0042A7E0(unit)` or `unit+900h == 6`: `00999F50(bot, 0099CFF40(bot, 0))` |

`[00F876B8]` is the **previous** step's double-buffer index (`docs/IN_MISSION_SUBSYSTEM_TICK.md`:
`00E0B6CC` is the current one and its old value is saved to `00F876B8` at `00875C5F`/`00875C7A`),
so `unit+9C0h` is a two-entry eight-byte-per-step record. `docs/BOT_TASK_STATES.md` line 238 reads
`unit+9C3h[[00F876B8]*8]` the same way.

### The plan slots

`0099B450 BSP_PilotBot_SeedPlanSlots(this = task+4h)` reads the unit from `this+2F0h` and seeds
**both halves** of five `{current, desired, active}` triples from the live control block. The
seeding runs at the top of `009998A0 BSP_PilotBot_Update` (`009998D2`), before the task states
write the `desired` halves:

| slot | triple | seeded from | axis | `0099BC00` output | clamp |
| --- | --- | --- | --- | --- | --- |
| 0 | `+274h`/`+278h`/`+27Ch` | `unit+9F0h` (`0099B456`) | `pwrInput` | `cmd[3]` = `+A08h` (`0099BC5D`) | `[0, 1]` |
| 1 | `+280h`/`+284h`/`+288h` | `unit+9E4h` (`0099B476`) | `yawInput` | `cmd[0]` = `+9FCh` (`0099BCB0`) | `[-1, 1]` |
| 2 | `+28Ch`/`+290h`/`+294h` | `unit+9ECh` (`0099B494`) | `rollInput` | `cmd[2]` = `+A04h` (`0099BD02`) | `[-1, 1]` |
| 3 | `+298h`/`+29Ch`/`+2A0h` | `unit+9E8h` (`0099B4B2`) | `pitchInput` | `cmd[1]` = `+A00h` | `[-1, 1]` |
| 4 | `+2A4h`/`+2A8h`/`+2ACh` | `unit+9F4h` (`0099B4D0`) | `airBrakeInput` | `cmd[4]` = `+A0Ch` | `[0, 1]` |

The offsets are relative to `task+4h`; the array base is `+274h` and the stride is `0Ch`
(`0099BC56 LEA ECX,[EDI+0Ch]`, `0099BCA9 [EDI+18h]`, `0099BCFB [EDI+24h]`, `0099BD4E [EDI+30h]`).
`0099D300 BSP_PilotBot_PlanControls` writes the `desired` halves and their `active` bytes -
`+278h`/`+27Ch` at `0099D399`/`0099D3A1` and `0099DC8F`/`0099DC97`, `+284h`/`+288h` at
`0099D35A`/`0099D362`, `0099D401`/`0099D409`, `0099D63B`/`0099D643`, `0099EA3E`/`0099EA46`,
`+290h`/`+294h` at `0099D384`/`0099D38C`, `0099D416`/`0099D41E`, `0099D6B7`/`0099D6BF`,
`+29Ch`/`+2A0h` at `0099D36F`/`0099D377`, `0099D679`/`0099D681`, `0099E739`/`0099E741`, and
`+2A8h` at `0099D3A8` and `0099D8DD`.

`0099BB40` (Ghidra body `0099BB40`-`0099BBF8`, `__thiscall(slot, float rate, float dt)`) is the
slew limiter and is short enough to give in full: `step = rate * dt`; when
`|slot->desired - slot->current| < step` it returns `slot->desired`, otherwise
`slot->current ± step` toward it. It does **not** write back - the new value only reaches the
command buffer - so the `current` half is authoritative only because `0099B450` reseeds it from
the live block every step.

`0099BEE0` (Ghidra body `0099BEE0`-`0099BF25`, `__thiscall(bot, void* out, float, float)`,
`RET 0Ch`) calls `0099BC00` with `ECX = bot+274h` (`0099BEF7`) and then copies `bot+2DCh`,
`bot+2E5h` and `bot+2E4h` into `out+16h`, `out+15h` and `out+14h`.

**`out+15h` and `out+16h` are dead.** `007B8C90` copies the sixth dword whole, so they land at
`unit+A11h` and `unit+A12h`, and a `.text` scan of every `mod=10` modrm reference to those two
displacements finds none: only `unit+A10h` has readers (the quantiser's byte tests at `007BB7B9`
and `007BB7ED`). The bot's `turboInput` and `gunFire` requests therefore do not reach `unit+9F9h`
or `unit+9FAh` through this path.

`0099BF30` (Ghidra body `0099BF30`-`0099C202`, `__thiscall(bot, float* cmd)`) is a band repair,
`coverage: partial` - the three axis arms and the throttle arm were read, the tail past
`0099C130` was not. For `cmd[1]` and then `cmd[0]`, and for `cmd[2]` only when the unit's
`+72Ch` `vtable[+38h]` holds, it calls `0099B940(table, &value)`; when that returns zero (the
value lies inside one of the table's `[lo, hi]` bands) the value is replaced by `±1.1f`
(`00CE6448`, `00D06BB0`) or `±1.0f` chosen by the sign of `unit+C68h`, and then clamped to
`[-1, 1]`. When `bot+258h < 1.0f` and is not positive, `cmd[3] = 0.01f` (`00D7A238`) and
`cmd[4] = max(cmd[4], -bot+258h)`. `0099B940` is `contract: unread` beyond its band-search shape.

## The player's path

`00519BB0` is `__thiscall(screen, float dt)`, `RET 4`, raw body `00519BB0`-`00519D3D`, **no Ghidra
function**, SEH handler table `00C6AA60`. It is the pilot HUD screen's per-frame update: vtable
base `00CEC578` (the vptr stored at `005193FB`, and referenced again at `0068A596` and
`0068CFE0`), slot `+38h` at `00CEC5B0`. The same table's `+28h` is `00519830
BSP_HudBomberScreen_Register` (`docs/IN_GAME_INTERFACE_SCREEN_SETS.md`, screen id `25h`).

| step | site | rule |
| --- | --- | --- |
| gate | `00519BCB`-`00519BE5` | `g = [00E188A8]`; when byte `g+61Fh` or byte `g+620h` is set, jump to the `00519D2E` exit |
| role | `00519BF0` | `00927F30 BSP_UnitInstance_IsLocalPlayerRole(screen+1Ch, 1)` - role slot 1, `unit+1B0h` |
| text | `00519C02`-`00519C92` | the GUI field `role_Text` (`00CEC768`) on `screen+2Ch` is set to `ingame.pilotplayer` (`00CEC754`) when the role holds and `ingame.pilotai` (`00CEC744`) when it does not, through `00AA7E00` and `00ABBE50`, then `00419CC0`/`00BD1510` |
| pilot gate | `00519C9A` | `007BB9A0(screen+1Ch)` |
| hand off | `00519CA3`-`00519CDB` | on false: `00647300 BSP_InGameHudRoot_SetSpectatedUnit([00E198C4]+40h, unit+9D4h)` then `004CC460 BSP_FrontEndManager_PushInterfaceRequest([00E198C4], 24h, unit)`, and return |
| hud | `00519CE8` | `005191B0(screen, dt)` - `contract: partial`; it ignores its own `this` and updates `[00E198C4]+4Ch` through `0051F330`, `[00E198C4]+68h` through `00609BD0` when byte `+5h` holds, and `[00E198C4]+50h` through `005454B0` with `0.5f`, `0.5f` and `[+4Ch]+38h` |
| **command** | `00519CF7` | `00519520 BSP_PlanePilotView_BuildPlayerCommand(screen, dt)` |
| parts | `00519CFE` | `00519020(screen)` - `contract: partial`; it walks `screen+1Ch`'s subobject list at `unit+48h` and, for each whose `vtable[+5Ch](25h)` holds, calls `vtable[+21Ch](2Ah)` and `vtable[+21Ch](2Bh)` |
| tail | `00519D1B`, `00519D29` | `00637620([00E198C4]+48h, 0.5f, 0.5f)` with `0.5f` from `00CE3800`, then `005484F0([00E198C4]+50h)`; both `contract: unread` |

`00519520` takes the frame delta: the prologue is `SUB ESP,28h; PUSH ESI; MOV ESI,ECX` and the
epilogue is `RET 4` at `00519821`, and its last act is
`00519810 MOV ECX,[ESI+1Ch]; 00519813 LEA EDX,[ESP+14h]; 00519817 PUSH EDX; 00519818 CALL 007B8C90`.
`docs/PILOT_CONTROLS.md` gives its signature as `(view)`; it is `(view, float)`.

`007BB9A0` (Ghidra body `007BB9A0`-`007BBA0C`, `bool __thiscall(unit)`, `RET 0`) is the
local-input gate and is short enough to give in full. It returns true only when all six hold:

1. byte `unit+C0Ch != 0` (`007BB9A3`);
2. `unit+AA0h <= 0.0f` (`007BB9B4` `COMISS` against `00D7A218`, so an unordered compare also
   passes);
3. `[unit+DECh]+44h == 0`, or `[unit+DECh]+48h == 1.0f` (`007BB9D0` `UCOMISS`, `LAHF`,
   `TEST AH,44h`, `JP`: the parity of `{ZF, PF}` is odd only on equality);
4. `unit+9D4h == 0` or byte `[unit+9D4h]+3B0h == 0` (`007BB9E7`);
5. `00604A20()` is false (`007BB9F0`);
6. byte `unit+5Dh == 0` (`007BB9F9`).

`unit+C0Ch` is cleared by `007D5D20` at `007D5D52`, so the gate is closed until something sets it.

## The two large control-block writers

### `007D1360`, the control-state apply - plane vtable `+18Ch`

`__thiscall(unit, msg)`, `RET 4`, Ghidra body `007D1360`-`007D1B4B`. `SUB ESP,8Ch` plus three
pushes puts the single stack argument at `[ESP+9Ch]`, read into `EDI` at `007D13B0`. Gated on
`unit+900h` in `{4, 5, 6, 7}` (`007D1383`-`007D139B`); `BL` is set when
`[00E188A8]+1FE4h == 1` (`007D1374`), the host flag `docs/BOT_SCHEDULER_OUTPUT.md` names.

It writes **both** blocks from the same six dwords at `msg+38h`..`+4Ch`, in two passes that
differ only in a byte flag:

| pass | control block | command block | flag |
| --- | --- | --- | --- |
| `007D163E`-`007D16E9` | `+9E4h`..`+9F8h` at `007D166D`-`007D169A` | `+9FCh`..`+A10h` at `007D16A3`-`007D16D4` | `unit+9E1h = BL` at `007D1642` |
| `007D1735`-`007D17C1` | `+9E4h`..`+9F8h` at `007D1757`-`007D1784` | `+9FCh`..`+A10h` at `007D178D`-`007D17C1` | `unit+9E1h = 1` at `007D1724`, `unit+9E0h = 1` at `007D174D` |

**It never touches `unit+A14h`.** A `.text` scan of the function for every `9xxh`/`A0xh`/`A1xh`
displacement finds no `+A14h` reference, so the apply bypasses the quantiser entirely: it writes
the quantised axes directly and reseeds the command block with the same values so the next
quantise starts consistent. `unit+9E0h` and `unit+9E1h` are its own two bytes, also cleared at
`007D13D9` and `007D1AA8`.

### `007C2880`, the matching send - plane vtable `+188h`

Raw body `007C2880`-`007C29E8`, `RET 8`, **no Ghidra function**; SEH prologue at `007C2880`,
`MOV ESI,ECX` at `007C289E`, and it sits in the nine vtables that also hold `007D1360` one slot
later. It is the producer of the record `007D1360` applies:

* `007C28A0`-`007C28C0`: the same `unit+900h` in `{4, 5, 6, 7}` gate;
* `007C28C2`-`007C28E1`: a scratch byte set when `unit+1B0h` (the pilot role slot) equals
  `[[EDI]+20h]`, or `8` when `[EDI]` is null;
* `007C2904` and `007C298C`: `007C2810(EDI)` twice - the lazy `0x84`-byte record;
* `007C2970`: `007D8330(unit+AB0h, ...)` over a `3Ch`-byte stack struct;
* `007C299A`: `007BDD30(msg, unit, record, 0, scratch, unit+674h, unit+9E4h)` with
  `007C2979 LEA ECX,[ESI+9E4h]` as the last push, so the **control block is the message's
  source**: `007BDEAF`-`007BDED6` copies the six dwords at `[&unit+9E4h]+0h`..`+14h` into
  `msg+38h`..`+4Ch`, the same fields the apply reads back;
* `007C29B0`: `00779FC0(unit, msg, [EDI])` with the stack object's vtable set to `00CE4974`;
* `007C29CB`: `00779F80(unit, EDI)` on both paths.

`007BDD30` (Ghidra body `007BDD30`-`007BDF4D`) is the message constructor: it calls
`BSP_SessionMessage_ConstructBase(0C1h)` and pre-sets `msg+38h`..`+4Ch` to `(0, 0, 0, 1.0f, 0, 0)`
at `007BDD88`-`007BDD9B`, the control block's own default shape.

`007C2810` (Ghidra body `007C2810`-`007C2874`) is the record's lazy getter, not a command
producer: given a non-null owner it returns `owner+0Ch`, allocating `operator_new(0x84)` on first
use and initialising the first six dwords to `(0, 0, 0, 1.0f, 0, 0)` with `DAT_00D7A24C`, then
calling `007BA7B0`. Its four call sites are `007C2904`, `007C298C`, `007C2A62` in `007C29F0` and
`007C2BE7` in `007C2AF0`.

**So the `0x84`-byte record is not the pilot command block.** It is the session-message payload
for kind `0C1h`, cached on the owner at `+0Ch`; its first six dwords merely share the control
block's default values. `docs/PILOT_CONTROLS.md` marked that identification as a hypothesis; it
is wrong, and the bot's real `out` is a six-dword stack buffer in `0099ACD0`.

### `007D0B80`, a message handler - plane vtable `+208h`

`__thiscall(unit, msg)`, `RET 4`, Ghidra body `007D0B80`-`007D1353` (the `RET 4` is at
`007D1351`, with twelve `int3` from `007D1354`). It is not a control writer in any general sense:
it dispatches on the message kind through `msg->vtable[+0Ch](kind)` with `66h` at `007D0BEB`,
`63h` at `007D0C24` and `007D0C37`, `64h` at `007D0D1C`, `65h` at `007D0D49` and `67h` at
`007D0DA4` - the `IsKindOf` shape - and copies `msg+20h` into `unit+C10h` when
`[00E188A8]+1FE4h == 2` or byte `unit+5Dh` is clear.

**Its only control-block write is `unit+9E8h = 0.0f` at `007D1331`** - zero the pitch input - and
a grep of the whole listing for every `9Exh`/`9Fxh`/`A0xh`/`A1xh` displacement returns that one
line. The guard is byte `unit+C3Ah == 0` and byte `unit+5Dh == 0` (`007D12FC`, `007D1305`), then
`unit->vtable[+70h](1)` runs and `unit+C3Ah` is set, and finally `0.1745329f` (`00CE3990`, ten
degrees in radians) must be greater than `unit+C68h` (`007D1325`).

## `007D5D20`, the property-bag reader - plane vtable `+A0h`

`__thiscall(unit)`, `RET` with no immediate, raw body `007D5D20`-`007D771E`, **no Ghidra
function**. `007D5AC0`'s stored body really does end at its `RET` at `007D5D1F`; the routine that
takes `LEA reg,[unit+9E4h]` starts at `007D5D20` with `PUSH -1; MOV EAX,FS:[0]` and SEH handler
table `00C8D786`, and is a single function: the only `RET` in its 6655 bytes is at `007D771E`,
after `MOV FS:[0],ECX` at `007D7710` and `ADD ESP,750h`. Plane vtable base `00D05F20` plus `A0h`
is `00D05FC0`, which holds it, and it appears in the same nine vtables as `007C2880`.

`coverage: partial` - only the Lua block `007D67CC`-`007D6CFF` was read. What is established:

* `007D5D4C`-`007D5D52` clear `unit+C08h` and byte `unit+C0Ch`, the `007BB9A0` gate;
* `007D6725` looks up the property `SubType` (`00CFA9D0`) through `008F2260`;
* `007D677F`-`007D6791` require `unit+C0h` to exist with `[unit+C0h]+4h == 3`, else the whole
  block is skipped to `007D7383`;
* `007D6797` builds a `14h`-byte reader on the stack, and the `control` and `dynamics` groups are
  read as tabled above. Because `BSP_LuaReader_ReadField` writes its `dest`, this routine **is** a
  producer of `unit+9E4h`..`+9FAh`, conditional on the spawn record carrying a Lua table, and it
  bypasses both the command block and the quantiser;
* it also writes `unit+900h` at `007D63EA`, `007D65DD`, `007D6600` (the literal `7`) and
  `007D711A`. That field is packet `cc2_plane_ground_ops`' subject and is only flagged here.

## Coverage

| address | name | body | coverage |
| --- | --- | --- | --- |
| `0099ACD0` | `BSP_PilotBot_Tick` | `0099ACD0`-`0099B1A3` raw, `RET 4` | partial: `0099ACD0`-`0099AE6B` unread |
| `00519BB0` | `BSP_HudPilotScreen_Update` | `00519BB0`-`00519D3D` raw, `RET 4` | complete |
| `00519520` | `BSP_PlanePilotView_BuildPlayerCommand` | `00519520`-`00519823`, `RET 4` | signature only; the body is `docs/PILOT_CONTROLS.md`'s |
| `007BB9A0` | `BSP_Plane_IsLocalInputEnabled` | `007BB9A0`-`007BBA0C`, `RET 0` | complete |
| `007C2810` | `BSP_PlaneStateRecord_GetOrCreate` | `007C2810`-`007C2874` | complete |
| `007C2880` | `BSP_Plane_SendControlStateMessage` | `007C2880`-`007C29E8` raw, `RET 8` | complete |
| `007D0B80` | `BSP_Plane_HandleStateMessageKinds` | `007D0B80`-`007D1353`, `RET 4` | partial: the kind arms past `007D0DA8` unread; the control write is complete |
| `007D1360` | `BSP_Plane_ApplyControlStateMessage` | `007D1360`-`007D1B4B`, `RET 4` | partial: the two restore passes complete, `007D17C1`-`007D1B4B` unread |
| `007D5D20` | `BSP_Plane_ReadPropertyBag` | `007D5D20`-`007D771E` raw, `RET 0` | partial: `007D67CC`-`007D6CFF` only |
| `0099B450` | `BSP_PilotBot_SeedPlanSlots` | `0099B450`-`0099B588` | complete for the five slots |
| `0099BB40` | `BSP_PilotBot_SlewSlot` | `0099BB40`-`0099BBF8` | complete |
| `0099BC00` | `BSP_PilotBot_EvaluatePlanSlotAxes` | `0099BC00`-`0099BDA0` | complete |
| `0099BEE0` | `BSP_PilotBot_EvaluatePlanSlots` | `0099BEE0`-`0099BF25`, `RET 0Ch` | complete |
| `0099BF30` | `BSP_PilotBot_RepairCommandBands` | `0099BF30`-`0099C202` | partial: past `0099C130` unread |

## `no_ghidra_function`

| address | name | end_address | evidence |
| --- | --- | --- | --- |
| `0099ACD0` | `BSP_PilotBot_Tick` | `0099B1A3` | the preceding routine `0099ABD0` (itself after ten `int3` from `0099ABC6`) ends `RET 4` at `0099ACCC`, with one `int3` at `0099ACCF`; `0099ACD0` opens `SUB ESP,1Ch; PUSH ESI; MOV ESI,ECX`; the only `RET` after it other than the same-shaped early-out at `0099AD65` is `0099B1A3`, followed by `int3` to `0099B1AF` |
| `00519BB0` | `BSP_HudPilotScreen_Update` | `00519D3D` | three `int3` from `00519BAD` after `00519B00`'s stored body ends at `00519BAC`; SEH prologue at `00519BB0`; `RET 4` at `00519D3D`, and `00519D40` begins the next routine (`RET 4` at `00519D4D`) |
| `007C2880` | `BSP_Plane_SendControlStateMessage` | `007C29E8` | eleven `int3` from `007C2875` after `007C2810`'s body; SEH prologue at `007C2880`; `RET 8` at `007C29E8` with `int3` to `007C29EF` |
| `007D5D20` | `BSP_Plane_ReadPropertyBag` | `007D771E` | `007D5AC0`'s body ends `RET` at `007D5D1F`; SEH prologue at `007D5D20`; the only `RET` in `007D5D20`-`007D771F` is at `007D771E`, with `int3` at `007D771F` and the next defined function at `007D7720` |

## `flow_gaps`

* `007D5AC0`: `docs/PILOT_CONTROLS.md` recorded `007D6983` and `007D69BA` as belonging to it
  because the index attributes them there. They do not; they are inside `007D5D20`. No gap
  remains once that boundary is taken from the disk bytes.
* No `_free`-class call in any routine read here produced a spurious return.

## Corrections to earlier documents

See `reports/pilot_command_path.json`'s `corrections` block for the machine-readable form.

1. **`docs/PLANE_FLIGHT.md`** - was: `unit+9E4h` roll, `+9ECh` yaw, `+9F4h` "fifth axis".
   Is: `+9E4h` is `yawInput`, `+9ECh` is `rollInput`, `+9F4h` is `airBrakeInput`.
   Evidence: the Lua key that follows each field pointer in `007D5D20`, plus the matching
   `dynamics` names on the latched copies, plus `0099BC00`'s unipolar clamp falling on exactly
   `+9F0h` and `+9F4h`.
2. **`docs/PLANE_FLIGHT.md`** - was: the rate law's yaw term is "`YawSpd` times the latched
   roll `unit+0BB0h`". Is: `+BB0h` is `yawF`, the latched `yawInput`, so the term is `YawSpd`
   times the latched **yaw**. The two readings that "disagree" in `docs/PILOT_CONTROLS.md`'s open
   questions do not: `+9ECh` carrying the horizontal aim error is a bank-to-turn roll command.
3. **`docs/PILOT_CONTROLS.md`** - was: `0099BEE0`'s `out` "is that record", the `0x84`-byte block
   `007C2810` allocates, marked a hypothesis. Is: `out` is a six-dword stack buffer in `0099ACD0`
   at `[ESP+10h]`, handed straight to `007B8C90`. The `0x84` record is the `0C1h` session-message
   payload cached at `owner+0Ch`; the two only share a default value.
4. **`docs/PILOT_CONTROLS.md`** - was: `00519520 BSP_PlanePilotView_BuildPlayerCommand(view)`.
   Is: `__thiscall(view, float dt)`, `RET 4`.
5. **`docs/PILOT_CONTROLS.md`** - was: the control-block writer list names `007D5AC0`.
   Is: `007D5D20`, and it does not store to the block itself - it passes the field pointers to
   `BSP_LuaReader_ReadField`, which writes them.
6. **`docs/PILOT_CONTROLS.md`** - was: the call sites of `00519520` and `0099BEE0` are open
   questions. Is: `00519CF7` in `00519BB0` and `0099B0A0` in `0099ACD0`, both in undisassembled
   code, each the single `E8` displacement in the image that lands on the target.

## Open questions

* The caller of `0099ACD0` and of `00519BB0`. Both are virtual-only; `0099ACD0` is `PilotBot`
  vtable `+0Ch` and `00519BB0` is `00CEC578+38h`, and no `MOV reg,[reg+0Ch]`-style scan was run
  for either.
* What sets `unit+C0Ch`, the `007BB9A0` gate that `007D5D20` clears.
* `unit+C68h`: an angle in radians, compared against ten degrees in `007D0B80` and used for the
  sign of the band repair in `0099BF30`. Its producer is outside this packet.
* Who writes `unit+9F9h` `turboInput` and `unit+9FAh` `gunFire`, given `0099BEE0`'s `out+15h`
  and `out+16h` are dead and the command block carries only `+A10h` into `+9F8h`.
* `unit+9C0h`, the eight-byte double-buffered per-step record whose bytes `+9C1h`, `+9C2h` and
  `+9C3h` gate the bot tick. `docs/MOTION_DIFFERENTIAL.md` reads `unit+9C0h` as a ship's maximum
  speed and `+9C8h` as a radius; the two layouts cannot both hold at the same base.
* `0099B940`, `0099CFF40`, `00999F50`, `0099A4C0`, `007B9140`, `0042A7E0`, `007D7A80`, `007D8330`,
  `00779FC0`, `00779F80`, `00637620`, `005484F0`, `007BA7B0`: `contract: unread`.
* `0099BF30` past `0099C130`, `007D1360` past `007D17C1`, `007D0B80`'s kind arms and
  `007D5D20` outside its Lua block.

## Correction from docs/GAMEPLAY_LOOSE_ENDS_2.md (packet cc2_gameplay_loose_ends_2)

- **Was:** line 389: docs/MOTION_DIFFERENTIAL.md reads unit+9C0h as a ship's maximum speed and +9C8h as a radius; the two layouts cannot both hold at the same base
  **Is:** they hold at two different bases: the stride-8 byte record is the plane family's layout and the MaxSpeed float is the class-06 ship family's
  **Evidence:** 00822C20 BSP_UnitInstance_SEntityInit writes the float at 00822C65 and appears in exactly five vtables (00CF9150, 00CFA818, 00CFB7D8, 00CFC470, 00D09718), each a class-06 family vtable + A0h; no plane vtable carries it, while 007CFE4C MOV byte [ESI+9C0h],BL in BSP_PlaneUnitInstance_Construct and the five stride-8 stores in FUN_007CDC70 treat the offset as bytes
