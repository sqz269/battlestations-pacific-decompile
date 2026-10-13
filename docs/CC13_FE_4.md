# Front-end segments 15 and 22, reconstruction pass (packet cc13-fe4)

Segment 15 (00527CB0..00544B60: control layout, HUD counters, credits, menu prompt, HUD
formation, frame stats, free camera, debug text lists) and segment 22 (005CD070..005DF510:
movie screens, multiplayer chat, error, pause and game-lobby screens). This pass takes the
functions that had a reviewed ledger name but no reconstruction record and no tag (334 when the
packet started, `local/bsp_index.sqlite`).

Files: `include/bsp/frontend_seg15_22_cc13.hpp`, `src/frontend_seg15_22_cc13.cpp` (registered in
`cmake/startup.cmake`). Records: `local/fe4_records.py` writes them (`ledger.upsert`, lease-checked).

## Status vocabulary

- **Reconstructed, build-tested**: C++ written from the live decompile (`bsp.py ghidra decompile`),
  checked against `bsp.py disasm-raw` wherever ECX, a stack argument, x87 or a store order
  mattered, and compiled by `scripts/build.ps1` (MSVC Win32). Nothing is ABI-compatible or
  game-validated, and nothing was run in `bsp_game`.
- **Backfilled**: an existing reconstruction elsewhere in `src/` with no record; checked against
  the decompile, record added (`reconstructed_ledger_backfilled_from_source`).

## Model

The routines take the image object as raw bytes (`Raw` from `bsp/ingame_gui_cc13.hpp`) and reach
the engine through `Seg15Host`, which extends `HudScreenServices` from the cc13-fe2 pass. Every
host method names the image routine it stands for; a global the image touches is a host accessor
that names its address (`global_object`, `global_block`). Routines of these segments that this pass
did not reconstruct are host calls too (for example the counter layouts 00529D70, 0052A100,
0052A860, 0052BB10 and the entry constructors 0052C1B0 / 0052C640).

ABI facts used throughout (all from disasm-raw):

- 004CC460 (push interface request) runs on different managers: the control layout uses
  *00E198B8 (options), the credits listener *00E198AC (main front end).
- 004D92B0 (action edge or repeat) takes ECX = *00E188A8.
- Collect-children slots (+24h) are `__thiscall(screen, out), RET 4`, pushing page-slot addresses
  through 004D6790.
- Native strings passed by value (count, data) are freed by the callee; the model takes a
  `std::string_view`.
- Several containers are MSVC 8 vectors `{allocator, first, last, end}`; `vector_insert_n` models
  `_Insert_n` once (1.5x growth, the xlen limit per element size) for 0052F9E0 and the later
  instances.

## Reconstructed

### Trivial slots

`kTrivialSlots` (35 rows, generated from disasm-raw by `local/fe4_triv.py`): screen ids 0Eh control
layout, 51h HUD counters, 08h credits, 55h menu prompt, 2Ah HUD formation, 57h frame stats, 2Bh free
camera, 36h movie camera debug, 15h chat, 1Dh error, 19h pause (multiplayer); the byte predicates,
empty bodies with their RET sizes, the adjustor thunks and the tail jumps (005361B0 -> 00535C30,
00536250 -> 00535DD0, 0053FF00 -> 0053F250).

### Containers

Pointer arrays `{data, size, capacity}`: reserve 00529700 / 00529760, resize 005297C0 / 005299D0,
push back 00529900 / 00529940, word push back 005298C0, erase 00529620, index 00529650, destroy
00529C50 / 0052AE40. The 18h-byte observer-handle records (vtable 00CE75CC, observed +14h, set byte
+10h): reserve 00529C70, push back 0052AD50, swap-remove 0052ADD0 (re-registers the observer pair),
resize 0052AE60, destroy 0052C0A0.

### Control layout (0Eh)

Construct 00527D00, destruct 00527DA0, register 00527E60 (FE_controls_X360_listbox +10h,
FE_controls_X360 +14h), bind 00527F90, exit 00528920, update 005289B0 (back = action 4Bh ->
interface 0Ch), collect 00528A60, button-flag take 00527CB0, listeners 00527D50 / 005289E0 /
00527E30.

00527F90 copies the stick-label texts: for each of the two name triples at 00E08674
(LeftStick*/RightStick* Vert/Horz/Click) and each group widget (Plane_Group, Ship_Group,
Submarine_Group at +1Ch/+20h/+24h), the child's text (+F4h) is copied into the next of 18 native
strings at 00E18DC0.

### HUD counters (51h)

Screen layout: +8h page GUI_counters, +0Ch HP entry, +10h countdown entry, +14h score entries
(pointer array), +20h visible list (pointer array). Entry (0ACh bytes): +0h root widget, +4h/+8h
position widgets, +28h text, +70h record array, +7Ch/+88h/+90h strings, +98h seconds, +A0h time
format, +A4h owns widget, +A5h..+A9h occlusion flags.

Routines: format time 00528B00, set value 00528BE0, bar 00528E90 (frac = max(value, 0.1) / max;
scales {frac, 1}, {frac * a, 1}, {clamp(a - b, 0, 1) * frac, 1}), occlusion 00528F70 / 00529200 /
005293B0 / 00529810 (with the body at 00529120, which has no Ghidra function), remove 0052AB40 /
0052AB90, construct 0052B030, show 0052B9B0 / 0052BFA0 / 0052C990 / 0052CAC0, ensure entry
0052C850, entry destruct 0052C0C0 / 0052CDC0, register 0052CBF0, destruct 0052CDF0, remove entry
0052CF00, collect 0052D1D0.

Session senders (message 23h, `bsp::NativeSessionMessage35` layout, sent through 00772330 with
ECX = *00E188A8 + 1EF0h): unit HP display 0052B400 (unit ids from each unit's +174h) and hide
0052B570; score display 0052B620, score pair 0052B780, hide 0052B900. These carry Lua
`DisplayUnitHP` / `DisplayScores` / `HideScoreDisplay` presentation to the other players.

Edges and constants from the image: 0.76 / 0.877 (00CED048 / 00CED050), 0.936 / 1.052
(00CED038 / 00CED040), 0.71 / 0.89 (00CED058 / 00CED05C), 0.355 / 0.2 (00CED030 / 00CE54A0), the
-900 wrap with +1000 (00CED028 / 00CE47A0); the wide-screen choice is *(*0109CF04 + 0Dh).

### Credits

Listeners 0052D200 / 0052D210 / 0052D6B0 / 0052E5F0, vswprintf wrapper 0052D1E0, string helpers
0052DFB0 / 0052E6D0 / 0052E0F0, item copy / assign / placement / copy-backward 0052E2C0 / 0052E340 /
0052E3B0 / 0052E410, item vector insert 0052F9E0, construct 0052FCE0 (scroll speeds 0.0444f, 0.5f,
0.01389f), collect 0052FD70, destruct 0052FD80.

### Menu prompt (55h)

Toggle listener 005309F0 (in session mode 2 the widget's bound record +D8h takes the flag), the
00E18E6C singleton destructor bodies 00530CC0 / 00530E00, register 005311C0 (_DialogueBox,
dialog_BG_FrameBox, _DialogueFarBlackPlane), record copy 00531C90 (48h bytes), record list clear
00531F80, collect 00532350 / 005329C0, button activate 00532D70 (results 1, 0, 2, 3).

### HUD formation (2Ah)

Action readers 005337C0 / 005353A0, widget sets 00533830 / 00533920 / 00533A30, listener tail call
00533B20, vec2 compare 00533D30, formation message destructor 005347C0, refresh 00534BC0 (Exit then
Enter when the formation UI flag and the group are set), input-context restore 00534C00, action
release tests 00534E90 / 00535EE0, member selection 00536100 (cursor = (-x / w + 0.5,
z / h + 0.5)), collect 00538D20, member marker 00538D60, construct 00538E30.

Correction: the menu_screens_2 note on 00538D60 calls the visibility call "decompile only"; the
listing makes it (00B6DA70 on marker +128h with 0.0f, 0).

### Frame statistics (57h)

Record zero 0053B9E0, construct 0053BA10, enter 0053BA70 (min/max seeded with +/-FLT_MAX),
printf 0053BAB0, accumulator reset 0053BB40, destruct 0053C440, collect 0053D220, and the per-frame
accumulate 0053C510 (ECX = *(game + 19C8h)): mode +8h 1 averages four profiler counters
(ids in 0109D014, 0109DB08, 0109DB14, 0109DB34) over 0.25 s windows, mode 2 counts frames over
2.0 s windows, mode 3 reports 1 / elapsed each frame; the rate updates the min (+7Ch) and max
(+80h) and goes to 0053BCC0 (host). The profiler record 00E18E80 is ended first and reset and
restarted last (tail jump to 00BE3260).

### Free camera (2Bh)

Observer release 0053D2C0, matrix set 0053D700 (00E192B0, dirty byte 00E192A9), effect entry
construct / destruct / copy 0053DC70 / 0053DCE0 / 0053E030, effect record destroy / copy / assign /
copy-backward 0053DF40 / 0053E300 / 0053E460 / 0053E6F0, effect list clear 0053E850, record vector
insert 0053FF10, test effects 0053E0E0 (5InchExplosionArmour +28h, 5InchBulletTracer +24h, then the
settings 1000.0, 0, 0.1, 4.0, 300.0, 30.0), effect lists 005401F0 / 00540260, destruct 005402D0,
collect 00540380, construct 00540BA0. Reference-counted handles are `{vtable, count +4}` released
through slot 0 at zero.

### Debug text lists, command bar, small STL instances

Row refresh 00542620 (the ledger name says DeleteRows; the body sets each row's text and colour:
+48h for an inactive row, +38h for the selected one, else +28h), up/down pollers 00542D70 /
00542EE0 / 005439B0 (one model, three pairs of tail-jump targets), command bar button 00543410
(ButtonLamp_Icon looked up and unused; text {32, 32, 32, 1} disabled, {255, 255, 230, 1} enabled;
focus when the index is the selection +14h). Element helpers: IntBool copy / fill 00542200 /
00542230 / 00542CB0 / 00542CE0, POD8 fill / copy-backward 00542BB0 / 00542BE0 / 00543050 / 005430A0,
IntBool vector insert 00543E90 / 005440E0, ByteSbo (24h, std::string at +4) helpers 00543A60 /
00543C20 / 00543CA0 / 00543CF0 / 00543D40 / 005443B0 / 005443F0. The catch-and-rethrow paths of
the uninitialised copies are not modelled.

### Segment 22: movie camera debug screen (36h)

Vehicle camera array accessor 005CD820, destruct 005CE4D0 (two trees erased through 005CE340 /
005CD710), collect 005CE5D0, construct 005CE610. The movie screens proper (005CD9C0, 005CDB00,
005CDC50, 005CDD70, 005CDF30, 005CDF40) and the movie-camera spectate pair (005CD0F0, 005CD160) are
the in-game movie wiring cc9 owns; they were not touched.

### Multiplayer chat (15h)

Wide-string append / concat 005CF090 / 005CF570, line destroy / copy 005CF0F0 / 005CF180 /
005CF460 / 005CF4F0, node buy 005CFA40 / 005CFAE0, register 005CF260 (_Chat +3Ch), system chat
lines 005CF320 (session mode 1 only; 00772170 with an empty sender and ids -1) and 005CF3D0 (loss:
side 0 -> channel 9, side 1 -> 10, kind 5), clear logs 005CFB80, construct 005CFDA0, destroy
005CFE60, exit 005CFF40, collect 005D0020, label 005D07E0 (prefix = player name + ": "), send
005D0B10 (channel 8, or 9/10 by the local slot record in a mission or team chat; an invalid slot
only raises sound request +1Ch), team flag 005D1000, mode 005D11F0.

Correction: 005D1020 (input clamp to 3Ch characters) and 005D10F0 (submit) are slots of the text
listener 00CF19C0 at screen+8h, not of the primary vtable; their offsets are listener-relative
(input string screen+20h, cursor screen+28h).

### Multiplayer error (1Dh)

Exit 005D1F90, focus dim 005D1FB0, dismiss 005D2010 (pops interface 1Bh; unless signed in with a
selected user in sign-in state 2: states 10h (in a mission), pop to 0Fh, states 9 and 4), update
005D2240, activate 005D2310 (widget listener at screen+0Ch), buttons 005D23E0 (listbox listener at
screen+8h), register 005D2440, message 005D24F0 (codes 1..7 and the join detail 00E08898: full,
public full, DLC failure with the marketplace, rank, session off), error code 005D2E70, collect
005D2F60. The marketplace opens through XShowMarketplaceUI for the active user slot.

### Multiplayer pause (19h)

Prompt results 005D3410 (pop the request and end the scene) / 005D3430 (quit flag 00E1AE75), typed
focus dims 005D3440 / 005D35A0 (types 3, 6, 12h), selection store 005D34E0 (00E19550), construct
005D3540, destroy 005D3710, activate 005D3790 (A2h activates the listbox selection, A3h resumes),
register 005D38A0 (GUI_pause_multi), exit 005D3960, collect 005D3990.

### Multiplayer game lobby helpers

Slot swap pair 005D4910, stored options 005D49E0 (00E1955C..00E19590), standard-mode test
005D4A10 (four unconditional mode reads), record flag 005D50E0, local slot record / validity /
ready 005D5130 / 005D51F0 / 005D5220, start test 005D5260, settings-open test 005D5490, slot for
menu row 005D5630, mode selector 005D5660, kick / slot-swap prompt results 005D5690 / 005D57F0,
local ready 005D5790, editable 005D5870, setting row count 005D5930, apply settings 005D62B0
(game+2020h and three broadcasts), auto-test setting 005D6350, selected player slot 005D63A0,
content masks 005D6BA0, load prompt dismiss 005D6CD0, local map availability 005D8B30, leave /
close prompt results 005D98C0 / 005D9FE0, local slot change 005D9D60, close game 005D9EF0, selected
slot request 005D9F50, local map apply 005D9F90, two-vector record 005DAC60, map entry copy 005DAF10,
leave to menu 005D8410 (the PUSH EDI before 00688C70 is the delayed callee-save of EDI, not an
argument), row objects delete 005D8A70, map entry destruct 005DACA0.

## Handoff (2026-10-12)

State: 268 records (35 trivial rows, 223 reconstructed routines, 10 backfills) out of the 334
candidates; 10 more are listed for cc9 below. 56 remain, all unreconstructed and without records.
Everything here is build-tested only (`scripts/build.ps1` passes); nothing is ABI-compatible or
game-validated.

Tools in the worktree's `local/` (not committed): `fe4_list.txt` (the 334 candidates),
`fe4_dump.py` / `fe4_asm.py` (live decompiles and disasm-raw dumps into `local/fe4_dec`,
`local/fe4_asm`; `fe4_asm.py calls <addr>` names the callees), `fe4_view.py` (compact decompile
view), `fe4_triv.py` (trivial-slot rows), `fe4_pe.py` (floats, doubles, strings from the exe),
`fe4_records.py` (all records, lease-checked; `--list` prints the covered addresses),
`fe4_remaining.py` (what is left), `fe4_calls_report.py` (the call-site report).

Remaining, by why they were left:

- x87 layout bodies (need disasm-raw throughout): HUD counter layouts 00529D70, 0052A100, 0052A470,
  0052A860, 0052BB10, relayout 00529A30, tick 0052B080; formation target drawing 00535430, quad
  model 00536310; error buttons 005D20A0; outline 005D63E0.
- Large bodies: control layout enter 00528240 (decompile stack confusion), formation order labels
  00536A90 and register 00538F80, frame stats lines 0053BCC0, free camera effect update 0053EE00 /
  0053F2D0 / bind 00540D50 / register 00540FE0, chat row format / rebuild / layout / update
  005D09A0 / 005D0C60 / 005D14D0 / 005D18A0, error enter 005D26D0, pause rows / enter / update /
  confirm 005D39A0 / 005D3B40 / 005D3F20 / 005D43C0, lobby player rows 005D73A0, lobby bind
  005DB050 (over 2000 decompile lines), map selection 005D92B0, lobby activate / focus / map
  preview / compatibility 005D5D50 / 005D6060 / 005D6E30 / 005D71B0.
- Counter entry constructors 0052C1B0 / 0052C640 and apply-record 0052CF60, formation destruct
  00538A80, UnitOrderIcon 00534870 (a per-class constant table; attack_commands.cpp mentions it).
- Smaller lobby routines needing an ECX pass: 005D5340 (slot AI reset; the decompile loses the loop
  counter), 005D67E0, 005D6C10, 005D8530, 005D8800, 005D8950, 005D98E0, 005D9A10, 005D9C50 (returns
  `1 << atol(suffix)` as a 64-bit mask, check), 005D9D90, 005DA000, 005DA210, 005DA530.
- 0053BC00 (fingerprint payload singleton): game_hosts_frontend.cpp logs it as a host step only.
- 005DECA0 refreshes the lobby map and resets the mission statistics (BSP_MissionStats_ResetForMission)
  and the loading-screen text; check with cc9 before reconstructing.

Next step: take the smaller lobby routines with one disasm-raw pass each, then the HUD counter
layouts (they share one entry layout, documented above).

## Verification

`reports/cc13_fe4_calls.json` (built by `local/fe4_calls_report.py` from the disasm-raw dumps)
lists the 712 direct call and tail-jump sites of the reconstructed bodies;
`python tools/verify_report_calls.py reports/cc13_fe4_calls.json` checks every one against the live
listing: 0 failed. Three sites were left out because they lie in unlisted code right after
005299D0 and 0052E3B0 (00529A22, 0052E401, 0052E408; the second is a string release at 0052E3F0
with no Ghidra function).

Corrections:

- 005D5260 is not a counter (ledger name CountUnreadyPlayers): it returns false as soon as a
  present human slot (+750h set, +751h clear) is not ready (+754h); with fewer than two humans and
  game+218Ch clear it returns false; mode 7 then returns true, any other mode needs participants on
  both sides (004BB770).
- 005D5870's decompile loses its flag argument; the listing shows the editable flag (host only)
  driving +18h / +228h and the reset block.
- 005D8B30 passes one argument to 0050FD10 (the decompile shows two).

## Backfilled

- 00530970 menu prompt exit (`src/menu_screens_2.cpp`), checked line by line.
- Light checks (ABI and callee sequence against disasm-raw, not a re-derivation):
  `src/menu_screens_2.cpp` credits 0052D9C0 / 0052FFC0 / 0052EE30, HUD formation 00536800 /
  0053A4D0, frame stats 0053C740, free camera 0053DD90 / 00540390; `src/mission_lobby_settings.cpp`
  005D5500 (`lobby_settings_menu_row`, which takes the effective mode as a parameter).

## For cc9

- 0053D230 `BSP_HudFreeCameraScreen_ForwardToUnit`, RET 0Ch, args vec3 by value: when the
  controlled unit (*00E188D8) answers its vslot +5Ch(6) true, the vector's second component is
  zeroed; then the unit's vslot +118h gets &vec. The free-camera update calls it on the mark action
  with the camera position (src/menu_screens_2.cpp `mark_position_0053d230`), so it hands a world
  point to the unit: an order path, not reconstructed here.
- 005D36B0 `BSP_PromptResult_Surrender`, __fastcall(ECX result): on 1,
  BSP_Session_RequestPlayerSurrender(*(game + 18C8h), 1) then BSP_Game_TogglePauseMenu. A match
  outcome request, left to cc9.
- The in-game movie screens and movie-camera spectate pair listed under the movie camera debug
  screen (ownership: in-game movie wiring).

## Update (cc13-fe10, 2026-10-13)

cc13-fe10 (`docs/CC13_FE_10.md`) took this list further: 005D5D50, 005D6E30, 005D9660 (renamed
StepSelectedSlotAi), 005D92B0, the pause rows / enter / update 005D39A0 / 005D3B40 / 005D3F20, the
chat row format / rebuild / layout 005D09A0 / 005D0C60 / 005D14D0, the relayout 00529A30, the
record apply 0052CF60, the formation destruct 00538A80, 0053BC00, and the free-camera bind /
register 00540D50 / 00540FE0 are reconstructed. Its Handoff section lists what is still open.
