# CC13 segment 53 (00851E10-008740E0): turning gun, gunnery AI, effects

Worker cc13-seg53, 2026-10-16. New code: `src/cc13_seg53.cpp`, `include/bsp/cc13_seg53.hpp`
(namespace `bsp::cc13_seg53`). Every call the image makes goes through a named `Seg53Host` hook at
the same point with the same arguments; the hook comment gives the callee's ABI. Nothing is wired
into the game. Names are hypotheses, not recovered symbols. Status for every row below:
reconstructed and build-tested, not ABI-compatible, not fixture-tested, not game-validated.

Evidence: the exported listings (`exports/bsp/functions/<addr>/assembly.txt`), disk bytes for
functions Ghidra has no body for (00855B90, 00855BD0, the 00858180..00858200 leaves), and constants
read from the PE. NaN arms follow the jump after each compare (noted per row).

## Torpedo and torpedo boat

| Address | Function | Notes |
|---|---|---|
| 00855B90 | `torpedo_projectile_show_node_00855b90` | With a model at +464h, 00B6DA70(ECX = [model+0Ch], 1.0f, 0). |
| 00855BD0 | `torpedo_create_message_resolve_target_00855bd0` | Net id word +128h through the two-bank entity table (00F89A10 split, banks 00F89A54/00F89AA8, bases 00F89A0C/00F89A60); 0 for id 0. Unreferenced. |
| 00857D60 | `torpedo_boat_class_recon_category_00857d60` | Returns 2. |
| 00857E20 | `torpedo_boat_class_create_entity_00857e20` | operator new(1190h), memset with no null test, 00857CD0(flag) when non-null, 009553D0(boat, class), boat+354h = class. |

None of these is torpedo or submarine AI: they are the class factory, a node show slot and an
unreferenced net-id resolver.

## Turning gun and its replication message (T_GUN_TURN_SYNC, type 0AEh, vtable 00D032B4)

| Address | Function | Notes |
|---|---|---|
| 00859410 | `gun_turn_sync_construct_00859410` | 0075B430(0AEh), field stores, four 00605070 wraps (+20h, +24h, +28h, +2Ch). |
| 00859C80 | `gun_turn_sync_write_00859c80` | 0075B480, two 004290B0 flags, then the message's vtable slot 14h (host hook). |
| 00859CC0 | `gun_turn_sync_write_angle_deltas_00859cc0` | Four channels against the record at +40h: resend when the wrapped delta reaches 0.01 (NaN resends), else write 0 and drift the sent value by 0.001 through 00438AA0; then 00429090(+30h, 2). 8 bits when +1Ch is set, else 12. |
| 00859FA0 | `gun_turn_sync_read_00859fa0` | 0075B4C0, two 00428D70 flags on stream+4, then vtable slot 18h. |
| 00859FE0 | `gun_turn_sync_read_angle_deltas_00859fe0` | Presence flags +36h/+37h/+34h/+35h gate 004293F0 reads; 00428D30(+30h, 2). |
| 00859830 | `unit_set_gun_aim_record_00859830` | +4A0h = mission clock, +4A4h..+4ACh, then 004B4D80(out, a5, a4) into +4B0h..+4B8h. |
| 008598B0 | `turning_gun_read_recent_shot_008598b0` | Copies the six-float shot record when clock - +4A0h < 0.25 (NaN skips). |
| 00859920 | `turning_gun_commit_step_angles_00859920` | +178h/+17Ch from +170h/+174h, both wrapped by 00605070. |
| 00859A20 | `turning_gun_write_aim_snapshot_fields_00859a20` | Early out on owner vtable[5Ch](6) or count >= 5; builds a stack sync message (00859410), 00779FC0 to the peer, 00779F80 stamp, inlined destructor stores vtable 00CE4974. |
| 00859B10 | `turning_gun_write_snapshot_fields_00859b10` | Fills an existing message from the gun, four 00605070 wraps, +38h/+3Ch from the target vector at +348h, then vtable slot 14h. |
| 0085A130 | `turning_gun_debug_dump_fields_0085a130` | 0072ADC0, then the writer's slots 4, 0Ch (five fields) and 8. |
| 0085A9A0 | `gun_target_in_fire_window_0085a9a0` | Mount and target refresh (00B6DB70, 00414DB0), local direction (0042D0D0, 0042B260), 00521370 yaw/pitch, ordered +-2pi folds, 007F60A0(-yaw, pitch). |
| 0085AB50 | `turning_gun_filter_heading_to_traverse_0085ab50` | Platform slot (005471B0 grow) then 007F6190; ST0 returned as double. |
| 0085AD00 | `turning_gun_aim_to_rest_angles_0085ad00` | 0085ABA0(slot+94h, slot+90h) unless slot+94h is exactly FLT_MAX (NaN still calls). |
| 0085B980 | `turning_gun_aim_at_world_direction_0085b980` | Pose refresh (00414DB0, 00B63D50), 0042D0D0, 0042B260, clamp, 0042CF10, _CIatan2 (as std::atan2), 0085ABA0(-h, a). |
| 0085BA30 | `turning_gun_apply_snapshot_fields_0085ba30` | Sequence test on +4CCh, 00927F30 roles 0 and 1, present channels, 0085B0F0. |
| 0085BB50 | `turning_gun_load_instance_state_0085bb50` | 0072F060; on a saved game the "_turningGun" Lua reader (009238A0, 004425C0, 00BD8E20, five 00BD6830, 00BD7A20, 00441A20). |

## Matrices, recon categories, debug draw

| Address | Function | Notes |
|---|---|---|
| 0085BCE0 | `recon_category_index_from_name_0085bce0` | 19-entry name table at 00E0B590, CRT _stricmp; 13h on a miss. |
| 0085BD50 | `recon_category_from_class_type_name_0085bd50` | Inlined "Destroyer" compare, then the 00425850 chain; unreferenced. |
| 0085C0F0 | `matrix_set_look_at_copy_0085c0f0` | EDX eye passes through to 00B63F10; 004134F0 copy. |
| 0085C220 | `matrix_set_from_euler_yxz_copy_0085c220` | EDX angles pass through to 0047C2A0; 004134F0 copy. |
| 0085F990 | `debug_draw_line_rgba_floats_0085f990` | Byte channels packed ARGB; renderer [00F8D394] slot B4h. Each channel is FMUL double 255.0 under the caller's control word, then FISTP under chop: on the 24-bit main thread the product is rounded to float first (cc13-fix15, aligned with cc13_big53 colour_byte24), so a product just below an integer can give a byte one higher than the double-chop model. |
| 0085FB30 | `debug_draw_line_rgba_floats_one_colour_0085fb30` | 0085F990 with one colour twice. |
| 00860030 | `debug_draw_screen_line_rgba_floats_00860030` | Renderer slot C0h with two 2-float points. |
| 00860AD0 | `debug_draw_arc_around_axis_00860ad0` | Frame from the axis, 0085DC80, centre as translation, 00860140. rgba is `const float*` (the fourth stack slot, loaded into EDX at 00860B64; 00860140 reads four floats through it). |
| 00860BB0 | `debug_draw_circle_around_axis_00860bb0` | 00860AD0 over 0..2pi; rgba `const float*` passed through. |

## Gunnery AI, director bridge, artillery bot config

| Address | Function | Notes |
|---|---|---|
| 00861970..008619C0 | `director_bridge_get_*` | Six byte getters on the director at bridge+4 (+3Ch, +222h, +220h, +221h, +223h, +3Dh). Dead accessors. |
| 00861A10..00861B10 | `artillery_bot_get_*` | Nine float getters of the [00E19994] level row (+0Ch..+2Ch). Dead. |
| 00861B30 | `gunnery_target_record_destruct_00861b30` | vtable 00D0D2FC, tail 00695870. |
| 00861B40 | `gunnery_target_record_on_observee_destroyed_00861b40` | Clears +1Ch. |
| 008626D0 | `gunnery_target_record_reset_aim_offset_008626d0` | (0, 0, +70h) rotated about +Y by +6Ch through 0085C3F0, in place. |
| 00861C20 | `flag_grid_set_cell_00861c20` | One cell, or the whole column for a negative row. |
| 00861CD0, 00861D20 | `unit_gunnery_ai_set_artillery/aa_group_allowed` | Bit 2 / bit 1 of the category words in the .data lists. |
| 00861D70, 00861DC0 | `unit_gunnery_ai_set_torpedo/depth_charge_group_allowed` | Words = 3 or 0, then byte +77h / +78h = the argument. |
| 00863620 | `entity_as_kind02_or_null_00863620` | vtable[5Ch](2) test. |
| 00863A80 | `unit_gunnery_ai_create_director_bridge_00863a80` | Owner vtable[140h] then [114h]; 14h bridge from 00BF681B; 008624C0(bridge, 1) even for a null bridge. |
| 00864510 | `unit_gunnery_ai_vslot60_noop_00864510` | RET 4. |

## Effects and weather

| Address | Function | Notes |
|---|---|---|
| 008658C0, 008580C0, 00872220/40/60 | node handle release | One body: 00B6DFA0 then clear. |
| 008658E0 | `world_atmosphere_release_node_008658e0` | +38h tested twice, then 00B6DFA0. |
| 00865B80 | `rain_descriptor_construct_00865b80` | "raindrop.tga", ten float defaults, colour 80D0D0FFh. |
| 00865CF0 | `rain_descriptor_get_singleton_00865cf0` | Function-static guard [00F875FC], atexit 00CDEA50. |
| 00865E30 | `owned_object_delete_if_set_00865e30` | vtable[4](1) then clear. |
| 00866760, 008669A0 | singleton holder destructors | 0041CC80 on holder+4, clear [00F87654] / [00F87650]. |
| 008674A0 | `effect_manager_clear_live_references_008674a0` | 004C9550(+10h, 0) then 008671A0. |
| 00869D20 | `effects_clear_lua_table_00869d20` | 006B8AD0 "Effects=nil\n". |
| 00869D70 | `particle_admit_at_position_00869d70` | 00AF4130 fade then 00AF3F80 per emitter; +66h admits at once. |
| 0086B7C0 | `effect_component_slot0c_false_0086b7c0` | Returns false. |
| 0086B990, 0086BC50, 0086BD60, 0086CFC0, 0086D010, 0086D060 | category getters | 3, 1, 6, 6, 6, 6. |
| 0086B9A0 | `effect_tracer_adjustor_thunk_0086b9a0` | ECX - 80h then 0086CF70. |
| 0086B9B0 | `effect_tracer_destruct_0086b9b0` | 0086B7E0 on +80h, then 0086AD50. |
| 0086CDB0 | `effect_named_record14_copy_construct_0086cdb0` | Floats, NativeString via 0041DD40 + memcpy, +10h. |
| 0086E670 | `effect_named_record14_array_append_0086e670` | Doubling reserve (0086CE10), 0086CDB0 placement. |
| 0086F900 | `effect_named_record14_array_destroy_0086f900` | 0086E6F0(0), free 00BF6989. |
| 0086FC90 | `pooled_string_list_owner_construct_0086fc90` | 004C3020 sentinel for the list at +8. |
| 0086FD90 | `particle_resource_record_destroy_if_set_0086fd90` | 0086FCF0 on the last record, count - 1. |
| 00871390 | `particle_resource_records_erase_unordered_00871390` | 00870C10 move-last-over, 0086FCF0, count - 1. |
| 00871A90 | `gameplay_effect_find_id_by_name_00871a90` | 004C1650 then 00871750. |
| 00871FF0 | `registered_model_effect_set_emitter_value_00871ff0` | 00AF6790 on [+1Ch]. |
| 00872080 | `registered_type4_effect_set_alpha_00872080` | 00BA9960 on [+34h]. |
| 008721A0 | `effect_node_is_nonpositive20_008721a0` | x <= 0 (NaN returns 0). |
| 00872770 | `registered_model_effect_set_value1dc_00872770` | [[+1Ch]+1DCh] = value. |
| 00872E80 | `lightning_storm_owner_reset_00872e80` | 00BC04C0 then CRT free 00BF65AC. |
| 00866730, 0086AC50, 0086AC80, 0086ACB0, 0086ACE0, 00873AB0 | `ref_ptr_destruct` | RefPtr destructor (InterlockedDecrement [00CE2220], vtable[0] at zero), returns ref. |
| 00866970, 00866C30, 0086B080, 0086B750 | `ref_ptr_release` | The same without the return value. |
| 0086AFC0 | `ref_ptr_attach_0086afc0` | Release, then store a raw pointer without AddRef. |

## Small leaves and helpers

00851EE0 / 00852140 fixed string builder appends; 00855FE0 observer link assign (006952A0 /
00694A60); 00858330 traceline scalar delete to the model pool (00AF1980, 00B748C0); 00858180,
008581B0, 00858200 RET leaves and 008581E0 returns false.

## Hosted (ledger only, no second copy)

- 00863840 / 008638B0 / 00863920 (artillery / depth-charge / torpedo group accepts target): the
  binary walks the .data category list, tests byte +70h+c, [+60h]->vtable[4](c), then
  008633D0(c, target). `GameGunneryHost::group_accepts_target_008637d0` reaches the same order,
  reading +77h/+78h (written by 00861D70 / 00861DC0) for categories 7 and 8. No mismatch seen.
- 00864BA0: __thiscall(ai; x, y, z) RET 0Ch, 00864680(ECX = [ai+68h]; point by value).
  `game_hosts_ship_ai.cpp` binds it to `unit_sees_point_00864680`.
- 00862660: copies the record offset +74h..+7Ch into [gun+398h]+84h..+8Ch and, when byte +88h is
  clear, sets it to (+84h < gun+474h) (FCOMIP/JBE, NaN gives 0). Hosted in
  `game_hosts_gunnery.cpp` (add_gun_to_target_record_00864ca0); equivalent.

## Skipped

- 00859950 (FUN_00859950): no Ghidra function, no callers; a sync-delta builder over a large stack
  frame (0077B280, 00927850). Not reconstructed.
- 0085C5D0 BSP_Ellipsoid_IntersectSegment, 00860140 ArcInFrame, 008605C0 AabbWireframe,
  00860710 Circle64, 008608B0 CircleWithHeading, 0085FB70 / 0085FFF0 arrows, 00860BE0 wire
  sphere, 00861260 text at a world point: x87-heavy debug geometry; not done in this pass.
- 00863130 BSP_UnitGunneryAi_LoadProperties (property reader with operator new and 12 keyed
  reads) and 00864380 BSP_UnitGunneryAi_Destruct (checked STL tree iteration): not done.
- 008624C0 BSP_UnitGunneryDirectorBridge_Apply: an existing semantic model
  (`apply_director_stance_008624c0`, unit_gunnery_pass.cpp); not re-verified here.
- 0086C120, 0086CE10, 0086E6F0 and the engine array instantiations (TMPL_*, Native*Vector*):
  container template bodies; not done.
