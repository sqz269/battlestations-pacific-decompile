# CC13 SEG47: segment 47 (007B2DD0-007D1DC0, flight) without reconstruction records

Packet `cc13-seg47` (cc13 lineage, Claude, 2026-10-16). Pool: the 263 segment-47 functions that
had no reconstruction record (`local/cc13_poolnames.py 47`). Code: `src/cc13_seg47.cpp` and
`include/bsp/cc13_seg47.hpp` (marker `cc13_seg47`), registered in `cmake/startup.cmake`.

Method. Every body was read from the disk bytes with Capstone (`bsp.py disasm-raw`), not from the
Ghidra pseudocode; constants were read from the PE (float vs double kept); x87 compares are
transcribed with their NaN arm (`FCOMI`/`JA` = above and ordered, `JBE` = below-or-equal or
unordered; `COMISS`/`JAE` likewise). Before writing each function the address, its name and its
behaviour were searched in `src`/`include` (`git grep`) and the ledger (`bsp.py lookup`); hits that
were only comments, host virtuals or model inputs did not count as implementations.

Shape. Objects are reached through `Raw` byte offsets into the Win32 layout. Every call the image
makes is a `Seg47Host` method named for its target (address in the name), invoked at the same
point with the same arguments; virtual calls are `*_vcall_<slot>` methods. Nothing is wired,
nothing is ABI-compatible; status is **reconstructed and build-tested** only. Descriptive names are
hypotheses, not recovered symbols.

Simplifications that recur (each also stated at the function):
- x87 products and sums that the image keeps in 80-bit registers before one float store are formed
  in `double` here; a last-bit difference after a double rounding is possible (007B41E0 +38h,
  007B8140, 007B9620, 007BAD70, 007BAE90, 007BB4C0).
- CRT `sqrt` / `atan2` (vendor) are `std::sqrt` / `std::atan2` on the stored floats.
- FLD/FSTP float copies would quiet a signalling NaN; the C++ copies are bitwise.
- Image globals are reached through `host.global_address` / `host.global_pointer`.

## 1. Reconstructed (201)

| Address | C++ | Original ABI (first clause of the comment) |
|---|---|---|
| `007B2DD0` | `owned_points_path_source_load_007b2dd0` | 007B2DD0, slot +1Ch of vtable 00D05440, __thiscall(ECX = source; archive), RET 4. |
| `007B2E40` | `owned_points_path_source_new_007b2e40` | 007B2E40, __fastcall(ECX = out, EDX = arg), EAX = out, SEH frame. |
| `007B2EC0` | `path_source_create_from_archive_kind_007b2ec0` | 007B2EC0, __fastcall(ECX = out, EDX = archive), EAX = out, SEH frame. |
| `007B3000` | `path_source_ref_load_from_archive_007b3000` | 007B3000, __thiscall(ECX = holder; archive), RET 4, SEH frame. |
| `007B3200` | `net_path_construct_007b3200` | 007B3200 BSP_NetPath_Construct, __thiscall(ECX = entity; id), RET 4, EAX = entity. |
| `007B32B0` | `net_path_class_name_007b32b0` | 007B32B0, __thiscall, EAX = 00D05660 "NetPath". |
| `007B32C0` | `net_path_sub170_slot0_007b32c0` | 007B32C0, __thiscall, EAX = 12h. |
| `007B32D0` | `net_path_slot180_noop_007b32d0` | 007B32D0 / 007B32E0: RET 4, no body. |
| `007B32E0` | `net_path_slot18c_noop_007b32e0` | 007B32D0 / 007B32E0: RET 4, no body. |
| `007B3350` | `path_replicator_entity_net_id_007b3350` | 007B3350, __thiscall(ECX = replicator): MOV AX, [[ECX+14h]+174h]. |
| `007B3370` | `net_path_register_in_world_lists_007b3370` | 007B3370, __thiscall(ECX = entity): 00928560, then push_back onto the lists at [entity+30h]+30h and [entity+30h]+384h. |
| `007B33A0` | `net_path_unregister_from_world_lists_007b33a0` | 007B33A0, __thiscall(ECX = entity): 00928570, then for each of the two lists walk node = list+4, node = node+4 to the node whose +8 is the entity and erase it (004837D0); the first absent match falls through to the second. |
| `007B3400` | `net_path_destruct_007b3400` | 007B3400 BSP_NetPath_Destruct, __thiscall(ECX = entity): re-stamps the six vtables (+310h gets 00D0546C then 00D053D8), 007B1A80(entity+310h), then tail-jumps to 0077E380(entity). |
| `007B34B0` | `point3_ptr_array_add_point_007b34b0` | 007B34B0, __thiscall(ECX = array; const float[3]*), RET 4. |
| `007B3730` | `camera_path_load_path_points_007b3730` | 007B3730, __thiscall(ECX = component; property holder), RET 4. |
| `007B38D0` | `scene_path_entity_load_holder_properties_007b38d0` | 007B38D0, __thiscall(ECX = entity): 009277E0(entity); when the holder [entity+C0h] is set and its +4 == 1, entity+54h = find([holder+8], "Party")+0Ch; then 007B34F0(entity+1E4h, [entity+C0h]). |
| `007B3920` | `entity_path_activate_load_points_007b3920` | 007B3920, __thiscall(ECX = entity): 00923840(entity); 007B34F0(entity+170h, [entity+C0h]). |
| `007B3940` | `net_path_activate_load_points_007b3940` | 007B3940, __thiscall(ECX = entity): 0077F0E0(entity); 007B34F0(entity+310h, [entity+C0h]). |
| `007B3960` | `net_path_send_create_message_007b3960` | 007B3960, __thiscall(ECX = entity; peer), RET 4, SEH frame, 120h-byte frame. |
| `007B3A00` | `path_create_message_create_entity_007b3a00` | 007B3A00, __thiscall(ECX = message), EAX, SEH frame. |
| `007B3A80` | `camera_path_activate_007b3a80` | 007B3A80, __thiscall(ECX = entity): 009277E0(entity); 007B3730(entity+1E4h, [entity+C0h]). |
| `007B3BA0` | `pilot_plan_element_destruct_007b3ba0` | 007B3BA0, __thiscall(ECX = element): vptr = 00D056E8 (the base destructor body). |
| `007B3CB0` | `bot_rocket_task_state_block_for_role_007b3cb0` | 007B3CB0, __thiscall(ECX = task), EAX: 007B8AD0([task+3FCh]) ? task+4E4h : task+520h. |
| `007B3CD0` | `bot_observer_holder_destruct_007b3cd0` | 007B3CD0, __thiscall(ECX = holder), SEH frame: vptr 00D056EC; when [holder+14h] is set 006952A0(ECX = [holder+14h], EDX = holder); then 00695870(holder). |
| `007B3D60` | `random_timer_construct_007b3d60` | 007B3D60, __thiscall(ECX = timer; float lo, float hi), RET 8, EAX = timer. |
| `007B3DC0` | `bot_task_state_base_exit_007b3dc0` | 007B3DC0 RET; 007B3DD0 RET 4; 007B3DE0 MOV AL,1; 007B3DF0 RET. |
| `007B3DD0` | `empty_one_arg_virtual_007b3dd0` | 007B3DC0 RET; 007B3DD0 RET 4; 007B3DE0 MOV AL,1; 007B3DF0 RET. |
| `007B3DE0` | `bot_task_state_base_predicate10_007b3de0` | 007B3DC0 RET; 007B3DD0 RET 4; 007B3DE0 MOV AL,1; 007B3DF0 RET. |
| `007B3DF0` | `bot_task_state_base_slot14_007b3df0` | 007B3DC0 RET; 007B3DD0 RET 4; 007B3DE0 MOV AL,1; 007B3DF0 RET. |
| `007B3E20` | `owned_object_holder_destruct_007b3e20` | 007B3E20, __thiscall(ECX = holder): vptr 00D05704; when [holder+4] is set, tail-jumps to its vtable +08h. |
| `007B3F70` | `bot_observer_holder_destruct_full_007b3f70` | 007B3F70, __thiscall(ECX = holder), SEH frame: 007B3CD0(holder+44h), then 004B7EF0(holder+24h). |
| `007B40C0` | `bot_task_base_slot08_007b40c0` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B40E0` | `bot_task_base_name_007b40e0` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B40F0` | `bot_task_base_slot20_007b40f0` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4100` | `bot_task_base_slot28_007b4100` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4120` | `bot_task_base_slot44_007b4120` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4130` | `bot_task_base_slot48_007b4130` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4140` | `bot_task_base_step_result_007b4140` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4150` | `bot_task_base_slot5c_007b4150` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B4160` | `bot_task_base_slot64_007b4160` | 007B40C0 XOR EAX; 007B40E0 "not defined"; 007B40F0 MOV AL,1; 007B4100 XOR AL; 007B4120 XOR AL; 007B4130 XOR EAX; 007B4140 EAX = 1; 007B4150 XOR EAX; 007B4160 RET 4. |
| `007B41E0` | `rocket_approach_construct_base_007b41e0` | 007B41E0, __thiscall(ECX = approach; arg, target), RET 8, EAX = approach, SEH frame. |
| `007B45A0` | `land_state_observer_vector_destruct_007b45a0` | 007B45A0, __thiscall(ECX = vector): 007B4500(vector, 0), then _free([vector]). |
| `007B45E0` | `bot_task_state_base_predicate18_007b45e0` | 007B45E0: XOR AL,AL; RET. |
| `007B4890` | `rocket_approach_redraw_run_speed_007b4890` | 007B4890, __thiscall(ECX = state): +48h = U(0.8, 1.1) * [[state+8]+18Ch] (00BD2F10 at 007B48B2, stream 1). |
| `007B4D90` | `rocket_attack_run_state_construct_007b4d90` | 007B4D90, __thiscall(ECX = state; owner), RET 4, EAX = state, SEH frame. |
| `007B5400` | `rocket_approach_body_aim_offset_007b5400` | 007B5400, __thiscall(ECX = approach; float* out), RET 4, EAX = out: copies +80h..+88h. |
| `007B5980` | `random_timer_step_007b5980` | 007B5980, __thiscall(ECX = timer; float dt), RET 4, AL. |
| `007B5BE0` | `unit_largest_extent_007b5be0` | 007B5BE0, __thiscall(ECX = unit), ST0. |
| `007B5C20` | `bot_condition_destruct_base_007b5c20` | 007B5C20 / 007B5D70 / 007B5E60 (three identical bodies), __thiscall(ECX = condition), SEH frame: 0042BCA0(condition+4), then vptr 00D05840. |
| `007B5D70` | `bot_condition_destruct_base_007b5d70` | 007B5C20 / 007B5D70 / 007B5E60 (three identical bodies), __thiscall(ECX = condition), SEH frame: 0042BCA0(condition+4), then vptr 00D05840. |
| `007B5E60` | `bot_condition_destruct_base_007b5e60` | 007B5C20 / 007B5D70 / 007B5E60 (three identical bodies), __thiscall(ECX = condition), SEH frame: 0042BCA0(condition+4), then vptr 00D05840. |
| `007B5C70` | `bot_condition_construct_base_007b5c70` | 007B5C70, __thiscall(ECX = condition; value), RET 4, EAX = condition, SEH frame. |
| `007B5CD0` | `bot_condition_deleting_dtor_007b5cd0` | 007B5CD0 / 007B5DC0 / 007B5EB0 (identical), __thiscall(ECX = condition; flags), RET 4, EAX = condition: the 007B5C20 body, then operator delete (00BF65AC) when flags & 1. |
| `007B5DC0` | `bot_condition_point_off_nose_deleting_dtor_007b5dc0` | 007B5CD0 / 007B5DC0 / 007B5EB0 (identical), __thiscall(ECX = condition; flags), RET 4, EAX = condition: the 007B5C20 body, then operator delete (00BF65AC) when flags & 1. |
| `007B5EB0` | `bot_condition_point_distance_deleting_dtor_007b5eb0` | 007B5CD0 / 007B5DC0 / 007B5EB0 (identical), __thiscall(ECX = condition; flags), RET 4, EAX = condition: the 007B5C20 body, then operator delete (00BF65AC) when flags & 1. |
| `007B5D30` | `bot_condition_point_off_nose_construct_007b5d30` | 007B5D30, __thiscall(ECX = condition; value, float a, b, c), RET 10h, EAX. |
| `007B5E20` | `bot_condition_point_distance_construct_007b5e20` | 007B5E20, __thiscall(ECX = condition; value, const float[3]*, float d), RET 0Ch, EAX. |
| `007B6680` | `rocket_approach_destroy_states_007b6680` | 007B6680, __thiscall(ECX = approach), SEH frame: the member destructors in reverse construction order: 007B45F0 on +2ECh, +29Ch, +27Ch, +258h; 006CDD70(+1C0h+18h), 007B45F0(+1C0h); 006CDD70(+128h+18h), 007B45F0(+128h); 007B65E0(+ECh); 004116D0(approach ? approach+D8h : null); 00695870(+58h). |
| `007B6830` | `rocket_approach_construct_007b6830` | 007B6830, __thiscall(ECX = approach; task, target), RET 8, EAX = approach, SEH frame. |
| `007B6CD0` | `bot_task_rocket_command_class_007b6cd0` | 007B6CD0, EAX = 00E08F48 (the rocket command class); 007B6CF0, EAX = 00CFA6B4 "Rocket". |
| `007B6CF0` | `bot_task_rocket_name_007b6cf0` | 007B6CD0, EAX = 00E08F48 (the rocket command class); 007B6CF0, EAX = 00CFA6B4 "Rocket". |
| `007B6EB0` | `ref_holder_assign_007b6eb0` | 007B6EB0, __thiscall(ECX = holder; state), RET 4: exit the current state (+4, vtable +08h) when set, store the new one and enter it (vtable +04h, no null test and no same-state test). |
| `007B6EE0` | `state_machine_set_current_state_007b6ee0` | 007B6EE0, __thiscall(ECX = machine; state), RET 4: when state != +4: exit the current one (when set), store, enter the new one (no null test). |
| `007B6F50` | `bot_task_rocket_set_state_007b6f50` | 007B6F50, __thiscall(ECX = task; state), RET 4: the 007B6EE0 shape on task+310h. |
| `007B7BA0` | `bot_task_rocket_stop_007b7ba0` | 007B7BA0, __thiscall(ECX = task): exit the current state (+310h, vtable +08h; +310h is not cleared); copy the global vector [00F87574..7Ch]; when +3C4h is set, 006952A0(ECX = +3C4h, EDX = task+3B0h) and +3C4h = 0; +3A4h..+3ACh = the copied vector; +3C8h = 0. |
| `007B7E10` | `bot_task_rocket_destruct_007b7e10` | 007B7E10, __thiscall(ECX = task), SEH frame: 007B6680(task ? task+3F8h : null), then 007B4030(task). |
| `007B7E70` | `bot_task_build_debug_string_007b7e70` | 007B7E70, __cdecl(string* out, const string& base, const char* suffix), EAX = out. |
| `007B7F20` | `bot_task_rocket_debug_name_007b7f20` | 007B7F20, __thiscall(ECX = task; string* out), RET 4, EAX = out, SEH frame. |
| `007B7FD0` | `bot_task_make_rocket_007b7fd0` | 007B7FD0, __fastcall(ECX = a, EDX = b), EAX, SEH frame: operator new(708h) (00BF681B); non-null -> 007B6A40(memory, a, b), else 0. |
| `007B80A0` | `target_flag_e4_set_or_global_007b80a0` | 007B80A0, __thiscall(ECX = target), EAX: 1 when +E4h is set or [00F874FD] is set. |
| `007B80C0` | `target_flag_e4_clear_or_global_007b80c0` | 007B80C0, __thiscall(ECX = target), EAX: 1 when +E4h is clear or [00F874FD] is set. |
| `007B8140` | `float_window_set_sample_and_sum_007b8140` | 007B8140, __thiscall(ECX = window; index, count, float sample), RET 0Ch. |
| `007B8450` | `plane_clear_flag_904_007b8450` | 007B8450, __thiscall(ECX = plane): when +904h is set, clear it. |
| `007B8550` | `index_block_decrement_all_007b8550` | 007B8550, __thiscall(ECX = block): decrements the fourteen dwords +18h, +1Ch, +20h, +24h, +70h, +28h, +3Ch, +40h, +44h, +54h, +58h, +5Ch, +6Ch, +74h, then rewinds each one that fell below -2 (signed JGE). |
| `007B8AA0` | `plane_set_turbo_byte_007b8aa0` | 007B8AA0, __thiscall(ECX = plane; char), RET 4: +9F9h = the byte. |
| `007B8C20` | `plane_is_cockpit_view_active_007b8c20` | 007B8C20, __thiscall(ECX = plane), AL = +A9Dh. |
| `007B8C30` | `plane_set_control_enabled_007b8c30` | 007B8C30, __thiscall(ECX = plane; bool enabled), RET 4. |
| `007B8CE0` | `plane_set_gear_mode_007b8ce0` | 007B8CE0, __thiscall(ECX = plane; mode, flag), RET 8. |
| `007B8D70` | `plane_gear_is_down_007b8d70` | 007B8D70, __thiscall(ECX = plane), EAX: 1 unless the gear channel ([plane+DECh]+28h) is enabled and its value is not 1.0f (UCOMISS: a NaN value answers 0 when enabled). |
| `007B8E70` | `plane_get_float_b2c_007b8e70` | 007B8E70, __thiscall(ECX = plane), ST0 = +B2Ch. |
| `007B8EF0` | `plane_lose_power_007b8ef0` | 007B8EF0, __thiscall(ECX = plane): 007DE600(plane+C98h); +C39h = 1. |
| `007B9080` | `plane_control_mode_speed_above_class_19c_007b9080` | 007B9080, __thiscall(ECX = control mode at plane+72Ch), EAX: 1 when plane->vtable[38h]() (the speed) > [plane+538h]+19Ch (FCOMI/JBE: a NaN answers 0). |
| `007B9100` | `weapon_controller_first_bomb_class_007b9100` | 007B9100 and 007B9510 (identical bodies), __thiscall(ECX = plane), EAX: the first non-null rack->vtable[218h](2Ah), else 0. |
| `007B9510` | `weapon_controller_first_bomb_class_007b9510` | 007B9100 and 007B9510 (identical bodies), __thiscall(ECX = plane), EAX: the first non-null rack->vtable[218h](2Ah), else 0. |
| `007B91C0` | `weapon_controller_has_ordnance_of_kind_007b91c0` | 007B91C0, __thiscall(ECX = plane; kind, loadout), RET 8, AL: some rack whose descriptor rack->vtable[220h](loadout) is non-null answers descriptor->vtable[08h](kind). |
| `007B9230` | `weapon_controller_first_slot_descriptor_007b9230` | 007B9230, __thiscall(ECX = plane), EAX: racks > 0 ? rack0->vtable[220h](1) : 0. |
| `007B92E0` | `weapon_controller_any_slot_answers_20c_007b92e0` | 007B92E0, __thiscall(ECX = plane), AL: some rack->vtable[20Ch]() answers. |
| `007B9320` | `weapon_controller_has_general_bomb_ordnance_007b9320` | 007B9320, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor is kind 2Ah and none of 2Ch, 31h, 2Bh, 33h, 2Dh (asked in that order, each a vtable[08h] call; the first answer that rules the rack out moves on). |
| `007B93E0` | `weapon_controller_has_drop_kamikaze_ordnance_007b93e0` | 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX = plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout). |
| `007B93F0` | `weapon_controller_has_torpedo_ordnance_007b93f0` | 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX = plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout). |
| `007B9400` | `weapon_controller_has_rocket_kind33_007b9400` | 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX = plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout). |
| `007B94F0` | `weapon_controller_has_depth_charge_ordnance_007b94f0` | 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX = plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout). |
| `007B9500` | `weapon_controller_has_level_bomb_ordnance_007b9500` | 007B93E0 / 007B93F0 / 007B9400 / 007B94F0 / 007B9500, __thiscall(ECX = plane; loadout), RET 4, AL: 007B91C0(plane; 2Fh / 2Bh / 33h / 2Ch / 31h, loadout). |
| `007B9410` | `weapon_controller_find_ready_rocket_007b9410` | 007B9410, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor is kind 33h with 0.0f >= descriptor+E0h (COMISS/JAE: a NaN does not answer). |
| `007B9480` | `weapon_controller_has_rocket_ordnance_007b9480` | 007B9480, __thiscall(ECX = plane; loadout), RET 4, AL: some rack descriptor is kind 33h with !(0.0f >= descriptor+E0h) (COMISS/JB: a NaN answers). |
| `007B9590` | `unit_instance_retire_ai_sub_nodes_007b9590` | 007B9590, __thiscall(ECX = plane): +520h = 0; when +DF4h is set, its vtable[10h]() and +DF4h = 0; the same for +6DCh; then tail-jumps to 00951FB0(plane). |
| `007B9620` | `plane_altitude_sink_predicate_007b9620` | 007B9620, __thiscall(ECX = plane), AL. |
| `007B96C0` | `plane_detach_into_hangar_007b96c0` | 007B96C0, __thiscall(ECX = plane): +C00h = 1; 00951F40(plane, 0). |
| `007B96F0` | `unit_find_gun_target_007b96f0` | 007B96F0, __thiscall(ECX = unit; f0, f1, f2, f3, f4, int n, float f6), RET 1Ch, EAX: [unit+C50h] null -> 0, else 007E2090(finder; the seven arguments, in order). |
| `007B9740` | `plane_neighbours_clear_target_007b9740` | 007B9740, __thiscall(ECX = unit): when [unit+C50h] is set, tail-jumps to 007E2070(finder). |
| `007B97E0` | `plane_get_squadron_007b97e0` | 007B97E0, __thiscall(ECX = plane), EAX = +9D4h (the squadron). |
| `007B98A0` | `firing_gun_registry_destruct_unwind_007b98a0` | 007B98A0, __thiscall(ECX = registry): [00F87294] = 0; vptr 00CE3818. |
| `007BA020` | `matrix_multiply_in_place_007ba020` | 007BA020, __thiscall(ECX = matrix; const matrix*), RET 4, EAX = matrix: product = 00413920(matrix; &stack 40h, other); 004134F0(matrix; product). |
| `007BA170` | `session_message_enginefire_construct_007ba170` | 007BA170 ENGINEFIREFAILURE, __thiscall(ECX = message; int index, float value), RET 8, EAX: type 65h; +24h = index; +4 = 1; vptr 00D02CF4; +20h = value. |
| `007BA1C0` | `session_message_engineslost_construct_007ba1c0` | 007BA1C0 ENGINESLOSTFAILURE, __thiscall(ECX = message; float), RET 4, EAX: type 66h; +4 = 1; vptr 00D02F94; +20h = value. |
| `007BA200` | `session_message_spinning_construct_007ba200` | 007BA200 SPINNINGFAILURE, __thiscall(ECX = message; char side, float), RET 8, EAX: type 67h; +24h = side; +4 = 1; vptr 00D02D08; +20h = value. |
| `007BA340` | `landscape_height_at_007ba340` | 007BA340, __thiscall(ECX = game; float x, float z), RET 8, ST0. |
| `007BA3E0` | `unit_is_gun_of_subtype156_007ba3e0` | 007BA3E0, __thiscall(ECX = unit), EAX: unit->vtable[5Ch](20h) and [[unit+3F4h]+80h] is 1, 5 or 6. |
| `007BA450` | `max3_007ba450` | 007BA450, __fastcall(ECX = const float* a, EDX = const float* b; const float* c), RET 4, ST0. |
| `007BA600` | `plane_store_message_vector_f0_007ba600` | 007BA600, __thiscall(ECX = plane; const float[3]*), RET 4: +F0h..+F8h = the vector. |
| `007BA760` | `unit_gun_fire_blocked_007ba760` | 007BA760, __thiscall(ECX = unit), EAX: 1 when +C3Ah or +5Dh is set. |
| `007BA7B0` | `plane_state_record_init_defaults_007ba7b0` | 007BA7B0, __thiscall(ECX = record): +2Ch = 10, +38h = 0, +30h = 3, +34h = 1; -1 into +28h, +24h, +20h, +1Ch, +18h, +44h, +40h, +3Ch, +5Ch, +58h, +54h, +74h, +70h, +6Ch; +78h..+80h = [00F87574..7Ch]; +60h..+68h = +78h..+80h; +48h..+50h = +60h..+68h. |
| `007BA890` | `session_message_resolve_entity_by_net_id_007ba890` | 007BA890, __thiscall(ECX = message), EAX: id = (uint16)+20h; id < [00F89A10] (signed) -> [[00F89A54] + (id - [00F89A0C])*10h + 0Ch], else [[00F89AA8] + (id - [00F89A60])*10h + 0Ch]. |
| `007BA8D0` | `session_message_plane_deadmeatrot_construct_007ba8d0` | 007BA8D0 PLANE_DEADMEATROT, __thiscall(ECX = message; plane), RET 4, EAX: type C8h; +4 = 1; vptr 00D03540; +20h..+28h = plane+BA0h..+BA8h. |
| `007BA950` | `session_message_plane_controlfactor_construct_007ba950` | 007BA950 PLANE_CONTROLFACTOR, __thiscall(ECX = message; seven dwords by value), RET 1Ch, EAX: 0075B430(C9h); REP MOVSD of the seven argument dwords to +20h..+38h; then the head bytes, +4 = 1, vptr 00D02E20. |
| `007BA990` | `session_message_plane_hit_construct_007ba990` | 007BA990 PLANE_HIT, __thiscall(ECX = message; int a, float b, c, d, const float[3]*), RET 14h, EAX: type CAh; +20h = a; +4 = 1; vptr 00D02E34; +24h/+28h/+2Ch = b, c, d; +30h..+38h = the vector. |
| `007BAA80` | `plane_state_record_copy_007baa80` | 007BAA80 and 007BAAF0 (identical), __thiscall(ECX = destination; const record*), RET 4, EAX = destination. |
| `007BAAF0` | `plane_control_state_copy_007baaf0` | 007BAA80 and 007BAAF0 (identical), __thiscall(ECX = destination; const record*), RET 4, EAX = destination. |
| `007BAC50` | `map_overlay_draw_ellipse_thunk_007bac50` | 007BAC50, RET 8 (ECX unused; unreferenced): v = {r, r}; 0093E9A0(overlay, &v) (__cdecl, two arguments). |
| `007BAC80` | `plane_hide_model_007bac80` | 007BAC80, __thiscall(ECX = node): 00B6DA70(node; 0.0f, 0); then node->vtable[2Ch](&{100000.0f, 100000.0f, 100000.0f}). |
| `007BAD00` | `plane_contact_pair_ram_007bad00` | 007BAD00, RET 4 (ECX unused): the contact pair callback. |
| `007BAD70` | `vector3_place_at_unit_distance_007bad70` | 007BAD70, __fastcall(ECX = float a[3] (in/out), EDX = const float b[3]). |
| `007BAE90` | `rate_limiter_step_007bae90` | 007BAE90, __thiscall(ECX = limiter; float target, float dt), RET 8. |
| `007BAF90` | `plane_unit_instance_init_control_block_007baf90` | 007BAF90, __thiscall(ECX = block; arg), RET 4, EAX = block. |
| `007BB060` | `plane_class_activate_resources_007bb060` | 007BB060, __thiscall(ECX = class; char force), RET 4, SEH frame. |
| `007BB110` | `unit_can_release_ordnance_007bb110` | 007BB110, __thiscall(ECX = unit), AL. |
| `007BB150` | `plane_publish_slot_byte_df0_007bb150` | 007BB150, __thiscall(ECX = plane; char flag), RET 4: +DF0h = flag ? [plane + (uint16)[00F876B8]*8 + 9C3h] : 0. |
| `007BB170` | `plane_set_pilot_command_last_time_007bb170` | 007BB170, __thiscall(ECX = plane; int slot, float time), RET 8, SEH frame. |
| `007BB2C0` | `plane_refresh_ground_contact_007bb2c0` | 007BB2C0, __thiscall(ECX = plane): when +C02h and +BF4h are set: the world-pose refresh 00414DB0(plane) when +C8h is clear, then +BF8h = 006BC530([plane+BF4h]; plane+FCh, plane+BFCh). |
| `007BB310` | `plane_update_cockpit_transform_007bb310` | 007BB310, __thiscall(ECX = plane). |
| `007BB4C0` | `triangle_vertex_distance_sum_007bb4c0` | 007BB4C0, __fastcall(ECX = const float a[], EDX = const float b[]), ST0. |
| `007BBA10` | `plane_on_impact_message_007bba10` | 007BBA10, __thiscall(ECX = plane; const message*), RET 4. |
| `007BBB70` | `unit_copy_vector_ac8_007bbb70` | 007BBB70, __thiscall(ECX = plane; float* out), RET 4: out = +AC8h..+AD0h. |
| `007BBC10` | `unit_pilot_target_is_007bbc10` | 007BBC10, __thiscall(ECX = plane; target), RET 4, EAX: +DF4h set and [[+DF4h] + (uint16)[00F876B8]*1Ch + 98h] == target. |
| `007BBC50` | `plane_is_grounded_on_owned_site_007bbc50` | 007BBC50, __thiscall(ECX = plane), AL. |
| `007BC550` | `plane_sync_elevator_attachment_for_state_007bc550` | 007BC550, __thiscall(ECX = plane). |
| `007BC610` | `plane_enter_cockpit_view_007bc610` | 007BC610, __thiscall(ECX = plane). |
| `007BC690` | `plane_leave_cockpit_view_007bc690` | 007BC690, __thiscall(ECX = plane). |
| `007BC8C0` | `weapon_controller_total_bomb_rounds_007bc8c0` | 007BC8C0, __thiscall(ECX = plane), ST0: a = sum of 006E3500(rack), b = sum of 006E3520(rack) over the +994h racks; result = (float)(a / (b > 1 ? b : 1)) (FILD/FIDIV; with no racks the divisor is 1 and a is 0). |
| `007BCA50` | `plane_ordnance_icon_kind_007bca50` | 007BCA50, __thiscall(ECX = plane), EAX: +96Ch when set; otherwise the first child answering vtable[5Ch](20h) with [[child+3F4h]+80h] == 0Ah gives [[[child+3F8h]+34h]+8]; none -> 0. |
| `007BCB30` | `plane_re_equip_007bcb30` | 007BCB30, __thiscall(ECX = plane; class id), RET 4. |
| `007BCC20` | `plane_formation_wait_value_007bcc20` | 007BCC20, __thiscall(ECX = plane), ST0: -1.0f when +5Eh or +5Dh is set or +9D4h, +9D0h or +DF4h is null; 0.0f when (plane+72Ch)->vtable[38h]() is false; otherwise tail-jumps to 00999AE0([plane+DF4h]). |
| `007BCCF0` | `plane_ordnance_release_point_world_007bccf0` | 007BCCF0, __thiscall(ECX = plane; float* out), RET 4, EAX = out (the level-bomb task's paratrooper branch, 009B7D7D). |
| `007BD120` | `weapon_controller_rocket_ignition_lift_007bd120` | 007BD120, __thiscall(ECX = plane), ST0: the first rack whose bullet class [[rack+3F8h]+34h] has +8 == 12h tail-jumps to 00809DE0(class); none -> 0.0. |
| `007BD1A0` | `plane_create_child_gun_ai_bots_007bd1a0` | 007BD1A0, __thiscall(ECX = plane): 0072C6A0(child) for every child gun of subtype 1, 5 or 6 (vtable[5Ch](20h), [[child+3F4h]+80h]). |
| `007BD1F0` | `plane_has_child_gun_of_subtype156_007bd1f0` | 007BD1F0, __thiscall(ECX = plane), AL: some child is such a gun. |
| `007BD240` | `plane_build_replication_delta_007bd240` | 007BD240, __thiscall(ECX = plane; record** arg), RET 4. |
| `007BD3E0` | `session_message_plane_create_construct_007bd3e0` | 007BD3E0 PLANE_CREATE, __thiscall(ECX = message; a1, plane, reference entity, a4, a5, float f6, f7, f8, char c9, a10), RET 28h, EAX, SEH frame. |
| `007C7750` | `plane_level_bomb_attitude_gate_007c7750` | 007C7750, __thiscall(ECX = plane; char check_doors), RET 4, AL: the level-bomb attitude gate. |
| `007BD600` | `pod_array64_reserve_007bd600` | 007BD600, __thiscall(ECX = array; count), RET 4: 40h-byte elements. |
| `007BD940` | `pod_array4_reserve_007bd940` | 007BD940, __thiscall(ECX = array; count), RET 4: the 007BD600 shape with 4-byte elements. |
| `007C2DB0` | `pod_array64_push_back_007c2db0` | 007C2DB0, __thiscall(ECX = array; const element*), RET 4: when size == capacity, 007BD600(array, max(capacity*2, 1)) (JG 1); the 40h bytes go to data + size*40h unless that address is null; size += 1. |
| `007C2F80` | `pod_array64_clear_007c2f80` | 007C2F80, __thiscall(ECX = array): capacity < 0 -> 007BD600(array, 0); the size counts down to 0 one step at a time (no element work), then size = 0. |
| `007C3FC0` | `pod_array64_destruct_007c3fc0` | 007C3FC0, __thiscall(ECX = array): the 007C2F80 body, then size = 0 and _free(data). |
| `007C90D0` | `ref_ptr_vector_pop_back_007c90d0` | 007C90D0, __thiscall(ECX = array of refcounted pointers): when size is non-zero, the last pointer p: when p is set, InterlockedDecrement(p+4) == 0 -> p->vtable[0]() and the slot = 0; then size -= 1. |
| `007C9150` | `pod_array4_destruct_007c9150` | 007C9150, __thiscall(ECX = array): 007C3000(array, 0), then _free(data). |
| `007CD2C0` | `plane_engine_effect_vector_destruct_007cd2c0` | 007CD2C0, __thiscall(ECX = vector): 007C9290(vector, 0), then _free(data). |
| `007BDB80` | `entity_cast_to_mrfs_gun_007bdb80` | 007BDB80, __thiscall(ECX = entity), EAX: entity when non-null and vtable[5Ch](21h) answers, else 0. |
| `007BDBA0` | `entity_world_matrix_refreshed_007bdba0` | 007BDBA0, __thiscall(ECX = entity), EAX = entity+CCh after the pose refresh 00414DB0 when +C8h is clear. |
| `007BDCB0` | `firing_gun_registry_construct_007bdcb0` | 007BDCB0, __thiscall(ECX = registry), EAX, SEH frame: vptr 00D05A80; +4 = 00BD1860() (BSP_CriticalSection_Create). |
| `007BDF50` | `plane_control_state_init_identity_007bdf50` | 007BDF50, __thiscall(ECX = state; float out[16]), RET 4, EAX = out: out = identity; 00467050(out; +20h, +24h, +28h) (the rotation from three angles); out+30h..+38h = state+2Ch..+34h. |
| `007C1D80` | `plane_reload_bomb_platforms_007c1d80` | 007C1D80, __thiscall(ECX = plane): child->vtable[200h]() for every child answering vtable[5Ch](25h) (the bomb platforms). |
| `007C1DB0` | `unit_count_remaining_ordnance_rounds_007c1db0` | 007C1DB0, __thiscall(ECX = plane), EAX: the sum of 006E3500(child) over the kind-25h children. |
| `007C1DE0` | `plane_rocket_round_count_007c1de0` | 007C1DE0, __thiscall(ECX = plane), EAX: the sum of child->vtable[21Ch](33h) over the kind-25h children. |
| `007C1F60` | `unit_count_droppable_rounds_007c1f60` | 007C1F60, __thiscall(ECX = plane), EAX: the sum of 006E3500(child) over the kind-25h children answering child->vtable[210h](2Ah, 0). |
| `007C2050` | `plane_slot_d4_forward_to_df4_007c2050` | 007C2050, __thiscall(ECX = plane; record* out), RET 4, EAX = out. |
| `007C2810` | `plane_state_record_get_or_create_007c2810` | 007C2810, RET 4 (ECX unused), EAX: holder null -> 0. |
| `007C2AA0` | `plane_get_heading_angle_007c2aa0` | 007C2AA0, __thiscall(ECX = plane), ST0: the pose refresh when +C8h is clear; -(float)atan2([plane+ECh], [plane+F4h]) (00BF701A _CIatan2 on the two stored floats; the CRT result is rounded to float before the FCHS). |
| `007C2EC0` | `firing_gun_registry_instance_007c2ec0` | 007C2EC0, EAX: the firing-gun registry singleton [00F87294]. |
| `007C33B0` | `firing_gun_registry_destruct_007c33b0` | 007C33B0, __thiscall(ECX = registry): vptr 00D05A80; 0041CC80(registry+4); [00F87294] = 0; vptr 00CE3818. |
| `007C3420` | `transform_record_init_identity_007c3420` | 007C3420, __thiscall(ECX = record), EAX = record: +0 = 0; 004134F0(record+4, stack identity). |
| `007C34B0` | `plane_engine_effect_record_copy_construct_007c34b0` | 007C34B0, __thiscall(ECX = record; const record*), RET 4, EAX, SEH frame: +0 = 0; when source+0 is set, +0 = it and InterlockedIncrement(it+4); 004134F0(record+4, source+4). |
| `007C3510` | `plane_engine_effect_construct_007c3510` | 007C3510, __thiscall(ECX = record; RefPtr effect by value), RET 4, EAX, SEH frame: +0 = 0; when effect is set, +0 = effect and InterlockedIncrement; 004134F0(record+4, stack identity); then the by-value parameter is released (InterlockedDecrement == 0 -> effect->vtable[0]()). |
| `007C3600` | `ref_ptr_release_007c3600` | 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical), __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0. |
| `007C3640` | `ref_ptr_release_007c3640` | 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical), __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0. |
| `007C3680` | `ref_ptr_release_007c3680` | 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical), __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0. |
| `007C94E0` | `ref_ptr_release_007c94e0` | 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical), __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0. |
| `007C9510` | `ref_ptr_release_007c9510` | 007C3600 / 007C3640 / 007C3680 / 007C94E0 / 007C9510 (identical), __thiscall(ECX = holder): the intrusive release of [holder], then [holder] = 0. |
| `007C3F60` | `weapon_controller_first_slot_ordnance_007c3f60` | 007C3F60, __thiscall(ECX = plane), EAX: the first rack whose vtable[220h](0) descriptor is non-null decides: its +14h, or 00F872D5 when that is null; no such rack -> 0. |
| `007C4470` | `effect_vector_construct_007c4470` | 007C4470, __thiscall(ECX = vector), EAX, SEH frame: vptr 00D05AB8, +4..+10h = 0, 007DDDE0(+14h), 007DDDE0(+20h). |
| `007C6F20` | `record_destruct_string_3c_007c6f20` | 007C6F20, __thiscall(ECX = record): the inline std::string tidy of the string at +38h (+3Ch buffer/pointer, +4Ch size, +50h capacity): capacity >= 10h -> operator delete(+3Ch); capacity = 0Fh, size = 0, buffer[0] = 0. |
| `007C0C60` | `plane_slot4c_point_event_007c0c60` | 007C0C60, plane vtable +4Ch, __thiscall(ECX = plane; float a[3], b[3], c[3], float d by value), RET 28h: 007BE9B0(plane+84Ch; the ten words), then 00924610(plane; the same ten words). |
| `007C3CB0` | `plane_step_countdown_964_007c3cb0` | 007C3CB0, __thiscall(ECX = plane; float dt), RET 4. |
| `007C3EA0` | `plane_issue_land_on_touchdown_007c3ea0` | 007C3EA0, __thiscall(ECX = plane), SEH frame. |
| `007C7430` | `plane_remote_touchdown_to_state4_007c7430` | 007C7430, __thiscall(ECX = plane). |
| `007C7800` | `plane_sync_cockpit_camera_gun_shake_007c7800` | 007C7800, __thiscall(ECX = plane; camera), RET 4. |
| `007CB9E0` | `plane_remote_enter_flight_state6_007cb9e0` | 007CB9E0, __thiscall(ECX = plane). |
| `007CC2D0` | `plane_control_mode_on_water_contact_007cc2d0` | 007CC2D0, __thiscall(ECX = control mode at plane+72Ch; arg), RET 4: 006E6450(mode; arg), then 007CB7F0(plane). |
| `007CD250` | `plane_append_flight_state_debug_007cd250` | 007CD250, __thiscall(ECX = string), EAX = string, SEH frame: a stack NativeString {length, data} from 007CCB30(&local); 00BD1A20(string; it) (append); the local's data returned to the sized pool with length + 1. |
| `007CC9B0` | `session_message_plane_create_create_entity_007cc9b0` | 007CC9B0 PLANE_CREATE create entity, __thiscall(ECX = message), EAX. |
| `007CD9A0` | `plane_bay_door_rack_holds_bomb_007cd9a0` | 007CD9A0, __thiscall(ECX = plane), AL. |
| `007CFCF0` | `plane_class_slot1c_007cfcf0` | 007CFCF0, __thiscall, EAX = 0Fh (the plane class id). |
| `007D0350` | `plane_get_field_93c_007d0350` | 007D0350, __thiscall(ECX = plane), EAX = +93Ch. |
| `007D1C90` | `plane_unit_instance_create_007d1c90` | 007D1C90, __thiscall(ECX = class; arg), RET 4, EAX, SEH frame: operator new (E94h) (00BF55BE), memset(0, E94h) before the null test, then 007CFD20(memory; arg, class); null -> 0. |
| `007C4830` | `plane_class_stall_range_speed_007c4830` | 007C4830, __thiscall(ECX = class), ST0: (float)(tuning+230h * class+184h) (0042E740; one x87 product stored to a float). |
| `007C4D90` | `plane_in_paratrooper_drop_altitude_007c4d90` | 007C4D90, __thiscall(ECX = plane), AL: alt = +9B4h; tuning+354h > alt -> 0; alt > tuning+358h -> 0 (FCOMI/JA both: a NaN passes); else 1. |
| `007C9170` | `plane_engine_effect_vector_reserve_007c9170` | 007C9170, __thiscall(ECX = vector; count), RET 4, SEH frame. |
| `007C9290` | `plane_engine_effect_vector_resize_007c9290` | 007C9290, __thiscall(ECX = vector; count), RET 4. |
| `007CCCA0` | `plane_engine_effect_vector_push_back_007ccca0` | 007CCCA0, __thiscall(ECX = vector; const record*), RET 4, SEH frame: when size == capacity, 007C9170(max(capacity*2, 1)); the record is copy-constructed (007C34B0) at data + size*44h unless null; size += 1. |
| `007CD520` | `plane_fx_component_construct_007cd520` | 007CD520, __thiscall(ECX = component), EAX, SEH frame: vptr 00D05E94; +8/+0Ch/+10h = 0; 007DDDE0(+18h); 007DDE00(+24h); 007C4470 on +48h, +74h, +A0h and +CCh; +F8h/+FCh/+100h = 0. |
| `007CDC70` | `plane_publish_step_record_007cdc70` | 007CDC70, __thiscall(ECX = plane): the per-slot step record at plane + (uint16)[00E0B6CCh]*8 + 9C0h (the slot word is re-read before each store): +9C1h = [plane+DECh] set and (channel +60h disabled or its value == 1.0f, UCOMISS: NaN -> 0); +9C0h = 007B9140(plane; 0); +9C2h = plane+520h; +9C4h = 007B |

## 2. Already implemented: verified and recorded ledger-only (13)

| Address | Status | Source | What was checked |
|---|---|---|---|
| `007B3320` | existing_implementation_verified_against_listing | `src/unit_kind_query.cpp` | table row {0x49, 0x007b3320, vtable 00D054D0, kAccept49 = {0,1,2,49h}} plus the dynamic +C4h test in unit_kind_body_answers; matches 007B3320-007B3349 (49h, 2, 1, 0, then [ECX+C4h]) |
| `007B8840` | existing_implementation_verified_against_listing | `src/band_007a_cc13.cpp` | 007B8840 is the set-target shape on the +60h channel: target byte +61h compared, stored with dirty +68h = 1, then dirty -> block +11h = 1; band_007a_cc13's generic helper (named for 007B8DA0) reproduces it |
| `007B8D40` | existing_implementation_verified_against_listing | `src/band_007a_cc13.cpp` | 007B8D40: channel [plane+DECh]+44h disabled -> 1, else UCOMISS value vs [00D7A218] = 0.0f with TEST AH,44h / JNP -> 1 only when equal and ordered; the band_007a helper's `value == wanted` matches including NaN |
| `007B8DC0` | existing_implementation_verified_against_listing | `src/band_007a_cc13.cpp` | 007B8DC0: channel C (+44h) target byte +45h set to 1 with dirty +4Ch, then block +11h; the band_007a helper with target 1 |
| `007B58D0` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | binary: [ECX+369h] != 0 && [00E17BF2] != 0; the host override (game_hosts_units.cpp:25283) returns only the global, ctl+369h LABELLED as set. Adoption: read ctl+369h |
| `007BA2E0` | existing_implementation_verified_against_listing | `src/game_hosts_units.cpp` | binary: a = (float)(1.5707963705062866 - h); 0 > a (FCOMI/JBE) -> a = (float)(a + 6.2831854820251465); out = ((float)cos a, 0, (float)sin a). The host lambda computes the same values (it returns dx/dz and the caller supplies y) |
| `007BC7A0` | existing_implementation_minor_deviation | `src/game_hosts_units.cpp` | binary: 007B9510 first rack; when it answers vtable[5Ch](33h) and 0.0f >= [rack+314h]+E0h (COMISS/JAE) -> 1; else pitch +C64h > -0.8726646304130554 (double) requires |bank +C68h| <= max(0, (float)(1.3962634801864624 - |pitch|)). The host carries the attitude part for bombs and omits the rocket pass; its comment states the rocket pass as +E0h > 0, the binary passes when +E0h <= 0 |
| `007BCE20` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | binary: m = FLT_MAX; for each rack whose bullet class [[rack+3F8h]+34h]+8 == 0Ah: v = class+DCh, !(v > m) (FCOMI/JA) -> m = v. The host walks the gunnery rows instead of the racks and uses `v <= best`, so a NaN v does not replace m there (it does in the image) |
| `007BCFA0` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | the 007BCE20 body on class+E4h (007BCFF1 FLD [EAX+E4h]); host torpedo_device_min_007bce20(slot, true), same NaN note |
| `007C1FB0` | existing_host_substitution_mismatch | `src/game_hosts_units.cpp` | binary: m = max over kind-25h children of child->vtable[1F8h]() (006E4170: RepeatTime x max(0, count + [rack+484h] - 1)), FCOMI/JA so a NaN replaces m; n = their count; result = (float)([plane+538h]+1F4h (BombDelay) * max(0, n - 1) + m). The host returns only the largest rack span and omits BombDelay x (n - 1) (reported URGENT to team-lead 2026-10-16) |
| `007C3C90` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | binary 007C3C90-007C3CA3: 007C1570(plane; 4 + (flag != 0), 0). game_hosts_units.cpp:14548 documents exactly that and composes the downstream 007C1570 / 007CA3F0 chain in ground_state_from_locked_007ca3f0 |
| `007C47F0` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | binary 007C47F0-007C480D: (float)(tuning(0042E740)+24Ch * class+184h). The host (game_hosts_units.cpp:7721) multiplies the Lua LevelFlight multiplier (default 1.8f) by the slot's stall speed (default 17.5f); same product when both are loaded |
| `007CFD00` | existing_implementation_verified_against_listing | `src/local_player_unit_lists.cpp` | binary: 1 for 0Fh, 5 or 4, else 0 (no dynamic-class test); the table row {0x0f, nullptr, 0x007cfd00, 0x00d05eac} accepts [15, 5, 4] |

## 3. Adoption notes (host files are read-only for this packet)

- **007C1FB0 (URGENT, level bomb; sent to team-lead 2026-10-16).** `lb_rack_timing_007c1fb0`
  (`game_hosts_units.cpp:23649`) returns only the largest rack span. The image returns
  `(float)(BombDelay([plane+538h]+1F4h) * max(0, n - 1) + m)` where `n` counts the kind-25h children
  and `m` is the largest child `vtable[1F8h]()` (instructions 007C1FCA-007C2036). Live in a normal
  mission whenever a level-bomb task installs (approach+A8h at 009B4572). No shared-RNG draw in
  007C1FB0 itself; the caller's U(0.3, 0.65) is on 00BD2F10.
- **007B58D0.** The kamikaze/depth-charge host override returns `[00E17BF2]` alone; the image also
  requires `ctl+369h` (007B58D0 `CMP byte [ECX+369h],0`). Labelled in the host already.
- **007BCE20 / 007BCFA0.** The host minimum uses `v <= best`; the image's FCOMI/JA lets a NaN
  replace the running minimum. NaN-only difference.
- **007BC7A0.** The host's comment states the rocket pass as `+E0h > 0`; the image passes when
  `0.0f >= +E0h` (007BC7CD `COMISS xmm0(0), [EAX+E0h]` / `JAE`). The host applies the gate to bombs,
  where the rocket pass does not fire, so no behaviour moves.
- **007BB110.** `game_hosts_units.cpp:5505` calls the channel "channel C"; the image reads the
  channel at `[unit+DECh]+60h` (channel C is +44h). The host passes the gate unconditionally
  (labelled), so only the comment is off.

- **007C3CB0 (shared RNG).** The engine-sputter countdown redraws `+964h` with U(0.07, 0.18) on
  stream 0 (ECX = 0) of 00BD2F10 at 007C3E33, so any host carrying it shares the process-wide
  generator; it fires only with `[[00E188A8]+1ED4h]` set, below 8 m unless the plane is in state 6.

## 4. Not done in this pass (49)

Skipped with a reason:
- `007B7290` rocket task debug draw (5E0h bytes of x87 line drawing behind `+35Dh`); debug only.
- `007CF4B0` STL `vector::insert(n)` instantiation (calls 007CDA90 `_Xlen`): vendor, out of scope
  (ruling 2b); `007CCD30` the MSVC `vector<record 2Ch>` tidy, likewise.
- `007C2680`, `007C2880`, `007C29F0`: the PLANE_CREATE / control-state / PLANE_SYNC senders build a
  3Ch-byte by-value block on the stack (007D8330) and pass 11+ stack words to 007BDD30; deferred to
  a pass that can transcribe the argument frame word by word.
- `007BEEE0` (18C0h), `007C4DD0` (CF0h), `007C7A40` (1040h), `007C8A80` (650h), `007C9860` (B90h),
  `007BE9B0` (530h), `007C4850` (540h), `007C6820` (410h), `007D0820` (360h), `007CD5E0` (350h):
  large bodies; deferred to a dedicated packet each.
- `007C21D0`, `007C2450`: the bomb / rocket impact predictors iterate an x87 fall-time solve up to
  ten times with 00419010 and 007BA340 in the loop; deferred with the 007C4850 flight constants.
- `007D06A0`: builds "<prefix><n>" std::strings with `_itoa` (00BF9010), looks each up with
  0071AD50 and appends to an MSVC vector with the inline push_back / 00505D50 insert; deferred.

Left for the next pass (not yet read): `007C07A0`, `007C1570`, `007C17D0`, `007C18B0`, `007C2090`, `007C2130`, `007C2AF0`, `007C3700`, `007C39B0`, `007C6760`, `007C6E10`, `007C74A0`, `007C75A0`, `007C7600`, `007C78A0`, `007C95A0`, `007C9770`, `007CA5F0`, `007CB440`, `007CB5F0`, `007CB7F0`, `007CBA50`, `007CC580`, `007CCA50`, `007CCFA0`, `007CD2F0`, `007CD930`, `007CDDB0`, `007CDF20`, `007CFA30`.
