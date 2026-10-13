# CC13 save, profile and settings reconstruction pass (segments 2 and 29)

Addresses: 0042b860, 0042b9e0, 0042ba00, 0042d680, 0042d770, 0042def0, 0042e960, 00437bd0, 00439000,
006ad470, 006af420, 006af780, 006af870, 006afb00, 006afe80, 006afec0, 006b5960

Packet cc13-save1, lineage cc13. Ranges: segment 2 (`0042A920..0043E840`) and segment 29
(`00696470..006BA620`; the air-operations tail `006BA620..006BEB10` is cc9's). Every name is a
descriptive hypothesis, not a recovered symbol. This continues `docs/CC13_SETTINGS_SAVEDATA.md` and
`docs/CC13_PROFILE_SAVE.md` (cc13-settings1), which reconstructed most of the save code but wrote no
reconstruction records.

## Scope

The index listed 206 functions in the two ranges with a reviewed ledger name and neither a
reconstruction record nor a tag. They split as follows.

| Group | Count | Action |
| --- | --- | --- |
| cc13-settings1 reconstructions without records (pickler, checkpoints, text dumpers, binding text, preset writer) | 48 | records backfilled after a RET-size check against the export listing |
| New reconstructions in `src/save_settings_cc13.cpp` | 16 | reconstructed, build-tested, recorded |
| Named `STL_*` template instantiations of the input-settings maps (segment 29) | 78 | tagged `stl_shape` (ledger only; library code) |
| Camera movers, HUD gauges, cc9-lane entity slots, `LuaMachine` helpers, air operations, plus `006AF780` (documented only) | 64 | left to their owners (below) |

The packet's string anchors (`BSP_Chk_Save`, `__unlockTo`, `__unlockName`, `__unlockFrom`,
`__playTime`) all resolve to functions already reconstructed in `src/profile_checkpoint.cpp` and
`src/profile_manager.cpp`; their missing records are among the backfill. `00435F40` (writes the
`BSP_Chk_Save` global table to an archive) has no caller and is not in the list (no ledger name of
the reviewed kind); it was not touched.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| Entity-interface slots `0042B860`, `0042B9E0`, `0042BA00`, `0042D680`, `0042D770`, `0042E960` | yes | `src/save_settings_cc13.cpp` | yes | no |
| Camera mover matrix slot `0042DEF0`, key destructor `00439000`, cache resize `00437BD0` | yes | `src/save_settings_cc13.cpp` | yes | no |
| Entity name map `006AFE80` / `006AF870` / `006AFEC0` / `006AF420` | yes | `MissionEntityNameMap` | yes | no |
| `006AFB00` restore entities, `006AD470` registry read, `006B5960` load tail | yes | `src/save_settings_cc13.cpp` (hooks for native reads) | yes | no |
| `006AF780` single-entity restore | yes | documented only | n/a | no |
| Backfilled records (48) | yes | earlier cc13-settings1 files | yes | as recorded in the earlier docs |

Nothing is ABI compatible or game-validated. The mission save/load entries `006B7710`, `006B5960`,
`006AFB00` and the load body `006AF8B0` have no static reference in the image
(`docs/CC13_SETTINGS_SAVEDATA.md`, "Reachability"); everything below them is static analysis only.

## Segment 2: shared entity-interface slots

cc13-camera2 named these default slots (`docs/CC13_SEGMENT2_CAMERA_MATH.md`, "Shared
entity-interface slots"; each is shared by 40-95 vtables) but did not reconstruct them. The host takes
the fields as values because the entity layout is owned by other lanes.

- `0042B860`, no arguments: returns `"Entity"` (`00CE3C7C`).
- `0042B9E0`, `ECX` entity: `[00E0CD80 + 4 * [ECX+C4h]]`, no range check.
- `0042BA00`, `__thiscall(entity, Vec3f* out)`, `RET 4`: `00414DB0` when byte `+C8h` is clear, then
  copies `+FCh..+104h`.
- `0042D680`, `__thiscall(entity, Vec3f* out, float t)`, `RET 8`. Ghidra's prototype is wrong (it
  shows the out pointer as `unaff_retaddr`). The listing calls vtable `+34h` (velocity) first, then
  multiplies and spills each product with `FSTP dword` (`0042D6A3..0042D6B7`), then refreshes the pose
  and adds `+FCh..+104h`. The products and sums are single float operations, so the result is the
  same under 24-bit or 53-bit x87 precision.
- `0042D770`, `out[16]`: identity; `1.0f` is the dword at `00D7A24C` (`00 00 80 3F`).
- `0042E960`, `ECX` entity: `+158h` when `+15Ch` is zero, else `+160h`; null becomes `""` (`00E1767D`).
- `0042DEF0` (camera mover vtable `+120h`): copies `+2F8h` when `+1BCh` and `+3ACh` are both
  nonzero, otherwise the identity.
- `00439000`: `_free(data)` only when the owns byte at `+4` is set.
- `00437BD0` (`docs/VEHICLE_CLASS_DESCRIPTORS.md`): grow through `00436530` and null-fill; shrink pops
  from the end, releasing each non-null descriptor (`InterlockedDecrement(desc+4)`, vtable slot 0 at
  zero) and nulling its slot. A negative size would walk below slot 0 in native code; the host throws.

## Segment 29: the entity name map

`006AFE80` (called from `BSP_Game_OnInit`) builds an object with vtable `00CF82BC` and a
case-insensitive `map<NativeString,int>` at `+4`. `006AF870` and `006AFEC0` erase the tree and free
the head; the host class owns a `std::map` with a `_stricmp` comparator.

`006AF420` (`ECX` this, `RET`, no static caller; reached from the unnamed load body `006AF8B0`):

1. Erases the tree (`006AF457 CALL 0058B520`).
2. Iterates `globals.thisTable` with `IterateFirst(table, key, value)` (`006AF4DE`; key at
   `[ESP+44h]`, value at `[ESP+30h]` at the base frame).
3. Skips a member whose `value.Dead` (`00CF829C`) is true by `lua_toboolean` (`00B66250`), and one
   whose `value.Name` (`00CE8ED0`) is not a Lua string: `00B660A0` tests `lua_type == LUA_TSTRING`
   strictly, so a numeric name is skipped.
4. For a new name, `map[name] = atol(GetString(key))` (`006AF697`, `006AF6A2` with `ECX = [ESP+44h]`
   (the key), `006AF6A8 CALL _atol`); for a repeated name, `map[name] = -1` (`006AF6B9`).

The lookups go through `00B67800`, which has `lua_gettable` semantics, so a non-table member raises
the same Lua error the host's `lua_getfield` raises. Uncertain: whether `LuaObject` iteration of a
missing `thisTable` raises; the host leaves the map empty.

## Segment 29: restoring entities (`006AFB00`, `006AF780`)

`006AFB00` is a method of the name map: `ECX` is saved at `[ESP+34h]` (`006AFB24`) and is the map
cleared at the end (`006AFD9B..006AFDC1`). The earlier doc left its argument flow open; the listing
gives:

1. The keys of `_savedata._entities` (`006AFB5A`, `006AFB73`) are collected with `GetInteger`
   (`00B66290`: `lua_tonumber`, narrowed through `FSTP float`, then `_ftol2`) on the key
   (`006AFC04`, `ECX = [ESP+3Ch]`, the key slot of `IterateFirst` at `006AFBE4`).
2. `00623170` sorts them ascending (`006AFCAF`).
3. A second vector receives one element per key, and every element is the constant 0: `006AFD25`
   stores `0` directly, and the `004322D0` insert at `006AFD4F` passes `[ESP+34h]`, which `006AFCDF`
   set to `EBP = 0`. No lookup from key to entity exists in the routine.
4. `006AF780` is called with each element (`006AFD89`), so every call receives a null entity.
5. The name map is cleared.

`006AF780` ignores `ECX` and uses its stack argument as the entity (`ESI`, `006AF799`): it calls
`009238A0` with `ECX = entity` (the entity's saved table under `_savedata._entities`), reads
`parentID` as type 4 through the reader's vtable `+10h`, reads `localFrame` as type 7 through
`00BD6830`, and calls the entity's vtable `+98h` with `(parentID, [game+19CCh], &localFrame)`
(`006AF821..006AF842`). With the null argument from `006AFB00`, the first access faults. The
native load path would therefore crash on any save that holds an `_entities` entry. This agrees with
the observation that nothing references the load entries: the code appears unfinished and is not
used. The host model passes `nullptr` to the callback, as the native code does, and returns the
sorted keys.

## Segment 29: load tail `006B5960` and registries `006AD470`

`006AD470` (`ECX = 00F89A08`, reader at `[ESP+4]`, `RET 4`): section `cont0` (`00CF827C`), `00951820`
into `ECX`, end; section `cont1` (`00CF8274`), `00951820` into `ECX+54h`, end. The reader's vtable
`+4h` begins a section with `(0, name)` and `+8h` ends it.

`006B5960` was re-read from the listing. The order is: sections `entIDcont` (`00CF830C`, `006AD470`),
`_game` (`00CF8304`, `004B4A20`) and `scoring` (`00916A00`); commit record `+8h` stored at
`[ESI+5Ch]` (`006B5A82`); `00874640(0.0f)` (`006B5A7C FLDZ`); `actUnitID` (`00CF82F8`) read as type 4
and passed to `00647300` on `[[00E198C4]+40h]` when non-null; `006B3DD0`, `006B23E0`; registry clear
(`006B17B0`, `005FE9E0`); `_savedata = nil` through `00B67350` (`lua_settable` semantics); and when
`[[game+1A08h]+4] != 0`, `006B8AD0("collectgarbage(\"collect\")", 0, 0, 2)` (`00CE4DE0`), which
forwards to `006B89F0` with the text as its own chunk name. Unlike the save path, no storage
continuation runs at the end.

The host `finish_mission_load_006b5960` runs the Lua steps through the existing pickler
(`register_global_functions_006b3dd0`, `decode_mission_lua_state_006b23e0`) and takes the native reads
as hooks. Scoring (`00916A00`) and the spectated unit are cc9-side state; the host only calls the
hooks.

## Records backfilled

Pickler, `src/mission_savedata_lua.cpp` (33): `006ADDC0`, `006AE400`, `006AE120`, `006AE730`,
`006AE280`, `006AE870`, `006AEA00`, `006AF120`, `006AEBF0`, `006AEDC0`, `006AEF70`, `006B2D30`,
`006B2E90`, `006B2FC0`, `006AF2B0`, `006B0020`, `006B0BE0`, `006B0650`, `006B1870`, `006B1E40`,
`006B3FD0`, `006B3130`, `006B4CB0`, `006B5BF0`, `006B1190`, `006B1490`, `006B3DD0`, `006B68C0`,
`006B23E0`, `006B7710` (Lua tail only), `006B17B0`, `006AFF20`, `006AFF90` (the last three as the
`MissionLuaFunctionNames` alias). Routines folded into a shared host walker carry the scope
`folded_into:<host function>`.

Other cc13-settings1 files (15): `006AD8D0`, `0043C1C0`, `0043C530` (`src/lua_text_dump.cpp`);
`00435D30`, `00436730`, `004379C0`, `00437C70` (Lua part), `00437250`, `00438020`, `0043CC20`
(`src/profile_checkpoint.cpp`); `0069E300`, `0069E9E0`, `006A33F0`, `006A9850`
(`src/input_binding_text.cpp`); `006A5640` (`src/control_preset_archive.cpp`).

Check made for each: the export listing's RET immediate matches the ABI comment in the header
(for example `006B3FD0 RET 20h`, `006AEDC0 RET 14h`, `0043C530 RET 8`, `006A9850 RET 10h`). The
behaviour was not re-derived; the earlier docs carry that evidence.

## Tags

78 `STL_*` functions in `0069B000..006A7690` were tagged `stl_shape` (evidence id `cs1_stl_seg29`).
They are the `std::_Tree`, `std::vector` and `std::pair` bodies of the input-settings maps, named by
`docs/INPUT_SETTINGS_TREE_CLUSTER.md`. Host code uses the standard containers. `006A1290`,
`006A1C40`, `006A2930` are covered as behaviour by `src/native_input_settings_tree_insertion.cpp`, and
`006A2310`, `006A3200`, `006A6D20` by `src/native_input_settings_vector_storage.cpp`.

## Left to owners

- For cc9: `006BC530`, `006BC6F0`, `006BCA80`, `006BD080` (air operations); `006AF3D0` (scene
  network-id stub, packet `cc9_scene_path_landscape`); entity slots implemented in cc9 files:
  `0042BAF0` (`game_hosts_gunnery.cpp`), `0042BB40` (`game_hosts_ship_ai.cpp`), `0042BB20`,
  `0042BE90`, `0042E950` (`game_hosts_units.cpp` and the approach headers), `004313D0`
  (`game_hosts_script_orders.cpp`), `00432480`; the mission camera bodies of `docs/MISSION_CAMERA.md`
  (`0042D5A0`, `0042DCD0`, `0042F0C0`, `0042F650`, `00432E60`, `00432ED0`). The scoring read
  `00916A00` and the spectated-unit setter called by `006B5960` are gameplay state; the host only
  calls hooks. No mission scoring that changes gameplay state lives in these ranges.
- cc13 camera lanes: `0042A920`, `0042C040`, `0042C4C0`, `0042C560`, `0042C610`, `0042CBE0`,
  `0042CC00`, `0042CEA0`, `0042DA30`, `0042EB90`, `004329D0`, `00435410` (named and cited in
  `include/bsp/segment2_camera_cc13.hpp`; none of the five copy slots surveyed has a host body yet).
- HUD: `004396F0`, `00439820`, `0043A290`, `0043A660`, `0043ABA0`, `0043B370`, `0042BA40`
  (implemented or cited in the HUD hosts).
- `TickElement` stubs `0042BB70..0042BBB0` (`src/tick_element_overrides.cpp`) and `0042E740`
  (`game_tuning_singleton.hpp`): implemented elsewhere; records belong to those owners.
- `LuaMachine` helpers `006B7E80..006B89F0` and `006B88E0`: cited by cc12's mission Lua documents and
  `src/game_hosts_lua.cpp`; skipped as in the earlier pass. `0043D8F0` (property-bag merge) is cited by
  cc9's `game_hosts_lua.cpp` packet `cc9_plane_scene_equipment`.

## Divergences and uncertainty

- `restore_entities_006afb00` reproduces the null-entity defect instead of resolving entities; a
  caller that wants working restores must not use it as is.
- `finish_mission_load_006b5960`: the `LuaReader` built over `_savedata` (`004425C0`) and its
  destructor (`00441210`) are folded into the hooks; the section names are cited, not passed.
- `MissionEntityNameMap::rebuild_006af420` copies a number key before converting it to text, which
  matches `LuaObject` pushing its own slot; the host does not reproduce `LuaObject` iteration of a
  non-table.
- The entity and camera slot functions take field values, not an entity pointer; they are not
  drop-in replacements.

## Evidence method

Live `bsp.py ghidra decompile` and `ghidra disasm` for every new routine; `disasm-raw` for
`00B66290`, `00B660A0`, `00B66250`, `00A677E0`, `006B8AD0`, `0042D680`; `ghidra bytes` for
`00D7A24C`, `00CF82F8`, `00CF8250`, `00CE4DE0`. Call-site rows are in `reports/cc13_save1_calls.json`
(`tools/verify_report_calls.py`: 25 rows, 0 failed). Ghidra: evidence appended to `006AFB00` and
`006AF780` through `tools/ghidra_annotate.py --apply` (prior state in the local annotation log).

## Handoff

Done: 16 new reconstructions, 48 backfilled records, 78 tags, two Ghidra evidence updates.

Open:

1. `006AF8B0` (the unnamed load body ahead of `006AF420`) still has no Ghidra function; reading it
   would complete the load side.
2. The five camera copy slots (`0042C040`, `0042C560`, `0042CBE0`, `0042CC00`, `0042DA30`) are
   trivial and unreconstructed; they belong with `src/segment2_camera_cc13.cpp`.
3. `006A9A40` and `006AAC50` (binding rebinding and conflict text) remain surveyed only
   (`docs/CC13_PROFILE_SAVE.md`, open item 1).
4. Whether anything can reach `006B5960` at run time; the null-entity finding suggests not.
