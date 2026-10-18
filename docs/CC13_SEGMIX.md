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
| Game code, reconstructed | 7 | `src/cc13_segmix.cpp`, build-tested (cc13-segmixB) |
| Game code, cc9-hosted (verify-and-record) | 1 | Recorded with a verdict |
| Game code, not done yet | 15 | Following commits |

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

## Files

- 88 tag rows in `config/tags/006b0000`, `006c0000`, `00730000`, `00740000`,
  `00750000`, `00760000`, `007b0000`, `00810000`, `00870000` and
  `008e0000.jsonl`.
- `include/bsp/cc13_segmix.hpp` and `src/cc13_segmix.cpp`, registered in
  `cmake/startup.cmake` (bsp_core).
- 8 function records in `config/reconstruction/00750000` and `00760000.jsonl`:
  7 bodies and 1 verify-and-record.
