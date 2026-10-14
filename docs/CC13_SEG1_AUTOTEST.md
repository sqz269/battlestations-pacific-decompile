# Segment 1: the automation driver (cc13-camera2)

Segment 1 is 00413D10-0042A830. Its keywords are mpakscenes, endgroup, crash, avoidzoneg, avoidzone,
training and terraingrid. This packet takes the non-gameplay part: the command-line automation driver
behind `auto mpak classes`, `auto mpak scenes` and `auto test <name>` (docs/APP_INIT_BOOTSTRAP.md
lines 122-124 and 386), with its watchdog, crash hook and `_auto_.log` writers. Code:
`include/bsp/seg1_autotest_cc13.hpp`, `src/seg1_autotest_cc13.cpp`. Status: **reconstructed and Win32
build-tested only**. There is no fixture and nothing was run in game. Call report:
`reports/cc13_camera2_seg1_calls.json` (50 rows, verify_report_calls 0 failed).

## Triage

`local/cm2_triage1.py` over the index: 496 functions, 125 with reconstruction records, 117 EH
funclets / CG / STL, 254 open. Of the open ones:

- **Avoid zones and terrain grid layers** (0041B4E0..0041F130, 00420830, 004216B0, 00421500..00424730):
  this is cc9's ship-AI / pilot terrain-avoidance data (AvoidZoneManager, AvoidZoneRegistry,
  TerrainGridLayer serialize/deserialize, the "AvoidZoneG"/"AvoidZone"/"TerrainGrid" strings). Listed
  only, not touched.
- **STL and template instantiations** (list/vector/tree helpers called from Unwind funclets and
  Catch_All blocks, e.g. 0041CB90 with 109 callers, 0041E0F0, 0041F5D0, "list<T> too long" at
  0041FDD0): left alone.
- **Small math helpers with no direct callers** (00413D10..00419F70): most are referenced only
  through vtables or COMDAT folding. Not triaged further.
- **The automation cluster** (00425500..00427C20): this packet.

## The driver object

The 50h-byte singleton from 004C1E90 holds the automation state. Its ledger name,
BSP_ProfileHintsOwner_GetSingleton, is provisional and comes from a hints reader. The same object
is constructed by 00426250, and 13 screen Update functions call 00427190 on it. Fields:

| offset | meaning |
|---|---|
| +4h | watchdog thread handle (004278A0) |
| +8h | mode: 1 mpak classes (00425560), 2 mpak scenes (00425570), 3 test (004278A0) |
| +Ch | frames until the driver's next step (200 at mpak-classes start, 50 after each step, 200 after the tactical library) |
| +10h | consecutive frames with a movie playing (00427190 stops it after 2) |
| +18h | campaign: 0 usa, 1 japan, 2 multi, 3 training |
| +1Ch | mission index (0-based) |
| +24h, +28h, +30h | counters / flags (+24h set by 00425570 when byte 00E17659 is set) |

## Reconstructed

| address | name | notes |
|---|---|---|
| `00425560` | BSP_AutoTest_SelectMpakClasses | mode 1, countdown 200 |
| `00425570` | BSP_AutoTest_SelectMpakScenes | mode 2, clears +14h..+28h |
| `004278A0` | BSP_AutoTest_ParseTestName | hook, start time, watchdog thread, `LoadWaitQuit:<campaign>,<mission>` |
| `004256F0` | BSP_ClockTimestamp_AddInto | out = left + right (rescaled into left's frequency) |
| `00425780` | BSP_ClockTimestamp_Less | equal frequency: tick compare; else whole seconds |
| `004258D0` | BSP_AutoTest_WatchdogThread | 900 s budget from the test start, 1 s polls, crash.log "Timed out." and exit code 3 |
| `00425B80` | BSP_AutoTest_FatalHook | function created by cc13-camera2; crash.log "<message>" and exit code 2 |
| `00426E60` / `00426ED0` / `00426F30` / `00427040` | BSP_AutoLog_Group / EndGroup / Value / Memory | StringBuilder lines `GROUP <name>`, `ENDGROUP`, `LOG <name_with_underscores> <value>`, and an empty Memory group |
| `00425B20` | BSP_RecordVector_CountFlagged | count of 18h-byte records with byte +11h set |
| `00426350` | BSP_AutoTest_EnsureScreen | list box `[[[00E198AC]+58h]+1B8h]`: when 00A9C990 gives the screen, listener (menu+8h) vtable[4](00425E50 row, list box) and 1; else 00A9C860(list box, screen) and 0 (row and list-box arguments added by cc13-seg1b) |

The test-name parse: 00BD20A0 splits the argument on ':'. The first piece must equal "LoadWaitQuit"
(__stricmp, 00CE3AD8). The second piece is split on ','. Its second element goes through atol - 1 into
+1Ch, an empty string when missing. Its first element is compared case-insensitively with "usa",
"japan", "multi" and "training" (00CE3AD4, 00CE39D4, 00CE39CC, 00CE39C0); no match leaves +18h
unchanged. The mode becomes 3 only when the test name matched. The hook, the start time
(00E08060..00E0806C from the frame clock's vtable 20h) and the watchdog thread are set up before the
parse, whatever the name. Strings near the writers (`_auto_.log`, `%d`, `%.3f`) show where the lines go.
The sink is the StringBuilder's destructor (00425F80), modelled as `log_line`.

Named only: `00425BD0` BSP_NativeString_ToLong, `00427AC0` BSP_NativeStringVector_Size,
`00425EB0` BSP_AutoTest_SelectListboxItem, `00425500` BSP_AutoTest_FocusLobby, `00427190`
BSP_AutoTest_Step, `00426650` BSP_AutoTest_NextMpakScene.

## Handoff (cc13-camera2)

Taken by cc13-seg1: 00427190, 00426650, 00425EB0, 00425500 and 00425A80 are reconstructed in
`src/scenes_seg1_cc13.cpp` (docs/CC13_SEG1.md), and `AutoTestDriver` gained the +20h byte. The list
below is the original handoff.

Next, in order:

- `00427190` BSP_AutoTest_Step (1776 bytes). It is the per-screen driver. Screen 0Ch counts movie
  frames and stops a movie after 2 (BSP_EngineMovie_Stop, cc9's; call it through a host, do not
  reimplement it). Screen 0Eh returns 1. Every other id decrements +Ch and, at 0, reloads 50 and
  switches on the screen id: 0 title skip, 1 the main menu (runs SCRIPTS/datatables/MPakScenes.lua
  once when byte 00E08070 is set, then 00426650 and the campaign page through 00426350 by +18h), 2
  the command prompt, 3 the mission list (00426350(+1Ch), +1Ch++ on success), 4 objective rows, 5
  the tactical library / mission list, 6 and later (not read). The pseudocode is readable; reconstruct
  it behind a screen host.
- `00426650` (2064 bytes): the MPakScenes Lua walker (MPakScenes.Modes.<n>, then MPakScenes.<name>).
- `00425A80` / `00425B50` clear 00E17664 (the driver singleton's publication) and store vtable
  00CE3818. Their relation to the ledger's complete 00426300 / 00426320 destructors is unresolved,
  so they are not named.
- `00427C20` is an STL tree leftmost-node walk (byte +15h is the nil flag). Left as STL.
