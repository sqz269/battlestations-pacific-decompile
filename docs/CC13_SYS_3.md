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

- Registry: 0098D470 / 0098D4E0 and the four wrappers call the registry directly. 0098D5C0 stores the
  low dword of its eight-byte buffer even when the read failed; the host
  `GameHardwareProbe::read_stored_qword` (src/game_hosts_init_tail.cpp) does not. Recorded, not
  changed.
- `query_processor_info_00992180`: a zero `ProcessorNameString` size returns false without closing the
  key. An `AMD Athlon(tm) 64` name scales the MHz by 1.5 with chop rounding (x87 at 009922E0).
- `read_language_from_options_00992830`: the next token after `Language` selects the LCID by its first
  letter; 409h otherwise.
- The XLive stats records, the online-system create/destroy pair, and the content-package array
  template (18h records) are complete.

## Records

| group | reconstructed and recorded |
|---|---|
| segment 35 message manager | 36 |
| segment 35 pools and fish/flag helpers | 35 |
| segment 66 | 42 |

## Handoff

(Written at the end of the pass.)
