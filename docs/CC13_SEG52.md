# CC13 segment 52 (008288D0-00851CB0): ship units, shipyard, stationaries

Worker cc13-seg53, 2026-10-16, after segment 53 (docs/CC13_SEG53.md). Struck ranges 00836000-00837FFF
and 0084E000-0084EFFF (live cc9 packets) were not touched. New code: `src/cc13_seg52.cpp`,
`include/bsp/cc13_seg52.hpp` (namespace `bsp::cc13_seg52`), every call through a named `Seg52Host`
hook. Status for every row: reconstructed and build-tested, not ABI-compatible, not fixture-tested,
not wired, not game-validated. Names are hypotheses.

## Reconstructed

| Address | Function | Notes |
|---|---|---|
| 0082AD20 | `native_string_pair_destruct_0082ad20` | Releases +8 then +0 through 00419CC0 / 00BD1510. |
| 0082B020 | `native_string_pair_assign_0082b020` | 0041DD40 + memcpy copy idiom on both strings. |
| 0082B170 | `native_string_pair_construct_0082b170` | Zeroed strings, then the copy idiom from two sources. |
| 0082B270 | `native_string_pair_copy_construct_0082b270` | The same from one pair. |
| 0082EC10 | `ship_traffic_record_copy_construct_at_0082ec10` | 0049FA70 then +4Ch, under the placement-new null test. |
| 0082EC90 | `ship_traffic_record_construct_0082ec90` | 0049F9B0 then +4Ch = 0. |
| 008346E0 | `ship_unit_bind_screw_and_rudder_nodes_008346e0` | Seven 0071AD50 node lookups (csavar_1..4, rudder, rudder_1, rudder_2), five floats zeroed. |
| 00835700 | `weapon_director_move_forbidden_00835700` | byte +3Dh == 0. |
| 00835710 | `weapon_director_set_fire_and_move_00835710` | vtable[40h](1) then vtable[44h](!value). |
| 00835840 | `weapon_director_detected_target_00835840` | Tail call 00803D40(party, [director+238h]). |
| 00835F10 | `ship_weapon_director_destruct_00835f10` | 006E0860 on +224h then 0071D480. |
| 00846D30 | `shipyard_unlink_from_world_lists_00846d30` | 006D3620, then 004837D0 on two world lists. |
| 0084C500 | `gun_apply_aim_message_if_function6_0084c500` | 0072D5B0(1, flag) for function 6. |
| 0084C640 | `st_gun_describe_state_0084c640` | 0085A130 then the writer's slots for lastFiredBarrel / singleShot. |
| 0084C6E0 | `rapid_turning_gun_apply_aim_and_fire_message_0084c6e0` | 0085B0F0, 0072D5B0, 00BF681B shot record at +41Ch, vtable[1D8h]. |
| 0084C7E0 | `rapid_turning_gun_handle_network_message_0084c7e0` | Message B0h to 0084C6E0, else 0072D830. |
| 0084C800 | `st_gun_read_archive_0084c800` | 0085BB50, then the Lua reader for "_singleTurningGun". |
| 0084C920 | `small_recon_plane_unit_instance_construct_0084c920` | 0074E0F0, eight vptrs, class id 15h, +AACh = 0Bh. |
| 0084C990 | `small_recon_plane_class_recon_category_0084c990` | Returns 0Bh. |
| 0084CA50 | `small_recon_plane_class_create_unit_instance_0084ca50` | operator new(E94h), 0084C920, 009553D0. |
| 0084CB30 | `small_recon_plane_unlink_from_world_lists_0084cb30` | 0074E610, then 004837D0. |
| 0084D8F0, 0084D910, 0084D930 | squadron block category filters | True for {0, 1}, {1, 2}, {0, 2, 3}. |
| 0084D960 | `squadron_block_slot50_0084d960` | 0 when +3Dh and +3Ch are both set. |
| 0084DB50 | `pilot_order_apply_and_raise_attack_mode_0084db50` | 0071C130, 007EFB60 when landing, 009F6DD0, 007ED3F0(1). |
| 0084F080 | `stationaries_read_archive_0084f080` | Per stationary "damage" field, clamped to the wreck threshold. |
| 0084F1A0 | `stationaries_write_archive_0084f1a0` | Per stationary "damage" field written. |
| 0084F750 | `stationary_apply_class_0084f750` | Class fields copied, RefPtr at +184h with Interlocked counts. |
| 0084F930 | `native_string_release_storage_0084f930` | 00419CC0 / 00BD1510 release idiom. |
| 0084FA70, 0084FAC0 | string + dword record constructors | Copy idiom plus +8. |
| 00851500 | `stationary_class_create_entity_00851500` | operator new(1ACh), 00748A40, 0084F750. |

## Hosted (ledger only)

- 00835940 / 008359C0 / 00835A40, the torpedo / ship / land avoidance senders. Each is
  __thiscall(ECX director; byte value) RET 4: stack message 0075B430(5Ah), +18h word 0, +1Ah and
  +1Ch bytes 0, +4 = 1, vtable 00CFD9C4, +20h = 7 / 8 / 9, +24h = MOVZX value, then
  0077C2A0(ECX = [director+34h]; &message, 7, 0). Existing model:
  `director_command_message_0071da50` (weapon_director.cpp), which gets the sub-kind and value
  right but is a build-struct model without the base constructor, the zero stores or the 0077C2A0
  route; game_hosts_script_orders.cpp delivers through a labelled loopback. No behaviour mismatch.
  Reported to the lead as torpedo AI.
- 0082ADA0 / 0082ADC0: 004218E0() then 004120D0(ECX = manager; [class+560h+4*depth] /
  [class+570h]). Hosted in game_hosts_ship_ai.cpp and game_hosts_ai.cpp.

## Skipped

- STL-member bodies: 0082DF00 / 0082E280 / 0082E5A0 (idle record with two std::map members),
  008390C0 / 00839160 (checked-iterator table lookups), 0084FA30 (vector erase), 00848D70 and
  00848D00 / 008486B0 (stock record copies through std::vector operator[] and 008484A0), 00846A80
  (shipyard destructor through three vector tidies).
- Large bodies not done in this pass: 0084DDA0 BeginCommand, 008509F0 SEntityInit, 00851580
  ReadLuaFields, 0084FCE0 LoadExplosionResource, 00851850 OnWrecked, 00845C10, 00846640,
  00843000 / 00843460 / 00842A80 / 008429E0 / 00845670 / 00847FE0 / 008482F0 (unnamed),
  0082DC30 / 0082DC80, the shipyard construct / allocate / hangar walk, 0082D040 (buoyancy, hosted
  in game_hosts_units.cpp) and 0082FE30.
- The engine container instantiations (TMPL_RefPtr_*, *PointArray_PushBack_*).
