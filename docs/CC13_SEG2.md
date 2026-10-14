# CC13 segment 2 leaves (cc13-seg2)

Packet: segment 2, 0042A920..0043E4D0 (profile save, unlocks, play time, difficulty in the
partition keywords; in practice mostly the camera mover, its vector and matrix helpers, the
observer references and the vehicle class registry / profile manager lifetime).

Code: `src/profile_seg2_cc13.cpp`, `include/bsp/profile_seg2_cc13.hpp`
(namespace `bsp::profile_seg2_cc13`). Objects are reached through `Raw` byte offsets in their
Win32 layout; every call the image makes goes through a `Seg2Host` method named for its target
address. The two lazy getters reuse the cc13-sys4 body `bsp::cc13_sys::lazy_singleton_get`
(src/engine_seg4_6_9_cc13.cpp). Names are hypotheses, not recovered symbols. Status of every
row: reconstructed and Win32 build-tested; no fixture, no original-ABI or game validation.

## Triage

`bsp.py range 0042a920 0043e4d0` gave 141 rows with no tag and no record (funclets excluded).
Each address was searched in `src`/`include` (both `00xxxxxx` and 6-digit forms). Many hits were
only host-interface methods or comments naming the address; those functions are not
implemented and some are reconstructed here.

## Reconstructed (71)

| Address | C++ | ABI | Body |
|---|---|---|---|
| 0042A920 | camera_mover_set_camera_node_0042a920 | thiscall, RET 4 | +1BCh = node |
| 0042A940 | camera_set_shake_max_0042a940 | thiscall(float), RET 4 | +1C4h = v; +1C0h = v when +1C0h > v. The native leaves its FLD of v on the x87 stack (not modelled) |
| 0042A970 | camera_set_shake_base_0042a970 | thiscall(float), RET 4 | +1C8h = v; +1C0h = v when v > +1C0h |
| 0042A9D0 | ship_camera_on_observed_released_0042a9d0 | thiscall, RET 4 | clear +39Ch when equal |
| 0042AAD0 | plane_chase_camera_on_observed_released_0042aad0 | thiscall, RET 4 | clear +378h when equal |
| 0042BA40 | entity_get_heading_0042ba40 | thiscall, RET, ST0 | 00414DB0, then -(float)_CIatan2(+ECh, +F4h) |
| 0042BA70 | camera_mover_get_default_extent_0042ba70 | (out), RET 4 | {512.0f, 384.0f} |
| 0042BA90 | camera_mover_return_ten_0042ba90 | RET, ST0 | 10.0f |
| 0042BAD0 | camera_mover_forward_to_target_slot_e8_0042bad0 | thiscall, RET | +3Ch vtable +E8h tail jump, else false |
| 0042D800 | camera_mover_clear_byte_c_0042d800 | stdcall(object), RET 4 | byte +0Ch = 0 |
| 0042DF80 | ship_captain_camera_copy_state_if_same_ship_0042df80 | thiscall, RET 4 | same +3ACh: 0042DC20, copy +3F4h..+3FCh |
| 0042CEA0 | plane_chase_camera_adopt_from_0042cea0 | thiscall, RET 4 | class 51h (or 54h holder via 00798D70) -> 0042CD50 |
| 0042EAC0 | ship_captain_camera_copy_state_from_camera_0042eac0 | thiscall, RET 4 | class 50h (or holder) -> 0042DF80 |
| 0042EB90 | submarine_camera_adopt_from_0042eb90 | thiscall, RET 4 | byte +404h = 1; class 4Fh (or holder) -> 0042DFD0 |
| 0042E0E0 | entity_set_ref_target_and_offset_0042e0e0 | thiscall, RET 8 | release +3C0h, store target and offset twice, vtable +DCh(0.0f) |
| 0042F650 | camera_mover_set_target_0042f650 | thiscall, RET 4 | re-register target +3ACh with observer +10h; drop focus ref +3B0h; +3D4h, +3D8h cleared |
| 00432E60 | camera_mover_set_target_and_mode_00432e60 | thiscall, RET 8 | +3F4h = mode, 0042F650, +3A4h from settings (00424C40) +450h * class +A0h, clear +3F8h/+3FCh |
| 0042BAF0 | entity_hit_not_handled_0042baf0 | RET 4 | false |
| 0042BB20 | entity_aim_offset_still_valid_0042bb20 | RET 0Ch | true |
| 0042BB40 | return_false_0042bb40 | RET | false |
| 0042BB70 | tick_element_place_step_pose_0042bb70 | RET 4 | empty |
| 0042BB80 | tick_element_tick_advance_0042bb80 | RET 4 | empty |
| 0042BB90 | tick_element_commit_step_pose_0042bb90 | RET | empty |
| 0042BBA0 | tick_element_unused_slot_0042bba0 | RET 4 | empty |
| 0042BBB0 | tick_element_predicate_0042bbb0 | RET | false |
| 0042AE20 | vector3_divide_scalar_to_0042ae20 | thiscall(out, float), RET 8 | out = v / s |
| 0042AE50 | vector3_scale_in_place_0042ae50 | thiscall(float), RET 4, EAX = v | v *= s |
| 0042AEB0 | vector3_add_in_place_0042aeb0 | thiscall, RET 4, EAX = v | v += w |
| 0042AEE0 | vector3_subtract_in_place_0042aee0 | thiscall, RET 4, EAX = v | v -= w |
| 0042B220 | vector3_dot_0042b220 | fastcall(ECX, EDX), RET, ST0 | (y*y + x*x) + z*z, float-rounded |
| 0042B3C0 | matrix_set_rotation_x_0042b3c0 | thiscall(float), RET 4 | 00B64640 then 004134F0 |
| 0042B3F0 | matrix_set_rotation_y_0042b3f0 | thiscall(float), RET 4 | 00B646E0 then 004134F0 |
| 0042B420 | matrix_set_rotation_z_0042b420 | thiscall(float), RET 4 | 00B64780 then 004134F0 |
| 0042B450 | matrix_set_rotation_x_negated_0042b450 | thiscall(float), RET 4, EAX = this | -0.0f - angle, 00B64640, 004134F0 |
| 0042B490 | matrix_set_rotation_y_negated_0042b490 | thiscall(float), RET 4, EAX = this | -0.0f - angle, 00B646E0, 004134F0 |
| 0042B680 | matrix_assign_0042b680 | thiscall, RET 4, EAX = this | 004134F0 |
| 0042B7F0 | matrix_orthonormalize_return_this_0042b7f0 | thiscall, RET, EAX = this | 0085DC80 |
| 0042D510 | matrix_set_identity_0042d510 | thiscall, RET, EAX = this | identity copied by 004134F0 |
| 0042E8C0 | matrix_set_identity_0042e8c0 | thiscall, RET, EAX = this | same body as 0042D510 |
| 0042D5A0 | matrix_set_forward_up_orthonormalize_0042d5a0 | thiscall, RET 8, EAX = this | rows 2 and 1, then 0085DC80 |
| 0042AFF0 | transform_get_world_matrix_0042aff0 | thiscall, RET | 00B6DB70 when +5Ch bit 1 clear; returns +F0h |
| 0042D210 | transform_get_world_position_0042d210 | thiscall, RET 4 | same refresh; out = +120h..+128h |
| 0042D870 | observer_ref_set_0042d870 | thiscall, RET 4 | swap the observed object at +14h (006952A0 / 00694A60) |
| 0042E700 | observer_ref_set_0042e700 | thiscall, RET 4, EAX = this | same |
| 0042E800 | observer_ref_set_0042e800 | thiscall, RET 4, EAX = this | same |
| 0042ECA0 | observer_ref_set_0042eca0 | thiscall, RET 4, EAX = this | same |
| 0042EC20 | observer_ref_on_observed_released_0042ec20 | thiscall, RET 4 | released vtable +4h == +14h: clear and unregister |
| 0042EC50 | observer_ref_on_observed_released_0042ec50 | thiscall, RET 4 | same |
| 0042E840 | class_46_dynamic_cast_0042e840 | fastcall, RET | object if vtable +5Ch(46h) |
| 0042E950 | unit_instance_name_or_fallback_0042e950 | thiscall, RET | +158h or "" at 00E1767D |
| 0042AC00 | global_config_owner_destruct_singleton_base_0042ac00 | thiscall, RET | [00F878E4] = 0, vtable 00CE3818 |
| 00435750 | profile_manager_owner_destruct_singleton_base_00435750 | thiscall, RET | [00E17680] = 0, vtable 00CE3818 |
| 00435770 | vehicle_class_registry_destruct_singleton_base_00435770 | thiscall, RET | [00F8A0B8] = 0, vtable 00CE3818 |
| 00435850 | lua_script_holder_close_00435850 | thiscall, RET | _free(+30h), then 00B65E80(+38h) |
| 004358C0 | profile_manager_destruct_004358c0 | thiscall, RET | vtable 00CE411C, 00B67700(+4), [00E17680] = 0, vtable 00CE3818 |
| 004356F0 | vehicle_class_registry_reset_index_tables_004356f0 | thiscall, RET | +10h[i] = +2010h[i] = i, i < 800h |
| 00437E10 | vehicle_class_registry_construct_00437e10 | thiscall, RET, EAX = this | vtable 00CE417C, vectors zeroed, tables identity, 00435960(+4010h, 400h) |
| 00437EA0 | vehicle_class_registry_destruct_00437ea0 | thiscall, RET | 004359E0 / 00437BD0 to zero with _free, [00F8A0B8] = 0 |
| 00437DC0 | registry_cache_destroy_00437dc0 | thiscall, RET | 00437BD0(0), _free |
| 00437DF0 | registry_cache_destroy_00437df0 | thiscall, RET | same |
| 00436370 | party_rows_destroy_00436370 | thiscall, RET | 004359E0(0), _free |
| 00436460 | party_rows_destroy_00436460 | thiscall, RET | same |
| 00437F50 | vehicle_class_registry_get_00437f50 | cdecl, RET | lazy getter: new 401Ch, 00437E10 |
| 0042E740 | game_tuning_get_0042e740 | cdecl, RET | lazy getter of 00F87440: new 6D0h, 007E2A20 |
| 0043AB50 | scoped_byte_override_f8d394_construct_0043ab50 | thiscall(byte), RET 4 | save and override [00F8D394]+10h |
| 0043AB80 | scoped_byte_override_f8d394_destruct_0043ab80 | thiscall, RET | restore it |
| 00431F20 | native_string_destroy_storage_00431f20 | thiscall, RET | pool release (data +4, length +0 plus 1) |
| 00431F40 | native_string_destroy_storage_00431f40 | stdcall, RET 4 | same |
| 00436490 | native_string_at_0c_destroy_storage_00436490 | thiscall, RET | the string at +0Ch |
| 0043BAE0 | native_string_at_8_destroy_storage_0043bae0 | thiscall, RET | the string at +8h |
| 0043D090 | native_string_at_8_destroy_storage_0043d090 | stdcall, RET 4 | the string at +8h |

Simplifications and uncertainties (also in the C++ comments and ledger evidence):

- 0042B220: the listing sums (y*y + x*x) + z*z (the decompiler shows a different order) and
  rounds the result to float. The intermediate sum is double here, not 64-bit extended.
- 0042BA40: the host returns `_CIatan2` as a double; x87 extended rounding before the float
  store is not reproduced.
- 0042A940: the native leaves its argument on the x87 stack; not modelled.
- 0042BAD0: the false path is `XOR AL,AL` with the upper EAX bits left as they were; it is
  modelled as a bool.
- SEH frames of 004358C0, 00437E10 and 00437EA0 only matter during unwinding and are not modelled.
- The three adopt bodies (0042CEA0, 0042EAC0, 0042EB90) were transcribed from the listing. The
  Ghidra decompile of them is garbled (empty `if` blocks).

## Existing implementations recorded (8)

| Address | Where | Finding |
|---|---|---|
| 0042B4D0 | src/marker_manager_cc13.cpp rotation_z_0042b4d0 | matches; builds straight into the output instead of a stack temporary copied by 004134F0 (same values) |
| 0042BE90 | src/plane_follow_law.cpp fabs_0042be90 | matches the listing (takes the float by value; the native takes a pointer in ECX) |
| 004396F0 | src/hud_ship_screen.cpp ship_screen_gauge_round_004396f0 | matches the listing |
| 00439950 | src/plane_wanderer.cpp plane_wanderer_random_direction_00439950 | last-ulp defect (cos/sin not rounded to float before the products); faithful path added by cc13-seg61 behind kPlaneWandererFloatTrigBound (default false) |
| 00432480 | src/ship_sub_entity_list.cpp sub_entity_list_self_00432480 | was a caller-array model; cc13-seg61 made it call 004323D0 push_back through PointerVectorHost |
| 0042C610 | src/mission_camera.cpp apply_zoom_0042c610 | located, not re-verified |
| 0042F0C0 | src/mission_camera.cpp keep_above_water_0042f0c0 | located, not re-verified |
| 00432ED0 | src/mission_camera.cpp update_ship_captain_00432ed0 | located, not re-verified |

## Skipped

- STL / template instantiations that need tags, not reconstruction: 0042B100, 0042BF10, 0042EE60,
  00432090, 00432360, 00432440, 0043B920, 0043BB60, 0043BF60, 0043C050, 0043C0B0, 0043CFE0,
  0043D110, 0043D330, 0043E120. 00432360 and 00432440 also pass registers to 00432050 in an
  ad hoc way that a faithful C++ call cannot express without that callee.
- Larger bodies not attempted in this packet (they need their own pass): the frustum tests
  0042E3D0/0042E460, 0042DCD0 (src/mission_camera.cpp models only its gate), 004329D0, 00435410,
  the projection and unprojection set 00439820, 00439DB0, 00439F50, 0043A0F0, 0043A290, 0043A660,
  0043A870, 0043B220, 0042EF90, the HUD digit gauge 0043ABA0/0043B370 and observer sets
  0043DF40..0043E060, the property map 0043D2D0/0043D4D0/0043D870/0043D8F0, 0043DD80,
  0043B450, 0043BDF0, 00435F40, 00436530, 00435960, 004359E0, 00438A00, 00438F90, 00439A00,
  00439AC0, 00439AF0, 004313D0, 00432380, 0043BFC0, 0043CFA0.
- No Ghidra function at the address yet (no export): 0042AA00, 0042C450, 0042CD00, 0042D830,
  0043AD80, 0043B9C0.
