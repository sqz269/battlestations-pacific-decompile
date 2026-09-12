# unit+61h, unit+0FC4h / unit+0FDCh, and where the ship AI's order really lands

Addresses: 009F4D10 00811960 00811D10 00825F20 00825F2C 0081ED40 0081EFC8 0081F05D 0081F06D
0081F1EC 008266C1 008350BD 009F50E0 009F50FC 009ED6B0 00E08F70

Packet `cc_ship_ai_states`, worker `agent/cc-ship-ai-states`, 2026-09-11 UTC. Ghidra was
read-only for this packet. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; every live query verified both. Descriptive names are
hypotheses, not recovered symbols.

## Answer to the packet question

**There is no hop from the ship AI's control block to `unit+0FC4h` / `unit+0FDCh`, because
`unit+61h` makes the two mutually exclusive.** The follow-up packet `unit_autopilot_pair` in
`docs/CRUISE_COMMAND.md` asked for the writer of that pair on the assumption that the AI feeds
it. It does not:

| site | test | effect |
| --- | --- | --- |
| `009F50FC` | `CMP byte [unit+61h],0; JNE 009F524F` | the whole ship AI controller update returns without a single host call |
| `008266C1` | `CMP byte [unit+61h],0; JZ` past the block | only while the byte is **set** does `00825F20` overwrite every order-ring slot and `unit+980h`/`unit+984h` from `unit+0FC4h`/`unit+0FDCh` |
| `008350BD` | the same byte | the propeller update reads `unit+61h ? unit+0FC4h : unit+980h` |

So `unit+61h` selects between two whole control regimes. While it is clear the ship AI runs and
its order arrives by the path below; while it is set the AI is switched off and the pair is the
only input. The pair is an override, not the AI's output.

## Where the AI's order does land

`009F4D10` is step 14 of the controller's frame sequence (`009F5227`,
`docs/SHIP_AI_STATES.md`). `__thiscall(blk)(float)`, `RET 4`, body `009F4D10-009F4D90`.

```
009F4D10: MOVSS XMM0,[00D7A260]        ; -1.0f
009F4D1B: MOV EAX,[ESI+3FCh]           ; the unit
009F4D21: FLD [ESI+324h]               ; the heading target
009F4D27: MOVSS [ESI+33Ch],XMM0        ; blk+33Ch = -1.0f
009F4D2F: MOV ECX,[EAX+0B40h]          ; the slot index
009F4D35: IMUL ECX,ECX,0x54            ; 84 bytes per slot
009F4D38: SUB EAX,ECX
009F4D3B: ADD EAX,0xAEC                ; slot = unit + 0AECh - 84*index
009F4D48: CALL 00811960                ; slot+44h = the limited heading
009F4D55: FSTP [EDI+40h]               ; slot+40h = blk+32Ch
009F4D58: MOV [EDI+4Ch],AL             ; slot+4Ch = 1
009F4D62: FSTP [EDI+48h]               ; slot+48h = blk+330h
009F4D6B: MOV [EDI+4Ch],AL             ; written a second time
009F4D71/78/87: 009F0100(blk, dt), 009EF350(blk), 009EF910(blk, dt)   ; bodies unread
```

`[blk+3FCh]` is the unit and not the AI object. Three independent facts settle it, which is the
producer-before-layout check of `docs/WORKER_VERIFICATION_CHECKLIST.md`:

1. `0081F1EC MOV [ESI+0B3Ch],ESI` in `BSP_UnitVehicleBase_Construct` stores the unit's own
   pointer at `unit+0B3Ch`, which is slot 0's `+50h`. `00811960` is documented in
   `docs/UNIT_RUDDER_CURVE.md` as reading its object's `+50h` as a unit.
2. `0081EFC8 MOV [ESI+0B40h],EBX` in the same constructor initialises the index.
3. `00825F5E MOV [ESI+0B40h],EAX` in `BSP_UnitInstance_UpdateShipMotion` writes the index with
   `ESI = unit` (`00825F32 LEA ESI,[EDI-310h]` with `EDI` the routine's `ECX`).

### The slot

Two slots, 84 bytes apart: index 0 at `unit+0AECh` and index 1 at `unit+0A98h`.

| offset on the slot | with index 0 | meaning | writer |
| --- | --- | --- | --- |
| `+40h` | `unit+0B2Ch` | the first distance, from `blk+32Ch` | `009F4D55` |
| `+44h` | `unit+0B30h` | the heading target, limited to a quarter turn about the unit's own heading | `00811960` |
| `+48h` | `unit+0B34h` | the second distance, from `blk+330h` | `009F4D62` |
| `+4Ch` | `unit+0B38h` | the valid flag | `009F4D58`, `009F4D6B` |
| `+50h` | `unit+0B3Ch` | the unit | `0081F1EC` |

`brain+0B2Ch`, `brain+0B34h` and `brain+0B38h` are a **different** trio, the AI's goal vector
and its flag (`docs/SHIP_AI_STATES.md`). The offsets coincide; the objects do not.

### The promotion, 00825F2C..00825F7C

The head of `BSP_UnitInstance_UpdateShipMotion` consumes the slot before anything else:

```
00825F2C: MOV EAX,[EDI+830h]           ; unit+0B40h, the index
00825F32: LEA ESI,[EDI-310h]           ; the unit
00825F38: IMUL EAX,EAX,0x54
00825F3D: MOV ECX,ESI ; SUB ECX,EAX
00825F3F: CMP byte [ECX+0B38h],0       ; slot[index]+4Ch
00825F4C: JZ 00825F7C                  ; nothing new this frame
00825F4E: MOV byte [EAX],0             ; clear the flag
00825F51: MOV EAX,1 ; SUB EAX,[ESI+0B40h]   ; next = 1 - index
00825F5E: MOV [ESI+0B40h],EAX
00825F77: CALL 00811D10                ; slot[next] = slot[index]
```

`00811D10` (body `00811D10-00811D76`) is a sixteen-dword field-by-field copy, so the pair is
double-buffered: the AI always writes the slot the unit is currently on, and the motion update
flips to the other one and carries the values across so the order keeps standing.

## What is still open

**No reader of `slot+40h`, `+44h` or `+48h` was found.** The grep of `00825F20`'s decompiled
body for `0xb2c`, `0xb34`, `0xb38`, `0xaec`, `0xa98` and `0x54` finds only the promotion block
above, so whatever converts a heading target and two distances into the order ring's `+148h` /
`+14Ch` is elsewhere. That is the last hop of the chain and the top follow-up.

**No producer of `unit+0FC4h` / `unit+0FDCh` exists outside the constructor.** Every store
encoding was scanned over `.text` at the direct displacements `0FC4h` and `0FDCh` and at the
seven shifted unit bases of `docs/UNIT_INSTANCE_UPDATE.md` (`0FB4h`, `0FA0h`, `0E54h`, `0DE0h`,
`0CB4h`, `0C38h`, `0898h` and the matching `+18h` displacements for the second field):
`F3 0F 11` movss, `0F 11` movups, `66 0F D6` movq, `D9 /2` and `D9 /3` fst/fstp, `89 /r` mov
r32 and `C7 /0` mov imm32, each with a wildcard ModRM. The only hits are `0081F05D` and
`0081F06D` in `BSP_UnitVehicleBase_Construct` and `004EAF01` in the unrelated `004EA870`. This
reproduces and widens the negative result of `docs/CRUISE_COMMAND.md`, and with the `009F50FC`
gate it means the pair's producer is not on the AI path at all.

## Coverage

| Routine | Coverage |
| --- | --- |
| `009F4D10` | complete apart from `009F0100`, `009EF350` and `009EF910`, all `contract: unread` |
| `00811D10` | complete: a sixteen-dword copy, no branches |
| `00825F20` | partial: only `00825F2C-00825F7C`, the AI order promotion. `00825F7C-00826D6B` is not projected here; `008266C1-0082674C` is quoted from `docs/CRUISE_COMMAND.md` and re-read for the gate polarity |
| `0081ED40` | partial: the three initialisers at `0081EFC8`, `0081F05D`/`0081F06D` and `0081F1EC` |
| `00811960` | not re-read. Its reconstruction is `unit_set_heading_target_00811960` in `include/bsp/unit_rudder.hpp` (`docs/UNIT_RUDDER_CURVE.md`) and this packet calls it unchanged |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/CRUISE_COMMAND.md` follow-up `unit_autopilot_pair`: "the AI controller writes a desired throttle and steering into `[state]+8` ... find the base register the writer uses" | There is no such writer. `unit+61h` gates the ship AI off entirely, and the AI's order goes into the unit's 84-byte order slot at `unit+0AECh` / `unit+0A98h` instead | `009F50FC` against `008266C1`; `009F4D2F..009F4D6B` |
| `docs/UNIT_COMMANDED_SPEED.md` follow-up `unit_autopilot_pair`: "who writes `unit+0FC4h` and `unit+0FDCh` ... not touched here" | Still open as a producer question, but it is no longer on the AI path, so it does not block driving a ship from an AI command | the scan above and `009F50FC` |
| `docs/UNIT_COMMAND_PRODUCERS.md`: "`+980h` ordered thrust: none exists in the image ... open, negative result" | Unchanged for a direct displacement, and now explained for one of the two regimes: `00826708` and `0082674C` write `unit+980h`/`+984h` through the ring base under the `unit+61h` gate | `00825F20`'s decompiled body, the block at decompiler lines 318-346 |

## no_ghidra_function

none. `009F4D10`, `00811D10`, `00825F20`, `0081ED40` and `00811960` all have Ghidra functions.
The one routine this doc references that does not, `009F50E0`, has its boundary in
`docs/SHIP_AI_STATES.md`.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `unit_ai_order_slot_reader` | `unit+0A98h`, `unit+0AECh`, `+40h`/`+44h`/`+48h`, `00825F20` `00825F7C..00826D6B`, `00811890`, `0082ECB0` | The last hop: who turns the promoted slot into the order ring's `+148h`/`+14Ch`. Until it is read, no reconstruction can drive a ship from an AI command end to end, and `src/ship_motion_probe.cpp --moveto` has to use a stand-in. |
| `unit_manual_autopilot_pair` | `unit+61h`, `unit+0FC4h`, `unit+0FDCh`, `0081F05D`, `0081F06D`, `008350BD` | Who sets `unit+61h` and who fills the pair while it is set. The byte is written nowhere this packet's `mov byte [reg+61h]` scan could attribute; it is probably reached through a shifted base like the pair itself. |
| `ship_ai_navigation_arm` | `009ED6B0` `009EDA26..009EF228` | Named in `docs/SHIP_AI_STATES.md`; it is the producer of the `+324h`/`+32Ch`/`+330h` this doc publishes. |

## Uncertainties

1. `slot+40h` and `slot+48h` are called distances from how `009ED6B0` builds them, not from a
   consumer. With no reader found, their units are an inference.
2. The slot count is two because the index is written only as `1 - index`. Nothing read bounds
   it further, and `0081EFC8`'s initial value was not transcribed.
3. `009F4D10` writes `slot+4Ch` twice with the same value. Whether the duplicate is a compiler
   artefact or a second field aliasing the same byte was not established.
4. `unit+5Dh`, the controller's second gate at `009F50F2`, was not investigated.
