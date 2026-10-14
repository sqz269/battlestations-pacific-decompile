# Segment 1: automation step, MPakScenes walker, terrain grid I/O (cc13-seg1)

Segment 1 is 00413D10-0042A7E0. cc13-camera2 already covered the automation driver's setup,
watchdog and log writers (docs/CC13_SEG1_AUTOTEST.md). This packet adds the driver's per-screen
step and the MPakScenes walker, the TerrainGrid stream writer and reader, two session bit-stream
payload helpers and a few leaf routines. Code: `include/bsp/scenes_seg1_cc13.hpp`,
`src/scenes_seg1_cc13.cpp`. Status: **reconstructed and Win32 build-tested only**. No fixture, nothing
run in game, not ABI-compatible. Names are hypotheses (the Ghidra names already in the ledger).

## Triage

`local/seg1_triage.py` over the index: 80 game functions in the range carry no function record (42
of them FUN_). They fall into these groups:

- **Avoid-zone navigation, belongs to cc9** (ship-AI lane): 00416270 BSP_AvoidZoneOutline_WalkToCircle,
  004171F0, 004174A0, 0041A030, 0041A1E0, 0041A4E0, 0041AC60, 0041B120, 0041B290, 0041B4B0,
  0041B4E0, 0041B510, 0041B580, 0041B680 (BSP_SpawnRequest_MemberPlacementLegal's helper), 0041B700,
  0041B7E0, 0041BAE0, 0041BC20, 0041BD10, 0041C0F0, 0041C490, 0041D0C0, 0041D150, 0041DE70,
  0041DF40, 0041E000, 0041EA30, 0041F980 (the AvoidZoneManager destructor), 00417E60, 00417FA0,
  00421500, 00421CE0, 00423520, 004235A0, 00423640, 00423960, 00423AF0, 00424680, 00424730.
  The avoid-zone debug drawing (004166E0, 00416840, 004169A0, 004178B0, 00417990; callers
  BSP_ShipAiDebug_Draw and two vtable-only wrappers) draws those zones and is left with them.
- **Blast-site notification, belongs to cc9** (gameplay): 00428420 (from
  BSP_Projectile_TraceSegmentAndImpact), 00428800 BSP_Explosion_NotifyBlastSites (already called
  through `call_00428800` in src/blast_damage.cpp) and their geometry helper 00428090
  BSP_Geometry_SegmentIntersectsSphere (sole caller 00428420).
- **Template and library instantiations, not reconstructed**: 0041E490 / 0041F370 / 00420660
  (a ref-counted pointer array's reserve, resize and teardown, siblings of the TMPL_PointerArray
  routines), 0041E7A0 / 0041F820 / 0041CB50 / 0041E780 / 004204A0 (pooled-string storage
  release/allocate instantiations), 0041F0D0 (list/tree teardown), 00427AC0
  (std::vector<8-byte>::size: `[v+4]` null -> 0, else `([v+8]-[v+4]) >> 3`).
- **Already reconstructed elsewhere** (names only in the list above): 00427C90, 00427E30, 00427EB0
  have host methods in several headers; 00424C40 BSP_GameSettings_GetSingleton is a host call
  everywhere.
- **This packet**: the rows below.

## Reconstructed

| address | C++ | notes |
|---|---|---|
| `00427190` | `autotest_step_00427190` | the per-screen step, all 17 cases of the jump table at 004277E4 |
| `00426650` | `autotest_next_mpak_scene_00426650` | the MPakScenes walker |
| `00425EB0` | `autotest_select_listbox_item_00425eb0` | select or activate a list box row |
| `00425500` | `autotest_focus_lobby_00425500` | 005D67E0(lobby, [lobby+8Ch]), 005D5870(lobby, 1) |
| `0041EF90` | `terrain_grid_layer_serialize_0041ef90` | one TerrainGridLayer to the stream |
| `0041F130` | `terrain_grid_save_layers_0041f130` | "TerrainGrid", the layer count, every layer |
| `004239E0` | `terrain_grid_layer_deserialize_004239e0` | one TerrainGridLayer from the stream |
| `004246E0` | `terrain_grid_layer_construct_from_stream_004246e0` | vtable 00CE38CC, empty grid, then 004239E0 |
| `004180A0` | `terrain_grid_cell_to_world_004180a0` | out = in * cell size (+0Ch) - half extent (+04h); provisional owner type, no callers |
| `00429140` | `write_native_word_array_00429140` | each word zero-extended, 00428F50(cursor, &temp, bits) |
| `00429B00` | `write_voice_data_payload_00429b00` | 64 header bits, count byte, count frames of 80 bits |
| `0042A480` | `read_voice_data_payload_0042a480` | the matching reader |
| `00418280` / `00425A80` | `singleton_base_destruct_00418280` / `_00425a80` | clear 00F8753C / 00E17664, vtable 00CE3818 |
| `00425BD0` | `native_string_to_long_00425bd0` | atol of the data, "" (00E17668) when null |
| `00429FD0` | `native_legacy_sbo_string_assign_cstring_00429fd0` | strlen, then 00408720 |

### 00427190, the step

`__thiscall(driver, int screen) RET 4`. Mode +8h == 0 returns 0 at once. On screen 0Ch with a
movie playing (`[[00E188A8]+38h]` set and its +60h == 1) +10h counts frames and 0045F600 stops
the movie once it passes 2; the call returns 0 without touching +Ch. Every other call clears +10h,
counts +30h on screen 0Ch in mode 3, and screen 0Eh returns 1 at that point. Then +Ch is
decremented; at 0 it is reloaded with 50 (before the range check) and the screen's case runs:

- 0: 004374F0 on the profile manager (00425C20), then 0068D8A0 (title skip).
- 1: runs `SCRIPTS/datatables/MPakScenes.lua` (00B69D40 on `[00E188A8]+1A0Ch`) once while byte
  00E08070 is set, then clears it. Modes 2 and 3 run 00426650; when byte 00E17659 is set and the
  campaign +18h has reached 6, 00425570 restarts the pass, 00426650 runs again and `[00E1765C]` is
  incremented. With +18h < 6, not loading (0057D430) asks for screen 0, else the table at
  00427828 maps campaign 0..5 to screens 1, 0, 5, 4, 2, 3 (00426350). Other modes, and +18h >= 6,
  take the top-level path: 00584AE0(0) when loading and the campaign is not multi, then screen 6.
- 2: 00532C50(1) when the menu command screen's +1D0h is 0, else 00532C50(0).
- 3: 00426650, then 00426350(+1Ch); +1Ch++ when that returns 1.
- 4: 00594BF0. 5: multi -> 00599340; otherwise 005885D0 and +Ch = 200. 6: 004FF6F0. 7: 005922F0.
- 8: 00426650; outside multi or with +1Ch >= 100, 005ECA90 (back). Otherwise +20h = 0 and, for
  +28h 0..6, 00425EB0 on the mode selector (`[[00E198B4]+44h]`, listener +8h, list box +20h) with
  the value from the table at 00427840: 3, 0, 1, 2, 4, 4, 4.
- 9: for +28h 0..6: +20h == 0 -> 00425500 on the lobby `[[00E198B4]+48h]`; otherwise, when the
  lobby exists and 005D71B0 accepts, +28h++ and +Ch = 200.
- 10: +20h = 1; for +28h 0..6 (+28h 4..6 first call 005D6350(lobby, +28h - 4)): 004C6890(+1Ch),
  00A9C7C0(lobby+228h list box, 0), 005E1CF0(0), 005DA530, lobby vtable[20h](0.0f).
- 0Bh: 004DA6C0 and +Ch = 200.
- 0Ch (no movie): mode 1 runs `class_enumeration()` through 006B8AD0; mode 3 sets +18h and +1Ch to
  666 and writes the Memory group (00427040). Every mode then toggles the pause menu (004DB030).
- 0Dh: 004F8960 on the movie player `[00E18D48]` (00AB04A0 on its +20h widget).
- 0Fh: while +24h < 2, the mode is not 1 and `[00F8A304]` is null or has no flagged records
  (00425B20), 00425EB0 with value 4 on `[[00E198C4]+D8h]` (list box +13Ch) and +24h++; otherwise
  the same call with value 5.
- 10h: 00425EB0 with value 4 on `[[00E198C4]+E4h]` (list box +20h).

The call returns 0 on every path except screen 0Eh. Simplifications: the 00425D10 singleton is
fetched inside the two menu-command host methods; the result sink (008875D0 / 00887670) and the
empty NativeString of the class_enumeration call live inside `lua_run_string_006b8ad0`.

### 00426650, the MPakScenes walker

`__fastcall(driver)`. A scene path is `MPakScenes.` + the campaign name + `.` + the decimal mission
(+1Ch). The names come from the pointer table at 00E08048: `us`, `japan`, `multi`, `training`,
`dlcusa`, `dlcjap`. Lookups go through the globals (00B67980) and 00B68D70. The loop:

1. Campaign 2 (multi): while the mode index +28h < 7: if the scene is a table, its element
   [+28h + 1] ends the scan when it is not nil and its integer is non-zero; otherwise
   `MPakScenes.Modes.<+28h>` ends it when found with a non-zero integer. Anything else is +28h++.
   After the scan, +28h >= 7 moves to the next mission with +28h = 0.
2. +1Ch >= 100 moves to the next campaign with +1Ch = 0.
3. Return when the campaign reaches 6 or the driver mode is not 2 (mpak scenes).
4. Return when the scene is found and is a non-zero integer, or a table in campaign 2.
   Otherwise +1Ch++, +28h = 0 and go to 1.

### TerrainGrid streams

The stream behind the holder takes a trailing 0 on every call. The writer 0041EF90 emits
`TerrainGridLayer` (+64h), then +04h (float, +58h), the dimension n at +08h (+54h), +0Ch, +10h
and +14h (floats), and n*n bytes from the grid (+28h). The count is n*n from +08h, not the
vector's size. An empty grid triggers the checked `&v[0]` handler (00BF6713) first, which is a
host call here. 0041F130 writes `TerrainGrid`, the list size (registry+0Ch) and each layer.
004239E0 reads the same six fields back (the class name is read and dropped), resizes the grid to
n*n with 004219A0 and reads the bytes. 004246E0 sets the vtable, empties the grid vector, then calls
004239E0. The layer type is `TerrainGridLayerRecord` from `scene_contents_hosts.hpp`.

### Bit-stream payloads

00429140 writes `count` words, each zero-extended into a dword temporary, at `bits` bits apiece
through 00428F50. The voice payload has a 64-bit header and a vector of 10-byte frames at +8.
00429B00 writes the header, the frame count truncated to a byte, then that many 80-bit frames.
0042A480 reads the header and the count, resizes the vector to the count (new frames zeroed), then
reads each frame into a temporary and copies it in. Both loops have checked-index calls (00BF6713)
that cannot fire, so they are not modelled.

## Skipped

- Avoid-zone navigation and its debug drawing: belongs to cc9 (listed above).
- 00428420, 00428800 and 00428090: blast-site notification, belongs to cc9.
- Template, STL and pooled-storage instantiations (listed above): library-shaped containers, left
  as such.
