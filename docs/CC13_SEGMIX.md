# CC13 segmix: undone rows in segments 51, 58, 30, 41, 42, 50, 61, 54 and 46

Packet cc13-seg40F (segmix), worker cc13-seg40F. The work is on the terms cc9
approved on 2026-10-16, with the 2026-10-18 addendum
(`local/cc13_gameplay_terms.txt` in the cc13 tree). The row list is
`local/cc13_undone_segmix.txt`, 133 rows. Names are hypotheses, not recovered
symbols. Nothing here is wired.

## Groups

| Group | Rows | Outcome |
|---|---|---|
| In the struck range 0081B000-0081EFFF (seg51) | 22 | Not touched |
| Template bodies | 88 | Tagged `stl_shape`, `reviewed`, evidence `sys6_shape` |
| Game code, reconstructed | 13 | `src/cc13_segmix.cpp`, build-tested (cc13-segmixB to F) |
| Game code, cc9-hosted (verify-and-record) | 7 | Recorded with a verdict |
| Game code, not done yet | 3 | Following commits |

## Template bodies (cc13-segmixA)

Each body was read from disk bytes (`bsp.py disasm-raw`) up to its RET. It is
an MSVC container or algorithm instantiation:
- list and tree node buy, clear and construct;
- vector Tidy and InsertN, with the `_Xlen` throw, allocate and the copy, fill
  and copy-backward helpers;
- the refcounted-pointer copy and fill, which use only [00CE221C]/[00CE2220]
  InterlockedIncrement/Decrement and slot 0 at zero;
- `allocator::allocate`, which throws `std::bad_alloc` (00BF6340, 00BF6885);
- `basic_string::find` (`memchr` plus the compare 004B3FC0);
- tree iterator increment and decrement.

None of them reads a game field or a game constant. The only image constants are
SEH handlers. The value types' own constructors and destructors (for example
008E79D0 BSP_PowerupClass_CopyConstruct, 008E67B0 BSP_PowerupClass_Destroy and
008E72B0 BSP_PowerupModifier_CopyConstruct) are called from the template bodies.
They are not part of the tagged rows.

| Segment | Addresses |
|---|---|
| 51 | 0081AA60, 0081ABF0 |
| 58 | 008E5060, 008E58C0, 008E58E0, 008E5AA0, 008E6040, 008E6080, 008E60C0, 008E6510, 008E6650, 008E6680, 008E66B0, 008E7AE0, 008E7EB0, 008E8480, 008E85F0, 008E93F0, 008E9680, 008E9700, 008E9790, 008E9960, 008EB5D0, 008EB670, 008EBBB0, 008ED660, 008EDC30 |
| 30 | 006BFA20, 006BFAE0, 006BFB90, 006BFBD0, 006BFDF0, 006C02F0, 006C03F0, 006C06A0, 006C1430, 006C1600, 006C1660, 006C1750, 006C4B00, 006C4C10, 006C4C80, 006C4CB0, 006C6B50, 006C9500, 006C97B0, 006C9A50 |
| 41 | 00737900, 00737C10, 0073E6E0, 0073FC50, 0073FD50, 00743190, 00745030, 00746920, 00747900, 0074A000, 0074A0B0, 0074A6F0, 0074A760, 0074A830, 0074AB30, 0074B780 |
| 42 | 0075B830, 00764BA0, 00764C20, 007653F0, 00766AA0 |
| 50 | 00814870, 00815340, 00817B50, 00817C10, 00817C40, 00817DA0, 00817E00, 00817F50, 00817FC0, 0081A920, 0081A9C0, 0081AA10 |
| 54 | 00876A80, 008773F0, 008776B0, 00877E40, 00877E50, 008780F0, 00878190 |
| 46 | 007B1E60 |

Some of these already appear as host hooks in other files, for example
`powerup_seg58_argon`, `cc13_seg30`, `cc13_sweepC`, `landscape_lua_seg55_argon`
and `cc13_bigB1`. Those hooks are bind-only, which matches the tag.

## Game code

### Segment 42: the mother ship (cc13-segmixB)

The bodies are in `src/cc13_segmix.cpp` (Cc13SegmixHost). They were
transcribed from disk bytes and are build-tested and unwired.

| Address | Name | ABI | Body |
|---|---|---|---|
| 00759020 | BSP_SessionMessage_MOTHERSHIP_CREATE_Destruct | thiscall(message) | vtables 00D0189C / 00CF8EB8, frees the +148h and +138h vectors, JMP 0080E380 |
| 007590E0 | BSP_SessionMessage_MOTHERSHIP_CREATE_Write | thiscall(message; writer), RET 4 | 00813B10, then 006D44F0 on +12Ch |
| 007592A0 | BSP_SessionMessage_MOTHERSHIP_CREATE_Construct | thiscall(message; word, ship), RET 8 | 00813A20(word, ship, 8Bh), the snapshot fields, 006CC800(+12Ch, ship+1188h or 0) |
| 00759330 | BSP_MotherShip_SendCreateMessage | thiscall(ship; peer), RET 4 | a 154h-byte message on the stack, 00779FC0, then 00759020 |
| 007593D0 | BSP_MotherShip_InitAll | thiscall(ship) | see below |
| 00759920 | BSP_SessionMessage_LANDINGSHIP_CREATE_ConstructForRead | fastcall(message) | 0080E2C0, words +12Ch/+12Eh = 0, vtable 00CFF8E8 |
| 00766EF0 | BSP_SessionMessage_MOTHERSHIP_CREATE_ConstructForRead | fastcall(message) | 0080E2C0, vtable 00D0189C, the snapshot fields |

What 007593D0 does, in order:
1. It runs the unit init (00822C20).
2. It finds the "elevator_2" note node (0071AD50) into +1284h and copies the
   node's local matrix into +1288h (00B6DB60, 004134F0). The node is not
   null-checked.
3. It builds the air-ops block at +1188h:
   - 006C0D20 takes class +814h..+81Ch, +820h and +824h;
   - 006CADD0 takes [ship+C0h];
   - block+3Ch is set to the site from 006D0A10 (the trivial 006BA690 is inlined).
4. For a saved entity ([+C0h]+4h == 3), it reads the saved Lua data:
   - "_airBase" feeds 006BEF30, the traffic table;
   - "_motherShip" is entered and left with nothing read.
5. It ticks the block with dt 0 (006BA670) and sets +11A8h to 1.

cc9 hosts part of this as semantics, without the function itself:
- the 006CADD0 mode-1 deck build for motherships (`game_hosts_scene_contents.cpp`
  ~2205);
- the mother-ship landing-deck holder (`game_hosts_units.cpp` ~2902).

This packet makes no mismatch claim against those.

Verify-and-record:

| Address | Name | Host | Verdict |
|---|---|---|---|
| 00758FE0 | BSP_MotherShip_DetachFromWorldLists | `game_hosts_units.cpp` `world_lists_unlink_unit_00928f24` (~3282) | Matches. The image runs 006DFFC0 and then erases the first node whose payload is the ship from the class list [ship+30h]+84h (004837D0). The host erases the first matching node of every world list, which is the same set. |

### Segment 61: ship systems and failures (cc13-segmixC)

Reconstructed: 0093A2C0 BSP_SessionMessage9B_Construct. It is thiscall(message;
a, b, byte c), RET 0Ch. It runs 0075B430(9Bh), then sets:
- the +18h word and the +1Ah and +1Ch bytes to 0;
- +20h = a, the delivery +4h = 1 and the vtable 00D03400;
- +24h = b and byte +28h = c.

cc9 counts the send as `Failure::section_state_message_0093a2c0`.

Verify-and-record. Every host file is cc9's and was not edited.

| Address | Name | Host | Verdict |
|---|---|---|---|
| 0092E5B0 | BSP_UnitController_ApplyNetworkCorrection | `game_hosts_units.cpp` `controller_0092e5b0` (~10641) | Gate matches. The whole body sits behind byte controller+60h (0092E5B6, JE to the RET 4). cc9 keeps the gate closed; its claim that 0092F2E0 is the only writer was not re-scanned here. The gated x87 correction body is modelled by no one; it is multiplayer only. |
| 00933BB0 | BSP_ShipSystems_SinkSprayEffect (was FUN_) | `game_hosts_gunnery.cpp` `ship_systems_sink_draws_00933da0_00933bb0` | Matches the labelled substitution. One stream-1 NextU32, and on % 5 == 0 two U1 draws. The effect spawns only when the point's y < 0. The effect itself is recorded. |
| 00933DA0 | BSP_ShipSystems_SinkGridEffects (was FUN_) | same | Matches. There is an nx x ny grid from class+A4h/A0h * 1.5 / 15 + 1, one NextU32 per cell, and on % 3000 == 0 two U1 draws (ECX = EDX + 1 = 1). |
| 00935540 | BSP_UnitInstance_SpawnSinkingWreckDebris | `game_hosts_gunnery.cpp` `sinking_wreck_debris_draws_00935540` | Matches. Kinds 0Eh and 8 skip. The length is truncated (CVTTSS2SI) and the count is `_ftol(DebrisNumPerMeter FIMUL length)`. Each piece takes eight unconditional U1 draws. See the note below. |
| 0093AA00 | BSP_ShipGlobals_FindFailureRowByKind (was FUN_) | `game_hosts_gunnery.cpp` `failure_descriptor_for` (12734) | Matches. The result is the first settings+3E4h row (stride 14h) whose +0h equals the kind, else 0. |
| 0093BD80 | BSP_LuaBinding_AddEngineJamFailure | `game_hosts_lua.cpp` `run_set_failure_0088eff0` (~8714) | Matches. The duration is argument index 2 (index 0 is the first argument), equal to Lua index 3, when there are at least 3 arguments and it is a number. Otherwise it is 1e10f (501502F9h at 00CE4970). |

Note on 00935540: the host forms the piece count in double, while the image
rounds the FIMUL product to 24 bits. DebrisNumPerMeter is 0.3, stored as
0.300000012f, which is above 0.3. The product is therefore never just below an
integer, and the truncated count is the same for every length. There is no live
difference, so no adoption note was sent.

Two more segment-61 bodies were reconstructed by cc13-segmixD. Both were FUN_ and
are renamed in the ledger and in Ghidra; the old values are in
`local/ghidra-annotations-20261018T153752Z.json`.

**0093B920 BSP_RepairTask_Destruct**, thiscall(task). This is the unit+A20h
task that 0081F72F destroys; cc9's `unit_destructor_levels.hpp` lists it as
not modelled. In order:
1. For each of its 20 effect slots in the +4h vector, it stops the children
   (00867B10), sets byte +9h to 1, releases the reference and zeroes the slot.
   Every slot read is checked, and the slot is zeroed twice.
2. It destroys the +14h vector through 0093B300, then frees it.
3. It destroys the +4h vector through 0081C940, then frees it.

0081C940 lies in the struck range, but it is only called here as a hook.

**0092F930 BSP_UnitController_UpdatePoseFromBody**, thiscall(controller;
seconds), RET 4. It rebuilds unit+74h:
1. The source matrix is one of two:
   - the unit's world matrix +CCh, when byte controller+14h is set;
   - otherwise, the dyn body's transform at seconds / 0.05f (00C43EA0, 00C33650).
2. When byte unit+61h is set, it keeps the matrix's angles except the middle one,
   which becomes unit+FACh. It keeps y and sets x and z from unit+FB0h and +FB8h.
3. It clears bytes unit+C8h and +10Ch and invalidates the child nodes.

`unit_rudder.cpp` declares a hook for it, `controller_update_0092f930`, that
nothing binds.

### Segment 46: the mission camera (cc13-segmixE)

These are two camera bodies that 007A91A0 BSP_MovCam_Step calls. `cc13_bigB1`
reaches both through hooks: `movcam_007a84d0` and
`movcam_update_hand_mode_007a87b0`. No body existed for either.

**007A84D0 BSP_MovCam_CycleTarget**, thiscall(camera; byte backward), RET 4. The
ledger name BSP_Recon_PublishedListCursor was replaced, and the Ghidra old value
is in `local/ghidra-annotations-20261018T154313Z.json`. The units come from the
active player's three lists: [[game+18ECh*4+18CCh]+30h] at +DD8h, +DE4h and +DF0h
(each a count, then a head).
- With no target [camera+38Ch], it takes the first unit of the first list that is
  not empty. That unit becomes the target if it answers kind 5. Otherwise the
  search steps on from it as in the next case.
- Otherwise it finds the target's index over the joined lists (00793610,
  00793670, 007936D0). It then steps, by -1 when the byte and camera+518h are
  both set and by +1 otherwise, wrapping, until a unit answers kind 5 (vtable
  +5Ch). The total count bounds the number of steps.
- The result goes to 007A6910. That is the unit, or null when nothing answers.

The `cc13_bigB1` hook declares the argument as an int, but the image tests only
its low byte.

Deviation: when the list walk runs past the end, the image reads [0+4], which
faults. The C++ returns a null unit instead.

**007A87B0 BSP_MovCam_UpdateHandMode**, thiscall(camera). It moves between the
target's vehicle cameras with the stick.
- The cameras are [[[camera+38Ch]+538h]+BCh]: 5Ch-byte entries with x at +38h, z
  at +40h and a disabled byte at +58h.
- The stick is camera+504h (dx) and +508h (dy, inverted), each with a dead zone
  of 0.25.
- The columns split at plus or minus class+A4h / 64. The new column is the
  current one plus dx.
- Pass 1 takes the enabled entry in the new column with the nearest z on the dy
  side. If it finds nothing, pass 2 does the same over the middle column, but
  only for a pure vertical push from a side column.
- It then calls 007A6710. When [camera+390h] is set, it calls 0079F710, and then
  00797070(0, 0) if 0079F710 did not fail.

The two reversed subtractions were read from the bytes: D8 E9 at 007A8A22 and
DE E2 at 007A8B7D. Both compute z0 - z, and only the magnitude is used.

**007A74F0 BSP_MovCam_SetPathWalkPointProperty** (cc13-segmixF). The signature
is thiscall(camera; LuaObject table by value, float a, byte reverse, float c,
name, variants), RET 28h. The Lua native 008B4DA0 is its only caller, and
`cc13_left55_seg56b` declares the hook. CC13_SYS_5B noted that its stack slots
were never traced; they are traced here from the frame offsets.

It clears the path records, then walks the table. Each key k fills camera+478h
slot k - 1 (when reverse is set) or k + 1 with a new 4Ch record. The fields come
from the value's keys; the strings were read from the PE:

| Key | Record field |
|---|---|
| `parent` | +14h |
| `pos` (`x`, `y`, `z`) | +18h..+20h |
| `rotRefEntity` | +24h, mode +48h = 1 |
| `rotRefPosW`, `rotRefPosL` | +28h..+30h, mode 2 / 3 |
| `rotRefRotW`, `rotRefRotL` (`x`, `y`) | +34h / +38h, mode 4 / 5 |
| `sustainBefore`, `sustainAfter`, `speed` | +3Ch, +40h, +44h |

The two entity slots go through the observer pair calls (006952A0 unregisters,
00694A60 registers), and each is checked against the other slot. It ends with
007A5A00 and destroys the by-value table.

When operator new returns null, the image stores the null record and then
faults on it. The C++ does the same.

## Files

- 88 tag rows in `config/tags/006b0000`, `006c0000`, `00730000`, `00740000`,
  `00750000`, `00760000`, `007b0000`, `00810000`, `00870000` and
  `008e0000.jsonl`.
- `include/bsp/cc13_segmix.hpp` and `src/cc13_segmix.cpp`, registered in
  `cmake/startup.cmake` (bsp_core).
- 20 function records in `config/reconstruction/00750000`, `00760000`, `007a0000`,
  `00920000` and `00930000.jsonl`: 13 bodies and 7 verify-and-record.
- 7 renames in `config/names/007a0000`, `00920000` and `00930000.jsonl` (007A84D0, 0092F930, 0093B920, 0093A2C0,
  0093AA00, 00933BB0, 00933DA0), applied in Ghidra; the old values are in
  `local/ghidra-annotations-20261018T153423Z.json`, `...T153752Z.json` and `...T154313Z.json`.
