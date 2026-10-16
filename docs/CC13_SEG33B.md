# CC13 segment 33, upper half (cc13-seg33b)

Packet cc13-seg33b, range 006EC000..006FA320 of segment 33 (the lower half, 006DECA0..006EC000,
is cc13-seg33a). Code: `src/cc13_seg33b_misc.cpp`, `src/cc13_seg33b_command_building.cpp`,
header `include/bsp/cc13_seg33b.hpp` (namespace `bsp::cc13_seg33b`).

Model: every routine works on the original object as raw bytes (Win32 layout, image offsets)
and reaches the engine through `Seg33bHost`, whose methods name the image routine, global or
virtual slot they stand for. These are new C++ interfaces, build-tested only: not ABI-compatible,
not fixture-tested, not game-validated, and nothing is wired into `bsp_game`. Names are
hypotheses, not recovered symbols.

Counts: 118 functions in the pool at or above 006EC000. 103 reconstructed, 14 recorded
ledger-only because a cc9 host or an existing table already implements them (adoption notes
below), 1 skipped.

Simplifications that apply throughout: SEH unwind actions (freeing a block when a constructor
throws, destroying a half-built pair) are not modelled; every routine says so where the image
has one. Where the image runs x87 arithmetic in extended precision and this code uses double,
the routine's comment says so.

## MCatapult (006EC140..006ED690)

| Address | C++ | Notes |
| --- | --- | --- |
| 006EC140 | `catapult_debug_dump_006ec140` | 0072ADC0, then a "_catapult" section with ammo/orgAmmo/startLaunch/launchTick. |
| 006EC240 | `catapult_pose_006ec240` | Yaw/pitch on the base and barrel nodes; the plane in state 3 gets (P * pitch) * yaw. |
| 006EC640 | `catapult_construct_006ec640` | Gun base, six vtables, observer member +494h, class id 28h. |
| 006EC7B0 | `catapult_is_kind_of_006ec7b0` | 28h, 20h, 1Eh, 4, 2, 1, 0 or +C4h. |
| 006EC7F0 | `catapult_destruct_body_006ec7f0` | 00653390 on +494h, then 0072D950. |
| 006EC860 | `catapult_create_006ec860` | new(4D8h), memset, construct; owner into +3F4h/+354h without a null test. |
| 006ECB50 | `catapult_apply_launch_message_006ecb50` | Id table lookup, observer swap, +48Ch, 006EC070. |
| 006ECBE0 | `catapult_init_device_006ecbe0` | Barrel arrays, arc angles, launch node, reload timers, 00729700. |
| 006ED000 | `catapult_launch_heading_006ed000` | Wrapped heading inside an arc window, else the nearest edge within 10.0f. |
| 006ED690 | `catapult_load_saved_state_006ed690` | "_catapult" saved table; empty magazine sets a FLT_MAX reload. |

006EC240's Ghidra pseudocode garbled the base-plus-barrel arm; the C++ follows the listing:
each node keeps its own local position and takes one rotation. 006ED000's wrap constants are
float pi widened to double (00CE3D18 -pi, 00CE3D28 pi, 00CE3828 2pi).

## LuaVariant map and vector instances (006ED890..006EE890) and the publishers

| Address | C++ | Notes |
| --- | --- | --- |
| 006ED890 | `lua_map_key_less_006ed890` | Comparator; ECX unused. |
| 006EDE40, 006EDEF0 | `lua_map_iterator_increment_*` | Identical bodies around 006EDA20. |
| 006EDF00 | `lua_variant_vector_push_back_006edf00` | Reserve max(2*cap, 1), copy-construct 14h-byte variant. |
| 006EDFB0 | `lua_map_iterator_decrement_006edfb0` | 006EDD80. |
| 006EDFC0, 006EE120 | `lua_map_init_head_006edfc0`, `lua_map_construct_006ee120` | Head node, size 0; the second has RET 8. |
| 006EE040 | `lua_map_pair_destruct_006ee040` | Value destructor, key string to the pool. |
| 006EE0B0 | `lua_variant_vector_tidy_006ee0b0` | resize(0), free. |
| 006EE0D0, 006EE150 | `lua_map_pair_construct_006ee0d0`, `lua_map_pair_copy_construct_006ee150` | Key (00884EF0) then value (00886DA0). |
| 006EE230 | `lua_map_buy_node_006ee230` | 34h-byte node. |
| 006EE4E0 | `lua_map_insert_unique_006ee4e0` | std::map::insert with 006EE2E0 as _Insert. |
| 006EE890 | `lua_map_erase_subtree_006ee890` | _Erase. |
| 006EEB20 | `mission_publish_input_event_006eeb20` | Mission.Ptr message call with {F, Message = 4, Shift, Ctrl, Alt}. No static caller. |
| 006EDB40 | `debug_swap_local_side_006edb40` | Debug side swap (game state 0Dh input 0Dh). |

Node layout: left +0h, parent +4h, right +8h, key +0Ch (10h bytes; string length +4h, data
+8h), value +1Ch (14h bytes), color +30h, is-nil +31h. 006EEB20 looks up `Mission.Dead`
(00CF829C) and never uses it; the C++ does the same lookup and destroy. 006EDB40 passes the
same selected field twice to slot 0 of unit+1E4h, as the image does.

## Fog-cloud model pool (006EF2A0..006F1440)

Same allocator template as the fish and flag pools of `engine_seg11_35_66_cc13.hpp`
(geometry: 194h-byte slots, two per 330h-byte block, free stack +328h, count +32Ch, slot
index +190h), but reconstructed separately because this instance uses different CRT entry
points (00BF55BE / 00BF6989 for the block array, 00BF681B / 00BF65AC for blocks).

| Address | C++ | Notes |
| --- | --- | --- |
| 006EF2A0 | `fog_cloud_render_006ef2a0` | 00B748E0(a1, a2, (float)unsigned a4, 0); the third argument is unused (Ghidra's parameter map was wrong). |
| 006EF3F0 | `fog_pool_init_block_006ef3f0` | Free stack {1, 0}. |
| 006F03A0 | `fog_pool_free_slot_006f03a0` | Under the section. |
| 006F06F0 | `fog_cloud_operator_delete_006f06f0` | Static pool 00E199B0. |
| 006F0700, 006F0830 | `fog_pool_construct_006f0700`, `fog_pool_destruct_006f0830` | Allocator-list link at 00E188B4. |
| 006F08C0 | `fog_pool_trim_006f08c0` | Vtable slot 0; no lock. |
| 006F0960 | `fog_pool_allocate_slot_006f0960` | Grows 2n+2. |
| 006F0C30 | `fog_cloud_deleting_destructor_006f0c30` | 00B750C0 then the pool. |
| 006F13E0 | `fog_cloud_record_assign_006f13e0` | String resize/memcpy, six fields. |
| 006F1440 | `fog_cloud_create_006f1440` | "Tej" model, a 20 x 20 quad at height z, cloud.dds. |

## MCommandBuilding and land-fort routines

Reconstructed (no host implements them):

| Address | C++ | Notes |
| --- | --- | --- |
| 006F1EE0, 006F1F00 | `command_building_lock_006f1ee0`, `command_building_unlock_006f1f00` | +764h section with depth +18h. |
| 006F1F80 | `command_building_die_006f1f80` | A single RET. |
| 006F2250 | `free_and_null_006f2250` | |
| 006F2330, 006F3140, 006F3410 | `dword_vector_*` | Saturated size, 2n+2 growth. |
| 006F23E0 | `random_offset_pair_init_006f23e0` | {range, -U(0, range)}, stream 1 of 00BD2F10. |
| 006F2780 | `command_building_init_from_properties_006f2780` | Scene fields, +790h for effective game modes 0..3. |
| 006F3010 | `command_building_refresh_pads_006f3010` | 006AC4D0 per pad. |
| 006F30A0, 006F3320, 006F34F0 | `observer_member_set_target_*` | Identical bodies. |
| 006F30D0, 006F3360, 006F3530, 006F33A0, 006F32B0, 006F3FC0 | `pointer_vector_*` | |
| 006F31B0, 006F35E0 | `garrison_entry_destruct_006f31b0`, `garrison_entry_copy_construct_006f35e0` | 64h-byte entry. |
| 006F3490 | `entity_as_kind_0c_006f3490` | |
| 006F34B0, 006F39B0 | `command_building_remove_paratrooper_006f34b0`, `command_building_add_paratrooper_006f39b0` | +7DCh under the lock. |
| 006F3570 | `word_array_push_back_006f3570` | |
| 006F3A20 | `command_building_any_pad_group_006f3a20` | Vtable +210h through 0048CBB0. |
| 006F3FE0, 006F4B30, 006F4BA0, 006F55F0 | `garrison_vector_*` | Inline entry destructor. |
| 006F4110, 006F4220, 006F4270, 006F4300 | `inferior_list_message_*` | D6h message: 8-bit count, 12-bit ids. |
| 006F4360 | `command_building_class_read_lua_006f4360` | Class Lua fields and SubType. |
| 006F4A00 | `command_building_garrison_inferior_failure_006f4a00` | "InferiorFailure" through vtable +198h. |
| 006F4CC0 | `command_building_update_006f4cc0` | 00745BE0 plus a stripped debug test. |
| 006F5460 | `command_building_handle_message_006f5460` | D3h..D6h, else 00744BE0. |
| 006F5610, 006F5AE0, 006F5C10, 006F6740 | construct, destruct, allocate, init-all | |
| 006F5790, 006F57C0, 006F57F0, 006F5980 | land-fort virtuals | |
| 006F59F0, 006F5AA0 | world-list detach | |
| 006F7670 | `command_building_globals_load_006f7670` | CommandBuildingGlobals into the tuning singleton 00E19A64. |

006F5610 draws once from the shared RNG 00BD2F10 (stream 1) for +7C0h. The ship-AI host's
LABELLED choice not to take that draw (game_hosts_ship_ai.cpp, the +7C0h seed) is already known;
it is not re-reported.

## Command list, command stream and convex object

| Address | C++ | Notes |
| --- | --- | --- |
| 006F7D10 | `command_list_new_node_006f7d10` | A null allocation still stores at address 8h, reproduced. |
| 006F7D40 | `command_list_append_006f7d40` | Stores go through a null node, as the image. |
| 006F7DD0 | `target_descriptor_resolve_position_006f7dd0` | |
| 006F99C0 | `command_stream_read_class_006f99c0` | The "name" field to 00467170. |
| 006F9AC0, 006F9B20, 006F9BF0 | stream readers | 006F9B20 reads a fifth value and drops it. |
| 006F9C20, 006F9C40, 006F9C80, 006F9CA0 | array holders | |
| 006F9CD0, 006F9D70 | `convex_object_construct_006f9cd0`, `convex_object_destruct_006f9d70` | 006F9CD0 has no stack argument (Ghidra listed one). |
| 006F9D20, 006F9D30 | `return_zero_*` | RET 8 / RET. |
| 006F9D40 | `convex_object_is_kind_of_006f9d40` | Three tokens at 00E19A98. |
| 006FA320 | `convex_hull_debug_draw_006fa320` | Every edge once (j >= i), white. |

## Recorded ledger-only, with adoption notes

Each of these is implemented by a cc9 host or an existing table; no second copy was written.
The notes list what the image does that the host does not. None of them touches the shared RNG
00BD2F10.

- **006F2940 SendNeutralize** (`command_building_neutralize_006f3270`, command_building_capture.cpp).
  Image 006F2955..006F29AF: +7A8h = 0.0f before the party test, then (party != 2) a D3h
  message {slot 9} built on the stack and routed by 0077C2A0(this)(msg, 7, 0). Host: progress 0
  and the slot-9 effect applied directly. Equivalent in single player as far as the flip goes;
  the routing (multiplayer replication) is absent. LIVE whenever a CommandBuilding's health
  reaches 0.
- **006F29D0 HandleMessageD5** (host: the capture tick stores progress/value/side).
  Image 006F29E6..006F2A21: when the side equals the local record's +28h, the old +7A8h is
  0.0f, the new progress is not 0.0f (UCOMISS; NaN counts as not zero) and the party is 2,
  00977DA0(ECX = [00F8A0C4])(this, 0) reports the capture warning. The host never makes this
  call. LIVE in every capture of a neutral building by the local side (presentation: the HUD
  warning).
- **006F2A50 TakeLandingPad** (`BuildingPadModel::pick_006f2e60(..., true)` at the 0074A540
  site and a first-free lambda at the 008207B5 site, game_hosts_ship_ai.cpp). Image
  006F2A67..006F2BC6: under the +764h lock; a null unit takes the first free pad; otherwise the
  nearest free pad by squared 3-D distance with a strict `<` against a running best seeded with
  FLT_MAX (00D7A248), so a pad at infinite or NaN distance is never chosen; then 006AC490(pad)
  (unit) assigns it before the lock is released. Host: pick_006f2e60 accepts the first free pad
  whatever its distance and assigns later (begin_landing_0074a990). Differences only for
  non-finite distances and for the assignment's timing. LIVE in landing-ship missions.
- **006F2C30 FindNearestLandingSite** (inline at 0074A50B and 008207B5, game_hosts_ship_ai.cpp).
  Image: world list 28 ([[00E188A8]+19CCh]+16Ch), other party, squared 3-D distance against
  (float)range squared with the range field picked by the kind argument (0: +7C8h, 1: +7A0h,
  other: +7C4h), first strict minimum; it also writes a status (3 none, 2 only own-party
  buildings, 1 out of range, 0 found). Host: kind 2 only, matches; the status is not produced.
  LIVE in landing-ship missions.
- **006F2D90 HasFreePad** (inline at 009F2095). Matches.
- **006F47F0 Repair** (`GameGunneryHost::repair_unit_to_fraction`). Image 006F4813..006F49B3:
  f = tuning[+2Ch + level*4] * dt / rate; hull arm 00877B90(min(max, f*max + health)) when
  0 < f and health < max; then f2 = tuning[+2Ch + level*4] * dt / tuning+40h (GunRepairTime) and
  every device of unit+48h (next +44h) with IsKindOf(4) is healed by the same rule through
  00877B90, and a device whose flag +5Dh is set and whose health reached max gets vtable
  +19Ch("destroyed", 0). Host: the hull arm only, with the caller's fraction (1.0 on a flip).
  The device arm is absent. LIVE on every flip (006F4D10 calls 006F47F0; the arguments (this, 1.0, 1.0) are as the host comment records them, not re-read here) and in
  006F7360's repair arm below.
- **006F57A0, 006F57B0, 006F57D0, 006F5830** land-fort constants and the master-or-self
  accessor: `land_and_structures.hpp`, `sensor_tables.hpp`, `hud_minimap.hpp`,
  `recon_slot_lists.cpp`. Match. (hud_minimap.hpp says 006F57D0 has no Ghidra function; it has
  one now.)
- **006F5920 IsSelectable** (`GameHudHost::Impl::unit_vtable_124`). Image 006F5920..006F5937:
  false when +790h is set and the class +70h is 58h. Host keeps only the four-byte arm, arguing
  +790h is clear in single player. The comment there says the setter needs effective game mode
  2 or 3; 006F28EE..006F292D actually sets +790h for modes 0, 1, 2 and 3. With 004BCA50
  reporting 8 in single player the conclusion holds. Not live in single player.
- **006F7360 FixedStepCallback_RepeatingStep** (ECX = building+310h; the countdown, the level
  arm and the +7D8h clamp are in game_hosts_ship_ai.cpp). Image arms the host lacks:
  (1) 006F738D..006F742D: with +7D4h set (written 1 at 006F3844 in RespawnGarrison 006F3660;
  nothing in 006F7360 clears it) and no observed garrison unit with id 0, a D6h message
  (006F4110) routed by 0077C2A0(msg, 4, 0), destroyed (006F4300), and the HUD panel refresh when
  the building is selected; (2) 006F7432..006F755C: with unit+378h set (written 1 for every unit
  at 0087BD19 in BSP_UnitInstance_InitHealthAndParts, so this arm is very likely LIVE) the
  building repairs itself every step through 006F47F0(this, dt, class+180h) and each garrison
  unit through 006F47F0(unit, dt, rate) with rate class+188h for an airfield (45h, or a 1Bh
  fort whose vtable +204h answers 45h), class+18Ch for a shipyard (46h likewise), else class
  +184h. The host's buildings do not regenerate between captures. Mode 2 skips straight to the
  +7D8h clamp (006F7387).
- **006F7F40 CommandClass_BaseRequiresTarget** (`game_hosts_commands.cpp` class table row).
  Match.
- **006F9EE0 ConvexObject_ComputeBounds** (`hull_terrain_contact.cpp` 006F9EE0 block). The
  centre (+14h..+1Ch) matches; the half extents (hi - lo) * 0.5 stored at +20h..+28h are not
  produced by the host. Readers of +20h were not traced.

The known 006F6760 slot-pick mismatch (unoccupied and AI-controlled-not-enabled slots, owner
slot power) is outside this range and not re-reported.

## Skipped

- **006F3CC0 CommandBuilding_DebugDrawId**: a debug-only overlay (gated by the debug owner
  0051F460 +2Ch) whose arrow geometry is x87 register arithmetic (006F3D54..006F3D6E,
  006F3ECC..) that needs a byte-level DC-form check before it can be transcribed; not done in
  this packet.
