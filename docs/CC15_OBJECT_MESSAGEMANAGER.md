# cc15-ob1: the MessageManager [game+21DCh]

Lane L1 of the cc15 single-player object bring-up (packet `cc15_host_message_manager`).
Switch: `kMessageManagerCc15Bound` in `src/game_hosts_mission_frame.cpp`.
Host object: `GameMessageManagerHost` (`include/bsp/game_hosts_message_manager.hpp`,
`src/game_hosts_message_manager.cpp`). Names are hypotheses, not recovered symbols.

## Evidence addresses and lifetime

| What | Address | Evidence |
| --- | --- | --- |
| allocation | 004DFA1A `PUSH 70h`, operator new | ConstructWorld 004DE610 |
| constructor | 00707480 at 004DFA39, store to game+21DCh at 004DFA4B | typed body `construct_message_manager_00707480` |
| destructor | 00707520 (vtable slot 0, body 007071F0) with 1 at 004D2D04 | DestroyWorld 004D2BB0, after +21CCh and before +21E0h |
| Messages.lua | 007065E0 at 004E124E (1), 004E12C9 (0), 004E17E9 (1) | 004DFB70, inside per-language FileBlocks |
| record resolve | 00705E00 from 00976B00 | typed bodies in `message_record_resolver.cpp` |
| key test | 00702300 + inlined find from 0097B5E0 (called by 0097D650) | `cc13_large62.cpp` raw forms |

The image rebuilds the manager on every mission load. The host builds it in the
`construct_world` load step and, on a second load, runs 007071F0 on the previous one
first. Because the host runs no DestroyWorld, the last manager is destroyed when the
mission frame host is destroyed.

### 007065E0's argument (correction)

The header described the argument as a side. The assembly shows that it is a load flag:
- 007066C4 loads `[ESP+2Ch]`, the argument, and pushes it as 00706160's first stack
  argument (`load`).
- 007066C8 pushes `-1` as the side.
- 004DFB70 passes 1 (004E124E), 0 (004E12C9, the "dummy DO NOT USE" unload of the
  language sweep) and 1 (004E17E9, the main load).

`load_messages_007065e0` now takes `bool load` and passes side -1. The filter inputs
were already right: `[game+5FCh]+1064h` is the enable byte and +1068h, +1074h and
+1080h are the id arrays. 00468B70 fills them from the scene tree's `unittypes` block
(`allied`, `japanese`, `neutral`, `neutralnationalities`).

## What the switch does

- **`construct_world`.** Runs 00707480 on a fresh 70h `MessageManagerLayout`. The
  tree heads are 58h and 18h zeroed blocks. The record map +4h is
  `bsp::MessageRecordStore`; the `name#side` set +64h is a case-insensitive
  `std::set`.
- **`load_warning_bank`.** This step was a record. It now runs the arm of 004DFB70
  that every load reaches (004E1374..004E17FB): 007065E0(1) over
  `Scripts/datatables/Messages.lua` through the typed 00706160 / 00704590 bodies. The
  script is read through the VFS (00BDF4C0 / 00BDF310), run by the repository's Lua
  5.1.1, and walked with `GuiLuaReader` (00BD8E20 / 00BD7A20 / 00BD6830 / 00BD68D0 /
  00BD5EB0 / 00BD5F50). This is the same stand-in the host uses for Warnings.lua and
  decals.lua.
- **00976B00's 00705E00.** It resolves from the record map, counted as
  `clip_records_resolved` and `clip_record_misses` in the
  `summary mission message manager` line. The `records_not_evaluated` count then stays
  at 0.
- **0097D650 / 0097B5E0.** These now run for kill and hit warnings whose text carries
  an escape separator (`resolve_entity_sound_keys_cc15`), counted as
  `sound_keys_resolved` and `sound_keys_unmatched`. The step builds the key prefix
  from the separator and the 00E0B080 relation word, then forms candidates from the
  unit category name (00E0B590) and the default-by-kind name. A candidate that the
  escape map (manager+11Ch, `warning_tables.entity`) holds rebuilds the text
  (0096EDD0). The first rebuilt text present in the record map wins.
- **Apply side.** The clip still waits on the voice manager (005B71D0 / 00974070), so
  gameplay does not change.

### Unit category (+170h interface, slot 0)

0097D650 reads the category from `(entity+170h)->vtable[0]()`, which returns one
constant per class. The table below comes from the constructors' stores to +170h. For
example, `006DFCB1 mov [esi+170h], 00CF9088` gives slot 0 = 006DFDC0, `mov eax, 3`.

| class id | class | slot 0 | category |
| --- | --- | --- | --- |
| 07 | MDestroyer (00CFC3AC) | 006FE4D0 | 1 destroyer |
| 08 | MSubmarine | 00852FB0 | 0Dh submarine |
| 09 | MMothership (00D01608) | 007581F0 `xor eax,eax` | 0 mothership |
| 0A | MCruiser | 006FB370 | 4 cruiser |
| 0B | MCargo | 006EB1D0 | 5 cargo |
| 0C | MLandingShip | 0074BBC0 | 6 landingship |
| 0D | MBattleship | 006DFDC0 | 3 battleship |
| 0E | MTorpedoBoat | 00857D60 | 2 torpedoboat |
| 10 | MPlaneBomber | 007D7790 | 7 levelbomber |
| 11 | MPlaneTorpedoBomber | 00951CB0 | 9 torpedobomber |
| 12 | MPlaneDiveBomber | 00951BD0 | 8 divebomber |
| 13 | MPlaneFighter | 007DDA20 | 0Ah fighter |
| 14 | MReconPlane | 0074E2B0 | 0Bh reconplane |
| 19 / 1A | MLandVehicle / LandConvoy | 0074DD40 / 004F24E0 | 0Eh landvehicle |
| 1B / 1C | MLandFort / MCommandBuilding | 00745A40 / 006F5830 | 0Fh landfort |
| 45 | MAirfield | 006D1D20 | 10h airfield |
| 46 | MShipyard | 00846B40 | 11h shipyard |

Other classes answer `false`, which keeps the count `sound_keys_not_evaluated`:
- the base plane reads unit+AACh (007D0350, `[ecx+93Ch]`);
- the squadron reads unit+354h (007EFA90, `[ecx+1E4h]`);
- the kamikaze class (00951D90 returns 0Ch) has no class id bound here.

## Seams (not hosted, each counted or logged)

| Seam | Image | Host |
| --- | --- | --- |
| sound banks | 00A83FD0 on [00F8BBE8], FMOD sample loading | `sound_banks_not_loaded`; the record's four references stay null |
| streamed-dialog probe | [0109CEEC] vtable +0Ch, 00BDD520 | `streamed_probes_not_evaluated`, answers false |
| voice language | 008D57A0 on [00F88980]: the catalog row's voice_dir, `authentic` with DL_Content_0000062, `english` fallback | the language string the mission frame host was constructed with, logged in the load line |
| record miss | 00705E00 creates a `<name>` record timed by [[00E198C4]+A4h]+7Ch/+80h | counted as `clip_record_misses`, answers null |
| spoken-language sweep | 004E0FAD: when [004C1E90()]+8h == 2, each other language loads (1) in its block, then unloads (0) in "dummy DO NOT USE" | not taken; the host has no profile-hints singleton |
| FileBlocks | "Warnings_<language>_<Ally/Jp>" / "Warnings_englishauthentic_..." (00BE0A30) | not built |
| message filters | [game+5FCh]+1064h..+1088h from the scene's `unittypes` (00468B70) | disabled with empty arrays; the host's scene read does not parse `unittypes` |
| script overrides | 00B69D40 also runs the 00BDEF90 override files | base file only |
| vtable[10h] / vtable[28h] names in 0097D650 | unit names | not bound; the installed entity map has no name keys |

## For other owners

- cc9: `dialog_message_timing` (game_hosts_script_orders.cpp around 7009) parses the
  message maps a second time. The manager now offers `load_message_map_00706760` and
  `unload_message_map_00706fc0` on `host.messages`; whether script_orders should call
  them is cc9's decision. This lane did not edit that file.
- Seams that point at the manager but have no host binding yet:
  - `warning_string_seg64_argon`'s `message_record_resolve_named_00705e00`;
  - `MessageFromHost::resolve_message_00705e00` (0096E5E0);
  - Large62Host's raw `manager`.

  Each can use `resolve_record_00705e00` / `record_map_contains_00702300`.

## Pairs

The runs are 2026-10-19, 4800 mission frames at 0.05 s. The baseline is origin/main ee7ced46d (merged into this
branch). ON is 1b1426acd with `kMessageManagerCc15Bound=true`.

| Pair | pair_diff | Gameplay |
| --- | --- | --- |
| USN04 baseline vs this tree OFF | exit 0, identical apart from noise | identical |
| USN04 OFF vs ON | exit 1 | identical: deaths, 62 plane death modes, 90-row unit table |
| USN01 OFF vs ON | exit 1 | identical: 36 deaths, 2251 hits, 30 plane death modes, 104-row unit table |

Rows that move from OFF to ON, all in both missions:
- Native rows `MessageManager::construct 00707480`, `load_messages 007065e0` and
  `destruct 007071f0` are added, each with calls=1.
- `MissionLoad::load_warning_bank` goes from UNIMPLEMENTED to concrete, so the load
  line reads 17 to 18 concrete and 26 to 25 records.
- A `message manager loaded Messages.lua` line is added. It shows:
  - records=247. The file has 250 entries, and three keys appear twice
    (`nmy_ship_ident_dispatcher3`, `nmy_ship_ident_dispatcher4`,
    `sub_periscope_lost`). The Lua table keeps one of each.
  - voice_probe_hits=0 and misses=494, which is two templates per record.
    `language=englishauthentic` is the lockit name the host was built with. 008D57A0
    would answer the row's voice_dir `authentic`, whose folders exist on disk. This is
    the voice-language seam: the probes only set the records' streamed and alternate
    flags, which nothing reads while the sound banks are a seam.
- The `summary mission warning activate` line changes:
  - USN04: `records_not_evaluated` goes from 9 to 0.
  - USN01: `records_not_evaluated` goes from 23 to 0, and `sound_keys_not_evaluated`
    from 2 to 0.
- A new `summary mission message manager` line appears:
  - USN04: clip_records_resolved=9, misses=0, sound keys 0/0. No kill or hit warning
    activated there.
  - USN01: clip_records_resolved=21, clip_record_misses=2, sound_keys_resolved=0,
    sound_keys_unmatched=2, key_tests=2, key_hits=0.

USN01's two unmatched keys and two misses are the same two activations. Both are
`@_ship_dest_<n>_randomplane` for a cargo ship, class 0Bh:
- The keys tried are `@ENEMYcargo` and then `@ENEMYdefault_ship`.
- Warnings.lua maps `cargo` to `nmy_ship`, and `default_ship` is commented out.
- The rebuilt text is `nmy_ship_ship_dest_<n>_randomplane`.
- Messages.lua has no such record. Its enemy ship-dest records are bb, ca, dd, cv and
  submarine only.

The image takes the same path. Its 00705E00 then creates the `<name>` record, which is
the record-miss seam. The miss is a property of the installed data, not of the host.

Fix from the first ON pair: the native row read `construct calls=2` for one
construction. The step also called `done()` while `construct_00707480` logs its own
row. 1b1426acd drops the extra call.

The switch stays OFF in this commit. Turning it ON is the lead's call; the pairs
above are the evidence.

## Flipped ON (cc15-flip, base 0bc136293, 2026-10-19)

OFF = 0bc136293, ON = the same tree with the switch true. Both run through tools/run_game.ps1 in the
reference env, `BSP_DEATH_TABLE=1 BSP_GUNNERY_RNG_STREAMS=1`, and every log has `rng streams: on`.
The runs are USN04 (`--frames 5000 --mission-frames 4800`) and the 12 wins rows (`--stop-after-end
40`).

- **Gameplay.** `pair_diff` reports gameplay identical on all 13 rows. The end, the clock, the death
  rows, the unit table and the guard line match. Ends: USN01 completed 1079.93 s, USN02 failed
  330.04 s, USN12 failed 859.91 s, PRCP01 completed 814.92 s, USN21 completed 2941.74 s, ESMP08
  completed 1274.27 s, LOMP10 completed 1303.70 s, JM06 completed 898.90 s, BSM09 EndMission
  282.05 s, BSM01 EndMission 812.93 s, LOMP06 failed 668.11 s, IJN18 no end.
- **Moved rows, every row.** These are the Pairs rows above: the three MessageManager native rows,
  `load_warning_bank` going concrete, the mission load line (18 -> 19 concrete, 25 -> 24 records), the
  `loaded Messages.lua` line (records=247, voice_probe_misses=494) and the new `summary mission
  message manager` line. In the `summary mission warning activate` line only
  `records_not_evaluated` (USN04 5 -> 0, USN01 117 -> 0) and `sound_keys_not_evaluated` move to 0.
  `activations`, `draws` and `clips` do not change.
- **Record-miss and unmatched-key lines (new, diagnostic).** Each names a record that the installed
  Messages.lua does not have. Natively, 00705E00 creates such a record (the record-miss seam):
  - The rebuilt texts `nmy_dd_` / `nmy_ca_` / `nmy_bb_` / `nmy_ship_ship_dest_*`, as in the USN01
    case above.
  - `under_att_5_randomship` and `under_att_dispatcher3`: the file has only `_1`..`_4` and
    `dispatcher`/`dispatcher2`.
  - `powerup_gain_dispatcher`: absent from the file.
- **(a) Dialog, voice, warning and Lua timing.** I ran an ordered diff of every line matching voice,
  warning, clip, message, dialog, tutorial, hint, subtitle, speech, popup, objective, Lua or script,
  with hex masked and times kept. It moves only the lines above, plus three noise lines: the
  renderer thread id, the masked `ShipAiSectorScan::clip_arc_zones` counter, and `summary mission
  load finished` with its concrete count. No `dialog message`, `dialog callback`, subtitle or
  objective line moves or changes its time.
- **(b) cc9's `dialog_message_timing`** (game_hosts_script_orders.cpp:7082) reads
  `scripts/datatables/messagemaps/<map>.lua` and `sound/messages/authentic/streamed_dialogs/*.fsb`
  itself. It does not touch the manager. Its `dialog message <name> voice=... clip=... key_end=...`
  lines are identical in content and order OFF and ON on every row, from 4 lines (LOMP06, USN12) to 35
  (PRCP01).
- **(c) Voice-language seam.** The `englishauthentic` probes only set a record's streamed and
  alternate flags. Their readers are `construct_voice_slot_00702cc0` and `start_voice_clip_005b9050`
  (voice_slot_start.cpp). No game host calls either one, and no ON log has a 005B9050, 00702CC0,
  005B71D0 or 00974070 row.
- **Save untouched.** Every file under `Documents\Battlestations-Pacific` has the same SHA-256, size
  and mtime before and after the runs.
- **Other lines.** The ring-scan `clear_37c` lines (SHIP_AI_OPEN_ITEMS 140), as in the earlier
  flips.
