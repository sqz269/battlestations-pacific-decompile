# Mission scene load (`004DFB70`, requests 0Ah and 0Bh)

Addresses: 004dfb70 004e5540 004dc6a0 004de610 004d4df0 004d30f0 004cd7f0 004c6890 004c9680 004c9ca0 004c3840 004f2800 004bb160 004bb440 0046df00 004da6c0 008860b0 004db920

Packet `mission_scene_load`, worktree `agent/scene-load`. Ghidra was read-only for this packet;
the ledger names below are hypotheses, not recovered symbols.

## What the path is

`docs/MISSION_TREE_BRIEFING_SCREENS.md` establishes that starting a mission is
`BSP_Game_RequestState(6)` then `BSP_Game_RequestState(0Ah)` from `00439020`. The drain
`BSP_Game_DrainStateRequestQueue` (`004E4430`) writes each dequeued request into `game+5D4h`
and dispatches; per `docs/GAME_FRAME_CONTROL.md` requests 0Ah and 0Bh both go to `004DFB70`.
Because the drain re-reads the count, both land in one pass: interface first, then the load.

`004DFB70` runs the **entire** load synchronously and returns with `game+5D4h = 0Ch`. Nothing is
posted to the frame loop's loading queue (`004FDE20` / `00509190` of `docs/APP_RUN_FRAME.md`);
`BSP_LoadingScreen_Begin` starts the render worker that animates the screen while this call blocks
(`docs/GAME_FRONTEND_ENTRY.md`). The transition into the simulation state is **not** made here:
state 0Ch is serviced next frame by `BSP_Game_UpdateDeviceWaitScreen` (`004DB920`), whose both
arms end in `004DA6C0`, and that is where `game+5D4h = 0Dh` is written.

## Calling convention and RET size

`004DFB70` is `__thiscall void (GGame* this)`. `004DFB94` moves `ECX` into `EDI` and the body uses
`EDI` as the game object throughout; there are no stack arguments and the epilogue at `004E18D3`
is `ADD EBP,74h; MOV ESP,EBP; POP EBP; RET` — **RET 0**. Ghidra renders it `__fastcall(int)`,
which is the same ABI for one register argument. The function has an SEH frame (handler
`00C6750C`) and a 74h-byte `EBP`-relative frame; 2239 instructions, 354 blocks, cyclomatic
complexity 183.

Sole caller: `004E4430`. 94 distinct callees.

## How requests 0Ah and 0Bh differ

Exactly one thing: the label of the VFS file block opened around the loading-screen bring-up,
chosen at `004DFDD5`.

| `game+5D4h` | Block label | Literal |
| --- | --- | --- |
| 0Ah | `loading_screen_scene` | 00CE7EF0 |
| 0Bh | `loading_screen_reload` | 00CE7ED8 |
| anything else | `this_will_not_work` | 00CE7EC4 |

The third arm is unreachable through the dispatch table. Nothing else in the body reads
`game+5D4h`; the only other access is the store of 0Ch at `004E086B`.

## What selects the mission

`game+5FCh` is the **loaded scene record**, and it is not set here — `004DFB70` only reads it.
It is written by `004C6890` (`BSP_Game_SelectSceneRecord`), which the mission-tree path reaches
through `BSP_Game_SetPendingScene` (`004E2770`). `004C6890(this = game, index)`:

- clamps `index` into `[0, game+5F0h]` and stores it at `game+60Ch` (the scene index `004DFB70`
  passes to `005D7070` at `004DFCCF`);
- walks the singly linked list at `game+5F4h` `index` steps through `+4h` and takes `+8h` as the
  record, storing it at `game+5FCh`;
- empties the override string at `game+600h/604h`;
- copies the low byte of `[record]+1098h` to `game+2015h`;
- picks the script slot with the same 8-or-9 rule `004DFB70` uses and stores it at `game+2028h`;
- copies `[record]+988h` side blocks of 0120h bytes into the eight slot records at
  `game+1010h + i*118h` — `record+4h` becomes slot `+20h`, `record+8h` becomes slot `+1Ch`.

Scene-record fields `004DFB70` reads (all pooled strings are the native `{int length; char* data;}`
pair, so the length is at the offset and the pointer at offset+4):

| Offset | Use |
| --- | --- |
| `+90Ch/+910h` | the scene path; argument 1 of the scene loader and the input of `004CD7F0` |
| `+928h + slot*8` | the mission script name table, stride 8, indexed by the normalised slot |
| `+980h/+984h` | a comma-separated list of localisation table names |
| `+988h` | side-block count, read by `004C6890` |
| `+1054h` | read by the render tail (`docs/GAME_RENDER_TAIL.md`), not here |
| `+1098h` | the mission id published to `[00F8A2FC]+48h` at `004E183D` |

The **side** comes from the local participant: `[game+18CCh + game+18ECh*4] + 28h`. Zero selects
the suffix `_Ally`, non-zero selects `_Jp` (`004E10E6`, `004E105F`, `004E1150`).

### Path resolution

Two different resolutions run, and neither is a plain concatenation of the scene path.

1. **VFS block names.** `004CD7F0` (`BSP_Scene_DeriveShortName`), `__thiscall(NativeString* out,
   NativeString* path, char)` with `ECX = out` and `EDX = path`, derives a short name: replace `\`
   with `/`, keep the text after the last `/`, truncate at the **second** `_` when there is one,
   then drop a trailing `.scn` compared with `_stricmp`. Both tail steps always execute; the `.scn`
   strip is a no-op once the underscore truncation fired. The short name is prefixed to give the
   five numbered blocks.
2. **The mission script.** `008860B0` builds `"Scripts/missions/" + name + ".lua"` (the extension
   literal is at 00CFD2C8) and hands it to `00885FB0(path, 1)`. Ghidra renders it
   `__cdecl(NativeString*)`; the call site at `004E0A38` loads `ECX` from `game+1A08h` (the script
   host) first, so the convention is provisional — see Uncertainties.

### Which script slot

`004E087B` normalises `game+614h`: slot 9 always becomes 8, and outside network play
(`game+1FE4h == 0`) without the force byte `game+61Ch` every slot other than 8 also becomes 8.
The normalised slot indexes the script-name table. The **raw** `game+614h` is re-read at `004E0A57`
for the arm decision: slot 9 runs only `luaEngineMovieInit`, every other slot runs the precache and
stage-init sequence. The nested compare at `004E0A5F..004E0A75` is the compiler sharing the `== 9`
test between the guarded and unguarded arms; the guards do not change the outcome.

## VFS file blocks

`BSP_FileBlock_Construct` (`00BE0A30`) is `BSP_VFS_EnterFileBlock`, so every name below is a
resource-grouping scope, not a state field (`docs/FILE_BLOCK_SETUP.md`).

| Enter | Name | Leave | Contents |
| --- | --- | --- | --- |
| 004DFE3F | `loading_screen_{scene,reload}` | 004DFE7E | `0057D0C0` then `BSP_LoadingScreen_Begin(1)` |
| 004DFF7A | `1_` + short name (heap) | 004E0370, virtual `+4h` | scene pass 1, world, Lua reset |
| 004E04B9 | `GvSpace_Init` (00CE7E94) | 004E0514 | in-mission HUD manager bring-up |
| 004E099A | `3_` + short name | 004E0A4B | `Scripts/missions/<name>.lua` |
| 004E0B95 | `4_` + short name | 004E0C48 | `luaStageInitMulti`, `luaStageInit` |
| 004E0D5C | `5_` + short name | 004E0DFF | `luaEngineMovieInit` |
| 004E0EB4 | `6_` + short name | 004E0F5B | `[00F8D394]` vtable `+E4h` |
| 004E11C3 | `Warnings_<lang>` + side (heap) | 004E125A | `007065E0(1)` |
| 004E129C | `dummy DO NOT USE` | 004E12D5 | `007065E0(0)` |
| 004E14D3 | `Warnings_englishauthentic` + side (heap) | virtual `+4h` at 004E17FB | `007065E0(1)` |
| 004E16CC | `Warnings_<lang>` + side (heap) | virtual `+4h` at 004E17FB | `007065E0(1)` |

There is no `2_` block; `GvSpace_Init` occupies that position. The prefixes are two-character
literals at 00CE7EC0, 00CE7E90, 00CE4EC8, 00CE7E78 and 00CE7E74.

## Loading screen

`BSP_LoadingScreen_Begin` (`0057CB60`) takes its **mode in ECX**, not a `this` pointer, and
`004DFE6A` is `MOV ECX,1`: mode 1, `LoadingScreenMode::LoadingImage`, which draws the picked
`mp.loading_NN` image and needs the configuration published first. That publication is
`0057D0C0`, a one-caller stub tagged `CG_static_dtor_stub` that reaches
`BSP_LoadingScreen_PublishConfig` (`0057CFF0`). The front-end shell (`004E4000`) passes mode 0
instead. `BSP_LoadingScreen_End` (`0057C250`) runs at `004E185C`, inside this same call, so the
screen never outlives the load.

## Front-end teardown

Before the loading screen goes up, `004DFB70` releases the two front-end managers through their
virtual `+0h` with argument 1 and nulls the globals: `00E198AC` unconditionally (`004DFDA6`) and
`00E198B4` only in single player (`004DFDC6`); in network play `006878F0` runs instead. It also
resets the session object at `game+1EF0h` through `0076DA60`, clears the two intrusive lists at
`00E18A64`/`00E18A70` in network play, and drops the previous mission result at `game+7188h`.

## Order of the load

Native order, with `ECX` taken from the listing:

1. `004DFB9D` `game+1EE7h = 0`.
2. `[00F8BBD8]+6Ch != 0` → clear it and `00A7A440([00F8BBD8]+4Ch)`.
3. `BSP_Game_SetCinematicMode(1,0,1)`, `ECX = game`.
4. `00F874FD = 0`, `00E08178 = 00E0E35C = [00D7A24C]`, `00E0E2FC = 1`, `game+610h = 0`.
5. `0076DA60`, `ECX = game+1EF0h`.
6. Participant tables. Single player: `004BB160(game)` repoints `game+18CCh..18E8h` at the eight
   embedded 118h-byte slot records starting at `game+1008h`; `004BB440(game, …)` claims a free
   record in the **other** eight-entry array at `game+748h` and stores it at `game+18CCh`, copying
   `game+1030h` to `+28h` and `game+102Ch` to `+24h`; `game+18ECh = 0`; `[game+18DCh]+28h` becomes
   `([game+18CCh]+28h == 0)`. Network: clear the two lists, reset the eight headers at `game+758h`
   (stride 118h) to `{0, FFFDh, 0}`, then `005D7070` / `00626930`.
7. Front-end teardown (above).
8. Loading-screen block, `0057D0C0`, `BSP_LoadingScreen_Begin(1)`.
9. Free `game+7188h` (the previous mission result). Ghidra shows a `return` here; the listing at
   `004DFEAA..004DFEB9` falls through — see Uncertainties.
10. `004DC6A0(game)` `BSP_Game_ConstructGlobalSubsystems`: opens the `Game_Global` block, runs
    `00886900`, `00800160`, `00901610`, `006F7B50`, `00803A40`, `006DBEB0`, collects Lua garbage,
    then allocates `game+21D0h` (0x58, `004A43C0`), `game+21E4h` (0x38), `00F88C30` (0x1C4, the
    power-up manager), `game+21E0h` (0x1B0) and more.
11. Block `1_`+name. `00E0AF20 = 0`; a "Scene initialization failed" string is built at `004E014D`
    and released without being read; `00874640(0)`; `006AD600` on `game+21D8h`;
    **`0046DF00([00E18680], scenePath, 0, 0, record, override, 0)`** — the scene-file load;
    `004DE610(game)` `BSP_Game_ConstructWorld`; `00951560` on `00F89A08` and `00F89A5C`; the Lua
    `thisTable` test and `recon` declaration around `005E2F00`; `004F2800`
    `BSP_Scene_ResolveNamedObjects` (`SpawnPoint`, `AirField`, `NavPoint`, `CameraPath`, …);
    `game+193Ch = 0`.
12. `game+1910h..192Ch = -1` (eight view ids); in network play they are copied from
    `game+770h + i*118h + E8h` and each valid one runs `0095BA60`.
13. `004D4DF0(game)` `BSP_Game_LoadSceneContents`: logs `Loading scene %s` with the scene path and
    runs scene-file passes 2 and 3 — `0046DF00(path, 0, 0, record, override, 1)` and
    `0046DF00(path, 0, 1, record, override, 0)`.
14. `007FA2D0` for each non-null `game+18CCh + i*4`.
15. Copy `[record]+980h` into a local (the localisation list); `004C3840(game, 0)` when
    `game+1FE4h == 1`.
16. `operator new(0x108)` then `0068A990` into `00E198C4`, the in-mission HUD manager.
17. Block `GvSpace_Init`: `004C9680`, `BSP_FrontEndFrame_GetOrCreate(3,0)`,
    `BSP_FrontEndFrame_SelectLayoutSet(3,0)`, `00E198C4` virtual `+4h` and `+8h`.
18. `[00F8BBCC]+210h = [game+18CCh + game+18ECh*4]+28h`; three `008053C0`; `00807A50`.
19. With the local slot in `[0,8)`: slot `+19h = 0`, slot `+30h = 008053C0()`, `008073C0`,
    `game+193Ch = 0`, `004C3CB0(game)`, `006485A0`.
20. Non-empty localisation list → split on `,` (`0094EC70` with 00CE4BFC), one
    `BSP_Localization_RegisterTableName` per element, then `BSP_Localization_ReloadTables(0)`.
21. `game+648h = 0`, `game+64Ch = 0.0f`; clear the list at `game+5CCh/5D0h`; `004218E0`, `00424D00`.
22. `[00F8BBF4]+64h = 1`; two `BSP_InputManager_Update(0.0f)`; `[00F8BBF4]+64h = 0`.
23. `game+1FE4h == 2` → build an event through `0075B430(0Ch)` and dispatch it into `game+1EF0h`
    through `00770AF0`. Other network modes → local slot `+0Eh = 1`, `+10h = FFFDh`, `+18h = 0`.
24. `0077F5E0`; `00E0AF20 = 1`; **`game+5D4h = 0Ch`** at `004E086B`.
25. Read the script name at `[record]+928h + slot*8`; `004D30F0(game)` clears the container at
    `game+1934h/1938h` and refills it from a Lua table through `004D0640`.
26. Non-empty script name → block `3_`+name, `008860B0` runs `Scripts/missions/<name>.lua`.
27. Raw slot 9 → if the Lua entry `luaEngineMovieInit` exists, block `5_`+name and `0045F440`.
    Otherwise `[00E188A8]+644h += 1`; `0045F520("luaPrecacheUnits")`; `0095CA70`, `004423A0`,
    `006E9C50`, `0084EB60`, `004B1890`, `004AF930`, `00474200`, `00476410`, `004A7120`, `006D74B0`,
    `00869D20`; Lua garbage; block `4_`+name with `0045F520("luaStageInitMulti")` and
    `0045F440("luaStageInit")`; `[00E188A8]+644h -= 1`.
28. Block `6_`+name: `[00F8D394]` virtual `+E4h`.
29. `game+1FE4h != 2` → Lua garbage.
30. Spoken warnings. When `[004C1E90()]+8h == 2` and the per-side flag
    `[[game+18CCh+local*4]+28h] + 14h + 004C1E90()` is clear, walk the installed-language vector
    (`008D76C0`) **from the back** and, for every entry that does **not** match the current
    language name, select it (`008D56C0`), load `Warnings_<entry><side>` with `007065E0(1)` and
    immediately replace it with the `dummy DO NOT USE` bank through `007065E0(0)`; then set the
    flag and restore the current language. Both arms then fall through to `004E1374`, which loads
    the bank actually in use: `Warnings_englishauthentic<side>` when `DL_Content_0000062` is
    installed (`007F8890`), otherwise `Warnings_<language><side>`.
31. `game+1FE4h == 1` or `game+61Ch` set → scan the eight slots for one with `+8h`, `+9h` and
    `+0Ah` all set and call `00A32350`.
32. `[00F8A2FC]+48h = [record]+1098h`; `00F1B038 = 0`; `BSP_LoadingScreen_End()`.
33. `BSP_GuiManager_GetOrCreate(0)` then `00AA0E20(0)`; `004C9CA0(game, 1)`;
    `BSP_InputManager_Update(0.0f)`; `00874640(0)`; `BSP_Game_SetCinematicMode(1,0,1)`.

## What it writes on the game object

| Field | Value | Site |
| --- | --- | --- |
| `+1EE7h` | 0 | 004DFB9D, first instruction |
| `+610h` | 0 | 004DFC08 |
| `+18CCh..18E8h` | the eight slot records | 004BB160 / 004BB440 |
| `+18ECh` | 0 (single player) | 004DFD77 |
| `+193Ch` | 0 | 004E0360 and 004E05A2 |
| `+1910h..192Ch` | -1, then the network view ids | 004E0378 |
| `+648h` | 0 | 004E075A |
| `+64Ch` | 0.0f | 004E0764 |
| `+5CCh/5D0h` | emptied list | 004E0770 |
| `+5D4h` | 0Ch | 004E086B |

`game+634h`, `+635h` and `+7184h`, the gates `docs/GAME_SIMULATION_GATE.md` and
`docs/GAME_WORLD_ENTITIES.md` read first in state 0Dh, are **not** written here. Neither are the
one-shots `+1EE1h..+1EE5h`. Those are armed by `004DA6C0` on the next frame.

## The world object and the subsystem pointers

`docs/GAME_WORLD_ENTITIES.md` lists four objects the world tick needs. Three of them are created
under `004DFB70`:

| Field | Created by | Site |
| --- | --- | --- |
| `game+21A0h` | `BSP_Game_OnInit` at startup, **not** here | 004E3E74 |
| `game+21D0h` | `004DC6A0` (step 10) | 004DC80F |
| `game+21D4h` | `004DE610` (step 11) | 004DF9DE |
| `game+19CCh` | `004DE610` (step 11) | 004DE686 |

`004DE610` also builds the ocean and sky (`World`, `Operator`, `sky_001`,
`Ocean initialization failed`, the `ShoreWaves` sources) and is the only caller-visible producer of
the world object.

## Completion, and how state 0Dh is reached

Nothing signals completion asynchronously. `BSP_LoadingScreen_End` at `004E185C` is inside
`004DFB70`, so the screen ends when the last step finishes. The state left behind is 0Ch.

A byte scan for `C7 ?? D4 05 00 00 0D 00 00 00` finds exactly two writers of state 0Dh:
`004E47FF` (inside the drain, the request 12h arm) and `004DA6C0`. `004DA6C0` is
`__fastcall(GGame*)`, carries the scope literal `GGame::SceneInit()`, and is reached from
`BSP_Game_UpdateDeviceWaitScreen` (`004DB920`, the state 0Ch handler), `00427190` and `00777850`.
It:

- `game+21F0h = 0` (the frame delta) and `00447060`;
- clears the one-shots `game+1EE1h`, `+1EE2h`, `+1EE4h`, `+1EE5h` and sets `game+1EE3h` to
  `game+1FE4h != 0`;
- in single player sets `[game+18CCh + local*4]+10h = 1`;
- `00A7A440(00F889A0)`;
- **`game+5D4h = 0Dh`**;
- `004C9CA0(0)` — the same interface switch `004DFB70` calls with 1;
- in network play `004D87B0`, and returns early if that changed the state;
- `BSP_Game_SetCinematicMode(0,0,1)` and `game+608h = 0`.

So the arming of the in-mission one-shots belongs to `004DA6C0`, not to `004DFB70`.

## `004E5540` compared

`docs/APP_INIT_GAME_ENTRY.md` establishes that `BSP_Game_BeginStartupSequence`'s `.scn` branch
runs `004E3AA0` (`GGame::OnInit`) and `004C88A0`, writes `game+5D4h = 10` and pushes request 10
(0Ah) with `004D3ED0(game+5D8h, &10)`. Decimal 10 is 0Ah, so **the startup `.scn` path does not
start a scene directly**: it drains the queue, then queues the same request `004DFB70` handles,
and the first drain of the frame loop dispatches it here. The shared callee is `004DFB70` itself.

Differences that matter:

- The startup path runs `004DD5B0` (`OnInitOnce`), `004C9A70` (`OnInitTitle`) and `004E3AA0`
  (`OnInit`) before the request, so `game+21A0h` and the resident content exist; the mission-tree
  path has already done that at boot.
- The startup path drains before pushing, so 0Ah is the only entry in the queue; the mission-tree
  path pushes 6 then 0Ah, so the interface change is serviced first in the same pass.
- Both leave `game+5D4h` and the queued request in agreement.

## State reached per routine

| Address | Name | State |
| --- | --- | --- |
| 004DFB70 | `BSP_Game_LoadMissionScene` | reconstructed, build-tested |
| 004CD7F0 | `BSP_Scene_DeriveShortName` | reconstructed, build-tested, fixture-tested |
| 008860B0 | `BSP_MissionScript_RunFile` | reconstructed (path only), build-tested |
| 004C6890 | `BSP_Game_SelectSceneRecord` | analysed |
| 004DA6C0 | `BSP_Game_EnterMissionState` | analysed |
| 004DC6A0 | `BSP_Game_ConstructGlobalSubsystems` | analysed |
| 004DE610 | `BSP_Game_ConstructWorld` | analysed (entry and outputs only) |
| 004D4DF0 | `BSP_Game_LoadSceneContents` | analysed |
| 004D30F0 | `BSP_Game_RefillScriptedNameList` | analysed, provisional |
| 004BB160 | `BSP_Game_ResetParticipantTable` | analysed |
| 004BB440 | `BSP_Game_ClaimParticipantRecord` | analysed |
| 004F2800 | `BSP_Scene_ResolveNamedObjects` | analysed |
| 004C3840 | `BSP_Game_AssignPartyPlayerSlots` | analysed |
| 004C9CA0 | `BSP_Game_ApplyInGameInterface` | analysed |
| 0046DF00 | `BSP_SceneDatabase_LoadSceneFile` | exported, entry conditions only |
| 004C9680 | not named | exported |
| 004E5540 | `BSP_Game_BeginStartupSequence` (existing) | read only, from the doc |

## Uncertainties

- **`participant+28h` has two conflicting readings.** `004DFD7C` writes the boolean
  `([game+18CCh]+28h == 0)` into `[game+18DCh]+28h`, so it holds 0 or 1 there. `004E0FCB` loads it
  into `ESI` and indexes `byte [ESI + 004C1E90() + 14h]`, which only makes sense for a pointer.
  Both readings cannot be right. The `_Ally` / `_Jp` selection is a plain `!= 0` test either way,
  so the reconstruction models it as an opaque `side_selector` and does not commit.
- **`008860B0`'s convention.** Ghidra renders `__cdecl(NativeString*)`, but `004E0A38` loads `ECX`
  from `game+1A08h` before the call. Either the register is dead or Ghidra missed a `this`. The
  header records `run_mission_script(path)` and the doc records the load of `ECX`.
- **Ghidra emits spurious `return` statements** throughout `004DFB70`'s decompilation because
  `_free` (`00BF65AC`, `00BF6989`) is annotated no-return. Every one checked resolves to a
  fall-through in the listing: `004DFEAA→004DFEB9`, `004E134E→004E1374` (the intervening
  `004E1353..004E1373` is the string release Ghidra dropped as unreachable). The listing, not the
  pseudocode, was used for control flow. **The no-return annotation on the free helpers should be
  cleared** — it is corrupting every SEH-heavy function in this area.
- The "Scene initialization failed" string at 00CE7EA4 is constructed at `004E014D` and released
  with no reader. Either an assertion whose body compiled away or a folded argument.
- `004D30F0`'s container at `game+1934h` was not identified; only the clear-and-refill shape and
  the per-element callee `004D0640` are established. The name is provisional.
- `004C9680` and the three `008053C0` calls at `004E0539` were not read.
- `0046DF00` (0x1062 bytes, 49 callees, `SceneBrowserGroups` / `LandVehicle` / `CommandBuilding` /
  `g_Terrain.*`) was identified as the scene-file parser from its literals and call sites only. Its
  three-pass argument scheme (`param_4` and `param_7` toggling across the three calls) is recorded
  but not explained.

## What remains, and follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `scene_file_parser` | 0046df00 0046a9f0 0046aab0 00467e10 | docs/SCENE_FILE_PARSER.md, include/bsp/scene_file.hpp | The `.scn` reader: its three passes, the record it fills, the entity and terrain keys it recognises |
| `game_world_construct` | 004de610 004cb030 006deca0 | docs/GAME_WORLD_CONSTRUCT.md | `game+19CCh` and `game+21D4h`: sizes, constructors, the ocean and sky bring-up, `Ocean initialization failed` |
| `mission_script_host` | 008860b0 00885fb0 0045f440 0045f520 00887b30 00887e50 | docs/MISSION_SCRIPT_HOST.md | The Lua host at `game+1A08h`: file load, named entry-point dispatch, the `game+644h` re-entry guard |
| `game_scene_records` | 004c6890 004e1d70 004e2770 | docs/GAME_SCENE_RECORDS.md | The scene list at `game+5F0h/5F4h`, how `004E1D70` builds a record, and the 0120h side blocks |
| `mission_device_wait` | 004db920 004da6c0 00427190 00777850 | docs/MISSION_DEVICE_WAIT.md | State 0Ch and the 0Ch→0Dh transition, including the one-shots `004DA6C0` arms |
| `warning_bank_loader` | 007065e0 008d76c0 008d56c0 007f8890 | docs/WARNING_BANK_LOADER.md | The spoken-warning bank eviction pass and the `DL_Content_0000062` switch |

## Reconstruction

`include/bsp/mission_scene_load.hpp` and `src/mission_scene_load.cpp`. The request-to-label
decision, the short-name derivation, the block names, the script path, the slot normalisation, the
side suffix, the bank names and the locale-list split are pure functions with explicit inputs.
`run_mission_scene_load(MissionSceneLoadState&, MissionSceneLoadHost&)` walks the sequence above
over an injected host with one method per native call site, in the style of
`bsp::run_application_frame`. `LoadingScreenMode` is reused from `bsp/frontend_entry.hpp`; nothing
is redefined. This is a semantic reconstruction, not an ABI-compatible replacement.
