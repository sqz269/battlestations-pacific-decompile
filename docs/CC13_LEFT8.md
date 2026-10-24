# CC13 left8: segment 8 leftovers (00491170..004BA200)

Packet cc13-left8 took the segment-8 functions the first pass (`docs/CC13_SEG8.md`) left without a reconstruction record. Code: `src/cc13_left8_seg8.cpp`, `include/bsp/cc13_left8_seg8.hpp` (namespace `bsp::cc13_left8`). Objects are reached through byte offsets in their Win32 layout; every call the image makes goes through a `Left8Host` method named for its target (virtual calls by vtable offset) at the same point with the same arguments, except memset/memcpy/vsprintf_s/snprintf, which are called directly. Checked-iterator traps are `invalid_parameter_00bf6713` calls at the image's test points (the image does not return from them). Names are hypotheses, not recovered symbols. Status: build-tested only; nothing here is wired into a game host.

RNG: 00491950 via 004B0F80, 00498E60, 004AB9A0, 004B18B0 and 004B3480 draw from 00BD2F10 with ECX = 0, the shared stream 0, and the code passes stream 0 to the host at the same points. A host that wires them must apply the cc9 RNG rule (presentation draws to a keyed stream, gameplay-coupled draws behind an OFF switch). 00498790 uses the deterministic hash 00484660, not an RNG.

x87: 00494F30, 0049A360/0049A410, 00498790, 004AFCA0 and 004B18B0 compute in double and round at the image's float stores; qword constants stay double (300.0 at 00CE3CA8, 42.0 at 00CE6840, 3.0 at 00D7A2B0, 369097.0 at 00CE6428, 0.001f-as-double at 00D7A318).

## Reconstructed

| Address | C++ function | Native ABI | Behaviour and uncertainty |
|---|---|---|---|
| 00491860 | `idle_group_release_member_00491860` | thiscall(ECX group, member) RET 4 | vtable 00CE69D4 slot 2. Member set: +18h = game time [00F876A4], member vtable +114h(0). The first i < [+24h] whose +74h slot holds the member is cleared and bit i of the vector<bool> at +80h set (0048F930 + reference checks). |
| 00491950 | `ship_idle_actor_instantiate_00491950` | thiscall(ECX group, index) RET 4 | Alternate bit from the vector<bool> at +28h (0048F930, 0048F230). Empty +74h slot: class from +54h/+44h, vtable +8 creates, +98h(owner, [owner+30h], &pose[index] of [owner+538h]+6ECh), +114h(group), +A0h(), alternate -> 004B0F80(&anim[index], 1, 0), +11Ch(). Always +D8h(). |
| 00491AC0 | `layered_water_entity_update_00491ac0` | fastcall(ECX entity) RET | vtable 00CE67AC slot 1, no direct caller. Four layers: layer box out of view -> 00481430 for every member and the visible byte cleared; in view, per cell: out of view with bit set -> 00481430 over the cell's member range and bit cleared; in view -> 0048C6A0 member, +A8h = 0078CF20 water height, +D8h(), bit set. Cell bits through 0048D3B0 begin()+c. |
| 00491E70 | `landing_traffic_remove_ship_00491e70` | thiscall(ECX manager, ship key, bool destroy) RET 8 | 00488630 find in the map at +28h. destroy: under the critical section [+4] (depth +18h), value vtable +0(1) and 0048F9A0 erase. Otherwise 0048C190 on the value. |
| 00494F30 | `ship_idle_actors_update_00494f30` | fastcall(ECX group) RET | +94h = 1; camera pose refreshed (00414DB0) when +C8h clear; per member with its +80h bit clear: pose translation transformed by 00B62D10, divided by w, distance to the camera vs (300.0 / [[game+19FCh]+1C4h])^2; inside -> 00491950, else (also unordered) the slot is detached and killed (00926D90(2)). x87 rounding at the image's float stores, computed in double. |
| 00497B80 | `ship_idle_actors_tick_00497b80` | fastcall(ECX group) RET | vtable 00CE69D4 slot 1: vtable +0Ch() true -> 00494F30, false -> 00487940. |
| 00496920 | `list_node_construct_00496920` | thiscall(ECX node, parent) RET 4 -> EAX node | vtable 00CE6600; child list +0Ch (00485270 sentinel); +8 parent, +4 = 0; appended to the parent's list (00485880, 0048FD20); +18h = -1.0e10f [00CE4ADC], +1Ch = 1.0f. |
| 004969D0 | `list_group_release_member_004969d0` | thiscall(ECX group, member) RET 4 | Slot 2 of four vtables (00CE68D4, 00CE68E4, 00CE6A08, 00CE6A68). Pops free-slot numbers below [+88h] from list B (+78h), then walks the member list A (+6Ch) numbering slots past the free numbers; each node holding the member has its number inserted into B (004857F0, 0048FC80) and is erased. Self-compare iterator checks omitted (cannot fail). |
| 00498790 | `traffic_cell_member_00498790` | thiscall(ECX tree, i, j) RET 8 -> EAX member | Creates the member of cell (i, j) from the deterministic hash 00484660 of {i, j, 0..3} (vector built with 00442190): jittered x/z, class picked by 00480A90 over the weights at +30h, vtable +8/+98h/+114h/+A0h, identity copy 004134F0, yaw 00467050. Not an RNG stream. |
| 00498C40 | `traffic_visible_cells_00498c40` | thiscall(ECX tree, Node** slot, const float* box) RET 8 | Called by 004879B0. Fine box: 0047BA40 range, members 00498790 placed on the water (0078CF20) and vtable +D8h(). Coarse box: node built by operator new(70h) + 00481510 when empty, then 004879B0 on the four quadrants. |
| 00498D60 | `temp_anim_construct_00498d60` | thiscall(ECX object, parent, NativeString* class, const matrix*) RET 0Ch -> EAX | 00496920; vtable 00CE6660; +20h = 004722D0 anim class; matrix copied to +28h; +68h..+70h written with translation - r and then translation + r (r = class+28h), both stores kept. |
| 00498E60 | `plane_debris_spawner_construct_00498e60` (was marker_construct; the plane-debris spawner, see 007D0B80) | thiscall(ECX spawner, parent, observed) RET 8 -> EAX | 00496920; vtable 00CE6830; observer sub-object +30h (vtable 00CE65E8) registered with 00694A60; +4 = 1; +4Ch = U(stream 0, 0, [00E186F4] - [00E186F8]). |
| 00498F80 | `plane_debris_spawner_create_00498f80` | thiscall(ECX owner, observed) RET 4 -> EAX | operator new(50h) then 00498E60(memory, [owner+8], observed); 0 when the allocation fails. |
| 0049A360 | `path_runner_tick_0049a360` | thiscall(ECX runtime, float dt) RET 4 | Speed +130h steps toward 3.0 when touched in the last 42 s (+18h vs [00F876A4] - 42.0) else toward 1.0; +84h += dt * speed; vtable +0Ch() -> 00496BD0 else 0048A5A0. Decompiler had the branches inverted; listing followed. |
| 0049A410 | `landing_lane_tick_0049a410` | thiscall(ECX lane, float dt) RET 4 | Path rebuild 0048C230 when +180h clear and ship +118Bh set; speed step only when +180h set (else +130h = 0); t = min(+18Ch * 0.5 + +84h, +190h); t == +190h -> 0074AD90 (landing finished); else 00497BB0(t, 0) or 0048A5A0 and self-delete when t == +190h. |
| 0049A580 | `plane_debris_spawner_destruct_0049a580` | fastcall(ECX spawner) RET | vtable 00CE6830; [00E186E8] -= 1 when +49h set; members of the 54h records killed; erase(begin, end) with 0047F930; 00495D40(0) (its uninitialised by-value record argument not modelled); observer released (006952A0, 00695870); storage freed; 00489DD0. |
| 0049C6E0 | `record4c_owner_destruct_0049c6e0` | fastcall(ECX owner) RET | vtable 00CE6850; as 0049A580 over 4Ch records (inlined copy for the empty tail), 00495E20(0), storage freed, 00489DD0. |
| 0049D540 | `traffic_tree_destruct_0049d540` | fastcall(ECX tree) RET | vtable 00CE6860; 00484AE0 prunes the root; rows destroyed by 004914A0 and freed; vectors +30h and +20h freed; 00489DD0. |
| 0049DC00 | `temp_add_anim_0049dc00` | thiscall(ECX manager, LuaObject by value) RET 14h | TempAddAnim: AnimClassName, Position, operator new(80h) + 00498D60 with the translated identity; tempID (when not nil) keys the map at +34h (0049CB60). Strings returned to the pool; table destructed. |
| 0049DE90 | `remove_temp_added_0049de90` | thiscall(ECX manager, NativeString* id) RET 4 | RemoveTempAddedStuff: slot = 0049CB60(+34h, id); a set slot gets vtable +0(1) and is cleared. |
| 0049D4C0 | `traffic_record_apply_from_lua_0049d4c0` | thiscall(ECX record, source) RET 4 | Caller 00832F54. Property block on the stack: 008F41A0(&block, 0), 0043D8F0(ECX = source, EDX = &block), 0049CF80(record, &block), 008F5410(&block). |
| 0049F0E0 | `traffic_manager_destruct_0049f0e0` | fastcall(ECX manager) RET | vtable 00CE68D0; 0041CC80 on +4; owned [+8] deleted; six maps emptied with 0049A230 x2, 00499B90, 004906D0, 00496390, 00493D50 and their heads freed. |
| 0049F260 | `traffic_refresh_if_set_0049f260` | fastcall(ECX object) RET | Tail call 0049E560(object) when [+0Ch] is set. |
| 0049F9B0 | `traffic_record_construct_0049f9b0` | fastcall(ECX record) RET -> EAX | 4Ch traffic record: map +34h (004858D0 head), map +40h (00443E20 head), scalars zeroed (+10h untouched). |
| 0049FA70 | `traffic_record_copy_0049fa70` | thiscall(ECX record, const record*) RET 4 -> EAX | Copy constructor: +0..+30h copied, then 004990B0 and 0049DEC0 copy the two maps. |
| 0049F910 | `traffic_record_destruct_0049f910` | fastcall(ECX record) RET | Maps +40h (00444B10) and +34h (00492A00) emptied and their heads freed. |
| 0049FB20 | `path_runtime_destruct_0049fb20` | fastcall(ECX runtime) RET | vtable 00CE68D4; 0048A5A0; vectors +120h/+110h (00432050)/+100h/+F0h/+E0h freed; records +8Ch and +20h destructed (0049F910); 00487110(+78h), 00485340(+6Ch); 00489DD0. |
| 004A02E0 | `soldier_type_destruct_004a02e0` | fastcall(ECX object) RET | The four element vectors +6Ch, +5Ch, +4Ch, +3Ch destroyed (00432050) and freed, then 0048EE90 (TrafficPathSet base). |
| 004A4520 | `landing_troops_launch_004a4520` | thiscall(ECX manager, config, a3, ship) RET 0Ch | Caller 0074A990. Under the critical section, per pad of config+1FCh: new group (operator new E4h + 004A0410, inserted with 004926A0) unless one exists; an existing group for the same ship class (+44h vs [[ship+538h]+814h]) only gets +DCh = ship; another one is told its ship left (0048C190), marked +20h = 1 and replaced. |
| 004A5B90 | `traffic_registry_kill_member_004a5b90` | thiscall(ECX registry, key, index) RET 8 | 004A5750 group vector; the first type-3 group with index < +24h: member +74h[index], group vtable +8(member), member +114h(0) and 00926D90(2). |
| 004A6510 | `path_runtime_owned_destruct_004a6510` | fastcall(ECX runtime) RET | vtable 00CE6A08; 004A5B30([[00E188A8]+21D0h], [+14Ch], runtime); 0049FB20. |
| 004A6580 | `ship_idle_group_destruct_004a6580` | fastcall(ECX group) RET | vtable 00CE69D4; 00487940; 004A5B30; vector<bool> +80h and +28h emptied; vectors +70h, +60h (00432050), +50h, +40h freed; 00489DD0. |
| 004A67C0 | `landing_lane_destruct_004a67c0` | fastcall(ECX lane) RET | vtable 00CE6A68; +180h set -> 00926D90(007ACA30([+20h]), 2); 004A6510. |
| 004AB9A0 | `weighted_entry_pick_004ab9a0` | fastcall(ECX table) RET -> EAX entry | Caller 0049C000. r = _ftol(U(stream 0, 0, (float)[+10h])); stop at the first entry whose +8 < r (the image's comparison), else r -= +8; past the end -> last. |
| 004A7D30 | `plane_part_class_read_lua_fields_004a7d30` | thiscall(ECX class, LuaObject* table) RET 4 | 0048F670; TimeToLive, StartVelocityAdd, StartVelocityDirAdd, AirDrag pairs, GravityMul (default 1.0), RotationSpeed, RotationDecline pairs; WaterSplashEfx / SmokeEfx handles (00B66380 default 0, 00870CD0 flag 0). |
| 004ACAB0 | `ship_part_class_read_lua_fields_004acab0` | thiscall(ECX class, LuaObject* table) RET 4 | 0048F670; StartVelocity, StartVelocityAngle, AirDrag pairs, GravityMul (default 1.0), RotationSpeed, RotationDecline pairs; WaterSplashEfx / SmokeEfx handles. |
| 004AEA00 | `ship_part_class_registry_destruct_004aea00` | fastcall(ECX registry) RET | vtable 00CE6F6C; classes deleted (increment 004ABDA0, subtree 004ADB80), 004AD2A0 deletes the 00E187DC objects, 004AE3E0 erase, head freed, [00E187E8] = 0, vtable 00CE3818. |
| 004AED50 | `ship_explosion_read_parts_table_004aed50` | custom(ECX index, DL force) RET -> EAX record | Existing record in the 00E187DC vector returned; else ShipExplosion["%d"] (nil and not forced -> 0): record 24h with PartsNum[1]/[2] and up to 99 Part%d entries (Weight >= 0, Class via 004AEC00), pushed onto +14h and the global vector. sprintf buffers bounded (image: 8 bytes). |
| 004AF290 | `ship_explosion_load_globals_004af290` | cdecl() RET | DeviceExplosionChanceDistRange[1]/[2] -> [00E187D4]/[00E187D0], DeviceExplosionChanceDistMul[1]/[2] -> [00E187CC]/[00E187C8]; 004AED50(i, 0) until 0. |
| 004AF890 | `soldier_class_create_instance_004af890` | fastcall(ECX class) RET -> EAX | operator new(1B0h) 00BF55BE + memset before the null test; 00489F30, vtables 00CE7008/00CE6FEC/00CE6FE4, id 3Ah; 0047B9E0 and +188h even for null. |
| 004AFCA0 | `soldier_advance_animation_phase_004afca0` | fastcall(ECX soldier) RET (tail jump 00484940) | +18Ch set: phase +1A8h advances by the frame time (scale 0) or distance / scale (0042B2F0); fmod by +194h when looping, else clamped to +194h - 0.001; anim node +24h(max(+190h, 0), phase, 1); last position copied. Then 00484940. |
| 004AFFE0 | `soldier_sentity_init_004affe0` | fastcall(ECX soldier) RET | +190h = -1, +18Ch = [[+188h]+30h] != 0, 0048DBD0; 00B6DCA0 on the anim node or its +34h child (by 00B6D980) with [00485520()+4]. |
| 004B0DE0 | `soldier_class_read_lua_004b0de0` | thiscall(ECX class, LuaObject* table) RET 4 | 0048F670; LoopLengths iterated (00B67080/00B67190/00B66420): each key string -> 00444BE0 slot in the map at +28h = (float)value. |
| 004B0F80 | `soldier_start_animation_004b0f80` | thiscall(ECX soldier, const name*, bool random_start, bool force) RET 0Ch | +18Ch set: anim id 00B78670; on change or force: duration 00B6DC90, rate 00444BE0, start U(stream 0, 0, duration) when random and duration > 0, node +24h(id, start, 1), last position. +1ADh = random_start. The image draws on the shared stream 0. |
| 004B1400 | `soldier_class_get_or_load_004b1400` | fastcall(ECX NativeString* name) RET -> EAX | Registry 004B1330 / 004B0CB0; on a miss SoldierClass[name] row, operator new(34h) + 004B12A0, name copied, vtable +4(row), stored. Every path ends with vtable +0Ch(). |
| 004B1540 | `soldier_registry_destroy_004b1540` | cdecl() RET | Every registered class deleted (increment 004AFB50), subtree freed (004B0600), head reset. |
| 004B1880 | `matrix_orthonormalize_return_004b1880` | thiscall(ECX matrix) RET -> EAX matrix | 0085DAD0(matrix). |
| 004B18B0 | `wreck_tick_phases_004b18b0` | thiscall(ECX wreck, float dt) RET 4 | +198h -= dt; below 0: phase +194h -= 1; 1 -> stop the fire effect, timer U(stream 0, v, v) with v = class+48h; 0 -> stop the smoke effect, U(w, w) with w = class+50h; otherwise 00926D90(2). Both bounds equal as in the image. |
| 004B2030 | `wreck_class_create_instance_004b2030` | thiscall(ECX class, source) RET 4 -> EAX | operator new(19Ch) + memset, 00489F30, vtables 00CE71A8/00CE7190/00CE7188, id 3Bh, 0047B9E0, 00926BE0; with a source: its matrix, re-parented into the container found by 009039D0 when it has no owner (00B63D50 inverse, 00439820 local position); vtable +98h; +C0h = 0. |
| 004B21E0 | `wreck_class_read_lua_fields_004b21e0` | thiscall(ECX class, LuaObject* table) RET 4 | 0048F670; Fire and Smoke sections (Chance, EmitterEfx handle via 00870CD0 flag 1, TimeMin, TimeMax; chance 0 when nil); Remain TimeMin/TimeMax. |
| 004B2760 | `wreck_place_on_surface_004b2760` | fastcall(ECX wreck) RET (tail jump 0042ED50) | Owner [+3Ch] of kind 44h: +A8h = 0087FA20 height - owner+A8h; normal 0087FB90 into row +84h; 0085DAD0; 0042ED50. |
| 004B3480 | `wreck_sentity_init_004b3480` | fastcall(ECX wreck) RET | 0048DBD0; phase and timer from U(stream 0) and the class chances (no record), the scene properties Fire/Smoke/Duration (type 1) or the save keys _wreck/state/timeLeft (type 3); 004B2760; +D8h; effects released and recreated (008687C0) for phase >= 2 (Fire, +18Ch) and > 0 (Smoke, +190h). |
| 004B3D20 | `wreck_class_get_or_create_004b3d20` | fastcall(ECX NativeString* name) RET -> EAX | Registry 004B3C50 / 004B39B0; on a miss WreckClass[name], operator new(54h) + 00489E60, vtable 00CE72CC, +28h/+34h/+38h cleared, name copied, vtable +4(row), stored. No activation. |
| 004B3E60 | `wreck_class_registry_clear_004b3e60` | cdecl() RET | As 004B1540 with increment 004B1B70 and subtree free 004B2DD0. |
| 004B3F10 | `wreck_class_get_or_create_activated_004b3f10` | ECX passed through to 004B3D20, RET -> EAX | 004B3D20, then vtable +0Ch when the class is non-null with +28h clear. |
| 004B4750 | `mission_scoring_commit_record_004b4750` | fastcall(ECX game) RET -> EAX | [game+21A0h] + [[game+21A0h]+1424h] * 284h + 4. |
| 004B4820 | `render_camera_update_job_run_004b4820` | thiscall(ECX job, unused) RET 4 | 0068A0D0(ECX = [00E198C4]). |
| 004B48E0 | `game_is_aabb_in_view_004b48e0` | thiscall(ECX game, const float* box) RET 4 -> AL | Camera +1ED4h and its +1BCh set: 00B71590(ECX = [camera+1BCh], box) != 0AAAh. |
| 004B4940 | `game_set_camera_near_plane_004b4940` | thiscall(ECX game, float) RET 4 | 00B6FBF0(ECX = [game+19FCh], near) when set. |
| 004B6720 | `segment_crossing_2d_float3_004b6720` | fastcall(ECX a0, EDX a1, b0, b1, out) RET 0Ch | x/z of four float3 points into float2, 004F3730, out = (x, 0, z). Uninitialised crossing locals start at 0 here (MINOR). |
| 004B6980 | `cloud_light_colours_construct_004b6980` | thiscall(ECX colours) RET -> EAX | (0.2, 0.2, 0.2, 0), (0.8, 0.8, 0.8, 0), (0.1, 0.1, 0.3, 0) from 00CE54A0 / 00CE74F8 / 00D7A2F0 / 00CE69C8. |
| 004B6E30 | `frontend_screen_mark_visible_004b6e30` | fastcall(ECX screen) RET (tail jump vtable +18h) | Bytes +4 and +5 = 1, 004F83B0(screen), then vtable +18h. |
| 004B7F70 | `format_string_512_004b7f70` | cdecl(dst, fmt, ...) RET -> EAX | _vsprintf_s(dst, 200h, fmt, va), result passed through. |
| 004B8410 | `format_string_255_004b8410` | cdecl(dst, fmt, ...) RET -> EAX | _vsprintf_s(dst, 0FFh, fmt, va). |
| 004B8450 | `format_string_4092_004b8450` | cdecl(dst, fmt, ...) RET -> EAX | _vsprintf_s(dst, 0FFCh, fmt, va). |
| 004B8490 | `format_string_256_004b8490` | cdecl(dst, fmt, ...) RET -> EAX | _vsprintf_s(dst, 100h, fmt, va). |

## Existing implementations (rule 1, recorded)

| Address | Existing code | Source | Note |
|---|---|---|---|
| 0049CF80 | `apply_traffic_properties` | `src/scene_traffic_groups.cpp` | Existing semantic model of BSP_TrafficRecord_ApplyProperties (the thirteen keys in image order, templates sub-block). Recorded under rule 1; not re-verified field by field in this pass. |
| 004B6B00 | `ScriptLoadDepth::adjust` | `include/bsp/mission_lua_host.hpp` | game+644h += entering ? 1 : -1; MissionLuaHost keeps the counter as a member (depth_). Listing 004B6B00..004B6B13 checked. |
| 004B6B20 | `ScriptLoadDepth::enter` | `include/bsp/mission_lua_host.hpp` | game+644h += 1, the member counter in MissionLuaHost. |
| 004B6B30 | `ScriptLoadDepth::leave` | `include/bsp/mission_lua_host.hpp` | game+644h -= 1, unclamped, the member counter in MissionLuaHost. |
| 004B6E50 | `close_front_end_screen` | `src/frontend_states.cpp` | Exit virtual +1Ch when +5 is set, both bytes cleared, commit 004F83B0: matches close_front_end_screen (host.screen_exit / screen_commit). |
| 004BA0D0 | `pointer_slot_construct_null (inlined fill)` | `src/sound_system_owner.cpp` | MOV EAX,ECX; MOV [EAX],0: represented by owner.pointers_80.fill(nullptr) in the sound owner constructor (the EH array iterator runs it three times). |
| 004B0312 | `allocate_native_soldier_registry_node_004b02a0 catch` | `src/native_soldier_registry_tree.cpp` | free the captured node [EBP-14h] and rethrow (00BF6885(0, 0)): the catch(...) { singleton_lifetime_free(node); throw; } in allocate_native_soldier_registry_node_004b02a0. |

## STL instantiations (named, no C++)

These carry no game logic; they are named `STL_*` in the name ledger.

| Address | Name | Shape |
|---|---|---|
| 00491330 | `STL_Tree_EraseSubtree_VectorValue_00491330` | recursive _Erase of a map whose value is a vector (frees value first/last/end at node+14h..+1Ch, isnil +21h) |
| 004914E0 | `STL_BitVector_CopyRange_004914E0` | checked vector<bool> bit copy forward (iterators by value, 00BF6713 checks) |
| 004916A0 | `STL_BitVector_CopyBackward_004916A0` | checked vector<bool> bit copy backward |
| 004924C0 | `STL_DwordVector_Buy_004924C0` | vector _Buy: 3FFFFFFFh length check (00490240) and 0047E970 allocate |
| 004926A0 | `STL_Tree_InsertEqual_004926A0` | multimap insert: lower walk on node+0Ch key, 004907A0 _Insert |
| 00492940 | `STL_Tree_InsertUnique_00492940` | map insert unique: walk, 0047DAB0 decrement, 0048FDC0 _Insert |
| 00492AD0 | `STL_Vector_InsertN_00492AD0` | vector<dword>::_Insert_n (0048D5D0 copy, 0048E2A0 fill_n) |
| 00492C90 | `STL_Vector_InsertN_00492C90` | vector<dword>::_Insert_n (0048D600, 0048E2D0) |
| 00492F60 | `STL_Vector_InsertN_00492F60` | vector<dword>::_Insert_n (0048D630, 0048E470) |
| 00493120 | `STL_Vector_InsertN_00493120` | vector<dword>::_Insert_n (0048D660, 0048E4A0) |
| 004932E0 | `STL_Vector_InsertN_Pod18_004932E0` | vector<18h POD>::_Insert_n |
| 004935B0 | `STL_Vector_InsertN_004935B0` | vector<dword>::_Insert_n (0048D690, 0048E4D0) |
| 00493770 | `STL_Vector_InsertN_Record54_00493770` | vector<Record54>::_Insert_n |
| 00493A00 | `STL_Vector_InsertN_Record4C_00493A00` | vector<Record4C>::_Insert_n |
| 00494060 | `STL_Vector_InsertN_Pointer_00494060` | vector<pointer>::_Insert_n |
| 00494930 | `STL_Vector_Assign_00494930` | vector operator= (004924C0 _Buy, copy) |
| 00494BC0 | `STL_Construct_VectorCopy_00494BC0` | _Construct: placement copy of a vector (004942E0) |
| 004951A0 | `STL_Vector_Assign_004951A0` | vector operator= (00486540 copy, destroy) |
| 00495570 | `STL_Tree_InsertHint_00495570` | map insert with hint (0047D090/0047DAB0, 00492940) |
| 00495F00 | `STL_Tree_InsertHint_00495F00` | map insert with hint (00482930/0047DBF0, 00493C90) |
| 004962C0 | `STL_Tree_Copy_004962C0` | recursive tree _Copy of string->float nodes (004441E0) |
| 00496460 | `STL_Tree_InsertUnique_StringCI_00496460` | map insert unique with the case-insensitive string compare (__stricmp) |
| 00496870 | `STL_Construct_VectorCopy_00496870` | _Construct: placement copy of a vector (004942E0) |
| 004990B0 | `STL_Tree_CopyConstruct_004990B0` | map copy constructor (004858D0 head, 0048D080 copy) |
| 00499280 | `STL_BitVector_DefaultConstruct_00499280` | vector<bool>() : size and word vector zeroed |
| 004993E0 | `STL_Tree_Construct_004993E0` | map constructor (00486180 head) |
| 00499700 | `STL_BitVector_Erase_00499700` | vector<bool>::erase (0048D3B0, 00496670, 00492E50) |
| 004998A0 | `STL_Tree_InsertHint_StringCI_004998A0` | map insert with hint, case-insensitive string key |
| 00499D10 | `STL_List_Splice_00499D10` | list _Splice with size transfer (00490130) |
| 00499D60 | `STL_List_Merge_00499D60` | list merge |
| 00499E70 | `STL_List_Sort_00499E70` | list sort with a comparator (00499D10 splices) |
| 0049A060 | `STL_Construct_VectorCopy_0049A060` | _Construct: placement copy of a vector |
| 0049A100 | `STL_UninitializedCopy_Vectors_0049A100` | uninitialized_copy of vector elements |
| 0049CA80 | `STL_Tree_Construct_0049CA80` | map constructor (004858D0 head) |
| 0049CAF0 | `STL_Tree_Construct_0049CAF0` | map constructor (00486070 head) |
| 0049CDC0 | `STL_Tree_BuyNode_0049CDC0` | tree _Buynode (24h node, vector value copy-constructed) |
| 0049DEC0 | `STL_Tree_CopyConstruct_0049DEC0` | string->float map copy constructor (00443E20 head) |
| 0049DFE0 | `STL_Tree_Construct_0049DFE0` | map constructor (00486100 head) |
| 0049E450 | `STL_UninitializedCopy_0049E450` | uninitialized_copy wrapper |
| 0049F270 | `STL_Tree_Construct_0049F270` | map constructor (00485F50 head) |
| 0049F330 | `STL_Tree_Construct_0049F330` | map constructor (00486210 head) |
| 0049F360 | `STL_Vector_InsertN_VectorElements_0049F360` | vector<vector>::_Insert_n |
| 0049F5E0 | `STL_Tree_InsertUnique_0049F5E0` | map insert unique (0047DD10, 0049E1F0 _Insert) |
| 004A49B0 | `STL_Tree_InsertHint_004A49B0` | map insert with hint (0047D750/0047DD10, 0049F5E0) |

## Skipped

- 004A9BD0: cc9 lane: BSP_PlanePartClasses_LoadGlobals feeds the plane failure/death modes that cc9_plane_death_modes owns (ledger name cites that packet).

## Not reached in this pass (left for a follow-up packet)

- 00496BD0: 547 lines; calls 007B03C0/007B04C0 camera-path sampling, landscape height and normal, fmod, 00480A90.
- 00497BB0: 392 lines; x87 sine sway, camera-path sampling, 00480A90.
- 0049A760: 401 lines; x87 sin/cos, 00BD2F10 stream-0 draws, vector resizes.
- 0049B1F0: 475 lines; x87, point effects, matrices.
- 0049E560: 309 lines; x87 box and cell setup for 00491AC0's entity.
- 0049FC80: 285 lines; derived soldier type constructor from Lua (00CE691C).
- 004A0D20: 1208 lines; camera-path knots, path points, stream-0 draws.
- 004A2FC0: 256 lines; landing-troop group tick (0048xxxx troop helpers).
- 004A3610: 501 lines; constructor with vehicle/soldier class lookups.
- 004A4B70: 227 lines; path runtime base constructor (00CE68D4).
- 004A50D0: 133 lines; scene traffic runtime constructor (calls 004A4B70).
- 004A5470: 70 lines; 00CE6A08 runtime constructor (calls 004A4B70).
- 004A5850: 88 lines; group constructor with buoy class lookups.
- 004A5C60: 162 lines; registry pass with AABB overlap and kills.
- 004A5F30: 222 lines; TempAddFloatsam reader (calls 004A5850).
- 004A66B0: 46 lines; landing lane constructor (calls 004A5470), x87.
- 004A6830: 135 lines; unit landing lane setup (calls 004A3610, 004A66B0).
