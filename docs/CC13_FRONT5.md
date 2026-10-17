# CC13 front5: wiring-frontier callees

Packet cc13-front5 (2026-10-17). The targets are unreconstructed callees that block the wiring of callers
which are already reconstructed (`local/output/wire_readiness.json`, ranked by `local/cc13_frontier.py`).
New code is in `src/cc13_front5.cpp` and `include/bsp/cc13_front5.hpp`, built into `bsp_core`. Nothing is wired.
Descriptive names are hypotheses, not recovered symbols. Every call the image makes goes through a host
method named for its target: `cc13_front5::ArchiveEmitHost` extends `cc13_seg95::Host` for the archive
emitters, and `cc13_front5::Front5Host` serves the rest.

Status of all reconstructed rows: built (Win32 MSVC). They are not fixture-tested, ABI-compatible or game-validated.

## Segment 95: archive text emitters

These use the shared globals 0109CEE0 (the FILE), 0109CEE4 (the compact flag), 0109CECC (the storage
singleton) and 0109CEE5 (the empty string). Each emitter ends with the same tail. In compact mode it copies
the text into the by-value argument of 00BD6100. Otherwise it calls `fwrite(data or 0109CEE5, length, 1, FILE)`
when a FILE is set, or else 00BD49B0(ECX = [0109CECC]; &text). Then it runs the inlined `~NativeString`:
00419CC0 returns the pool and 00BD1510(data, length + 1, 1) takes the block back. That release is skipped for
null data. `src/archive_text_writer.cpp` already held fragment models of these routines over its own types;
the new bodies use the segment 95 `NativeStr` / `TaggedValue` types that their callers use.

| Address | Function | ABI | Unblocks |
|---|---|---|---|
| 00BD6100 | `archive_emit_compact_encoded_00bd6100` | `__stdcall(NativeStr by value)`, RET 8 | 00BD6DB0 `stdio_stream_write_string` |
| 00BD6990 | `archive_emit_indent_00bd6990` | `__fastcall(ECX = depth)`, RET | 00BD6D00, 00BD8D10, 00BD8D70 |
| 00BD6AC0 | `archive_emit_terminator_00bd6ac0` | `__fastcall(CL = root)`, RET | 00BD6D00, 00BD8D70 |
| 00BD6BD0 | `archive_emit_string_00bd6bd0` | `__stdcall(NativeStr by value)`, RET 8 | 00BD6D00, 00BD8D10 |
| 00BD7B30 | `archive_emit_key_00bd7b30` | `__fastcall(CL = root; tag, payload)`, RET 8 | 00BD8D10, 00BD8D70 |
| 00BD7F50 | `archive_emit_value_00bd7f50` | `__stdcall(tag, payload)`, RET 8 | 00BD8D70 |

With these six, the `cc13_seg95` stdio stream writers 00BD6D00, 00BD6DB0, 00BD8D10 and 00BD8D70 have every
callee reconstructed.

- **00BD6100.** Each byte that is not a space, CR, LF or TAB is written as `(char >> 4 arithmetic) | (char << 4)`
  at byte width. The data pointer is re-tested against null before every read. The byte is staged in the
  caller's length slot and written with `fwrite(&b, 1, 1, [0109CEE0])`.
- **00BD6990.** The image calls 0041DD40(&s, depth, 1), then `memset(' ')` when the length is non-zero, then
  `memset('\t')` when the data is non-null. The spaces are always overwritten, and the result is `depth` TABs.
- **00BD6AC0.** 0041E870 builds the local from 00CE4390 `"\n"` at the root and from 00CE438C `",\n"` otherwise.
  The local is not zeroed first, which is also how 00BD8D10 uses 0041E870.
- **00BD7B30.** At the root the key is `name + " = "` (00CF8284), built with 004261A0 and then the inlined
  copy. Nested keys start from `"["` (0041DD40 with preserve 0, then memcpy). Tag 0 appends `"\"" + name + "\""`
  with three concatenations. Tag 1 uses 004CACA0 and tag 2 uses 00781740. Any other tag appends nothing. The
  code then appends `"] = "` (00D68364) by resize and memcpy. Temporaries are created and released in the
  image's frame order.
- **00BD7F50.** The 840-instruction body was paged with the release blocks collapsed (`local/front5_asmfilter.py`).
  - Tag 0: 004CAD40 replaces `\` with `\\`, then `"` with `\"`, both with count 7FFFFFFFh. The result is then quoted.
  - Tag 3: `"true"` or `"false"`, chosen by the payload's low byte only, then appended with 00425E10.
  - Tag 4: the id from `[0109CEDC](ECX = payload)`, appended with 004CACA0.
  - Tags 5, 6 and 8: vectors built with 006AF2B0 and 004261A0 and assigned with 00425F40. Tag 8 copies its four
    floats into the frame before any call. The 0041DD20 destroys follow frame order.
  - Tag 7: a 4x4 matrix of 16 floats, written row by row as `"{ "`, then `"%f, "` four times, then `"}, "`,
    and closed with a final `"}"`.
  - Tag 10: a quoted 004263B0 integer.
  - Any other tag emits the empty string. A float payload passes through FLD/FSTP, so it is an exact copy
    except that a signalling NaN is quietened.

## Segment 26 and the segment 52 getters

| Address | Function | ABI | Unblocks |
|---|---|---|---|
| 0064A2F0 | `hud_ship_screen_close_order_menu_0064a2f0` | `__fastcall(ECX = screen)`, RET | 00603120 `hud_pause_screen_enter` (still needs 008DA180) |
| 0064B650 | `ship_captain_camera_construct_0064b650` | `__fastcall(ECX = block)`, EAX = block | 0067C1A0 `torpedo_boat_view_bind_unit_camera` (has a wired caller) |
| 0064A520 / 0064A530 | `quad_fade_target_clear/set` | `__fastcall(ECX)`, RET; +20h = 0 / 1 | 00609BD0 `plane_screen_update_instruments` |
| 00652A30 | `byte_08_get_00652a30` | `__fastcall(ECX)`, RET, AL = [ECX+8] | 0068C0B0 `ingame_interface_toggle_tactical_overlay` (has a wired caller) |
| 008356B0 / 008356C0 / 008356F0 | `ship_float_244_get` / `ship_byte_243_get` / `ship_float_248_get` | `__fastcall(ECX)`, RET; ST0 / AL / ST0 | 007214C0 `snapshot_build` |

- **0064A2F0.** The function clears the bytes at +157h and +156h and calls `[+38h]->vtable[34h](0)`. It then
  calls 00545360(ECX = [[00E198C4]+50h]). Next it reads unit = [00E188D8] and setting = [screen+14Ch], stores
  `[unit+A44h] = setting`, and calls 00983C10(ECX = [00F8A0C4]; [unit+A20h], setting). It finishes with
  0068AB80(ECX = [00E198C4]; 0, 0) and a tail jump to 0068AB30(ECX = [00E198C4]).
- **0064B650.** The constructor calls the base 00519290 and then writes its own fields: +3ECh = BF060A92h
  (-30 degrees in radians, from 00CEC728), +3F0h = 3FC6D3F2h (89 degrees, from 00CEC400), +3F4h = 0, the byte
  +400h = 0, and four vtables. It has no null test. The existing `construct_ship_captain_0064b650` in
  `src/mission_camera.cpp` is a semantic model that skips the 00432750 RNG draws. This body performs the
  writes; its base call 00519290 is a host hook.

## Segment 6

- **0047BB70 `object_forward_vslot_08_0047bb70`.** Callers load ECX, but the body never reads it. It does
  `MOV ECX,[ESP+4]; PUSH [ESP+8]; CALL [[ECX]+8]; RET 8`, so the ABI is `__thiscall` with an unused ECX and the
  stack arguments (object, argument), returning EAX. It unblocks 004F0CC0 `wreck_create_from_scene` (which still
  needs 008F0DF0) and 0074DB70 `land_vehicle_on_destroyed`.
- **004768D0: existing implementation, recorded ledger-only.** `read_native_vertex_position_004768d0` in
  `src/native_vertex_position_read.cpp` was checked against the listing:
  - the +50h == 0 copy;
  - formats 2/3/5/7/8/10/12/13/16;
  - the divisors 255.0 (00CE4B48), 32767.0 (00CE5F98) and 65535.0 (00CE5F90);
  - the D3DX half import;
  - the scale/bias record `[+50h] + [+18h]*20h` and the FLD/FSTP copy-out.

  Deviation: for any other format the C++ returns false, while the image scales an uninitialised local
  (undefined input). The ABI is `__thiscall(ECX = stream; float* out, index)`, RET 8. This record lets
  00BAF8A0 `coastline_clip_draw_section` and 0073F6E0 `mesh_decal_collect_tree_triangles` (which also needs
  0070FDB0) bind to the existing body.

## Segment 53

- **008624C0 `director_bridge_apply_008624c0`.** `__thiscall(ECX = bridge; byte force)`, RET 4. The bridge is
  {+0 ai, +4 director, +8 countdown, +0Ch..+10h last pushed bytes}. The function runs these steps:
  1. `ai+7Ch = director+3Dh`.
  2. It decrements the countdown. When force is set or the count is `<= 0` (signed), it sets force to 1 and the
     count to 10.
  3. Stance director+3Ch against +0Ch: when it changes (or on force), it writes three dwords over ai+70h..+7Bh,
     either 01010101h or 0, and restores the bytes at +77h and +78h. In the zero case it then sets +70h and +71h
     when `[ai+50h]->vtable[5Ch](0Fh)` returns true.
  4. +222h goes to 00861D70, +220h to 00861CD0 and +221h to 00861D20, each on force or change. +223h goes to
     00861DC0 on change only; the image has no force test there (008625D9).

  The image pushes the byte through the argument slot, so the upper three bytes of each pushed dword are
  stale. `apply_director_stance_008624c0` (`unit_gunnery_pass`) and `director_update_arms` are decision
  models, and the gunnery host in `game_hosts_gunnery.cpp` (read-only) uses them. This is the whole body.
  It unblocks 00863A80 `unit_gunnery_ai_create_director_bridge` (which has a wired caller).
- **0086C120 `water_tracer_destroy_curves_0086c120`.** `__fastcall(ECX = tracer)`, RET. The function destroys
  the members in reverse order:
  - +3Ch: 0086A430(0), then free;
  - +30h, +24h and +18h: 00818030(0), then free;
  - the qword vectors at +0Ch and +0: 0086A350(0) when the capacity is negative (signed), then the inlined
    count-down loop to 0, the count set to 0, and free(data).

  The data pointers are not cleared. It unblocks 0086D180 `water_tracer_read_lua` (which still needs 0086A350).

## Segment 40: the gun

| Address | Function | ABI | Unblocks |
|---|---|---|---|
| 0072E510 | `gun_construct_0072e510` | `__thiscall(ECX = block; argument)`, RET 4, EAX = block | 006E3C00 `rack_construct` |
| 0072B2D0 | `gun_update_barrel_recoil_0072b2d0` | `__thiscall(ECX = gun; float dt)`, RET 4 | 0085A270 `gun_update` |
| 0072ADC0 | `gun_save_fields_0072adc0` | `__thiscall(ECX = gun; archive)`, RET 4 | 0085A130 `turning_gun_debug_dump_fields` |
| 0072F060 | `gun_load_saved_state_0072f060` | `__fastcall(ECX = gun)`, RET | 0085BB50 `turning_gun_load_instance_state` |

- **0072E510.** The base 00728960(block, argument) runs first. The field stores then follow in the image's
  order, among them:
  - the six vtables;
  - the embedded sub-object at +424h (vtable 00CFDEA0), whose +428h points back at the block;
  - +458h = 00CE75CC;
  - -1 at +3FCh and +440h;
  - the class id 20h at +0C4h;
  - 0.0f at +474h and +478h.
- **0072B2D0.**
  - Barrel visibility: the function refreshes `[gun+3BCh]` through 00B6DB70 when bit 2 of +5Ch is clear. It
    calls 007297B0 when `0 > [node+124h]` (ordered) and 00729870 otherwise.
  - It then calls 0043FF40(gun, dt).
  - The loop runs over the 14h-byte barrel records with the checked index (00BF6713). UCOMISS, LAHF and
    `TEST AH,44h` leave only an equal, ordered speed idle, so a NaN speed counts as active.
  - For an active record, the time bank is `float(bank + dt)`. While the bank is at least 1/120, the function
    subtracts 1/120 from it. A negative speed adds the speed to the position, multiplies the speed by the
    damping [cfg+ACh], and sets the speed to [cfg+B4h] when [cfg+B0h] >= position. A zero or positive speed
    adds it to the position and zeroes the position, the speed and the bank when the position is >= 0. The
    constant is 00CEF0B8 (float) and 00CFDF80 (the same value as a double).
  - Precision: every sum and product is of floats, so it is exact in double before the single float rounding.
    The result is therefore the same under 24-bit and 53-bit precision control. Every comparison is written so
    that NaN takes the image's arm.
- **0072ADC0.** The function calls 0043FEA0(gun, archive) and then writes the gun through archive vtable slots
  +4 (open), +8 (close) and +0Ch (write), with tagged keys and values passed by value. A bool value has only its
  low byte written into the stack slot. The `fireModeAmmo` entries are `MOVZX` words from the record +4. The
  barrel fields use the checked index. `genParam` saves itself through its own vtable +8h.
- **0072F060.** This is the inverse of 0072ADC0, reading through the 14h-byte frame reader that 004425C0
  constructs from 009238A0's saved LuaObject.
  - The `fireModeAmmo` words and `fireMode` are read through the reader's vtable +10h, and `fireMode` goes to
    0072D5B0(mode, 1). Every other key is read with 00BD6830.
  - The `loadTime` entries are read and then overwritten with 0.0f (0072F468).
  - `genParam` is read only when 00BD5EB0 reports the key. A non-null result from 00731AE0 replaces `[+41Ch]`,
    after the old object's deleting destructor `vtable[0](1)` runs.
  - The reader's destructor is inlined (vtable 00CE44FC, 00B67700 per LuaObject, then 00BF65AC).
  - `[gun+54h] = [[gun+3F0h]+54h]` runs on every path.

  Uncertainty: the int slot at ESP+10h is reused for every int read and is uninitialised in the image; the C++
  zeroes it. The image's value matters only when the reader leaves the slot unwritten.

## Not reconstructed

- **0072F830 BSP_Gun_SpawnShotAndEffects: skipped.** It is hosted piecewise in cc9's `game_hosts_gunnery.cpp`
  (read-only; see 0072FB35..0072FBA9 and the tube and bomb spawn paths) and modelled in `gun_dispersion` and
  `projectile_impact`. A standalone copy would be a second implementation of a cc9 lane function. Its
  dispersion step (0072FEFB..00730035, per `gun_dispersion.hpp`) was not re-examined for 00BD2F10 draws in this
  packet. It blocks 00730160 `gun_throw_magnitude_0073031d`.
- **00735100 BSP_MissionNarrative_Update: existing, already recorded.**
  `GameScriptOrdersHost::run_narrative_update_00735100` (`src/game_hosts_script_orders.cpp`, a gameplay host)
  already carries the record `existing_implementation_minor_deviation`. No change was made. 005BC920
  `hud_narrative_update` can bind to that host method.
