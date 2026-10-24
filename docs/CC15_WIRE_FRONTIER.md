# cc15-wf1: single-player wiring frontier (2026-10-19)

## Object bring-up ranking (cc15-obr, 2026-10-19)

This section is a read-only plan. No host code was written for it. The user has approved building
these objects. Out of scope: Xenon/Xbox Live (00A3E3B0, 0067C8F0, 00A3F3D0), multiplayer-only
objects (the 006887E0 multiplayer menu, 00780670 session queue), the renderer, and render milestone D
onward.

**Sources.**
- `tools/wire_readiness.py --frontier` on a USN04 log, `cc9-physics1/local/p24r_on_u04.log`
  (2026-10-19 01:28). It reports 191 reached unwired stand-ins, close to wf1's 186.
- The map is `build/win32/bsp_game.map` from the main checkout (10-16), so the identifiers may be slightly stale.
- Each object's listing was checked. The allocation sizes and the slot stores below were re-read from the
  bytes: 004DFA1A, 004DC87D..004DC913, 004E1CFA, 007FDC11.

**How rows are counted.**
- "Rows" are frontier rows (USN04 calls > 0) that the object unblocks, including the cc9 hand-over rows.
- "Unlock" is the frontier tool's count of callees-done bodies that come in when the row is wired.
- A row that needs two objects is counted under the one built later in the recommended order, and is
  marked (+X).
- The five voice-manager rows are not in the frontier list. They are counted from
  `docs/CC15_WARNING_APPLY_GAP.md` and shown separately.

| Rank | Object | Rows | Unlock | Constructor (size), slot | Host file (owner) | Existing bodies | Depends on | Size |
|---|---|---|---|---|---|---|---|---|
| 1 | In-mission interface manager and its screens (D+E) | 17 (+5 voice) | 13+6+3 | 0068A990 (108h) at [00E198C4]; Init 0068CC70 builds 42 screens | hud, hud_world (cc9) | `in_mission_interface_runtime.cpp` over the projected `InGameInterfaceManager`; per-screen bodies typed or raw | A, B, 004DC6A0 power-up manager, hint tracker, units, input contexts | L |
| 2 | Native GUI manager and widget tree (A) | 9 | 2 (008CA010) | 004C12B0 get-or-create, then 00AA5D70 (88h) at [00F8BC5C] | frontend (cc15); 00AA7DC0 and 0043ABA0 are cc9 rows | `gui_widget_owner.cpp`, `native_gui_page_*.cpp`, `native_gui_widget_position.cpp`, `gui_manager_cc13.cpp` (17), `native_platform_focus_owners.cpp` | Lua page scripts, fonts (00AD4C30), a per-widget scene node (renderer; needs a stand-in) | L |
| 3 | Front-end managers, MainMenuScreen and MissionTree (C) | 8 | 1 (00592640) | 00686170 (78h) at [00E198AC], 00689800 (4Ch) at [00E198B8], 00689D90 (4Ch) at [00E198BC], 0068D760 (44h) at [00E198C8]; 005902E0 MainMenuScreen (578h) at +58h; 005CA880 MissionTree (34h) at +5Ch | menu (mixed: cc13 helm and cc15), mission (cc15) | construct bodies exist; the host runs `MainMenuManager` / `FrontEndWorld` projections | A, B, MissionProgress (detail rows) | L |
| 4 | Global subsystems 004DC6A0 | 1 | 20 (+10 cc13-powerup's) | five owners: traffic 004A43C0 (58h) at +21D0, panel 00452660 (38h) at +21E4, power-up 008EDC60 (1C4h) at [00F88C30] (**cc13-powerup's**), warnings 0098A020 (1B0h) at +21E0, weather 00445B10 (7Ch) at +21C4 | mission_frame (cc15) call site. The power-up owner, its row 008E9AF0, AddPowerup and the power-up reads belong to cc13-powerup | all five constructor/loader pairs are typed and fixture-tested; `GlobalSubsystemHost` (6 virtuals) and `GlobalSubsystemContext` (23 sub-contexts) have no host implementation yet | VFS FileBlock, both Lua owners (+1A08, +1A0C), the 00432650 config singleton, the 00871BA0 effect chain | L |
| 5 | Screen registry and base screen (B) | 4 | 0 | 004F7180 base (vtable 00CEAE54); registry [00E18B60+id*4] | menu (mixed) | 004F75A0..D0 are lone `RET` bodies | A (screens are built from pages) | S |
| 6 | SceneDatabase class table (related; it is not one of the global subsystems) | 2 | 28 | 0046F350 (170h) at [00E18680], built in OnInit 004E3AA0 | scene_contents (cc15) | 004F2800 and 004EE250 are reconstructed; the host already uses the lookup (scene_contents:711) | 006AF3D0 (needed by 004E99B0) | M-L |
| 7 | Hint ("award") tracker | 2 | 0 | 004E18E0 (A0h), lazy singleton [00E198D0] through 004E1CA0 | object in game_hosts.cpp (shared); call sites in mission (cc15) | all methods reconstructed (`seg10_cc13.cpp`, `frontend_seg16_24_28_cc13.cpp`, `gui_screens_3.cpp`, `cc13_sweepA2.cpp`) | profile+0Ch shown-hint list (00690CD0), 006921B0 hint tables (optional) | S (M with definitions) |
| 8 | MissionProgress / profile owner (game+650h; +6B4h = profile+64h) | 2 (+MM) | 0 | 00920E10 (24h), allocated by 007FDB20 ResetToDefaults at OnInitTitle 004C9AFA | menu (mixed), mission (cc15), mission_result (cc13 history) | `mission_progress.cpp`, `native_mission_progress_owner.cpp`, `profile_reset.cpp`, `profile_archive.cpp`, `storage_backend.cpp` | storage backend [0109CECC] and save name (real load only) | S empty / L loaded |
| 9 | MessageManager (game+21DCh) | 1 (+4 seams) | 0 | 00707480 (70h), built in ConstructWorld 004DE610 at 004DFA39 | mission_frame (cc15); seams in script_orders (cc9) | every method typed in `engine_seg11_35_66_cc13.cpp` except 007030D0 and 00707520 | Lua reader, FileStore probes, sample cache 00A83FD0 on [00F8BBE8], language settings [00F88980] | M |
| 10 | Native loading queue | 1 | 3 | 004FDBA0 (20h), lazy singleton [00E18D4C] through 004FDE20 | game_hosts.cpp (shared) | `native_loading_queue_owner.cpp`; 00509190 Update is build-tested | FileStore jobs | S-M |
| 11 | FrontEndFrame (related) | 1 | 0 | 00517D10 (164h) at [00E18D80] through 004C1AC0 | menu, mission_frame; hud copies are cc9's | the row has no identifier (`unknown_no_identifier`) | A, B | M |
| 12 | Native application object | 1 | 0 | 00737970 (1Ch), on WinMain's stack | game_hosts.cpp (shared) | `native_render_service_base.cpp`; the host restates +18..+1A | none | S |
| 13 | Native VFS manager pump | 1 | 0 | 00BDB0B0 over the provider tree | game_hosts.cpp (shared) | `native_vfs_pending_routes.cpp`; the host pumps its own VFS | the native VFS layout | M (fidelity only) |

Totals: 50 frontier rows, 5 voice rows and 76 unlocks. These leave out cc13-powerup's power-up manager:
008E9AF0 and 10 unlocks. Row 1's 00612DA0 still needs that manager, so it waits on cc13-powerup. The profiler 004C1DD0 (30h at [0109DB50],
unlock 3) also counts as an object, but it is the host's own `profiler_` and its row has no
identifier, so it is not ranked.

Handed to cc15 by cc13-frontier (2026-10-19, docs/CC13_FRONTIER_MERGE.md). The only blocker of these nine
presentation rows is the in-mission interface manager [00E198C4] and its screens (D+E): 00673130,
00A7B710, 00644DB0, 005BD420, 005BD590, 00646040, 0068AB80, 005251C0, 005B5D40. cc13 won't build them.
They are parked with D+E until the user decides on the render side.

Still blocked after every object above is built:
- 00683820 GuiScroller needs the input manager (instance+4h → +36B4h/+3714h), and there is no input backend.
- 00780670 is the network session queue.
- 00AB98F0 is already hosted.

### Recommended lanes

- **Parallel from the start.** These have disjoint files and no dependency on each other:
  - (L1) mission_frame: MessageManager, then 004DC6A0. The two are serial in one lane: same file, and
    the warning owner at +21E0 shares the warning bank path with `kWarningMessageBankCc15Bound`.
  - (L2) mission + menu + mission_result: the empty profile owner with MissionProgress, plus the hint tracker
    object. 00690CD0 reads the same profile.
  - (L3) game_hosts.cpp: loading queue, application, VFS pump. These are low value; fill a slot with them.
  - (L4) scene_contents: the SceneDatabase class table.
  - (L5) frontend: GUI A.
- **Serial.**
  - GUI B and C come after A. C also comes after L2, because it shares game_hosts_menu.cpp and
    game_hosts_mission.cpp and the MissionDetail rows read MissionProgress.
  - FrontEndFrame comes after B.
  - In-mission D+E (cc9) comes after A, B, 004DC6A0 (the power-up manager for 00612DA0 and 008E9AF0)
    and the hint tracker (005FB080).
  - The voice manager comes after D+E and the MessageManager.
  - Loading real profiles (MissionProgress L) comes after the storage backend. It is not needed for a fresh profile.

### Per-object bring-up briefs

**MessageManager (game+21DCh).**
- *Lifetime.* 004DE610 ConstructWorld allocates it: `PUSH 70h` at 004DFA1A, ctor at 004DFA39, store at
  004DFA4B. It is a world field, rebuilt every mission load, and it is not registered anywhere.
  004D2BB0 DestroyWorld calls vtable slot 0 (00707520, body 007071F0) with 1 at 004D2D04, after
  +21CC and before +21E0.
- *Messages.lua.* Loaded by 007065E0, three times inside 004DFB70 (004E124E, 004E12C9, 004E17E9),
  each in a per-language voice FileBlock.
- *Readers of +21DC.* 13 functions: 004483F0, 004D2BB0, 004DE610, 004DFB70, 00703380, 00706060, 00777850,
  008C61C0, 008C6490, 0096E5E0, 00976B00, 00976DB0, 0097B5E0.
- *Order.* Host it on the game/world host so that script_orders (008C61C0 LoadMessageMap) can reach it.
- *Glue (about 30 overrides).* Implement `MessageManagerCalls` (include/bsp/engine_seg11_35_66_cc13.hpp:138)
  and bridge `records_04` to `MessageRecordStore`. Then point four seams at the object:
  - engine_seg49_64_78's `resolve_message_00705e00`
  - warning_string's `message_record_resolve_named_00705e00`
  - `MessageFromHost::resolve_message_00705e00`
  - Large62Host `manager`
- *Agree with cc9.* cc9 must agree on the duplicate parser `dialog_message_timing`
  (game_hosts_script_orders.cpp:7009-7050).
- *Check first.* 007065E0's argument: the header says side, the loader passes a load mode. Check the assembly.
- *Wire after.* 007065E0, then 008D57A0 together with L2.
- *Reach.* USN04 calls 007065E0 twice. Warning activations are proven in USN01 and JM08, but not in USN04.
- *An ON pair should move.* `summary mission warning activate`: `records_not_evaluated` and
  `sound_keys_not_evaluated` go to 0, and `clips` is unchanged but has non-null records. Gameplay stays
  identical, because the apply stays gated on the voice manager.

**Global subsystems 004DC6A0.**
- *Lifetime.* 004DFB70 calls it (004DFEBB) on every mission load (state requests 0Ah/0Bh). The host
  guard "once per process" at mission_frame:1092 is not native. In order, it runs:
  - the `Game_Global` FileBlock
  - 00886900, 00800160, 00901610, 006F7B50, 00803A40 and 006DBEB0
  - Lua `collectgarbage`
  - the effect-name loop
  - the five owners in the table
- *Teardown.* DestroyWorld 004D2BB0 destroys them in this order: +21D0, +21C4, +21E0, +21E4, then
  [00F88C30] at 004D2D84, before the world at +19CC.
- *Stage it.*
  - First switch: traffic, weather, warnings, panel.
  - Power-up and robot each get their own switch, agreed with cc9. Power-up changes the substitutes at
    gunnery:7417/12815/13091, units:6266/39397, script_orders:4708 and hud:3361. Robot changes the
    robots.lua substitute at units:3857.
  - Match the teardown order in the same change.
- *Wire after.* 008E9AF0 (cc9, 19870 calls) and 00612DA0 (with E).
- *Reach.* USN04 calls it twice.
- *An ON pair should move.*
  - The `global_subsystems` step goes from record to concrete.
  - TrafficConfig `group_walk_00487270` goes to done.
  - `collect_powerups` and `powerup_applies` move.
  - The modifier factors move only if the power-ups byte [00E0C978] is set in USN04, which is unverified.

**Hint tracker.**
- *Lifetime.* Natively it is created by the first 004E1CA0 call, in OnInit 004E3AA0 at 004E3EFB, which also
  calls 006921B0 ReloadHintTables. It is destroyed by the singleton-lifetime manager (004E1C80 → 004E1A50).
- *Host today.*
  - Stubs at mission:1083 (`notify_award_tracker`, 0068EA00) and mission:1200 (`flush_award_tracker`,
    00690CD0).
  - mission_frame:2054-2080 stands in for the per-frame passes with its own `hint_cooldowns`.
  - menu:512 is mislabelled "reset_award_tracker 004374f0". 004374F0 is
    ProfileManager_RefreshCheckpointState.
- *Order.* Build it with empty containers at OnInit. 00690CD0 reads the profile's shown-hint list at
  *(game+65Ch), so it lands with the profile owner. Loading the definitions (006921B0) is optional.
- *Reach.* USN04: 0068EA00 runs twice, 00690CD0 once.
- *An ON pair should move.* Those two log entries go from unimplemented to implemented. Gameplay does not
  change, because the queue and the history stay empty.

**MissionProgress / profile owner.**
- *Lifetime.* game+6B4h is a pointer at profile+64h; the profile block is inline at game+650h.
  007FDB20 ResetToDefaults allocates it: `PUSH 24h` at 007FDC11, then 00920E10, then the store at 007FDC41.
- *Callers of 007FDB20.* OnInitTitle (004C9AFA), sign-in 0067CC60, NewProfile 00590E60, the profile ctor
  007FEE20, and the read path 007FEFE0 / 007FF100.
- *Real load chain.* RequestRead 007FF100 → storage operation ([0109CECC]) → CompleteRead 007FEFE0 →
  ReadArchive 007FDF00 → 00920000.
- *Writers.* 009205E0 at mission end, behind `kMissionRecordMergeBound` in mission_result, which keeps
  its own `progress_` stand-in today.
- *Order.*
  - Build the empty owner at title init.
  - Point `map_flag_visible` (mission:740) and mission_result at it.
  - Load real profiles later.
- *An empty progress is faithful.* For a fresh profile the native binary also hides every flag, since only
  `mission_completed_00 != 0` counts. So ON is correct when no profile is loaded. Check whether this
  installation has a saved profile first.
- *Reach.* USN04 calls 005C2F70 36 times. JM05 is not needed.
- *An ON pair should move.* All 36 `map_flag_visible` results go from true to false. The menu-only
  difference is expected.

**GUI A (native GUI manager and widget tree).**
- *Lifetime.* 004C12B0 builds it at startup in 0073BAE0 (Phase 7). 00AA5E20 then loads the resources.
  00AA6340 destroys it (provisional).
- *Pages.* Held at +14h. 00AA5840 loads a page and 00AC6600 builds it from 18 widget classes. GuiText
  00AB9650 is the text context that 00AB6BD0 reads at +FCh/+178h.
- *Scene node.* A widget's scene node (+4Ch, flag word at node+138h) is renderer-side. The host must
  keep a stand-in node: today it uses a `node_id` counter. 00AA7DC0 touches the renderer only when
  widget+74h is set.
- *Wire after.* These do not touch the renderer: 00AA6740, 00AA6BC0, 00AB6BD0, 00AA31F0. Then 00AA6720,
  00AD4C30 (it needs an image-shaped font), and 008CA010. Then the cc9 rows 00AA7DC0 (317912 calls) and
  0043ABA0.
- *Reach.* USN04, before the mission.
- *An ON pair should move.* The MissionDetail entries `text_clip_height`, `briefing_text_height` and
  `finish_list_box`, and HudMarkers `widget_size`.

**GUI B and C (front-end managers and MainMenuScreen).**
- *Lifetime.* EnterFrontEndShell 004E4000 builds the managers on base 00684E10. They are destroyed by
  004DA650 / 004DB190. Requests 16h/06h/09h only deactivate them (00683AA0).
- *Main menu.* Init 00686380 builds seven screens at +58h..+70h, commits `INTF_MAINMENU` and enables GUI
  layer 1. Destructor 00686C90.
- *Map point geometry.* 00588C70 has no separate world-map camera: it uses the screen's zoom/anchor at
  +19Ch.
- *Wire after.*
  - 004F75A0..D0
  - 00686170, 00689800, 00689D90, 0068D760
  - 00583E50, 00588C70, 00592640
  - 005C3850, through the MissionTree screen
- *Reach.* USN04, before the mission. Each construct row runs once.
- *An ON pair should move.* The `FrontEndManager::create_*`, `TitleInit::create_attract_screen` and
  `MissionDetail::commit_page_state` entries.

**In-mission D+E (cc9).**
- *Lifetime.* 004DFB70 allocates [00E198C4] (004E0452..004E0480). Init 0068CC70 loads
  `interface/Textures/game.ats`, builds the 42 screens and registers them in B. Mission teardown
  004DA780 destroys it at 004DAB82.
- *Wire after.*
  - 00636D90 (trivial)
  - 005251C0
  - 00644DB0 and 00646040 (the HUD root, 174h, 0068BF60)
  - 00673130 (support, 480h, 006735D0)
  - 005BD420 and 005BD590 (the minimap, 1B0h, 005C0810, which needs the scene capture points at
    game+19CCh+16Ch)
  - 00640D70, 00643360, 00642C20 (markers)
  - 008C7300 (movie screens)
  - 005B5D40 and 005B7390 (narrative; the slot is unverified)
  - 00612DA0 (with 004DC6A0)
  - 005FB080 (with the hint tracker)
  - 00648C20
  - 0068AB80 last, because it collapses screens +BCh, +54h and +4Ch
- *Voice manager.* The voice manager is [[00E198C4]+A4h] (packet `cc15_host_voice_manager`). Its rows
  005B71D0, 00974070, 005A2650, 005B7790 and 005BC640 come after this and need FMOD voice slots
  (00702CC0). Real clips also need the MessageManager's records.
- *Reach.* USN04 (00644DB0 is called 39739 times). For voice, use USN01 or JM08, which have activations.
- *An ON pair should move.* Each HudRootScreen, InGameInterface and HudMinimap log entry goes to
  implemented with the frontier call counts. Warnings go from Queued to applied only once the voice
  manager lands.

**Loading queue, application, VFS pump (game_hosts.cpp).**
- *Loading queue.* 004FDE20 is a lazy 20h singleton: ctor 004FDBA0, registered through 00BD0C30. 00737A50
  RunFrame calls it every frame, then 00509190.
- *Wire after.* `update_loading_queue` (game_hosts.cpp:1185, 40200 calls). The queue is probably empty in a
  mission (unverified).
- *Application and VFS pump.* 00737970 and 00BDB0B0 are fidelity-only. The host already restates both.

**SceneDatabase class table.**
- *Lifetime.* Built in OnInit by 0046F350 and destroyed in OnDestroy 004DC5C0 (0046F3E0).
- *What 004F2800 does.* Registers 26 classes into the map at +34 through 004EE250. Its 27 unlocks are the
  scene creators: mine, convoy, wreck, path, landscape, spawn point, effects, camera path, cloud.
- *Wire after.* 004E99B0, once 006AF3D0 is done.
- *Reach.* USN04 runs both rows during mission load.

## wf1 frontier (original)

Source: `python tools/bsp.py index` (fresh, 446368 edges), `tools/wire_readiness.py`, then
`--frontier` on a USN04 run from cf2989718 (`--frames 5000 --press-start-frame 30 --menu-select
USN04 --mission-frames 4800 --mission-frame-seconds 0.05`; 54 deaths, 796 hit records, exit 0).

The tool reported 186 reached unwired stand-ins (calls > 0 in the native table): 103 `ready`,
58 `callees_done`, 19 `unknown_no_identifier`, 6 `blocked_on_callees`. "Ready" means that the
callees are done. It does not mean the host holds the call site's inputs. I checked each row in a
file that is not cc9's against its host site. Most of them fail on inputs (list below).

Owners: cc9 owns gunnery, units, ai, ship_ai, script_orders, hud (and hud_world, a HUD host
whose history is cc9 and cc13) and commands. cc15 owns scene_contents, mission, mission_frame and
session_messages. game_hosts.cpp and game_hosts_lua.cpp are shared.

## Ready now (strict)

| # | Address / name | Host file | ABI | Owner | Why it is ready | State |
|---|---|---|---|---|---|---|
| 1 | 004CD7F0 BSP_Scene_DeriveShortName at 004D4E80 | game_hosts_scene_contents.cpp | typed (path, append, ShortNameModeFields) | cc15 | 004D4E74 pushes 1, and the host already holds game+61Ch/+614h/+1FE4h as `mode_forced`/`raw_game_mode`/`multiplayer` (passed in by 004dfb70's caller) | **wired**, `kSceneShortNameCc15Bound` (OFF) |
| 2 | 004D87B0 BSP_Game_CheckMultiplayerPlayerCount | game_hosts_mission_frame.cpp (`MissionEntryBinding::check_multiplayer_player_count`, a record today) | callback (`MissionPlayerCountHost`) | cc15 | The first gate is game+1FE4h (004D87CC MOV ESI,[EDI+1FE4h]; JE 004D8993), and the host keeps it as `scene_state.session_mode` = 0. The other two gates are `scene_state.have_scene_record` and `entry_state.game_state`. In single player the body returns at the first gate, so the host callbacks are never reached. | **not wired**: the file was leased to cc13-powerup, then to cc13-aimunit (until 10:25Z). I queued `cc15-wf1-mf`. |

Row 1's pair (USN04, OFF = bf235ac61 tree build, ON = pair_export flip): `pair_diff` exit 1.
Gameplay, unit table and native table are identical. The only moved line is ON's evidence note
`scene short name: usn_19SOLO (mode 614=0 61c=0 1fe4=0)`. The host's raw mode is 0, not
the stand-in's 8. Both map to index 8 ("SOLO") through 004CDB61.

## Hand to cc9 (reached in USN04, not wired, inputs not verified by wf1 unless noted)

UNIMPLEMENTED in the native table (calls in USN04):

| Address / name | Host file | Calls | Note |
|---|---|---|---|
| 00C5BB30 dyn_world_substep (ShipMotion::rigid_body_substep_schedule) | units | 86400 | Mit/Dyn physics. Needs the physics world image, so check before calling it ready |
| 0098B370 spatial_sweep_segments (MissionCamera::collision_ray) | hud | 21885 | camera probe ray against the target's collision |
| 009FABE0 bot_set_direction_from_heading (BotStateMoveTo::direction) | units | 5609 | |
| 00673130 support_manager_update_info_panel (camera_screen_frame_step) | hud | 4800 | |
| 00A7B710 set_sound_listener_environment | hud | 4800 | sound backend side, check |
| 00644DB0 hud_root_poll_selection | hud | 4539 | |
| 005BD420 select_nearby_capture_point (HudMinimap::prepare_frame) | hud_world | 4539 | |
| 005BD590 construct_minimap_icon_entry | hud_world | 9 | |
| 007BB110 unit_can_release_ordnance | units | 3797 | **not ready**: the host comment says unit+9C3h, the per-slot byte, is not held |
| 008E9AF0 power_up_manager_collect_active | hud | 2270 | **not ready**: the power-up manager [00F88C30] is not built |
| 008069A0 BSP_Recon_PublishGroupLevelFromMembers | gunnery | 162 | |
| 0071FB90 director_path_object_construct | commands | 45 | |
| 009A7650 bot_approach_dogfight_set_target | units | 34 | |
| 007C11E0 BSP_Plane_NotifyFlightState | units | 12 | |
| 00646040 hud_root_set_interface, 0068AB80 collapse_in_game_overlays | hud | 6 / 6 | |
| 00954990 unit_broadcast_controlled_audio_flag, 0080E290 BSP_Unit_AimTurningGunsToRest, 005251C0 hud_screen_reset_selection | hud | 3 / 1 / 2 | |
| 005B5D40 hud_narrative_hide_text | script_orders | 1 | |

The host already runs its own typed restatement of each of these, which the table shows as
"concrete". Wiring one means swapping that restatement for the recon body, and that is cc9's call:
0079A3B0 HudMovieCamera_Update, 0084BAD0, 0085AD00, 008636A0, 00863A80, 008777D0, 008FDAF0,
00926E80, 00729BC0, 0072BF10 (gunnery); 009DE2F0 (ship_ai); 00720CD0, 008356F0 (commands);
007CC820, 007EF010 (units); 00AA7DC0 (hud_world); 006B8610, 00888D20, 00B66200 (lua).

## Rejected as not really ready (cc15 and shared files)

- 0046BF20 / 0046BF70: already behind `kVehicleClassPreloadCc15Bound`, which is OFF after the USN21 crash.
- 004D2A80: already behind `kMissionStartTemporaryReleaseCc15Bound`.
- 00469610 queue_entity_command: milestone 2l issues each authored token directly. The producer
  would need 0077D600 issue and 00925A90 lookup in the deferred-resolve host, and the command side is cc9's.
- 004697B0, 004C8AA0: these read raw over the scene database and the 00F8A09C/00F8A0B0 globals. The
  host keeps those as std::vector, not image-shaped.
- 00B0FD70 remap textures, 00874640 / 006AD600 render resets, 00B1BF90 render queue: renderer.
- 007065E0 load_messages: the MessageManager [game+21DCh] is not hosted.
- 0096C070 proximity point effect: render-side effect object.
- 00A91020: there is no input backend. The host injects the device edge.
- 004C3840: 004E044D gates it on game+1FE4h == 1, which never holds in single player.
- 004DC6A0 global subsystems, 004E99B0 class creator, game_hosts.cpp 004FDE20 / 00737970 /
  00BDB0B0: native object bring-up.
- 005C2F70 map_flag_visible: MissionProgress (game+6B4h) is never loaded from the profile. ON
  would hide every flag the native shows.
- 008D57A0 voice_language_name: the voice folder is in the language catalog, but the
  DL_Content_0000062 ownership input is not hosted (the header cites 00706760, which the source
  has as load_message_map, so that evidence conflicts). Its consumer, 0058C9BE, is unimplemented anyway.
- 00583E50, 00AB6BD0, 00AA6BC0, 00588C70, 00683820: these read raw over GUI screens and widgets or the world-map camera.
- 0068EA00, 00690CD0: the hint and award tracker is not hosted.
- 00A3E3B0, 0067C8F0, 00A3F3D0: XenonSystemManager [00F8ABE8] is not built (online).
- 0076DA60, 007FA2D0, 006878F0: session and multiplayer steps.
- 00AB98F0 glyph_quad: the host already runs `write_font_quad_00ab98f0_fragment`.
- 00427190 BSP_AutoTest_Step (menu): mode 0 returns at once, so it is correct to wire, but
  `AutoTestStepHost` has about 73 pure virtuals for a no-op. It is deferred, not rejected.
