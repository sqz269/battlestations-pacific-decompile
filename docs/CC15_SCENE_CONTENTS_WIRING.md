# cc15-wC1: scene-contents ready-row wiring

Addresses: 004d4720 0095c1c0 00bb5ac0 0095bab0 00bb5670 004697b0 00469610 00926420

Packet cc15-wC1 took the `game_hosts_scene_contents.cpp` rows that
`tools/wire_readiness.py --standins` lists and wired the one coherent group whose
inputs this host actually has. Everything here is a typed restatement in
`src/game_hosts_scene_contents.cpp`; the cc13 bodies it restates take raw image
storage this process does not build. Names are hypotheses, not recovered symbols.

## Wired: the pass-2/3 class preload aliases (`kSceneClassPreloadAliasesCc15Bound`, OFF)

`004D4DF0` (`BSP_Game_LoadSceneContents`) runs, between scene-file pass 2
(Registration) and pass 3 (Instantiate):

| site | routine | restated as |
|---|---|---|
| `004D54F3` | `004D4720` `BSP_Scene_BuildClassPreloadAliases`, `__thiscall(ECX = 010904E4)` | `scene_build_class_preload_aliases_004d4720` |
| (callee) | `0095C1C0` `BSP_Scene_ClassPreloadAliasForLuaClass`, `__fastcall(ECX = out, EDX = class id)` | `scene_class_preload_alias_0095c1c0` |
| (callee) | `0095BAB0` `BSP_VehicleClass_IsEnemyOfLocalParty`, `__fastcall(ECX = class index)` | `scene_class_is_enemy_0095bab0` |
| `004D54F8` | `00BB5AC0` `BSP_Scene_ResolvePreloadAliases`, `void(void)` | `scene_resolve_preload_aliases_00bb5ac0` |

Inputs, and the host state that holds each one:

- the census `00F8A09C` (first `+4`, last `+8`) that `0095C640`/`0095C550` fill during
  pass 2: `Impl::preload_census`, filled by `VehicleClassPreloadBinding::append_census`
  on the Registration arm (`0046D51A`), whatever the switch;
- the registry singleton `00437F50` (`+10h` index map, `+4010h`/`+4014h` party rows):
  `Impl::vehicle_registry` (`type_to_class_index`, `party_required`);
- `VehicleClass[e].Type` and `.Race`: `GameMissionLuaHost::read_vehicle_class_row` /
  `read_vehicle_class_integer`;
- the local party `[[game+18CCh + 4*[game+18ECh]]+28h]`: `local_player_party()`
  (slot 0, single player);
- `00BDF4C0`: `GameVfsHost::resolve_existing`.

Outputs: the alias list `010904E4` (`Impl::class_preload_aliases`) and the plane
party-row bytes. Neither has a reader: `010904E4`/`010904E8` are referenced only by
`004D4720`, `00BB5AC0`, `004D54EE` and the two static initialisers, and no host path
reads `VehicleClassRegistry::party_required`. The switch therefore moves log rows
only. ON prints one line:
`scene class preload aliases: census=N aliases=N plane_marks=N enemy=N own=N resolved=N ...`.

USN01 OFF/ON pair (2026-10-18, same tree, `tools/pair_diff.py` exit 1, gameplay
identical, unit table identical over 86 rows): the only moves are the two native
rows (`004D4720`, `00BB5AC0` UNIMPLEMENTED -> concrete, 1 call each) and the line
`census=15 aliases=15 plane_marks=6 enemy=3 own=3 resolved=0 first=class_207.mpak
last=class_enemy_166.mpak`. `resolved=0` is expected: this installation carries no
`.mpak` file at all, and the native resolve hands the name back lower-cased.

Uncertain: `00BDF4C0`'s output for an alias with no file is the native runtime's;
a local party outside 0..2 would read the next row's byte in the image and reads as
unmarked here.

## Dropped, with the reason

- `004697B0` DestroyPendingObjects: walks the scene database's `+24h` vector. None of
  the 74 readers of `[00E18680]` was found pushing into it, and this host has no
  pending-object vector, so the input does not exist.
- `00469610` QueueEntityCommand: the strings exist, but filling database `+14Ch`
  makes `0046AAB0` issue each authored command a second time on top of milestone
  2l's direct issue (gameplay; the issue path is the units/commands lanes').
- `00926420` EntityFullPathName: its argument is the hierarchy parent node, which is
  always null here (`004DE610` builds no world node).
- `0046A9F0`, `00469B60`, `00469E40`, `009514B0`, `004CD7F0`, `008F54F0`: already
  run as typed host restatements (logged implemented); the ready rows are raw-storage
  forms with nothing more to wire.
- `0046CF40`, `0046D930`, `007B34F0`: the host parses and creates entities through its
  own typed path; the raw bodies need the image's `008F2260` bag records and scene nodes.
- `00ADD290`, `00ADE820`, `00ADAC30`, `00ADAC40`, `00AEA900`: the terrain height field
  is already live under `kSceneLandscapeTerrainBound` (cc9_landscape_terrain); the raw
  bodies need the terrain object's image layout and would move ground heights.
