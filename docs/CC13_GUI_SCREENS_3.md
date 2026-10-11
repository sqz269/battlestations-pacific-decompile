# GUI screens of segments 16, 28 and 4 (packet `cc13_gui5_screens3`)

Worker cc13-gui5, 2026-10-11. Lease `cc13_gui5_screens3` on 00544E60-00554600, 00680DB0-00696450
and 004486C0-00453C30. Source: `src/gui_screens_3.cpp`, `include/bsp/gui_screens_3.hpp`. Everything
below is **reconstructed and build-tested** only: nothing is wired into the game host and nothing
was run in `bsp_game`. Names are hypotheses.

Conventions follow docs/CC13_MENU_SCREENS_2.md: screen vtable slots +00 id, +10 Register, +14
Layout, +18 Enter, +1C Exit, +20 Update, +24 Collect; listener vtables are numbered by the slot
offset of the full listener interface (the first entry is slot 08h, `BSP_GuiWidgetListener_Empty08`).

## Triage

- Segment 16. The packet keywords (`basicship2`, `basicplane3`, `basicsub`, `button_framebox`)
  belong to the HUD hints screen (id 34h) and its tutorial chain; that screen is reconstructed
  whole below. The rest of the segment is the weapon-group screen 2Eh (partly named earlier), the
  main-menu command owner (cc13 frontend packets), screen 2Ch (camera-heavy, named only) and the
  control binding screen's listener slots (named only; the screen belongs to the options packets).
  `0054E440 BSP_TutorialHint_AdvanceBasicStep` is called from `GGame::OnMove` and is a host hook
  in `src/game_hosts_mission_frame.cpp` (cc9); it is reconstructed here in my file and not wired.
- Segment 28. The `gvmultimenu`/`pushrequestinterface`/`warning` routines were already named
  (MultiMenu, HudWarningScreen, FrontEndManager). What was left is the hint tracker (the
  `BSP_AwardTracker_*` object of docs/GAME_AWARD_TRACKERS.md, which feeds the hints screen) and
  the observer plumbing. Small tracker helpers and the deque instantiations are named.
- Segment 4. The `panel`/`sequence`/`setpanel`/`suppressinterruptmsg` keywords are the mission
  dialog sequencer: 004507D0 parses a dialog table (`priority`, `requestTime`,
  `suppressInterruptMsg`, `defaultPause`, `nextUnit`, `sequence`, `breakRequested`, `lastPanel`,
  `panelStates`) and 0044BE50 builds one command per `sequence` element. Both are cited by cc9's
  docs/SQUADRON_LAND_TASK.md and hosted by cc9's packet `cc9_dialog_sequencer`
  (`src/game_hosts_script_orders.cpp`), as are 00452360 and 00452BD0, so nothing there was
  reconstructed. Only the uncited command-class slots were named (below).

## Functions Ghidra lacked

`local/g5_scan.py` (copied from menus4's scan) lists `.rdata` dwords pointing into the three ranges
at addresses with no function. Only hits inside the vtable area (00CE0000..00D00000, aligned) were
taken; extents from `local/g5_extent.py`, holes from `local/g5_holes.py` (00549DD0 has two and
0054EFC0 one `LEA` alignment hole inside loops), containment by `local/g5_contain.py` (every start
reported no containing function). 31 functions were defined with `tools/ghidra_define_function.py`;
the record is `reports/cc13_gui5_defined.json`: 00547370, 00547380, 00549200, 00549590, 00549DD0,
0054A980, 0054D660, 0054D670, 0054E370, 0054ECF0, 0054ED00, 0054EF90, 0054EFC0, 00550410, 00551BB0,
00551BC0, 005539E0, 00689BD0, 006935B0, 00694250, 00449A30, 00449A50, 00449AB0, 0044B380, 0044B3B0,
0044B3E0, 0044C220, 0044C270, 0044C2F0, 0044C340, 0044CCD0. 0054ED00 took its existing ledger name
`BSP_FrontEndScreen_Register` (a 5-byte `JMP 004F71D0` in vtable 00CEE408 +10h). The segment-28 and
segment-4 definitions were named in the second batch (below).

## HUD hints screen (id 34h)

| Slot | Address | Name |
|---|---|---|
| ctor | 0054D680 | `BSP_HudHintsScreen_Construct` (was tagged `CG_array_ctor_helper`) |
| dtor body | 0054D550 | `BSP_HudHintsScreen_Destruct` (called by 0054D840) |
| +00 | 0054D660 | `BSP_HudHintsScreen_GetScreenId` (`MOV EAX,34h`) |
| +04 | 0054D670 | `BSP_HudHintsScreen_Virtual04True` (`MOV AL,1`) |
| +10 | 0054D860 | `BSP_HudHintsScreen_Register` |
| +14 | 0054D980 | `BSP_HudHintsScreen_Layout` |
| +18 | 0054E380 | `BSP_HudHintsScreen_Enter` |
| +1C | 0054E3D0 | `BSP_HudHintsScreen_Exit` |
| +20 | 0054E6B0 | `BSP_HudHintsScreen_Update` |
| +24 | 0054E370 | `BSP_HudHintsScreen_CollectChildren` |
| -- | 0054DE90 | `BSP_HudHintsScreen_PlayCue` |
| -- | 0054E020 | `BSP_HudHintsScreen_ShowHint` (called by `00690FD0 BSP_AwardTracker_TriggerHint`) |
| -- | 0054E440 | `BSP_TutorialHint_AdvanceBasicStep` (name kept) |
| -- | 0054E610 | `BSP_HudHintsScreen_Dismiss` (`hud_refresh_0054e610` in menu_screens_2) |

Vtable 00CEE290, 7Ch-byte object at HUD manager +A8h (docs/HUD_SCREEN_PAGES.md). The constructor
loads two cue samples by name from the char* table 00E08698..00E086A0:
`environment/environment_warning_hint.fsb` (cue 0, normal hints) and
`environment/environment_warning_critical.fsb` (cue 1, pausing hints).

Fields (offsets from the listings): +08h pausing, +09h cleared alongside (no reader found), +0Ch/+10h
samples, +14h/+18h tracked sounds, +1Ch page, +20h/+24h pause/normal groups, +28h title, +2Ch pause
text, +34h normal text, +38h button text, +3Ch/+40h pause icons, +44h normal icon, +48h/+4Ch frame
boxes, +50h persistent (shown with 0 seconds), +54h seconds left, +58h grow time 0.15 (00CE7818),
+5Ch grow elapsed, +60h/+68h frame target sizes, +70h..+78h the normal text's registered position.

Behaviour:

- **ShowHint 0054E020** `(char pausing, text, title, int seconds)`, RET 10h. In a session
  (`game+1FE4h != 0`) without `+218Ch`, a set `game+61Fh` suppresses all hints. `seconds = -1`
  asks for the subtitle reading time `2 * (lines(+ECh) * sub+7Ch + sub+80h)` where `sub` is the HUD
  manager's +A4h screen (33h, subtitles); 0 makes the hint persistent. A pausing hint shows the
  pause group, calls `004D94F0(game, 1)`, hides text/icons/button/title until grown, sets the
  frame target height to text height + 0.15, places the title above (`y - (h/2 + 0.027)`) and the
  button below (`y + h/2 + 0.037`) the frame box, and plays cue 1. A normal hint sets the text,
  shifts it 0.025 left when untimed, sizes the frame to `(width/960 + 0.3 or 0.25, height +
  0.006)` and plays cue 0 when timed. The x87 sequences were read from the listing
  (`local/g5_l_0054e020.txt`); the 0.027/0.037/0.15 constants are floats widened to double.
- **Update 0054E6B0**: idle without an active tracker hint (name length 0). The frame box grows
  from half to full size over 0.15 s; when grown, a normal hint reveals its text (and icon when
  timed) every frame (+5Ch is not latched in that branch) and releases cue 0; a pausing hint
  latches, reveals text, icons, title and the button prompt `hints.pressa` (pad prompts, 00F88A30
  set) or `hints.pressenter`, and releases cue 1. In effective game mode 8 a pausing hint freezes
  the scene once grown (`004CD0F0(1,1,1)`, input context 1Bh level 5) and waits for input action
  4Ah or profile query 0Eh, then unfreezes and chains the next basic step itself. Otherwise action
  4Ah (pausing only) or profile query 0Eh calls AdvanceBasicStep. Unless persistent, +54h counts
  down and at zero hides whichever group is shown and finishes the tracker hint.
- **AdvanceBasicStep 0054E440**: hides the pause group, resets input actions FFh and 4Ah, finishes
  the active hint and triggers the next step of `BASICPLANE -> BASICPLANE2 -> BASICPLANE3`,
  `BASICSHIP -> BASICSHIP2`, `BASICSUB -> BASICSUB2` (case-insensitive) with forced 0. Ghidra's
  decompiler drops the trigger tail as unreachable; the listing (0054E59B..0054E5AE) keeps it.
- **Dismiss 0054E610**: zeroes +54h, hides the shown group (pausing: also `004D94F0(0)` and, when
  the game's cinematic hide flag +634h is set, cinematic off, input context 1Bh level 0 and
  `004D9830(game, 1)`), then finishes the tracker hint.
- **Enter/Exit/Collect**: Enter shows the page, hides both groups, clears the hint pause and the
  tracker queue (0068EA00). Exit hides the groups, clears the pause, the tracker list +48h and the
  tracker's +1Ch name. Collect hands `screen+1Ch` to 004D6790.

`004D94F0(game, flag)` (segment 10, outside the lease, not renamed): when `game+620h` differs from
the flag and `game+624h` is 0 it stores the flag and passes the screens 28h, 55h, 57h, 5Dh and 34h
with it to 004D6480, which the movie screens also call. It reads as "pause everything but these
screens"; the name `BSP_Game_SetHintPause` is left for the owner of segment 10.

Input records: the input manager's action array at `*(004BEC00()+4)` has 30h-byte records;
`00535EE0(action)` is "pressed this frame" (`+20h` set and `+1Ch > 0`, unless `+28h` set and `+24h >
0`). Update inlines the same test for action 4Ah (record +DE0h).

Lua-visible behaviour for cc9: none directly; every hint arrives through the tracker's
TriggerHint 00690FD0, whose callers were not surveyed in this packet.

## Named without reconstruction

- Screen 2Ch (vtable 00CEE408, manager +94h, constructor 0054F8C0, in the idle-camera set
  `{2Ch, 29h, 49h}` of `src/ingame_interface.cpp`): GetScreenId 0054ECF0, Enter 0054F900, Exit
  0054EFC0, Update 00550410, observer adjustor 0054EF90. The bodies are camera work (entity lookup
  by name, observer pairs, look-at 00B63F10, `BSP_Game_SetCameraMover`) and are left for a camera
  packet.
- Screen 2Eh: GetScreenId 00547370, adjustor 00547380; weapon-group observer slot 00549200
  (`OnObservedEvent`, kind 5 steps the group).
- Main-menu command owner listener slots: 00549DD0 hover, 00549590 no-op, 0054A980 adjustor.
- Control binding listener slots: 005539E0 released, 00551BC0 hover, 00551BB0 no-op.
- Hint tracker: 0068E420 `FinishActiveHint` (+60h name emptied), 0068EA00 `ClearHintQueue`,
  0068EA70 `TouchRecentHint` (most-recent-last list at +54h), 00692FB0 `TriggerForcedHint`, and the
  `deque<NativeString>` instantiations 0068E780 pop_front, 0068EB90 push_back, 0068E8A0 grow map.

## Uncertainty

- `004D9830` is tagged as a scalar deleting destructor but is called on the game singleton with 1
  from the hints screen; its role is open.
- Game mode 8 read as the tutorial mode is a hypothesis from the BASIC* chain it gates.
- Floating-point: the reconstruction computes in float/double as the listing's stores do, not in
  x87 extended precision.

## Second batch: segment-4 command slots, session message E1h, segment-28 slots (names only)

The dialog command classes behind 0044BE50 (base vtable 00CE4940; msg 00CE49CC, setpanel 00CE49DC,
hidepanel 00CE4950, pause 00CE4960, callback 00CE49EC; type ids 0..4 from slot +04h, hidepanel's
and pause's getters 004483A0/004483C0 lie below the lease and were not defined). Slot +08h reads
and +0Ch writes through tagged values `{kind 0 string, 1 int, 2 float; payload}`; the reader's
+0Ch is read-with-default and +10h a plain read, the writer's +0Ch writes one key. Named:

- 0044C060 `BSP_DialogCommand_WriteBase` (`panel` = index + 1, then `type`), 0044C220/0044C270/
  0044C2F0/0044C340 the msg/setpanel/pause/callback writers (`message`; `name` + `character`;
  `time`; `callback`), 0044A3F0 `BSP_DialogCommand_ReadBase`, 0044A4C0 `..SetPanel_Read`, the three
  type getters 0044B380/0044B3B0/0044B3E0 and the two 8/0Ch-byte scalar deletes 00449A30/00449A50.
- Behaviour cc9 may want (Lua-visible through the dialog tables): every command that reads
  `panel` (default 1) stores 0 over the value right after the read (0044A42F..0044A431,
  0044A507), and pause/callback set panel 0 without reading, so the authored `panel` key never
  reaches the command object. A `type` string other than the five names leaves the factory's
  object null and the original then calls through it (0044BFBB), i.e. it crashes.
- Session message type E1h (vtable 00CE4B30): 00449A70 write body (byte +10h in 8 bits, string
  +18h, byte +20h in 2 bits, string +24h), 00449AB0 read body, 0044CCD0 type test.
- Segment 28: 00689BD0 attract-screen no-op slot, 006935B0 observer no-op slot, 00694250 the
  scalar delete of the class whose singleton is 00E198E0.

## Handoff (cc13-gui5, 2026-10-11)

Commits on agent/cc13-gui5: 10c567ae9 (hints screen reconstruction, 31 definitions, 35 names) and
the handoff commit (18 more names, this section). 10 functions reconstructed and build-tested
(`./scripts/build.ps1` passes, 2/2 tests), none wired or run in `bsp_game`; 53 ledger names applied
to Ghidra with evidence comments (prior values in the worktree's
`local/ghidra-annotations-20261011T2221*.json` and `..T2233*.json`); `reports/cc13_gui5.json` has 101
call rows, `tools/verify_report_calls.py` 101 checked, 0 failed. Scripts are in the worktree's
`local/` with the `g5_` prefix (copies of the menus5/menus6 helpers, paths fixed to `local/g5_*`).

Open, in order of value:

1. `00690FD0 BSP_AwardTracker_TriggerHint` (1284 instructions, RET 8): the producer of every hint
   and the only caller of ShowHint 0054E020; it decides pausing/text/title/seconds. Reconstructing
   it would complete the hint path; the tracker's other unnamed helpers are 0068EE00, 0068F030 (1813 instructions, RET 18h), 00690B20, 00690C30, 00690CA0, 00690CD0 (the
   `flush_award_tracker` host stub in `src/game_hosts_mission.cpp`), 006921B0. Check
   docs/GAME_AWARD_TRACKERS.md and `src/award_trackers.cpp` first.
2. Screen 2Ch Enter/Exit/Update (0054F900, 0054EFC0, 00550410): camera work for the idle-camera
   interface; give it to a camera worker with x87 support.
3. `004D94F0` (game, segment 10) reads as "set hint pause"; rename under a segment-10 lease.
4. Weapon-group screen leftovers in segment 16 (00544E60, 00544FA0, 00545270, ...): check for
   0077D600 unit commands before taking them; 00549200 steps the group on observer kind 5.
5. Segment 4's dialog sequencer stays with cc9 (packet `cc9_dialog_sequencer`).