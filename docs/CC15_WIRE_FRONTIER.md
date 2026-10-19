# cc15-wf1: single-player wiring frontier (2026-10-19)

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
