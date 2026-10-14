# CC13 segment 8 class data and loaders (cc13-seg8)

Packet: segment 8, 00491330..004BA0D0. The partition keywords are soldier types, land-vehicle
classes, wreck class, smoke effects and class tables. This pass took the ship part, plane part
and wreck class families, the soldier entity constructor, and the small singleton and
reference helpers at the end of the segment. Per-frame entity behaviour was left to cc9:
004B18B0 Wreck_TickPhases, 004AFCA0 Soldier_AdvanceAnimationPhase, 004B2760
Wreck_PlaceOnSurface and 004B3480 Wreck_SEntityInit.

Code: `src/classes_seg8_cc13.cpp`, `include/bsp/classes_seg8_cc13.hpp`
(namespace `bsp::classes_seg8_cc13`). Objects are reached through `Raw` byte offsets in their
Win32 layout. Every call the image makes goes through a `Seg8Host` method named for its
target; memset and memcpy are called directly. The registry getters reuse
`bsp::cc13_sys::lazy_singleton_get`. Names are hypotheses, not recovered symbols. Status of
every reconstructed row: reconstructed and Win32 build-tested; no fixture, no original-ABI or
game validation.

## Reconstructed (45)

| Address | C++ | ABI | Body |
|---|---|---|---|
| 004A7120 | plane_part_class_clear_lua_global_004a7120 | cdecl, RET | 006B8AD0("PlanePartClass=nil\n", 0, 0, 2) on the game's Lua machine |
| 004AF930 | soldier_class_clear_lua_global_004af930 | cdecl, RET | the same with "SoldierClass=nil\n" |
| 004B1890 | wreck_class_clear_lua_global_004b1890 | cdecl, RET | the same with "WreckClass=nil\n" |
| 004A7650 | plane_part_refresh_visibility_004a7650 | thiscall, RET | node vtable +38h(part+74h), 00B6DA70(node, +18Ch, 1) |
| 004AC0B0 | ship_part_refresh_visibility_004ac0b0 | thiscall, RET | same |
| 004A7B50 | plane_part_construct_004a7b50 | thiscall(source), RET 4 | 00489F30, vtables, class id 3Fh, +18Ch = 1.0f |
| 004AC870 | ship_part_construct_004ac870 | thiscall(source), RET 4 | same with class id 40h |
| 004AF7E0 | soldier_construct_004af7e0 | thiscall, RET | 00489F30, vtables, class id 3Ah |
| 004B1EE0 | wreck_construct_004b1ee0 | thiscall, RET | 00489F30, vtables, effects 0, class id 3Bh |
| 004A82C0 | plane_part_destruct_004a82c0 | thiscall, RET | stop and release the +188h effect, 00487380 |
| 004ACFA0 | ship_part_destruct_004acfa0 | thiscall, RET | same |
| 004B2590 | wreck_destruct_004b2590 | thiscall, RET | stop +18Ch/+190h, release +190h/+18Ch, 00487380 |
| 004B2680 | wreck_save_archive_004b2680 | thiscall(archive), RET 4 | 00924D90, then wreckclass / _wreck / state / timeLeft |
| 004A7BF0 | plane_part_class_construct_004a7bf0 | thiscall, RET | 00489E60, vtable 00CE6BD4, +5Ch/+60h = 0 |
| 004AC910 | ship_part_class_construct_004ac910 | thiscall, RET | 00489E60, vtable 00CE6F34, defaults 100, 100, 0.5, 1.2, 0..., 1.0 |
| 004B1F70 | wreck_class_construct_004b1f70 | thiscall, RET | 00489E60, vtable 00CE72CC, byte +28h, +34h/+38h = 0 |
| 004A7C10 | plane_part_class_destruct_004a7c10 | thiscall, RET | release +60h, +5Ch, 00489E80 |
| 004AC990 | ship_part_class_destruct_004ac990 | thiscall, RET | release +58h, +54h, 00489E80 |
| 004B1F90 | wreck_class_destruct_004b1f90 | thiscall, RET | release +38h, +34h, 00489E80 |
| 004A7CB0 | plane_part_class_create_instance_004a7cb0 | thiscall, RET | new 190h, memset, 004A7B50 |
| 004ACA30 | ship_part_class_create_instance_004aca30 | thiscall, RET | new 190h, memset, 004AC870 |
| 004A9960 | plane_part_class_load_from_globals_004a9960 | thiscall(name), RET | registry lookup or load from the PlanePartClass Lua table |
| 004AEC00 | ship_part_class_load_from_globals_004aec00 | thiscall(name), RET | the same for ShipPartClass |
| 004AE990 | ship_part_class_registry_construct_004ae990 | thiscall, RET | vtable 00CE6F6C, head node 004AC2F0 |
| 004B3B50 | wreck_class_registry_construct_004b3b50 | thiscall, RET | vtable 00CE734C, head node 004B1DD0 |
| 004B3BC0 | wreck_class_registry_destruct_004b3bc0 | thiscall, RET | 004B3170 erase, delete the head, [00E187F8] = 0 |
| 004AEB30 | ship_part_class_registry_get_004aeb30 | cdecl, RET | lazy getter of 00E187E8 (new 10h, 004AE990) |
| 004B3C50 | wreck_class_registry_get_004b3c50 | cdecl, RET | lazy getter of 00E187F8 (new 10h, 004B3B50) |
| 004A83C0 | native_string_release_storage_004a83c0 | thiscall, RET | sized-pool release |
| 004AD150 | native_string_release_storage_004ad150 | thiscall, RET | same |
| 004B2820 | native_string_release_storage_004b2820 | thiscall, RET | same |
| 004B4E00 | hint_tracker_destruct_singleton_base_004b4e00 | thiscall, RET | [00E198D0] = 0, vtable 00CE3818 |
| 004B4E20 | singleton_00f8a71c_destruct_base_004b4e20 | thiscall, RET | [00F8A71C] = 0 |
| 004B4EB0 | singleton_00e19a64_destruct_base_004b4eb0 | thiscall, RET | [00E19A64] = 0 |
| 004B4ED0 | singleton_00f8998c_destruct_base_004b4ed0 | thiscall, RET | [00F8998C] = 0 |
| 004B4560 | win32_platform_consume_close_request_004b4560 | thiscall, RET | read and clear byte +180h |
| 004B4F40 | entity_id_table_pair_reset_004b4f40 | thiscall, RET | 00951560 on the pair and on pair+54h |
| 004B5670 | participant_record_set_claimed_004b5670 | thiscall(byte), RET 4 | byte +8 |
| 004B5680 | participant_record_set_gate_byte9_004b5680 | thiscall(byte), RET 4 | byte +9 |
| 004B6210 | session_clear_flag_27d_and_set_local_mode_004b6210 | thiscall, RET | 0076FAD0(0) when byte +27Dh, then clear it |
| 004B79F0 | world_class_list_head_004b79f0 | thiscall, RET | world+13Ch |
| 004B81C0 | node_ref_unlink_and_clear_004b81c0 | thiscall, RET | 00B6DFA0 on *ref, clear |
| 004B83B0 | node_ref_unlink_and_clear_004b83b0 | thiscall, RET | same |
| 004B83D0 | native_scene_ref_release_and_clear_004b83d0 | thiscall, RET | 00B72250 on *ref, clear |
| 004B8470 | mission_entity_ref_kill_and_clear_004b8470 | thiscall, RET, EDX input | 00926D90(*ref, caller EDX), clear |

Simplifications and uncertainties (also in the C++ comments and ledger evidence):

- The loaders 004A9960 and 004AEC00 model the three native 14h-byte Lua objects as an opaque
  `LuaObject`, created and destroyed through the host in the native order. 004A9960 inlines the
  004A7BF0 constructor body; the C++ calls that function, which has the same effect.
- A failed allocation in the loaders is not guarded, as in the native, which would then fault
  on the name assignment.
- 004A7CB0 / 004ACA30 memset the allocation before testing it for null, as the native does.
- 004B8470 forwards the caller's EDX, which is not otherwise defined at that point, as
  `edx_in`.
- SEH frames that only matter during unwinding are not modelled: the destructors, the loaders,
  and the 004B3B50 / 004B3BC0 registry bodies.
- Archive values (004B2680) are the native 8-byte `{tag, value}` pairs passed by value: tag 0
  is a string, 1 an int, 2 a float.

## Existing implementations recorded (4)

The class tests 004A7BC0 (3Fh), 004AC8E0 (40h), 004AF840 (3Ah) and 004B1F40 (3Bh) are covered by
`entity_is_kind_of_inherited` in `src/entity_class_ids.cpp`. Each native test returns true for
its own id, 37h, 0, or the entity's +C4h. MINOR: the C++ returns false for 0, or for an id equal
to +C4h, when +C4h is not a class in its table.

## Skipped

- cc9 (per-frame behaviour): 004B18B0, 004AFCA0, 004B2760, 004B3480. 00491950 (ShipIdleActor_Instantiate) not examined.
- Not attempted in this pass (larger loaders and STL bodies): 004A7D30 / 004ACAB0 / 004B21E0
  (ReadLuaFields), 004B2030 (WreckClass_CreateInstance, with landscape placement),
  004A9BD0 / 004AF290 / 004AED50 (global loaders), 004AEA00 (ship part registry destructor),
  004B3D20 / 004B3E60 / 004B3F10, the soldier class 004AF890 / 004AFFE0 / 004B0DE0 /
  004B1400 / 004B1540, 004AB030 and 004A6B90 (SEntityInit bases), and the 96 FUN_ bodies at
  00491330..004A6830, 004AB9A0, 004B0F80 and 004B1880. Those have no exports and were not
  examined.
- 004B4750..004B8490 accessors already named and mentioned by other headers (FormatString*,
  camera and script-depth helpers) were not re-verified.
