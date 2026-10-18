# CC13 sys pass 3: segments 35, 66 and 11 (cc13-sys3)

Worker cc13-sys3 on packet cc13-sys3, 2026-10-12, branch `agent/cc13-sys3`. This is a reconstruction pass
over three engine and content segments:

- segment 35, `00701B70`-`0070BBA0`: messages, whoSaysThat, data tables, fish schools and flags;
- segment 66, `0098C630`-`00996060`: hardware detection, the BSM_HWD registry key, language, the online
  system base and the downloadable-content manager;
- segment 11, `004E7DD0`-`004F9D30`: scene-database entities, 2D geometry helpers and the front-end
  screen sets.

The targets were the functions with a ledger name but no reconstruction record and no tag in
`local/bsp_index.sqlite` (263 at the start). Nothing was run in the game, and nothing is wired into a
host.

Status: **reconstructed, build-tested** only. That means `./scripts/build.ps1` passed and ctest passed
2 of 2. Nothing here is ABI-compatible, fixture-tested or game-validated. No test was added.

New code: `src/engine_seg11_35_66_cc13.cpp`, `include/bsp/engine_seg11_35_66_cc13.hpp`
(namespace `bsp::engine_seg11_35_66_cc13`), registered in `cmake/startup.cmake` on `bsp_core`.

## Conventions

- Native layouts are kept where the routine is a layout operation: `NativeString` with an explicit
  `NativeStringStorage` (0041DD40 resize, the 00419CC0 / 00BD1510 release pair), `{data, count,
  capacity}` arrays, MSVC 2005 tree nodes. Message records use the existing semantic types
  `VoiceClipRecord`, `VoiceTimedKey` and `VoiceClip` (`include/bsp/voice_playback.hpp`,
  `include/bsp/scheduled_voice_types.hpp`), so node offsets past the tree links are a projection.
- No image global is invented. Every global word a routine reads or writes is passed in (the
  allocator-list head 00E188B4, the static pools 00E19ACC / 00E19B14, the DirectSound counter and
  name 00F8A0E0 / 00F8A0E8, the online-system words 00F8A2FC / 00F8A300, the download manager
  00F8A304, the GPU table 00E0CFF4).
- Calls to routines owned elsewhere go through host interfaces whose virtuals name the image address
  (`MessageManagerCalls`, `PoolMemory`, `HardwareCollectCalls`, `XContentApi`, ...).
- Transient strings the image builds and releases inside one routine are `std::string`.
- The property visitor (abstract vtable 00CE374C; the Lua reader 00CE44FC) is `PropertyVisitor` with
  the slots +4h enter, +8h leave, +0Ch read-or-default, +10h read, +14h has-key, taking the
  eight-byte `GuiLuaVariant` pairs of `include/bsp/gui_lua_reader.hpp`. The archive writers use slot
  +0Ch with two pairs (`PropertyWriter`).
- Win32 calls the image makes directly (registry, QueryPerformanceCounter, RDTSC, fopen) are made
  directly.

## Segment 35: the message manager

The manager is the 70h-byte object at game +21DCh (vtable 00CFD3A4, built by 00707480 at 004DFA1A).
Layout (`MessageManagerLayout`): record map +4h (MSVC tree, node 58h, nil byte +55h, keys compared
case-insensitively), a NativeString vector +14h..+1Ch, three 13h category filters +20h/+33h/+46h and
their group flags +59h/+5Ch/+5Fh, and a string set of loaded maps +64h (node 18h, nil +15h).

Message records are read from Lua tables. The string anchors are `text` (00CF1010), `subtitles`
(00CFD348), `voice` (00CFD340), `tutorials` (00CFD334), `actor` (00CFD32C), `LANGUAGE` (00CFD320),
`.fsb` (00CFD318), `sound/messages/streamed_dialogs/` (00CFD2F4), `Actors` (00CFD368), `Messages`
(00CFD35C), `whoSaysThat` (00CFD2E8), `msg` (00CE4AD8), `Scripts/datatables/messagemaps/` + `.lua`,
`Scripts/datatables/Messages.lua`, `DL_Content_0000062`, `englishauthentic` and the separator `#`
(00CF828C).

`read_message_record_00704590` was taken from the listing (the decompile drops the visitor arguments):

1. `text` into record +0h, default "".
2. When `subtitles` exists, for i = 1, 2, ... while `[i]` exists: resize the timed keys to i
   (00704190, zeroed strings), take back() (checked: an empty vector reaches 00BF6713), enter `[i]`,
   read `[1]` string, `[2]` float, `[3]` float, `[4]` string with default "", `[5]` bool with default
   false, leave.
3. `voice` into +0Ch, default "". An empty voice stores -1 in +8h and returns.
4. A voice containing `tutorials` becomes `voice[0, idx+9) + "/" + language + voice[idx+9, end)`.
5. `actor` (default ""), +8h = 0, +14h = 1.
6. For each of the four slots: release the slot's reference; when `flags[slot]` is set and the
   template at 00E08FCC is not null, replace every `LANGUAGE` in the template (004CAD40) with the
   language (008D57A0), probe `dir + actor + "/" + voice + ".fsb"`, then `dir + voice + ".fsb"`,
   through the file store [0109CEEC] slot +8h; the first that exists loads through 00A83FD0 (ECX
   [00F8BBE8]) into the slot and clears +14h. The templates are
   `sound/messages/LANGUAGE/streamed_dialogs/`, `sound/messages/LANGUAGE/english/`,
   `sound/messages/LANGUAGE/japanese/` and `sound/messages/other/` (00CFD274, 00CFD250, 00CFD22C,
   00CFD214).
7. When nothing loaded and +18h is empty, `sound/messages/streamed_dialogs/` + voice + `.fsb` is
   probed through slot +0Ch and stored in +18h.

`load_messages_007065e0` sets flags[1], [2] or [3] from the local player's side kind (0, 1, other),
so US messages take the `english` slot, Japanese messages the `japanese` slot and anything else the
`other` slot. A message map loads with flags {1, 0, 0, 1} (00CFD2A4): the streamed and `other` slots.

`load_message_map_00706760` keys loaded maps as `name#side`. In profile-hints mode 2 it walks every
other language (`<language>_<key>` FileBlock, load, unload) before the real load. The real load
mounts a FileBlock named `englishauthentic_<key>` when the profile owns `DL_Content_0000062`, else
`<language>_<key>`, inserts the key, loads, then deletes the block.

`write_loaded_message_maps_00703020` (ledger `BSP_MessageMap_CountEntries`) writes the loaded keys
as `{1, i} -> text`; `load_message_map_list_007072c0` reads them back, splits at the last `#` and
reloads. The ledger name of 00703020 undersells it; it was kept.

## Segment 35: fish-school and flag model pools

Both pools are one template (`PoolGeometry`): fish 320h-byte blocks of two 18Ch slots, flag 3444h-byte
blocks of 20h 1A0h slots. `src/band_0067_cc13.cpp` already modelled the allocate/free bookkeeping
without the native layout; the new code is the native-layout form, and covers construction,
destruction, the empty-block release (vtable slot 0) and the dword-vector helpers. The fish-school
parameter defaults (`init_fish_school_params_00708b00`) were read from the image constants: 9C4h,
25, 25, 60, 50, 10, 50, 0, -20, 0, 2, 2, 1, 1, 4, 7, 2, 1, 2, 0.3, 0.75.

## Segment 66

- Registry: 0098D470 / 0098D4E0 and the four wrappers call the registry directly. All four wrappers
  return the inner call's AL (cc13-fix23 corrected 0098D5C0 / 0098D5F0, previously void; 0073C3B0
  sums the four read answers). 0098D570 types its four bytes REG_QWORD (0098D57C MOV EDX,0Bh), not
  REG_DWORD (corrected by cc13-fix23). 0098D5C0 stores the
  low dword of its eight-byte buffer even when the read failed; the host
  `GameHardwareProbe::read_stored_qword` (src/game_hosts_init_tail.cpp) does not. Recorded, not
  changed.
- `query_processor_info_00992180`: a zero `ProcessorNameString` size returns false without closing the
  key. An `AMD Athlon(tm) 64` name scales the MHz by 1.5 with chop rounding (x87 at 009922E0).
- `read_language_from_options_00992830`: the next token after `Language` selects the LCID by its first
  letter; 409h otherwise.
- The XLive stats records, the online-system create/destroy pair, and the content-package array
  template (18h records) are complete.
- Download manager (vtable 00D1D7D8), from the listing: slot 6 `enumerate_new_content_00994b10`
  appends the display names of installed packages that match no active record to the global
  string vector at 00F88A24 (004CDC20) and returns whether it appended any; slot 9
  `refresh_content_licenses_00994f40` rebuilds the active records from the enumeration, reusing the
  previous record of the same name; slot 10 (ledger `BSP_DownloadManager_UnmountPackage`) only
  collects the paths of the mounted packages into a list; the destructor logs and unmounts
  (`Unmount active content (dir: %s)(mpkg: %s)`, `Remove active content package (%s)`).
- Stats: 00993220 and 00994310 read the eight player slots (game +18CCh..) and the per-side score
  blocks ([game+21A0h]+70h + slot*284h) through a read-only `StatsGameView`.

## Segment 35: fish schools (presentation)

`respawn_fish_007077b0`, `init_fish_school_00709310` and `create_fish_school_00709620` were taken from
the listing. All four 00BD2F10 draws are presentation draws on the shared stream 0 in the image; the
reconstruction routes them through `PresentationRandom` so a host can give them their own stream. The
x87 arithmetic runs on the main thread at 24-bit precision (docs/CC13_FPU_PRECISION.md), so float
arithmetic is the closer model; the angle uses the double pi at 00CE3D28 and the placement is
`x = rx sin t U(-0.5, 0.5)`, `y = ry sin t U(-0.5, 0.5)`, `z = (cos t - 1) rz / 2` with
`t = i / count * pi`.

## Segment 11

- Presentation entities: NavPoint, MovieCamPos and MovieCamLookat share one creator body
  (`create_named_scene_entity_004e99b0` with a `FixedEntitySpec`); CameraPath copies its 40h frame to a
  local first; the SimpleEffect (5Bh) and PeriodicEffect (5Ch) creators defer the hierarchy write to
  004F03C0 while the frame-job pool says so. Every creator ends with the same name tail: resize
  entity+154h to strlen(name) without preserve, then copy exactly that many bytes.
- Front end: the update trampoline 004F71F0, the four empty base virtuals of vtable 00CEAE54,
  004F8390 (ECX is the LEVEL: the target vector is 00E18CE8 + level*10h; the ledger comment called the
  argument unused), the tactical-library helpers and the small Lua / string helpers.

## Backfilled records (existing reconstructions, checked)

| address | existing code |
|---|---|
| 00994100, 00994110 | `src/network_online_cc13.cpp` (listing: CMP [ECX+60h],0/1; SETE AL) |
| 00709AC0, 00709B60 | `src/band_0067_cc13.cpp` (listing checked: FSUB / FMUL-then-FADD with float temporaries) |
| 0098DF90 | `src/band_0090_cc13.cpp` `recommend_detail_level` |
| 004F7740 | `src/simulation_gate.cpp` |
| 004F85D0, 004F8670, 004F87B0 | `src/frontend_screen_sets.cpp` `set_front_end_screen_set_level` (levels 2, 3, 5) |
| 004F8830 | `src/frontend_states.cpp` `run_front_end_screen_pump` |
| 004F05F0, 004F06C0, 004F0790, 004F0860, 004F0930, 004F0A00, 004F0FB0, 004F10B0 | `src/scene_unit_creators.cpp` `create_scene_unit_004f0520` with the class row |
| 004E98E0 | `register_scene_unit_004e96d0` (the thunk is one JMP) |
| 004E98F0, 004E9900, 004E9910 | same, **partial**: the 006BCF40 / 008433C0 tail calls are not modelled |
| 004F2800 | `src/scene_entity_factory.cpp` class table, **partial** (data projection, no map inserts) |
| 004F1D70 | `src/scene_record_side_blocks.cpp`, **partial** (the MultiPlay slot table only) |
| 004EB5A0 | `src/scene_weather_cc13.cpp` `read_particle_set` |
| 004F4840 (004F4430, 004F3810 partial) | `src/plane_follow_law.cpp` `circle_tangent_points_004f4840` (fused chain) |
| 004F49F0 | `src/game_hosts_ship_ai.cpp` `segment_distance_2d_004f49f0` |
| 004E9620, 004F2560 | `src/entity_class_ids.cpp` `entity_is_kind_of_inherited` (the model also requires the dynamic id to be registered) |

## For cc9 (gameplay entities in segment 11, not reconstructed here)

These create, register or tear down entities that the simulation acts on, so they stay with cc9:
LandingPoint (004E9520, 004E9670, 004E9690, 004E9D40), SpawnPoint (004F1610, 004F1910, 004F1940,
004F19A0, 004F1A60), LandConvoy (004F2410, 004F25D0, 004F2600, 004F2660, 004F2700), Stationary
(004F0BE0), Wreck (004F0CC0), WaterMine (004E9F80), the stock lists (004EA080), the scene header copy
(004E8BF0, reached from cc9's 004F1D70), Path (004EA650) and Landscape (004F11C0, 004F1390, 004F1400,
004F1420, 004F1460; cc9 packet `cc9_scene_path_landscape` binds the Path and Landscape creators).
The entity creators share the shape documented above (`store_entity_name`, vtable +98h placement), so a
cc9 reconstruction can reuse `create_named_scene_entity_004e99b0` with a spec.

## Not reconstructed: library code

The 27 `std_*` functions in segment 66 (`00990740`-`009926A0`: basic_filebuf, ctype<char>, ios_base,
use_facet, runtime_error, ...) are MSVC 2005 standard library instantiations. Their library names are
kept and no record is written, as in docs/CC13_FILES_1.md.


## Segment 35: flag cloth (presentation)

From the listing: the wave step 0070AA70 clamps dt to 0.5, runs `1 - ftol(dt / -0.025)` midpoint steps,
and draws the wind's x component from 00BD2F10 stream 0 (routed through `PresentationRandom`); the wind
is `normalize(U(-0.01, 0.01), sin a, -cos a) * 100`. Forces (00709EF0) are the wind projected on each
point normal, gravity +58h, velocity drag +54h and structural springs; the integration (0070A450)
weights the force by the column, `(i % columns) / (columns - 1)`, so column 0 stays pinned. The vertex
write (0070A710) emits a front and a back face with normals scaled by -0.5 and +0.5. The render
(0070AC00) skips lod > 0.5 and fades the alpha linearly from lod 0.4 to 0.5.

## Segment 66: Direct3D survey

`query_display_driver_00991ef0` takes a host survey of adapter 0 (driver version, description, device
id, X8R8G8B8 modes, multisample support, pixel shader version). Shader model is 2 for PS 2.0 and up,
else 1, and forced to 1 for device ids 301h, 302h, 311h-314h, 320h-323h, 326h, 327h, 341h-344h (the
GeForce FX family), from the sixteen-entry stack table at 009920CC.

## Records

Reconstruction ledger, packet `cc13-sys3`: **204 records**: 173 point at
`src/engine_seg11_35_66_cc13.cpp`, 31 are backfills of existing code (7 of them `reconstructed_partial`).
Three ledger names got appended evidence and their Ghidra comments were refreshed through
`tools/ghidra_annotate.py --apply` (00703020, 009954E0, 004F8390); no name was changed.

Call sites: `reports/cc13_sys3_calls.json` lists 63 host call rows; `tools/verify_report_calls.py`
checked all 63 against the live listing, 0 failed.

Index at the end: 59 of the 263 targets have no record: the 27 library functions, the 25 cc9 entities
above, the four cc9-side geometry helpers below and three open items.

## Open items

- `00707B00` BSP_FishSchoolModel_UpdateCameraFade (BC0h bytes): the per-frame fish behaviour (camera
  distance fade, steering toward the respawned target, per-fish update through cross products). Not
  reconstructed; it needs a full listing pass for the x87 chains.
- `0098C9C0` BSP_HardwareInfo_DescribeOsVersion (9A0h bytes): the MSDN "DisplaySystemVersion" sample
  (GetVersionExA with OSVERSIONINFOEX, ProductOptions\ProductType, the Q246009 hotfix probe), strcat into
  info +234h and the >= 5 major flag at +C34h. Mechanical but long; not reconstructed.
- `00995690` BSP_DownloadManager_ScanContentFolder (vtable 00D1D7D8 slot 8): verifies each installed
  package (XLiveContentVerifyInstalledPackage into record +12h), mounts `<path>\content`, scans for a
  `.mpkg` and logs "Damaged content %s". Not reconstructed (register-spilled decompile, needs the listing).
- Geometry helpers used by cc9's ship AI (`004F3090` rectangle separation, `004F3360` partition
  parameters, `004F3DA0` triangle containment, `004F4880` segment clip): x87 stack-juggling bodies; left
  for whoever owns the polygon-partition caller. `004F3030` and `004F40C0` are done.

Packet cc13-left3 (`docs/CC13_LEFT_3.md`, section 3) has since reconstructed `00707B00`, `0098C9C0`
and `00995690`. The geometry helpers above remain with their owner.

## Handoff

Done and committed on `agent/cc13-sys3`. Nothing is wired into a host. To continue: the open items
above, in that order of value (the fish update is the only one with behaviour a viewer sees). The
`PresentationRandom` callback is the hook for the RNG rule: a host that wires the fish or flag code
must give it its own stream, not 00BD2F10 stream 0.
