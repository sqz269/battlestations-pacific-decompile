# CC13 GPR 61: segments 61 and 62 reference models (class family, repair task, failures, unit setters)

Packet `cc13-gpr61` (cc13 lineage, Claude). Segment 61 is `0092E0B0`-`00951F20` and segment 62 is
`00951F40`-`00966890`. cc9 approved this reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg61_62_cc13.cpp` and `include/bsp/gameplay_seg61_62_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

At the start, 189 functions in the two segments had a ledger name but no reconstruction record and no tag
(`local/gpr61_list.py todo` over the index). `00944FD0` (GenerateObject) was leased to `cc9-lua52` and was left
alone. `00675C40` is cc9's and is outside these segments.

Every model was read from the disk bytes with Capstone `disasm-raw`. The x87 sequences were traced with
`sc2_x87.py`, and each reg-reg arithmetic opcode was read from its bytes (see section 5). Everything in the
model file is **reconstructed and build-tested**. None of it is ABI-compatible, wired or game-validated.

## 1. Models (`src/gameplay_seg61_62_cc13.cpp`)

### 1.1 The vehicle-class family

| address(es) | model | what it does |
| --- | --- | --- |
| `00951AA0` `00951AF0` `00951B40` `00951C20` `00951D00` `00951DE0` `00951E30` `009536E0` `00953770` `0095FEE0` `0095FF50` `0095FFC0` `00960050` `00963B20` `00963BA0` `00963C20` `00963CB0` `00963D30` `00963DB0` `00963E40` `00963EC0` | `kClassDescriptorCtors`, `class_descriptor_ctor` | The 21 descriptor constructors. Each is `__thiscall(this)`, RET 0, and does four things: calls its base constructor, stores the primary vtable at `+0h`, stores the secondary vtable at `+6Ch`, and zeroes its own fields. The bases are `007D1E50` for planes, `00963380` for ships, `00749050` for vehicles and `00749180` for the command building. LandVehicle zeroes `+138h` and `+140h..+14Ch`, DummyTarget zeroes `+140h`, and Submarine zeroes `+80Ch`. |
| `00953470` `00953500` `00953590` `00953620` `00953650` `00953680` `009536B0` `00953740` `009537D0` `0095FF30` `0095FFA0` `00960030` `009600A0` `009635E0` `00963B70` `00963BF0` `00963C80` `00963D00` `00963D80` `00963E90` `00963F10` (and `00963E10`) | `kIsKindOfRows`, `is_kind_of` | Each IsKindOf is one flat CMP/JE chain over constants and calls no base class, so the row is the whole function. Planes are {own, 0Fh, 5, 4}. The two recon subclasses are {own, 14h, 0Fh, 5, 4}. Ships are {own, 6, 5, 4}. The command building is {1Ch, 1Bh, 5, 4}. Airfield, shipyard, land vehicle and dummy target are {own, 5, 4}. |
| `00951B60` `00951C40` `00951D20` | `kPlaneInstanceCtors` | The DiveBomber, TorpedoBomber and Kamikaze instance constructors. Each calls `007CFD20(this, a, 0)`, stores eight vtables, and sets class id `+C4h` (12h, 11h, 17h) and `+AACh` (8, 9, 0Ch). |
| `00953720` `009537B0` `00963E00` | `kClassNameGetters` | `MOV EAX, imm32`, which returns "MSmallReconPlane", "MLargeReconPlane" or "MSubmarine". |
| `009635D0` | `ship_class_underwater_armour_009635d0` | `FLD [ECX+6B4h]`. |
| `00963C70` | `landing_ship_class_slot_c12_clear_00963c70` | `+809h == 0`. |
| `00951F20` | `vehicle_class_derive_constants_default_00951f20` | Calls `00876180` (`MOV AL,1`), so it always returns true. |
| `00939E50` `00939F20` `00939F40` | `kControllerVariants`, `controller_b_periscope_shape_00939e50`, `controller_construct_variant_c_00939f20`, `controller_construct_variant_d_00939f40` | The unit-controller variants: `00939CB0(arg)` and then the vtable. C and D are reconstructed as bodies (cc13-fix3): the `base_construct` hook for 00939CB0, then the vptr store, returning `this`. Variant B is the submarine's (its only caller is `00852210`). It also zeroes `+394h` and `+398h`. When the model has the note node "periszkop", it then keeps at `+394h` the first shape on the body chain whose flags have bit 2. |

### 1.2 The ship repair task (`unit+A20h`)

| address | model | what it does |
| --- | --- | --- |
| `00939F90` `00939FA0` | `repair_set_fire_seconds_00939f90`, `repair_set_water_seconds_00939fa0` | Store `+38h` (fire) and `+34h` (water). |
| `00939FD0` `00939FE0` | `repair_set_hull_repair_enabled_00939fd0`, `repair_set_failure_repair_enabled_00939fe0` | Store the bytes `+45h` and `+46h`. |
| `0093A470` | `repair_add_fire_seconds_0093a470` | 1. Calls the warning fire channel `00983780` for every amount.<br>2. Sets `+38h = f(+38h + s)`, `+3Ch = f(+3Ch + s)` (a running fire total) and `+44h = 0`.<br>3. Computes `+34h*+2Ch + +38h*+30h` in extended. When that is greater than health `+370h`, it tail-calls `0090E6C0(game+21A0h, unit)`. An unordered compare returns. |
| `0093A4F0` | `repair_add_water_seconds_0093a4f0` | Calls `009832F0` only when `s > 0.0`. Sets `+34h = f(+34h + s)`, `+40h = +34h` and `+44h = 0`, then runs the same test. |

### 1.3 InferiorFailure and session messages 7Dh/7Eh

| address | model | what it does |
| --- | --- | --- |
| `00953DA0` | `unit_set_inferior_failure_00953da0` | Takes the name at `prop+4h`. A non-null name equal to "InferiorFailure" under `_stricmp` builds message 7Dh (vtable `00D1A57C`, `+4h = 1`, `+18h/+1Ah/+1Ch = 0`, `+20h = 256.0`), routes it with `0077C2A0(msg, 7, 0)` and returns true. The value argument is never read. RET 8. |
| `00953EA0` | `unit_clear_inferior_failure_00953ea0` | The same with message 7Eh (vtable `00D1A590`). |
| `00953E60` | `unit_query_inferior_failure_00953e60` | On a match it sets `*out = +720h` and returns true. Otherwise it returns false and does not write `*out`. RET 0Ch. |
| `009532E0` (and `00953360`) | `inferior_failure_message_009532e0/_00953360` | The message constructors. |
| `00953330` `009533B0` | `inferior_failure_message_is_kind_of` | {7Dh or 7Eh, 62h, 49h, 46h}. |

### 1.4 MT_ROLEOWNER device walks (arm `0095AC1D`)

The slot tables were read from the image: `00E0A510 = {1,5,6}`, `00E0A520 = {7}`, `00E0A528 = {8,9}`, each
ending at a value >= 12. The unit's twelve typed device lists are at `+394h + k*12`. The iterators are
`00952080` (first) and `009520E0` (next).

| address | model | what it does |
| --- | --- | --- |
| `00953F60` | `role_owner_walk_00953f60` | Runs only on a release (`msg+2Ch == 0`). Every device with IsKindOf(20h) and kind 1, 5 or 6 is released with `00729F70`. The exception is a kind-6 device when `unit+1BCh == msg+28h`. |
| `00954000` | `role_owner_walk_00954000` | Applies to kinds 2, 3, 4 and 6. On a take, it calls `vtable[154h](0, slot)`, and for kind 6 also `0072D5B0(0, 0)`. On a release, it calls `00729F70`. |
| `009540D0` `00954170` | `role_owner_walk_slots_009540d0` | Walks the devices on typed lists {7} and {8, 9}. On a take, it calls `vtable[154h](0, slot)`. On a release, it calls `vtable[154h](0, 8)` and then `vtable[1E8h](0)`. |

### 1.5 Unit setters

| address | model | what it does |
| --- | --- | --- |
| `009565A0` | `unit_skill_apply_level_009565a0` | ECX is `unit+38Ch`. Nothing happens when `unit+5Dh` is set. Otherwise it sets `unit+390h = level`, calls `[unit+6DCh]->vtable[18h](level)`, and calls `vtable[1C4h](level)` on each IsKindOf(20h) device. |
| `009583C0` `009584D0` `009585B0` | `unit_set_world_position_009583c0`, `unit_set_heading_009584d0`, `unit_set_world_matrix_009585b0` | The position and heading setters first decide whether to send the relocate message (`007EDF00` / `007EDEB0` + `0077C7B0`). They send it only when the unit is not a ship (6) or plane (0Fh), `world+1FE4h == 1` and `world+218Ch == 0`. The position setter calls `vtable[114h]` and stores `+A4h` and `+6A4h`. The heading setter calls `0047F200`. The matrix setter copies into `+74h` and stores `+6A4h` from the matrix's `+30h`. All three then clear `+C8h` and `+10Ch`, call `0042ED50` on each device, call `[+310h]->vtable[0Ch]` and call `vtable[D8h]`. |
| `00954990` | `unit_broadcast_controlled_audio_flag_00954990` | For a ship with a `+BBCh` object, it calls `vtable[20h](flag ? 0.0 : 1.0)`. It then calls `0072B540(flag)` on each `+428h` node and sets `+6B4h = flag`. |
| `00957450` | `unit_add_launched_child_00957450` | There are ten 18h-byte slots from `+53Ch`, each with its child at `+14h`. The first empty slot takes the child (with `00694A60` for a non-null child), and the function sets `+630h = slot` and `+62Ch = 0`. A positive `+638h` is decremented. |
| `009553D0` | `unit_set_vehicle_class_009553d0` | Swaps the `+538h` reference with an interlocked increment and decrement. When the count reaches 0, it destroys the old class through `vtable[0]`. |
| `00956240` `00956390` `009564E0` | `plane_class_create_unit` | CreateUnit for the plane classes. It calls `new(E94h)`, memsets the object to 0, runs the instance constructor and the inlined `009553D0` swap, and sets `+354h = cls`. A failed allocation is not guarded. |
| `00956200` `00956350` `009564A0` | `plane_unregister_from_world_lists` | Calls `0074E610`, then erases the unit's first node from `[unit+30h]` + `12Ch` (Kamikaze), `F0h` (DiveBomber) or `E4h` (TorpedoBomber). |
| `00953250` | `unit_store_dummy_object_id_00953250` | Sets `+6B8h = id`. |
| `00951F40` | `plane_set_visible_00951f40` | Does nothing without a `+4A4h` node. Visible: `00710AD0`, then `00B6DA70(node, 1.0, 0)`. Hidden: `00710B80`, then `00B6DA70(node, 0.0, 0)`. The ledger name `BSP_Plane_SetElevatorAttachment` does not match this body; `engine_seg56_cc13.cpp:1331` calls it `plane_set_visible`. |

### 1.5b Submarine periscope, frozen hull, class registry, current player

| address | model | what it does |
| --- | --- | --- |
| `009327F0` | `submarine_break_periscope_009327f0` | Sets `unit+122Ch = 2`, `+125Ch = 0.0`, `+1258h = clock [00F876A4]` and `+1254h = f(settings+4C4h + clock)`. |
| `009373C0` | `submarine_break_periscope_009373c0` | Does nothing unless the controller holds the periscope shape `+394h`, which `00939E50` sets. With the shape, it hides `[unit+1214h]` (`00B6DA70(0.0, 0)`), runs the `009327F0` stores and calls `00C47FC0(2)` on the shape. Callers are `008132C0` (SetFailure "Periscope") and `009377E0`. The ledger name `ClearShapeCollisionBits` describes only the last call. |
| `00937630` | `controller_apply_frozen_forces_00937630` | With `unit+1189h` clear it calls `009329C0(dt)`. With it set, it zeroes the body's linear and angular velocity (`00C37E50`, `00C37E20`) and calls `0080E170(0.0)`. It has no direct caller (a vtable slot). |
| `0095B9E0` | `class_registry_release_cached_0095b9e0` | Grows the registry cache `+4h` to index+1, releases the entry and stores null. |
| `0095BA30` | `class_registry_clear_0095ba30` | Resizes both registry vectors to 0. |
| `0095BAB0` | `class_registry_party_row_clear_0095bab0` | Returns false unless the local player index is 0..7. Otherwise it grows the 3-byte party rows (`+4010h`) to index+1 and returns `rows[index][player+28h] == 0`. The ledger name `IsEnemyOfLocalParty` is not confirmed. The model rejects a party byte outside 0..2, which the image does not test. |
| `00965810` | `unit_is_current_player_unit_00965810` | Returns false for null. A plane compares its `+9D4h` with `[00E188D8]`; any other unit compares itself. |

### 1.6 Math

| address | model | what it does |
| --- | --- | --- |
| `0093EEE0` | `round_to_step_0093eee0` | `f(step * ftol(x/step + 0.5))`. `00BF7420` is `_ftol2_sse` (`CVTTSD2SI`), so the value is truncated. The only caller is `0093F280` (the sound overlay). |
| `0093EFC0` | `matrix44_not_equal_0093efc0` | Sixteen `FUCOMIP` / `TEST AH,44h` / `JP` steps. It returns true on any unequal or unordered element. Its only caller is `0093FF10`. |
| `00952340` | `rotate_about_point_00952340` | `__fastcall(out, pivot, axis, point, angle)`, RET 0Ch. Computes the float difference, rotates it with `0085C3F0` (a callback), then adds the pivot back. Its only caller is `00954650`. |

## 2. Adoption table (cc9 terms)

None of these models draws from `00BD2F10` on any stream.

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `repair_add_fire_seconds_0093a470`, `repair_add_water_seconds_0093a4f0` | `src/game_hosts_gunnery.cpp:11150` `damage_control_add_seconds`. The host does not keep the `+3Ch` fire total, the `+40h` water copy, the `+44h` clear or the warning-channel calls. | the `DamageControl` row (water/fire seconds and ticks) and the unit health `+370h` | none | OFF: none. ON: the same pending test. The only new outputs are the warning-channel calls (`00983780` for every fire add, `009832F0` only for a positive water add) and `+3Ch`/`+40h`, which no host reader consumes yet. |
| the four repair setters | `src/game_hosts_gunnery.cpp:13089` (`set_hull_repair_enabled_00939fd0`, already a store); the others have no host setter | message arms 9Eh/9Fh (`0082203D`, `008220D7`) | none | none |
| `unit_set/clear/query_inferior_failure_*` | `src/game_hosts_lua.cpp:6836` (query through `units->inferior_failure_720`) and `src/game_hosts_units.cpp:12864` (the airfield's 7Dh arm). The clear (7Eh) has no host path. | the property name from the Lua call; `+720h` from the units host | none | ON: the host would send 7Dh/7Eh on the session pump instead of applying at the call. Single-player effect is limited to timing (the message applies on the next pump). |
| `role_owner_walk_*` | none (`src/game_hosts_script_orders.cpp:3189` covers only role availability) | the 4Bh message (`+28h` slot, `+2Ch` take), the device list with IsKindOf(20h) and the descriptor kind `+80h`, and `unit+1BCh` | none | none in single player (MT_ROLEOWNER is a multiplayer message) |
| `unit_skill_apply_level_009565a0` | `src/game_hosts_script_orders.cpp:3118` (`row_->skill_level = level`) | the level from SetSkillLevel, the dead byte `+5Dh`, `+6DCh`, devices | none | ON: each gun device would get `vtable[1C4h](level)`. Its gun-bot effect is not modelled here, so the measurable change depends on what the host binds to the device skill. |
| pose setters `009583C0` / `009584D0` / `009585B0` | `src/game_hosts_units.cpp:34512` (`place_at_world_position_008193a0` writes the position only) | the position, heading or matrix from the script call; the IsKindOf 6 and 0Fh answers; `world+1FE4h` and `+218Ch` (MP mode) | none | ON: the previous position `+6A4h` follows the teleport, so a host velocity derived from `+6A4h` stops seeing a jump. Device poses are invalidated (`0042ED50`). In SP the relocate message is never sent. |
| `unit_broadcast_controlled_audio_flag_00954990` | `src/game_hosts_hud.cpp:1661` (`set_unit_controlled_audio`, record only) | the controlled-unit flag from `00645600`; the `+BBCh` object and `+428h` gun list | none | presentation only (the audio mix) |
| `unit_add_launched_child_00957450` | none (the caller is `00821E80`'s message arm; no host arm found) | the child handle from the message; the slots `+53Ch`; `+638h` | none | none until the arm is bound |
| class family tables, `is_kind_of` | `src/local_player_unit_lists.cpp:12` (ids only) and `src/vehicle_class.cpp` (descriptor rows) | none | none | none (the tables add the base constructor, `+6Ch` vtable and zeroed fields) |
| `plane_class_create_unit`, `plane_unregister_from_world_lists`, `unit_set_vehicle_class_009553d0` | `src/unit_world_registration.cpp:55` (rows only) | the class pointer; the owner's lists | none | none |
| `submarine_break_periscope_009373c0`, `_009327f0` | `src/submarine_model.cpp:285` (clock fields only); the SetFailure "Periscope" arm in `src/game_hosts_lua.cpp:6771` | the controller's periscope shape `+394h` (from `00939E50`), clock `[00F876A4]`, settings `+4C4h` PeriscopeRepairTime | none | ON: the mast node hides and the shape collision call runs; the state and timers are the same as the existing model's |
| `controller_apply_frozen_forces_00937630` | none (no direct caller; a controller vtable slot) | `unit+1189h` | none | none until the slot is bound |
| `class_registry_*` (`0095B9E0`, `0095BA30`, `0095BAB0`) | none | the registry singleton `00437F50`; the local player index `world+18ECh` and party byte `player+28h` | none | none until a caller is bound |
| `unit_is_current_player_unit_00965810` | the host callbacks `is_player_controlled_00965810` (`src/options_hardware_cc13.cpp:236`, `src/warning_reports_cc13.cpp:162`) and `src/game_hosts_mission_frame.cpp:1750`, which does not model the plane `+9D4h` arm | the controlled unit `[00E188D8]`; IsKindOf(0Fh); the plane's `+9D4h` | none | ON: a plane whose `+9D4h` is the controlled unit counts as the player's |
| `round_to_step_0093eee0`, `matrix44_not_equal_0093efc0` | none (sound-overlay debug draw) | | none | none |
| `rotate_about_point_00952340` | `src/game_hosts_units.cpp:20381`, inside `df_predict_00954650`, which rotates in double. The model keeps the image's float stores of the difference and the result. | pivot, axis, point and angle from `00954650` | none | ON: float rounding of the predicted point moves by at most an ulp-scale amount per prediction |

## 3. Pointer records (host or earlier reconstructions checked against the listing)

| address | source | check |
| --- | --- | --- |
| `0095E9A0` | `src/ship_ai_approach_update.cpp:1366` `ship_ai_torpedo_hits_to_sink_0095e9a0` | Every branch matched the listing. `+5Dh` and a null projectile return 0. `0 >= remaining` returns 0. The two max picks (FCOMIP then JBE, so a tie keeps the second). `00419010(lo, 1, hi, 0, armour)`, then the `> 0` test returning `[00D7A248]`. The excess uses `DE E1` (`ST1 = ST0 - ST1`, so `(lo+hi)/2 - armour`), and the final division uses `DE F9` (`remaining / (per_hit*frac)`). |
| `00951FC0` | `src/game_hosts_gunnery.cpp:7917` (inline in the gun tick) | Matched. A non-negative value becomes `-v - 35.0`, otherwise `-0.0 - v`. When the result is above 80.0 it is reset to 0 (`TEST AH,44h` / `JNP`). NaN behaves the same in both. |
| `009327F0` | `src/submarine_model.cpp:285` `submarine_break_periscope_009373c0` (the doc names `009327F0` its twin) | This is a fragment. The model covers `+125Ch = 0`, `+1258h = clock` and `+1254h = f(settings+4C4h + clock)`. The `+122Ch = 2` store is not part of that struct. The full body is modelled in section 1.5b, which gives a second record. |
| `00951220` | `include/bsp/scene_traffic_groups.hpp:78` `TrafficRecord` with `TrafficRecordDefaults` | This is a fragment. The defaults match the image: `new(4Ch)`, `0049F9B0`, `+4h/+8h = -1`, `+0Ch` and `+10h = 0`, `+14h = 1`, `+18h = 40.0`, `+1Ch = 10.0`, `+24h = 5.0`, `+28h..+30h = 0`, then the `00950FC0` push. |
| `009512F0` | `src/scene_traffic_groups.cpp:231` `read_traffic_item` | Matched. The expect order is item, name, `{`, path, `=`, name. The lookup runs only on a non-empty name. Then come the optional `properties`, `0049CF80` and `}`. The creation order differs and is labelled in the source. |
| `009514B0` | `src/scene_traffic_groups.cpp:286` `read_traffic_block` | Matched. `00951160` clears. The loop has the brace counter with `item` before `{`/`}`. The item path skips the consume. Then comes `[game+21D0h]` `004A5620`. |

## 4. Named, not reconstructed here (left for later; no records added)

- **Lua and spawn manager** (avoided: Lua mission natives): `00941D30` `009420A0` `00943C00` `00944210`
  `00944680` `00944FD0` (leased by cc9-lua52) `00945850` `00945A20` `00945C30` `00946380` `00946390` `009478B0`
  `009481A0` `009483D0` `00948CC0` `00949300` `00949530` `00949750` `0094A140` `0094C480` `0094C490`, and
  `0093BD80` (the AddEngineJamFailure Lua argument reader). Several have reconstructions in `src/lua_spawn_new.cpp`
  and `src/game_hosts_lua.cpp`, but they were not checked here.
- **Gunnery hosts** (avoided): `00955630` (a full reconstruction exists at `src/game_hosts_gunnery.cpp:5740`,
  `docs/GUN_GRAVITY_ARC.md`; it needs a record from its owner), `00955830`, `00955EB0`, `00956C20`, `00957740`,
  `0095F500`, `00954A10`, `009542B0`.
- **Plane and air ops** (avoided): `00954650` (host approximation at `src/game_hosts_units.cpp:20348`), `00953920`,
  `00953A60`, `00956EE0`.
- **Presentation and front end**: the map overlay (`0093E510`, `0093EAE0`, `0093EC80`, `0093EDC0`, `0093EEB0`),
  the sound overlay (`0093F280`, `0093F4B0`, `0093FE90`, `0093FF10`), the 3D scenes (`0093CB10`, `0093CC20`,
  `0093D250`, `0093D2D0`), the flag (`009574D0`, `0095DE00`), `0093AA90` (section failure effect slots) and
  `00959230` (display-name key).
- **Unit controller and physics**: `0092E5B0` (a host exists at `src/game_hosts_units.cpp:9381`; it was not
  checked), `0092F2E0`, `00935540`.
- **Class registry and loading**: `0095B8D0` `0095BB10` `0095C170` `0095C1C0`
  `0095C640` (a source exists at `src/scene_traffic_groups.cpp:342`; it was not checked) `0095CA50` `0095CA70`
  `0095DB40` `0095E5B0` `0095F1C0` `00963F90` `00964020` `009653B0` `00965480` `00963310` `00963370`.
- **Scene text writer and files**: `00950000`-`00950350`, `00951820`, `00951940`, `0093AC30`,
  `0093AFF0`, `00946450`, `00957080`, `00946FC0`, `009555A0`.

## 5. Verification

- `./scripts/build.ps1` in this worktree passes: the build succeeds and 2/2 tests pass. It was rerun after the
  final `git merge origin/main`.
- `tools/x87_dc_form_scan.py --range 0092E0B0-00966890 --forms dc,de` finds no DC-form site in any modelled or
  checked body. The DE sites there were read from their bytes:
  - `0093EEEA` `DE FA` (FDIVP ST2: x / step);
  - `0095EAD8` `DE E1` (FSUBRP ST1: `ST0 - ST1`);
  - `0095EB1F` `DE F9` (FDIVP ST1);
  - `0093A498`/`0093A4AA`/`0093A4C3`/`0093A545` FADDP and `0093A4C1`/`0093A543` FMULP ST3. These are commutative.

  `sc2_x87.py 0093EEE0 0093EF0C` gives `f(S8 * ftol(S4 / S8 + 0.5))`.
- `tools/verify_report_calls.py local/gpr61_calls.json` checked 42 call rows (every direct call and tail jump
  named in sections 1 and 3) and none failed. `local/gpr61_calls2.json` checked 11 more rows (section 1.5b) and none failed.

## 6. Handoff

Open items, in suggested order:
1. A pointer record for `00955630` (host gravity arc) after a listing check. This is cc9's, so ask first.
2. The `0095C640` preload against `src/scene_traffic_groups.cpp:342` (249 listing lines).
3. `0093AA90` section-failure effect slots and `0092E5B0` network correction, if a presentation or MP pass wants them.
