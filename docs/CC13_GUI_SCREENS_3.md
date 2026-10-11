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

## Hint tracker TriggerHint 00690FD0 (second turn, reconstructed, build-tested)

`hint_tracker_trigger_00690fd0` in `src/gui_screens_3.cpp`; the whole listing 00690FD0..006921A1
was read (`local/g5_l_00690fd0.txt`, filter `local/g5_flt.py`). `__thiscall(tracker, NativeString*
name, char forced)`, RET 8. The frame is steady after the prologue (`AND ESP,-8`, `SUB ESP,2B0h`,
three pushes): the new definition's copy sits at S+68h, the active/previous definition's copy at
S+190h, title S+38h, text S+24h, Lua command S+1Ch.

**The definition** (128h-byte value of the map at tracker+30h) was mapped from the copy
constructor 005FE010 and the Lua loader `0068F030 BSP_AwardTracker_LoadHintDefinitions`, whose
stack definition is at +98h: +00h Title, +08h Type (default `hints.gameplay_general_hint`), +10h
Description, +18h LuaCommand, +20h/+28h/+30h AlliedTitle/AlliedLuaCommand/AlliedDescription,
+38h/+40h/+48h the Japanese three, +50h the loader's byte argument, +54h TitleDescList (8 rows of
18h: title, description, Lua command), +114h critical, +115h hidden, +116h missionspecific,
+118h storingrule (default `repeatable`), +120h cooldowntime, +124h duration.

**Flow:**

1. +04h forces one trigger; otherwise the hints setting 00F8898C must be non-zero, and 1 admits
   only forced triggers. A name missing from the definition map is dropped.
2. With an active hint (+60h): unless the new hint is critical and the active one is not, the
   trigger is queued on the deque +68h when it is forced and both definitions have +50h, or when
   `*(*(00E198C4)+E4h)+4` is set; it is dropped when only the active definition has +50h.
3. storingrule `unique` drops a hint the profile has seen (`007F8E00` on game+650h);
   `missionunique` drops one already in the list +48h; `cooldown` with a non-zero cooldowntime
   drops it while the map +3Ch holds the key and otherwise starts the cooldown.
4. Text: the generic Description (with Title and LuaCommand); else, by the local side's nation
   (`*(game+18CCh+slot*4)+28h`, 0 Allied, 1 Japanese), that nation's three fields; else the
   TitleDescList row of the local player slot (game+18ECh). If none applies but the other
   nation has a description, the hint is dropped. On PC prompts (00F88A30 clear) the key
   `<description>_pc` replaces the description when the localisation table resolves it.
5. The previous timed hint (+1Ch, its seconds in +24h) counts as seen when the hints screen has
   less than a quarter of them left (`(int)hints+54h`, compared after FILD against +24h * 0.25);
   it is then stored under its own rule, and re-triggering the same name is dropped.
6. The hints screen is dismissed (0054E610); a non-empty Lua command runs through
   `006B8AD0(command, 0, 0, 2)` on `*(game+1A08h)+4` with a Hungarian diagnostic for failure or
   success; unless hidden, `ShowHint(critical, text, title, duration)`.
7. History: key = Type, prefixed `AAAAAA` for `hints.missionhint`. The map +94h gets the name
   moved to the back of its list (or a new single-entry list, mirrored into +7Ch); names new to
   +94h's list are also appended to +7Ch's.
8. Bookkeeping: a critical, timed or hidden hint is marked seen (profile `007FAE20`, or the
   recent list +54h for missionspecific); critical/hidden hints are also stored under their
   rule and clear +1Ch/+24h; timed hints set +1Ch = name and +24h = the screen's seconds;
   untimed non-critical hints clear +1Ch/+24h and set +28h = name. Finally +60h = name.

**Lua-visible for cc9:** the hint definitions come from a Lua table (field names above), and a
shown hint's `LuaCommand` (or its Allied/Japanese/row variant) runs as mission Lua through the
`game+1A08h` host. `forced` and the hints setting decide whether script-triggered hints show at
all. The reconstruction is not wired; nothing here changes game behaviour.

Names: `0068E450 BSP_AwardTracker_FindHintDefinition`, `0068E4C0 BSP_AwardTracker_FindCooldown`,
`0068F030 BSP_AwardTracker_LoadHintDefinitions` (provisional), and the corrected evidence of
`0068EA70 BSP_AwardTracker_TouchRecentHint` (the argument is a by-value NativeString, not a kind
and a pointer). Uncertain at first: the meaning of +50h (resolved in the third turn: the OTHER-group flag); `005FFB20` on the active name is a mapped-value
lookup that would insert a default definition, which the reconstruction treats as "not critical,
no +50h".

## Idle camera screen 2Ch (second turn, reconstructed, build-tested)

`idle_camera_enter_0054f900`, `idle_camera_exit_0054efc0`, `idle_camera_update_00550410` and
`idle_camera_on_released_0054f3e0` in `src/gui_screens_3.cpp`; the four listings were read whole
(`local/g5_l_0054f900.txt`, `..0054efc0.txt`, `..00550410.txt`). Renamed from `BSP_HudScreen2C_*`
to `BSP_HudIdleCameraScreen_*`, plus Construct 0054F8C0 and Destruct 0054EC90.

The screen cycles a camera through authored shots. Shots live in per-game-mode `std::vector`s of
1Ch-byte records: allied (nation 0) at `00E19328 + mode*10h`, japanese (nation 1) at
`00E193A8 + mode*10h` (begin pointers `00E1932C`/`00E193AC`). A shot is `{+00h eye entity, +04h
target entity, +08h eye name, +10h target name, +18h enabled}`. Who fills these vectors was not
traced.

- **Enter**: makes the 384h-byte camera node once (`00432750`, attached with an identity matrix by
  `00923870(0, game+19CCh, &I)`, observed through the screen's observer sub-object at +8), copies
  the game view node's world matrix (`game+19FCh`, refreshed by `00B6DB70` unless `+5Ch & 2`) into
  `camera+74h`, sets the timer to 10.0 (00CE38B8, so the first Update switches shot at once), the
  shot to 0, and the camera mover to the node (`004BC410`). For both nations it resolves eye then
  target by name through `00925A90` on `game+19CCh`; a missing eye leaves the old target pointer
  untouched. Found entities are observed once; a shot is enabled only when both were found.
- **Update** (`dt`, RET 4): timer += dt. With the local side's list (nation 0 allied, else
  japanese) it advances to the next enabled shot when the timer passes 6.0 s (00CE6630), when the
  current shot is disabled, or on input action 11Ah; it goes back on 11Bh. Both actions are
  ignored while `*(*(00E198C4)+BCh)` (`006529E0`) blocks input; a blocked 11Ah falls through to the
  11Bh test. A change resets the timer. Then both entities' poses are refreshed when stale
  (target first), and the camera's world matrix becomes the inverse of the look-at from the eye
  entity's position (+FCh) to the target's, world up (0, 1, 0) (`00B63F10`, `00B63B30`, `004134F0`,
  `0042ED50`). The search loops have no exit: a list with no enabled shot hangs the native code,
  and the reconstruction keeps that.
- **Exit** unregisters every non-null shot entity of both nations and leaves pointers, camera and
  mover as they are. **OnObservedReleased** (observer slot +04h on the sub-object) clears the
  camera pointer when the camera goes, else nulls and disables every shot naming the entity.

x87: only the timer `FLD/FADD/FSTP` in Update; the look-at and inverse are the existing camera
routines and run host-side. No RNG is drawn.

## Hint tracker helpers and the hint loader (third turn, reconstructed, build-tested)

All in `src/gui_screens_3.cpp`, listings read whole. None is wired.

**Lua bindings.** The HintSystem binding table at 00E0B7F0 pairs `ShowHint` 008D1F50,
`ShowHintForced` 008D2190, `AddStoredHint` 008D24F0, `RemoveStoredHint` 008D2340, `HideHint`
0088CEC0 and `IsHintActive` 008D2850 (the wrappers themselves are in segment 56, not leased, not
renamed). The two stored-hint wrappers call into the tracker:

- `00690B20 BSP_AwardTracker_AddStoredHint` (Lua AddStoredHint): a known hint with storingrule
  `unique` is marked in the profile (`007FAE20` on game+650h), with `missionunique` appended to
  the list +48h, in both cases only when `0068E600` says it has not been shown.
- `00690C30 BSP_AwardTracker_RemoveStoredHint` (Lua RemoveStoredHint): only for a shown hint;
  resets +18h (last unit class) to 13h when it equals the player unit's class, then unmarks the
  profile (`007FAE60`) when the profile holds it, else removes it from +48h with
  `0068EE00 BSP_AwardTracker_DropMissionSeenHint` (rebuilds the list without case-insensitive
  matches).

**History maps.** `00690CA0 BSP_AwardTracker_ClearNewHintHistory` empties +7Ch and is called
only by the pause screen's Register (00602280), so +7Ch reads as "hints new since the pause
menu was last opened". `00690CD0 BSP_AwardTracker_RebuildHintHistory` empties +94h and refills it
from the profile's shown-hint list `*(game+65Ch)`: every name whose definition is not
missionspecific goes under its Type (no `AAAAAA` prefix, nothing into +7Ch). Ghidra shows a
`return` after the new-key arm; the listing jumps back to the loop head (00690F92).

What the gameplay host stub `flush_award_tracker` (`src/game_hosts_mission.cpp`, caller
`005922F0` play-selected-mission at 00592314) stands for: 00690CD0 is that rebuild, not a flush.
It touches only the history map and reads the profile; it does not show, queue or store hints.
The file is cc9's and was not edited.

**Loader.** `006921B0 BSP_AwardTracker_ReloadHintTables` empties +7Ch, runs
`Scripts/datatables/HintSystemMessages.lua`, empties the definition map +30h, and loads
`HintSystemMessages.SYSTEM.{Weapon, Unit, Gameplay}` with the flag 0 and
`HintSystemMessages.OTHER.{Single, Multi}` with the flag 1 through `0068F030
BSP_AwardTracker_LoadHintDefinitions`, then rebuilds +94h and clears +60h, +1Ch, +24h, +04h,
setting +18h = 13h. So the definition's +50h byte is **"OTHER-group hint"**: TriggerHint queues a
forced OTHER hint behind an active OTHER hint and drops a SYSTEM hint while an OTHER hint is
active (see the TriggerHint section).

0068F030 (`__thiscall(tracker, LuaObject table by value, char other)`, RET 18h) reads per entry:
`id` (no nil test), the ten string fields (nil gives ""), `critical`, `storingrule` (default
`repeatable`), `cooldowntime`, `duration` (default 12), `Type` (default
`hints.gameplay_general_hint`), `hidden`, `missionspecific` (which then forces Type to
`hints.missionhint`; the inlined assign compares against EBP = 11h set at 0068FAEF), and
`TitleDescList`, then assigns `map+30h[id]`. Two native quirks are recorded in the source:
`cooldowntime` is stored only when present and the default constructor `005FD650` does not
clear +120h, so an entry without it inherits the previous entry's value (the first entry an
uninitialised stack word); and TitleDescList rows are written with no bound, so a ninth element
would overwrite +114h onward. The reconstruction carries the cooldown across entries and stops
the rows at eight.

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
## Handoff update (cc13-gui5, second turn, 2026-10-11)

Items 1 and 2 above are done: TriggerHint 00690FD0 (commit 13c58e207) and the idle camera
screen 2Ch Enter/Exit/Update/OnObservedReleased (this commit), all build-tested and not wired.
Ledger: 13 more names (4 tracker, 8 idle camera, 1 corrected), 5 more function records;
`reports/cc13_gui5.json` now 192 call rows, 0 failed. Open:

1. The hint-definition loader `0068F030` (1813 instructions) and its caller `006921B0`: the
   field map in the TriggerHint section gives the offsets; the meaning of the +50h byte argument
   is the main unknown and decides when a forced hint queues.
2. Remaining tracker helpers 0068EE00 (caller 00690C30), 00690B20, 00690CA0, 00690CD0 (the
   `flush_award_tracker` stub in `src/game_hosts_mission.cpp`).
3. The filler of the idle-camera shot vectors 00E19328/00E193A8 (probably mission load).
4. Items 3 to 5 of the first handoff still stand.
## Handoff update (cc13-gui5, third turn, 2026-10-11)

Done this turn: the tracker helpers 00690B20, 00690C30, 0068EE00, 00690CA0, 00690CD0 and the
loader 0068F030 with its reload 006921B0 (7 functions, build-tested, not wired). The +50h byte
is resolved. Still open: the HintSystem Lua wrappers in segment 56 (008D1F50, 008D2190, 008D24F0,
008D2340, 0088CEC0, 008D2850; mission-script visible, so cc9 should decide on them), the
idle-camera shot filler, and items 3 to 5 of the first handoff.