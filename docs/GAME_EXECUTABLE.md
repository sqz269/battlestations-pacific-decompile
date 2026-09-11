# bsp_game.exe, milestone 1

Addresses: 008f81f0 (WinMain), 0073d410 (cSkeletonAppMidway::Init), 00becda0 (Win32 platform
object), 00becee0 (window configuration), 00beb2c0 (save storage), 00b32410 (renderer
constructor and `Direct3DCreate9`), 00b2aeb0 (device creation), 00bec1a0 (platform message
loop), 00bed3b0 (window procedure), 004ca2f0 (window close policy), 00737970 (application
construction), 00737a50 (application frame), 00737f30 (application shutdown).

Packet `game_executable_milestone_1`, owner `agent/cc-game-exe`. Sources:
`src/game_main.cpp`, `src/game_hosts.cpp`, `include/bsp/game_hosts.hpp`. Target registration
is in `cmake/startup.cmake`. Report: `reports/game_executable_milestone_1.json`.

## What the milestone is

Before this packet the repository built `bsp_core` plus probes and tests: reconstructed
sequences with injected host interfaces, exercised by diagnostic programs. There was no
process that ran the startup spine itself. Milestone 1 is that process, and only that: a
Win32 executable that enters at the reconstruction of 008f81f0, walks the reachable phases of
0073d410, opens the game window, creates the Direct3D 9 device, runs 00bec1a0 with the
reconstructed application frame 00737a50, clears and presents every frame, and shuts down in
the order the recovered exit sequence prescribes.

The milestone deliberately does not add reconstruction. Every host method the spine needs is
supplied in one of two ways, and never in a third:

1. A concrete implementation, either a reconstructed routine already in `src/`, or the plain
   Win32 / Direct3D 9 call the native routine makes inline at that site.
2. The unimplemented-host policy in `GameHostLog`: the method name and the native call-site
   address it stands for are recorded, a neutral value is returned, and nothing is invented.

No stub pretends to be game behaviour, and `config/reconstruction/meta.json` still reports
`game_rebuilt: false`: a process that presents a cleared back buffer is not the game.

## The sequence a run executes

1. `WinMain` attaches the parent console when one exists, parses `--frames` and `--log`, opens
   the run log, and calls `bsp::run_win_main` with the four arguments 008f81f0 ignores.
2. `CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)` then `CoInitializeSecurity` with
   `RPC_C_AUTHN_LEVEL_NONE` and `RPC_C_IMP_LEVEL_IMPERSONATE` (008f81f8, 008f820a).
3. Games Explorer registration (008f8245 `CoCreateInstance(CLSID_GameExplorer)` through
   008f82c4) is logged and skipped. The native path calls `ExitProcess` when
   `IGameExplorer::VerifyAccess` denies the title; a reconstruction milestone has no reason to
   consult a machine parental-controls policy, so the interface is never created.
4. `CoUninitialize` (008f82cc), then the random-thread registry 00bd2e20 / 00bd2fe0 over the
   reconstructed `bsp::RandomThreads`, with the one-byte per-thread slot of 00bf681b.
5. `CreateMutexA` on the single-instance name at 00d16a64 (008f8301). A second instance takes
   the already-running branch: language resolution 008f7db0 followed by the `MessageBoxW` at
   008f83c5, and the process returns 0 without closing the mutex, exactly as the original does.
6. `SetThreadAffinityMask(GetCurrentThread(), 1)` (008f83fc). The game resource factory
   singleton at 008f840b is unimplemented.
7. Application construction 00737970, which is the default `ApplicationFrameState`: byte +18h
   set, +19h and +1Ah clear.
8. Application initialize, `flags=0` and `mode="cachedload"` as pushed at 008f8419.
   The phases that run concretely are: module path capture 00439040, the allocation-stats
   object 00be2900, the frame-clock singleton 00bedfb0, the object-handle resolver slots
   006ad0d0, the command-line switch table 0073ce20, the Win32 platform object 00becda0, save
   storage 00beb2c0 under `Documents\Battlestations-Pacific\save`, window configuration
   00becee0, and device creation 00b2aeb0. The hardware probe 0073c3b0, the VFS phase, the
   settings load 008d8190, renderer resources, input, sound, locale, GUI, world effects and
   game entry are recorded as unimplemented phases.
9. `bsp::platform_run_loop_00bec1a0` with frames enabled. Each iteration without a pending
   message calls `bsp::run_application_frame`, then clears and presents.
10. Shutdown. 00737f30 tears down 23 singletons, the GUI manager, the game object and four
    datatable files, none of which exist here, so that teardown is one unimplemented record.
    The two steps the process genuinely owns run: the device is released and the window is
    destroyed and its class unregistered. Then application destruct 008f8444, the singleton
    lifetime manager 008f8449 (unimplemented), `CloseHandle` on the mutex 008f846e, and the
    random-thread teardown 00bd3050 / 00bd30d0.

## What it does on screen

A 640x480 captioned window titled `Battlestations Pacific` appears at the top left of the
desktop and shows a dark blue cleared back buffer for as long as the run lasts. Closing it
sets the pending-close byte at platform+180h; the next frame consumes it through the recovered
front-end branch of 004ca2f0, sets the global exit byte 00e1ae75, and the frame propagates
that to platform+181h so the loop finishes and shutdown runs. Nothing else is drawn: there is
no title page, no GUI layer and no game object, because none of those phases are reconstructed
into this milestone.

The resolution is not read from the registry. The settings block at 00f88980 is filled by
008d8190 in phase 5, which is unimplemented, so the run uses the 640x480 fallback pair stored
at 008d841f when a parsed resolution is not in the supported table.

Two details are milestone additions rather than recovered behaviour, and are marked as such in
the source:

- The clear colour and the `Clear` / `BeginScene` / `EndScene` / `Present` sequence. The native
  renderer frame routine behind renderer virtual +20h is not reconstructed; this is the
  milestone's own present, recorded in the log with the address `milestone`.
- `game_window_procedure` implements only the WM_CLOSE branch of 00bed3b0 and forwards
  everything else to `DefWindowProcA`. The activation, resize, input and sizing-loop branches
  are not reconstructed, and the native handler's window-extra storage is replaced by one
  process-wide platform pointer because that layout is not recovered.

## Host methods

Generated from the run log of `bsp_game.exe --frames 60 --log local/game_run.log`:
43 concrete, 25 unimplemented, 68 distinct methods and phases reached.

The call counts are what the 60 frame run observed. `game_state` is reached twice per frame
because 00737a50 reads it before the input edge test and again after the game update; the
second read is dead in the native body but is preserved by the reconstruction.

| Host method | Native call site | Status | Calls in the 60 frame run |
| --- | --- | --- | --- |
| `StartupHost::com_initialize` | `008f81f8` | concrete | 1 |
| `StartupHost::com_initialize_security` | `008f820a` | concrete | 1 |
| `StartupHost::game_explorer_create` | `008f8245` | **unimplemented** | 1 |
| `StartupHost::com_uninitialize` | `008f82cc` | concrete | 1 |
| `StartupHost::random_threads_initialize` | `00bd2e20` | concrete | 1 |
| `StartupHost::allocate_thread_slot` | `00bf681b` | concrete | 1 |
| `StartupHost::random_threads_register_current` | `00bd2fe0` | concrete | 1 |
| `StartupHost::create_single_instance_mutex` | `008f8301` | concrete | 1 |
| `StartupHost::set_thread_affinity_to_first_processor` | `008f83fc` | concrete | 1 |
| `StartupHost::publish_game_resource_factory` | `008f840b` | **unimplemented** | 1 |
| `StartupHost::application_construct` | `00737970` | concrete | 1 |
| `StartupHost::application_initialize` | `0073d410` | concrete | 1 |
| `Phase 0 capture_module_directory` | `00439040` | concrete | 1 |
| `Phase 0 construct_allocation_stats` | `00be2900` | concrete | 1 |
| `Phase 0 construct_frame_clock_singleton` | `00bedfb0` | concrete | 1 |
| `Phase 0 install_object_handle_resolvers` | `006ad0d0` | concrete | 1 |
| `Phase 1 parse_command_line` | `0073ce20` | concrete | 1 |
| `Phase 1 probe_hardware` | `0073c3b0` | **unimplemented** | 1 |
| `Phase 2 vfs_provider_manager` | `00beda60` | **unimplemented** | 1 |
| `Phase 2 mount_packages` | `0073cb10` | **unimplemented** | 1 |
| `Phase 3 construct_win32_platform` | `00becda0` | concrete | 1 |
| `SaveStorageHost::special_folder_path` | `00beb2f3` | concrete | 1 |
| `SaveStorageHost::create_directory` | `00beb35e` | concrete | 2 |
| `Phase 3 initialize_save_storage` | `00beb2c0` | concrete | 1 |
| `Phase 5 load_game_settings` | `008d8190` | **unimplemented** | 1 |
| `PlatformWindowHost::load_arrow_cursor` | `00becf0f` | concrete | 1 |
| `PlatformWindowHost::register_class` | `00becf80` | concrete | 1 |
| `PlatformWindowHost::adjust_window_rect` | `00becfd6` | concrete | 1 |
| `PlatformWindowHost::create_window` | `00bed01d` | concrete | 1 |
| `PlatformWindowHost::set_window_long` | `00bed0a4` | concrete | 2 |
| `PlatformWindowHost::set_window_pos` | `00bed0c9` | concrete | 1 |
| `PlatformWindowHost::desktop_client_rect` | `00bed0f3` | concrete | 1 |
| `PlatformWindowHost::window_color_depth` | `00bed16b` | concrete | 1 |
| `PlatformWindowHost::show_window` | `00bed19c` | concrete | 1 |
| `Phase 3 configure_platform_window` | `00becee0` | concrete | 1 |
| `RendererHost::direct3d_create` | `00b32410` | concrete | 1 |
| `RendererHost::create_device` | `00b2aeb0` | concrete | 1 |
| `Phase 4 renderer_resources` | `00b14a10` | **unimplemented** | 1 |
| `Phase 5 input_settings` | `005547d0` | **unimplemented** | 1 |
| `Phase 5 sound_system_initialize` | `00a88770` | **unimplemented** | 1 |
| `Phase 6 locale_tables` | `00aa09d0` | **unimplemented** | 1 |
| `Phase 7 gui_startup` | `00aa06d0` | **unimplemented** | 1 |
| `Phase 8 world_effects_startup` | `00af0b10` | **unimplemented** | 1 |
| `Phase 9 game_entry` | `00740840` | **unimplemented** | 1 |
| `StartupHost::platform_run_loop_dispatch` | `00bec1a0` | concrete | 1 |
| `PlatformLoopCallbacks::pretranslate` | `00bec20a` | **unimplemented** | 2 |
| `ApplicationFrameHost::profiler_set_frame_slot_color` | `004c1dd0` | **unimplemented** | 60 |
| `ApplicationFrameHost::profiler_begin_frame_slot` | `00be3640` | **unimplemented** | 60 |
| `ApplicationFrameHost::game_state` | `00e188a8+5d4` | **unimplemented** | 120 |
| `ApplicationFrameHost::input_action_pressed` | `004c43c0` | **unimplemented** | 60 |
| `ApplicationFrameHost::advance_frame_clock` | `00bedc30` | concrete | 60 |
| `ApplicationFrameHost::frame_interval` | `00bee070` | concrete | 60 |
| `ApplicationFrameHost::game_on_move` | `004e4a40` | **unimplemented** | 60 |
| `ApplicationFrameHost::exit_requested` | `00e1ae75` | concrete | 60 |
| `ApplicationFrameHost::tick_vfs_providers` | `00bdb0b0` | **unimplemented** | 60 |
| `ApplicationFrameHost::update_loading_queue` | `004fde20` | **unimplemented** | 60 |
| `ApplicationFrameHost::profiler_end_frame_slot` | `00be3660` | **unimplemented** | 60 |
| `ApplicationFrameHost::profiler_end_frame` | `00be34d0` | **unimplemented** | 60 |
| `RendererHost::clear_and_present` | `milestone` | concrete | 60 |
| `ApplicationFrameHost::request_loop_exit` | `0109cf04+181` | concrete | 1 |
| `StartupHost::application_shutdown` | `00737f30` | concrete | 1 |
| `ApplicationShutdownHost::singleton_teardown` | `00737f80` | **unimplemented** | 1 |
| `StartupHost::application_destruct` | `008f8444` | concrete | 1 |
| `StartupHost::destroy_singleton_lifetime_manager` | `008f8449` | **unimplemented** | 1 |
| `StartupHost::close_mutex` | `008f846e` | concrete | 1 |
| `StartupHost::random_threads_unregister_current` | `00bd3050` | concrete | 1 |
| `StartupHost::release_thread_slot` | `00bf65ac` | concrete | 1 |
| `StartupHost::random_threads_shutdown` | `00bd30d0` | concrete | 1 |

Five host methods exist but were not reached by this run, because the paths that call them are
not taken: `StartupHost::game_explorer_verify_access` (008f82aa) and
`game_explorer_release` (008f82c4) need the Games Explorer interface;
`StartupHost::exit_process` (008f82f0) is the parental-controls denial;
`StartupHost::resolve_language` (008f7db0) and `error_message_box` (008f83c5) belong to the
already-running branch, which needs a second instance. `resolve_language` is concrete: it
reads the options file under `CSIDL_PERSONAL` through `bsp::PhysicalFile`, scans its tokens
with `startup_language_from_options_tokens`, and falls back to the registry LCID only when the
file cannot be opened, as 008f7db0 does.

## Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.

`bsp_game.exe --frames 60 --log local/game_run.log` exits 0 with:

```
summary window_created=1 device_created=1 device_hr=0x00000000 back_buffer=640x480
        frames_presented=60 loop_finished=1 exit_code=0
device created hr=0x00000000 flags=0x44 size=640x480 windowed=1 format=21 depth=75
```

`flags=0x44` is `D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING`, format 21 is
`D3DFMT_A8R8G8B8` and depth 75 is `D3DFMT_D24S8`, matching the parameters
`d3d9_create_device_prefix_00b2aeb0` stores. `--frames N` makes the run headless-testable: the
frame callback requests loop exit once N frames are presented, and `WinMain` returns 1 if
fewer than N were presented, so the exit code alone validates a run.

The close path was validated the same way, by sending WM_CLOSE to the running process instead
of by hand: `bsp_game.exe --log local/game_close.log`, then `CloseMainWindow()` from the shell.
The run presented 25450 frames, then recorded `CloseRequestPolicy::front_end_branch [004ca2f0]`
and `request_loop_exit [0109cf04+181]` once each, finished the loop and exited 0. That is the
one host method reached by the close run and not by the frame-limited run.

This is a runtime-validated process, not a game-validated one. It proves the spine executes
and presents; it proves nothing about asset loading, gameplay, or binary compatibility with the
original executable.

## Next milestones

1. **Title page.** Phase 5 settings 008d8190 from the registry so the window uses the
   configured resolution, phase 6 locale tables 00aa09d0, phase 7 GUI startup 00aa06d0 with
   the font registry and the GUI layer draw, and the logo sequence that game state 1 selects.
   The frame then draws a layer instead of a cleared buffer.
2. **Main menu.** Phase 9 game entry 00740840 into game state 2, the front-end screen sets, and
   the input edge test at 004c43c0 through the input polling the input packet connected, so the
   menu responds to a keypress.
3. **Supporting work both depend on.** Phase 2, the VFS provider manager 00beda60 and the
   package mounts 0073cb10: until those run, no asset the GUI needs can be read, and phase 4
   renderer resources 00b14a10 has nothing to load.
