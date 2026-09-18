# The AI heading to ordered rudder chain (packet `cc8_ship_ai_heading_to_rudder`)

Addresses: 0080E170, 0080E190, 00813020, 009F3F80 (`009F4034`), 009F4CDE..009F4D04,
00937572, 0092E8C0, 00811960; read as evidence 009F50E0, 009D8CE0, 009F3E30, 00811D80,
0081F980, 00816A40, 0080DAD0, 0080DA00, 00812F66, 0042AC60.

Worker `agent/cc8-ship-rudder-hop`, 2026-09-18. Ghidra read-only: no renames, comments,
prototypes or saves.

## The headline: the chain was answered on 2026-09-12 and four documents still deny it

Three documents, one of them written by this worker earlier today, state that "on the
image's evidence the AI never writes the order ring". **That is wrong, and it was already
known to be wrong.** `docs/SHIP_AI_THROTTLE_TO_RING.md`, packet `cc_ai_throttle_ring`,
answered the whole question six days ago: the AI writes the ring's write slot directly with
`0080E170` and `0080E190` at the tail of `009F3F80`, and `00816A40` is not on that path.
Both setters are named in the ledger, reconstructed in `src/ship_ai_throttle_ring.cpp` and
bound as `ShipAiRing::set_write_slot_*`, which is why USN02 reports `ring hops=96000
writes=96000` and `total_path=41584.83` over 150 s.

The later negative rests on a rel32 scan of `.text` for calls reaching `00816A40` or
`0080DAD0`. The scan is correct; the AI simply uses neither entry point, and nothing
reconciled the two readings.

So this packet is a reconciliation, and what it adds on top of the prior document is:

1. a byte-level writer census of `unit+984h` over every store form and every way of
   reaching the address, each negative carrying a positive control;
2. the cursor discipline at `00813186..008131D3`, which is the one link the prior document
   left open in its uncertainty 5, "whether the write cursor can be stale relative to the
   read cursor on a networked client";
3. the correction of four documents and one run-time log note.

## The chain, every hop with its address and ABI

| # | address | what | ABI |
| --- | --- | --- | --- |
| 1 | `009F4072` | `heading = unit->vtable[50h]()`, the hull's own yaw; `ECX = [blk+3FCh]` | `__thiscall float(void)`, `RET 0`, concrete `006DFD60` (`docs/SHIP_AI_RUDDER_HOP.md`) |
| 2 | `009F409E` | a ship latched astern steers about the reciprocal: `00438AA0(heading, [00D7A264])` | two stack floats, `RET 8` |
| 3 | `009F40BB` | `error = 00438B10(blk+324h, heading)` - the heading target minus the hull heading, wrapped | two stack floats, `RET 8` |
| 4 | `009F44F7` | `009DA250 BSP_ShipAi_RudderFromHeadingError(error)` - the rudder law, into `blk+1D4h` | reconstructed, 93000 calls on USN02 |
| 5 | `009F4BA7..009F4C12` | the deadbands: with `abs(blk+1D0h) < 0.05` and `abs(0092D730()) < 1.0f`, `blk+1D4h` is zeroed; the slew step is `dt * 1.5` | `00D7A270`, `00D7A24C`, `00CE3D78` |
| 6 | `009F4CE8` | **`0080E190(unit, rudder)`** | `__thiscall void(float)`, `RET 4`, `ECX = [blk+3FCh]` |
| 7 | `009F4CFB` | **`0080E170(unit, throttle)`** | the same |
| 8 | `00813020` | the ring tick clamps `slot[read]` and steps the live pair toward it | `__thiscall void(float dt)`, `RET 4`, `ECX = unit+838h`; run from `00826121` |
| 9 | `0081311F` | `0042AC60` with `ECX = ring+14Ch`: the ordered rudder itself | `__thiscall void(float goal, float step)`, `RET 8` |
| 10 | `00826B54` | `0092E8C0(controller, unit+984h, dt)` - the steering | `docs/SHIP_MOTION.md` |
| 11 | `00937572` | `MOV EDX,[EDI+1Ch]; FLD [EDX+984h]` in `BSP_UnitController_ApplyShipForces` | the same rudder into the force model |

### The two setters, in full

```
0080E170 BSP_UnitOrderRing_SetWriteSlotParamA        0080E190 ...ParamB
  MOV   EAX,[ECX+97Ch]                                 MOV   EAX,[ECX+97Ch]
  MOVSS XMM0,[ESP+4]                                   MOVSS XMM0,[ESP+4]
  SHL   EAX,5                                          SHL   EAX,5
  MOVSS [EAX+ECX+838h],XMM0                            MOVSS [EAX+ECX+83Ch],XMM0
  RET   4                                              RET   4
```

`ECX` is the **unit**, not the ring. `unit+838h` is the ring base, so `unit+97Ch` is the
write cursor `ring+144h`, the slots are ten records of `20h` bytes at `ring+0`, and the two
setters write `slot[write]+0` and `slot[write]+4`. Those are the throttle and the rudder
that the tick reads back at `00813020`'s `[ECX]` and `[ECX+4]`.

### Why the ring is not a player-only path: the cursor discipline

`00813020`'s tail settles it. After stepping the live pair it copies the write slot forward
and moves both cursors:

```
00813158  ESI = ring+144h (write); EAX = (write + 1) % 10           ; 0081315E..00813166
0081317C  REP MOVSD, ECX = 8        ; slot[next] = slot[write], 32 bytes
00813183  slot[next]+8 = 1
00813186  if ([[00E188A8]+1FE4h] != 2)                              ; single player
00813197      ring+140h = ring+144h ; the read cursor follows the write cursor
008131A3      ring+144h = next
008131B0  else                                                      ; a network client
008131B0      ring+140h += 1, wrapping at 10                        ; playback
008131C9      ring+144h = next
```

So in a single-player session **what the AI wrote last tick is exactly what the live pair
steps toward this tick**, with one tick of latency. A client instead walks the read cursor
forward one buffered slot per tick, which is what makes the same ten slots a network
playback buffer. One structure, two regimes, and the AI owns the write end in both.

The `REP MOVSD` at `0081317C` is also why a displacement census cannot see the slot copy.

## The writer census of `unit+984h`, complete

Every store form and every way of reaching the address, each with a positive control so no
negative is vacuous.

| scan | result | control |
| --- | --- | --- |
| `d9 ?? 84 09 00 00` | 14 hits, every ModRM is `8x`, so every one is `FLD`. No `FST`/`FSTP` | `d9 9e` occurs 3407 times in `.text` |
| `f3 0f 11 ?? 84 09 00 00` | none | `f3 0f 11 86` occurs 2232 times |
| `c7 ?? 84 09 00 00`, `66 0f d6 ?? 84 09 00 00` | none | the same prefixes occur elsewhere |
| `89 ?? 84 09 00 00` | one, `004DA331` in `BSP_SceneRecord_Construct`, a different structure | - |
| `8d ?? 84 09 00 00` (address-taking) | **none** | `8d ?? 4c 01 00 00` and `8d ?? 48 01 00 00` both return dozens |
| `40 6a 81 00`, `d0 da 80 00`, `00 da 80 00`, `b0 d9 80 00` in `.rdata`/`.data` | none: no vtable or table holds `00816A40`, `0080DAD0`, `0080DA00` or `0080D9B0` | `60 fd 6d 00` returns nine `.rdata` hits including `00CFC420` |

Nothing writes `unit+984h` by displacement and nothing takes its address, so every write
goes through the ring base `unit+838h`. Those, complete:

| writer | form | object and rule |
| --- | --- | --- |
| `00812F66` | `MOVSS [EAX+14Ch]` | `BSP_UnitOrderRing_Construct`, the initial value |
| `0080DA3A` | `MOVSS [ECX+14Ch]` | `BSP_UnitOrderRing_SetRudderImmediate`. Its only caller is `0081F980`, reached from `00758210` and `BSP_SubmarineUnit_Serialize`: a save/restore path, not a command path |
| `0082674C` | `MOVSS [EBX+14Ch]` | the inlined manual-autopilot setters in `00825F20`, gated on `unit+61h` |
| `0081311F` | `0042AC60` with `ECX = ring+14Ch` from `00813104 LEA EDI,[EBX+14Ch]` | the ring tick, stepping toward `slot[read]+4`. **This is the one the AI drives** |

`0092E8C0 BSP_UnitController_ApplySteering` has exactly one caller, `00825F20`, and appears
in no vtable, so `unit+984h` is the only steering input a ship has. AI ships and player
ships therefore steer through the **same** field; what differs is only which entry point
fills the slot the tick reads.

### The `unit+61h` regimes are exclusive, verified here

`009F50FC CMP byte [EAX+61h],0; JNE 009F524F` exits `BSP_ShipAi_ControllerStep` outright,
and `008266C1` selects the autopilot pair on the same byte. So the autopilot pair is a
manual override and never an AI path, which confirms `docs/GAME_EXECUTABLE.md`'s reading
from the other side. `unit+9E4h` at `00826B2E` is the rudder jam, a damage state.

## What `slot+44h` is, and what it is not

`00811960 BSP_UnitHeadingCommand_SetLimitedTarget` writes the limited heading target to
`slot+44h` at `00811A19`, from its only caller `009F4D10 BSP_ShipAi_PublishOrderSlot`.
**That value is not the steering input.** The drive routine takes its target from
`blk+324h` at `009F40BB` and never reads `slot+44h`.

`slot+40h`/`+44h`/`+48h` are published *state*, for other units to read. Their one
consumer on the prior document's `IMUL 54h` scan is `009D8CE0`, and that scan is a
displacement census: a reader holding a pointer to the slot would not appear in it.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/UNIT_AI_ORDER_SLOT_READER.md`, `docs/SHIP_AI_RING_WINNER.md` and `docs/SHIP_AI_RUDDER_HOP.md`: "on the image's evidence the AI never writes the order ring" | The AI writes it every step through `0080E190` at `009F4CE8` and `0080E170` at `009F4CFB`. The scan that missed it looked only for calls reaching `00816A40` or `0080DAD0`. `docs/SHIP_AI_THROTTLE_TO_RING.md` had said so six days earlier | the two setter bodies; `009F4CDF`/`009F4CF2 MOV ECX,[ESI+3FCh]`; `writes=96000` on USN02 |
| `docs/UNIT_AI_ORDER_SLOT_READER.md`: `ship_ai_throttle_to_ring`, "where `blk+1D0h` and `blk+1D4h` go ... this is the packet's biggest open question" | It was not open when that was written. `docs/SHIP_AI_THROTTLE_TO_RING.md` answers it, and `src/ship_ai_throttle_ring.cpp` reconstructs `009F4B99..009F4D04` and both setters | the prior document's own "Answer to the packet question" |
| `docs/SHIP_AI_THROTTLE_TO_RING.md` uncertainty 5: "whether the write cursor can be stale relative to the read cursor on a networked client was" not established | Established. `00813197` sets `ring+140h = ring+144h` in a single-player session, so the live pair follows the AI's own last write with one tick of latency; the client branch at `008131B0` advances `ring+140h` by one instead, walking the buffered slots | `00813186 CMP [[00E188A8]+1FE4h],2` and the two epilogues at `008131A9` and `008131CF` |
| `docs/UNIT_AI_ORDER_SLOT_READER.md`: `009D8CE0`'s "own caller is unknown - Ghidra records none" | Its caller is `009F3E30`, and Ghidra does record it. `009F3E30` itself has no caller in the graph | `python tools/bsp.py callers 009d8ce0` |
| `docs/SHIP_AI_RUDDER_HOP.md` and `src/game_hosts_ship_ai.cpp`: "the AI's published heading still has no established path to `unit+984h`" | The path is established and implemented, but it does not run through `slot+44h`: the drive routine steers from `blk+324h` | `009F40BB` |
| the census record `ShipAiOrder::slot_to_order_ring` at `00825F7C` | Misnamed for a third reason, and now also **unnecessary**: the thing it says is missing exists. Its comment and log note are corrected in place; the key is still not renamed so seven packets of counts stay comparable | `src/game_hosts_ship_ai.cpp` |

Appended sections, not rewrites, in the two documents this packet does not own.

## Reconstruction

**None, and none is needed.** Every hop already has one:
`bsp::ship_ai_order_ring_hop_009f4b99`, `bsp::ship_ai_ring_set_write_slot_rudder_0080e190`,
`bsp::ship_ai_ring_set_write_slot_throttle_0080e170` in `src/ship_ai_throttle_ring.cpp`;
`00813020` in `bsp/unit_state_message.hpp`; `0092E8C0` as
`bsp::ship_apply_steering_0092e8c0`. The host methods `ShipAiRing::set_write_slot_rudder`
and `ShipAiRing::set_write_slot_throttle` are bound in `src/game_hosts_ship_ai.cpp` and
`src/game_hosts_units.cpp`. Writing a fourth module would have duplicated them.

The only code change in this packet is the correction to the stale comment and log note.

## no_ghidra_function

none. `0080E170`, `0080E190`, `00813020`, `009F3F80`, `009F3E30`, `009D8CE0`, `0081F980`
and `009F50E0` all have Ghidra functions. `009F4034` is a label inside `009F3F80`'s body.

## Validation

No behavioural change, so no new run was taken and none is attributable. The measurements
stand from the same tree earlier today:

* USN02: `hull=125 deaths=3 total_damage=13673.7 attributions=126`, `ship ai ring
  hops=96000 writes=96000 rudder_law=93000 deadbands=17810 live_pair_changes=35010`,
  `world units=32 motion_ticks=96000 simulated=150.00 s moved=33.43 total_path=41584.83`.
* USN01: `hull=23 deaths=1 total_damage=220.0`.
* `./scripts/build.ps1` Win32 `/W4 /WX` clean, `ctest` 2/2.

`live_pair_changes=35010` and `total_path=41584.83` are the run-time evidence that the
rudder does follow the AI's heading: the live pair moves and the ships travel. The
`driven=0` field in the same line is a **dead counter** - nothing in the tree increments
`units_driven` - and it should not be read as a gate. Removing or wiring it is the first
follow-up below.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ship_ai_census_cleanup` | `00825F7C`, `units_driven` | Retire the `ShipAiOrder::slot_to_order_ring` record, which three packets have now corrected without renaming, and either wire or delete `units_driven`, whose `driven=0` reads as a failure and is not one |
| `ship_ai_ring_slot_record` | `00813020`, `0080E170`, `00812D40` | The ten `20h`-byte slots in full: `+8` is the consumed flag, `+0Ch`/`+10h` and `+14h`/`+18h` are the clamp bounds, `+1Ch` a byte the tick copies to `ring+150h`. Only `+0`, `+4`, `+8` and the bounds are read here |
| `ship_ai_published_slot_consumers` | `009D8CE0`, `009F3E30`, `009E3C00`, `00811D80` | What a ship does with another ship's published heading. `009F3E30` is now known to be `009D8CE0`'s caller and has no caller of its own, so the entry point is still open |
| `unit_order_ring_network_regime` | `00813186`, `00E188A8+1FE4h` | The client branch of the cursor advance, and what fills the ten slots on a machine that is not simulating |
