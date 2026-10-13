# CC13 GPR X: second reference-model pass (segments 47, 52, 61/62, 41)

Packet `cc13-gprx`, branch `agent/cc13-gprx`. This is a second reference-model pass over gameplay
segments that cc9 approved. Same terms as the first pass:
- only the cc13 files were written;
- no host file was edited and nothing is wired;
- no game run was made.

Status words follow AGENTS.md. Every model here is **reconstructed and build-tested**. None is
ABI-compatible, fixture-tested or game-validated. Evidence was read from the installed executable's
bytes (`bsp.py disasm-raw`, the tree-local Capstone scripts `local/gprx_*.py`) and from live Ghidra
decompiles. The listing decides every comparison.

None of the models in this packet draws from `00BD2F10` on any stream.

## 1. Item 1: segment 47 control-factor functions defined in Ghidra

`007BBDC0` and `007BBF10` had no Ghidra function (docs/CC13_GPR_47.md 1.7). Both were defined under
the write lock with `tools/ghidra_define_function.py 007BBDC0 007BBF0A 007BBF10 007BBF9B` (record
`local/gprx_defined.json`). The tool takes the ledger name. The bodies are:
- `BSP_Plane_SetControlFactor`, `007BBDC0`-`007BBF0A` (330 bytes, SEH frame, two `RET 0Ch` exits);
- `BSP_Plane_GetControlFactor`, `007BBF10`-`007BBF9B` (139 bytes, `INT3` padding at `007BBF9B`).

The ledger evidence was then applied with `tools/ghidra_annotate.py --apply --addresses 007BBDC0
007BBF10`. The prior values are in `local/ghidra-annotations-20261013T095433Z.json`.

The live decompiles agree with section 1.7 of docs/CC13_GPR_47.md:
- `"roll"` (`__stricmp`), `"pitch"` and `"power"` set `+BCCh`, `+BD0h`, and `007B8140`/`007DE5E0`;
- message `C9h` goes out through `0077C7B0`;
- the getter reads `"roll"`, `"Pitch"` and `"Power"`.

The three setter call sites that docs/CC13_GPR_47.md could only read from the disk now pass
`tools/verify_report_calls.py`: `007BBE65`, `007BBE7B` and `007BBED8`.

'Non-Returning Functions - Discovered' was not touched. The define tool disassembles the byte range
explicitly and does not run flow discovery.

## 2. Item 2: segment 52 remainder

### 2.1 The caller of shipyard slot `1A8h` (`00845AC0`): found

A tree-local scan (`local/gprx_slot_scan.py 1A8`) looked for every `MOV r,[r+1A8h]` that is followed
by `CALL r`. It found seven indirect callers:
- `00440FED` in `00440F50`;
- `006D2526` in `006D2510`, the airfield tick;
- `007472A7` in `00747170`;
- four in `00B3`..`00B6` library code.

The shipyard's caller is **`00747170`, the LandFort OnDestroyed**, at `007472A7`. That function is
already modelled as `on_destroyed_00747170` (`src/gameplay_seg41_cc13.cpp:200`, `host.master_die()`).
The chain is:
1. A shipyard's hangars are LandForts. `00849F70` (BSP_Shipyard_LoadFromProperties) resolves each
   `"Hangar %d"` object by name (`00925A90`). It then calls `00744A20` (BSP_LandFort_SetMaster) on
   it at `0084A083`, passing the shipyard. The shipyard was saved at `[esp+18h]` in the prologue
   (`00849F97`), so the hangar's `+738h` is the shipyard.
2. When a hangar LandFort dies (`00747170`), with state `+70h == 1` and the clock gate, the code
   tests a non-null master `+738h`. If one is present, it calls `master->vtable[1ACh](1e10f)`
   (`[00CE4970]`). For the shipyard that slot is `0095DA00` (BSP_UnitInstance_AddDamage).
3. Then, only while the master is still live (`+5Ch` set; `+5Dh`, `+60h` and `+5Eh` clear;
   `0074728B`..`007472A1`), the code calls `master->vtable[1A8h]()`, which is `00845AC0`.

So the shipyard is not polled every tick like the airfield (`006D2526`). It is told each time a
hangar dies. `0095DA00` returns early when `+71Ch != 0` and `+728h > 0`. That means a shipyard
adopted by a CommandBuilding that still has health in `+728h` ignores the 1e10 damage. It survives
to reach `00845AC0`, which raises "InferiorFailure" once the last hangar is gone. Any other shipyard
takes the 1e10 damage (`0087D730`, then `ApplyDamage`) before `00845AC0` runs. Whether that kills
it at once was not traced past `ApplyDamage`. `00845AC0` still runs only if the live bytes are
unchanged at `0074728B`.

The other caller does not reach a shipyard. `00440F50` (BSP_DeviceEntity_OnKilledLua) calls its
owner's slot `1A8h` only when the owner answers `IsKindOf(1Bh)` and its class `Fortress` byte
`+175h` is clear. The shipyard's IsKindOf (`00846C00`) answers 0, 1, 2, 4, 5, 46h and `+C4h`.
`00848131` stores 46h into `+C4h`, so the answer to 1Bh is no. Only `00745940` (BSP_LandFort_Construct)
stores 1Bh.

This makes `shipyard_hangars_lost_00845ac0` wireable. The adoption table in section 6 has the note.

### 2.2 The receiver of message `5Eh` (`00835F60`): found

`00835F60` builds the message at `00835FE5`..`0083601B` and routes it with `0077C2A0(msg, 7, 0)`
at `0083602A` on the owner `+34h`. The fields are:
- the base `0075B430(5Eh)`;
- vtable `00D02E98`;
- the target id `+20h = 0`, `+22h = 0`;
- force `+23h = 1` (`00836017`, `AL` = 1);
- `+2Ch = 1`.

On delivery:
1. `00780120` (BSP_Session_DispatchEntityMessage) asks the message class `vtable[0Ch](59h)` at
   `00780636`. `0075B090` answers true for 5Eh, 59h, 49h and 46h.
2. It wraps the entity's weapon director, `entity->vtable[114h]()`, through `00778820`
   (`00780649`) and calls `00721A40` (`00780653`).
3. `00721A40` (BSP_WeaponDirector_ApplyGameUnitMessage) runs only while
   `[[director+34h]->vtable[13Ch]()]+5Eh` is clear. Its category cascade reaches the 5Eh arm at
   `00721BD3`, which calls `00720FE0(msg)`: id 0 resolves to null.
4. Then comes `00836240(null, force = 1)` at `00721BF1` (BSP_WeaponDirector_StoreFireTarget).

`00836240` with `force = 1` always passes its gate. It sets `+23Ch = 1`, and when `+238h` was
non-null it unregisters the observer pair and stores `+238h = 0`.

So the release is **asynchronous**. The target stays in `+238h` until the session pump delivers
5Eh, and the release leaves the forced byte `+23Ch` **set**. With `+23Ch` set and `+238h` null, a
later unforced store still passes (`+238h == 0`). The latch only blocks unforced retargets while a
target is held.

The host's fire-target setters (`src/command_execution.cpp:304`, `src/cruise_command.cpp:276`)
stand for the send side (`00835930`). No host models this receive path. The section 6 table gives
the adoption note.

### 2.3 Whether the Allied stock lists number from N+1: they do

`local/gprx_stocks.py` parses each `"JapanList"`/`"AlliedList"` bag of a support site, then applies
`0084D170`'s walk to it:
- Japan reads `Stock 1..` up to the first missing number `k`;
- Allied then reads from `Stock k`.

`0084D25D MOV EBX,1` sets the shared counter once, and the Allied walk starts with `PUSH EBX` at
`0084D37E`. A missing `"JapanList"` (`0084D255`) still leaves EBX at 1, so the Allied walk then
starts at 1.

Results for this installation (all files 2024-07-13, the untouched bulk):

| file | sites | Japan numbers | Allied numbers | Allied start | entries lost |
| --- | --- | --- | --- | --- | --- |
| `missions/ijn/ijn_11_operation_to.scn` | 3 | 1..20 | 21..40 | 21 | none |
| `missions/ijn/JM/ijn_02_force_z.scn` | 15 | 1..20 | 21..40 | 21 | none |
| `missions/COTP-IJN/ijn_02_force_z.scn` | 15 | 1..20 | 21..40 | 21 | none |
| `missions/COTP-IJN/PRCPIJN/ijn_02_force_z.scn` | 15 | 1..20 | 21..40 | 21 | none |

The authored Allied stocks continue the Japan numbering, so the shared counter loses nothing in
these scenes.

The non-zero counts the walk takes per site are:
- ijn_11: 7, 6 and 6 for both lists. Count `-1` counts as non-zero.
- ijn_02 copies: 2 Japan and 0 Allied.

`support_site_activate_0084d170` already models the shared counter.

### 2.4 `0084D560`, the supply-class list (modelled)

`support_site_build_supply_class_list_0084d560` (`src/gameplay_seg52_cc13.cpp`). The function is
`__thiscall(site, list* out, int team, bool flag)`, `RET 0Ch`, body `0084D560`-`0084D7B0`. The
decompile garbles the list copy, so this reading is from the listing.
- **Owner check.** `00923810(1)` returns the site's `+3Ch`. When that owner is present and not live,
  the result is the empty list (`0084D5B1`..`0084D5CF`, jumping to the copy-out at `0084D78C`).
- **Source list.** Team 0 copies AlliedList `+328h`. Any other team copies JapanList `+31Ch`
  (`0084D5D5`..`0084D603`).
- **Key.** The default is 61h (`0084D617`). In a session (`game+1FE4h != 0`) with `flag` set and a
  local player unit (`game+18F0h[game+18C8h]`), the key is that unit's `vtable[1Ch]()`.
- **Garrison failures.** With an adopting CommandBuilding (`+344h`), the function calls `006F4A00`
  with 46h (`0084D65B`) and with 45h (`0084D677`). `006F4A00` is **not a count**; its ledger name
  is corrected to `BSP_CommandBuilding_AnyGarrisonOfKindInferiorFailure`:
  - it walks the garrison `+778h` (100-byte rows, unit at `+14h`, count `+77Ch`);
  - for each unit with `IsKindOf(kind)` it asks `vtable[198h]("InferiorFailure", 0, &out)`;
  - it returns 1 on the first set flag.
- **Filter.** Each entry's class is `00964790(type, readRace = 1)` (`0084D6BC`).
  - A shipyard failure drops the classes that answer `vtable[18h](6)`.
  - An airfield failure drops those that answer `vtable[18h](0Fh)`.
  - A kept class whose `vtable[1Ch]()` equals the key marks the list.
- **Player entry.** A marked list gets `{type = player unit +70h, count = -1, squad = 3}`. The node is
  inserted before the head, so it is appended at the **tail** (`0084D747`..`0084D78A`). If the key
  stayed 61h and some class answered 61h, `0084D74D` would read `+70h` of a null unit.
- **Copy-out.** `0084D480` copies the result into `out`.

This closes the chain from 2.1. When a shipyard adopted by a CommandBuilding loses its last hangar,
`00845AC0` raises InferiorFailure, and that building's support sites stop offering kind-6 classes.
The kind-6 and kind-0Fh class codes are not named here.

### 2.5 `008433C0`, the shipyard stock registration (modelled)

`shipyard_register_stock_008433c0` (`src/gameplay_seg52_cc13.cpp`). The function is
`__thiscall(ScenePropertyBag*)`, bare `RET`, body `008433C0`-`00843456`.
1. It takes `"Party"` `+0Ch` with no null check (`008433D3`).
2. Then, for `k = 1..20` (`00843449 CMP EDI,14h / JLE`), it tests `"Stock k"`. A missing number does
   **not** stop this loop, unlike `0084D170`.
3. A `"Stock k"` that is a non-null bag of type 6 with a non-zero `"Type"` calls `0095BA60(type,
   party)` (BSP_VehicleClass_MarkPartyRequired) and then `[00E18680]->0046BE90(type)`
   (BSP_SceneEntity_RegisterHiddenStockUnit).

### 2.6 Read, not modelled

- `0084DDA0` (squadron BeginCommand): air ops, which the brief excludes.
- `00846640` (the `"stock"` Lua property getter): Lua glue. It builds `{classid = (rec+4)->+70h,
  count = rec+8}` per `+774h` record (stride 20h).
- `008509F0` (stationary SEntityInit): mostly presentation (`XOceanReflect`,
  `XStaticShadowFactor`, model groups).
- `00851580` (stationary class reader): Lua reader data. Its `HPBlack`/`Armor`/`Height` defaults
  need the reader's default pairs, which the decompile garbles. Not done.
- `0082DC30`, `0082DC80`, `00835700`, `00835710`: small helpers (a ref-counted vector ctor/dtor, a
  `+3Dh == 0` test, a `vtable[40h]`/`[44h]` toggle). No gameplay model was needed.
- `0082D040`: already reconstructed (`include/bsp/ship_buoyancy_elements.hpp`).
- The `0ADh` delivery timing for `0084C5B0` still needs cc9's decision (docs/CC13_GPR_52.md 1.1).

## 3. Item 3: segments 61/62 remainder

### 3.1 `00964020`, the class preload (modelled)

`class_preload_00964020`, `class_preload_row_00964020`, `kClassPreloadRows`
(`src/gameplay_seg61_62_cc13.cpp`). The function has no caller and no vtable dword, so it is an
unreferenced eager preload. It walks every pair of the global `"VehicleClass"` table, reads
`value."Type"` and picks one of 18 rows. **The compare order** is:
1. Destroyer, Cruiser, Cargo and BattleShip use `__stricmp` on the raw string, and only when the
   string is non-null (`00964142`).
2. The rest (from Submarine at `009642C5` to CommandBuilding at `009645E5`) use `00425850` on the
   native string.

| type | size | allocation | constructor |
| --- | --- | --- | --- |
| Destroyer, Cruiser, Cargo, BattleShip | 808h, 80Ch, 80Ch, 80Ch | `00BF55BE` + `00BF79F0` memset | inlined `00963380` + the two vtable stores (as `kClassDescriptorCtors`) |
| Submarine, TorpedoBoat | 840h, 814h | `00BF55BE` + memset | `00963DB0`, `00963E40` |
| MotherShip | 870h | `00470B80` | `00963EC0` |
| ReconPlane, Fighter, DiveBomber, TorpedoBomber, Kamikaze, Bomber | 60Ch each | `00470B80` | `00951AA0` .. `00951DE0` |
| AirField, Shipyard, LandVehicle | 140h, 138h, 150h | `00470B80` | `0095FEE0`, `0095FF50`, `0095FFC0` |
| LandFort | 180h | `00470B80` | `00749180`: the bare base, not a descriptor |
| CommandBuilding | 1ACh | `00470B80` | `00951E30` |

After construction, the loop:
1. sets class `+70h` to the integer key (`00964638`);
2. calls `class->vtable[8](value)`;
3. replaces the registry cache slot `00437F50()+4h[key]`. It grows the cache to key+1 (`00437BD0`) and
   releases the old entry (`InterlockedDecrement`, destroyed through `vtable[0]` at zero).

An unknown type returns `AL = 0` at once (`00964727`). LandingShip, SmallReconPlane,
LargeReconPlane and DummyTargetVehicle have no arm, so a table that holds one stops the preload.

### 3.2 `009653B0` and `00965480` (modelled)

- **`009653B0`** (`class_get_or_create_from_lua_index_009653b0`), `__fastcall(index)`: when
  `VehicleClass[index]` is a table it returns `00964790(index, readRace = 1)` (`0096544C MOV DL,1`),
  else 0. Its callers are `00A06840` and `00A072D0`.
- **`00965480`** (`class_find_by_name_00965480`), `__fastcall(NativeString* name)`. This is a
  **lookup by name**, so the ledger's "ResolveLuaClassList" was wrong. It is renamed
  `BSP_VehicleClassRegistry_FindByName` (provisional) and applied in Ghidra. It runs in two phases:
  - Phase 1 searches the registry cache `+4h[0..+8h)` for an entry whose `+54h` name matches.
  - Phase 2 iterates the `"VehicleClass"` pairs. It needs an integer key (`009655A9`), a table value
    (`009655BA`) and a string `"Name"` (`00CE8ED0`) that matches. It then returns
    `00964790(key, 1)` (`0096562A`).

  The match rule: a null name data matches a null or empty class name. A null class name matches
  only a name whose length word is 0 (`00965695`). Otherwise the rule is `__stricmp` (`0096569D`).
  The caller is `00487470`.

### 3.3 `0093AA90`, the section-failure effect (modelled)

`repair_set_section_failure_effect_0093aa90` returns the action. The function is
`__thiscall(RepairTask* = unit+A20h, section, key_b, on)`, `RET 0Ch`. It is the 9Bh arm at
`00822023`. The argument order was taken from the frame: `[esp+14h]` after the SEH pushes and EBX
is the section, and `[esp+24h]` after four register pushes is `key_b`. The ledger's "middle argument
use was not isolated" is now answered: `key_b` is the damage section's second key.
- **Index check.** An index at or past the slot vector (`+8h`..`+0Ch`) calls `00BF6713`.
- **Off.** A held effect gets `00867B10` (stop its children) and `+9h = 1`, and the slot is released
  to null (`00484620`).
- **On.** This branch applies only when the slot is empty. `008782A0(unit, section, key_b)` finds the
  section (`unit+348h` elements with `+4h == section` and `+8h == key_b`). If the section record has
  an effect name at `+24h` and the unit has a node at `+4A4h`, a point effect is created on the node
  from record `+18h` and stored in the slot.

This function is presentation only.

## 4. Item 4: segment 41 remainder (landing ship)

### 4.1 `0074BEC0` InitAll (modelled)

`landing_ship_init_all_0074bec0` (`src/gameplay_seg41_cc13.cpp`), vtable `[0A0h]` of `00CFFA30`.
1. `00822C20` (the unit SEntityInit) runs first.
2. The bytes `+118Ah`, `+1189h`, `+1188h`, `+118Bh` and `+118Ch` are set to 0 in that order, and
   `+1190h = 2.0f` (`0074BEF4`..`0074BF17`).
3. Then the ramp records. For `i = 1, 2, ...`, the note node `"rampa<i>"` (`00476B90`, then `0071AD50`
   case-sensitive) ends the loop when it is absent. Otherwise `+1198h` is resized to `i` records of
   4Ch (`0074BA20`), and record `i-1` gets:
   - the node;
   - the node's local matrix (`00B6DB60`);
   - `+44h = 0`;
   - `+48h = -1.5707964f` (`00CE3CCC`).

   These are the turn and target that `ramp_node_step_0074b44b` steps. With no `"rampa1"`, the
   vector is left untouched.
4. `+11A4h = +11ACh = 0`, `+11A8h = -1e10f` (`00CE4ADC`) and `+11B0h = 2.0f`.
5. The two ramp points are taken from the class resource `[+538h]+50h`. `+1214h` gets point 0 of
   point group `"soldierbox"` index 1 (`00718870`, `00484270`), and `+1220h` gets point 0 of index 2.
   A missing group gives (0, 0, 0).

These agree with the existing host seeds in `include/bsp/building_pads.hpp:63`. That comment places
them in "the construct at `0074C0CD..0074C108`", which is inside this InitAll.

### 4.2 `0074C280`, the save load and pad binding (modelled)

`landing_ship_load_saved_state_0074c280`, vtable `[0A4h]`.
1. `0081F980` runs first.
2. For a source record `+C0h` of kind 3 (a save), the `"_landingship"` keys are read through
   `00BD6830`. That reader slot has no presence test, so an absent key is written as false or 0.0f
   (docs/GUI_LUA_READER.md).
   - The bytes: nyitva, horgony, partraszallas, gyorsPartraszallas, partraszalltunk.
   - The floats: rampaElfordulas, lastTalaj, nyitzarTimer.
   - Only when the horgony just read is set (`0074C4B4`): horgonyPont (tag 5) and horgonyMatrix
     (tag 7).
3. **Every** kind then re-reads `+C0h` with no null check (`0074C541`):
   - **Kind 2.** The spawn record's words `+12Ch` (pad) and `+12Eh` (building). Either zero
     returns. Otherwise `0074A990(pad, building)`, resolving the building first through `00521E30`.
   - **Any other kind** (a save included), from the scene bag:
     - `"LandingCommanderPlayer"` of value type 0 sets `+120Ch`; otherwise `+120Ch = 8`.
     - `"SpawnPhase"` of type 0 sets `+1208h`.
     - With both `"LandingPoint"` and `"CommandBuilding"` present (no type test), it calls
       `0074A990(LandingPoint word, CommandBuilding word)`.

The comment at `src/game_hosts_ship_ai.cpp:13382` puts this `0074A990` call
(`0074C57C..0074C618`) in "the craft's InitAll 0074BEC0". It is in `0074C280`, slot `0A4h`. The
behaviour the host assumes is the same; only the attribution is off. That host file is cc9's, so it
was not edited.

## 5. Ledger and Ghidra

Function records (`config/reconstruction`, status `semantic_build_tested`, coverage complete):
- seg52: `0084D560`, `008433C0`;
- seg61/62: `00964020`, `009653B0`, `00965480`, `0093AA90`;
- seg41: `0074BEC0`, `0074C280`.

Name records:
- `006F4A00` renamed `BSP_CommandBuilding_AnyGarrisonOfKindInferiorFailure`;
- `00965480` renamed `BSP_VehicleClassRegistry_FindByName`;
- evidence appended to `00845AC0` (the caller, 2.1) and `00835F60` (the receiver, 2.2).

All four were applied with `ghidra_annotate.py --apply`. The prior values are in
`local/ghidra-annotations-20261013T102036Z.json`.

Ghidra: the two functions of section 1 were created. No record was added for them: their function
records already exist from cc13-gpr47.

## 6. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs | RNG (stream, order) | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `shipyard_hangars_lost_00845ac0`, now with its caller | none. The airfield's copy is `src/game_hosts_units.cpp:33954` (`kAirfieldDestructionRuleBound`, per tick). The shipyard would be driven from the hangar death, `on_destroyed_00747170` (`src/gameplay_seg41_cc13.cpp:201`, `host.master_die()`). | the hangar LandFort's master `+738h` (from `00849F70`), the master's live bytes after `vtable[1ACh](1e10)`, `+71Ch`/`+728h` (the `0095DA00` early return), and the hangar records | none | OFF: none. ON: a hangar death first hands the shipyard 1e10 damage. That is expected to kill an unadopted shipyard (not traced past `ApplyDamage`), so a game that destroys a hangar would lose the shipyard. An adopted shipyard (`+71Ch`, `+728h > 0`) ignores the damage and, at its last hangar, takes InferiorFailure. Count shipyard deaths and InferiorFailure raises per pair. |
| `director_release_fire_target_00835f60` plus the receive path (2.2) | none (no host receive path for 5Eh); the send-side setters are `src/command_execution.cpp:304` | the director's `+238h`/`+23Ch`, the session pump delivery | none | OFF: none. ON: a director whose target died or turned friendly keeps it until the next pump, then clears it with `+23Ch = 1`. Expect the gun-target hold to end one pump later than an immediate clear. |
| `support_site_build_supply_class_list_0084d560` | none (the support manager `00675C40` is cc9's and not hosted) | the site lists `+31Ch`/`+328h`, the owner's live bytes, the session player unit, `+344h` and the two `006F4A00` answers, and `00964790` per type | none | none in single player without the support manager. ON with it: classes of kind 6 or 0Fh disappear from a site whose garrison shipyard or airfield is in InferiorFailure. |
| `shipyard_register_stock_008433c0` | none (no host registers hidden stock units from `"Stock %d"`) | the bag's `"Party"`, `"Stock 1..20"` and `"Type"` | none | ON: one `0095BA60` and one `0046BE90` per authored non-zero stock, so classes load earlier. No gameplay change is expected. |
| `class_preload_00964020`, `class_get_or_create_from_lua_index_009653b0`, `class_find_by_name_00965480` | none (`src/vehicle_class.cpp` resolves classes through its own table) | the `"VehicleClass"` Lua table and the registry cache | none | `00964020` has no caller, so nothing. The other two: a by-name class lookup now also matches cache entries first. Same classes. |
| `repair_set_section_failure_effect_0093aa90` | the arm slot `include/bsp/unit_message_arms.hpp:298` (abstract) | the 9Bh message's section, key_b and on byte; the task slots; `008782A0`; unit `+4A4h` | none | presentation only |
| `landing_ship_init_all_0074bec0` | the seeds in `include/bsp/building_pads.hpp:63..82` | the `"rampa<i>"` nodes, the `"soldierbox"` point groups 1 and 2 | none | OFF: none. ON: the same seeds; the ramp records and the two soldier points now come from the model instead of host constants. |
| `landing_ship_load_saved_state_0074c280` | the pad binding at `src/game_hosts_ship_ai.cpp:13382` (labelled as InitAll) | the source record kind, the spawn record words `+12Ch`/`+12Eh` or the scene keys, the save table | none | OFF: none. ON in single player: the same `0074A990` call for a spawned craft. `+120Ch` becomes 8 when no commander is authored. A save load restores the ramp bytes and the anchor. |

## 7. Verification

- `tools/verify_report_calls.py local/gprx_calls.json`: 34 call rows checked, 0 failed. The rows
  cover every call site this doc cites, including the three control-factor calls of section 1.
- `tools/x87_dc_form_scan.py --forms dc,de` over every modelled range found 0 sites. The scanner was
  checked against a known range, `0095E9A0`-`0095EB40`, where it found the `DE E1`/`DE F9` pair. The
  models carry no x87 arithmetic: the float work is constant stores and copies.
- `./scripts/build.ps1` in this tree passed after the final `git merge origin/main`: `bsp_game.exe`
  linked and ctest passed 2 of 2. No test was added.
- No snapshot was taken. The Ghidra function count rose by the two functions of section 1, so the
  lead's next `bsp.py snapshot` will pick them up.

## 8. Open and handoff

- **cc9's decision**: whether to drive `00845AC0` from the hangar death (2.1). Note that the 1e10
  damage comes first.
- **The `0ADh` delivery timing for `0084C5B0`**: unchanged, still cc9's call.
- **The meaning of class kinds 6 and 0Fh** for `vtable[18h]`, and of the key `vtable[1Ch]` in
  `0084D560`. These are not named.
- **Still unread**: `00851580`'s default values, the archive pair `0084F080`/`0084F1A0`, and the raw
  `FUN_` entries of docs/CC13_GPR_52.md 6 beyond the four small helpers in 2.6.
- **In segments 61/62, still open**: the class registry loaders `0095B8D0`, `0095BB10`, `0095C170`,
  `0095C1C0`, `0095C640`, `0095CA50`, `0095CA70`, `0095DB40`, `0095E5B0`, `0095F1C0`, `00963F90`,
  `00963310` and `00963370`.

Tree-local helpers, not committed:
- `local/gprx_slot_scan.py`: the indirect-slot caller scan;
- `local/gprx_stocks.py`: the stock walk;
- `local/gprx_flow.py` and `local/gprx_callsite.py`: listing filters;
- `local/gprx_rd.py`: PE reads;
- `local/gprx_calls.json`: the verifier input;
- `local/gprx_ledger.py`: the record script.
