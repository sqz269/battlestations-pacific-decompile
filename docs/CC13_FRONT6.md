# CC13 front6: wiring-frontier callees

Packet cc13-front6 (2026-10-18). The targets are unreconstructed callees that block the wiring of callers
which are already reconstructed (`local/output/wire_readiness.json`), ranked by how many blocked rows each
one blocks alone and how many of those sit under wired callers (`local/front6_rank.py`, adapted from
`local/cc13_frontier.py`). Excluded: segments 67 and 59, the struck cc9 ranges, vendor/library bodies
(bound, not reconstructed), and anything a cc9 host already implements (recorded instead).

New code is in `src/cc13_front6.cpp` and `include/bsp/cc13_front6.hpp`, built into `bsp_core`. Nothing is
wired. Descriptive names are hypotheses, not recovered symbols. Every call the image makes goes through a
`cc13_front6::Front6Host` method named for its target; image globals go through `global_address` /
`global_pointer`. Status of all reconstructed rows: built (Win32 MSVC). They are not fixture-tested,
ABI-compatible or game-validated.

## Readiness before and after

`python tools/wire_readiness.py` in this tree (source-level approximation, see the tool's docstring):

| | ready | blocked_on_callees | wired |
|---|---|---|---|
| before (origin/main at the worktree base) | 961 | 224 | 6002 |
| after | 997 | 162 | 6015 |

"Unblocks" below names the caller rows whose state is now `ready` (w = the row itself has a wired caller),
plus rows still blocked on another callee.

## One-instruction bodies

All verified from the bytes on disk (`local/front6_dis.py`). `__fastcall(ECX)` below means the only input
is ECX and the body ends in a plain RET.

| Address | Function | ABI and body | Unblocks |
|---|---|---|---|
| 0054D3C0 | `hints_screen_seconds_left_0054d3c0` | `__fastcall(ECX = screen)`: CVTTSS2SI EAX,[ECX+54h]. Truncating; NaN and out-of-range give 80000000h, kept through `_mm_cvttss_si32` | 00690FD0 (w) |
| 004B47E0 | `debug_print_004b47e0` | `__cdecl` variadic, bare RET (the debug print is compiled out); callers push (x, y, format, ...) and clean | 00B1EF60 (w) |
| 00ADEAF0 | `terrain_tile_model_00adeaf0` | `__fastcall(ECX = tile)` -> [ECX+24h] | 00ADB980 (w) |
| 00AF1130 | `particle_resources_sub_object_00af1130` | `__fastcall(ECX)` -> [ECX+24h] | none ready yet (00AF0F20 has no wired caller) |
| 00ADAC30 | `height_field_cells_x_00adac30` | `__fastcall(ECX = height field)` -> [ECX+38h] | 00AEB770 (w), with 00ADAC40 |
| 00ADAC40 | `height_field_cells_z_00adac40` | `__fastcall(ECX = height field)` -> [ECX+3Ch] | 00AEB770 (w), with 00ADAC30 |
| 00BBDDC0 | `atmosphere_source_00bbddc0` | `__fastcall(ECX)` -> [ECX+0Ch] | 00BBDA50 (w) |
| 00B4CBB0 | `render_pass_camera_00b4cbb0` | `__fastcall(ECX = pass)` -> [ECX+0Ch] | 00B50050 (w) |
| 00B0CF20 | `post_effect_enabled_00b0cf20` | ECX ignored, XOR AL,AL; RET. Only AL is defined; EAX bits 8-31 keep the caller's value (not modelled) | 0051FA50 (no wired caller) |
| 00A4C2C0 | `empty_body_00a4c2c0` | bare RET; the caller 004DB20F passes ECX = 004C1710 result | 004DB190 (w) |
| 006F7B50 | `empty_body_006f7b50` | bare RET | 004DC6A0 (w) |
| 00778800 | `empty_body_00778800` | bare RET; caller 0076D99F passes ECX = EDI | 0076D8E0 |
| 00786CA0 | `empty_body_00786ca0` | `__thiscall(ECX; three dwords)`, RET 0Ch and nothing else | 00783E00, 00783EA0 |
| 00B916F0 | `quadtree_type_00b916f0` | RET -> EAX = [010903E0] | 00B72030 (w) |
| 008D45C0 | `global_f88978_get_008d45c0` | ECX ignored, RET -> EAX = [00F88978]. Callers call it a language count or a settings query; the body only reads the global | 005F42D0, 005F58F0 |
| 0078E470 | `camera_view_unit_0078e470` | `__fastcall(ECX = view)` -> [ECX+38Ch] | 004BFED0 |
| 005662F0 | `mode_selector_set_005662f0` | `__fastcall(ECX = mode)`: [00E086C0] = ECX | 005EC990 |
| 008DA180 | `objective_manager_008da180` | `__fastcall(ECX = owner)` -> ECX + 24h (LEA, an address) | 00603120 |
| 007F7D90 | `session_message_root_store_vtable_007f7d90` | `__fastcall(ECX)`: [ECX] = 00CE4974 (the BSP_SessionMessageRoot vtable). Probably an inlined base-destructor vtable store; provisional | 007752D0 (w) |
| 007B9000 | `run_surface_mark_released_007b9000` | `__fastcall(ECX)`: byte [ECX+0C01h] = 2 | 009CD540 |
| 005CE750 | `option_panel_set_flag_8d_005ce750` | `__fastcall(ECX = panel)`: byte [ECX+8Dh] = 1 | 005E51D0 still blocked on 005DF510 |
| 00472520 | `field_28_get_00472520` | `__fastcall(ECX)` -> [ECX+28h] | none (006E4640 has no identifier) |
| 00479BC0 | `vehicle_slot_class_00479bc0` | `__fastcall(ECX = slot)` -> [ECX+28h] | 00487470 |
| 0047BBA0 | `forward_object_vslot_08_0047bba0` | `__thiscall(ECX ignored; object)`, RET 4: MOV ECX,[ESP+4]; CALL [[ECX]+8]; returns its EAX. It dispatches the ARGUMENT, not the caller's ECX ([game+21D0h] at 004E9EF6); a header comment in another file calls it a forwarder to the manager's vtable[8], which is wrong | 004E9E40 still blocked on 008F0DF0 (segment 59, excluded) |

## Larger bodies

| Address | Function | ABI | Evidence and uncertainty | Unblocks |
|---|---|---|---|---|
| 0077C160 | `net_entity_write_replication_state_0077c160` | `__thiscall(ECX = entity; archive)`, RET 4 | Assembly 0077C160-0077C29B. 00928450(entity; archive). Enter "_netEntity" (00D03E20), enter "_reconData" (00D03E14), 00805020(ECX = entity+1E4h; archive), one leave. Write "hasPlrCmd" (00D03E08) = {3, byte +290h}. When [entity+284h] is non-null and its +14h is the entity: enter "formation" (00D03DFC), 0070E890(ECX = [entity+284h]; archive), leave. Enter "lastBanto" (00D03DF0), write "id" (00CF16BC) = {1, word +2C8h zero-extended}, leave. Write "lastBantoPlayerIndex" (00D03DD8) = {1, dword +2D8h}, leave. The archive's vtable +4/+8/+0Ch are host methods; key payloads are the image string addresses. The type-3 payload's upper three bytes are uninitialised stack bytes in the image and zero here | 008776E0 (w); 007B0BA0 |
| 005D7070 | `find_menu_record_for_entry_kind_005d7070` | `__fastcall(ECX = entry index)` -> EAX | Assembly 005D7070-005D71AA. When [[00E188A8]+5F0h] <= index (signed), it returns 0. Otherwise node = 004BE940(ECX = game+5F0h; index), and kind = [node+1098h] with 29h mapped to 26h and 2Ah to 27h. It scans the 434h-byte records of [[00E198AC]+5Ch]+28h/+2Ch, re-reading the manager each pass. The loop stops with 0 on a null vector or index >= size (the size is a signed division, compared unsigned). The inlined checked index re-tests and calls 00BF6713. Each record's leading NativeString is compared with 004260B0(&local; kind): the lengths must be equal, then both empty matches, otherwise `_stricmp` 00BF7FBF(record, key). The local is released through 00419CC0 / 00BD1510(data, len+1, 1) when its data is set. A match returns the index, so "not found" and "index 0" both return 0, as in the image. `game_hosts_ready.cpp` holds an unimplemented stand-in (`select_menu_record_005d7070`) | 005E1990 (w), 005E21B0 (w) |
| 00A04240 | `ai_hint_weight_entity_multiplier_00a04240` | `__fastcall(ECX = entity)` -> ST0 float | Assembly 00A04240-00A04310. The key is the entity pointer. w = 1.0f (00D7A24C). The first lookup uses `std::map` find 00A03B20 (bound; host hook) on 00F8A740, end = [00F8A744]. The second uses the map at 00F8A750 + 0Ch*[00E0E344], end = [map+4]: w = float(w * [node+10h]). Each inlined checked-iterator test calls 00BF6713, at the same points. The first result is read from the local iterator, the second through the returned EAX. FMUL m32 / FSTP m32 rounds once whatever the precision control. `game_hosts_ai.cpp` (cc9) notes 00A04240 as unread; this body does not change it | 00A046C0 (w) |
| 0077A2F0 | `unit_health_colour_0077a2f0` | `__thiscall(ECX = out float[4]; float health)`, RET 4 -> EAX = out | Assembly 0077A2F0-0077A4A0. Three function-local statics are set once under guard bits 1/2/4 of 00F8718C. 00F8717C = {3EC8C8C9, 3F68E8E9, 3E149495, 1.0}, 00F8716C = {3F7DFDFE, 3F70F0F1, 3E008081, 1.0} and 00F8715C = {3F57D7D8, 3E2CACAD, 3E2CACAD, 1.0}, read from 00D03DAC-00D03DC8 (1.0 from 00D7A24C). If health > 0.5f (COMISS 00CE3800; JBE sends <= and unordered to the other arm): t = float(2*(h - 0.5)) using the double at 00D7A280, s = 1 - t, out = scale(00F8716C, s) + scale(00F8717C, t). Otherwise t = 2h, s = 1 - t, out = scale(00F8716C, t) + scale(00F8715C, s). Here scale is 00BEE140 (ECX = out, EDX = &scalar, stack vec, RET 4). DE E1 is FSUBRP, so s = 1 - t. Every stored value rounds once at 24- or 53-bit PC (h - 0.5 is exact in double and the doubling is exact), so floats are used. `game_hosts_hud_world.cpp` records 0077A2F0 as a float `unit_health` stand-in, which does not match this colour-returning body (adoption note) | 0077A4B0, 0077A510 (no wired caller yet) |
| 0073E9A0 | `dword_array_reserve_0073e9a0` | `__thiscall(ECX = {data, count, cap}; int requested)`, RET 4 | Assembly 0073E9A0-0073E9FC, instruction-identical to 00B22650 apart from call targets. A request below 1 becomes 1. It returns when capacity >= request (signed). fresh = 00BF55BE(n*4). Each dword is copied into a non-null destination slot, re-reading data and count every pass. Then 00BF6989(data) runs even when data is null; data = fresh, capacity = n | 0073FBF0, 0073FD00 |
| 0086A350 | `float_pair_array_reserve_0086a350` | same | Assembly 0086A350-0086A3B3. Same as 0073E9A0 with 8-byte elements, each copied as FLD/FSTP dword +0 then +4. The x87 round trip turns a signalling NaN quiet (bit 22). That is modelled explicitly; other values are bit-exact | 0086C120, 0086D180 |
| 0086CE10 | `named_record14_array_reserve_0086ce10` | same | Assembly 0086CE10-0086CF04. Clamp to 1 and test capacity as above. fresh = 00BF55BE(n*14h). 0086CDB0(ECX = slot; old record) runs for each non-null slot. Each old record's string {+8 length, +0Ch data} is then released through 00419CC0 / 00BD1510 when its data is set. Then 00BF6989(old), and data and capacity are published. The EH frame (00C95847) is not modelled | 0086E670 |
| 0086E6F0 | `named_record14_array_resize_0086e6f0` | `__thiscall(ECX = array; int count)`, RET 4 | Assembly 0086E6F0-0086E768. If count > capacity, it calls 0086CE10 (the body above). Growing zeroes +8/+0Ch of each new record whose address is non-null. Shrinking decrements the count field first, then releases that record's string. Finally count field = count | 0086F3B0, 0086F900 |

## Existing implementations, recorded ledger-only

| Address | Existing body | Verification |
|---|---|---|
| 00B22650, 00B226B0, 00B22710 | `reserve_native_material_{third,render,sampler}_cache_*` in `src/native_material_state_cache.cpp` | The three bodies are instruction-identical except for call targets. The implementation adds `require()` overflow/consistency checks the image lacks (error paths only) and allocates through `singleton_lifetime_allocate` instead of 00BF55BE. Copy, free and publish order match. Unblocks 00B22E90, 00B22EE0, 00B22F30 (each w) |
| 006BC530 | `GameUnitsHost::Impl::landing_over_runway_006bc530` in `src/game_hosts_units.cpp` (cc9, read-only) | Checked against 006BC530-006BC5CE: half extents float(B0h*0.5) and float(B4h*0.5) (DC C9 is st1 *= st0), the 004142E0 inverse transform with +48h, and *height = local y when the pointer is non-null. JBE after FCOMIP sends NaN to false. The host takes a typed `LandingDeck` copy of the holder. Unblocks 007BB2C0, 007D9F60 (neither ready by itself: no wired caller) |
| 00718F50 | `StructuredNode_ReadCharacterString_00718f50` in `src/flak_note_seg36_37_argon.cpp` | Checked against 00718F50-00718FE2 (00BEA010, 00E19B94 fallback, strlen, 00408720, release of the local). Unblocks 00719000, 00719080 |
| 007F8CA0 | `PcProfileIoHost::profile_has_save_name_007f8ca0` in `src/pc_profile_io.cpp` (typed host model) | The image compares the profile string {+34h, +38h} with 00436710 (assign "" from 00CE3A0C). A zero length gives false; otherwise the empty local gives true. The result equals the host's `!save_name.empty()`. The host does not model the local string or its release. Unblocks 008D6C00 (w) |

## Bound library bodies (rule 2b), name records only

These are stock library code. The name ledger evidence now carries the bind note and the STL/CRT
identification, so `wire_readiness.py` classes them as library-bound.

| Address | Identification | Rows |
|---|---|---|
| 00441210 | `std::vector<LuaObject>::_Tidy` | 0087BF80 (w), 00AC6600 (w), plus 0052FFC0, 007016C0, 008ECEC0 |
| 00440600 | `std::_Uninit_fill_n` for LuaObject (EH frame, copy-construct per 14h slot) | 00440A30 |
| 00453ED0 | MSVC `vector copy constructor iterator` | 00BAF350 (w), 00BAFC00 (w) |
| 00403560 | MSVC `vector constructor iterator` (__stdcall, RET 10h; bytes verified) | 00BE3820 |
| 00A9D9D0 | `std::list<void*>::assign(first, last)` | 00A9E230 (w) |

## Skipped or deferred

- **00531130 BSP_TextContext_SetGlyphReference**: `src/gui_text_glyph_reference.cpp` holds a typed model
  that replaces the pool resize with a `std::u16string` copy and adds a throw. It is neither faithful enough
  to record nor something to duplicate here.
- **00698A10, 005C6A70, 0087CA80, 0061B0F0**: 450-780 decompiled lines each; too large for this packet.
- **0078CFF0** (environment/fog apply, about 25 callees with register inputs) and **00748A40** (the cc9
  `game_hosts_units.cpp` fragment): left for a packet that can check the listing in full.
- **005DF510** (783 lines) still blocks 005E51D0. **008F0DF0** is in segment 59 (excluded) and still blocks
  004E9E40.
