# Mission state frame (entering 0Dh and the in-mission dispatch)

Addresses: 004e4a40 00447060 004d87b0 00685c80 004c9ca0 00446ef0 004da6c0 004db920 004d7ea0 004d7970

Packet `cc_mission_frame`, worktree `agent/cc-mission-frame`. Ghidra was read-only for this packet;
every name below is a hypothesis, not a recovered symbol.

This packet closes the two remaining holes in the mission-state entry and turns the in-mission
branch of `BSP_Game_OnMove` into one ordered host dispatch, so a headless mission frame can be
wired by implementing methods rather than by re-reading the listing.

## What the entry leaves behind

`docs/MISSION_STATE_ENTRY.md` reconstructs the two entry routines themselves. `004DA6C0`
`BSP_Game_EnterMissionState` is what the per-frame update expects to have run:

| It sets | To | Why the frame needs it |
| --- | --- | --- |
| `game+5D4h` | **0Dh** | every guard in the in-mission branch tests this value |
| `game+21F0h` | `0.0f` | the scaled delta the whole world tick receives, restarted at zero |
| `game+634h` / `game+635h` | both clear (`004CD0F0(0,0,1)`) | opens the outer simulation gate at `004e50b0` |
| `game+608h` | 0 | the companion byte of the same gate |
| `game+1EE1h..+1EE5h` | cleared, `+1EE3h` = `game+1FE4h != 0` | the result-and-failure record the exit writes again |
| `game+30h` | released (`00447060`) | the deferred dynamics list, below |

It **creates no manager**. Every subsystem the frame walks was built one frame earlier by
`BSP_Game_LoadMissionScene 004DFB70` through `004DC6A0` (`game+21D0h` plus the five global
constructions) and `004DE610` (`game+19CCh`, `+19E8h`, `+19ECh`, `+19F0h`, `+19FCh`, `+21C8h`,
`+21CCh`, `+21D4h`, `+21DCh`, `+21E8h`); `game+21A0h` comes from `BSP_Game_OnInit` at startup.
`004DA6C0` only resets state.

The interface it requests is `BSP_Game_ApplyInGameInterface(game, 0)` at `004DA746`, and the
argument polarity is the reverse of the ledger's "loading" reading: see Corrections. The entry
call **tears down** the loading element, releases `interface/textures/allbutingame.ats`, selects
front-end layout set 3, and writes the latch that starts the in-engine movie. It calls no script
entry: the mission script is already running, started by the scene load.

### `00447060`, the deferred dynamics release

`__thiscall void(sub)` on `game+30h`, `RET` at `0044713a`. The release target is captured once at
`0044706a`, before anything else: `MOV EAX,[00e188a8] / MOV EBP,[EAX+18h]`. It is a sub-object of
the game singleton, not of the list.

1. `00447075..004470b2`: walk the 20h-stride record vector at `+14h`/`+18h` and hand each record's
   `+0Ch` dword to `00c34f70` with that captured ECX. The loop **re-reads begin and end on every
   pass** and stops as soon as the index is no longer below the current element count, so a release
   that shrinks the vector cuts the walk short. That is reproduced.
2. `004470b4..00447106`: `erase(begin, end)` inlined. The move loop copies the empty range
   `[end, end)` and `+18h` is set back to `+14h`.
3. `00447109..0044712e`: the same erase on the 8h-stride range at `+4h`/`+8h`, through `00446ef0`.
   `00446ef0` is `std::vector<T>::erase(iterator, iterator)` with `_SECURE_SCL` checked iterators:
   `RET 14h`, five dwords, a hidden return-iterator pointer plus two `{container, ptr}` pairs; the
   move loop at `00446f30` copies eight bytes per element.

Both ranges therefore end empty. What the records hold, and which subsystem owns `game+30h`, is
still open; only `record+0Ch` is ever read.

### `004D87B0`, the multiplayer player-count check

`__fastcall void(GGame*)`, `RET` at `004d8996`, SEH frame `00c664f0`. Three early returns
(`004d87c1` `game+1FE4h`, `004d87cd` `game+5FCh`, `004d87d9` `game+5D4h != 0Dh`), then the arm
selector `[game+218Ch]` at `004d87f6`.

The **counting arm** (selector clear) counts sides 0 and 1 either through `004bb770`, or, when
`[00f8a2fc]+4Dh` is set, by walking the eight `118h`-stride blocks at `game+748h` with `004b5530`
and taking the side from `+28h`. Effective game mode 7 needs more than one participant in total
(`004d88c6`); every other mode needs both sides non-empty (`004d88dc`). On the first failure it
arms `00e18b40 = 00f876a4 + 8.0` (the double at `00ce3db0`), sets `game+1EE5h`, latches `00e18b44`
and returns. On a later failure it raises `ingame.multi_notenoughplayer`, and in mode 1 past the
deadline calls `004d7970(0)` - the call that moves `game+5D4h` and makes `004da6c0`'s re-check at
`004da75a` meaningful.

The **side-balance block** at `004d88eb` is the shared continuation of both arms, not the
alternative arm: see Corrections. It needs `00e188bd` clear and `game+1FE4h == 1`, takes its length
from `[[00e188a8]+5FCh]+988h`, and initialises three side counters to `-99` so that "never seen"
stays separable from "seen but empty". A slot with `+9h` set is skipped; a slot with `+1Bh` set
marks its side seen without counting it. Only an exact zero on side 0 or side 1 fires `00982990`,
`00e188bd = 1` and `00530650`.

## The in-mission branch of `BSP_Game_OnMove` (004e4a40)

The frame spine - prologue, input poll, window close, blocking screen, the three front-end states,
the render-queue open, the request drain, the delta scale and the frame clock - is
`docs/GAME_FRAME_CONTROL.md` and `bsp::run_game_frame_control`. What follows is everything the
mission state adds, in listing order, read from the disk bytes `004e4a40-004e5535` (the stored
Ghidra body is still eight bytes).

`run_mission_frame` in `src/mission_state_frame.cpp` executes exactly this order with exactly these
guards. `mission_frame_step(i)` carries the same rows as data: host method, call site, callee,
owner area, and the symbol on main that already implements the step.

| # | Call site | Callee | Host method | Owner | Reconstruction on main |
| --- | --- | --- | --- | --- | --- |
| 1 | 004e4e0f | 004b6260 | `session_counts_mission_start_004b6260` | session | `session_counts_mission_004b6260` |
| 2 | 004e4e25 | 004bcaa0 | `adjust_mission_start_counters_004bcaa0` | session | `adjust_mission_counters_004bcaa0` |
| 3 | 004e4e67 | 00987590 | `update_warning_manager_00987590` | script | `update_mission_events_00987590` |
| 4 | 004e4e6c | - | `run_input_effect_sets_004e4e6c` | input | `run_input_effect_lists_004e4e6c` |
| 5 | 004e4feb | 004d8cd0 | `record_action_deadlines_004d8cd0` | unit | `record_action_deadlines_004d8cd0` |
| 6 | 004e5036 | 00778560 | `multiplayer_tick_00778560` | session | `run_multiplayer_tick_00778560` |
| 7 | 004e50ab | 00be3640 | `begin_game_profile_block` | profiler | `profiler_begin_frame_slot_00be3640` |
| 8 | 004e50e3 | 0041e870 | `register_game_block_label` | profiler | none |
| 9 | 004e5133 | 004c40a0 | `update_in_mission_subsystems_004c40a0` | unit | none |
| 10 | 004e5147 | 004cce50 | `begin_engine_movie_004cce50` | HUD | none |
| 11 | 004e5153 | - | `pause_gate_branch_004e5153` | pure | `decide_simulation_gate_branch` |
| 12 | 004e5227 | - | `tutorial_hint_step_available` | HUD | none |
| 13 | 004e522d | 0054e440 | `advance_tutorial_hint_0054e440` | HUD | none |
| 14 | 004e5234 | 004db030 | `toggle_pause_menu_004db030` | HUD | none |
| 15 | 004e523b | 004db030 | `toggle_pause_menu_004db030` | HUD | none |
| 16 | 004e524c | - | `in_game_interface_active` | HUD | none |
| 17 | 004e5252 | 0068c1f0 | `update_in_game_interface_0068c1f0` | HUD | `audio_environment_0068c1f0` (partial) |
| 18 | 004e5259 | 004c40f0 | `update_interface_only_004c40f0` | HUD | `run_interface_only_update_004c40f0` |
| 19 | 004e526d | 0068ec10 | `hint_tick_cooldowns_0068ec10` | script | `tick_hint_cooldowns_0068ec10` |
| 20 | 004e5279 | 00692b00 | `hint_drain_queued_00692b00` | script | `should_drain_queued_hint_00692b00` |
| 21 | 004e5285 | 00692b60 | `hint_unit_class_00692b60` | script | `unit_class_hint_00692b60` |
| 22 | 004e5291 | 006926f0 | `hint_weapon_006926f0` | script | `surface_weapon_hint_006926f0` |
| 23 | 004e529d | 00692580 | `hint_environment_00692580` | script | `environment_hint_00692580` |
| 24 | 004e52a9 | 00692fd0 | `hint_zone_first_get_00692fd0` | script | `scan_capture_zones_00692fd0` |
| 25 | 004e52b5 | 00692960 | `hint_strategic_map_00692960` | script | `should_show_strategic_map_hint_00692960` |
| 26 | 004e52ca | 00914ef0 | `update_bot_scheduler_00914ef0` | unit | `update_bot_scheduler_00914ef0` |
| 27 | 004e52df | 006dc1a0 | `update_markers_006dc1a0` | HUD | `update_markers_006dc1a0` |
| 28 | 004e52f4 | 00481640 | `update_entity_manager_00481640` | unit | `update_entity_manager_00481640` |
| 29 | 004e5309 | 00865ab0 | `update_rain_descriptor_00865ab0` | renderer | none |
| 30 | 004e5325 | 00bbddd0 | `update_ocean_00bbddd0` | renderer | `run_ocean_and_effects_tick` |
| 31 | 004e533a | 00740e10 | `update_decals_00740e10` | renderer | `update_decal_manager_00740e10` |
| 32 | 004e534f | 0094c8f0 | `update_00f89b3c_0094c8f0` | renderer | none (trivial body in this build) |
| 33 | 004e535a | 008eb110 | `update_power_ups_008eb110` | unit | `update_power_ups_008eb110` |
| 34 | 004e5377 | 00867ee0 | `update_effect_manager_00867ee0` | renderer | `run_ocean_and_effects_tick` |
| 35 | 004e5382 | 00903670 | `flush_entity_activations_00903670` | unit | `flush_entity_activations_00903670` |
| 36 | 004e5389 | 004d7ea0 | `check_mission_completion_004d7ea0` | script | `check_mission_completion_004d7ea0` |
| 37 | 004e53ad | 00b19a10 | `set_particle_clock_time_00b19a10` | renderer | `set_particle_clock_time_00b19a10` |
| 38 | 004e53b6 | 004c40f0 | `update_interface_only_004c40f0` | HUD | `run_interface_only_update_004c40f0` |
| 39 | 004e53d9 | 00af0450 | `set_foliage_shader_time_00af0450` | renderer | none |
| 40 | 004e540f | 00af0c50 | `build_foliage_visible_set_00af0c50` | renderer | `build_foliage_visible_set_00af0c50` |
| 41 | 004e5416 | 004c6c70 | `apply_gui_visibility_004c6c70` | HUD | `decide_gui_visibility_004c6c70` |
| 42 | 004e541b | - | `interface_manager_present` | HUD | none |
| 43 | 004e542d | 00685c80 | `update_interface_music_00685c80` | **sound** | none (named by this packet) |
| 44 | 004e5434 | 004d80d0 | `update_multiplayer_interface_004d80d0` | session | none |
| 45 | 004e5442 | 006840f0 | `service_pending_menu_requests_006840f0` | HUD | `service_pending_menu_requests_006840f0` |
| 46 | 004e5462 | 00776230 | `pump_peer_queues_00776230` | session | `run_menu_interface_drain_004e5434` |
| 47 | 004e5469 | 004c40f0 | `update_interface_only_004c40f0` | HUD | `run_interface_only_update_004c40f0` |
| 48 | 004e5477 | 006840f0 | `service_pending_menu_requests_006840f0` | HUD | `service_pending_menu_requests_006840f0` |
| 49 | 004e5496 | 00941140 | `apply_sound_requests_00941140` | sound | `apply_sound_request_00941140` |
| 50 | 004e549d | 004d8620 | `update_front_end_screens_004d8620` | HUD | none |
| 51 | 004e54b0 | 00be3660 | `end_game_profile_block` | profiler | `profiler_end_frame_slot_00be3660` |
| 52 | 004e54ca | 00be3640 | `begin_render_profile_block` | profiler | `profiler_begin_frame_slot_00be3640` |
| 53 | 004e54d1 | 004ca440 | `render_004ca440` | renderer | none |
| 54 | 004e54e4 | 00be3660 | `end_render_profile_block` | profiler | `profiler_end_frame_slot_00be3660` |
| 55 | 004e54eb | 004ca1f0 | `finish_render_frame_004ca1f0` | renderer | none |
| 56 | 004e550c | - | `frame_metrics_enabled` | profiler | none |
| 57 | 004e551e | 00757ce0 | `submit_frame_metrics_00757ce0` | profiler | none |

### The guards between them

- **004e4df4** `game+5D4h == 0Dh` admits steps 1-4. Steps 5-7 run in every state.
- **004e4e00** the one-shot `game+1EE7h` admits steps 1-2 once per mission; step 2 additionally
  needs step 1 true and `game+624h == 0`.
- **004e4e31** `game+634h != 0` skips steps 3-4 entirely: a cinematic suspends the warning director
  and the injected-input sets, not just the simulation.
- **004e4e50** `COMISS` against the `0.0f` at `00d7a218` with `JBE`, so a scaled delta of exactly
  zero does not run step 3. Step 3 also needs `game+21E0h` non-null.
- **004e5000..004e5020** step 6 is skipped only when all three hold: state 0Dh, `[00f876b0] > 0`
  and a positive scaled delta. Every other frame ticks the network.
- **004e50b0** the outer gate: `game+634h == 0 || game+635h != 0`. **004e5118/004e5124** then
  require state 0Dh and `game+7184h == 0`. Failing any of the three jumps to `004e53b4`, which runs
  step 38 and nothing else: steps 9-37 are all inside the gate.
- **004e5153..004e5211** selects one of three arms. `kSimulationOnly` (`004e517a`) jumps straight to
  step 19; `kPauseMenuOpened` runs steps 12-15; `kInterfaceOnly` runs steps 16-18. The pause arm's
  tutorial half needs `game+634h` set **and** `[[00e198c4]+A8h]+8h` set, and it falls through into
  the second toggle, so that arm calls `004db030` twice in a row (`004e5234`, `004e523b`).
- **004e52f9** `game+19E8h != 0` admits steps 29-30.
- **004e53bb** state 0Dh admits steps 39-40 only.
- **004e5455..004e5488** steps 46-48 form the do/while Ghidra removes as unreachable. It runs the
  menu state machine to a fixed point with no iteration bound, keyed on the out byte `006840f0`
  writes through its pointer argument.
- **004e54b5** reads a byte both incoming paths zeroed at `004e548a`, so the render block is never
  skipped; no gate is modelled for it.

### The ECX the pseudocode drops

Every `call getter; mov ecx,eax; call method` pair and every `mov ecx,[esi+off]` before a call is
lost in the decompiler output. From the listing: step 26 `game+21A0h`, step 27 `game+21D4h`,
step 28 `game+21D0h`, step 30 `game+19E8h` with `game+19FCh` as its first argument, step 31
`00e1aea0`, step 32 `00f89b3c`, step 33 `00f88c30`, step 35 `game+19CCh`, steps 39-40 `00f8c274`,
step 43 `00e198ac`, step 46 `game+1EF0h`, step 6 `[00e188a8]+1EF0h`. Step 37's argument is the
global time in **milliseconds** (`00f876a4 * the double 1000.0 at 00ce47a0`), not a delta; step 39's
argument is `[[game+5FCh]+1054h]`; step 40's second argument is `[[game+19F0h]+A8h]+D0h` when that
object's `+CCh` byte is set and `+C8h` when it is clear.

## The exit

Nothing in the frame leaves state 0Dh directly. Step 36 (`004d7ea0`) is the only producer: while
the request queue is empty and `game+7188h` holds a mission-result object whose `+21h` is set, it
enqueues **state request 0Fh**. The drain dispatches 0Fh to `BSP_Game_EndScene 004d7970`, which
`docs/MISSION_RESULT_DECISION.md` already reconstructs: once per scene behind the `game+1EE2h`
latch it writes `game+1EE1h = aborted`, raises the debrief `00920a20`, commits the 284h score
record into the profile through `009205e0` on a normal end or discards the records through
`007fa1b0` / `00916980` on an abort, and then, while `game+5D4h` is 0Dh or 0Fh, broadcasts
end-of-scene and enqueues **request 10h**, the teardown. This packet references that; it does not
re-derive it. `004d87b0` is the other route into `004d7970`, through the mode-1 deadline above.

## Corrections

- **docs/MISSION_STATE_ENTRY.md, `004D87B0`.** "With `session+29Ch` set it instead counts entries
  per side over `game+18CCh`" reads the block at `004d88eb` as the alternative arm. It is the
  shared continuation. `004d88e4` (`MOV byte [00e18b44],0`, the success path of the counting arm)
  falls straight through into `004d88eb`, and the selector-set arm jumps to the same label; only
  the counting arm's failure paths bypass it. So a healthy multiplayer mission runs the side-balance
  check every frame, and it is not conditional on the selector at all. The same passage omits that
  the walk length comes from `[[00e188a8]+5FCh]+988h`, that the three counters start at `-99`, and
  that the `+1Bh` byte marks a side seen without counting it.
- **Ledger `004C9CA0` `BSP_Game_ApplyInGameInterface`.** The argument is not "loading": `004c9cc0`
  `CMP byte [ESP+1Ch],0 / JE 004c9d6b` sends argument **0** - the mission-entry call - to the arm
  that tears the loading element down, and argument **1** - the scene-load call at `004e1873` - to
  the arm that allocates it (`push 34h`, ctor `00636d90`, stored at `[00e198c4]+D4h`, `+4h = 1`).
  The entry arm additionally releases `interface/textures/allbutingame.ats` through `00aefa30` with
  `ECX = [00f8c26c]`, selects front-end layout set 3 (`BSP_FrontEndFrame_SelectLayoutSet(3,1)` at
  `004c9dff`), and at `004c9e1d..004c9e2c` writes **`game+1EE0h = (the mission key at game+2198h was
  not found in the container at game+650h)`**. That is the one-shot `BSP_Game_OnMove` tests at
  `004e5138` to start the in-engine movie, so the mission entry is its writer - a fact
  `docs/GAME_SIMULATION_GATE.md` carries as an unattributed latch.
- **docs/GAME_ON_MOVE_MAP.md step 23.** `00685c80` is listed as GUI/HUD with `DAT_00e198ac`
  unidentified. It is sound: `__thiscall(manager, float rawDelta)`, `RET 4`, and its body re-applies
  the float at `00f889a8` to the two objects at `manager+50h` and `manager+54h` through `00a864f0`
  and advances each with `00a874d0(rawDelta)`, suppressed while `[00e198b8]+40h` reports an active
  screen. `00f889a8` is the settings block `00f88980 + 28h`, read only here, in
  `BSP_MainMenu_StartTitleMusic` and in `BSP_Application_Initialize`, so it reads as the music
  volume. Named `BSP_InterfaceManager_UpdateMusicStreams`.
- **docs/GAME_ON_MOVE_MAP.md step 18.** "The pause branch runs `0054e440()` and `004db030()` twice"
  is right about the count but not about the condition: `0054e440` needs `game+634h` set as well as
  the hint object's `+8h` byte, and the two `004db030` calls are the tutorial arm's own call at
  `004e5234` falling through into the shared call at `004e523b`. A pause with no cinematic running
  calls `004db030` exactly once.

## Uncertainties

1. The owning subsystem of `game+30h`, and what the 20h dynamics records hold beyond `+0Ch`.
2. Whether `00a864f0` sets a volume or a target, and therefore whether step 43 is a re-apply or a
   fade retarget. Only the argument's provenance is established.
3. `[00f8a2fc]+4Dh`, the selector between the two participant counts in `004d87b0`.
4. `game+624h`, still carried raw: it suppresses the mission-start counter bump at `004e4e18` and
   the same bump inside `004d7970`.
5. What `0094c8f0` (step 32) would do in a build where its body is not trivial, and what
   `00f89b3c` is beyond "constructed by `00945820` inside `004DE610`".

## no_ghidra_function

none. Every routine this packet read has a Ghidra function. `004e4a40` has one whose stored body is
eight bytes (`004e4a40-004e4a47`), a defect already recorded in
`reports/game_onmove_body_repair.json`; every address in the table above was taken from the disk
listing instead, and the index's call graph for the function agrees with it.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `game_dynamics_list` | 00447060 00447b80 00445db0 00446ef0 00c34f70 | docs/GAME_DYNAMICS_LIST.md | Still open from `mission_state_entry`: what the `game+30h` subsystem integrates per frame and what the 20h record is. This packet reconstructed only the release. |
| `in_mission_subsystem_tick` | 004c40a0 00875bb0 004c3cb0 00447b80 | docs/IN_MISSION_SUBSYSTEM_TICK.md | Step 9, the fixed four-call sequence that opens every simulated frame and has no reconstruction. |
| `interface_stream_audio` | 00685c80 00a864f0 00a874d0 00f889a8 005b8c30 | docs/INTERFACE_STREAM_AUDIO.md | Step 43: what `manager+50h`/`+54h` are, and whether `00a864f0`/`00a874d0` are a level set plus a fade advance. |
| `mission_frame_render_entry` | 004ca440 004ca1f0 004c6c30 | docs/GAME_RENDER_FRAME.md follow-up | Steps 53 and 55 are the only unreconstructed calls left in the render block. |
| `front_end_active_screens` | 004d8620 00425d10 00e19698 | docs/FRONT_END_ACTIVE_SCREENS.md | Step 50, the last update before the render block; its slow path collects screens the fast path skips. |

## Reconstruction

`include/bsp/mission_state_frame.hpp`, `src/mission_state_frame.cpp`,
`reports/mission_state_frame.json`.

- `release_all_dynamics_00447060` over `DynamicsReleaseHost`, one method.
- `run_mission_player_count_check_004d87b0` over `MissionPlayerCountHost`, plus the pure rules
  `mission_player_count_check_runs`, `mission_player_counts_sufficient`,
  `count_bound_side_entries` and `side_balance_broken`.
- `run_mission_frame` over `MissionFrameHost`, one pure-virtual method per native call site in
  call order, and `mission_frame_step()` exposing the table above as data.
- `SimulationGateBranch` is reused from `bsp/simulation_gate.hpp`; `GameStateId`,
  `MissionOneShots` and the entry hosts are reused from `bsp/game_frame_control.hpp` and
  `bsp/mission_state_entry.hpp`. Nothing is redefined.

## State reached

| Address | State |
| --- | --- |
| `00447060` | reconstructed, build-tested |
| `004d87b0` | reconstructed, build-tested |
| `004e4a40` | in-mission branch reconstructed as a host sequence, build-tested |
| `00685c80` | analysed and named |
| `004c9ca0` | analysed (argument polarity and the `+1EE0h` write only) |
| `00446ef0` | identified as `std::vector::erase(iterator, iterator)`, not reconstructed |

Nothing here is ABI-compatible or game-validated. `run_mission_frame` orders host calls; it does not
contain any subsystem's behaviour.
