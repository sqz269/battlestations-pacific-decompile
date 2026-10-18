# October 18 reconciliation lane

Packet: the 96 rows in coordinator `local/codex_tail_R.txt`. Owner: agent/oct18-rec.
No game hosts or original installation are modified; this is reconstruction accounting
and bounded source work. Descriptive names remain hypotheses. Build evidence does not
establish original ABI compatibility or game validation.

## Progress board correction

Baseline at origin/main 21b82decf: 203 remaining (all segments), 19,994 game functions
done, 20,197 live game targets. The history parser remembered the first fragment at an
address and discarded every later function record. This hid whole bodies already
recorded in the ledger, especially naked MSVC definitions the source-name regex misses.
`first_ledger_records` now promotes a fragment when the first whole-function record
appears, using that function record's commit date and author. No duplicate implementation
or duplicate function record is required. A focused synthetic history checks promotion,
its timestamp, a fragment that stays partial, and an already-complete function.

## Compiler and library exclusions

The following original scalar-deleting destructors are compiler-generated wrappers:
00403610, 00403C60, 004041F0, 004042A0, 00405FC0, 00406210, 00406240, 00406270,
00406290, 004062F0. Exports show an ECX object, stack deletion-flags byte, conditional
`_free` under flag bit 0, and the returned object. The SAP variants perform member
cleanup or call the existing destructor first. This exclusion applies to those
generated deleting entries, not to the in-house Dyn physics implementation.

007512B0 and 007512F0 are telemetry library scalar-deleting entries: RTTI identifies
`iostdnet::ZMemoryPooledObject` and `iometrics::ZMetric::IStoredData`; both restore the
telemetry base vtable and conditionally call TELEMETRY_iostdnet_00A51E70. They are
outside the game-code contract even though RTTI tags obscured that classification.
Evidence: per-address `exports/bsp/functions/<address>/decompiled.c` and assembly;
existing inventory comments and direct callee identities. No Ghidra annotations changed.

## Missing Dyn/Mit bodies

`src/oct18_rec.cpp` / `include/bsp/oct18_rec.hpp` supply thirteen complete entries:

| Address | Body and original ABI |
|---|---|
| 00403C40 | Empty virtual method; ECX receiver, one stack word, RET 4. |
| 00404540, 00404550 | Empty terrain virtual methods; ECX receiver, three stack words, RET 0Ch. |
| 00405AA0 | Empty triangle virtual method; ECX receiver, RET. |
| 00405BF0 | Triangle support point; ECX shape; float output/direction and float transform on stack, RET 0Ch. |
| 00405D80 | Double-input/output support point; same stack contract and float transform, RET 0Ch. |
| 00407750, 00407BB0 | Reclaim empty pool pages; ECX pool, RET. Explicit host free binding replaces CRT call. |
| 00408300 | Store ECX dword to [EAX], RET; explicit pointer/value C++ interface. |
| 00409D00 | Load [EAX] to EAX, RET; explicit pointer C++ interface. |
| 00408E00 | Free non-null [EAX], RET; explicit header/free-binding interface. |
| 0040AB80 | Clear three dwords at ECX, leave EAX=receiver, RET. |
| 0040AB90 | Free non-null [ECX], RET; explicit header/free-binding interface. |

The empty virtual methods are real in-house entries, not inferred library exclusions.
The pool routines remove only pages whose free-stack depth is 128, move the last
table entry into the gap, rewrite all 128 moved slots' page-index trailers, retry
that slot, then find the first page with nonzero depth. They do not acquire locks,
shrink the page table or clear freed vector headers. Layout is the existing proven
`cc13_dyn2::MitPoolAllocator36x128` header, also shared by the 20-byte pool.

The two triangle routines select the greatest projected vertex, then transform that
vertex. Source retains all original x87 instructions and float spills. The double
variant narrows direction components to float and widens float-spilled outputs;
replacing it with double arithmetic would change its behavior. JBE branches retain
original equal/unordered selection. Named inline assembly was decoded from PE bytes,
cross-checked with both complete pseudocode exports and their assembly; no absolute
data or call targets occur. EDX dummy parameters adapt thiscall's stack layout.
No original game integration or runtime/ABI equivalence is claimed.
MSVC Win32 compilation and both existing CTest groups pass. Extracting the two
triangle symbols from the resulting COFF object confirms exact whole-body PE byte
identity: 393 bytes at 00405BF0 and 469 bytes at 00405D80. This is static proof;
the compiled bodies have not been used by a running game.
A focused manifested Win32 probe also passes both pool layouts through the same
mixed-page case: two empty pages including the moved page, one partial page,
all 128 trailer indices rewritten, first-free hint recomputed, capacity/lock retained.
Thirteen naming records and Ghidra plate comments were updated under the annotation
write lock; prior values are in `local/ghidra-annotations-20261018T230059Z.json` and
the project was saved. Existing class/virtual-slot names were retained; only the
two previously generic single-word register helpers received semantic names.

## Existing bodies reconciled without rewriting

00408A78 is Ghidra's split continuation of the already complete logical 004089E0
growth routine. `finish_growth` in `src/native_legacy_sbo_string.cpp` covers its copy,
old-buffer release and header/terminator commits. It is not independently callable:
original EBX preserve-count and parent EBP/EH frame are live. The new record explicitly
points to that parent coverage; existing fragment and original function boundaries stay.

008F81F0 is `run_win_main` in `src/winmain_startup.cpp`, the existing full startup
control flow through injected platform/application services. Original stdcall four
arguments are unused; EAX=0, RET 10h. The documented safe std::string path buffer and
host-bound COM/language/EH interfaces remain limitations; this is source reconstruction.

00AD2750 already has a complete raw-storage getter in `src/native_gui_frame_box_factory.cpp`.
Its ledger record was introduced only by a merge and therefore absent from the board's
non-merge history. Refreshing that existing record with reconciliation evidence gives
it reviewable provenance without duplicating its body. Original ECX box, signed16 stack
index, EAX record, RET 4; checked-invalid-parameter and post-callback pointer reloads remain.

007F23A0's old `ledger_only_007f23a0` record now has a companion naming the existing
`plane_formation_station_007f23a0` C++ implementation. It covers all five formation
shapes, width floor, morale scaling, mirrors and the world transform; existing
`game_hosts_units.cpp` supplies leader/class/tuning inputs. This remains a semantic
source/host composition with guards, not an original object-layout/ABI claim.
00B865A0 and 00BB97B0 records also now carry their actual C++ names; the latter shares
the full native enumeration body behind provider entry 00BB98F0.

## Three incomplete semantic projections completed

`set_xbox_mode_008d44c0`, `mission_completed_0090c560` and
`read_quoted_runs_008efb00` in the new source supply whole raw-storage drivers.
Their injected callbacks are explicit dependency contracts, not substitute bodies.

The settings driver retains both byte stores, the current game/owner reloads,
the nullable Lua machine gate, current settings-byte command selection and (0,0,2)
arguments. The mission driver preserves raw find/iterator-owner/end-node checks,
including the continuation when the CRT invalid-parameter callback returns.

The quoted-run driver initializes exactly the written bytes of its 20h stack writer,
then resets, peeks, consumes quoted tokens, decodes/appends, joins and clears in
the original order. The original EH handler 00CA4710 selects FuncInfo 00DD72B4:
two unwind states, table 00DD72A4 = {-1,00CA4700,-1,00CA4708}; those funclets call
008EF030 (reader destruction) and 0045C4C0 (base destruction). The new driver's
host C++ EH mirrors these two cleanup phases and terminates if cleanup itself throws.
Native SEH registration/exception identity is not claimed. Existing parser projection
and its input-size restrictions remain separate; no host wiring changed.

Win32 build and both CTest groups pass. The manifested focused probe checks raw
settings byte/machine gating, a returning iterator validator, quoted-run call order,
and failures in both unwind states, as well as the pool fixture. Three existing names
were retained and evidence comments appended under the Ghidra write lock, the project
saved, and affected exports refreshed. Prior annotations are in the local annotation log.

## Uncertain whole functions retained as fragments

No packet row was skipped for a lease conflict. Two rows cannot honestly receive
whole-function credit from their existing source:

- **007D1360**: `pilot_cmd_apply_state_message_007d1360` is only the six-dword
  control/command copy projection. Original 007D1360..007D1B4B also gates host/local
  ownership, serializes the message, invalidates/applies world/controller poses,
  replays fixed steps, then performs x87 correction. The decompiler's overlapping
  temporaries and `unaff_EBX` do not supply a valid contract; for example the actual
  007D173B gate tests stack byte [ESP+12h]. A whole driver needs assembly-led temporary,
  register, callback and alias recovery. The current message/result API omits those effects.
- **00864880**: the existing source is explicitly a pure ranging fragment with draws
  supplied as arguments. The full raw constructor also initializes observer/class/vtable
  state and registers EDX=record against ECX=subject at 00694A60, performs its own RNG
  calls and pose refreshes, and calls target velocity slot +34h twice. Original sqrt
  at 00864991 takes ST0, not a conventional stack double. The pseudocode's
  `unaff_retaddr` is not an acceptable think-time input: assembly 00864A8D loads config
  +88h and spills it at [ESP+44h]. A complete constructor needs these raw provider,
  x87 and observer-base cleanup contracts. Arithmetic alone does not cover them.

Both existing fragment records now state these concrete gaps. The coordinator should
assign dedicated whole-function recovery packets if those two entries are to be closed.

## All 96 packet rows

(a) existing source retained/reconciled; (b) generated/library excluded;
(c) new complete source; (d) uncertain whole body retained as fragment.

| Address | Decision | Source / C++ entry |
|---|---|---|
| 00403610 | (b) | `original export` / `compiler/library wrapper` |
| 00403C40 | (c) | `src/oct18_rec.cpp` / `radix_noop_00403c40` |
| 00403C60 | (b) | `original export` / `compiler/library wrapper` |
| 004041F0 | (b) | `original export` / `compiler/library wrapper` |
| 004042A0 | (b) | `original export` / `compiler/library wrapper` |
| 00404540 | (c) | `src/oct18_rec.cpp` / `terrain_noop_00404540` |
| 00404550 | (c) | `src/oct18_rec.cpp` / `terrain_noop_00404550` |
| 00405AA0 | (c) | `src/oct18_rec.cpp` / `triangle_noop_00405aa0` |
| 00405BF0 | (c) | `src/oct18_rec.cpp` / `triangle_support_float_00405bf0` |
| 00405D80 | (c) | `src/oct18_rec.cpp` / `triangle_support_double_00405d80` |
| 00405FC0 | (b) | `original export` / `compiler/library wrapper` |
| 00406210 | (b) | `original export` / `compiler/library wrapper` |
| 00406240 | (b) | `original export` / `compiler/library wrapper` |
| 00406270 | (b) | `original export` / `compiler/library wrapper` |
| 00406290 | (b) | `original export` / `compiler/library wrapper` |
| 004062F0 | (b) | `original export` / `compiler/library wrapper` |
| 00407750 | (c) | `src/oct18_rec.cpp` / `pool36_reclaim_empty_pages_00407750` |
| 00407BB0 | (c) | `src/oct18_rec.cpp` / `pool20_reclaim_empty_pages_00407bb0` |
| 00408300 | (c) | `src/oct18_rec.cpp` / `store_word_00408300` |
| 00408A78 | (a) | `src/native_legacy_sbo_string.cpp` / `finish_growth` |
| 00408E00 | (c) | `src/oct18_rec.cpp` / `vector_free_00408e00` |
| 00409D00 | (c) | `src/oct18_rec.cpp` / `load_word_00409d00` |
| 0040AB80 | (c) | `src/oct18_rec.cpp` / `vector_construct_empty_0040ab80` |
| 0040AB90 | (c) | `src/oct18_rec.cpp` / `vector_free_0040ab90` |
| 004837D0 | (a) | `src/native_unit_list_erase.cpp` / `erase_native_unit_list_004837d0` |
| 004845A0 | (a) | `src/native_unit_list_remove.cpp` / `remove_native_unit_list_payload_004845a0` |
| 004B7EC0 | (a) | `src/native_parent_list_header_storage.cpp` / `initialize_native_parent_list_header_storage_004b7ec0` |
| 006965A0 | (a) | `src/native_input_action_deadlines.cpp` / `invoke_native_input_action_deadline_callback_006965a0` |
| 006AA460 | (a) | `src/native_input_settings_lifetime.cpp` / `destroy_native_raw_input_settings_006aa460` |
| 006AB6B0 | (a) | `src/native_input_settings_lifetime.cpp` / `construct_native_input_settings_006ab6b0` |
| 00737A50 | (a) | `src/app_frame.cpp` / `run_application_frame` |
| 00737F30 | (a) | `src/app_shutdown.cpp` / `shutdown_application` |
| 007420B0 | (a) | `src/cc15_land_convoy.cpp` / `land_convoy_bind_motion_007420b0` |
| 00742180 | (a) | `src/cc15_land_convoy.cpp` / `land_convoy_kill_members_00742180` |
| 007422A0 | (a) | `src/cc15_land_convoy.cpp` / `land_convoy_remove_member_007422a0` |
| 007512B0 | (b) | `original export` / `compiler/library wrapper` |
| 007512F0 | (b) | `original export` / `compiler/library wrapper` |
| 007D1360 | (d) | `src/pilot_command_path.cpp` / `pilot_cmd_apply_state_message_007d1360` |
| 007F23A0 | (a) | `src/plane_formation.cpp` / `plane_formation_station_007f23a0` |
| 00864880 | (d) | `src/ship_gunnery_ver_cc13.cpp` / `artillery_ranging_record_construct_00864880` |
| 008D44C0 | (c) | `src/oct18_rec.cpp` / `set_xbox_mode_008d44c0` |
| 008EF170 | (a) | `src/native_scene_property_record_type1_float_storage.cpp` / `construct_native_scene_property_record_type1_float_storage_008ef170` |
| 008EF1B0 | (a) | `src/native_scene_property_record_type2_string_storage.cpp` / `construct_native_scene_property_record_type2_string_storage_008ef1b0` |
| 008EF230 | (a) | `src/native_scene_property_record_type4_storage.cpp` / `construct_native_scene_property_record_type4_storage_008ef230` |
| 008EF270 | (a) | `src/native_scene_property_record_type7_storage.cpp` / `construct_native_scene_property_record_type7_storage_008ef270` |
| 008EF2B0 | (a) | `src/native_scene_property_record_type5_storage.cpp` / `construct_native_scene_property_record_type5_storage_008ef2b0` |
| 008EF2F0 | (a) | `src/native_scene_property_record_type8_byte_array_storage.cpp` / `construct_native_scene_property_record_type8_byte_array_storage_008ef2f0` |
| 008EFB00 | (c) | `src/oct18_rec.cpp` / `read_quoted_runs_008efb00` |
| 008F0340 | (a) | `src/native_reference_payload_copy.cpp` / `copy_native_reference_payload_008f0340` |
| 008F41F0 | (a) | `src/native_scene_property_clone.cpp` / `clone_native_scene_property_bag_008f41f0` |
| 008F54F0 | (a) | `src/native_scene_property_bag_merge.cpp` / `merge_native_scene_property_bag_008f54f0` |
| 008F81F0 | (a) | `src/winmain_startup.cpp` / `run_win_main` |
| 0090C560 | (c) | `src/oct18_rec.cpp` / `mission_completed_0090c560` |
| 00A3EAC0 | (a) | `src/engine_seg11_35_66_cc13.cpp` / `active_user_slot_00a3eac0` |
| 00A90ED0 | (a) | `src/engine_seg11_35_66_cc13.cpp` / `enumerate_input_noop_00a90ed0` |
| 00AA38E0 | (a) | `src/engine_seg11_35_66_cc13.cpp` / `gui_layer_visible_00aa38e0` |
| 00AA9390 | (a) | `src/native_gui_widget_base_storage.cpp` / `construct_native_gui_widget_base_00aa9390` |
| 00ABE960 | (a) | `src/engine_seg11_35_66_cc13.cpp` / `normalize_angle_00abe960` |
| 00AD2600 | (a) | `src/native_gui_frame_box_factory.cpp` / `construct_native_gui_frame_box_default_00ad2600` |
| 00AD26D0 | (a) | `src/native_gui_frame_box_factory.cpp` / `native_gui_frame_box_current_state_bits_00ad26d0` |
| 00AD2750 | (a) | `src/native_gui_frame_box_factory.cpp` / `native_gui_frame_box_state_record_00ad2750` |
| 00B13510 | (a) | `src/native_render_diagnostic_labels.cpp` / `reset_native_render_diagnostic_label_00b13510` |
| 00B17E10 | (a) | `src/native_material_parameters.cpp` / `register_native_material_parameter_00b17e10` |
| 00B1CB30 | (a) | `src/native_render_queue_access.cpp` / `read_native_render_batch_sort_configuration_00b1cb30` |
| 00B250B0 | (a) | `src/native_plane_set.cpp` / `copy_native_plane_set_records_00b250b0` |
| 00B3B3C0 | (a) | `src/native_material_program_compiler_actual.cpp` / `NativeMaterialProgramCompilerActual::continue_material_pass_00b3b3c0` |
| 00B44D60 | (a) | `src/native_material_parameters.cpp` / `bind_native_material_parameter_00b44d60` |
| 00B51B50 | (a) | `src/native_render_batch_lifetime.cpp` / `reserve_native_render_batch_entries_00b51b50` |
| 00B57B50 | (a) | `src/native_shader_sampler_reader.cpp` / `read_native_shader_sampler_00b57b50` |
| 00B62D10 | (a) | `src/native_camera_plane_transform.cpp` / `transform_native_vector4_00b62d10` |
| 00B650B0 | (a) | `src/native_camera_frustum.cpp` / `normalize_native_camera_plane_00b650b0` |
| 00B67800 | (a) | `src/native_lua_objects.cpp` / `native_lua_get_by_name_00b67800` |
| 00B68340 | (a) | `src/native_lua_fundamentals.cpp` / `construct_native_lua_fundamentals_00b68340` |
| 00B6DAB0 | (a) | `src/engine_seg11_35_66_cc13.cpp` / `node_local_position_00b6dab0` |
| 00B82390 | (a) | `src/native_scene_registry_storage_leaves.cpp` / `allocate_native_scene_registry_sentinel_00b82390` |
| 00B82570 | (a) | `src/native_scene_registry_storage_leaves.cpp` / `fill_native_scene_registry_iterators_00b82570` |
| 00B84C60 | (a) | `src/native_ambient_color.cpp` / `native_ambient_color_address_00b84c60` |
| 00B84C90 | (a) | `src/native_fog_access.cpp` / `get_native_fog_underwater_color_00b84c90` |
| 00B84CA0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_68_00b84ca0` |
| 00B84CB0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_6c_00b84cb0` |
| 00B84CC0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_70_00b84cc0` |
| 00B84CD0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_74_00b84cd0` |
| 00B84CE0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_78_00b84ce0` |
| 00B84CF0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_7c_00b84cf0` |
| 00B84DA0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_80_00b84da0` |
| 00B84DB0 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_84_00b84db0` |
| 00B84E20 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_88_00b84e20` |
| 00B84E30 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_8c_00b84e30` |
| 00B84E40 | (a) | `src/native_fog_access.cpp` / `load_native_fog_scalar_90_00b84e40` |
| 00B84FD0 | (a) | `src/native_fog_access.cpp` / `get_native_fog_directional_color_00b84fd0` |
| 00B865A0 | (a) | `src/native_mesh_section.cpp` / `rebuild_native_mesh_section_vertex_layout_00b865a0` |
| 00BB97B0 | (a) | `src/native_mpkg_enumeration.cpp` / `enumerate_native_mpkg_names_00bb98f0` |
| 00BD0560 | (a) | `src/native_singleton_vector_leaves.cpp` / `fill_native_singleton_slots_00bd0560` |
| 00BD0700 | (a) | `src/native_singleton_vector_insert_count.cpp` / `insert_count_native_singleton_slots_00bd0700` |
| 00BE80B0 | (a) | `src/native_filestore_provider_lifetime.cpp` / `get_native_file_store_provider_00be80b0` |
| 00BE9630 | (a) | `src/fingerprint_payload.cpp` / `decode_fingerprint_payload_00be9630` |
