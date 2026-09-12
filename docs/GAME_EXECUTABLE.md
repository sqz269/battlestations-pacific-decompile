# bsp_game.exe, milestones 1 through 2m

Milestone 2m is the current state of the executable, and its section corrects the earlier
ones. Milestone 1 is the spine it was all built on.

Addresses added by milestone 2a: 0073d604-0073d899 (the phase-2 VFS block of Init), 00beda60
(provider manager), 004fc150 / 00736a90 / 00736b60 (the three provider factory singletons),
00be0660 (factory registration), 00be1890 (mount), 0073cb10 with the two call sites 0073d881
and 0073d888 (package scan), 00738360 (resource search paths), 0073d94f-0073d98d (the factory
tail), 0073db41-0073db69 (phase 6 parsers), 008d8190 (settings load) and 008d5150 (options
path). Packet `game_executable_milestone_2a`, owner `agent/cc-game-vfs`. Sources:
`src/game_hosts_vfs.cpp`, `include/bsp/game_hosts_vfs.hpp`, plus the milestone-1 files.
Report: `reports/game_executable_milestone_2a.json`.

## Milestone 1

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
   game entry are recorded as unimplemented phases. **Milestone 2a moves the VFS phase and the
   settings load into the concrete list, and corrects the position of the command-line parse
   and the hardware probe; see the milestone 2a section.**
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
at 008d841f when a parsed resolution is not in the supported table. **Superseded by milestone
2a**: phase 5 now runs, and the window opens at the resolution the options file selects.

Two details are milestone additions rather than recovered behaviour, and are marked as such in
the source:

- The clear colour and the `Clear` / `BeginScene` / `EndScene` / `Present` sequence. The native
  renderer frame routine behind renderer virtual +20h is not reconstructed; this is the
  milestone's own present, recorded in the log with the address `milestone`.
- `game_window_procedure` implements only the WM_CLOSE branch of 00bed3b0 and forwards
  everything else to `DefWindowProcA`. The activation, resize, input and sizing-loop branches
  are not reconstructed, and the native handler's window-extra storage is replaced by one
  process-wide platform pointer because that layout is not recovered.

## Milestone 2a: the virtual file system and the settings

Milestone 1 recorded phase 2 and phase 5 as two unimplemented phases, which is why the title
page and the main menu could not be started: no asset could be read and the window size was a
constant. Milestone 2a makes both phases real, over the VFS types the archive and mount
packets already reconstructed. It still adds no reconstruction of its own; the two integration
bindings live in `src/game_hosts_vfs.cpp`.

### Phase 2, 0073d604-0073d899

`GameVfsHost` implements `bsp::VfsStartupHost`, so the recovered sequence in
`src/vfs_startup.cpp` drives the run rather than the process re-deriving the order. What is
concrete: the provider manager (`bsp::VfsProviderManager` over `bsp::VfsProviderFactories`,
which is the physical, FileStore and MPKG factories in registration order), the system-path
buffer from `GetCurrentDirectoryA` at 0073d697, the three 00be1890 mounts with the priorities
and ownership bytes of `kVfsStartupMounts`, the two 0073cb10 package scans, and the search-path
registration 00738360. The manager lives for the whole run, so later milestones read through
the same mounts startup created.

| Mount | Virtual path | Priority | Ownership | Native site |
| --- | --- | --- | --- | --- |
| current directory | `.` | 0 | 1 | 0073d6f9 |
| current directory | `persistent_data` | 99 | 1 | 0073d792 |
| `filestore` | `.` | 300 | 0 | 0073d829 |

Three corrections to milestone 1's ordering, all taken from the native listing. The hardware
probe 0073c3b0 is called at 0073d610, inside the phase-2 first-time gate, not before it. The
command line is parsed at 0073d94a, after the whole phase-2 block, which is why `cachedload`
cannot influence any phase-2 mount. The factory tail 0073d94f-0073d98d and the phase-6 parser
registrations therefore follow the parse, in that order.

What is still unimplemented in phase 2: the hardware probe 0073c3b0, the `.mpak` factory
00736b60 and its registration, the PAK registry 00736c30 with the manager store 00bd9230 and
the lock 00bb40b0, and the whole of phase 6 (the resource manager 004c1400 and the two parsers
00736dd0 / 00736ea0 through 00b80a50). `cachedload` is carried and reported by
`set_manager_cached_load_00bd9f90`, but the reconstructed manager has no cached-load slot, so
the flag reaches no provider yet. The two manager handlers stored at 0073d642 and 0073d652 are
recorded with their addresses: both targets are a single `C3`, so storing them cannot change
behaviour.

The mount system path is whatever `GetCurrentDirectoryA` returns, exactly as the original
reads it. `--game-root <dir>` calls `SetCurrentDirectoryA` before Init, so pointing a run at an
installed game moves the process rather than injecting a path into the recovered call.

The installed copy this milestone was validated against ships no `.mpkg` archives: its data is
loose in the install tree. Both package scans enumerate successfully and find zero entries, and
the loose files resolve through the priority-0 physical mount. A run against an installation
that does ship archives will mount them through the same callbacks, with
`package_mount_priority_0073cb10_fragment` supplying the 1000-or-`patch`+suffix priority.

### Phase 5, 008d8190 at 0073daa5

`GameSettingsBinding` implements `bsp::GameSettingsHost`. The options file is read from
`SHGetSpecialFolderPathA(CSIDL_PERSONAL)` joined with the per-title directory and
`options.txt`, through `bsp::PhysicalFile`; the registry language value, the desktop size and
the path-B fallback are the native reads. The load runs before window creation at 0073dc0f,
which is the ordering constraint the phase exists for: arguments 7, 8, 3, 4 and 9 of 00becee0
are read straight out of the settings block, and argument 4 is VSync while argument 9 is the
antialias sample count (the two corrections at the end of `docs/APP_INIT_PLATFORM.md`).

Two capability tables have no reconstructed source, because both come from the renderer vector
that phase 4 would fill:

- The supported-resolution table `DAT_00f8895c`, assigned at 008d81bf from renderer+1Ch. An
  empty table makes 008d8190 reject every parsed resolution and fall back to 640x480, so the
  milestone supplies the adapter's own `EnumAdapterModes` list. **This is a milestone
  addition, not recovered behaviour**, marked as such in the source. The same interface
  supplies the shader-model ceiling `TRIV_body_00b200b0` returns.
- The antialias level table `DAT_00f88968`. It is left empty, which is exactly the no-snapping
  branch of the loader tail, so the file's `Antialias` value survives unchanged. An invented
  list would silently move the sample count.

A defect in the shared reconstruction, found here and not fixed here because
`src/app_bootstrap.cpp` belongs to another packet: the native token comparison at 008d8190 runs
through `FUN_00467cc0`, which calls `BSP_CString_CompareInsensitive`, so token names are
matched case-insensitively. `apply_options_token` compares them with `==`. The game's own
writer 008d6170 emits `Vsync ` while the reader literal at 00d15ef4 is `VSync`, so a file the
game wrote does not round-trip through the reconstruction. `GameSettingsBinding` canonicalizes
recognized token spellings before handing the text to the loader and logs every token it had
to recase; the validation run recased exactly one, `Vsync -> VSync`. The fix belongs in
`apply_options_token`.

VSync and the antialias count reach the renderer init request that 00becee0 builds, and the
run log records them there, but `d3d9_create_device_prefix_00b2aeb0` models neither
`PresentationInterval` nor `MultiSampleType`, so they stop at that boundary and the device is
created with the recovered constants.

### What changed on screen

The captioned window is no longer a constant 640x480. It opens at the resolution the options
file selects, windowed or fullscreen as the file says, and the back buffer follows: the
validation machine's `options.txt` asks for 2560x1440 windowed, and both the window and the
device report that size. Nothing new is drawn inside it. The frame is still the milestone's own
`Clear` / `BeginScene` / `EndScene` / `Present`, because the renderer resource phase 00b14a10,
the GUI startup 00aa06d0 and the game entry 00740840 are all still unimplemented. What changed
behind the window is that assets can now be read: the run resolves and reads real files out of
the installed game through the mounted providers.

### What the executable now does

`--game-root <dir>` enters an installed game before Init. `--vfs-probe <virtual path>` is
repeatable and resolves one path through 00bdf4c0 then opens it through 00bdf310, printing its
byte count; a probe that reads nothing makes the process exit 3, so a scripted check needs only
the exit code. Every run also logs each mount with its priority and ownership, the two package
scans with their entry counts and dispositions, the settings values applied, and three fixed
probes: a GUI script, the locale table the settings language selects, and one texture.

## Host methods

Generated from the run log of
`bsp_game.exe --frames 60 --log local/game_run.log --game-root "<install>"`:
54 concrete, 31 unimplemented, 85 distinct methods and phases reached. Milestone 1 was
43 concrete and 25 unimplemented over 68.

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
| `Phase 2 probe_hardware` | `0073c3b0` | **unimplemented** | 1 |
| `Phase 2 construct_provider_manager` | `00beda60` | concrete | 1 |
| `Phase 2 install_manager_handlers` | `0073d642` | concrete | 1 |
| `Phase 2 file_store_factory` | `004fc150` | concrete | 1 |
| `Phase 2 register_provider_factory` | `00be0660` | concrete | 2 |
| `Phase 2 mpkg_factory` | `00736a90` | concrete | 1 |
| `Phase 2 current_directory` | `0073d697` | concrete | 1 |
| `Phase 2 mount_system_path` | `00be1890` | concrete | 3 |
| `Phase 2 mount_packages` | `0073cb10` | concrete | 2 |
| `Phase 2 register_resource_search_paths` | `00738360` | concrete | 1 |
| `Phase 1 parse_command_line` | `0073ce20` | concrete | 1 |
| `Factory tail mpak_factory` | `00736b60` | **unimplemented** | 1 |
| `Factory tail register_provider_factory` | `00be0660` | **unimplemented** | 1 |
| `Factory tail pak_archive_registry` | `00736c30` | **unimplemented** | 1 |
| `Factory tail set_manager_pak_registry` | `00bd9230` | **unimplemented** | 1 |
| `Factory tail set_manager_cached_load` | `00bd9f90` | concrete | 1 |
| `Factory tail create_pak_registry_lock` | `00bb40b0` | **unimplemented** | 1 |
| `Phase 6 resource_manager` | `004c1400` | **unimplemented** | 2 |
| `Phase 6 animation_channels_parser` | `00736dd0` | **unimplemented** | 1 |
| `Phase 6 register_type_parser` | `00b80a50` | **unimplemented** | 2 |
| `Phase 6 bone_parser` | `00736ea0` | **unimplemented** | 1 |
| `Phase 3 construct_win32_platform` | `00becda0` | concrete | 1 |
| `SaveStorageHost::special_folder_path` | `00beb2f3` | concrete | 1 |
| `SaveStorageHost::create_directory` | `00beb35e` | concrete | 2 |
| `Phase 3 initialize_save_storage` | `00beb2c0` | concrete | 1 |
| `Phase 5 load_game_settings` | `008d8190` | concrete | 1 |
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
| `PlatformLoopCallbacks::pretranslate` | `00bec1d8` | **unimplemented** | 8 |
| `ApplicationFrameHost::profiler_set_frame_slot_color` | `004c1dd0` | **unimplemented** | 60 |
| `ApplicationFrameHost::profiler_begin_frame_slot` | `00be3640` | **unimplemented** | 60 |
| `ApplicationFrameHost::game_state` | `00737acc` | **unimplemented** | 120 |
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

Milestone 2a run, `bsp_game.exe --frames 60 --log local/game_run.log --game-root "<install>"`,
exit 0:

```
mount <install>\ -> "." priority=0 ownership=1 device=-1 created
mount <install>\ -> "persistent_data" priority=99 ownership=1 device=-1 created
mount filestore -> "." priority=300 ownership=0 device=-1 created
package scan 1 enumerated=1 entries=0 complete=1
package scan 2 enumerated=1 entries=0 complete=1
options token recased Vsync -> VSync
Options: unknown token HardwareReported
settings resolution=2560x1440 index=0 fullscreen=0 vsync=1 antialias=0 shader_model=2
         language=englishauthentic
vfs probe interface/_common.lua       resolved=1 opened=1 bytes=8499
vfs probe lockit/englishauthentic.lng resolved=1 opened=1 bytes=80
vfs probe effects/a_fiji_terr_atl.dds resolved=1 opened=1 bytes=2796336
window created 2560x1440 at 0,0 color_depth=32
summary window_created=1 device_created=1 device_hr=0x00000000 back_buffer=2560x1440
        frames_presented=60 loop_finished=1 exit_code=0
summary vfs_ready=1 mounts=3/3 package_entries=0 package_mounts=0 cachedload=0 probes=3/3
host methods 54 concrete, 31 unimplemented
```

`--vfs-probe` was checked both ways: `--vfs-probe fonts/fonts.lua` reads 1115 bytes and exits
0, `--vfs-probe does/not/exist.lua` exits 3.

Milestone 1's run, for comparison:

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

This is a runtime-validated process, not a game-validated one. It proves the spine executes,
reads real assets out of an installed game and presents; it proves nothing about gameplay or
binary compatibility with the original executable. In particular the mounted providers were
exercised against a loose installation only: no `.mpkg` archive was mounted by either package
scan, so the MPKG path through the same callbacks is still unexercised here.

## Milestone 2b: fonts, the GUI startup phase and the title pages

Addresses: 0073bae0 (the fonts and GUI bring-up 0073e13c calls), 007371d0, 00ac3910,
0053bc00 / 00be9620 / 00be9760, 004c12b0, 00aa5d70, 00aa5e20, 00aa5840, 00aa7e00, 00ac6600,
00aaa710, 00aeeaf0, 00a9ec70, and 00aa09d0 / 00aa06d0 for the correction below. Packet
`cc_exe_2b`, owner `agent/cc-exe-2b`. Sources: `src/game_hosts_frontend.cpp`,
`include/bsp/game_hosts_frontend.hpp`, plus edits to `src/game_hosts.cpp`,
`src/game_hosts_fonts.cpp`, `src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2b.json`. Ghidra was read-only for this packet.

Milestone 2a ended with a cleared back buffer: the assets could be read but nothing used
them. Milestone 2b runs the fonts and GUI half of `BSP_Application_Initialize` and loads the
GUI pages the title bring-up needs, so the process now evaluates real page scripts, builds
real widget trees, and draws them.

### Phase 7, 0073bae0 at 0073e13c

`GameFrontendHost` implements `bsp::GuiStartupHost`, so the recovered sequence in
`src/gui_startup.cpp` drives the phase rather than the process re-deriving its order. The
font work was split out of `GameFontHost::initialize` into
`load_descriptors_00ac3910` and `preload_fingerprint_payload_00be9760` for exactly that
reason: `run_gui_startup` calls them at the two points `0073bae0` calls them, with the
fingerprint preload between the descriptor load and the GUI manager, not merged into it.

What one run performs, in that order:

| Step | Native site | Result on this installation |
| --- | --- | --- |
| language font path | 008d4890 | empty, as the shipped English descriptor has no `fontpath` |
| font registry | 007371d0 | the process's own registry object |
| descriptor load | 00ac3910 | 6 fonts, 19 VFS resource opens |
| fingerprint preload | 0053bc00 / 00be9760 | 240 defined bytes out of `fonts/arial19.dat` |
| GUI manager | 004c12b0 / 00aa5d70 | created once, before the `After InitGui` checkpoint |
| resource list | 00aa5e20 | 9 of 10 entries acquired, 9 stores, byte +84h cleared |

The six fonts are `ViperTitle`, `Arial16`, `Viper19`, `Arial20`, `Arial15` and `Arial18`,
each with its data file, glyph sheet and the `white.tga` alpha texture resolved and loaded;
`Viper19` and `Arial16` are the two the title pages ask for. `Fonts\white.tga` resolves to
`effects/white.dds` through the VFS search paths, which is what makes the shared alpha
texture work at all.

The one entry that comes back null is `data/interface/textures/whiteGui.tga`, which the VFS
rejects on this installation. That is the `+28h` null the native gate at 00aa5e88 allows, so
the sequence continues. The other nine are real: `interface/textures/common/transparent.tga`
loads, and the two group entries are GUI **pages**, not widgets built in code.

### The two group entries are pages, and 00aa7e00 finds rather than creates

`_Mouse` loads `interface/_mouse.lua` and `_Highlight` loads `interface/_highlight.lua`,
both at Priority -1000, through the reconstructed page loader 00aa5840 and the per-page Lua
evaluation 00ac6600. All six child entries (`MousePtrFE_Icon`, `MousePtrGUI_Icon`,
`hl_FrameBox`, `hlCircle_FrameBox`, `safezone_43_FrameBox`, `safezone_169_FrameBox`) are then
found as direct children of those pages' roots. This is the run-time confirmation of the
correction `docs/GUI_LAYOUT_LOADER.md` added to `docs/APP_INIT_FONTS_GUI.md`: 00aa7e00
creates nothing.

### The title pages

`docs/GAME_TITLE_INIT.md` establishes what `GGame::OnInitTitle` brings up: `00518250` loads
`FE_frame` and `FE_frame_title` for frame sets 0 to 2, and the title screen's activate loads
`FE_initial`. The executable loads those three directly after Init. It does **not**
reconstruct the front-end state machine that would request them; that is packet
`cc_frontend_states`, and `Title bring-up GGame::OnInitTitle [004c9a70]` is one unimplemented
host record standing for the whole of it.

Each page gets a private Lua 5.1 state with base, table, string and math (mask 0x65), runs
`interface/_Common.lua` and then `interface/<page>.lua`, and its global `GuiScreen` table is
snapshotted and walked by 00aaa710. Two things make that work on the installed data:
`_Common.lua` resolves case-insensitively to the shipped `_common.lua`, and
`scripts/fundamentals.lua` supplies `Platform()`, which the bootstrap's `PC=true` resolves,
so `["Visible"] = Platform(true,false)` reaches the snapshot as a boolean. No function value
ever lands in `GuiScreen`, which is what the snapshot would reject.

The five pages produce 29 widgets, 22 of them carrying an authored texture:

| Page | Priority | Widgets | What it is |
| --- | --- | --- | --- |
| `_Mouse` | -1000 | 11 | the two cursors and the nine-piece scaling cursor |
| `_Highlight` | -1000 | 5 | the selection frame boxes and the two safe-zone boxes |
| `FE_frame` | 0 | 6 | the flag, the top and bottom rails and their shadows |
| `FE_frame_title` | 0 | 3 | the winged title plate and the menu title text |
| `FE_initial` | 0 | 4 | the Pacific map background, the logo and the press-start text |

Every widget is logged with its key, its type from the key suffix (00aa2490), its resolved
position through the parent chain (00aa6750), its size pair, its authored visibility and its
texture or material. For an `Icon` the texture comes from the reconstructed authored reader
00ab3310; for the other types it is the first `States` entry's `Texture`, and the material is
`ShaderName` or, for a `Text`, `Font`.

The scene-graph half has no reconstruction and is recorded as such: `create_widget_node`
(00b75030) and `set_node_parent` (00b6e680) run 24 times each, `create_page_root_node`
(00aa6720) and `clear_page_root_list` (00b6d890) five times each, and the two widget vtable
hooks +74h and +78h 24 times each. They hand back an incrementing diagnostic id, which is
never zero because 00aa7e00 skips a child whose node pointer is null.

### The locale phase now states a lookup, not just a count

Phase 6 loads `lockit/englishauthentic.lan` and 6856 keys. The run now names the table it
loaded and resolves three ids through 00a9ec70: `globals.pleasewait` gives "Please Wait" and
`globals.newplayer` gives "New Player", while `FE.init_legal` (the id `fe_initial.lua` gives
its copyright text) is present but empty in this installation's table.

### The sprite bridge, which is not a reconstruction

The native GUI draw path belongs to other owners, so the widgets are drawn by an
executable-side Direct3D 9 bridge in `src/game_hosts_frontend.cpp`, installed as the overlay
the milestone's own `Clear` / `BeginScene` / `EndScene` / `Present` runs. Two textured
triangles per widget, `D3DFVF_XYZRHW` with alpha blending, textures created by
`D3DXCreateTextureFromFileInMemoryEx` over bytes the mounted VFS read, and sub-rectangles
from the reconstructed atlas parser 00aeeaf0. **It is labelled a bridge in the source and the
header, and nothing about its drawing is recovered behaviour.** Three of its decisions are
its own and are not claims about the game:

- It parses all twelve atlas descriptors the installation ships under `interface/textures`
  and merges their 678 items into one list. `GGame::OnInitTitle` loads exactly one,
  `interface/textures/allbutingame.ats`, and the native VFS content-suffix list (manager
  +48h/+4Ch) would turn that one name into the DXT variant a machine ships. The
  reconstructed startup never populates that list, so `allbutingame.ats` does not resolve and
  the variants are enumerated instead.
- It draws a widget whose page authored no `Visible` key. The projected default at +E4h is
  false; in a running game the value is pushed by 004f83b0, which reads the owning screen's
  active byte (+5h, `MOVZX EDX,[EBX+5h]` at 004F8434) and forwards it into every non-null
  child its +24h collector reports through the child's vtable +34h, one level deep (whether
  it recurses depends on each handler); the pump's exit pass clears the byte at 004F88B3 and
  calls the same routine at 004F88B7, so it publishes current visibility rather than showing
  (docs/FRONTEND_STATE_MACHINE.md, docs/MAIN_MENU_PATH.md). The title handover 0068d8d0
  leaves the press-start screen at +5h = 1 with 004f83b0 already called, so FE_initial's
  widgets receive true once; FE_frame and FE_frame_title come from the 0x164 layout-set
  singleton at 004c1ac0 rather than a registry screen, so "always visible" for those two is
  unverified. In this process no front-end screen owns the five directly loaded pages, so
  the screen-level term has no input and the bridge's rule stands in for it. A widget the
  page authored as hidden, or one 00aa5e20's
  virtual +34h calls hid, stays hidden, and so does its subtree: that is why the cursors and
  the highlight boxes do not appear.
- It orders quads back to front by the authored Z alone. The native render order
  (`RenderOrder`, `geOrder`) is another owner's.

`drawn: yes`. 13 textures and 6 quads, presented on all 60 frames of the frame-limited run.
Text widgets draw nothing: they carry a font, not a texture, and glyph drawing is the font
owner's.

### What it looks like on screen

The window opens at the options file's resolution and now shows the title page: the Pacific
theatre map as the background, the US flag and the top and bottom rails of the front-end
frame, the winged `BATTLESTATIONS` title plate and, behind it, the red Pacific logo. The
capture was taken from the client area of a running window, not a mock-up; it is written to
the ignored `local/title_page.png` and is not committed.

The logo and the title plate overlap because all three pages are loaded at once and nothing
chooses between them. In the game, the front-end screens decide which page is active and push
their own visibility byte down through 004f83b0; that is packet `cc_frontend_states`. The
installed frame texture reads `MIDWAY MODDERS` because this installation is modded.

### Host methods

`bsp_game.exe --frames 60 --log local/game_run.log --game-root "<install>"`, exit 0:
**81 concrete, 36 unimplemented, 117 distinct methods**. The same tree before this packet ran
67 concrete and 28 unimplemented over 95, so 2b added 22 methods, 14 of them concrete. A run
closed with `CloseMainWindow` instead of a frame limit reports 82 concrete, because it also
reaches `CloseRequestPolicy::front_end_branch` (004ca2f0).

The methods this packet introduced:

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `Phase 7 fonts_and_gui_startup` | `0073bae0` | concrete | 1 |
| `GuiStartupHost::language_font_path` | `008d4890` | concrete | 1 |
| `GuiStartupHost::font_registry` | `007371d0` | concrete | 1 |
| `GuiStartupHost::load_font_descriptors` | `00ac3910` | concrete | 1 |
| `GuiStartupHost::preload_fallback_glyph_table` | `0053bc00` | concrete | 1 |
| `GuiStartupHost::gui_manager_get_or_create` | `004c12b0` | concrete | 1 |
| `GuiManagerResources::renderer_load_texture` | `00aa5e60` | **unimplemented** | 2 |
| `GuiManagerResources::load_page` | `00aa5840` | concrete | 2 |
| `GuiManagerResources::find_child` | `00aa7e00` | concrete | 6 |
| `GuiManagerResources::set_visibility` | `00aa5faf` | concrete | 8 |
| `GuiManagerResources::clear_ready_flag` | `00aa6305` | concrete | 1 |
| `TitlePages::load_page` | `00aa5840` | concrete | 3 |
| `GuiLayoutHost::vfs_name_exists` | `00aa58c3` | concrete | 5 |
| `GuiLayoutHost::widescreen_enabled` | `00aa8750` | concrete | 29 |
| `GuiLayoutHost::create_page_root_node` | `00aa6720` | **unimplemented** | 5 |
| `GuiLayoutHost::create_widget_node` | `00b75030` | **unimplemented** | 24 |
| `GuiLayoutHost::set_node_parent` | `00b6e680` | **unimplemented** | 24 |
| `GuiLayoutHost::widget_vtable_74` | `00aaade8` | **unimplemented** | 24 |
| `GuiLayoutHost::widget_vtable_78` | `00aaae5b` | **unimplemented** | 24 |
| `GuiLayoutHost::derived_property_reader` | `00aaa710` | **unimplemented** | 29 |
| `GuiLayoutHost::clear_page_root_list` | `00b6d890` | **unimplemented** | 5 |
| `Phase 6 locale_lookup` | `00a9ec70` | concrete | 2 |
| `Title bring-up GGame::OnInitTitle` | `004c9a70` | **unimplemented** | 1 |

`GuiLayoutHost::instantiate_page_model` (00aa58cc) exists and was not reached: none of the
five pages has a `<name>.mmod`, so every one took the plain-root branch.

### Corrections

1. **The phase-7 label in the milestone 2a host table is wrong.** That table lists
   `Phase 6 locale_tables [00aa09d0]` and `Phase 7 gui_startup [00aa06d0]`. 00aa06d0 is
   `BSP_Localization_ReloadTables`, a localisation routine: both it and 00aa09d0 belong to
   the locale half of `BSP_Application_Initialize` at 0073e06a-0073e135. The GUI startup is
   0073bae0 at 0073e13c, which creates the GUI manager (004c12b0) and runs its resource list
   (00aa5e20). The follow-up note at the end of this file already said the old `gui_startup`
   label referred to locale reload; this states which address the phase actually is.
2. **The "54 concrete, 31 unimplemented" figure in the milestone 2a section is stale.** The
   settings, input-script, locale and font owners that landed between 2a and this packet took
   the same run to 67 and 28 before 2b began.
3. **Milestone 2a's next-milestone item 1 ("Title page") is superseded by this section**, in
   part: the locale tables, the font registry and phase 7 are done and a title page is drawn,
   but through a bridge rather than the GUI layer draw, and without the front-end state
   machine that selects the page.
4. `docs/APP_INIT_FONTS_GUI.md`'s original reading, that `00aa5840` and `00aa7e00` are "two
   widget factories" creating the ten resources, is confirmed wrong at run time, as its own
   appended correction from `docs/GUI_LAYOUT_LOADER.md` says: 00aa5840 loads a page from
   `interface/<name>.lua` and 00aa7e00 finds an existing direct child.
5. **`PlatformLoopCallbacks::pretranslate` cited the wrong address.** The milestone 1 table
   gave 00bec20a. Reported by packet `cc_frontend_states` and verified here against the
   image: 00bec20a is `MOV byte ptr [ESI+0x43],1`, the loop-finished store in the epilogue of
   00bec1a0 at 00bec209-00bec214, reached once per process. The pretranslation call is
   `CALL 00c2f1d2` at 00bec1d8, inside the PeekMessageA success arm (00bec1cd `CALL EDI`,
   `TEST EAX,EAX`, `JZ 00bec1f1`), and its result at 00bec1dd gates the translate/dispatch
   pair. 00c2f1d2 is `JMP dword ptr [00ce25dc]`, the import thunk for
   `XLivePreTranslateMessage` (ordinal 5030). The 8 calls the 60 frame run logged are 8
   messages, which only the in-arm call can produce. The table, the run log and
   `GameLoopCallbacks::pretranslate` now cite 00bec1d8; the behaviour, returning false with
   no XLive library bound, was already right.
6. **`ApplicationFrameHost::game_state` is a field read, not a call.** The milestone 1 table
   gave `00e188a8+5d4`. Reported by packet `cc_frontend_states` and verified here: 00737acc
   is `MOV ECX,dword ptr [00e188a8]` followed by `MOV EAX,dword ptr [ECX+0x5d4]`, and
   00737b33 repeats the same pair. 00e188a8 is the GGame singleton pointer and 5D4h is a
   field inside the object it points at, so the count of 120 over 60 frames is one field read
   twice per frame rather than two call sites. The citation is now 00737acc. It stays
   unimplemented here because the executable has no GGame object; see the follow-ups.

### Code with no Ghidra function

None. Every address this packet touched already has a Ghidra function and a reviewed ledger
name; the packet appended run-time evidence to 0073bae0, 004c12b0, 00aa5e20, 00aa5840,
00aa7e00, 00ac6600 and 00aeeaf0 rather than adding names.

### Follow-up packets

1. **The GUI layer draw**, so the sprite bridge can be deleted. The renderer's GUI draw path
   and the widget geometry writers (`src/gui_native_geometry.cpp`, `src/gui_render_order.cpp`,
   `src/font_geometry.cpp`) already exist as reconstructions; nothing binds them to a device.
2. **The front-end screens**, packet `cc_frontend_states`: which page is active, and the
   visibility push through 004f83b0 that the bridge currently substitutes a rule for.
3. **Text drawing.** `title_Text` and `press_start_Text` resolve their font and their string
   id today and draw nothing. The font geometry and the GUI text widget are reconstructed;
   the missing piece is the same device binding as item 1.
4. **The renderer texture entry point** `*(00f8d394)` virtual +64h and the atlas registry
   00aef280, so `whiteGui`, `transparent` and every page texture come from the native path
   instead of the bridge's D3DX loader.
5. **The VFS content-suffix list** at manager +48h/+4Ch. It is why `allbutingame.ats` does not
   resolve to `allbutingame_dxt1.ats`, and it is the same mechanism `00bdef90` uses for Lua
   script overrides, which the executable already carries as an empty list.
6. **The scene graph**: 00b74eb0 / 00b75030 / 00b6e680 and the two widget vtable hooks, the
   largest remaining unimplemented group in the run.
7. **The frame's game state and the profiler**, once packet `cc_frontend_states` merges.
   `include/bsp/app_frame_game_state.hpp` on that branch supplies `bsp::GameStateSlot`,
   `read_game_state_00737acc` and `apply_drained_game_state` (the drain's store at 004e449e),
   which turn `ApplicationFrameHost::game_state` into a real field read over a slot that
   `bsp::drain_state_requests_004e4430` advances; its writers are 004e3ac2, 004c9a70, 004e449e
   and 004e4279. The same header reconstructs the four profiler host methods (00be3260,
   00be3640, 00be3660, 00be34d0) over caller-owned arrays with one host method for
   `QueryPerformanceCounter` at 00ce2270. That would move six of this run's unimplemented
   records to concrete without any new analysis. It is not done here because the header lives
   on another worker's branch and is not in this worktree.

## Milestone 2c: the frame's game state, the Init tail, the title pump and the main menu

Addresses: 00737acc / 00737b33 (the game-state field), 004c1dd0 / 00be3260 / 00be3640 /
00be3660 / 00be34d0 / 00ce2270 (the profiler counter pair and its clock); 0073c3b0 with
0098d4e0 / 0098d470 / 0098d570 / 0098d590 / 0098d5c0 / 0098d5f0 and the detection object
009927d0 (the phase-2 hardware probe), 00736b60 / 00be0660 / 00736c30 / 00bd9230 / 00bb40b0
(the provider factory tail), 004c1400 / 00736dd0 / 00736ea0 / 00b80a50 (the phase-6
parsers), 0073fa70 / 00af06a0 / 00740410 / 0073e9a0 / 00740840 (the phase-9 decal table);
004c9a70 / 00518250 (the title bring-up), 0068d8d0 / 0068d850 / 0068d8a0 (the title
handover), 004f7180 / 004f71d0 / 004f71a0 / 004f71f0 / 004f8830 / 004f83b0 / 004f7620 (the
screen registry, its pump and its commit), 0067c840 / 0067ca80 / 0067cb40 / 0067c870 /
0067d860 / 0067cfb0 (the press-start screen), 004d7920 / 004e4430 (the state-request ring),
004e4000 (the shell entry), 004cc460 / 00684600 / 00684700 / 00683aa0 / 00683e90 / 006840f0
/ 00686380 / 005884a0 (the front-end managers), 004f8710 / 004d8c00 (the level-4 sets),
004c40f0 and 004c43c0. Packet `cc_exe_2c`, owner `agent/cc-exe-2c`. Sources:
`src/game_hosts_menu.cpp`, `include/bsp/game_hosts_menu.hpp`,
`src/game_hosts_init_tail.cpp`, `include/bsp/game_hosts_init_tail.hpp`, plus edits to
`src/game_hosts.cpp`, `src/game_hosts_vfs.cpp`, `src/game_hosts_frontend.cpp`,
`src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2c.json`. Ghidra was read-only for this packet.

Milestone 2b drew the title page by loading three GUI pages directly and guessing their
visibility. This milestone deletes both approximations: the pages are owned by front-end
screens, the pump decides which screen is up, and the recovered visibility push publishes
the answer. It also makes the four Init phases that were still records real, and it adds an
injected keypress so the whole press-start-to-main-menu path runs in a headless run.

### 1. The frame's game state and the profiler

`include/bsp/app_frame_game_state.hpp` supplies `bsp::GameStateSlot`, so
`ApplicationFrameHost::game_state` is now the field read `00737acc` performs rather than a
constant. The executable owns that one integer and every recovered writer reaches it: the
title bring-up writes 2, the shell entry writes 3 and then 5, and the drain's store at
004e449e writes whatever it popped. The frame still reads it twice per frame, so the call
count is two per frame exactly as the native body produces.

The four profiler methods run `bsp::profiler_begin_frame_slot_00be3640`,
`profiler_end_frame_slot_00be3660`, `profiler_set_slot_color` and
`profiler_end_frame_00be34d0` over arrays the process owns, with one host method for the
indirect `QueryPerformanceCounter` at 00ce2270. Two values are the milestone's own and are
marked as such in the source, because the image does not carry them:

- The slot capacity. 00be3820 sizes all six allocations from `00e15118 + 32h`, and 00e15118
  is filled at run time, so the executable reserves eight slots.
- The tick scale at 0109db48, which is zero in the image. A zero scale makes every converted
  value non finite, so the run supplies seconds per performance-counter tick. The profiler's
  output unit is therefore this substitution's, not a recovered one.
- The PERF_APP_UPDATE slot index at 0109d014, also filled at run time by the counter
  registration 00408720. Slot 0 is skipped by both 00be3640 and 00be3660, so the frame uses
  slot 1.

### 2. The Init tail

**Phase 2, the hardware probe 0073c3b0 at 0073d610.** The registry half is concrete:
0098d5c0 and 0098d5f0 read `MemSize`, `CPUSpeed`, `GPUDeviceID` and `SoundDevice` from
`HKLM\SOFTWARE\Eidos\BSM_HWD` through 0098d4e0, demanding REG_QWORD for the three numbers
and REG_SZ for the name, and 0098d570 / 0098d590 write them back through 0098d470. The
machine half is the detection object 009927d0, which has no reconstruction (packet
`hardware_detection_object`), so its four accessors and the localized message table 00996060
are unimplemented records. The validation machine has no such registry key, so
`bsp::probe_hardware_0073c3b0`'s all-four gate returns immediately and
`run_hardware_probe_tail_0073c3b0` never runs; that is the only reason the message and the
write-back are unexercised here.

Two decisions in this phase are the milestone's own and are not claims about the game. The
`MessageBoxW` at 0073c827 is not raised and the registry write-back at 0073c843 is not
performed unless `--hardware-probe-commit` is given: an unattended run must not block on a
modal dialog or rewrite a machine's stored profile. Both are logged with the values the
native would have used.

**The provider factory tail, 0073d94f-0073d98d.** All five calls are now real. 00736b60
creates the 8-byte `.mpak` factory singleton and 00be0660 appends it to the manager's
factory list, which is a plain `push_back` with no deduplication; 00736c30 builds the 1Ch
PAK archive registry with `enabled = 1` and `provider_limit = 0x64`; 00bd9230 stores it in
the manager's provider-policy slot; 00bb40b0 publishes the process-wide lock. The factory's
Create 00bb83a0 is packet `mpak_provider_create`, so the MPAK factory is registered and
creates nothing.

**Phase 6, 0073db41-0073db69.** 004c1400 hands back one resource manager for both
registrations, as it must, and 00b80a50 registers the `AnimationChannels` and `Bone` parsers
into its map by the string their vtable slot +4h returns. The 0x28-byte manager 00b81040
builds is not reconstructed; the process holds the parser map at manager+8h, which is the
only field 00b80a50 touches. Slot +8h of each parser, 00b8a910 and 00b8a990, is packet
`resource_item_parsers` and records the gap.

**Phase 9, the decal table 00740840 at 0073de8c.** 0073fa70 publishes the decal system at
00e1aea0, then the loader opens its own Lua state, runs
`scripts/datatables/decals.lua` through the mounted VFS and walks the `Decals` global. The
run parses **3 definitions** and every field matches the installed file:

| Name | Size | Radius | Maxnum | Texture | Shader | LifeTime | FadeOutTime |
| --- | --- | --- | --- | --- | --- | --- | --- |
| machine_gun_decal | 0.33 | 2 | 130 | white.tga | decal.mshd | 15 | 2 |
| explosion_hole | 4 | 2 | 10 | white.tga | decal.mshd | 30 | 2 |
| landscape_hole | 5 | 2 | 100 | white.tga | decal.mshd | 30 | 2 |

The three resource loads the record constructor and the walk make (`decal.mvfm` through
renderer vtable +34h, the texture through +64h and the shader through +48h) are
unimplemented records, because the renderer at 00f8d394 is another owner's.

### 3. The title pump

`GGame::OnInitTitle` 004c9a70 now runs in process through `bsp::run_title_init`. It writes
game state 2 into the field the frame reads, selects front-end frame set 0 through 00518250,
which loads `FE_frame` and `FE_frame_title`, and activates the title screen. The activate
0068d8d0 builds the 0x18-byte press-start screen, registers it into registry slot 5Ch, sets
both flag bytes, commits through 004f83b0 and enters it; its register override 0067ca80 is
what loads `FE_initial`. Milestone 2b loaded all three pages directly; now each one has an
owner.

Every frame after that, the front-end branch of `GGame::OnMove` runs through
`bsp::run_front_end_state_frame`: the title object's update 0068d850 forwards to the
press-start screen's 0067cfb0, then the shared tail pumps the registry with 004f8830. The
native reaches 0067cfb0 twice per frame in state 2, once through 0068d850 and once through
the pump's update pass, and so does this; the 120 frame run records 62 calls over its 31
state-2 frames.

**The substitute visibility rule is gone for owned pages.** 004f83b0 publishes the screen's
applied byte +5h to every layout the screen's +24h collector reports, and the sprite bridge
now reads that byte. A page no screen owns keeps the 2b rule: that is the five 00aa5e20
resource-list pages and the two frame layouts 00518250 selects, which belong to the 0x164
singleton at 00e18d80 rather than to a registry screen. The run performs four commits: the
press-start screen on entry and on exit, and the main-menu screen on entry.

The press-start screen's own arms need the XenonSystemManager at 00f8abe8, which this
process does not build, so `sign_in_phase` and its neighbours are unimplemented records and
the idle arm only pulses the prompt. Its two concrete element calls are real: the prompt
widget's colour (vtable +50h) and its visibility (vtable +34h) reach the loaded `FE_initial`
widget every frame.

### 4. The main menu

`--press-start-frame N` injects the input edge the page waits on. The injection is
`bsp::start_action_00a92aa0` on one `bsp::InputActionRecord`, which is exactly the rising
edge `bsp::action_pressed_this_frame_004c43c0` reports; every other frame runs
`begin_action_frame_00a92370`, so the edge is one frame long. No input binding code was
touched, and the executable's action table holds that one record, so every other action index
has no record and 004c43c0's own gate skips it.

The path then runs as `docs/MAIN_MENU_PATH.md` orders it, through
`bsp::advance_main_menu_path`. One frame carries it from the press to game state 5:

| Step | Native | What the run did |
| --- | --- | --- |
| PressStartPoll | 0067D2DD, 004C43C0 | the injected edge on frame 30 |
| TitleSkip | 0068D8A0 | storage reset recorded, ring empty |
| RequestShellState | 0068D8BC, 004D7920 | state request 4 enqueued, hold byte cleared |
| DrainStateRequest | 004E4430 | 4 popped, stored through 004e449e |
| EnterShell | 004E4000 | ShellReady; 00686380 built the seven screen records |
| PushInterfaceRequest | 004E4259, 004CC460 | INTF_MAINMENU onto the pending record |
| ActivateManager | 004E4269, 00684700 | manager+3Ch set; 005884A0 recorded |
| PublishShellReady | 004E4279 | game+5D4h = 5 |
| ServiceInterfaceRequest | 004E5442, 006840F0 | see the correction below |
| ApplyInterfaceRequest | 00685826, 00684600 | applied = pending |
| PublishScreenSet | 006858BA, 004F8710 / 004D8C00 | level-4 set {1}, context set {1} |
| EnterScreen | 004C4165, 004F8830 | pass A drops slot 5Ch, pass B enters slot 1 |
| ScreenVisible | 004F8925 | pass C updates slot 1 for the rest of the run |

The frame-path boundary the doc warns about is reproduced: while the state is 1, 2 or 4 the
pump runs from the branch's tail at 004e4ca3, and from the instant 004e4279 writes 5 it runs
from `BSP_Game_UpdateInterfaceOnly` 004c40f0 instead. The 120 frame run pumps 120 times, 31
through the branch and 89 through 004c40f0.

The seven main-menu screen classes are not reconstructed. `docs/MAIN_MENU_SCREENS.md`
records their constructors, sizes, screen ids, virtuals and layouts, so 00686380's
`create_screen` builds the executable's own registry record from that table and registers it
under the recovered id; the constructor addresses and the register virtuals stay
unimplemented records. Only screen id 1's layout `FE_main` is loaded, because only screen id
1 is in the level-4 set the shell publishes. The top-level menu list is built in code by
00584AE0 through 00AAB4C0, not by the layout, so no menu items are drawn.

### 5. The screenshot switch

`--screenshot <path>` saves the back buffer as a PNG through `D3DXSaveSurfaceToFileA`, taken
from `d3dx9_40.dll` with the same dynamic import the font resources use. The capture runs
between `EndScene` and `Present` so it sees the finished buffer, and it fires on the last
frame of a `--frames N` run or on the frame a close request is observed. The path is
resolved to an absolute one at parse time, before `--game-root` changes the current
directory, so a relative path never lands inside the installed game.

### What it looks like on screen

Before the injected press the window shows the title page exactly as milestone 2b did: the
Pacific theatre map, the US flag, the top and bottom rails and the winged
`BATTLESTATIONS PACIFIC` title plate. The installed frame texture reads `MIDWAY MODDERS`
because this installation is modded.

After the press the press-start page is gone and the main-menu background is up: the flag,
the two rails and the now empty title plate remain, and `FE_main`'s `bg_01_Icon`
(`FE\bg\main_alpha.tga`) fills the middle. Nothing chose to hide `FE_initial`: the level-4
set lists only screen id 1, the recompute 004f7620 therefore stops wanting slot 5Ch, the
pump's exit pass runs the screen's exit virtual and clears its applied byte, and 004f83b0
publishes that byte to the page. That is the mechanism milestone 2b substituted a rule for.

`drawn: yes`. The bridge holds 6 quads while the title is up and 5 once the main menu is,
from 13 textures over 678 merged atlas items. Both captures are written to the ignored
`local/title.png` and `local/run.png` and are not committed.

### Host methods

`bsp_game.exe --frames 120 --press-start-frame 30 --log local/game_run.log --screenshot
local/run.png --game-root "<install>"`, exit 0: **145 concrete, 76 unimplemented, 221
distinct methods**. Milestone 2b's run reported 81 and 36 over 117, on a tree that has since
taken other owners' work, so the difference is not this packet's alone. A run closed with
`CloseMainWindow` instead of a frame limit reports 146 concrete, because it also reaches
`CloseRequestPolicy::front_end_branch` (004ca2f0).

The six methods milestone 2b's follow-up 7 named are now concrete:

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `ApplicationFrameHost::game_state` | `00737acc` | concrete | 240 |
| `ApplicationFrameHost::profiler_set_frame_slot_color` | `004c1dd0` | concrete | 120 |
| `ApplicationFrameHost::profiler_begin_frame_slot` | `00be3640` | concrete | 120 |
| `ApplicationFrameHost::profiler_end_frame_slot` | `00be3660` | concrete | 120 |
| `ApplicationFrameHost::profiler_end_frame` | `00be34d0` | concrete | 120 |
| `ApplicationFrameHost::input_action_pressed` | `004c43c0` | concrete | 89 |

The edge test runs 89 times rather than 120 because the frame reaches 00737ae7 only outside
game states 1, 2 and 4, which is `bsp::is_front_end_game_state`: the 31 title frames skip it
and every frame from the shell onwards runs it.

So are the eleven Init-tail records 2a and 2b carried:

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `Phase 2 probe_hardware` | `0073c3b0` | concrete | 1 |
| `Factory tail mpak_factory` | `00736b60` | concrete | 1 |
| `Factory tail register_provider_factory` | `00be0660` | concrete | 1 |
| `Factory tail pak_archive_registry` | `00736c30` | concrete | 1 |
| `Factory tail set_manager_pak_registry` | `00bd9230` | concrete | 1 |
| `Factory tail create_pak_registry_lock` | `00bb40b0` | concrete | 1 |
| `Phase 6 resource_manager` | `004c1400` | concrete | 2 |
| `Phase 6 animation_channels_parser` | `00736dd0` | concrete | 1 |
| `Phase 6 bone_parser` | `00736ea0` | concrete | 1 |
| `Phase 6 register_type_parser` | `00b80a50` | concrete | 2 |
| `Phase 9 decal_definitions` | `00740840` | concrete | 1 |

The front-end methods this packet introduced, in call order. Everything not listed as
concrete is the unimplemented policy with its native call site on the record.

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `TitleInit::open_named_block` | `00be0a30` | **unimplemented** | 1 |
| `TitleInit::reset_player_profile` | `007fdb20` | **unimplemented** | 1 |
| `TitleInit::load_texture_atlas` | `00af0060` | **unimplemented** | 1 |
| `TitleInit::select_front_end_frame_set` | `00518250` | concrete | 1 |
| `FrontEndFrameLayouts::acquire` | `00518380` | concrete | 2 |
| `TitleInit::create_attract_screen` | `00689d90` | **unimplemented** | 1 |
| `TitleInit::create_title_screen` | `0068d760` | concrete | 1 |
| `TitleInit::title_screen_activate` | `0068d8d0` | concrete | 1 |
| `TitleInit::sign_in_state_valid` | `0067c8f0` | **unimplemented** | 1 |
| `TitleScreen::create_press_start_screen` | `0067c840` | concrete | 1 |
| `FrontEndScreen::register` | `004f71d0` | concrete | 8 |
| `PressStartScreen::load_layout` | `00aa5840` | concrete | 1 |
| `FrontEndScreen::collect_children` | `004f75d0` | concrete | 4 |
| `FrontEndScreen::set_child_visible` | `004f83b0+34h` | concrete | 4 |
| `FrontEndScreen::commit_visibility` | `004f83b0` | concrete | 4 |
| `PressStartScreen::enter` | `0067cb40` | concrete | 1 |
| `PressStartScreen::clear_page_context` | `00518d60` | **unimplemented** | 1 |
| `FrontEndScreens::pump` | `004f8830` | concrete | 120 |
| `FrontEndScreens::update_interface_only` | `004c40f0` | concrete | 89 |
| `FrontEndFrame::update_title_screen` | `0068d850` | concrete | 31 |
| `PressStartScreen::update` | `0067cfb0` | concrete | 62 |
| `PressStartScreen::set_element_color` | `0067cfb0+vtable50` | concrete | 62 |
| `PressStartScreen::set_element_flag` | `0067d081` | concrete | 124 |
| `XenonSystemManager::sign_in_phase` | `00a3e500` | **unimplemented** | 64 |
| `XenonSystemManager::request_sign_in` | `00a3f3d0` | **unimplemented** | 2 |
| `PressStartScreen::request_main_menu_state` | `0068d8a0` | (not reached) | 0 |
| `TitleSkip::request_game_state` | `004d7920` | concrete | 1 |
| `TitleSkip::release_state_request_hold` | `0068d8c6` | concrete | 1 |
| `MainMenuPath::drain_state_request` | `004e4430` | concrete | 1 |
| `MainMenuPath::enter_front_end_shell` | `004e4000` | concrete | 1 |
| `FrontEndShell::game_on_init` | `004e3ac2` | concrete | 1 |
| `FrontEndShell::destroy_title_screen` | `004e4071` | concrete | 1 |
| `FrontEndShell::effect_manager_context` | `004c1650` | concrete | 1 |
| `FrontEndShell::create_manager_ac` | `00686380` | concrete | 1 |
| `FrontEndManager::create_main_menu_manager` | `00686170` | concrete | 1 |
| `FrontEndManager::create_screen` | `00bf681b` | **unimplemented** | 7 |
| `FrontEndScreen::load_layout` | `00aa5840` | concrete | 1 |
| `FrontEndScreen::bind_layout` | `004f7590` | **unimplemented** | 6 |
| `MainMenuPath::push_interface_request` | `004cc460` | concrete | 1 |
| `MainMenuPath::activate_main_menu_manager` | `00684700` | concrete | 1 |
| `MainMenuPath::start_main_menu_screen_object` | `005884a0` | concrete | 1 |
| `MainMenuPath::set_game_state` | `004e4279` | concrete | 1 |
| `MainMenuPath::service_pending_interface_requests` | `006840f0` | concrete | 1 |
| `MainMenuPath::apply_interface_request` | `00684600` | concrete | 1 |
| `MainMenuPath::map_interface_to_screen` | `00685820` | concrete | 1 |
| `MainMenuPath::publish_screen_set_level4` | `004f8710` | concrete | 1 |
| `MainMenuPath::publish_input_context_set_level4` | `004d8c00` | concrete | 1 |
| `FrontEndScreen::enter` | `004f75a0` | **unimplemented** | 1 |
| `FrontEndScreen::exit` | `004f75b0` | (not reached) | 0 |
| `PressStartScreen::exit` | `0067c870` | concrete | 1 |
| `FrontEndScreen::update` | `004f75c0` | **unimplemented** | 89 |
| `GameFrameControl::simulation_spine` | `004e4d32` | **unimplemented** | 120 |
| `Phase 9 decal_definitions` | `00740840` | concrete | 1 |

The ten `HardwareProbeTailHost` methods exist and were not reached, because the validation
machine has no stored hardware profile and the all-four gate returns first. The seven
`TitleMusicHost` methods were all reached and are all unimplemented: FMOD is another
owner's.

### Corrections

1. **`docs/MAIN_MENU_PATH.md`'s step 13 cannot fire on a cold boot.** That step reads
   006840F0 as seeing `manager+4h != manager+20h`. `BSP_MainMenu_Init` 00686380 has already
   run 00684600 with INTF_MAINMENU at 006868AD before it activates the manager, so the two
   records are equal before the shell's push at 004E4259 writes the same id into the pending
   record. 006840F0 therefore finds nothing to service and the path would stall. The
   executable reports the channel as already applied in exactly that case and publishes the
   screen set itself; the end state is the same, and the reason is recorded in
   `src/game_hosts_menu.cpp`. What actually publishes the set in the original is 00684700's
   replay at 0068477E, which goes through the manager's own virtual +10h (00685820);
   `bsp::activate_front_end_manager_00684700` calls the base 00684600 there and says so.
2. **Milestone 2a's `Phase 9 game_entry [00740840]` label is wrong**, as
   `docs/APP_INIT_TAIL.md` already recorded. 00740840 is the decal definition loader: it
   enters no game state and touches no screen set. The log label is now
   `Phase 9 decal_definitions`.
3. **Milestone 2b's follow-up 2 is done.** The front-end screens own the title pages and the
   visibility push 004f83b0 replaces the bridge's substitute rule for them. The rule still
   stands in for pages no screen owns, which is the five 00aa5e20 resource entries and the
   two 00518250 frame layouts; 2b's caveat about those two is unchanged and still unverified.
4. **Milestone 2b's follow-up 7 is done**, over the header that has since merged.
5. **The milestone 2b host table's `Title bring-up GGame::OnInitTitle [004c9a70]`
   unimplemented record is gone.** The bring-up runs.

### Code with no Ghidra function

None. Every address this packet touched has a Ghidra function and a reviewed ledger name,
including the seven empty base virtuals 004f7570..004f75d0 that
`docs/FRONTEND_STATE_MACHINE.md` listed as undefined; the integrator has since defined them.
The packet appended run-time evidence to 0073c3b0, 00736b60, 00736c30, 00bb40b0, 00b80a50,
004c1400, 00740840, 004c9a70, 0068d8d0, 004f8830, 004f83b0, 0067cfb0, 004e4000, 00686380,
00684700, 006840f0, 004f8710, 004c40f0, 00518250, 00be34d0, 00be3640, 00be3660 and 00737a50
rather than adding names.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.

```
hardware profile incomplete under HKLM\SOFTWARE\Eidos\BSM_HWD; the probe returns
        without comparing anything
mpak factory singleton cache=010904d4 vtable=00cfea20 extension=.mpak
pak archive registry cache=010904d8 enabled=1 provider_limit=100
resource type parser AnimationChannels  registered=1 map=1
resource type parser Bone               registered=1 map=2
decal definitions parsed: 3
title init: game state 2, press-start slot 92, registry screens 1
press-start input action 78 injected on frame 30
state request 4 enqueued by the press-start skip
front-end shell entry outcome=ShellReady
level-4 screen set published: 1 id(s), first=1
sprite bridge quads=5 textures=13/14 atlas_items=678 rebuild=32
summary init_tail hardware_probe=1 stored_values=0 factories=3 parsers=2 pak_registry=1
        pak_lock=1 decals=3
summary frontend title_init=1 press_start_slot=92 screens=7 screen_pages=4 pump_frames=120
        enters=2 exits=1 commits=4
summary mainmenu press_start_frame=30 injected=1 shell=1 manager=1 screen=1
        step=ScreenVisible state=5
summary screenshot=1 path=local\run.png
host methods 145 concrete, 76 unimplemented
```

`--vfs-probe` still works both ways: `--vfs-probe fonts/fonts.lua` exits 0 and
`--vfs-probe does/not/exist.lua` exits 3. The close path was validated by sending WM_CLOSE
to a running process with `--press-start-frame 30` and no frame limit: it presented 50459
frames, reached the main menu, recorded `CloseRequestPolicy::front_end_branch [004ca2f0]`
once, wrote its screenshot and exited 0.

This is a runtime-validated process, not a game-validated one. It proves the recovered
front-end state machine carries a cold boot from the title page to a visible main menu over
real installed data. It proves nothing about gameplay, about the GUI layer's own draw, or
about binary compatibility with the original executable.

### Follow-up packets

1. **The seven main-menu screen classes**, 005902E0 and its six siblings with their register,
   enter, exit and update virtuals. Until they exist the main menu has a background and no
   items: 00584AE0's seven-entry list through 00AAB4C0 is what builds them.
2. **The GUI layer draw**, still 2b's follow-up 1. The sprite bridge is now driven by real
   screen visibility, which makes it a closer stand-in and no less of one.
3. **The XenonSystemManager at 00f8abe8.** It is what the press-start screen's own arms wait
   on, and without it the executable's injected edge has to stand in for the sign-in flow the
   native runs between the keypress and 0068d8a0.
4. **The loading screen**, 0057cb60 / 0057c250 / 0057bec0 and the 0x58 singleton at 00e194b4.
   The shell entry raises and lowers it; here both calls are records, so the shell's eight
   progress reports go nowhere.
5. **`hardware_detection_object`**, 009927d0 and 00992830, plus the localized message table
   at 00d1e918 that 00996060 indexes. They are the whole machine side of the phase-2 probe.
6. **`mpak_provider_create`** (00bb83a0, 00bb8240, 00bb82f0) and **`resource_item_parsers`**
   (00b8a910, 00b8a990), the two bodies behind the singletons phase 2 and phase 6 now
   register.
7. **The title music 005884a0's FMOD side** and the 0x54 stream object 00a877d0 builds.

## Milestone 2d: text on screen

Addresses: 00abb630 (the Text property reader), 00ab8c30 (font by name through the
registry lookup 00ac3570), 00abaed0 (set localised source), 00a9fad0 / 00a9f4b0 (the
localisation resolver), 004c5e60 (the non-localised widening), 00aba8d0 (the geometry
update that chooses a builder and uppercases the text), 00a9ec30 with 00a9eba0 and
00c0391c (that uppercase pass), 00ab9fd0 / 00aba270 / 00aba860 (the two layout builders
and the wrapped vertical offset), 00ab98f0 with 00ad4480 (the glyph quad writer and glyph
selection), 00ab6b50 (the colour re-apply), 00e12fd4 (the vertical scale), 00ab3310 (the
Icon reader, now recorded at its own site), and 00aa6720 / 00b6d890 / 00b74eb0 / 00b75030
/ 00aaa710 / 00ad08e0 / 00ab8400 / 00ab8530 / 00ab8ce0 / 00b692c0 for the corrections and
the new records. Packet `cc_exe_2d`, owner `agent/cc-exe-2d`. Sources:
`src/game_hosts_text.cpp`, `include/bsp/game_hosts_text.hpp`, plus edits to
`src/game_hosts_frontend.cpp`, `src/game_hosts.cpp`, `src/game_main.cpp` and their
headers. Report: `reports/game_executable_milestone_2d.json`. Ghidra was read-only.

Milestone 2c left every Text widget resolving its font and its string id and drawing
nothing. This milestone runs the rest of that path over the reconstructions that were
already on main, and the two title strings are readable in a `--screenshot` capture.

### The path one Text widget takes

`GameTextHost` implements `bsp::GuiTextHost`, so the order is 00abb630's order rather
than one the process invents. Per widget:

| Step | Native site | What the run does |
| --- | --- | --- |
| property read | 00abb630 | `bind_gui_text_properties_00abb630` over the evaluated page table |
| font | 00ab8c30 | `find_font_00ac3570` on the live registry the phase-7 startup filled |
| set source | 00abaed0 | the authored `DefaultText`, always with the localise flag 00abbd90 pushes |
| resolve | 00a9fad0 / 00a9f4b0 | `LocaleTextResolver` over the loaded `englishauthentic` table |
| uppercase | 00a9ec30 / 00a9eba0 | run when the font descriptor is `uppercase_only`, which Viper19 is |
| builder choice | 00aba8d0 | +FCh defaults to 1, so both widgets take the wrapped builder |
| layout | 00aba270 | `build_font_wrapped_00aba270_fragment` |
| quads | 00ab98f0 / 00ad4480 | one `write_font_quad_00ab98f0_fragment` per placement |
| vertical offset | 00aba860 | added to each normalised y after the quad write |
| colour | 00ab6b50 | the widget's +50h, re-read live on every bridge rebuild |

What the 120 frame run laid out:

| Page | Widget | Font | Source | Glyphs | Container | Measured |
| --- | --- | --- | --- | --- | --- | --- |
| `FE_initial` | `press_start_Text` | Viper19 | `LOG INTO ALTERBSP THEN PRESS ENTER TO CONTINUE` | 46 | 499 | 461.0 |
| `FE_frame_title` | `title_Text` | Viper19 | `MAIN MENU` | 9 | 768 | 101.0 |

Neither id is in the installed table, so 00a9f4b0's documented fallback ran and appended
the stripped key widened byte by byte. That is not a failure of the lookup: this
installation's `fe_initial.lua` authors an English literal where the stock page authors
`FE_xbox.init_start`, and `fe_frame_title.lua` authors `MAIN MENU` directly. The
container width 499 for a `Size.x` of 0.520833 is the native truncation, not an error:
0.520833f times 960 is 499.99968.

`FE_main`'s two Text widgets, `SubtitlesNormal_Text` and `SubtitlesWideScreen_Text`,
author no `DefaultText`. The native widget has no text either, so they are counted and
skipped rather than reported as a miss. The main menu's own labels are not layout text at
all: 00584AE0 builds them in code through 00AAB4C0, which is still milestone 2c's
follow-up 1.

### Three values this milestone supplies

- The vertical scale the quad writer and the wrapped builder multiply every y by is the
  mutable global 00e12fd4, which the image initialises to 0.75. Nothing in the
  reconstructed startup writes it, so a cold process keeps that value; a running game may
  not. The run reports the number it used.
- The text context's normalised space is placed at the widget's resolved position with
  its own pivot subtracted, the same corner the bridge already draws an Icon from. The
  native places it through the widget's scene node transform, which this process does not
  build. **This is a bridge decision, not recovered behaviour.**
- The glyph quads are drawn by the sprite bridge with fixed-function state, one textured
  triangle pair per glyph over the font's own glyph sheet, modulated against the widget
  colour. The native binds `guifontbilinear.shfx` through 00ab8ce0 and submits the shared
  vertex and index objects 00ab8400 creates and 00ab8530 sections; all three are
  unimplemented records. `src/d3d9_font_probe.cpp` already draws one glyph through the
  real shader (docs/FONT_MATERIAL_DRAW.md); binding that path to the run is the GUI layer
  draw packet, not this one.

### What it looks like on screen

Before the injected press, the title page now reads
`LOG INTO ALTERBSP THEN PRESS ENTER TO CONTINUE`, centred low over the Pacific theatre
map in Viper19, at the prompt's pulsing alpha. The winged title plate reads `MAIN MENU`
in the authored gold. After the press the press-start page is gone and `MAIN MENU` stays
in the plate over the menu background. Both captures come from the client area of a
running window and are written to the ignored `local/title.png` and `local/run.png`.

`drawn: yes`. 55 glyph quads while the title is up, 9 once the main menu is, on top of the
6 and 5 icon quads milestone 2c reported. The plate reading `MAIN MENU` during the title
phase is not a claim about the game: no front-end screen owns `FE_frame_title`, so its
visibility is still the bridge's substitute rule, which is milestone 2b's unchanged caveat
about the two 00518250 frame layouts. This supersedes milestone 2c's "the now empty title
plate".

### The new switch

`--screenshot <path>` captured the last frame of a `--frames N` run, or the frame a close
request was seen. `--screenshot-frame N` names a frame instead, so one run photographs the
title before the injected press-start and another the menu after it. A negative value, and
the absence of the switch, keep the old rule.

### Host methods

`bsp_game.exe --frames 120 --press-start-frame 30 --log local/game_run.log --screenshot
local/run.png --game-root "<install>"`, exit 0: **154 concrete, 80 unimplemented, 234
distinct methods**. Milestone 2c's run on the same tree reported 145 and 76 over 221. A
run closed with `CloseMainWindow` reports 155 concrete, because it also reaches
`CloseRequestPolicy::front_end_branch`. A title-only run (`--frames 40`, no press) reports
129 concrete and 49 unimplemented and draws all 55 glyph quads.

The methods this packet introduced:

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `GuiText::read_properties` | `00abb630` | concrete | 2 |
| `GuiText::find_font` | `00ab8c30` | concrete | 2 |
| `GuiText::resolve_localised` | `00a9fad0` | concrete | 2 |
| `GuiText::uppercase_text` | `00a9ec30` | concrete | 2 |
| `GuiText::build_wrapped` | `00aba270` | concrete | 2 |
| `GuiText::apply_color` | `00ab6b50` | concrete | 2 |
| `GuiText::write_glyph_quad` | `00ab98f0` | concrete | 2 |
| `LocaleText::crt_uppercase` | `00c0391c` | concrete | 55 |
| `GuiIcon::read_properties` | `00ab3310` | concrete | 18 |
| `GuiText::create_glyph_buffers` | `00ab8400` | **unimplemented** | 2 |
| `GuiText::ensure_draw_sections` | `00ab8530` | **unimplemented** | 2 |
| `GuiFrameBox::read_properties` | `00ad08e0` | **unimplemented** | 5 |
| `GuiLayoutHost::allocate_model_slot` | `00b74eb0` | **unimplemented** | 28 |

Four more exist and were not reached, because the paths that call them are not taken:
`GuiText::widen_source` (004c5e60) needs a Text whose source is not localised, and the
native call site pushes the literal 1 for every authored `DefaultText`;
`GuiText::build_single_line` (00ab9fd0) needs an authored `Multiline` of false, and the
constructor default is true; `GuiText::font_shader` (00ab8ce0) needs a font name that
changes after the first resolve; and `LocaleText::lua_context_value` (00b692c0) needs a
`#...#` marker, which no shipped title or menu string carries. That last one is the
unimplemented policy rather than a guess: the native resolver reaches its Lua owner
through the global at 00e1ae90 and its virtual +14h, and this process builds no such
owner.

Two records were renamed rather than added, for the reasons in the corrections below:
`GuiLayoutHost::create_page_root_node` (00aa6720) is now
`GuiLayoutHost::bind_page_root_scene_node`, and `GuiLayoutHost::clear_page_root_list`
(00b6d890) is now `GuiLayoutHost::propagate_root_registration`.

### Corrections

1. **00aa6720 is not a page-root factory.** It is `BSP_GuiWidget_SetSceneNode`: four
   instructions that store a node at widget+4Ch and, when it is non-null, clear the low
   two bits of node+138h. What builds a plain page root is 00aa5840's own branch, the
   18Ch `cGroup` of 00b8f5e0 that
   `GuiNativeScene::create_plain_page_root_00aa5840_fragment` reconstructs; the setter
   receives the result. Milestone 2b's host table used the wrong verb for this address.
2. **00b6d890 is not a list clear.** It is `BSP_Node_PropagateRootRegistration`, the
   complete recursive pass that registers or unregisters a subtree against a requested
   scene root through 00b721f0 and 00b72220. 00ac6825 calls it with the page's own root.
3. **The per-widget scene node is a pair of calls.** 00aa6640
   `BSP_GuiWidget_CreateWithSceneNode` takes a canonical 188h slot out of the model pool
   at 01090054 with 00b74eb0 and constructs the 184h generated model in it with 00b75030.
   Milestone 2b recorded only the second, so the run under-reported the allocation. Both
   are recorded now, 28 calls each.
4. **The single `derived_property_reader [00aaa710]` record said less than it could.**
   00aaa710 is the base half of the reflection pair; its caller is the leaf class's own
   reader, and three of the four types the title and menu pages use have a reconstruction.
   The table now names 00ab3310 for an `Icon` (which was already running, inside the
   bridge's own widget walk, just not recorded at its call site), 00abb630 for a `Text`
   and 00ad08e0 for a `FrameBox`, and keeps 00aaa710 only for the types with none.
5. **Milestone 2b's follow-up 3 is done for the title and menu pages, and its stated
   blocker was wrong.** That item said the missing piece was "the same device binding as
   item 1", the GUI layer draw. It was not: the reconstructed layout and quad writer emit
   normalised GUI coordinates, so the existing sprite bridge draws them with no new device
   work. What the GUI layer draw is still needed for is the font material and the shared
   glyph buffers, which is why 00ab8ce0, 00ab8400 and 00ab8530 remain records.
6. **Milestone 2c's "the now empty title plate" is superseded.** The plate reads
   `MAIN MENU` in both phases.

### What stays unimplemented, and why

- **00ab8ce0, 00ab8400, 00ab8530**, the font material and the shared glyph vertex and
  index objects. Both need a renderer material owner and a device-backed buffer factory.
- **00ad08e0**, the FrameBox reader. It is reconstructed, but it takes
  `GuiFrameBoxTextureServices`, the native renderer's texture acquire and release pair,
  and this process owns no such reference.
- **The scene graph**, 00aa6720, 00b74eb0, 00b75030, 00b6e680, 00b6d890 and the two widget
  vtable hooks, 96 calls in a 120 frame run. Every one has a reconstruction with an
  integrated-storage status, but none is a free function: they compose into
  `GuiWidgetOwnerRuntime` over a `NativeModelEnvironment` and a `NativeGroupEnvironment`,
  which need real slab pools and the image's own constants at 00d7a24c and 00ce4970.
  Nothing in the repository builds that composition yet, so it is a packet rather than a
  wiring change. This is the correction to this packet's own brief: the trio is
  reconstructed, and it is still not something the executable can call today.
- **The VFS content-suffix list at manager +48h/+4Ch.** No routine in the ledger is named
  as its writer, and no query found one. Populating it would be an invention, so it stays
  the empty list milestone 2b reported and `allbutingame.ats` still does not resolve to
  its DXT variant.
- **`FrontEndScreen::enter` / `exit` / `update`** (004f75a0, 004f75b0, 004f75c0). The base
  bodies really are a single RET, but these records stand for the leaf class's override,
  and none of the seven main-menu classes is reconstructed.
- **`ApplicationFrameHost::tick_vfs_providers`** (00bdb0b0): the ledger names a fragment
  in `src/vfs_pending.cpp`, and no such symbol exists in the tree.
  **`update_loading_queue`** (004fde20): the singleton accessor is named, the object it
  constructs is not reconstructed.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function. One name was added,
00a9ec30 `BSP_TextContext_UppercaseUtf16InPlace`, which Ghidra carried as `FUN_00a9ec30`;
run-time evidence was appended to 00ab98f0, 00aba270, 00abb630, 00a9fad0, 00aa6720,
00b6d890, 00b74eb0 and 00ab8c30.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing
ctest case `reconstructed_math` passes, 1 of 1. No test cases were added.

```
text bridge open: 6 fonts, 6856 locale keys, vertical_scale=0.7500 (00e12fd4)
  text press_start_Text  font=Viper19 wrapped id=LOG INTO ALTERBSP THEN PRESS ENTER TO
        CONTINUE (no table entry) glyphs=46 lines=1 container=499 width=461.0 upper=1
  text title_Text        font=Viper19 wrapped id=MAIN MENU (no table entry) glyphs=9
        lines=1 container=768 width=101.0 upper=1 color=(0.98,0.86,0.60,1.00)
sprite bridge quads=61 (text_glyph_quads=55) textures=13/14 atlas_items=678 rebuild=1
sprite bridge quads=14 (text_glyph_quads=9)  textures=13/14 atlas_items=678 rebuild=32
summary text bridge=1 widgets=4 runs=2 glyphs=55 quads=9
summary screenshot=1 frame=-1 path=local\run.png
host methods 154 concrete, 80 unimplemented
```

`--vfs-probe` still works both ways: `fonts/fonts.lua` exits 0 and `does/not/exist.lua`
exits 3. The close path was validated by sending WM_CLOSE to a running process with
`--press-start-frame 30` and no frame limit: it presented 36289 frames, reached the main
menu, recorded `CloseRequestPolicy::front_end_branch [004ca2f0]` once, wrote its
screenshot and exited 0.

This is a runtime-validated process, not a game-validated one. It proves the reconstructed
localisation, font layout and glyph geometry produce readable text out of real installed
data, drawn at the position the recovered layout computes. It proves nothing about the
native GUI draw, about the font shader, or about binary compatibility with the original
executable.

### Follow-up packets

1. **The GUI layer draw**, still milestone 2b's follow-up 1. Deleting the sprite bridge
   now also means binding 00ab8400, 00ab8530 and 00ab8ce0 to a device and a material
   owner; `src/d3d9_font_probe.cpp` already proves the shader half against one glyph.
2. **The scene graph composition.** A `GuiWidgetOwnerRuntime` with a
   `NativeModelEnvironment` and a `NativeGroupEnvironment` the executable can own would
   make six records concrete at once.
3. **The FrameBox reader** 00ad08e0 over a `GuiFrameBoxTextureServices` the executable can
   supply, which is the same texture-ownership question as the renderer texture entry
   point.
4. **The VFS content-suffix list writer**: identify what fills manager +48h/+4Ch.
5. **The Text widget's shadow half.** 00ab72d0's presets are read and stored, and both
   title widgets author `DefaultShadow`, but nothing draws the second, offset glyph run:
   the native shares one geometry pair between the main and shadow sections at 00aba8d0,
   which is the buffer factory this packet records as unimplemented.
6. **The seven main-menu screen classes**, still milestone 2c's follow-up 1. No amount of
   text drawing puts the menu's own items on screen, because 00584AE0 builds them in code.

## Milestone 2e: from the main menu to the mission load request

Addresses: 005caaf0 with 00b66bd0 / 00b6a020 / 00b69d40 / 00b67800 / 00b669a0 / 0057bec0 /
00586150 (the mission-tree screen's register virtual and the Lua state it opens), 005c3470
(the id lookup its tail runs), 005861b0 (the main-menu screen's bind-layout virtual),
00584ae0 with 00aab4c0 (the seven top-level list entries), 00580940 with 005c27e0 and
005c3be0 (the mission-selection publish and the page pairing), 00599db0 with 004d92b0
(the per-frame page machine and its action query), 00598b60 (the page dispatcher),
0058c010 (the mission-detail page builder), 00592640 and 005922f0 (the footer command and
the play action), 0058bdf0 with 00626930 (the start and the statistics reset), 00439020
(the two state requests), 004e2770 / 004e1d70 / 004c6890 (the pending scene, the scene
record and its selection), 0046df00 with 008d9cf0 (the `.scn` header pass) and 004dfb70
(the load the run stops in front of). Packet `cc_exe_2e`, owner `agent/cc-exe-2e`. Sources:
`src/game_hosts_mission.cpp`, `include/bsp/game_hosts_mission.hpp`, plus edits to
`src/game_hosts_menu.cpp`, `src/game_hosts_text.cpp`, `src/game_hosts_frontend.cpp`,
`src/game_hosts.cpp`, `src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2e.json`. Ghidra was read-only for this packet.

Milestone 2d left the executable on a main menu with a background and readable title text
and no way in. This milestone carries a run from that menu to the mission load request: it
loads the real mission tree out of the installed game, publishes one mission's selection
through the recovered page rule, builds the mission-detail page over the installed
world-map layout, takes that page's play action, and stops at the first host the load needs
from the renderer.

### The new switch

`--menu-select <mission id>` names the mission the mission-tree screen's loader asks the
shell for at 00586150. **The key is the record's Lua `id`**, which for a campaign mission is
a short code such as `USN02`; `usn_2_java` is the scene file's stem, not the id, and an id
the tree does not carry takes 005CAAF0's own clamp to the first mission of the first group.
Every earlier switch is unchanged.

### 1. The mission tree, 005CAAF0

The mission-tree screen is registry slot 2 and the only one of the seven whose bind step
reads a data table rather than a GUI layout, which is what
`docs/MAIN_MENU_SCREENS.md`'s class row already says. `GameMissionHost` runs
`bsp::load_mission_tree_005caaf0` over a `MissionTreeScriptHost`: a `PcStorageLuaOwner` on
stock Lua 5.1.1 opened with library mask 4, `Scripts/datatables/MissionTree.lua` run
through the mounted VFS (its own `DoFile` of `MultiGlobals.lua` included), and the
`MissionTree` global read through a `MissionTreeLuaView` over the live reader. The view is
this milestone's own bridge between the reconstructed readers and the interpreter; the
reading rules are all 005C6A70's, 005C5DA0's and 005C9F70's.

The installed table, all 143 missions of it:

| Group | Name | Missions | First ids |
| --- | --- | --- | --- |
| 0 | Training grounds | 22 | TRN1 TRN2 TRN5 TRN3 |
| 1 | IJN campaign | 30 | IJN01 IJN02 IJN03 IJN04 |
| 2 | USN campaign | 36 | BSM01 BSM02 BSM03 BSM04 |
| 3 | IJN DLC | 27 | JM02 JM03 JM04 JM05 |
| 4 | USN DLC | 28 | PRCP01 PRCP02 PRCP03 PRCP04 |

`multiMissionInfos` holds 34 further records. The group order is an independent confirmation
of `docs/MAIN_MENU_MISSION_DETAIL.md`'s table: the index the tree keeps is the index
0058C010 switches on, and group 1 is Japan while group 2 is the United States.

### 2. The page pairing, 00580940

With `USN02` selected the tree's tail resolves group 2, mission 13. 005C27E0 reads the low
byte of the record's `allied` block and answers side index 0, so the publish writes
`00E194D8 = 2`, `00E194DC = 13`, `00E08878 = -1` and page **5**, `CampaignUs`. That is the
page `docs/MAIN_MENU_MISSION_DETAIL.md` predicts for a side-0 mission of group 2, reached
from the other side of the relation, so the `{4,5}` pair really is Japan then the United
States.

`00599DB0` then runs for real: the mission-list arm of the page machine takes page 5,
selects `missions_US_Group` into +110h, reads the two zoom actions (neither of which has a
record in this process) and reaches the shared epilogue with scene id 3.

### 3. The mission-detail page, 0058C010

`005861B0` loads the three page roots the screen keeps and binds the fourteen widget
handles the detail page drives. All three pages load and all fourteen widgets bind. The
executable leaves the three roots hidden after the bind, because the page builder that
would show or hide one on the top-level page (00584AE0) is not reconstructed; that is this
milestone's own decision, not recovered behaviour, and 0058C010's own visibility block then
shows `FE_worldmap_historical` exactly as 0058C6E0 does.

0058C010 runs over the real record: group 2 takes the 0058C0AA arm, `+564h` is set and
`+565h` cleared, all 36 missions of the group are visited, the visibility block hides the
two briefing roots and `training_Group` and shows the world map, the briefing text is set
from the record's `background` key, and `00E08874` becomes 9.

`--menu-select TRN1` exercises the other side of the same gate and confirms it:
005CAAF0 resolves the training grounds, 00580940's group-0 arm at 00580994 sends the page to
8, and 0058C092 rejects published group 0, so no mission-detail page is built, no footer
command exists and the run stops there with exit 0.

### 4. The briefing start and the load request

The page's `globals.continue` footer command reaches 005922F0 through 00592640. The record's
`MovieName` is empty on this installation, so the play action takes the no-movie arm at
00592325 and calls 0058BDF0 directly. There the record's `forcedDifficultyLevel` is the
inherit value 3 and the main-menu flag at +5Ch is clear, so the difficulty comes from
game+6B0h; the checkpoint test is not taken; 004E2770 sets the pending scene from the
record's `sceneFile`; and 00439020 enqueues 6 then 0Ah, which the drain services in one
pass. The sound, movie and profile call sites of that arm are all unimplemented records:
`MissionPlay::flush_award_tracker` (00690CD0), `MissionPlay::stop_front_end_audio`
(00A85C00) and the five movie methods were not reached because the movie arm was not taken,
`MissionStart::checkpoint_differs` (007F8D60) and `write_checkpoint` (00437C70) are the
profile owner's, and `MissionDetail::start_preview_movie` (0058C8A0),
`request_streamed_dialog` (0058C9BE) and `audio_language_folder` (008D57A0) are the movie
and audio owners'.

### 5. The scene record and the `.scn`

004E2770's Create arm builds one record through 004E1D70 and 004C6890 selects index 0. The
header pass is concrete: 008D9CF0's read is the same VFS pair every other asset takes, and
the reconstructed reader runs over the 48680 bytes of
`universe/Scenes/missions/USN/usn_2_java.scn`.

| Fact | Value |
| --- | --- |
| `uniqueID` into record+1098h | 1 |
| short name (004CD7F0) | `usn_2` |
| database stem +144h | `universe/Scenes/missions/USN/usn_2_java` |
| numbered VFS blocks | `1_usn_2` `3_usn_2` `4_usn_2` `5_usn_2` `6_usn_2` |
| mission script (008860B0) | `Scripts/missions/usn_2_java.lua` |
| side blocks in the record | 0 |
| slot records filled | 0 |
| entities | 34 |
| distinct classes | 2 |

The classes are `DestroyerGen` x32 and `NavPoint` x2, both registered and both
instantiate-only. Every one of those numbers matches `bsp_mission_scene_probe.exe` on the
same file, which is the point: the executable and the fixture read the installed scene the
same way. The entity walk itself belongs to passes 2 and 3 inside 004D4DF0 and reaches no
field of the record; it is run here only to say what the selected mission's scene holds.

The record's side-block count stays 0 because the header pass does not fill it, so all eight
slot records get their in-use byte cleared and nothing else. That is 004C6890's documented
behaviour for a record with no side blocks, not a failure.

### 6. Where the load stops

Request 6 is dequeued as the front-end owner's interface change and request 0Ah is dequeued
as 004DFB70's. The executable does not enter 004DFB70. It logs the whole 68-step host
inventory of `mission_load_host_steps` with each step's owner area, and stops at the first
step whose owner is the renderer or the scene graph:

```
MissionLoad(world)::global_subsystems        004dc6a0
MissionLoad(renderer)::reset_render_scene    00874640   <- the run stops here
```

54 of those steps are inventory records: the run names them and their owners, and no call
site was reached. That is why this run's unimplemented count is much larger than 2d's; the
122 that a call actually reached is the comparable number.

### What it looks like on screen

Before the injected press the title page is unchanged from milestone 2d. After it the main
menu comes up as 2d described, and then the mission-detail page replaces it: the Pacific
theatre world map fills the frame, the winged plate still reads `MAIN MENU`, and the
historical panel on the left carries the whole of the selected mission's briefing text,
justified in Arial16 over the reconstructed wrapped layout. The capture is written to the
ignored `local/run.png`.

`drawn: yes`. 705 quads on the mission-detail page, 692 of them glyphs, against 14 on the
main menu and 61 on the title page. The briefing panel is 683 of those glyphs.

Nothing draws the mission map markers, because the installed layout has none; see the
corrections.

### Host methods

`bsp_game.exe --frames 240 --press-start-frame 30 --menu-select USN02 --log
local/game_run.log --screenshot local/run.png --screenshot-frame 200 --game-root
"<install>"`, exit 0: **193 concrete, 176 unimplemented, 369 distinct methods**. Milestone
2d's run on this tree reports 154 and 80 over 234 and still does, because a run without
`--menu-select` is byte-for-byte the milestone 2d run. Of the 176, **54 are load-inventory
records** and 122 were reached by a call.

The methods this packet introduced, in call order. Everything not marked concrete is the
unimplemented policy with its native call site on the record.

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `MissionTree::construct_lua_state_owner` | `00b66bd0` | concrete | 1 |
| `MissionTree::open_lua_libraries` | `00b6a020` | concrete | 1 |
| `MissionTree::run_script` | `00b69d40` | concrete | 1 |
| `MissionTree::open_table` | `00b67800` | concrete | 1 |
| `MissionTree::report_progress` | `0057bec0` | **unimplemented** | 5 |
| `MissionTree::requested_mission_id` | `00586150` | concrete | 1 |
| `MissionTree::close_lua_state` | `00b669a0` | concrete | 1 |
| `MissionTreeScreen::load_tables` | `005caaf0` | concrete | 1 |
| `MainMenuScreen::bind_layout` | `005861b0` | concrete | 1 |
| `MainMenuScreen::load_layout` | `00aa5840` | concrete | 3 |
| `MainMenuScreen::find_child` | `00aa7e00` | concrete | 14 |
| `MainMenuScreen::build_top_level_page` | `00584ae0` | **unimplemented** | 1 |
| `MainMenuScreen::allocate_list_entry` | `00aab4c0` | **unimplemented** | 1 |
| `MainMenuScreen::locale_lookup` | `00a9ec70` | concrete | 7 |
| `MainMenuSelect::side_index` | `005c27e0` | concrete | 1 |
| `MainMenuSelect::group_completed` | `005c3be0` | concrete | 1 |
| `MainMenuSelect::publish_selection` | `00580940` | concrete | 1 |
| `MainMenuScreen::update` | `00599db0` | concrete | 2 |
| `MainMenuUpdate::action_fired` | `004d92b0` | concrete | 3 |
| `MainMenuUpdate::set_active_mission_group` | `0059a4b4` | concrete | 2 |
| `MainMenuUpdate::map_zoom_input` | `0059a517` | concrete | 1 |
| `MainMenuUpdate::drive_map` | `00588c70` | **unimplemented** | 2 |
| `MainMenuUpdate::animate_page_group` | `00683820` | **unimplemented** | 1 |
| `MainMenuUpdate::animate_detail_group` | `00683820` | **unimplemented** | 1 |
| `MainMenuUpdate::blend_map_offset` | `00414130` | **unimplemented** | 1 |
| `MainMenuUpdate::update_scene` | `004c1e90` | **unimplemented** | 2 |
| `MainMenuScreen::page_dispatch` | `00598b60` | **unimplemented** | 1 |
| `MainMenuScreen::build_mission_detail_page` | `0058c010` | concrete | 1 |
| `MissionDetail::request_page_audio` | `00518d60` | **unimplemented** | 1 |
| `MissionDetail::set_background_icon_state` | `0058c04c` | **unimplemented** | 1 |
| `MissionDetail::read_tree_selected_group` | `0058c066` | concrete | 1 |
| `MissionDetail::read_tree_selected_mission_index` | `005c3850` | concrete | 1 |
| `MissionDetail::publish_selection` | `0058c071` | concrete | 1 |
| `MissionDetail::set_active_group_widget` | `0058c0aa` | concrete | 1 |
| `MissionDetail::set_campaign_flags` | `0058c0b4` | concrete | 1 |
| `MissionDetail::bind_selected_map_point` | `0058c18b` | concrete | 1 |
| `MissionDetail::map_flag_visible` | `005c2f70` | **unimplemented** | 36 |
| `MissionDetail::append_map_point` | `004215d0` | concrete | 36 |
| `MissionDetail::set_widget_visible` | `0058c6e0` | concrete | 17 |
| `MissionDetail::commit_page_state` | `00583e50` | **unimplemented** | 1 |
| `MissionDetail::set_briefing_text` | `00abaed0` | concrete | 1 |
| `MissionDetail::text_clip_height` | `00aa6740` | concrete | 1 |
| `MissionDetail::briefing_text_height` | `00ab6bd0` | **unimplemented** | 1 |
| `MissionDetail::reset_scroller` | `00683790` | **unimplemented** | 1 |
| `MissionDetail::set_scroll_range` | `006834a0` | **unimplemented** | 1 |
| `MissionDetail::start_preview_movie` | `0058c8a0` | **unimplemented** | 1 |
| `MissionDetail::audio_language_folder` | `008d57a0` | **unimplemented** | 1 |
| `MissionDetail::request_streamed_dialog` | `0058c9be` | **unimplemented** | 1 |
| `MissionDetail::set_page` | `0058cac3` | concrete | 1 |
| `MissionDetail::set_footer_commands` | `0054b530` | **unimplemented** | 1 |
| `MissionDetail::clear_help_line` | `0054a0c0` | **unimplemented** | 1 |
| `MissionDetail::clear_list_box` | `00a9bec0` | **unimplemented** | 1 |
| `MissionDetail::set_list_box_flags` | `0058cd9e` | **unimplemented** | 1 |
| `MissionDetail::move_list_box` | `00aa8240` | **unimplemented** | 1 |
| `MissionDetail::finish_list_box` | `00aa6bc0` | **unimplemented** | 1 |
| `GuiText::set_localised_source` | `00abaed0` | concrete | 1 |
| `MainMenuScreen::footer_command` | `00592640` | **unimplemented** | 1 |
| `MainMenuScreen::play_selected_mission` | `005922f0` | concrete | 1 |
| `MissionPlay::flush_award_tracker` | `00690cd0` | **unimplemented** | 1 |
| `MissionPlay::start_selected_mission` | `0058bdf0` | concrete | 1 |
| `MissionPlay::clear_help_line` | `0054b530` | **unimplemented** | 1 |
| `MissionStart::set_current_mission_key` | `0058be40` | **unimplemented** | 1 |
| `MissionStart::set_pending_scene` | `004e2770` | concrete | 1 |
| `MissionStart::select_scene_record` | `004c6890` | concrete | 1 |
| `MissionStart::publish_loading_config` | `0057d060` | **unimplemented** | 1 |
| `MissionStart::set_mission_key_mirror` | `0058befa` | **unimplemented** | 1 |
| `MissionStart::reset_mission_stats` | `00626930` | concrete | 1 |
| `MissionStats::set_mission_name` | `00e19798` | **unimplemented** | 1 |
| `MissionStats::set_debriefing_text` | `00e197a0` | **unimplemented** | 1 |
| `MissionStats::clear_container` | `00626930` | **unimplemented** | 12 |
| `MissionStart::set_effective_difficulty` | `0058bf58` | **unimplemented** | 1 |
| `MissionStart::checkpoint_differs` | `007f8d60` | **unimplemented** | 1 |
| `MissionStart::request_mission_start` | `00439020` | concrete | 1 |
| `MissionStart::reset_front_end_timer` | `00a92c40` | **unimplemented** | 1 |
| `MissionStart::finish_start` | `004d2a80` | **unimplemented** | 1 |
| `SetPendingScene::enter_file_block` | `00be0a30` | **unimplemented** | 1 |
| `SetPendingScene::leave_file_block` | `00bdcb30` | **unimplemented** | 1 |
| `SceneFileReader::read_scene_file` | `008d9cf0` | concrete | 1 |
| `SetPendingScene::load_scene_header_pass` | `0046df00` | concrete | 1 |
| `SetPendingScene::notify_award_tracker` | `0068ea00` | **unimplemented** | 1 |
| `MissionLoad::apply_pending_interface` | `00684600` | **unimplemented** | 1 |

The 54 `MissionLoad(<owner>)::<method>` records that follow are the load inventory, one per
non-pure step of `mission_load_host_steps`, with the owner area in the name. They are not
call sites this run reached.

Twelve methods exist and were not reached, because the paths that call them are not taken.
Nine of them are the movie arm of 005922F0 (`suspend_page_for_movie`,
`stop_front_end_audio`, `apply_pending_interface`, `arm_movie_surface`,
`commit_movie_visibility`, `enter_movie_surface`, `play_mission_movie`, `mark_movie_active`
and `set_movie_completion`), which needs a record carrying a `MovieName`; `USN02` does not
carry one. `MissionDetail::map_point_position` (00AA6750) needs a numbered point icon to
exist, and correction 3 says why none does. `MissionStart::write_checkpoint` (00437C70)
needs the profile's checkpoint value to differ. `SetPendingScene::destroy_scene_record`
(004BF930) needs an existing scene record, and this is the run's first one.

### Corrections

1. **The mission-tree reader fills one side-block home and every consumer reads another.**
   `read_mission_record_005c6a70` in `src/mission_tree_data.cpp` writes
   `MissionRecordData::sides`, the `MissionSideBlockData` pair, but `mission_side_index`,
   `mission_side_block`, `mission_opens_briefing`, `mission_side_index_005c27e0` and
   `run_start_selected_mission_0058bdf0` all read `MissionRecord::sides` through
   `record.screen`, which the reader never writes. A record loaded from the real table
   therefore answers every side query from a default-constructed block: the side index comes
   out 1 for every mission and `00580940` sends a United States mission to the Japanese page
   4. This was found by running it, and it is not fixed here because
   `src/mission_tree_data.cpp` belongs to another packet; the executable projects the
   reader's blocks into the screen-facing record itself and says so in the source. **The fix
   belongs in `read_mission_record_005c6a70`.** With the projection in place the run
   reproduces the documented pairing exactly.
2. **`mission_load_host_steps` records the wrong owner for the `.scn` header pass.** That
   table gives `SetPendingSceneHost::load_scene_header_pass` (0046DF00) the scene-graph
   owner. The header pass needs the reader and the VFS and nothing else: it runs concretely
   here. The scene graph is first required by the two passes inside 004D4DF0
   (`load_scene_contents`), which is the row that should carry the boundary.
3. **The numbered mission map icons are not in the shipped layout.**
   `docs/MAIN_MENU_MISSION_DETAIL.md` reads the per-mission loop as finding
   `mission_mapflag_<n>_Icon` and `mission_mappoint_<n>_Icon` under the group widget that was
   just copied into +110h. On this installation `missions_US_Group` and its four siblings are
   authored as empty groups with `Visible = false` and no children at all; only
   `mission_mapflag_template_Icon` and `mission_mappoint_template_Icon` exist, as direct
   children of the page root. All 72 lookups of the 36-mission group therefore miss, which
   the native handles the same way (00AA7E00 returning null is skipped). What clones the two
   templates into the numbered icons is neither 005861B0 nor 0058C010 and is a follow-up.
4. **005861B0 cannot reach three of its widgets with a single 00AA7E00.**
   `content_main_Text`, `test_Clipbox` and `video_Movie` are nested three levels under
   `historical_Group` in `fe_worldmap_historical.lua`, and 00AA7E00 walks direct children
   only. The native must chain lookups to get there; the executable does the same and logs
   which of the fourteen handles was a direct child (eleven) and which was nested (three).
5. **The mission id is not the scene stem.** `--menu-select` takes the record's Lua `id`,
   `USN02`; `usn_2_java` is the `sceneFile` stem that `src/mission_scene_probe.cpp` defaults
   to. An id the tree does not carry silently becomes the first mission of the first group,
   which is 005CAAF0's own clamp, and the run says so.
6. **The seven top-level menu labels are not layout text.** Milestone 2d left them empty on
   screen. 00584AE0 builds them in code as list-box entries through 00AAB4C0, so no page
   authored them and the sprite bridge has nothing to draw. What the executable can do, and
   now does, is resolve each of the seven label ids through the locale table phase 6 loaded:
   Single Player, Multiplayer, Tactical Library, Options, `globals.live`, Downloadable
   Content and Quit, all seven enabled from the table at 00CEF77C. Drawing them needs the
   list-box widget owner.

### What this milestone supplies rather than recovers

- **The three page roots are hidden after the bind.** 005861B0 loads them and the page
  builder decides what is shown; 00584AE0 is not reconstructed, so the executable hides all
  three and lets 0058C010's own visibility block show the world map.
- **The scripted action edge.** `00599DB0` asks 004D92B0 whether an action fired. This
  process has no input device bound to the menu, so the step machine arms one action per
  frame instead. The page machine's own control flow is untouched.
- **The movie completion is run directly.** When a record does carry a `MovieName`, 005922F0
  leaves the start to 0058D9D0, which the movie player calls when the clip ends. The movie
  player is another owner's, so the executable runs the completion itself and logs it.
  `USN02` has no movie, so this path was not taken in the validation run.
- **The mission script path** in the report is derived from the scene file's own base name,
  which is what `src/mission_scene_probe.cpp` does. The native indexes the record's script
  table at +928h, and the header pass does not fill it.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function and a reviewed ledger name.
No name was added; run-time evidence was appended to 00580940, 005caaf0, 0058c010, 005922f0,
0058bdf0, 004e2770, 004c6890, 0046df00 and 004dfb70. 005861b0 was left alone because it is
leased to `agent/orch5-menu-layout`.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.

```
  menu item 0 FE.main_singleplayer    enabled=1 Single Player
  menu item 6 FE_pc.main_quit         enabled=1 Quit
mission tree: 5 groups, 143 missions, 34 multiplayer entries
mission tree selection: group=2 mission=13 id=USN02 name=New - Battle of the Java Sea
        scene=universe/Scenes/missions/USN/usn_2_java.scn
  side blocks: allied enabled=1 briefing="USN02" hints=3; japanese enabled=0 ...
mission selection published: group=2 mission=13 side=0 page=5 sub_selection=-1
main-menu update arm=MissionList page=5 scene=3 epilogue=1
mission detail page built=1 page=9 group=2 missions=36 flags=0 points=0 missing_icons=72
main-menu update arm=MissionDetail page=9 scene=4 epilogue=1
scene universe/Scenes/missions/USN/usn_2_java.scn: 48680 bytes, uniqueID=1, 34 entities in
        2 classes, short name "usn_2", mission script "Scripts/missions/usn_2_java.lua"
  scene class DestroyerGen  x32  registered instantiate-only
  scene class NavPoint      x2   registered instantiate-only
  scene vfs blocks: 1_usn_2 3_usn_2 4_usn_2 5_usn_2 6_usn_2
scene record: path=universe/Scenes/missions/USN/usn_2_java.scn present=1 mission_id=1
        side_blocks=0 slots_filled=0 selected_index=0
mission load host inventory: 68 steps, 57 need a subsystem owner; the load stops at
        reset_render_scene [00874640] renderer
sprite bridge quads=705 (text_glyph_quads=692) textures=13/14 atlas_items=678 rebuild=33
summary mission select=USN02 tree=1 groups=5 missions=143 selected=USN02 list_page=5
        detail=1 requested=1 step=Stopped
host methods 193 concrete, 176 unimplemented
```

Every earlier switch was rechecked on the same binary: a 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3. All four match milestone 2d exactly.

This is a runtime-validated process, not a game-validated one. It proves that the
reconstructed mission tree, page pairing, mission-detail builder, briefing start and scene
record carry a cold boot from the main menu to a populated scene record over real installed
data, and that the executable reads the same `.scn` the fixture probe does. It proves
nothing about the mission load itself, about the world, or about binary compatibility with
the original executable.

### Follow-up packets

1. **`read_mission_record_005c6a70` should fill the screen-facing side-block projection.**
   Correction 1 above; it is a two-line fix in the producer and it removes the executable's
   bridge.
2. **The mission-list page builder 00597870**, and the other page builders 00584F50,
   005853C0 and 005886F0, so a run can navigate the menu rather than publish a selection
   into it.
3. **The page dispatcher 00598B60**, whose campaign arm at 00599318 is what calls the detail
   builder in the original.
4. **Whatever clones the two template icons into the numbered per-mission map icons.** It is
   not 005861B0 and not 0058C010, and without it the world map shows no missions.
5. **00AAB4C0 and the list-box widget**, so the seven top-level labels and the mission lists
   are drawn rather than only resolved.
6. **00588C70, the world-map drive**, and 00414130, the mission-detail offset blend: the two
   routines the page machine calls every frame that this process records.
7. **004DFB70 itself**, which needs 004DC6A0's global subsystems and then the renderer
   (00874640) and scene-graph (004D4DF0) owners. The 68-step inventory in the report is the
   worklist.

## Milestone 2f: a headless mission frame

Addresses: 004dd627 / 00884be0 (the mission Lua machine and its one call site), 006b8740 /
006b8610 / 006b89f0 / 006b8ad0 (the machine construction, the 560-row registration and the
two chunk runners), 00b6a303 with 00b69e00 (the `DoFile` global the binding table does not
carry), 004dc72f / 00886900 (the global script folders), 004e02d0 / 005e2f00 (the
`LobbySettings` table), 00885110 / 00885fb0 / 004e0a3f / 008860b0 (the script file, its
content variants and the mission chunk), 004e0a96 and 004e0c2e / 0045f520 and 004e0c3c and
004e0df3 / 0045f440 with 00b66200 (the entry points and the defined check); the load walk
004dfbcc / 00a7a440, 004dfbd8 / 004cd0f0, 004dfc0e / 0076da60, 004dfd18 / 004bb160,
004dfe65 / 0057d0c0, 004dfe6f / 0057cb60, 004dfebb / 004dc6a0, 004e0194 / 00874640,
004e019f / 006ad600, 004e01d7 / 0046df00, 004e01de / 004de610, 004e01e8 / 00951560,
004e0356 / 004f2800, 004e03e5 / 004d4df0, 004e046c / 0068a990, 004e04df / 004c9680,
004e04e7 / 004c1ac0, 004e06bc / 00aa0d30, 004e0700 / 00aa06d0, 004e085f / 0077f5e0,
004e08e4 / 004d30f0, 004e0a9b / 0095ca70, 004e0ae8 / 006b8ad0, 004e124e / 007065e0,
004e185c / 0057c250, 004e1873 / 004c9ca0, 004e0305 (the `thisTable` self table); the four
calls of the in-mission subsystem tick 004c40ad / 00875bb0, 004c40b4 / 004c3cb0, 00904bf0 and
004c40dd / 00447b80; the state 0Ch handler 004db920 with 004db95d /
00a91020 and 004db9b4 / 00a92c40; the entry 004da6c0 with 004da6df / 00447060, 004da71e,
004da734 / 00a7a440, 004da746 / 004c9ca0, 004da755 / 004d87b0 and 004da769 / 004cd0f0; and
the 57 call sites of the in-mission branch of 004e4a40 that `bsp::mission_frame_step()`
carries. Packet `cc_exe_2f`, owner `agent/cc-exe-2f`. Sources: `src/game_hosts_lua.cpp`,
`include/bsp/game_hosts_lua.hpp`, `src/game_hosts_mission_frame.cpp`,
`include/bsp/game_hosts_mission_frame.hpp`, plus edits to `src/game_hosts_mission.cpp`,
`src/game_hosts_menu.cpp`, `src/game_hosts.cpp`, `src/game_main.cpp` and their headers.
Report: `reports/game_executable_milestone_2f.json`. Ghidra was read-only for this packet.

Milestone 2e reached the mission load request and stopped in front of the first
renderer-owner host. This milestone carries the run past it: the load walks the whole of its
recovered inventory, the mission Lua machine comes up on real Lua 5.1.1 and runs the
installed mission's script, the state 0Ch handler enters the mission, and the in-mission
branch of `GGame::OnMove` runs for as many frames as `--mission-frames` asks for.

### The new switch

`--mission-frames N` names how many in-mission frames to run once the load has finished and
`004da6c0` has written game state 0Dh. Zero, the default, runs none. Every earlier switch is
unchanged, and a run without `--menu-select` is byte-for-byte the milestone 2d run.

### 1. The load walks its whole inventory

Milestone 2e logged the 68-step inventory of `mission_load_host_steps` and stopped at the
first step whose owner is the renderer or the scene graph. Milestone 2f walks the 47
`MissionSceneLoadHost` rows of that table in order: **10 run in process, 32 take the explicit
unimplemented policy with the neutral value the reconstruction documents, and 5 are
session-mode arms the single-player load never takes**. The walk ends by writing the value
004e086b writes, so the run leaves `game+5D4h` at 0Ch.

**This is the milestone's own driver, and it is not `bsp::run_mission_scene_load`.** That
routine's step at 004dfebb hands the caller a `bsp::GlobalSubsystemInvocation`, a bundle of
references to twenty configuration contexts that nothing in the repository builds, so the
recovered driver cannot be entered from this process at all. What the walk keeps is the
table's order, which is the reconstruction's own recovered ordering; what it does not keep
are the driver's arm guards, which is why the five session-mode steps are reported as arms
not taken rather than as performed. Entering the real driver is follow-up 1.

The ten steps that run:

| Step | Call site | Callee | What it does here |
| --- | --- | --- | --- |
| `set_cinematic_mode` | 004dfbd8 | 004cd0f0 | both cinematic bytes through the recovered rule |
| `reset_single_player_slots` | 004dfd18 | 004bb160 | the eight 118h records at game+1008h, slot 0 claimed |
| `release_main_menu_manager` | 004dfd90 | 00e198ac | the global is nulled; the frame reads the same flag |
| `release_mission_result` | 004dfe83 | — | game+7188h cleared, which is why the exit is unreachable |
| `global_subsystems` (one inner step) | 004dc72f | 00886900 | 21 global and autoload scripts |
| `sync_lobby_settings_from_lua` (was `lua_reset_state`, correction 3) | 004e02d0 | 005e2f00 | the `LobbySettings` table, values excepted |
| `input_update` | 004db9b4 | 00a92c40 | the reconstructed action records, no backend |
| `run_mission_script` | 004e0a3f | 008860b0 | `Scripts/missions/usn/usn_2_java.lua` |
| `lua_call_entry_point_a` | 004e0a96, 004e0c2e | 0045f520 | `luaPrecacheUnits`, `luaStageInitMulti` |
| `lua_call_entry_point_b` | 004e0c3c | 0045f440 | `luaStageInit` |
| `publish_mission_id` | — | 00f8a2fc | +48h from record+1098h |

The five numbered VFS file blocks the load opens are named from the recovered short name:
`1_usn_2 3_usn_2 4_usn_2 5_usn_2 6_usn_2`.

### 2. The mission Lua machine

`GameMissionLuaHost` implements `bsp::MissionLuaHostServices`, so the order is 00884be0's
order rather than one the process invents: `bsp::initialise_mission_lua_host` drives the
bring-up, `bsp::run_lua_chunk` the two embedded chunks, `bsp::run_script_with_variants` the
mission chunk and `bsp::call_entry_point_if_defined` the entry points. The interpreter is the
repository's stock Lua 5.1.1, which `docs/MISSION_LUA_MACHINE.md` establishes as the matched
library once `LUA_COMPAT_LSTR` is 2.

What one run performs, in that order:

| Step | Native site | Result on this installation |
| --- | --- | --- |
| state, panic, GC pause | 006b8740 | one `lua_State`, `LUA_GCSETPAUSE` 100 |
| standard libraries | 006b8790 | 7 opened, `package` and `require` absent |
| `PC=true` | 006b8ad0 | ran |
| binding table | 006b8610 | **560 globals**, first `Log`, last `TerminateExecution` |
| fundamentals | 00884c94 | ran from the mounted `Scripts/fundamentals.lua` |
| `DoFile` | 00b6a303 | installed from the owner layer, called 12 times |
| global folders | 00886900 | **21 scripts**, no chunk error |
| `LobbySettings` | 005e2f00 | table created with its 13 fields |
| mission chunk | 008860b0 | 1 chunk, no content variant, loaded and ran |
| self table | 004e0305 | `thisTable` created empty, `recon` cleared |
| entry points | 0045f520, 0045f440 | 3 dispatched, all status 0 |

The bindings are real Lua globals whose bodies are host records: a script that calls one gets
the call logged with its name and argument count and a nil result, and the row's own address
goes on the record. `luaStageInit` reached three of them, and only three:

| Binding | Row address | Calls |
| --- | --- | --- |
| `CreateScript` | 00898750 | 1 |
| `Scoring_RealPlayTimeRunning` | 008b87f0 | 1 |
| `LoadMessageMap` | 008c61c0 | 1 |

`luaEngineMovieInit` was not called: 004e0a50 reads the raw script slot and only slot 9 takes
that arm, and this mission resolves to slot 8.

Packet `cc_mission_natives` merged during this packet's turn, and two of its findings are in
the run. The load creates the `thisTable` self table at 004e0305 and clears `recon` on the
same pass; the table is empty here because 00928a00 fills one slot per entity and this process
creates none. And the nineteen entity-returning rows of the table take their recovered tail:
`bsp::mission_binding_returns_entity` selects them, and each one pushes the nil of 0089903C,
the arm the native takes when the `thisTable` lookup produced nothing, and returns one value.
That matters to a script, because a binding that returns one value is not the same as one that
returns none. `CreateScript`, which this mission's stage init calls, is one of the nineteen.

**The mission script name is the executable's derivation, not the record's field.** 008860b0
takes the name from the scene record's script table at +928h, which the header pass does not
fill (milestone 2e). `Scripts/missions/` holds no loose `.lua`, so the name must carry a
subdirectory (`docs/MISSION_LUA_MACHINE.md`, gap 2). The executable derives it from the scene
file's own parent directory folded to lower case, checks the result against the mounted tree
and falls back to each of the eight installed subdirectories in turn. For
`universe/Scenes/missions/USN/usn_2_java.scn` that gives `Scripts/missions/usn/usn_2_java.lua`,
which resolves.

### 3. The state 0Ch handler and the entry

`bsp::run_mission_device_wait` runs the single-player input-device arm. 00a91020 at 004db95d
has no backend in this process, so nothing would ever report a button down and the arm would
wait forever. **The executable injects one device-down sample**, which the recovered latch at
table+DDh turns into exactly one rising edge; that injection is the executable's own, the same
substitution `--press-start-frame` makes for the title page. The arm then runs 00a92c40 at
004db9b4 over the reconstructed action records and tails into 004da6c0.

`bsp::run_mission_state_entry` then runs. It writes 0Dh, releases the deferred dynamics list
at 004da6df (0 handles: the list is empty here), arms the five one-shot bytes all clear with
`+1EE3h` = 0 for a single-player session, requests 004c9ca0 with 0 at 004da746, skips 004d87b0
at 004da755 because the session is not networked, and clears both cinematic bytes at 004da769.
That last write is what opens the simulation gate, and it is why all 60 frames that follow
report `simulated=1`.

### 4. The frames

`bsp::run_mission_frame` executes the in-mission branch in native order with the recovered
guards. Of its 57 steps **28 run a reconstruction in process and 29 are records**. The
concrete ones are the mission-start counters (004b6260, 004bcaa0), the warning director
(00987590), the input effect sets (004e4e6c), the action deadlines (004d8cd0), the four
profiler brackets, the pause-gate decision (004e5153), the in-mission subsystem tick
(004c40a0), two of the seven hint passes, the bot scheduler, the markers, the entity manager,
the decals, the power-ups, the activation flush, the mission-completion poll (004d7ea0), the
particle clock, the GUI visibility decision, the menu request service (006840f0) and the sound
request queue (00941140).

Step 9, 004c40a0, is concrete because packet `cc_mission_tick` merged during this packet's
turn. Two of its four calls run a reconstruction: the fixed-step driver 00875bb0 and the
dynamics frame pass 00447b80. **The driver's four-test gate opens on values this process
actually holds** rather than on invented ones: the load's own 004bb160 / 004bb440 step claimed
slot 0 and the entry's 004da71e wrote its +10h ready word, which is exactly what
00875bb1..00875c02 tests. The clock then behaves as the rule says it should. A 60 frame run
accumulates less than one 0.05 second step and runs **0** of them, and a 2000 frame run runs
**19**, which is 0.95 seconds of simulated time; the job waves and the sixteen per-step
subsystem calls behind each step are records. Call 2's guard and its latch at game+193Ch run
and fire once, and calls 3 and 4's own containers are empty.

**The frame runs with no world.** Every container those routines walk is empty, so each one
runs to its own end over nothing. That is a real run of the recovered routine and it is not a
claim that the game's world was ticked; the per-frame log line reports `units=0 events=0`
because there are no units and no events, not because the passes were skipped.

Per-frame totals over the 60 frame run: 60 simulated, 0 paused, 0 units, 0 events, 0 input
entries erased, 3 script calls (the three `luaStageInit` made, which no frame adds to).

### 5. The exit

`docs/MISSION_RESULT_DECISION.md`'s path out of state 0Dh has exactly one producer inside the
frame, 004d7ea0, and it needs a mission-result object at game+7188h. 004dfe83 released the
previous one during the load and nothing in this process builds a new one, so the poll returns
on its first test every frame, state request 0Fh is never enqueued and 004d7970 is
**unreachable here**. The run says so in its summary rather than inventing a result object.
The frame loop therefore ends because `--mission-frames` was satisfied, and the process exits
0.

### What it looks like on screen

Nothing changes when the mission starts. The window still shows the mission-detail page
milestone 2e described: the Pacific theatre world map with the historical panel's briefing
text, 705 sprite-bridge quads of which 692 are glyphs. The reason is that the in-mission frame
draws through 004ca440 and 004ca1f0, both records, and the pages the sprite bridge draws
belong to the front end, which the load released as a host record rather than by tearing down
the bridge. **A mission is running behind an unchanged picture.** Putting the mission on screen
needs the renderer owner, which is follow-up 3.

### Host methods

`bsp_game.exe --frames 300 --press-start-frame 30 --menu-select USN02 --mission-frames 60
--log local/game_run.log --screenshot local/run.png --game-root "<install>"`, exit 0:
**246 concrete, 243 unimplemented**. Milestone 2e's run on its own tree reported 193 and 176.
A run closed with `CloseMainWindow` instead of a frame limit reports 247 concrete, because it
also reaches `CloseRequestPolicy::front_end_branch` (004ca2f0); a 2000 frame run reports 246
and 246, because it reaches three more of the fixed-step driver's records.

The full per-step table, with the call site and callee of every row, is
`reports/game_executable_milestone_2f.json` (`machine_steps`, `load_steps`, `entry_steps`,
`frame_steps`). The counts by group:

| Group | Steps | Concrete | Records |
| --- | --- | --- | --- |
| The Lua machine | 17 | 14 | 3 |
| The load walk | 47 rows of the inventory | 10 | 32 (5 arms not taken) |
| State 0Ch and the entry | 11 | 6 | 5 |
| The in-mission frame | 57 | 28 | 29 |
| The subsystem tick inside step 9 | 4 | 3 | 1 |

Three of the load's records are worth naming because they are the boundary the next packets
have to cross: `global_subsystems` (004dc6a0, world), `reset_render_scene` (00874640,
renderer) and `load_scene_contents` (004d4df0, scene graph). The last one is where the
entities would appear.

### Corrections

1. **Milestone 2e's "the load stops at `reset_render_scene [00874640]`" is superseded.**
   00874640 is still the first renderer-owner step and is still a record; it is no longer
   where the run stops. The 2e host table's 54 load-inventory records are now 32 records, 10
   concrete steps and 5 arms not taken.
2. **`include/bsp/mission_lua_host.hpp` cites the wrong call site for `luaPrecacheUnits`.**
   Its `kMissionLuaEntryPoints` comment reads `0045f520 at 004e0aa5`. The live listing has
   `CALL 0045f520` at **004e0a96** and `CALL 0095ca70` at 004e0a9b, which is the first of the
   eleven precache calls the same file attributes to 004e0a9b..004e0acd; 004e0aa5 is inside
   that block. `python tools/bsp.py ghidra xrefs 0045f520` reports only 004e0a96 and 004e0c2e
   inside 004dfb70, and `disasm-raw 004e0a90 --length 24` shows both instructions. The fix
   belongs in that header.
3. **`MissionSceneLoadHost::lua_reset_state` is named from its call site, not its body.**
   005e2f00 is `BSP_Game_SyncLobbySettingsFromLua`: it opens the `LobbySettings` global and
   walks the thirteen slots of 00e08908, and it resets no Lua state. That is exactly the
   mistake rule 1 of `docs/WORKER_VERIFICATION_CHECKLIST.md` names, and
   `docs/MISSION_LUA_MACHINE.md` already recovered the body. The rename belongs in
   `include/bsp/mission_scene_load.hpp`, which this packet does not own.
4. **This packet's own first pass recorded step 9 as unimplemented.** Packet
   `cc_mission_tick` merged during the turn and the step now runs
   `bsp::run_in_mission_subsystem_tick_004c40a0` in process, together with the fixed-step
   driver 00875bb0 and the dynamics frame pass 00447b80. The same merge brought
   `cc_mission_natives`, which is where the `thisTable` self table and the entity-returning
   tail come from.
5. **Milestone 2e's mission script path was the bare form.** Its report derived
   `Scripts/missions/usn_2_java.lua` from the scene stem, and no installed script occupies a
   path directly under `Scripts/missions/`. The executable now derives
   `Scripts/missions/usn/usn_2_java.lua` and confirms it against the mounted tree; the scene
   summary line that milestone 2e printed still shows the bare form and is corrected by the
   derivation line that follows it.

### What this milestone supplies rather than recovers

- **The load driver.** The order comes from `bsp::mission_load_host_steps()`, the
  reconstruction's own table, but the walk is the executable's, so the arm guards
  `bsp::run_mission_scene_load` applies are not applied. Five steps are reported as arms not
  taken on that basis.
- **The device-wait edge.** One injected device-down sample, because the single-player arm
  polls an input backend this process does not build.
- **The mission script name**, derived from the scene path because the record's +928h script
  table is not filled by the header pass.
- **The two profiler slot indices** for the game and render blocks. 0109db08 and 0109db14 are
  both filled at run time by the counter registration 00408720 and are zero in the image, so
  the executable picks slots 2 and 3, the same substitution milestone 2c recorded.
- **The `LobbySettings` values.** The table is created with its thirteen recovered field names
  so a script can index it; every value is zero, because 005e2f00 syncs them from session
  state this process does not own. The sync itself is a record.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function and a reviewed ledger name.
No name was added; run-time evidence was appended to 004dfb70, 004db920, 004da6c0, 00884be0,
00886900, 005e2f00 and 008860b0.

`python tools/verify_report_calls.py reports/game_executable_milestone_2f.json` checks 96 call
rows and reported **51 failures, all of one kind** until the listing repair of 2026-09-11 (see the repair record in `docs/GHIDRA_LISTING_DEFECTS.md`; it now reports 96 rows checked, 0 failed): every call site of the in-mission frame
lies inside `BSP_Game_OnMove` 004e4a40, whose stored Ghidra body is the eight bytes
004e4a40-004e4a47. That is the first row of `docs/GHIDRA_LISTING_DEFECTS.md`, which says the
verifier reports call sites inside that routine as "in no Ghidra function" until
`tools/ghidra_scripts/RepairListingDefects.java` runs. The addresses themselves come from
`bsp::mission_frame_step()`, which read them from the disk bytes 004e4a40-004e5535. The other
45 rows, the machine, the load, the entry and the three call sites inside 004c40a0, all pass.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.

```
mission script name derived from the scene path: universe/Scenes/missions/USN/usn_2_java.scn
        -> Scripts/missions/usn/usn_2_java.lua (the record's +928h script table is not filled
        by the header pass)
mission lua machine: 7 libraries, 560 bindings, platform chunk=1 fundamentals=1 DoFile=1
mission load resumes past the renderer owners: scene=universe/Scenes/missions/USN/usn_2_java.scn
        short=usn_2 script=usn/usn_2_java slot=8 engine_movie_arm=0
global script folders: 21 scripts ran, 0 did not load cleanly
LobbySettings created with 13 fields, all zero: the values are the session owner's
mission script Scripts/missions/usn/usn_2_java.lua: 1 chunk(s) run, base ok=1
  entry point luaPrecacheUnits     defined=1 dispatched=1 status=0
  entry point luaStageInitMulti    defined=1 dispatched=1 status=0
  native CreateScript                 argc=1 phase=luaStageInit
  native Scoring_RealPlayTimeRunning  argc=1 phase=luaStageInit
  native LoadMessageMap               argc=2 phase=luaStageInit
  entry point luaStageInit         defined=1 dispatched=1 status=0
mission load finished: 47 steps, 10 concrete, 32 records, 5 arms not taken; game state = 0x0C
  numbered VFS blocks: 1_usn_2 3_usn_2 4_usn_2 5_usn_2 6_usn_2
device-wait edge injected: the state 0Ch arm waits on 00a91020, which needs an input backend
        this process does not build
self table "thisTable" created empty and "recon" cleared; 00928a00 adds one slot per entity
        and this process creates none
mission state entry: state=0x0D entered=1 dynamics_released=0 interface=tear down (0)
        cinematic_cleared=1 one_shots{aborted=0 end_latch=0 networked=0 dropped=0
        not_enough_players=0}
  mission frame 1 simulated=1 paused=0 units=0 events=0 script_calls=3 erased=0
  mission frame 60 simulated=1 paused=0 units=0 events=0 script_calls=3 erased=0
summary mission load finished=1 concrete=10 records=32 state=0x0D entered=1
        script=Scripts/missions/usn/usn_2_java.lua
summary mission lua bindings=560 natives=3 calls=3
summary mission fixed steps=0 at 0.050 s each (00875bb0's own clock at 00f876a4/00f876ac)
summary mission frames requested=60 ran=60 simulated=60 paused=0 units=0 events=0
        script_calls=3 interface_updates=60
summary mission exit reachable=0: no mission-result object at game+7188h, so 004d7ea0 never
        enqueues 0Fh and the debrief path of 004d7970 is unreachable in this process
host methods 246 concrete, 243 unimplemented
```

The same binary with `--frames 3000 --mission-frames 2000` exits 0 and reports
`summary mission fixed steps=19 at 0.050 s each`, with the job waves and the per-step
subsystem block recorded 19 times each.

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3. All four match milestone 2d exactly.
The close path was validated by sending WM_CLOSE to a running process with
`--press-start-frame 30 --menu-select USN02 --mission-frames 60` and no frame limit: it
presented 37507 frames, ran all 60 mission frames, recorded
`CloseRequestPolicy::front_end_branch [004ca2f0]` once and exited 0.

This is a runtime-validated process, not a game-validated one. It proves that the recovered
load inventory, the mission Lua machine, the state 0Ch handler, the mission entry and the
in-mission branch of `GGame::OnMove` run end to end over real installed data, and that the
installed mission's script loads and its stage init runs on a machine built the way 00884be0
builds one. It proves nothing about the world, about the renderer, or about binary
compatibility with the original executable: no entity was created, nothing was drawn by the
mission, and every subsystem the frame ticked was empty.

### Follow-up packets

1. **`bsp::run_mission_scene_load`'s own driver.** Standing up a `GlobalSubsystemContext` the
   executable can own would let the recovered driver run instead of this milestone's walk,
   which is the only way the load's arm guards are applied. It is the same composition
   question milestone 2d's follow-up 2 raised for the scene graph, one layer up.
2. **`load_scene_contents` 004d4df0**, the Registration and Instantiate passes. Until it runs,
   the mission has no entities, which is why every world pass of the frame ticks nothing. It
   needs the scene graph and `construct_world` 004de610 before it.
3. **The mission render block**, 004ca440 and 004ca1f0, which is `mission_frame_render_entry`
   in `docs/MISSION_STATE_FRAME.md`'s follow-up table. Without it a running mission cannot
   change what is on screen.
4. **The fixed step's own body**, 00875cc0's three job waves and the sixteen per-step calls
   at 00875e0c. The driver and its clock now run; what each step would do is still a record,
   and it is where the simulation's own work lives.
5. **A mission-result object at game+7188h**, so the exit path `docs/MISSION_RESULT_DECISION.md`
   reconstructs becomes reachable and a headless run can finish a mission rather than stop at
   a frame count.
6. **Entity-returning bindings that return a real entity.** The nineteen rows now take their
   recovered nil tail, which is correct for a process with no entity; filling `thisTable`
   needs 00928a00 and therefore the scene contents of follow-up 2. That is what
   `docs/MISSION_LUA_MACHINE.md`'s six failing probe scripts are waiting for.
7. **The two header fixes in corrections 2 and 3**, both one-line changes in files this packet
   does not own.


## Milestone 2g: the fixed step's body and the way out of the mission

Addresses: the sixteen per-step calls 00875e0c, 00875e24, 00875e33 / 00875e3a, 00875e3f,
00875e44, 00875e55, 00875e64, 00875e91, 00875e96, 00875e9b, 00875ea2, 00875ebf, 00875ec4,
00875ec9, 00875eda with the world gate 00875e69-00875e7f, the three job waves 00875cc0, the
interpolation wave 00875f43 and the tail gate 00875fd1-00875ff7; 0089a5e6 / 0089a390 with
0089a40e (004cd390) and 0089a469 (004d7970), the EndScene body 004d79b7 / 004d79eb /
004d7a0f / 004d7a23 / 004d7a37 / 004d7a42 / 004d7a78 / 004d7acc / 004d7aec, the poll
004e5389 (004d7ea0) with 004d7f32 / 004d7f42 / 004d7f72, the drain 004e4d0d (004e4430) with
its tail 004e4a2e, the request 10h arm 004e458a with 004e4710 / 004e4715 / 004e4722 /
004e474a / 004e4763 / 004e476c / 004e4773 / 004e4778 / 004e4782 / 004e47a7, the state 11h
arm 004e504b-004e5097 and the request 04h dispatch 004e44fa (004e4000). Packet `cc_exe_2g`,
owner `agent/cc-exe-2g`. Sources: `src/game_hosts_fixed_step.cpp`,
`include/bsp/game_hosts_fixed_step.hpp`, `src/game_hosts_mission_result.cpp`,
`include/bsp/game_hosts_mission_result.hpp`, plus edits to
`src/game_hosts_mission_frame.cpp`, `src/game_hosts_mission.cpp`, `src/game_hosts_menu.cpp`,
`src/game_hosts.cpp`, `src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2g.json`. Ghidra was read-only for this packet.

Milestone 2f ran the fixed-step driver with all four of its blocks as records and ended the
run on a frame count, because nothing built the mission-result object the exit needs. This
milestone runs the step's own body over the two reconstructions that have since landed, and
makes the mission end the way the game ends it.

### The new switch

`--mission-complete-frame N` makes, on in-mission frame N, the call a mission script's
`PlayBinkMovie(name, true)` makes: 0089a480 to 0089a390 to 004cd390. Negative or absent
injects nothing and the run ends on `--mission-frames` exactly as 2f did. Every earlier
switch is unchanged.

### 1. The fixed step's own body

`FixedStepHost`'s four methods are no longer records. `GameFixedStepHost` implements
`bsp::FixedStepFanoutHost` and `bsp::FixedStepJobWaveHost`, so the order is
`docs/FIXED_STEP_FANOUT.md`'s and `docs/FIXED_STEP_JOB_WAVES.md`'s rather than one the
process invents.

**The sixteen per-step calls.** One of them has a reconstruction on main and runs:
`apply_dynamics_buoyancy_004462d0` at 00875e24, over the same `bsp::GameDynamicsState` the
frame's fourth tick call walks. Ten are records with their owner area. The remaining five,
rows 9 to 13, are **skipped every step**, because the world gate at 00875e69 reads
`[[game+19CCh]+4ACh]` and this process builds no world object: `construct_world` 004de610 is
a load record. That is the native behaviour for an inactive world, not a substitution, and it
is why 00874c90 never splices a pending tick registration into a group.

| Row | Site | Callee | Owner | In this run |
| --- | --- | --- | --- | --- |
| 1 | 00875e0c | 00c5c540 | physics | record |
| 2 | 00875e24 | 004462d0 | unit | **concrete** |
| 3+4 | 00875e33 / 00875e3a | 0042e630 then 0098bdb0 | registry | record |
| 5 | 00875e3f | 00874de0 | engine | record |
| 6 | 00875e44 | 00926700 | entity | record |
| 7 | 00875e55 | 00888230 | script | record |
| 8 | 00875e64 | 00929460 | script | record |
| 9-13 | 00875e91-00875ebf | 00778450, 0077ec20, 00874c90, 00925f20, 0076ffc0 | session, engine, entity | skipped, world gate closed |
| 14 | 00875ec4 | 00926700 | entity | record |
| 15 | 00875ec9 | 009273a0 | entity | record |
| 16 | 00875eda | 00903610 | world | record |

**The four job waves.** All four run the recovered loop over the five 68h groups at
00f876c0, and every group is empty: an element is built only by 00875890, whose nine
construction sites are unit, aircraft and objective paths this process never reaches, and the
one routine that would move a constructed element into a group is row 11 behind the closed
world gate. So the waves admit nothing, queue nothing and dispatch nothing, and the four job
bodies and the pool's two slots are host methods that were not reached. The waves still run:
the three per-step waves make 15 group visits per fixed step and the interpolation wave 5 per
frame, 9950 of them in the long run.

**The tail hook.** 00875fd1's four tests are evaluated once per driver call and all 1990
evaluations found the gate closed: 00f8ab04 has no writer in this process and the in-mission
interface manager 00e198c4 is the front-end owner's. The hook 00a317f0 is a single `RET` in
this build, so the closed gate costs nothing. `bsp::run_fixed_step_driver_00875bb0` stops
after the interpolation wave and carries no host method for the tail, so the executable runs
it at the driver's own tail position and only when the driver's gate let the body run.

A 120 frame mission run accumulates less than one 0.05 s step and runs **no** fixed step at
all, exactly as 2f reported; the 2000 frame run runs **18**, and it is the run that exercises
the fan-out.

### 2. The mission result and the exit

`docs/MISSION_RESULT_DECISION.md`'s producer is the mission script, and this executable has
no script that reaches it, so `--mission-complete-frame` makes the call instead. What follows
the call is recovered:

| Step | Site | What the run did |
| --- | --- | --- |
| `PlayBinkMovie` binding | 0089a5e6 | the binding body 0089a480 is a record; its two arguments are the executable's |
| prefix and allocate | 0089a3fc, 0089a40e | `movies/usn_2`, the 24h object at game+7188h with +21h set and +8h = -200.0f |
| **EndScene, synchronously** | 0089a469 | 0089a390 calls 004d7970 itself when the debrief byte is set |
| the body, once per scene | 004d79b7-004d7a47 | debrief bring-up recorded, the record committed through 009205e0, metrics recorded, game+1EE2h latched |
| enqueue 10h | 004d7aec | game+5D4h was 0Dh, so the teardown request is queued |
| the poll | 004e5389 | **does not run**: its first test at 004d7ea4 wants an empty queue and 10h is in it |
| drain 10h | 004e4d0d, 004e458a | 004da780 recorded, the result's movie handed to 004f8a20, 004f89d0 registered, the result released at 004e4773, state 11h, suspension raised |
| the state 11h arm | 004e504b-004e5087 | game+7184h is clear, so 004e506b lowered the suspension and 004e5087 enqueued request 04h |
| drain 04h | 004e44fa | the front-end shell, which after a mission is the debrief front end, recorded |

The run then has nothing left to do in a headless process, so it requests the application
loop exit through the same byte `--frames` uses and **exits 0 on the exit path rather than on
the frame count**. Two frames separate the injection from the front-end request.

Three things on that path are the executable's own and are marked in the source: the call
itself, the movie that never plays (004f8a20 needs the GUI movie widget), and the decision to
record the request 04h dispatch instead of re-entering the menu host's shell. The scoring
record 009205e0 commits is the zeroed one the load produced, because
`Scoring_SetMissionCompleted` 008b8ad0 is the script's and this mission's stage init never
reaches it.

### Host methods

`bsp_game.exe --frames 400 --press-start-frame 30 --menu-select USN02 --mission-frames 120
--mission-complete-frame 90 --log local/game_run_2g.log --game-root "<install>"`, exit 0:
**252 concrete, 252 unimplemented**. The same binary with `--frames 3000 --mission-frames
2000 --mission-complete-frame 1990` reports **254 and 261**, because it runs 18 fixed steps
and therefore reaches the fan-out's own records. A `--mission-frames 60` run with no
`--mission-complete-frame` reports 247 and 243, against milestone 2f's 246 and 243 on its own
tree: the interpolation wave moved from record to concrete and the frame's new request drain
added one record, `Drain::post_drain`.

The methods this packet introduced. Everything not marked concrete is the unimplemented
policy with its native call site on the record; the call counts are the 2000 frame run's.

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `FixedStep::run_job_waves` | `00875cc0` | concrete | 18 |
| `FixedStep::interpolation_wave` | `00875670` | concrete | 1990 |
| `FixedStepFanout::simulate_physics_world` | `00875e0c` | **unimplemented** | 18 |
| `FixedStepFanout::apply_dynamics_buoyancy` | `00875e24` | concrete | 18 |
| `FixedStepFanout::refresh_moved_spatial_nodes` | `00875e3a` | **unimplemented** | 18 |
| `FixedStepFanout::run_step_callbacks` | `00875e3f` | **unimplemented** | 18 |
| `FixedStepFanout::drain_deferred_entity_events` | `00875e44` | **unimplemented** | 36 |
| `FixedStepFanout::drain_queued_lua_calls` | `00875e55` | **unimplemented** | 18 |
| `FixedStepFanout::run_due_entity_think` | `00875e64` | **unimplemented** | 18 |
| `FixedStepFanout::flush_pending_entity_queues` | `00875ec9` | **unimplemented** | 18 |
| `FixedStepFanout::release_expired_world_objects` | `00875eda` | **unimplemented** | 18 |
| `MissionEndMovie::lua_binding_play_bink_movie` | `0089a480` | **unimplemented** | 1 |
| `MissionEndMovie::prefix_movie_name` | `0089a3fc` | concrete | 1 |
| `MissionEndMovie::set_end_of_mission_movie` | `0089a40e` | concrete | 1 |
| `MissionEndScene::end_scene` | `004d7970` | concrete | 1 |
| `MissionEndScene::debrief_bringup` | `004d79b7` | **unimplemented** | 1 |
| `MissionEndScene::multiplayer_score_query` | `00f8a2fc+vtable198` | **unimplemented** | 1 |
| `MissionEndScene::record_metrics` | `004d7a42` | **unimplemented** | 1 |
| `MissionResult::enqueue_state_request` | `004d3ed0` | concrete | 1 |
| `MissionTeardown::tear_down_mission` | `004e4710` | **unimplemented** | 1 |
| `MissionTeardown::play_end_movie` | `004e474a` | **unimplemented** | 1 |
| `MissionTeardown::set_movie_completion` | `004e4763` | **unimplemented** | 1 |
| `MissionTeardown::close_result_gui` | `004e476c` | **unimplemented** | 1 |
| `MissionTeardown::release_result` | `004e4773` | concrete | 1 |
| `Drain::request_04_front_end_shell` | `004e4000` | **unimplemented** | 1 |
| `Drain::post_drain` | `00a95960` | **unimplemented** | 1992 |
| `MissionCompletion::mission_result` | `004d7ea0` | concrete | 1989 |

More host methods exist and were not reached, and the report lists each with the reason: the
five gated fan-out rows, the four job bodies with the pool's queue and dispatch, the tail
hook, the six `DynamicsBuoyancyHost` methods behind an empty list, the aborted and
multiplayer arms of EndScene, the poll's own movie calls, the objective setters no script
touched, and the whole movie-player boundary the completion 004f89d0 would cross. `MissionCompletion::mission_result` was read on 1989 of the
1990 frames: the one frame it was not is the frame the synchronous EndScene had already
queued request 10h, which is the run-time evidence for correction 1.

### Corrections

1. **The poll is not how a mission that plays an end movie leaves state 0Dh.**
   `docs/MISSION_RESULT_DECISION.md`'s chain has the script raise the movie, 004d7ea0 enqueue
   0Fh and the drain dispatch it. The decompiled body of 0089a390 shows it calls 004d7970
   **itself** at 0089a469 whenever the script's `goToDebrief` byte is set, and EndScene then
   enqueues 10h at 004d7aec because game+5D4h is 0Dh. The poll's own first test at 004d7ea4
   wants an empty request queue, so with 10h already queued the poll never runs and its 0Fh
   arm is unreachable from a `PlayBinkMovie(name, true)` issued in a running mission. The run
   log shows exactly that: the poll ran on every in-mission frame and read game+7188h on all
   but the one frame the injection fired on. The chain's 0Fh arm needs the result object to
   survive into a frame with an empty queue, which is another call order, not this one.
2. **The drain suspension's lifecycle is closed.** `docs/GAME_FRAME_CONTROL.md` left it open
   ("game+5ECh is cleared by 004c7ed0 and written by 004ddb90; neither was analyzed"). Both
   sites that latch the byte also register the routine that clears it: 004d7f42 in the poll
   and 004e4763 in the teardown arm both call 004f8970 with 004f89d0, and 004f89d0 calls
   004c7ed0, which clears game+7184h at 004c7ed7 and game+5ECh at 004c7efe. The state 11h arm
   at 004e506b is the second clear, and it only fires while game+7184h is clear, so a movie in
   flight holds the suspension until its completion runs. Without that pair a mission whose
   poll enqueued 0Fh would latch the drain and never dispatch its own request.
3. **`result+8h` is a GUI depth, not a score.** The same doc left the constant unread and
   `bsp::MissionResult::score` in `include/bsp/game_frame_control.hpp` inherited the guess.
   00ce77e4 holds `00 00 48 C3`, -200.0f, and 004d7f1d loads it as the third stack argument of
   004f8a20, which `bsp/movie_player.hpp` recovers as the movie widget's local Z; the drain's
   teardown arm loads 1000.0f from 00ce3804 for the same argument at 004e4732. The rename
   belongs in that header, which this packet does not own.
4. **The request 10h arm's two effects are alternatives, not a sequence.**
   `docs/GAME_FRAME_CONTROL.md` reads it as "leaves state 11h at 004e4778, enqueues 04h at
   004e47a7 when the queue is empty". 004e4722 tests game+7188h first: with a result object
   the arm plays it, registers the completion, closes the result GUI, releases the result,
   writes state 11h, raises the suspension and **jumps over** the enqueue at 004e4789; with no
   result object it falls to 004e478b and enqueues 04h. It also tests game+1EE3h at 004e4715
   and takes the 004e48d7 sub-path when the session is networked.
5. **`post_drain_00a95960`'s call site.** `include/bsp/game_frame_control.hpp` cites 004e4a24,
   which is the `FLD` of the scaled delta; the `CALL 00a95960` is at 004e4a2e.
6. **Milestone 2f's `summary mission exit reachable=0` is superseded.** What was missing was
   the producer, not the poll.
7. **`docs/GHIDRA_LISTING_DEFECTS.md`'s 004e4a40 row no longer bites.** The stored Ghidra body
   of `BSP_Game_OnMove` is now 004e4a40-004e5537, so `tools/verify_report_calls.py` checks
   call sites inside it: this packet's report passes 42 of 42 rows with no known-defect
   exemption, where milestone 2f needed one for 51 rows.
8. **`PlayBinkMovie`'s debrief flag is its third argument and defaults to false.**
   `docs/MISSION_RESULT_DECISION.md` reads the binding as `PlayBinkMovie(name,
   goToDebrief)`. 0089a480 zeroes the byte at 0089a5a2 (`XOR BL,BL`), asks for the argument
   count at 0089a5a4 and only reads a boolean when there are **three or more** (`CMP EAX,3`
   at 0089a5a9), taking argument index **2** at 0089a5ae; `MOV DL,BL` at 0089a5e0 is what
   0089a390 receives. The installed scripts match: `Scripts/global/commandhelpers.lua`, one
   of the 21 global scripts this run loads, calls `PlayBinkMovie(Mission.MissionComplParams
   .Movie, "", true)` in `luaMissionCompleted_ComplMovie` and the same three-argument form
   with `MissionFailParams` in `luaMissionFailed_FailMovie`, while a mid-mission movie such
   as `PlayBinkMovie("campaigns/BSM/m0102.bik")` passes one argument and therefore never
   ends a mission. Win and loss reach the same native call, which is what that doc's summary
   already says.
9. **A mission with no completion movie ends through a different binding, and that binding
   is gated on the scoring record.** The same helper's other arm is `luaDelay(EndScene, 12)`,
   and `EndScene` is binding row 008b01b0. Its body reads the commit slot's record,
   `[game+21A0h] + 4 + [[game+21A0h]+1424h]*284h`, and when that record's `+0h` is clear in a
   single-player session it raises a menu prompt through `BSP_MenuPromptScreen_Raise` and
   returns **without** calling 004d7970; only a completed record, or a multiplayer session,
   reaches `BSP_Game_EndScene`. A script therefore has to call `Scoring_SetMissionCompleted`
   before it ends a mission this way. The executable's own record is zeroed, so this packet
   injects the movie arm rather than this one.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function and a reviewed ledger name;
the eight job-wave vtable targets `docs/FIXED_STEP_JOB_WAVES.md` listed have since been
defined by the integrator. No name was added.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2g.json` checks 42 call
rows and reports 0 failures.

```
mission complete injected on frame 90: the executable calls the path a script's
        PlayBinkMovie(name, true) takes
mission end movie requested: name="movies/usn_2" go_to_debrief=1 result+8h=-200.0
        (00ce77e4, the movie's local Z at 004f8a20)
state request 0x10 enqueued, queue=1
GGame::EndScene aborted=0 first_run=1 commit=1 teardown=1 broadcast=0 state=0x0D
mission teardown: the end movie was handed to 004f8a20, the completion 004f89d0 was
        registered, the result object was released and the game left state 0Dh for 11h with
        the drain suspended at game+5ECh
mission end wait: state 11h, game+7184h clear, so 004e506b cleared the drain suspension and
        enqueued request 04h
  mission exit frame 1: game state 0x11, requests queued=1
  mission exit frame 2: game state 0x04, requests queued=0
summary mission exit frames=2 injected=1 completed=1 state=0x04
summary mission exit reachable=1: the mission ended through 004d7970: request 10h, the
        teardown arm 004e458a, state 11h and request 04h
host methods 252 concrete, 252 unimplemented
```

and, from the 2000 frame run:

```
fixed-step fan-out 00875e0c, the sixteen per-step calls at 0.050 s (00d0de84); the world gate
        00875e69 is closed
   1 00875e0c -> 00c5c540 simulate_physics_world_00c5c540            physics  record
   2 00875e24 -> 004462d0 apply_dynamics_buoyancy_004462d0           unit     concrete
   9 00875e91 -> 00778450 pump_session_00778450                      session  skipped (world gate)
  fixed step 1: 11 of 16 fan-out sites ran, world gate closed, rows 9-13 skipped
fixed-step job waves: the five 68h groups at 00f876c0 are empty. Every element comes from
        00875890, whose nine construction sites are unit, aircraft and objective paths this
        process does not run, and the splice 00874c90 is row 11 behind the closed world gate
summary fixed step body steps=18 fanout_sites=198 concrete=18 records=162
        gate_closed_steps=18 gated_sites_skipped=90
summary fixed step waves groups=270 interpolation_groups=9950 elements=0 queued=0
        dispatches=0 tail_open=0 tail_closed=1990
host methods 254 concrete, 261 unimplemented
```

Every earlier switch was rechecked on the same binary: a 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3, and a `--mission-frames 60` run without
`--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`. The first three match milestone 2d exactly. The close path
was validated the same way as before, by sending WM_CLOSE to a running process with
`--press-start-frame 30 --menu-select USN02 --mission-frames 60` and no frame limit: it
presented 28722 frames, ran all 60 mission frames, recorded
`CloseRequestPolicy::front_end_branch [004ca2f0]` once, reported 248 concrete and exited 0.

This is a runtime-validated process, not a game-validated one. It proves that the recovered
fixed-step body runs with its real gates, and that the recovered end-of-mission path carries a
running mission out of game state 0Dh to the front-end request and ends the process there. It
proves nothing about the simulation the fan-out would drive: every list it walks is empty,
no job was ever queued, and no movie was played.

### Follow-up packets

1. **The five gated fan-out rows and the job-wave groups need a world.** Both are waiting on
   the same thing as milestone 2f's follow-up 2, `load_scene_contents` 004d4df0 with
   `construct_world` 004de610: a world object whose +4ACh byte is set opens the gate, and
   entities are what construct the tick elements the waves walk.
2. **The ten fan-out callees with no reconstruction**, in the order the doc's own follow-up
   table gives them: `spatial_index_rebucket`, `deferred_entity_event_queue`,
   `entity_pending_queues`, `entity_think_dispatch`, `sentity_init_all`,
   `session_step_vs_frame`, `world_expired_objects`, plus 00c5c540 and 00874de0.
3. **`bsp::run_fixed_step_driver_00875bb0` should carry the tail gate**, so a caller does not
   have to run 00875fd1 beside it.
4. **The debrief front end**, 00920a20 with the `GUI_scoring` page 0060d740 (packet
   `mission_debrief_bringup`), and the front-end shell entry 004e4000 as the mission host can
   reach it: today request 04h is recorded because the shell's screens belong to the menu
   host.
5. **`Scoring_SetMissionCompleted` 008b8ad0 and the thirty other `Scoring_*` bindings**
   (packet `scoring_binding_table`), so the record 009205e0 commits carries a result instead
   of zeros.
6. **The movie player as the executable can own it**, which would let 004f8a20 play the
   result's clip, hold game+7184h and make the completion 004f89d0 the thing that lowers the
   drain suspension, as it is in the game.
7. **The poll's own 0Fh arm** is still unexercised, and corrections 1 and 8 say why: a
   three-argument `PlayBinkMovie` in state 0Dh reaches EndScene synchronously, and a
   one-argument one leaves +21h clear. The arm needs a result object with +21h set that
   survives into a frame with an empty queue, which means a call made outside states 0Dh and
   0Fh; finding a script that makes one is the next step.
8. **The `EndScene` binding 008b01b0** and its menu prompt arm (correction 9), which is the
   other way a mission ends and the reason `Scoring_SetMissionCompleted` has to run first.


## Milestone 2h: the mission's entities, and the HUD pages on screen

Addresses: 004d4df0 with its call site 004e03e5 and the two scene-file passes at 004d54de and
004d5530; 0046cf40's per-pass split 004d433-0046d5de, with 0046d426 / 0046c550, 0046d448 /
008f2260, 0046d51a / 0095c640, 0046d531 / 0046bf70, 0046d540 (descriptor[2]), 0046d5a4
(descriptor[1]), 0046d5b2 / 00bf681b and 0046d5cd / 00922e20; 004f0520 with 004f0531, 004f053d
/ 00964790, 004f054b, 004f054f / 004c1130, 004f0584, 004f058e / 004f03c0 and 004f05b6 /
0041dd40; 004e96d0 through the thunk 004e98e0; 0046f160 and 0046f350 (the scene-database
constructor, which is what fills the always-generate set at +164h, with the inserts at 0046f2fa
and 0046f30f); 008f67b0 `CPropTreeLibrary::Load` with 008f6fc0 and 008f7100 (the property-group
and enum library); 0068a990 / 0068cc70 with the request push at 0068d73a / 004cc460; 0068aca0,
004f8530, 004d8a50 and 004f7620 (the level-1 set and its recompute); 00687300 / 00686c90 /
00683aa0 with 00686db9 / 004f83b0 and 00686e2e (the main-menu manager's destruction). Packet
`cc_exe_2h`, owner `agent/cc-exe-2h`. Sources: `src/game_hosts_scene_contents.cpp`,
`include/bsp/game_hosts_scene_contents.hpp`, `src/game_hosts_hud.cpp`,
`include/bsp/game_hosts_hud.hpp`, plus edits to `src/game_hosts_mission_frame.cpp`,
`src/game_hosts_mission.cpp`, `src/game_hosts_menu.cpp`, `src/game_hosts.cpp`,
`src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2h.json`. Ghidra was read-only for this packet.

Milestone 2g ran a mission with no world: the load recorded `load_scene_contents`, so no entity
of the selected mission was ever created, and the picture on screen was still the front end's.
This milestone runs both scene-file passes over the selected mission's `.scn`, creates the
units the instantiate pass reaches, brings up the in-mission HUD screens and puts their pages on
screen.

### The new switch

`--screenshot-mission-frame N` captures the application frame on which in-mission frame N of
004e4a40 ran, so a capture can be aimed at the HUD without counting application frames from the
injected press. It takes precedence over `--screenshot-frame`. Every earlier switch is
unchanged, and a run without `--menu-select` is still byte-for-byte the milestone 2d run.

### 1. The property-group and enum library, and why it decides everything else

The first pass at this packet created **nothing**. The gate 0046C550 resolves a single-player
mission through its mode-8 arm at 0046CC5B, which reads `GenerateInGame` out of the entity's
property bag and rejects the entity when the byte at +0Ch is clear; every `DestroyerGen` in
`usn_2_java.scn` authors an empty `"MultiType" { }` and no `GenerateInGame` at all. The mission
script settles that this cannot be the game's behaviour: `Scripts/missions/usn/usn_2_java.lua`
calls `FindEntity("DeRuyter")`, it does not create the ship.

What was missing is the schema. 0046CF40 builds an entity's bag in two steps: step 6 merges
each name of the `properties ( ... )` list into it through 008F54F0, and step 7 parses the
authored body into the same bag through 008F5A00. The groups come from a library the shipped
game carries and nothing in the repository read:

| Fact | Evidence |
| --- | --- |
| the loader is `CPropTreeLibrary::Load` | the literal at 00D1653C, pushed at 008F6812 as its scope marker |
| it tokenizes with the scene tokenizer | 838h allocation at 008F68A2, 008D9CF0 at 008F68DA, delimiters `;{}=:()` at 00D16534 |
| it parses a body with the scene property parser | 008F5A00 at 008F6AD7, and 00469B60 at 008F6A7F to intern the name |
| its two top-level keywords are `properties` and `enum` | the only other literals its body carries |
| its caller loads `.enums` then `.props` | 008F7106 and 008F711B push 00D1655C and 00D16554 into 008F6FC0 |
| the folder is enumerated through the VFS | 00886280 at 008F701B |

On this installation that is 15 files under `universe/library`: 22 property groups, 33 enum
tables and 1522 symbols. `properties MultiEntity` in `global.enums` declares
`GenerateInGame = B true` and `GenerateInEngineMovie = B true`, and
`properties GameUnit (MultiEntity)` inherits it, so every `GameUnit` entity defaults to
generating. `properties Ship(Common)` and `properties Command` supply the rest.

**008F67B0 is not reconstructed and the executable's reader is not a reconstruction of it.**
The VFS enumeration, the tokenizer and the property-block parser are all recovered code; what
the executable adds is the top-level `properties`/`enum` dispatch and the merge, and it is
labelled as a stand-in in the source with the host record `SceneContents::property_library_load
[008f67b0]`. It is here because without the schema the gate's answer is wrong for 94% of every
`.scn` in the game.

The same library also settles the `Type` values. `enum ShipClasses` is in `global.enums`, so
`Type = E ShipClasses : DeRuyter` resolves to 20 and `Party = E Party : Allied` to 0. The run
resolves all 32 units, which is the Java Sea order of battle:

| Type | Id | Party | Count |
| --- | --- | --- | --- |
| `Kagero` | 276 | Japanese (1) | 6 |
| `Shiratsuyu` | 289 | Japanese (1) | 6 |
| `PACK3_Icarus` | 265 | Allied (0) | 5 |
| `Clemson` | 25 | Allied (0) | 4 |
| `DeRuyter` | 20 | Allied (0) | 2 |
| `Fubuki` | 73 | Japanese (1) | 2 |
| `Kuma` | 70 | Japanese (1) | 2 |
| `Myoko` | 293 | Japanese (1) | 2 |
| `Northampton` | 19 | Allied (0) | 1 |
| `PACK3_Fiji` | 263 | Allied (0) | 1 |
| `York` | 21 | Allied (0) | 1 |

### 2. The scene contents pass, 004D4DF0

`GameSceneContentsHost` implements `bsp::SceneContentsHost`, so the order is
`bsp::load_scene_contents_004d4df0`'s rather than one the process invents, and each of the two
`read_scene_file` steps runs `bsp::run_scene_file_reader_0046df00` with the pass flags its call
site pushes. Both passes visit all 34 entities of the 48680-byte file.

The **always-generate set** the gate searches at 0046C741 was `docs/SCENE_ENTITY_FACTORY.md`'s
first uncertainty ("nothing was found that inserts into it"). It is filled by the scene-database
constructor: 0046F350 allocates 170h bytes, 0046F160 constructs the object stored at 00E18680,
and inside it 0046F23F builds the set at this+164h and 0046F2FA and 0046F30F insert **47h
(`Path`) and 3Dh (`Cloud`)** through 0046AC50. The sibling set at this+158h is filled in the
same block with 44h, 5Eh, 1Dh, 47h and 3Dh. Neither of this mission's two classes is exempt, so
every gate answer here comes from the mode arm or from the missing-`MultiType` branch.

What the two passes did:

| Class | Id | Seen | Generated | Registration bodies | Created | Creator |
| --- | --- | --- | --- | --- | --- | --- |
| `DestroyerGen` | 07h | 32 | 32 | 32 | **32** | 004F0520, concrete |
| `NavPoint` | 41h | 2 | 2 | 0 | 0 | 004E99B0, record |

The two `NavPoint`s generate through the gate's missing-`MultiType` branch at 0046C5A6, which
also appends a deferred entity record (0046C450, a record here); their creator is one of the
twelve fixed-size classes and has no reconstruction. The 32 ships run the whole of 004F0520:
the `Type` lookup, the descriptor resolve, the instance, the placement decision, the placement,
the name and the `Command` property, which all 32 author as `Cruise`. On the registration pass
each one runs 0095C640, 0046BF70 and `bsp::register_scene_unit_004e96d0`, which marks its
vehicle class as required for its party; the run produces **32 party-class marks**.

### 3. The in-mission HUD

Packet `cc_hud_updates` merged during this packet's turn, so the manager is not a record. The
executable runs `bsp::construct_in_game_interface_0068a990` and then
`bsp::init_in_game_interface_0068cc70` over an `InMissionInterfaceInitHost`, which means the
order the 42 screens are built and registered in is 0068CC70's own rather than one this file
invents. The 42 leaf classes are still not reconstructed: the allocation at 00BF681B and the 37
leaf constructors are records, and each screen's registry record is the executable's own, the
substitution milestone 2c makes for the seven main-menu screen classes. What the +10h override
then does is recovered: 004F71D0 into the **same 95-slot registry at 00E18B60 the front-end
screens use**, the page each screen loads through 00AA5840 and the widget names it binds through
00AA7E00. That is **42 of 42 screens, 52 page loads over 47 distinct pages, 204 of 373 named
widgets bound**. The 169 that miss are nested deeper than one level, which 00AA7E00 cannot reach
on its own; that is the same limit `docs/MAIN_MENU_MISSION_DETAIL.md`'s correction 4 records for
005861B0.

Init's last act is `004CC460(20h, 0)` at 0068D73A, which pushes **INTF_SCENE3D with a null
payload**. The executable applies exactly that id rather than choosing one: 006840F0 services
the request, 00684600 hands it to the manager's own virtual +10h, and
`bsp::apply_in_game_interface_0068aca0` takes the 20h arm. With no payload the arm is not a unit
classifier at all, it is the one path that publishes, and in a single-player session it
publishes:

| Level-1 screens | 29h, 49h, 44h, 35h |
| --- | --- |
| Level-1 input contexts | 4, 11h, 12h, 0Ch, 0Bh |
| Pages those screens hold | `GUI_powerups`, `GUI_unit`, `GUI_selector` (slot 44h), `GUI_minimap` (slot 35h) |

29h and 49h load no page. The level-1 publish runs `bsp::set_front_end_screen_set_level` at
level 1, which runs the recompute 004F7620 over the same stack the managers publish level 4
into, so the pump 004F8830 enters those four screens and 004F83B0 commits their pages'
visibility. The pump itself now runs in the mission: milestone 2f recorded
`update_interface_only [004c40f0]` because the registry had nothing in it, and it is concrete
here.

**Step 17 of the in-mission frame runs too.** 004E5252 calls the manager's per-frame update
0068C1F0 behind the gate `00E198C4 != 0 && [00E198C4]+3Ch != 0`, and milestone 2f recorded both
the gate and the call. `bsp::update_in_game_interface_0068c1f0` is `cc_hud_updates`' as well, so
the recovered body runs 90 times over the 120 frame run. Almost everything it reads belongs to
one of the 42 screen classes, to the camera at game+19FCh or to the controlled unit at 00E188D8,
and this process owns none of those, so every one of them is a record with its own address: the
spectate walk finds no units because its list hangs off the world object 004DE610 would build,
no input action fires, and the audio tail reaches 00A7B710 with `Underwater`, which is the
recovered rule's answer for a camera height and a water height that are both zero rather than a
claim about the mission. The one thing the executable supplies is the manager's own active byte
at +3Ch: 00684700 sets it through the manager's vtable slot +08h, Ghidra reports only three
direct callers of 00684700 and none of them is this manager, and nothing in the reconstructed
load reaches that dispatch, so the executable raises the byte itself and records 00684700. The
other half of 00684700, deactivating every other registered manager, has already happened,
because the load destroyed the main-menu manager.

**The front-end pages come down through recovered code.** Milestone 2f said "a mission is
running behind an unchanged picture" because the load released the main-menu manager as a
record. It is not a record: 004DFDA6 calls the manager's vtable slot 0, which the vtable at
00CF774C gives as 00687300, the deleting destructor. That calls `BSP_MainMenu_Destroy`
00686C90, which collects the seven screens it owns, runs each active one's exit virtual, clears
+4h and +5h and commits through **004F83B0 at 00686DB9**, destroys each one, and then calls
`BSP_FrontEndManager_Deactivate` 00683AA0 at 00686E2E, whose tail publishes the empty level-4
screen set and the empty level-4 input contexts. In the run the sprite bridge drops from 705
quads to 13 on that frame.

### What it looks like on screen

Everything before the mission is unchanged: the title page of milestone 2d, then the main menu,
then the mission-detail page with the Pacific world map and the briefing text. When the load
releases the main-menu manager that page disappears, and what is left is the front-end frame
(the flag, the two rails and the winged `MAIN MENU` plate, which belong to the 0x164 layout-set
singleton rather than to a registry screen, so milestone 2b's caveat about those two pages is
unchanged) and, over it, the HUD.

The HUD is the four level-1 pages, laid out where their own `.lua` puts them: the minimap
cluster in the top right (`minimap_islandmap_Icon` at 0.867, 0.165 with its compass, its glass
and the capture icons), the powerup and unit templates in the middle left, and two text runs
that resolve for real, `Medal_Text` = "Fighter Ace" in Arial15 and `WeaponInfo_Text` =
`ingame.selector_atrillery` = "Artillery" in Arial16. **The large `Error` image is the page's
own authored texture**: `minimap_islandmap_Icon` names `error.tga` with the material
`minimap_terrain.mshd`, and the running game replaces it with the mission's island map; nothing
in this process does. The templates are drawn in place because the HUD root's update 00649860,
which clones them per unit, is analysed and not reconstructed.

`drawn: yes`. 70 quads on the captured mission frame, 29 of them glyphs, against 13 with the
HUD not yet up. The capture is written to the ignored `local/run_2h.png` and is not committed.

### The unit passes still tick nothing, and why

This is the one number the milestone does not improve. 32 unit records exist and **every unit
pass of the frame and of the fixed step ticked 0 of them**. That is not an empty scene: every
container those passes walk hangs off the world object `construct_world` 004DE610 would build
at game+19CCh, and that step is still a load record. So the entity manager reference at
game+21A0h is null, 00481640's gate at world+4ACh is closed and the fixed step's gate at
00875E69 reads a world that does not exist, which is why rows 9 to 13 of the fan-out are still
skipped exactly as milestone 2g reported. `construct_world` is now the single step between this
executable and a ticking simulation.

### Host methods

`bsp_game.exe --frames 400 --press-start-frame 30 --menu-select USN02 --mission-frames 120
--mission-complete-frame 90 --screenshot local/run_2h.png --screenshot-mission-frame 60 --log
local/game_run_2h.log --game-root "<install>"`, exit 0: **290 concrete, 318 unimplemented**.
Milestone 2g's run on its own tree reported 252 and 252; on the tree this packet branched from,
before this packet, the same command reported 275 and 288. A `--mission-frames 60` run with no
`--mission-complete-frame` reports 283 and 300, and a run closed with `CloseMainWindow` reports
284 and 300.

The methods this packet introduced, grouped. Everything not marked concrete is the
unimplemented policy with its native call site on the record; the full per-step table with the
call site and callee of every row is `reports/game_executable_milestone_2h.json`.

| Host method | Native call site | Status | Calls |
| --- | --- | --- | --- |
| `MissionLoad::load_scene_contents` | `004d4df0` | concrete | 1 |
| `SceneContents::property_library_load` | `008f67b0` | **unimplemented** | 1 |
| `SceneContents::read_scene_file` | `008d9cf0` | concrete | 2 |
| `SceneContents::read_scene_file_registration` | `0046df00` | concrete | 1 |
| `SceneContents::read_scene_file_instantiate` | `0046df00` | concrete | 1 |
| `SceneContents::generation_gate` | `0046c550` | concrete | 68 |
| `SceneContents::registration_type_lookup` | `008f2260` | concrete | 34 |
| `SceneContents::register_vehicle_class_preload` | `0095c640` | **unimplemented** | 32 |
| `SceneContents::register_multiplayer_stock` | `0046bf70` | **unimplemented** | 32 |
| `SceneContents::register_scene_unit` | `004e96d0` | concrete | 32 |
| `SceneContents::launch_class_from_lua` | `00b68d70` | **unimplemented** | 32 |
| `SceneContents::create_scene_unit` | `004f0520` | concrete | 32 |
| `SceneContents::class_creator` | `004e99b0` | **unimplemented** | 2 |
| `SceneContents::property_bag_holder` | `00922e20` | **unimplemented** | 32 |
| `SceneGate::append_deferred_entity_record` | `0046c450` | **unimplemented** | 4 |
| `SceneUnit::vehicle_class_descriptor` | `00964790` | **unimplemented** | 32 |
| `SceneUnit::create_instance_from_descriptor` | `006fe590` | **unimplemented** | 32 |
| `SceneUnit::placement_deferred` | `004c1130` | **unimplemented** | 32 |
| `SceneUnit::world_parent_node` | `004de610` | **unimplemented** | 32 |
| `SceneUnit::place_instance` | `00928860` | **unimplemented** | 32 |
| `SceneUnit::set_instance_name` | `0041dd40` | concrete | 32 |
| `SceneUnit::queue_entity_command` | `00469610` | **unimplemented** | 32 |
| `InGameInterface::construct_manager` | `0068a990` | concrete | 1 |
| `InGameInterface::manager_init` | `0068cc70` | concrete | 1 |
| `InGameInterface::publish_manager_global` | `0068ccfe` | concrete | 1 |
| `InGameInterface::base_init` | `00683a90` | concrete | 1 |
| `InGameInterface::allocate_screen` | `00bf681b` | **unimplemented** | 42 |
| `InGameInterface::screen_constructor` | `0068cc70` | **unimplemented** | 42 |
| `InGameInterface::activate_manager` | `00684700` | **unimplemented** | 1 |
| `InGameInterface::screen_register` | `004f71d0` | concrete | 42 |
| `InGameInterface::screen_find_child` | `00aa7e00` | concrete | 204 |
| `InGameInterface::push_interface_request` | `004cc460` | **unimplemented** | 1 |
| `InGameInterface::apply_pending_interface` | `0068aca0` | concrete | 1 |
| `InGameInterface::apply_base_request` | `00684600` | **unimplemented** | 1 |
| `InGameInterface::hud_root_set_interface` | `00646040` | **unimplemented** | 1 |
| `InGameInterface::collapse_overlays` | `0068ab80` | **unimplemented** | 1 |
| `InGameInterface::set_level1_screen_set` | `004f8530` | concrete | 1 |
| `InGameInterface::set_level1_input_contexts` | `004d8a50` | concrete | 1 |
| `MainMenuManager::deleting_destructor` | `00687300` | **unimplemented** | 1 |
| `MainMenuManager::destroy` | `00686c90` | concrete | 1 |
| `MainMenuManager::commit_screen_visibility` | `00686db9` | concrete | 7 |
| `MainMenuScreen::deleting_destructor` | `00686e14` | **unimplemented** | 7 |
| `FrontEndManager::deactivate` | `00683aa0` | concrete | 1 |
| `MissionFrame::update_interface_only` | `004c40f0` | concrete | 90 |
| `MissionFrame::in_game_interface_active` | `004e524c` | concrete | 90 |
| `MissionFrame::update_in_game_interface` | `0068c1f0` | concrete | 90 |
| `InGameInterface::update` | `0068c1f0` | concrete | 90 |

`SceneUnit::hierarchy_deferral` (004F03C0) exists and was not reached: 004C1130's answer is the
record's false, so all 32 units take the placement branch.
`SceneGate::resolve_parent_pose` (0046C6B7) and `SceneGate::mode_area` were not reached either:
the reader composes the parent frame itself and passes a null parent identity, and single
player never reaches a mode with a play-area test.

### Corrections

1. **`include/bsp/scene_entity_factory.hpp` has the registration filter inverted.** Its comment
   reads "on the registration pass the reader keeps only entities whose class id is one of these
   six". The six ids are the ones that **skip** the registration body. 0046D453 branches on
   whether the bag carries `Type`: with `Type` present and a class id that is none of 47h, 19h,
   1Bh, 1Ch, 34h or 4Dh, the main body at 0046D50B runs 0095C640, 0046BF70 and descriptor[2];
   a match on any of the six, and the `Type`-absent path at 0046D547, both fall into the tail at
   0046D54B, which runs 0046BF70 and descriptor[2] only for 4Dh and 44h. `DestroyerGen` is 07h
   and takes the main body, which is what `docs/SCENE_UNIT_CREATORS.md`'s registration table
   already said; the two documents disagreed and the listing settles it. The fix belongs in that
   header, which this packet does not own.
2. **`docs/SCENE_ENTITY_FACTORY.md`'s first uncertainty is closed.** The set at scene database
   +164h is filled by the constructor 0046F160 with 47h and 3Dh; see section 2.
3. **`docs/SCENE_UNIT_CREATORS.md`'s "the enum symbol tables are not in any shipped data file"
   is wrong.** They are `universe/library/global.enums` and the six `enum` blocks in the
   `.props` files, loaded by 008F67B0.
4. **`include/bsp/scene_unit_creators.hpp`'s `kVehicleClassIndexTableEntries` is twice the real
   size.** It reads 0x1000 from `(4010h - 10h) / 4`; `include/bsp/vehicle_class.hpp` has the
   same singleton with a forward map of 800h entries at +10h and an inverse map of 800h at
   +2010h, which is what fills that span. The two headers disagree about one field.
5. **Milestone 2f's "a mission is running behind an unchanged picture" is superseded.** The
   release of the main-menu manager is a full destructor chain that takes the front-end pages
   down; see section 3.
6. **`include/bsp/mission_scene_load.hpp`'s `script_slot` and `script_slot_forced` are the game
   mode.** They are game+614h and game+61Ch, the two fields 004BCA50 reads; the mission script
   slot and the scene generation mode are the same pair, which is why a single-player load
   resolves both to 8.
7. **Packet `cc_hud_updates` merged during this packet's turn**, so the manager's
   construction 0068A990, its Init 0068CC70 and its per-frame update 0068C1F0 are no longer
   records: this milestone's first pass applied the level-1 set by hand over its own screen
   walk, and the final version runs the reconstruction. Milestone 2f's
   `MissionFrame::in_game_interface_active [004e524c]` and
   `update_in_game_interface [0068c1f0]` records are both gone.
8. **`bsp::SceneFileReaderHost::instantiate_entity` cannot express a nested entity's gate
   inputs.** 0046CF40 passes the entity's own `localframe` as argument 4 and the parent's world
   frame inline as arguments 7..22, and 0046C550 uses them differently; the host receives only
   the product. Every entity of this mission is top level, so the run is unaffected, and the
   executable reports the nested count.

### What this milestone supplies rather than recovers

- **The property-group and enum library reader.** Section 1. Its VFS enumeration, tokenizer and
  property-block parser are recovered; the `properties`/`enum` dispatch and the merge are not.
- **The unit instance.** `create_instance_from_descriptor` hands the creator the executable's
  own entity record so the rest of 004F0520 runs; the native 1188h allocation behind the
  descriptor's vtable +28h is recorded at 006FE590 and nothing claims its layout. This is the
  substitution milestone 2c makes for the seven main-menu screen classes.
- **The 42 HUD screens** are the executable's own registry records built from the recovered slot
  and page tables, for the same reason; the allocation and the 37 leaf constructors are records
  inside `bsp::init_in_game_interface_0068cc70`.
- **The manager's active byte at +3Ch.** 00684700 sets it through the manager's vtable slot
  +08h, and nothing in the reconstructed load reaches that dispatch, so the executable raises it
  and records 00684700. Without it the request Init pushes is never serviced and the frame's
  step 17 never runs.
- **The vehicle-class registry's forward index map** is filled with the identity, which is the
  state 00592640 and 00506550 leave (both reset all 800h pairs and rewrite one, and 00592640 is
  the footer command this run already takes). Without it 0095BA60 finds no class index.
- **The three mission-detail page roots** are hidden when the manager is destroyed, because the
  executable holds them directly rather than through the main-menu screen's +24h collector.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 004e98e0 | 004e98e0 | `JMP 004e96d0`, the registration creator of `DestroyerGen`, `SubmarineGen`, `LandingShipGen` and `TBoatGen`, already recorded by `docs/SCENE_UNIT_CREATORS.md` and still undefined |

Every other address this packet touched has a Ghidra function. Three names were added, 008F67B0
`CPropTreeLibrary_Load` (the one name here that is not a hypothesis: the class and method name
are ASCII in the image at 00D1653C), 0046F160 `BSP_SceneDatabase_Construct` and 0046F350
`BSP_SceneDatabase_CreateSingleton`; run-time evidence was appended to 004D4DF0, 0046CF40,
0046C550, 004F0520, 004E96D0, 0068ACA0, 004F8530 and 00686C90. 0068CC70 was read and cited but
not annotated: it is leased to `agent/cc-hud-updates` for packet `cc_hud_updates`.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2h.json` checks 46 call
rows and reports 0 failures; five rows are reported as indirect because the native call goes
through a register.

```
property library: 15 file(s) under universe/library, 22 property group(s), 33 enum table(s),
        1522 symbol(s) (008f67b0 CPropTreeLibrary::Load is not reconstructed; this is the
        executable's own reader over the recovered tokenizer and 008f5a00)
Loading scene universe/Scenes/missions/USN/usn_2_java.scn
scene contents pass 2 Registration: 34 entities visited, 0 groups, uniqueID=1
scene contents pass 3 Instantiate: 34 entities visited, 0 groups, uniqueID=1
  scene type DestroyerGen ShipClasses:DeRuyter=20 party=Allied(0) x2
  scene type DestroyerGen ShipClasses:Kagero=276 party=Japanese(1) x6
  scene class DestroyerGen  id=07 seen=32 generated=32 rejected=0 created=32
        registration_bodies=32 concrete 004f0520
  scene class NavPoint      id=41 seen=2  generated=2  rejected=0 created=0
        registration_bodies=0  record 004e99b0
in-mission HUD manager: 42 of 42 screens registered through
        bsp::init_in_game_interface_0068cc70 (37 leaf constructors are records), 52 of 52 pages
        loaded, 204 of 373 named widgets bound; INTF_SCENE3D (20h) pushed by Init at 0068d73a
the main-menu manager was destroyed: 00687300 -> 00686c90 -> 00683aa0, whose tail publishes the
        empty level-4 screen set (004f8710) and the empty level-4 input contexts (004d8c00), so
        the front-end pages come down
in-mission level-1 set for INTF_SCENE3D (null payload, single player): screens 29h 49h 44h 35h
        | contexts 04h 11h 12h 0Ch 0Bh | pages GUI_powerups GUI_unit GUI_selector GUI_minimap
sprite bridge quads=13 (text_glyph_quads=9)  textures=13/14 atlas_items=678 rebuild=34
sprite bridge quads=70 (text_glyph_quads=29) textures=14/15 atlas_items=678 rebuild=35
summary mission scene contents mode=8 entities=34 generated=34 rejected=0 created=32
        registration_bodies=32 party_class_marks=32 property_groups=22 enum_tables=33
summary mission unit passes: 32 unit record(s) exist and every unit pass of the frame and of
        the fixed step ticked 0 of them, because construct_world 004de610 is a load record
summary mission exit reachable=1: the mission ended through 004d7970: request 10h, the
        teardown arm 004e458a, state 11h and request 04h
summary mission hud screens=42/42 pages=52/52 widgets=204/373 interface=20h level1_screens=4
        level1_contexts=5 pump_frames=90 update_frames=90 audio_environment=Underwater
host methods 290 concrete, 318 unimplemented
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3: all four match milestone 2d exactly. A
`--mission-frames 60` run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`. The 2000 frame run exits 0 and runs 23 fixed steps with the
same fan-out disposition milestone 2g recorded; the step count is driven by wall-clock time, so
it moves with the work per frame and 2g's own long run reported 18. The close path was
validated by sending WM_CLOSE to a running process with `--press-start-frame 30 --menu-select
USN02 --mission-frames 60`: it presented 52219 frames, ran all 60 mission frames, recorded
`CloseRequestPolicy::front_end_branch [004ca2f0]` once and exited 0.

This is a runtime-validated process, not a game-validated one. It proves that the recovered
scene-file passes, generation gate, registration body and unit creator create the selected
mission's units out of real installed data with their real classes and parties, and that the
recovered interface dispatch puts the in-mission HUD's own pages on screen. It proves nothing
about the simulation: no created unit is ticked by anything, no HUD widget is updated, and the
world object every one of those passes needs does not exist.

### Follow-up packets

1. **`construct_world` 004DE610**, the world object at game+19CCh with its +4ACh byte and the
   entity manager at game+21A0h. It is now the single step between this executable and a
   ticking simulation: 32 unit records exist and nothing walks them.
2. **`prop_tree_library`**: 008F67B0, 008F6FC0 and 008F7100 in full, including the group
   inheritance the parenthesised name declares, the merge rule of 008F54F0's third argument and
   where the loaded library is published so 008F2260 resolves against it. That would delete this
   milestone's own reader.
3. **`scene_unit_class_factory`**: 00964790 and the unit instance its descriptor's vtable +28h
   allocates (006FE590 for `MDestroyer`). It is the boundary between a created entity record and
   a real unit.
4. **`scene_fixed_entity_classes`**: 004E99B0 and the other eleven fixed-size creators, which is
   what the two `NavPoint`s need.
5. **The HUD root update 00649860**, which clones the `GUI_powerups` and `GUI_unit` templates
   per unit. Until it runs the templates are drawn where the page authored them.
6. **The three central screens' update virtuals**, 00649860, 006435D0 and 005C0F20, which are
   analysed and not reconstructed; nothing on the HUD moves without them. The manager's own
   update 0068C1F0 already runs here, so these three are what is left between a drawn HUD and a
   live one.
7. **The three header fixes in corrections 1, 4 and 6**, all one-line changes in files this
   packet does not own.
8. **What raises the in-mission interface manager.** Its vtable slot +08h is 00684700, the base
   Activate, but no direct caller of 00684700 is this manager and nothing in the reconstructed
   load reaches the dispatch. Until it is found, the manager's +3Ch byte is the executable's.

## Milestone 2i: the destroyers tick and move

Addresses: 009037f0 with its call site 004de69c (the two 0Ch chain headers at world+4h and
world+8h), 00904bf0 at 004c40ce with the gate 00904c00, the dispatch 00904c18 and the link
00904c1a, 00904600 at 00904c2b with 00904ae3 / 00904b09 / 00904b2a and the producer 00905080;
004c3cb0 at 004c40b4 with 004bfdf0 at 004c3ce3, the three walk heads 004c3cf8 / 004c3d61 /
004c3eb4, 008ddf90 at 004c3e90 and the merge 004c2be0; 008255b0 with its twelve steps'
call sites 00825608, 00825640, 0082571d, 008257f5, 0082584b, 0082586b, 008258cf, 00825920,
008259b9, 008259c9, 008259df, 008259e6, 00825a54, 00825af4, 00825bcf, 00825c0c, 00825c74,
00825d5a, 00825d6c, 00825d7e and 00825db4; 00825f20 with 00813020 at 00826121, 0078cf20 at
00826985, 008e6430 at 00826a21, 00937440 at 00826a6d, 0080fc30 at 00826aab, 0092d300 at
00826b29, 0092e8c0 at 00826b54, 0092be80 at 00826b6a and the tail virtual at 00826b84;
00816a40 with the clamp 00815440; 004c0890 with 004c0893, 004c08f7 and 00b0d7b0 at 004c0905;
and the load's own 004c1ac0 at 004e04e7 with 00518250 at 004e04ee. Packet `cc_exe_2i`, owner
`agent/cc-exe-2i`. Sources: `src/game_hosts_world.cpp`, `include/bsp/game_hosts_world.hpp`,
`src/game_hosts_units.cpp`, `include/bsp/game_hosts_units.hpp`, plus edits to
`src/game_hosts_mission_frame.cpp`, `src/game_hosts_lua.cpp`, `src/game_hosts_hud.cpp`,
`src/game_hosts_menu.cpp`, `src/game_hosts_mission.cpp`, `src/game_hosts.cpp` and their
headers. Report: `reports/game_executable_milestone_2i.json`. Ghidra was read-only.

Milestone 2h created the mission's 32 units and reported the one number it could not improve:
every unit pass of the frame and of the fixed step ticked 0 of them, because nothing walked
them. This milestone is the other half of that. The world's entity walk now runs over the
created units every mission frame, each unit's own update runs, and the whole Java Sea order of
battle makes way under its authored command.

### The new switches

`--order throttle=<f>,rudder=<f>` and `--order-frame N` issue one player order to the
controlled unit on the in-mission frame the second switch names. `--mission-frame-seconds S`
runs each in-mission frame with a fixed delta instead of the wall clock, so a headless run
accumulates simulated time deterministically and the fixed-step driver's own clock does not
depend on how fast the machine presents; zero, the default, keeps the wall clock, which is what
every earlier milestone's run used. Every earlier switch is unchanged, and a run without
`--menu-select` is still byte-for-byte the milestone 2d run.

`--mission-frame-seconds` is what makes a headless run show motion at all. The fixed-step
driver accumulates the frame's own delta, so a 300 frame mission run on wall-clock time
accumulates about **0.9 seconds** of simulated time and runs 18 fixed steps; the same run with
`--mission-frame-seconds 0.05` accumulates **14 seconds** and runs 280. Both are reported below.

### 1. The world walk

`00904BF0` reads exactly one thing from the world object: `[[world+4]]`, the head field of a
0Ch-byte chain header that `009037F0` allocates at `004DE69C`, immediately after the
constructor (docs/WORLD_ENTITY_UPDATE.md). The executable owns that header instead of the
world, which is why the walk can run while `construct_world` 004DE610 is still a load record.
The chain is the instantiate pass's creation order; the gate is the byte at entity+5Ch
(00904C00) and the link is entity+38h (00904C1A), which is the whole of the routine.

Each entity's `vtable[0DCh]` is `008255B0` for a unit, and it runs: `GameUnitsHost` holds one
`bsp::UnitInstanceState` per created instance over a canonical `bsp::PoseRefreshView` of the
+74h, +C8h, +CCh and +10Ch fields. Of the twelve steps' call sites, two run a reconstruction
(`0092D730`, the body-axis forward speed, and `0092BE80`, the controller step whose recovered
body is one RET) and the rest are records with their own addresses. The recovered timer
fragments inside the routine do real work: the age at +524h, the two countdowns at +6D8h and
+728h, the bubble timer at +BC8h and the hit latch at +1010h/+1011h all advance.

One field had to be decided rather than read. `bsp/unit_instance.hpp` names the byte at
unit+5Dh `simulate` and defaults it to **true**, and gates steps 2 and 8 of 008255B0 on it;
docs/LOCAL_PLAYER_UNIT_LISTS.md's filter requires the same byte **clear** for a unit to reach
any of the eight lists, and `ship_throttle_gate_00826994` zeroes the throttle when it is set.
Two of the three readings say a live ship has it clear, so the executable holds it clear. The
cost is that the reconstruction's steps 2 and 8 are skipped, and neither could run here anyway:
step 2 also needs the ocean's world Y below zero and step 8 also needs the settings byte at
00424C40()+680h. The contradiction is that header's to resolve, not this packet's.

The walk closes with `00904600` at 00904C2B. The list at world+4B0h is **empty**, and the
reason is recovered: its only producer is `00905080`, the `AddMatrixInterpolator` Lua binding,
which is one of the 560 binding rows and a host record, and no script of this mission called
it. So the pass runs, visits nothing and retires nothing, and its four call sites
(00904AE3, 00904B09, 00904B2A) are records that were not reached.

Over a 300 frame mission run the walk visits **8960 entities and updates all 8960**: 32 units on
each of the 280 frames the run reaches before the mission ends, with no entity ever gated out,
because every created unit's +5Ch is set.

`004C3CB0` is no longer a guard with a recorded body. The clear `004BFDF0`, the three walks and
the five-call merge tail all run, once, on the first in-mission frame. The result over this
mission is `walk0_ships=32`, `walk0_rest=32` and zero in the other six: walk 0's chain answers
IsKindOf(06h) for every unit, because every one of them is a ship leaf, and the two lists walks
1 and 2 would fill have no source here. One thing is a stand-in and is recorded as one: walk 0 reads
`[[game+18CCh + slot*4]+30h]+DDCh`, the local player's unit registry, which nothing in this
process fills, and docs/LOCAL_PLAYER_UNIT_LISTS.md is explicit that what each of the registry's
five triples holds is not settled. The executable hands walk 0 the created units so the
recovered classify chain runs over real class ids, and records the heads of walks 1 and 2.

### 2. The motion

The class descriptor comes out of the installed data through recovered loading. The scene's
`Type = E ShipClasses : <symbol>` resolves through the enum library milestone 2h added, and
that id **is** the index of the installed `VehicleClass[N]` row: 19 Northampton, 20 DeRuyter,
21 York, 25 Clemson, 70 Kuma, 73 Fubuki, 263 leander, 265 Tribal, 276 Kagero, 289 Shiratsuyu
and 293 Myoko all match the scene's own symbols. The table is already in the mission Lua state,
because `Scripts/datatables/autoload/vehicleclasses.lua` is one of the 21 scripts the recovered
global-script step `00886900` runs, so the executable reads `MaxSpeed`, `MaxAccel`,
`Retardation`, `MaxRotAngle`, `MaxRotAngleChangeRatio` and `Type` straight out of it. `Type`
then selects the leaf class id through the recovered `vehicle_class_kind_row`, which is why a
DeRuyter is an `MCruiser` (0Ah) and a Kagero an `MDestroyer` (7) even though both are
`DestroyerGen` in the scene.

`00825F20` is **not** reached from `008255B0`; it is a separate virtual, and
docs/SHIP_MOTION.md names three call sites (0085542F, 00749B2C, 00644A38) and reads none. Where
it runs in a frame is therefore the executable's decision, and it is recorded as one: it runs
once per unit per fixed simulation step, because 0.05 s is the period the order ring is written
for (`kUnitStateMessageTickSeconds` and `kFixedSimulationStepFloat` are the same constant at
00D0DE84). Inside it the recovered chain runs whole: the ring tick 00813020, the keel sample
point, the throttle gate against the local wave height, the target-speed product, the force
model 00937440 through the controller's vtable slot 0, the boost block, the speed command
0092D300, the steering half 0092E8C0 and the controller step 0092BE80.

Every unit's authored `Command` is `Cruise`. What that token means is not recovered: 00469610
queues it and 0046AAB0 resolves it against a command registry whose command objects have no
reconstruction. The executable turns it into one order-ring order of throttle 1 and rudder 0
through the recovered `00816A40`, whose `00815440` clamps both into [-2,+2], and says so. The
order under the write cursor is refilled every step so the order keeps standing, which is what
`src/ship_motion_probe.cpp` does for the same reason.

The controlled unit is the first created instance, bound through `bsp::set_controlled_unit_004c0890`.
A ship answers IsKindOf(5), so the resolution picks the unit itself; it answers neither
IsKindOf(0Fh) nor IsKindOf(18h), so the routine takes its zero path and the listener handle at
00E188DC is cleared rather than published. That is the routine's second contract, not an error.
`00645600`, the HUD root's own select sequence, is not run: it needs a HUD root this process
does not own.

Four stand-ins are the same four `src/ship_motion_probe.cpp` reports, each a host record that
says so: the ocean sampler 0078CF20 (a flat sea at y = 0), the gameplay scale 008E6430 (the
literal 1.0f at 00D7A24C), the rudder curve settings at 00424C40()+438h..+44Ch (the three
denominator knots forced to 1) and the rigid-body integrator (an explicit Euler step, because
the game integrates in the physics library behind 00C32000 / 00C37E20 / 00C37E50). The two hull
dimensions the keel point uses are left at zero: only class+A0h has a recovered Lua key, and
with a flat sea and an upright hull the gate at 00826994 passes either way. It does: all 32
units report `applies=1` for the whole run.

What the command math produces is the probe's own result, on the mission's real ships. Over 14
seconds of simulated time the controlled `DeRuyter` reaches a forward speed of **16.451**
against its table's `MaxSpeed` of 16.4622, which is 99.9 percent, and with the player's rudder
at 0.5 it settles at a yaw rate of **0.06109 rad/s** against its `MaxRotAngle` of 0.122173,
which is exactly half. Its heading turns from 0 to -38.59 degrees and it ends 195 m from where
the scene placed it. Every one of the 32 ships moves, between 163 and 210 m depending on its
class's `MaxSpeed`; the ones still under the authored `Cruise` order run straight, and only the
controlled one turns, which is what one player order into one ring should do.

| Unit | Type | Party | Class id | Moved (m) |
| --- | --- | --- | --- | --- |
| `DeRuyter` (controlled) | DeRuyter | Allied | 0Ah `MCruiser` | 195.17 |
| `Kortenaer` | PACK3_Icarus | Allied | 07h `MDestroyer` | 199.62 |
| `Houston` | Northampton | Allied | 0Ah `MCruiser` | 163.76 |
| `Exeter` | York | Allied | 0Ah `MCruiser` | 163.05 |
| `Haguro` | Myoko | Japanese | 0Ah `MCruiser` | 173.04 |
| `Jintsu` | Kuma | Japanese | 0Ah `MCruiser` | 201.61 |
| `Sazanami` | Fubuki | Japanese | 07h `MDestroyer` | 209.52 |

The full 32-row table is in the run log. The `DestroyerGen` scene class produces both leaf
classes, which is the point of reading `Type` out of the installed table: a DeRuyter, a
Northampton, a York, a Myoko and a Kuma are all `MCruiser`.

### 3. The front-end frame, and why it stays

The mission load's own `select_front_end_layout` row runs now. It is `004C1AC0(3,0)` at
004E04E7 followed by `00518250(3,0)` at 004E04EE, and set 3 is the pause pair `GUI_pause` /
`GUI_pause_title` (`bsp/title_init.hpp`, `FrontEndFrameSet::Pause`). **The call does not
commit**, and `00518250` releases the other sets' handles and records the requested set only on
a committing call (00518272, 0051864A). So the answer to "do the front-end frame pages come
down when the mission starts" is no: `FE_frame` and `FE_frame_title` stay loaded, exactly as
milestone 2h's capture showed, and what decides whether they are drawn is the GUI layer's own
consumer of the 0x164 singleton, which is milestone 2b's unchanged caveat. The pause layouts
this row loads are published hidden, because no pause screen exists here to publish a byte for
them; that is the executable's value for milestone 2b's substitute rule, and it is logged.

The minimap's unit markers were **not** added. `GUI_minimap` carries the six team groups
(`minimap_units_blue_Group` and its five siblings) each with one `item_ship_Icon` template, and
`bsp/hud_updates.hpp` already reconstructs the placement (`hud_minimap_icon_position`, the
anisotropic 1/1024 and 1/768 divisors, and `hud_minimap_icon_rotation`, pi/2 minus the
heading). What is missing is the space those divisors put an icon into.
`docs/HUD_CENTRAL_UPDATES.md` reads `00427EB0` as returning "an icon-space float3"; it is three
instructions (body 00427EB0..00427EC8) that refresh the entity's pose when +C8h is clear and
return `unit+0FCh`, the translation row of the world pose, so the differences the placement
divides are **world** units. This mission's ships are spread over more than 5000 world units in
each axis, and 5000/1024 is far outside the 0..1 space the widget tree composes positions in,
so either the map half-extent `00432650` supplies culls almost all of them or the minimap group
carries a scale this packet did not find. Placing markers on either guess would be an invented
scale, so it stays a follow-up rather than becoming a stand-in.

### What it looks like on screen

Nothing changes. The capture at in-mission frame 200 is milestone 2h's picture unaltered: the
front-end frame (the flag, the two rails, the winged `MAIN MENU` plate and the modded
`MIDWAY MODDERS` / `eidos` bottom rail), and over it the HUD's four level-1 pages, the minimap
cluster in the top right with the page's own authored `error.tga` filling the island-map icon,
the compass, the white powerup template and the two text runs `Artillery` and `Fighter Ace`.
The sprite bridge holds the same 70 quads, 29 of them glyphs. **Thirty-two ships are under way
behind a picture that does not show them**, for the same reason milestone 2f gave: what the
mission would draw goes through 004CA440 and 004CA1F0, both records, and the HUD's own per-unit
icon pass is section 3's follow-up. The capture is written to the ignored `local/run_2i.png`
and is not committed.

### Host methods

`bsp_game.exe --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 300
--order-frame 20 --order throttle=1,rudder=0.5 --mission-complete-frame 280 --screenshot
local/run.png --screenshot-mission-frame 200 --log local/game_run.log --game-root "<install>"`,
exit 0: **306 concrete, 331 unimplemented**. Milestone 2h's run on its own tree reported 290 and
318. The same command with `--mission-frame-seconds 0.05` reports **305 and 331**, one fewer,
and the one it does not reach is `FixedStep::interpolation_wave`: the driver runs that wave only
while the accumulator is still above zero after its steps (00875f43), and a frame delta of
exactly one step leaves it at zero. A `--mission-frames 60` run with no
`--mission-complete-frame` reports 301 and 322 and still ends on the frame count with
`summary mission exit reachable=0`.

The per-step table with the call site and callee of every row this packet adds is
`reports/game_executable_milestone_2i.json` (`world_steps`, `unit_instance_steps`,
`motion_steps`, `controlled_unit_steps`, `load_steps`). The counts by group:

| Group | Steps | Concrete | Records | Stand-ins |
| --- | --- | --- | --- | --- |
| The world walk and the interpolator pass | 7 | 4 | 3 | 0 |
| The eight local-player unit lists | 6 | 2 | 3 | 1 |
| The unit instance update 008255b0 | 21 | 2 | 19 | 0 |
| The motion virtual 00825f20 | 16 | 8 | 4 | 4 |
| The controlled-unit bind 004c0890 | 4 | 2 | 2 | 0 |
| The load's front-end layout row | 3 | 2 | 1 | 0 |

### Corrections

1. **Milestone 2h's "every unit pass of the frame and of the fixed step ticked 0 of them" is
   superseded for two of those passes.** The world walk 00904BF0 and the motion virtual
   00825F20 run over the created units, because the only thing 00904BF0 reads from the world is
   the chain header 009037F0 allocates, and the executable owns it. What still ticks nothing is
   every pass that reads the world object itself: the entity manager at game+21A0h is null and
   the fixed step's world gate at 00875E69 reads a world that does not exist, so rows 9 to 13
   of the fan-out are still skipped. The run summary line says both halves.
2. **This packet's brief asked for the front-end frame pages to be hidden "if the state
   machine's screen sets say they are gone". They do not say that.** Section 3: the load's own
   00518250 call does not commit, so it releases nothing.
3. **This packet's brief read milestone 2h's `Error` tile as an atlas miss.** Milestone 2h
   already recorded that it is the page's own authored texture: `minimap_islandmap_Icon` names
   `error.tga` with the material `minimap_terrain.mshd`, and the running game replaces it with
   the mission's island map. Resolving a different atlas would not change it.
4. **The scene's `Type` id is the `VehicleClass` row index.** Milestone 2h used it only to name
   the class. All eleven of this mission's symbols index the matching installed row.
5. **`docs/HUD_CENTRAL_UPDATES.md` calls `00427EB0`'s result "an icon-space float3".** It is
   the entity's **world** translation: the body 00427EB0..00427EC8 is `if (unit+C8h == 0)
   BSP_EntityPose_RefreshWorld(); return unit + 0FCh`, and unit+FCh is pose row 3. Whatever
   compresses a world offset into the minimap's own space is somewhere else in 005C0F20; see
   section 3.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function and a reviewed ledger name. No
name was added; run-time evidence was appended to 00904bf0, 00904600, 004c3cb0, 008255b0,
00825f20, 00813020 and 004c0890.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2i.json` checks 40 call
rows and reports 0 failures; seven rows are reported as indirect because the native call goes
through a register or a vtable slot.

```
world units: 32 created instance(s) carried into the frame, 32 with a VehicleClass row out of
        the installed table
world entity chain: [[world+4]] holds 32 entity(ies), linked by +38h
controlled unit: 00e188d8 = "DeRuyter" (DestroyerGen DeRuyter, party 0); 00e188dc was cleared,
        because the resolved object answers neither IsKindOf(0Fh) nor IsKindOf(18h)
front-end frame set 3 requested with commit=0: the active set stays 0 and nothing is released
local-player unit lists rebuilt: walk0_ships=32 walk0_rest=32 ships=0 squadrons=0 airfields=0
        shipyards=0 land_forts=0 merged=0
world+4B0h holds 0 matrix interpolator record(s): the only producer is 00905080, the
        `AddMatrixInterpolator` binding, and no script of this mission called it
player order issued to "DeRuyter": throttle=1.000 rudder=0.500 through 00816a40
ship motion gate on "DeRuyter": keel=(250.00, 0.00, -3000.00) wave=0.00 applies=1
  world frame 280  entities=32 walked=8960 updated=8960 interpolators=0
        lists{ships=32 rest=32 merged=0}
summary mission world units=32 walked=8960 updated=8960 motion_ticks=576 simulated=0.90 s
        controlled=DeRuyter moved=1.15 total_path=35.19
host methods 306 concrete, 331 unimplemented
```

and, from the `--mission-frame-seconds 0.05` run:

```
  controlled unit frame 10   t=   0.50  x= 250.00 z= -2999.66  heading=  0.000  fwd= 1.350
        throttle=1.000 rudder=0.000  yaw= 0.00000
  controlled unit frame 100  t=   5.00  x= 249.43 z= -2962.89  heading= -7.185  fwd=14.780
        throttle=1.000 rudder=0.500  yaw=-0.05486
  controlled unit frame 280  t=  14.00  x= 238.80 z= -2805.15  heading=-38.591  fwd=16.431
        throttle=1.000 rudder=0.500  yaw=-0.06109
unit motion: 280 motion step(s) of 8960 unit tick(s) over 14.00 s of simulated time,
        8960 instance update(s) of 008255b0
summary mission fixed steps=280 at 0.050 s each
summary mission world units=32 walked=8960 updated=8960 motion_ticks=8960 simulated=14.00 s
        controlled=DeRuyter moved=195.17 total_path=6210.07
host methods 305 concrete, 331 unimplemented
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3: all four match milestone 2d exactly. A
`--mission-frames 60` run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`, and the 300 frame run above still leaves state 0Dh through
004d7970 and exits on the front-end request rather than on the frame count.

One environment note, because it cost this packet most of its turn: Direct3D 9 reports
`adapter count = 0` while the machine's interactive Windows session is disconnected, and the
executable then exits during Init with `startup failed: Renderer adapter identification failed`
and `device_hr=0x80004005`. That is the session, not the build: a `bsp_game.exe` built from
`main` fails identically. A run needs a connected session.

This is a runtime-validated process, not a game-validated one. It proves that the recovered
world entity walk, the unit instance update, the matrix-interpolator pass, the eight
local-player unit lists, the ship motion chain and the controlled-unit bind run end to end over
the mission's real units and real class data, and that the recovered command math drives a
destroyer to its table's own maximum speed and to a rudder-proportional fraction of its own
maximum turn rate. It proves nothing about the world object none of those passes has, about the
physics the stand-in integrator replaces, or about what a running game would draw.

### Follow-up packets

1. **`construct_world` 004DE610**, still milestone 2h's follow-up 1 and still the single step
   between this executable and the passes that read the world object: the entity manager at
   game+21A0h, the +4ACh ready byte and the fixed step's gate at 00875E69.
2. **The minimap's own scale.** `hud_minimap_icon_position` is reconstructed and `00427EB0`
   hands it world coordinates (correction 5), so what is left is the term between them:
   `00432650`'s map half-extents, and whatever transform the six team groups carry. With it the
   `item_ship_Icon` templates can be placed and a mission's units appear on the HUD.
3. **The `Cruise` command object**, and the rest of the `CommandType` registry 0046AAB0
   resolves against: it is what turns an authored scene command into a real order.
4. **`00825F20`'s caller.** 0085542F and 00749B2C have no Ghidra function and 00644A38 is in an
   undefined region; defining the three would settle where in a frame the motion virtual runs
   and remove this packet's own placement decision.
5. **Step 11's three routines** 008252C0, 00956600 and 00834E90, which every unit update this
   milestone runs reaches and records, and `00815AA0` over effect groups a real unit instance
   would carry. `include/bsp/unit_timers.hpp` and `include/bsp/unit_water_anchors.hpp` merged
   from `cc_unit_subupdates` during this packet's turn and reconstruct 00834820, 00834CC0 and
   00834A70, the bow wave, the stern wave and the spray; none of them is one of the three
   addresses step 11 calls, so the three stay records here.
6. **The local player's unit registry**, so walk 0 of 004C3CB0 has its real source and walks 1
   and 2 have one at all.

## Milestone 2j: the game's own motion inputs, a trajectory dump and an honest mission picture

Addresses: 0078cf20 with its two leaves 0078c890 at 0078cf3e and 00b9cf50 at 0078cf5f, and its
call sites 00826985 (in 00825f20) and 00825972 / 0082599f (in 008255b0); 008e6430 at 00826a21;
00424c40 at 0082e893 with its producer 0083b5e0 at 00424bfc, the script run 00b69d40 at
0083b6e6, the global lookup 00b67800 at 0083b73d and the fragment 0083ce56..0083d10d with
00b67690 at 0083ce84, 00b67700 at 0083ce98, 00b67800 at 0083ceb1, 00b67720 at 0083ceca and
00b66270 at 0083ced9; 00c41550 at 00c5bb5a, 00c5b1b0 at 00c5c491 and 00c5bb30 at 00c5c66d;
004c9ca0 with its two call sites 004e1873 and 004da746 and its own calls 00bf681b at 004c9cd7,
00636d90 at 004c9ced, the three vtable dispatches at 004c9d11 / 004c9d7e / 004c9daa, 004f83b0
at 004c9d88, 00aefa30 at 004c9dcf, 004c1ac0 at 004c9dff, 00518250 at 004c9e06 and 007f8d60 at
004c9e1d; and 004c1ac0 at 004e04e7. Packet `cc_exe_2j`, owner `agent/cc-exe-2j`. Sources:
`src/game_hosts_units.cpp`, `src/game_hosts_trajectory.cpp`,
`include/bsp/game_hosts_trajectory.hpp`, plus edits to `src/game_hosts_lua.cpp`,
`src/game_hosts_hud.cpp`, `src/game_hosts_menu.cpp`, `src/game_hosts_mission.cpp`,
`src/game_hosts_mission_frame.cpp`, `src/game_hosts.cpp`, `src/game_main.cpp` and their
headers. Report: `reports/game_executable_milestone_2j.json`. Ghidra was read-only.

Milestone 2i moved 32 destroyers with four labelled stand-ins in the motion path. Packet
`cc_ship_inputs` then recovered the producer of each of them. This milestone replaces all four
in the executable, adds a trajectory dump so the result can be compared against the running
game, and makes the mission frame stop drawing the main menu.

### The new switch

`--trajectory-csv <path>` writes one row per unit per fixed simulation step, with the header
row `step,t,unit,class,x,y,z,heading,fwd_speed,throttle,rudder,yaw_rate`. The path is
resolved before `--game-root` changes the current directory, for the same reason `--screenshot`
is: the run enters the installed game's read-only directory. Every earlier switch is unchanged.

Three details of the format are the consumer's, not this packet's preference, and packet
`cc_motion_trace` supplied all three while this packet was open:

- The speed column is `fwd_speed`. `tools/motion_trace_compare.py` accepts `fwd_speed`,
  `fwd_spd` or `speed` and nothing else; `forward_speed`, which this packet first emitted,
  parses as a missing optional column and defaults to `0.0`, so the whole speed channel would
  have read as zeros and the comparison would have reported a several-m/s divergence that was
  not there. Only `t` is strictly required by that reader.
- **One file per unit is written beside the combined one**, as `<stem>.<unit>.csv`. The
  combined file interleaves 32 trajectories, and a reader that takes consecutive rows as one
  trajectory splices them into a nonsense path with no error. The per-unit files have the same
  header and only that unit's rows, so the single-file form works with the compare tool as it
  stands and no `--unit` filter is needed.
- **`yaw_rate` is `dot(w, row1)`, not the angular velocity's world y.** Row 1 is the hull's own
  up axis and its component is the one `0092E8C0` slews toward the commanded rate. The two are
  the same number only while the hull is upright, and they separate exactly when the limiter at
  `0092EA15` is doing something, which is the case worth comparing. The run log's own
  `controlled unit` line still prints the world y, which is milestone 2i's column.

A **step 0 row** holds the pose the scene placed, before any motion step. Without it the first
row is already one step into the motion, and a consumer that aligns on its first sample folds
that step's displacement and rotation into the alignment. The 300 frame run therefore writes
8992 rows, 32 units by 281 blocks.

### 1. The four stand-ins, replaced

**The rudder curve.** Milestone 2i forced the three denominator knots to 1, because
`docs/SHIP_MOTION.md` had never located the block at `00424C40()+438h..+44Ch`.
`docs/UNIT_RUDDER_CURVE.md` located it: the block is on the gameplay settings singleton and its
only writer is the Lua-driven loader `0083B5E0`, which the singleton's constructor `00424A10`
tails into at `00424BFC`. The executable now runs that loader's head and its curve fragment.
`0083B6C3` formats the literal `Scripts\datatables\ShipGlobals.lua` at `00D0B67C` into a path
and `00B69D40` runs it; this process has one Lua state, the mission machine's, so the file is
run through the recovered file runner `00885110` on that state and `00B69D40` is a record.
`00B67980` then `00B67800` at `0083B73D` take the `ShipGlobals` global, and the fragment
`0083CE56..0083D10D` runs through `bsp::unit_rudder_curve_load_0083ce56` over the live
interpreter, with its five `00B67xxx` calls as host methods. The run reads

```
rudder curve loaded from ShipGlobals["Navigator"]["TurnMultipliers"]: min (0.000, 0.400)
        med (0.500, 1.500) max (1.000, 2.000)
```

which is the installed `shipglobals.lua` exactly, read out of the game's own data rather than
transcribed into the source. `00424C40` is a singleton, so the block is one table shared by
every ship of the mission; the executable holds one and hands it to all 32.

**The ocean sampler.** `0078CF20` runs whole: the wave field `0078C890` times the coverage mask
`00B9CF50`, formed at x87 precision and rounded once at `0078CF69`. Only the two leaves are
records, and they answer the routine's own values rather than invented ones: `docs/OCEAN_HEIGHT.md`
evidences that the field returns exactly `0.0f` when `field+F9h` is set or the amplitude at
`field+24h` is zero, and that the mask returns exactly `1.0f` when no region covers the point,
which is the open-sea case. So the flat sea is a stated contract with a reachable state behind
it. **The receiver is a renderer-owner record**: `0078CF20` takes `[[00E188A8]+19F0h]` and hands
`[world+A8h]` to both leaves, and that object is the renderer/scene owner's, the same pointer
`00AF0C50`'s foliage builder takes its camera from; `construct_world` `004DE610` is still a load
record. The unit update's own two samples at `00825972` and `0082599F` take the same host.

**The gameplay scale.** `008E6430` runs over an empty category list. The accumulator starts at
the `1.0f` at `00D7A24C` and no modifier record is registered in this process, so the filter
`008E4680` is never reached and the `1.0` is the routine's own value, not a literal a host
returns.

**The integrator.** `bsp::ship_integrate_stand_in` is gone. Every motion step now runs
`00C41550` then `00C5B1B0`, the Dyn library's two integration phases, in the order `00C5BB30`
runs them. The motion tick has just written both velocities onto the body through `00C37E50` /
`00C37E20`, so the velocity phase sees no force, no gravity and no damping and only rebuilds the
world inverse inertia, and the position phase is what turns the velocities into a pose. Three
things stay records or stated contracts, because no producer was found for them: the substep
schedule `00C5BB30` at `00C5C66D` (world+00h, the substep size, is unknown, so one substep of
the whole 0.05 s game step is taken), the hull body's mass, inertia and damping (`00C37F40`,
`00C37E70`, `00C37E00` and `00C37DE0` have no caller on the hull path, so they are never called
and the body carries none), and `M+18h` / `M+1Ch`, the two speed clamps the position phase
applies, which are set out of range so the clamps never fire. A zero there would zero the
velocity on the first substep, which is why leaving them at the default was not an option.

**What it did to the trajectory.** The same command, measured on this worktree before and after,
on the controlled `DeRuyter` (`VehicleClass[20]`, `MaxSpeed 16.4622`, `MaxRotAngle 0.122173`):

| at t = 14 s | before (2i stand-ins) | after (2j producers) |
| --- | --- | --- |
| forward speed | 16.431 | 16.454 |
| heading, degrees | -38.591 | -20.302 |
| yaw rate, rad/s | -0.06109 | -0.03054 |
| straight-line distance, m | 195.17 | 187.81 |

The yaw rate halves, and the reason is the shipped curve: at full throttle the denominator is
`2.0`, so `0082ECB0` returns `MaxRotAngle / 2` before the rudder scales it, and a rudder of 0.5
settles at `MaxRotAngle / 4`. The forward speed rises slightly for the reason
`docs/UNIT_RUDDER_CURVE.md` gives for the probe: a slower turn keeps more of the velocity on the
hull's forward axis, and `0092D300` rewrites only the axial component. The before row was taken
by stashing this packet's changes and rebuilding, and it reproduces milestone 2i's published
numbers exactly.

### 2. The trajectory dump

`--trajectory-csv` writes from the fixed step, immediately after `00825F20` has run for every
unit, where the step the motion ran in is unambiguous, plus the step 0 block before the first
step. The 300 frame run writes **8992 rows**, 32 units by 281 blocks, into the combined file and
into 32 per-unit files. `t` is simulated seconds, `x, y, z` is pose row 3 (`unit+FCh`), the same
three floats the keel-point arithmetic reads at `00826897`, `008268A1` and `008268AF`; `heading`
is degrees of `atan2(row2.x, row2.z)`; `fwd_speed` is `0092D730` over the body axis and the
linear velocity; `throttle` and `rudder` are `unit+980h` and `unit+984h` as the order ring
published them; and `yaw_rate` is `dot(w, row1)`, the component about the hull's own up axis.

**The comparison it was written for passes.** `docs/MOTION_DIFFERENTIAL.md` measured milestone
2i's executable against `src/ship_motion_probe.cpp` on this same `VehicleClass[20]` DeRuyter and
found the one divergence the stand-in curve caused: at full throttle and rudder 1 the executable
settled at `-0.12217` rad/s, the whole `MaxRotAngle`, against the probe's `-0.06109`, which is
`MaxRotAngle / 2.0`. With the curve wired, `--order throttle=1,rudder=1` settles at
**-0.06109 rad/s**, and `python tools/motion_trace_compare.py --trace <the DeRuyter file>
--probe <bsp_ship_motion_probe.exe --class 20 --throttle 1.0 --rudder 1.0> --align-origin`
reports **no channel outside its tolerance**:

| channel | peak delta | final delta |
| --- | --- | --- |
| speed, m/s | 0.2188 | -0.0000 |
| heading, degrees | 0.0434 | 0.0005 |
| position, m | 0.0957 | 0.0031 |
| yaw, rad/s | 0.00101 | — |

That is with `--order-frame 1`. With the brief's `--order-frame 20` the same comparison reports a
final heading delta of **1.70 degrees**, and the cause is the switch rather than the model: the
executable runs the authored `Cruise` order (throttle 1, rudder 0) for the first 20 in-mission
frames, which is 1.0 s of straight running the probe does not have. The remaining peak speed
delta of 0.22 m/s is the probe's own print granularity, which is every ten steps, interpolated.

### 3. The picture: the front end comes down

Milestone 2i asked whether the front-end frame pages come down when the mission starts and
answered no, because the load's own `004C1AC0` / `00518250` row at `004E04E4` does not commit.
That reading of the row was right and its conclusion was too early. **The committing call is in
`BSP_Game_ApplyInGameInterface` `004C9CA0`**, which milestone 2i recorded.

`004C9CA0` is two routines sharing a frame. `004C9CC0` tests its one byte argument: non-zero
takes `004C9CCD..004C9D68`, which allocates the loading element at `[00E198C4]+D4h` and sets its
`+4h` byte, and zero takes `004C9D6B..004C9EA2`, which tears that element down, releases
`interface/textures/allbutingame.ats` through the atlas manager, and runs
`00518250(set 3, commit 1)` at `004C9E06`. The load calls it with **1** at `004E1873` and the
mission-state entry calls it with **0** at `004DA746`. The executable now runs both arms. The
committing call releases every other set's backdrop, panel and title layouts, which is exactly
the `FE_frame` / `FE_frame_title` pair the title bring-up acquired, and acquires set 3's
`GUI_pause` pair hidden. The run log reads

```
front-end frame set 3 selected with commit=1: 00518250 released every other set's backdrop,
        panel and title layouts and made 3 the active set (it was 0)
```

One bridge-side change was needed with it. `GuiPageRegistry` in this process owns every page for
the whole run and has no destroy path, so releasing the handle left the sprite bridge drawing
`FE_frame` from widget records that outlive the page. The substitute for the native destruction
is to push the page hidden before releasing it, in `MenuFrameLayoutHost::gui_manager_release_layout`,
and it is labelled there as the substitute it is.

**The minimap's `error.tga` is not an atlas miss and not a missing material texture.**
`minimap_terrain.mshd` is the installed `shaderfx/gui/minimap_terrain.shfx`, a GUI
post-pipeline shader effect with two samplers, `RadarMap` at texture register 0 with clamped
addressing and `FadeBorder` at index 1, and it names **no texture file at all**. Sampler 0 is
the widget's own texture slot, which is why the authored `error.tga` reaches it, and the icon is
therefore already on the same texture path every other HUD texture takes. What replaces it at
run time is the mission's own radar map, which `BSP_HudMinimapScreen_Register` `005BEC50` binds
after taking the widget by name at `005BED21` through `00AA7E00`, and which the renderer owner
produces. There is nothing more for this process to load, so the renderer-owner host is
recorded and no texture is invented.

### 4. `LobbySettings` is nil in single player, and the substitute is gone

Packet `cc2_lobby_settings` reconstructed `005E2F00` in full while this packet was open, so the
load's `sync_lobby_settings_from_lua` row is no longer a record. Milestone 2f's substitute was a
zeroed thirteen-field table, created on the reasoning that "without the table the multiplayer
scripts index a nil global at their first line, which is why the step exists at all". **The
routine's own answer is the opposite of that guess.** `005E2F93..005E2FCC` is a single-player
early-out that sets the global **nil** at `005E2F59` and publishes no table at all, and
`game+1FE4h` is zero here, so that is the path this run takes. The executable now takes it. The
mission Lua run is unchanged by the change: the same three bindings are reached and no chunk
error appears, so nothing in USN02 indexed the global the substitute existed for. The three mode
flags `00E0C978`, `00E17BF2` and `00E08880` come out as `(1, 0, 1)`, which is the
single-player branch at `005E2FAB` forcing them, and the command points at `00E0CFB4` as the
`2400.0` constant at `00CE396C`, because the effective game mode 8 is at or above the Island
Capture bound. The `MultiLobbySettings` option registry `008D2F50` loads from
`Scripts/datatables/MultiGlobals.lua`, which this process does not load, so the option payload
is a host record; the native map is a `std::map`, so a missing key reads as zero there too, and
the early-out means no payload is read on this path anyway.

### What it looks like on screen

**The flag, the two rails and the winged `MAIN MENU` plate are gone.** The capture at in-mission
frame 200 shows the cleared dark blue buffer with the HUD's own level-1 pages over it and
nothing else: the minimap cluster in the top right, its island-map icon filled by the authored
`error.tga` (the green `Err` run and the blue `Err` run of the two stacked icons) with the
compass ring and the tick marks beside it, the two text runs `Artillery` and `Fighter Ace`, the
white powerup template between them, a small round icon at the top left, and a status bar with
a medal at the bottom right that milestone 2i's picture had hidden behind the front-end frame's
bottom rail. The sprite bridge drops from **70 quads to 57**. Thirty-two ships are still under
way behind a picture that does not show them, for milestone 2f's unchanged reason: what the
mission would draw goes through `004CA440` and `004CA1F0`, both records. The capture is written
to the ignored `local/run_2j.png` and is not committed.

### Host methods

`bsp_game.exe --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 300
--mission-frame-seconds 0.05 --order-frame 20 --order throttle=1,rudder=0.5
--mission-complete-frame 280 --trajectory-csv local/trajectory.csv --screenshot local/run.png
--screenshot-mission-frame 200 --log local/game_run.log --game-root "<install>"`, exit 0:
**325 concrete, 337 unimplemented**. The same command on this tree before this packet reports
305 and 331, which is milestone 2i's published pair. The wall-clock form of the same command
(no `--mission-frame-seconds`) reports 326 and 337, and a `--mission-frames 60` run with no
`--mission-complete-frame` exits 0 with `summary mission exit reachable=0` as before.

The per-step table with the call site and callee of every row this packet adds is
`reports/game_executable_milestone_2j.json` (`rudder_curve_steps`, `ocean_steps`,
`rigid_body_steps`, `mission_entry_steps`, `lobby_settings_steps`, `minimap`). The counts by
group:

| Group | Steps | Concrete | Records |
| --- | --- | --- | --- |
| The rudder curve and its producer | 10 | 8 | 2 |
| The ocean sampler and the gameplay scale | 5 | 3 | 2 |
| The rigid-body substep | 3 | 2 | 1 |
| `005E2F00`, the lobby settings sync | 3 | 1 | 2 |
| `004C9CA0`, both arms | 13 | 5 | 8 |
| The minimap's radar map | 1 | 0 | 1 |

**No stand-in is left in the unit motion path.** What used to be four stand-ins is now two host
records with evidenced return values (the two ocean leaves), three uncalled body setters and two
out-of-range clamp fields, each named above with the follow-up that would settle it.

### Corrections

1. **Milestone 2i read the load's front-end layout row as `004C1AC0(3,0)` followed by
   `00518250(3,0)`.** The row at `004E04E4` is `PUSH EBX; PUSH 3; CALL 004C1AC0; MOV ECX,EAX;
   CALL 00518250`. The two pushes are `00518250`'s arguments, not `004C1AC0`'s: `004C1AC0` is
   `BSP_FrontEndFrame_GetOrCreate`, the lazy getter for the `164h` object at `00E18D80`, and it
   takes none and only supplies `ECX`. The executable's record
   `MissionLoad::select_menu_layout 004c1ac0`, which called it "the menu-layout selector on a
   second singleton", is replaced by a concrete `MissionLoad::front_end_frame_get`: the
   executable owns that object.
2. **Milestone 2i's "do the front-end frame pages come down when the mission starts: no" is
   superseded.** They do, one step later, in `004C9CA0`'s zero arm at `004DA746`. Milestone 2i's
   statement about the load's own row stands; its conclusion about the mission did not.
3. **Milestone 2i's four stand-ins are gone.** `docs/SHIP_MOTION.md`'s stand-in list was already
   corrected by `docs/UNIT_RUDDER_CURVE.md` for the probe; this is the same correction applied
   to the executable.
4. **Milestone 2h's and 2i's reading of the minimap `error.tga` as the page's authored texture
   is right, and is now explained.** The material names no texture, so `error.tga` is sampler
   0's own authored value and no other texture exists to load. Milestone 2i's correction 3 said
   resolving a different atlas would not change it; the reason is the shader effect's two
   samplers, not the atlas.
5. **A released GUI page was still drawn.** Not a native reading but a bridge defect this packet
   found by releasing pages for the first time: `release_screen_page` erased the page from the
   screen-page set and left its widget records in the bridge. The release path now pushes the
   page hidden first.
6. **Milestone 2f's `LobbySettings` substitute was the wrong way round.** It created a zeroed
   thirteen-field table so that "the multiplayer scripts" would not index a nil global. The
   reconstruction of `005E2F00` says single player leaves the global **nil**, and the USN02 run
   with the global nil reaches the same three bindings with no chunk error. Section 4.
7. **This packet's first trajectory header was `forward_speed`, which the consumer cannot
   read.** `tools/motion_trace_compare.py` takes `fwd_speed`, `fwd_spd` or `speed`, and an
   unrecognised optional column defaults to `0.0` silently, so the mistake would have shown up
   as a several-m/s speed divergence rather than as an error. The column is `fwd_speed`, and the
   same round of peer review moved `yaw_rate` from the world y to the hull's up axis and added
   the step 0 row. Reported by packet `cc_motion_trace`; see "The new switch".
8. **`docs/MOTION_DIFFERENTIAL.md`'s turn-rate divergence is closed.** It measured milestone
   2i's executable at `-0.12217` rad/s against the probe's `-0.06109` at full throttle and
   rudder 1, and attributed the factor of two to the stand-in denominator. With the curve wired
   the executable settles at `-0.06109`, and the compare tool reports a final heading delta of
   0.0005 degrees. That doc's note that "a `bsp_game.exe` milestone-2i log still forces the three
   knots to 1" is superseded for milestone 2j and later.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function. No name was added; run-time
evidence was appended to 0078cf20, 008e6430, 00424c40, 0083b5e0, 0082e890, 0082ecb0, 00c41550,
00c5b1b0, 00c5c540, 004c9ca0, 00518250 and 005e2f00.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2j.json` checks 29 call
rows and reports 0 failures; three rows are reported as indirect because the native call goes
through the loading element's vtable.

```
gameplay settings: Scripts/datatables/ShipGlobals.lua run through 00885110 (the owner's own
        runner 00b69d40 at 0083b6e6 is a record), chunk ok=1, `ShipGlobals` is a table
rudder curve loaded from ShipGlobals["Navigator"]["TurnMultipliers"]: min (0.000, 0.400)
        med (0.500, 1.500) max (1.000, 2.000)
ocean sampler 0078cf20 runs, both of its leaves are records: its receiver is
        [[00e188a8]+19F0h] and both calls take [world+A8h], the renderer/scene owner's field
        object, so the wave field answers its own disabled value 0.0f and the coverage mask its
        own open-sea value 1.0f, and the product 0078cf64 is exactly 0.0f
gameplay scale 008e6430 runs over an empty category list: no modifier record is registered in
        this process, so the product is the 1.0f the accumulator starts at (00d7a24c)
rigid body: 00c41550 then 00c5b1b0, one substep of the whole 0.0500 s game step
LobbySettings: the global is set nil by the 005e2f93 early-out (single player,
        game+1FE4h = 0), fields nil=0 number=0 string=0; mode flags powerups=1
        reload_payload=0 map=1, command points 2400.0
in-game interface applied with 0: the loading element is torn down and 00518250(3, commit=1)
        at 004c9e06 releases every other set's layouts
  controlled unit frame 280  t=  14.00  x= 244.36 z= -2812.28  heading= -20.302  fwd= 16.454
        throttle= 1.000 rudder= 0.500  yaw=-0.03054
summary mission world units=32 walked=8960 updated=8960 motion_ticks=8960 simulated=14.00 s
        controlled=DeRuyter moved=187.81 total_path=6202.64
summary mission trajectory csv=local\trajectory.csv rows=8992
summary mission lua bindings=560 natives=3 calls=3
summary bridge_open=1 atlas=interface/textures/allbutingame_dxt1.ats atlas_items=678
        textures=14 quads=57 frames=320
host methods 325 concrete, 337 unimplemented
```

and, from the acceptance run `--order-frame 1 --order throttle=1,rudder=1`:

```
  controlled unit frame 280  t=  14.00  x= 233.55 z= -2804.62  heading= -41.253  fwd= 16.430
        throttle= 1.000 rudder= 1.000  yaw=-0.06109
motion differential: live trace vs reconstructed probe
  samples compared   281        time span 0.00 .. 14.00 s
  channel         peak |delta|   final delta
  speed (m/s)           0.2188       -0.0000
  heading (deg)         0.0434        0.0005
  position (m)          0.0957        0.0031
  yaw (rad/s)          0.00101
  no channel left its tolerance over the compared span
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua` exits
0 and `--vfs-probe does/not/exist.lua` exits 3: all four match milestones 2d and 2i exactly. A
`--mission-frames 60` run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`, and the 300 frame run above still leaves state 0Dh through
004d7970 and exits on the front-end request rather than on the frame count.

This remains a runtime-validated process, not a game-validated one. What it now proves, that
milestone 2i did not, is that the mission's ships are driven by the game's own authored turn
curve read from the game's own data file, by the game's own water sampler, by the game's own
modifier product and by the game's own two integration phases, that the executable and the
reconstruction probe agree to 0.0005 degrees of heading over 14 s on the same class, and that
the recovered mission entry takes the front end off the screen. It still proves nothing about
the hull body's mass, inertia and damping, about the forces nothing pushes onto it, about the
library's substep size, or about what a running game would draw. In particular, the two sides
that now agree share every one of those gaps: the agreement is between two runs of the same
reconstruction, not between the reconstruction and the game.

### Follow-up packets

1. **`dyn_world_construction`**: `world+00h`, `world+34h` and `world+3Ch..+44h`. The substep
   size is the last unknown between this executable's integration and the game's own.
2. **`ship_hull_body_creation`**: the writer of `controller+2Ch`, and `M+18h` / `M+1Ch`, which
   this milestone had to set out of range so the position phase's clamps never fire.
3. **`unit_force_channel`**: `0074F2E0` and `unit+10D4h`. Nothing pushes force onto a body here,
   so the velocity phase integrates only what the motion tick wrote directly.
4. **`ocean_wave_field_sample` and `ocean_coverage_regions`**, the two leaves this milestone
   records, and the producer of `[[00E188A8]+19F0h]+A8h`.
5. **`construct_world` 004DE610**, unchanged from milestones 2h and 2i.
6. **The minimap's own scale**, unchanged from milestone 2i follow-up 2, and the radar map the
   renderer owner would put into sampler 0.
7. **The `Cruise` command object**, unchanged from milestone 2i follow-up 3.

## Milestone 2k: the moving ships reach the HUD

Addresses: 004cc460 at 00648605 (in 006485a0) with 006840f0, 00684600 and 0068aca0's two
arms of id 20h (0068ae0b's unit-kind classifier at 0068ae1c and the re-entry 0068afb4);
005c0f20 with 005bd420 at 005c0f3f, 004b4b00 at 005c154e, 00432650 at 005c157e and
005c158a, 0087d7b0 at 004ddfc4 (in 004ddb90), 00414db0 at 005c15bb, 00927880 at 005c15f2,
00427eb0 at 005c1687, 00427e30 at 005c16ce, 005be110 / 00bf681b / 005bd590 / 005c0700 /
00694a60 at 005c170a..005c1767, 00b6db70 at 005c17a7, 00bf701a at 005c17e0, 00bf7030 at
005c1a52, 00aa7dc0 at 005c1c99 and the rotation virtual at 005c1cd0, with the widget binds
005bed41, 005be2a7, 005be392, 005beb68 and 005bebd8; and 006435d0 with 00aa1fe0 at
00643616, 00640620 at 00643695 and 00643d41, 0063bcd0 at 006436a3, 006394b0 at 006436c1,
0063b5e0 at 006436cc, 006430c0 at 006436e5, 00804350 at 0064373f, 005220c0 at 00643755,
00b6fde0 at 00643892, 0043a290 at 006438e8, 00643360 at 00643a58, 00642c20 at 00643ba9,
00523020 at 00643bea, 006434e0 at 00643c72 and 00640d70 at 00643c8e, then 0080e490,
0063abd0, 004323d0, 008ddf90, 0043f080 and 00642040 inside 006430c0, 00639990, 00414db0,
0063a6c0, 00803dc0, 0077a2f0 and 0063d1e0 inside 00642040, 00638e50 inside 0063a6c0,
0043a660 inside 00638e50, 00b70490 / 00b62d10 / 00aa1fe0 inside 0043a660 and 00abbe50
inside 0063d1e0. Packet `cc_exe_2k`, owner `agent/cc-exe-2k`. Sources:
`src/game_hosts_hud_world.cpp`, `include/bsp/game_hosts_hud_world.hpp`, plus edits to
`src/game_hosts_hud.cpp`, `src/game_hosts_frontend.cpp`, `src/game_hosts_menu.cpp`,
`src/game_hosts_lua.cpp`, `src/game_hosts_units.cpp`, `src/game_hosts_mission_frame.cpp`
and their headers. Report: `reports/game_executable_milestone_2k.json`. Ghidra was
read-only.

Milestone 2i moved 32 destroyers and milestone 2j drove them with the game's own inputs,
and both ended with the same sentence: thirty-two ships are under way behind a picture that
does not show them. This milestone is the first half of the answer. It does not draw the
world; it drives the two HUD screens whose update virtual packet `cc_hud_minimap`
reconstructed, so the mission's own units reach the HUD the game would show them on.

### 1. The interface request carries a unit, and that is what raises the two screens

Milestone 2h applied the request Init pushes at 0068D73A, `004CC460(20h, 0)`, and reported
the level-1 set its arm publishes: `29h, 49h, 44h, 35h`. That is correct for **that**
request and it is not where a mission ends up. `docs/IN_GAME_INTERFACE_SCREEN_SETS.md` reads
the 20h arm in full: with a payload it never touches a setter at all, it is a unit-kind
classifier that re-enters the object's own virtual `+10h` with a different id. A ship
answers `IsKindOf(6)` at 0068AE5C, so the re-entry is **25h INTF_CAPTAIN**, whose arm at
0068B115 publishes eleven screens, `4Dh` (the world markers) and `35h` (the minimap) among
them.

Milestone 2i already binds a controlled unit through `bsp::set_controlled_unit_004c0890`, so
the payload exists. The executable pushes `004CC460(20h, <the controlled unit>)` on the
load's own unit row and the recovered path services it: 006840F0, 00684600 and
`bsp::apply_in_game_interface_0068aca0`, whose 20h arm probes the unit through the recovered
`bsp::unit_is_kind_of_006fe530` over its real class id and re-enters itself. Every part of
that except the push is recovered code running on recovered data.

```
in-mission level-1 set for INTF_SCENE3D (null payload, single player) applied as 20h:
        screens 29h 49h 44h 35h | contexts 04h 11h 12h 0Ch 0Bh
in-mission level-1 set for INTF_SCENE3D (controlled unit payload, single player) applied as
        25h: screens 29h 49h 44h 27h 4Dh 45h 46h 26h 2Eh 35h 50h | contexts 06h 04h 11h 12h
        0Ch 0Bh | pages GUI_powerups GUI_unit GUI_selector GUI_markers GUI_ship GUI_repair
        GUI_ship_effects GUI_ship_damage GUI_binoculars GUI_cross_gunstate GUI_cross_ship
        GUI_minimap GUI_Warning
```

**The push is the executable's, the id is not.** The native pushers of that request are the
HUD root's own 006485A0, 00649860 and 00647300, all three of which read a HUD root object
this process does not own; the push is recorded at 00648605 with 006485A0's call site. The
ordering is the executable's too: the load creates the units on its `load_scene_contents` row
and builds the HUD manager on a later row, so the request outlives the push and the first
004C40F0 pass that finds a manager services it.

Once 4Dh and 35h are in level 1 the recovered pump 004F8830 enters them and calls their
update virtuals, which is exactly where the native calls 005C0F20 and 006435D0. Nothing in
this milestone calls either routine directly.

### 2. The minimap: one icon per unit, where the recovered transform puts it

`bsp::hud_minimap_place_unit_icons_005c154e` drives the pass. What it reads:

| Step | Native | In this run |
| --- | --- | --- |
| the two radii | 00432650 +6Ch and +70h, written by 0087D7B0 | **4000 and 4000, read out of the installed `scripts/datatables/globals.lua`** |
| the camera's unit | 004B4B00 | **stand-in**: the controlled unit |
| the camera heading | 00B6DB70, `[renderer+110h]`, `[renderer+118h]` | **stand-in**: the controlled unit's own forward row |
| the team unit list | `[[[00E188A8+18ECh*4+18CCh]+30h]+E0Ch]` | the created units, the same stand-in milestone 2i records for walk 0 of 004C3CB0 |
| the four byte filters | 005C1628..005C164A | run over the created instances |
| `IsKindOf(5)` | vtable +5Ch | run over the recovered class chain |
| vtable `+B8h` | 005C1675 | recorded; `reports/hud_minimap.json` resolves the slot to 0043F080, whose gate has already passed |
| the visibility cull | 005C16D3 and 005C19B1 | drops 15 of the 31 reachable units |
| the rim clamp and the placement | 005C1A3C..005C1C99 | run whole |
| the icon rotation | 005C1CA1..005C1CD0 | run beside the pass, which does not cover it |

`0087D7B0` is not reconstructed, so it is a record and only its two `Globals["Minimap"]`
reads are performed, on the same Lua machine milestone 2j runs `ShipGlobals.lua` on:

```
minimap range: scripts/datatables/globals.lua run through 00885110, chunk ok=1,
        Globals["Minimap"] read (MinimapRange=4000.0 VisibilityRange=4000.0); 0087d7b0, the
        loader that writes them into the global config object at +6Ch and +70h, is a record
```

**The icon itself is the executable's.** The native per-unit entry is 0Ch bytes from
00BF681B, constructed by 005BD590, inserted by 005C0700 and attached by 00694A60, and all
four belong to packet `hud_minimap_icon_entries`; all four are records. What the executable
creates instead is a sprite-bridge clone of the page's own `item_ship_Icon` template, chosen
from the six colour groups by 00639990's own non-objective rule on the unit's team byte, and
parented to `unit_marker_Group`. That parent is the one the reconstruction calls the per-unit
icon map's parent at screen +F0h, and it is the only candidate whose authored position,
(0.865167, 0.164028), is the minimap centre: the six colour groups sit at 0.64375 and
0.2264..0.3597, which no small local offset would bring onto the map. Which group the native
entry is parented to is that packet's open question, not this one's answer.

**The player's own ship carries no dot**, because 005C1650 skips the camera unit, and with
the camera unit standing in as the controlled unit that is the controlled ship. What
represents it is `minimap_dir_Icon`, the direction wedge at the centre, which 005C1872
rotates by `camera_heading - icon_heading + pi/2`.

### 3. The markers: the pass runs whole and its own selection produces one marker

`bsp::hud_markers_screen_update` runs every block of 006435D0, and
`bsp::hud_markers_add_unit_marker_006430c0` runs the gate, the flag derivation, the dispatch
and the screen-bounds rule for the unit it is handed. The result is **one** marker, and that
is the routine's answer rather than a shortfall: 006435D0 marks the controlled unit, the
interface manager's target, the squad members inside their own radius, the objectives, the
command units and the target group, and this process has only the first of the six.

| Source | Native | Why it reaches nothing here |
| --- | --- | --- |
| the controlled unit | 006436E5 | **the one marker this mission produces** |
| the interface target | `[[00E198C4+CCh]+4Ch]` | set by the selector screen, one of the 42 records |
| the squad members | `[[00E188A8+19CCh]+16Ch]` | the world object's list; and `unit+7C4h` has no recovered field, so `hud_marker_within_radius` rejects every mate against a zero radius |
| the objectives | 00643360 at 00643A58 | no objective container is built |
| the command units | 00642C20 at 00643BA9 | gated on the group manager at game+1FE4h |
| the target group | 006434E0 at 00643C72 | 00523020 answers no target |

Marking all 32 ships would be an invented selection, so the executable does not.

**The projection is a stand-in and is labelled as one everywhere it appears.** 0043A660 asks
00B70490 for the view-projection matrix with `ECX = [00E188A8+19FCh]`, and this process
builds no camera, so the projection host is a fixed top-down orthographic camera fitted to
the mission's own unit bounds:

```
markers camera: **an executable-side stand-in**, a fixed top-down orthographic camera over
        the mission's own unit bounds x[-5000, 5000] z[-8000, 250], half extent 6000.
        00b70490 and 00b62d10, the view-projection matrix and the transform 0043a660 needs,
        are records
```

Everything downstream of it is recovered: the clip mask, the mode-1 mapping, the widescreen
rescale of 00638E50, the clip rectangle 006435D0 publishes into 00E197C4..00E197D8 and the
eight-corner accumulation of 0063A6C0. The eight corners collapse onto the anchor, because
`[unit+538h]`'s three extents are all zero here (only +A0h and +A8h have a recovered Lua key
and both are zero for these ships), so 0063AB75's collapse path is the one that runs and the
run reports `collapsed=1`. The marker widget is a clone of the page's own `sidemarker_Group`,
because the per-marker writer 0063D1E0 has no reconstruction. Its `Unit_name_Text` keeps the
page's authored `Unit Name`: 0063D751 fills it through 00ABBE50, the plain C-string setter,
and the executable's text path carries localisation ids only, so nothing invents a string.

### 4. Cruise

Packet `cc_cruise_command` is **not** on main as of 2b6cce8b, so the authored
`Command = E CommandType : Cruise` token is still turned into one order-ring order of
throttle 1 and rudder 0 by milestone 2i's own decision. What this milestone adds is the
reporting the packet brief asked for: the end-of-run distance table now carries the ordered
pair each unit is running under, straight out of `unit+980h` and `unit+984h` as the ring
published them. A 300 frame run with no `--order` moves all 32 ships:

```
  unit                 type         party class throttle   rudder   start x   start z         x         z    moved  gate
  DeRuyter             DeRuyter         0    10    1.000    0.000     250.0   -3000.0     250.0   -2806.9   193.10     1  <- controlled
  Java                 DeRuyter         0    10    1.000    0.000    -250.0   -3000.0    -250.0   -2806.9   193.10     1
  Minegumo             Kagero           1     7    1.000    0.000    4000.0   -4500.0    3791.2   -4500.0   208.76     1
```

`--order` still overrides the controlled unit's pair, and the table shows that too.

### What it looks like on screen

The picture changes twice. First, the captain's HUD arrives: the eleven screens 25h publishes
put the repair wheel and its four quadrant icons in the middle left, the engine telegraph
with its `STOP`, `HALF`, `3/4` and `FULL` dial in the bottom right with the rudder indicator
beside it, the damage bar under them, and the minimap cluster in the top right with its
compass ring over the authored `error.tga` island map. The sprite bridge goes from **71 quads
to 192** on the frame the request is applied.

Second, the units appear on the minimap: **sixteen small ship icons inside the compass ring**,
red for the Allied party and white for the Japanese one, each rotated to its own heading, and
one unit marker near the middle of the screen with the `1254` distance text and the health
bar of the `sidemarker_Group` template. Two captures of the same command, at in-mission
frames 40 and 280, show the icons in different places: the white group above the compass
centre moves right across the dial, the two red icons below it swap sides as the map rotates
under the controlled ship's own heading, and the marker itself travels with the DeRuyter. The
captures are `local/run_2k_f40.png` and `local/run_2k_f280.png`, both ignored and not
committed.

Both captures were taken at 1024x768 through an isolated `--settings-personal-root`, and that
is worth saying plainly: at the validation machine's own 2560x1440 the wide-screen X fixup
moves every `WideScreenAlign = "Right"` widget by `+0.1416667` (00AA8771), which puts the
minimap centre at x = 1.0083 while the sprite bridge maps x = 1 to the right edge of the back
buffer. The whole cluster, dots included, then sits half outside the window. That is the
bridge's own coordinate mapping, not a change this milestone made, and it is a follow-up.

`drawn: yes`. 194 quads on the captured mission frame, 57 of them glyphs, against 13 with the
front end down and the HUD not yet up.

### Host methods

`bsp_game.exe --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 300
--mission-frame-seconds 0.05 --order-frame 0 --order throttle=1,rudder=0.5
--mission-complete-frame 290 --screenshot local/run_2k.png --screenshot-mission-frame 280
--log local/game_run_2k.log --game-root "<install>"`, exit 0: **349 concrete, 377
unimplemented**. Milestone 2j's published pair for the same shape of command is 325 and 337.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2k.json`. The counts by group:

| Group | Steps with a call site | Concrete | Records |
| --- | --- | --- | --- |
| The unit-payload interface request | 8 | 3 | 5 |
| The minimap update 005c0f20 | 23 | 11 | 12 |
| The markers update 006435d0 | 38 | 9 | 29 |

### Corrections

1. **Milestone 2h's "the executable applies exactly that id rather than choosing one" is
   right for Init's request and wrong as the mission's final state.** The 20h arm with a
   payload publishes nothing; it classifies. See section 1.
2. **This packet's brief asked for "the world-to-minimap transform from the mission's map
   bounds". There is no such transform.** The minimap is camera-relative and camera-rotated,
   and `docs/HUD_MINIMAP.md`'s own correction already says that 00432650's +6Ch and +70h are
   `Minimap.MinimapRange` and `Minimap.VisibilityRange`, not map bounds. The mission's bounds
   appear in this milestone only as the marker stand-in camera's extent.
3. **`docs/MOTION_DIFFERENTIAL.md`'s closed divergence has reopened, on the other side.**
   Milestone 2j measured the executable and `src/ship_motion_probe.cpp` agreeing to 0.0005
   degrees of heading over 14 s. Commit 4977f563 then gave the probe the hull body 00937C90's
   tail creates and the dynamics world 004DDB90 fills, so the probe applies the body's
   angular damping and settles at **0.05513 rad/s** at full throttle and rudder 1, while this
   executable still leaves the body without mass, inertia or damping, exactly as milestone 2j
   states, and settles at **0.06109**. The executable's own numbers are unchanged from
   milestone 2j's published acceptance run, so nothing here regressed; what is missing is the
   same hull-body creation on the executable's side, which is milestone 2j's follow-up 2.
4. **`cc_cruise_command` is not on main**, so the `Cruise` token is still milestone 2i's own
   one-order mapping. Section 4.
5. **Milestone 2b's "it orders quads back to front by the authored Z alone" is still true for
   authored widgets, and this milestone shows the rule is not the game's.** `GUI_minimap`
   puts its island map at Z -5 and its frame, glass and direction wedge at -22, -23 and -24,
   so a Z-only order draws the frame over the unit icons; the page's own `geOrder` puts
   `unit_marker_Group` (0) in front of every one of them. `geOrder` is not a draw order
   either, because `sidemarker_Group` gives `HP_Icon` 5 and `HP_BG_Icon` 3, which is
   front to back. The native render order is another owner's, so the bridge keeps the Z rule
   for authored widgets and adds exactly one rule of its own: **a run-time clone is drawn
   after every authored quad**. No earlier capture moves.
6. **`00639990`'s colour index is the marker screen's, and this milestone uses it for the
   minimap too.** `docs/HUD_MINIMAP.md` states that the six group colour names are the same
   six values it returns; the mapping from index to group is `hud_minimap_icon_entries`'
   open question, so the executable's choice is labelled as its own.

### What this milestone supplies rather than recovers

- **The camera.** Every one of the three camera inputs is a stand-in with its native call site
  recorded: the camera's unit (004B4B00), the renderer basis the minimap heading comes from
  (00B6DB70 with `[renderer+110h]` and `[renderer+118h]`) and the view-projection matrix the
  marker projection needs (00B70490 and 00B62D10). The crosshair pick 0043A290 needs the same
  camera and is a record that answers "no hit" rather than a made-up point.
- **The two icon widgets.** A minimap icon is a sprite-bridge clone of the page's own
  `item_ship_Icon`; a marker is a clone of the page's own `sidemarker_Group`. The native
  entry and the native writer are five and one records.
- **The GUI extent**, (1, 1), which is the unit square the sprite bridge multiplies by the
  back buffer. 00AA1FE0 is a record.
- **The push of the unit-payload interface request**, and its ordering relative to the HUD
  manager's construction. The id it resolves to is not the executable's.
- **One sprite-bridge draw-order rule** for the widgets the executable created, correction 5.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet names has a Ghidra function body. The hole
`docs/HUD_MARKERS_RUNTIME.md` records at 00643C1C..00643C68 is inside 006435D0's own body,
and this packet's rows avoid it: the group-member step is cited at 00643C72, the site Ghidra
covers. No name was added; run-time evidence was appended to 005C0F20, 006435D0, 006430C0,
0063A6C0, 0068ACA0 and 0087D7B0.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2k.json` checks 61
call rows and reports 0 failures.

```
minimap range: scripts/datatables/globals.lua run through 00885110, chunk ok=1,
        Globals["Minimap"] read (MinimapRange=4000.0 VisibilityRange=4000.0)
minimap page bound: 6 of 6 colour templates, unit_marker_Group bound, compass bound, island
        map bound, direction wedge bound; the RadarMap render target minimap_terrain.mshd
        samples is the renderer owner's and stays a record
minimap unit icons: 14 icon(s) created out of 32 created unit(s); the camera unit is the
        controlled unit (004b4b00 is a record, there is no camera at game+19FCh), so the
        player's own ship carries no dot and is the centre direction wedge
  minimap frame 40   placed=14 drawn=19  first=Java local=(-0.00976, -0.00040, -1.0)
  minimap frame 280  placed=16 drawn=21  first=Java local=(-0.00958, -0.00217, -1.0)
markers selection: 006435d0 marks the controlled unit, the interface manager's target, the
        squad members within unit+7C4h, the objectives, the command units and the target
        group. This process has only the controlled unit, so the pass added 1 marker(s)
summary mission hud screens=42/42 pages=52/52 widgets=204/373 interface=25h
        level1_screens=11 level1_contexts=6 pump_frames=290 update_frames=290
summary mission minimap range=4000 visibility=4000 from_data=1 icons=16 placed=16 culled=15
        frames=581 heading=-0.3820 rad
summary mission markers added=1 rejected=0 on_screen=1 collapsed=1 widgets=1 frames=579
        camera=orthographic half=6000
summary bridge_open=1 atlas=interface/textures/allbutingame_dxt1.ats atlas_items=678
        textures=14 quads=194 frames=330
host methods 349 concrete, 377 unimplemented
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3: all four match milestones 2d, 2h, 2i
and 2j exactly. A `--mission-frames 60` run with no `--mission-complete-frame` still ends on
the frame count with `summary mission exit reachable=0`, and the 300 frame run above still
leaves state 0Dh through 004D7970 and exits on the front-end request rather than on the frame
count. `--trajectory-csv` writes the same twelve columns, and the acceptance form
`--order-frame 1 --order throttle=1,rudder=1` reproduces milestone 2j's published numbers
exactly: `heading -41.253, fwd 16.430, yaw -0.06109` at t = 14 s.

This remains a runtime-validated process, not a game-validated one. What it now proves, that
milestone 2j did not, is that the recovered interface classifier turns a controlled ship into
the captain HUD's own screen set, that the recovered minimap pass places one icon per unit at
the position the recovered camera-relative transform computes with the range the game's own
data file carries, and that the recovered marker pool, gate, dispatch and screen-bounds rule
run end to end over a real unit. It proves nothing about the camera all three of those
readings ultimately need, about the render order the sprite bridge stands in for, or about
the five marker sources this mission never reaches.

### Follow-up packets

1. **The in-mission camera at game+19FCh.** It is the single stand-in behind the minimap
   heading, the marker projection and the crosshair pick, and it is what would turn this
   milestone's three records into three reads.
2. **`hud_minimap_icon_entries`**: 005BE110, 005BD590, 005C0700, 005BCC00 and 00694A60, so
   the 0Ch-byte entry and the group it is parented to replace the executable's clone.
3. **`ship_hull_body_creation` in the executable**, correction 3: 00937C90's tail and
   004DDB90's world settings, which `src/ship_motion_probe.cpp` already runs.
4. **`gui_render_order`**, correction 5, and with it the sprite bridge's coordinate mapping
   for a wide-screen back buffer, which is what puts the minimap cluster half outside the
   window at 2560x1440.
5. **`hud_minimap_capture_points`** and **`hud_minimap_terrain_texture`**, the two other
   follow-ups `docs/HUD_MINIMAP.md` lists.
6. **The five marker sources**: `hud_marker_objective_pass`, the interface manager's target,
   the squad list on the world object, the command-unit sweep and the target group.
7. **`construct_world` 004DE610**, unchanged from milestones 2h, 2i and 2j.
8. **The world itself.** The ships are on the HUD now; what still draws nothing is the scene,
   004CA440 and 004CA1F0, both records since milestone 2f.

## Milestone 2l: the mission's orders as the game issues them

Addresses: 0046aab0 with its issue call site 0046ac0b and the descriptor block
0046aba2-0046abf3; 0077d600 with the builder 007798d0 and the AI-group notify
0077d787-0077d7a3; 00816e30 with 00816e9c / 0077a050, 0081733e / 0071d880, 0081734b
(00835e90), 0081735d and the unprojected arm block 00816f7c-00817330; 0071ecf0 with
0071ed19, 0071ed43 / 007788b0, 0071ed54 / 00a2bd90, 0071ed62 (0071e550), 0071ed6c /
0071c830 and 0071ed81 / 0077c2a0; 00721a40's 5Ch arm with 00721b47 / 00721030, 00721b4f /
007216d0 and 00721b5a (008358d0); 008358d0 with 008358df / 0071e6c0 and 00835930 / 00835860;
0071e6c0 with 0071e6c3 / 0071d780, 0071e70c / 0071e200, 0071e72a / 0071d6d0, 0071e73c
(00836040), 0071e764, 0071e76c / 0071db50, 0071e78a / 00694a60, 0071e795 and 0071e7c9 /
006e38e0; 0071be40; 00835c70's `cruise` arm 00835e0e-00835e5d with 00835e12 / 0071d810,
00835e46 and 00835e58 / 00835ac0; 009e1170's AI arm 009e1265-009e13b1 with 009e126b /
008356f0, 009e1282 / 0092d730, 009e130e / 008356c0, 009e135c / 008356e0, 009e1367 /
009e0040, 009e138c / 008356d0, 009e1397 / 009dffb0 and 009e13a6 / 009dbf90, and its player
arm 009e11e8; 0078061c, 00780649 / 00778820 and 00780653 / 00721a40 (the session dispatch);
00898750, 00875e55 / 00888230 and 00875e64 / 00929460 (the script manager); 00928a00 with
0077e830 and the entity tail 0089903c; and 00b2aff9 with 00c0683c for the precision read.
Packet `cc_exe_2l`, owner `agent/cc-exe-2l`. Sources: `src/game_hosts_commands.cpp`,
`include/bsp/game_hosts_commands.hpp`, plus edits to `src/game_hosts_units.cpp`,
`src/game_hosts_lua.cpp`, `src/game_hosts_scene_contents.cpp`,
`src/game_hosts_mission_frame.cpp`, `src/game_hosts_mission.cpp`, `src/game_hosts_menu.cpp`,
`src/game_hosts.cpp`, `src/game_main.cpp` and their headers. Report:
`reports/game_executable_milestone_2l.json`. Ghidra was read-only for this packet.

Milestones 2i to 2k moved 32 destroyers by writing an order-ring order that stood in for
each ship's authored `Cruise`, and said so. `docs/CRUISE_COMMAND.md` then recovered what the
token means, and the answer is the opposite of an order: **`Cruise` is a latch.** When it
becomes a unit's current command it captures the ring's ordered pair and the unit's heading
and re-applies what it captured every step, so a ship whose ring is zero holds zero. This
milestone runs that path instead of the stand-in, and the mission's ships stop.

### 1. The command path, end to end

`GameCommandsHost` owns one weapon director per created unit and runs the chain the game
runs. Nothing in it is new reconstruction: every step is a call site of
`bsp::SceneDeferredReferenceHost`, `bsp::EntityOrderHost` or `bsp::CruiseCommandHost`.

| Hop | Native | What one authored `Cruise` did |
| --- | --- | --- |
| resolve | 0046aab0 | the token against the 26-row registry at 00e19a70, first match, case-insensitive |
| issue | 0077d600 at 0046ac0b | 007798d0 at 0077d7be builds MT_COMMAND, ordinal at +20h, flags 1 at +21h |
| route | 0077c2a0 at 0077d7d3 | recorded; the executable delivers the message to the same process |
| apply | 00816e30 | flags 1, so 0081733e clears every slot and 0081735d issues |
| director | 0071ecf0 | the AI-group block skipped, 0071e550 drops nothing, 0071c830 builds MT_GAMEUNIT_SETCMD |
| receive | 00721a40's 5Ch arm | session mode 1, message flag 1, so 00721b5a calls 008358d0 |
| push | 008358d0 / 0071e6c0 | slot 0 stored, director+30h None to 1 at 0071e795 |
| current | 0071be40 | mode 1, so slot 0's command is the current one |
| latch | 00835c70's `cruise` arm | 00835ac0 with the ring pair and the heading |
| step | 009e1170's AI arm | once per unit per fixed simulation step |

**Not all 32 ships author `Cruise`.** Thirteen do; the other nineteen carry `None`, which is
the default `properties Command` declares in `universe/library/commandunit.props` and is
value 1 of that file's own `enum CommandType`, not a command class. `None` matches no
registry name, so 0046aab0 ends those records at 0046ab13 and builds no message for them.
That corrects milestone 2h, and it means milestone 2i's stand-in was wrong twice over: it
gave a throttle to nineteen ships that authored no command at all.

All thirteen that do reach the latch **latch a zero**, because the load leaves the ring at
its constructed state and `00835ac0` therefore stores `cruiseIsHeading = 1`,
`cruiseSteerOrHeading = the heading` and `cruiseThrust = 0`. Thirteen ships hold station for
all 290 in-mission frames, and the only ship that moves is the one `--order` drives.

**The latched pair reaches no ring, and that is the packet's boundary.** 009e1170's three
setters 009dbf90, 009dffb0 and 009e0040 write the AI controller block at `[state]+8`
(+1d0h throttle, +1d4h rudder, +1d8h heading, +1c4h mode), and the hop from that block to
`unit+0fc4h` / `unit+0fdch`, which is what 00825f20 copies into the ring under the
`unit+61h` gate at 008266c1, has no recovered writer. That is `docs/CRUISE_COMMAND.md`'s
follow-up `unit_autopilot_pair`, and it is now the single step between a latched command and
a ship that obeys it.

The controlled unit takes a different arm: with `unit+184h` set, 009e1170 runs
009e11e8..009e1262, which forwards the ring's **confirmed** pair and touches no cruise
field. That arm is not projected, so it is recorded and the AI arm is not run in its place.

### 2. The new `--order` forms

`--order` still takes `throttle=<f>,rudder=<f>` and writes the controlled unit's ring
through 00816a40, which is the player's own path and is unchanged. It now also takes the
name of one of the 26 command classes, with an optional `:<entity>` target, and issues it
through the whole chain above:

| Form | What the run does |
| --- | --- |
| `--order cruise` | resolves, pushes slot 0, latches (heading arm, steer 0.000, thrust 0.000) |
| `--order stop` | resolves and pushes, raises the command stage, latches nothing: 00835e17 compares the command against 00e08f70 first |
| `--order settarget:Haguro` | resolves and reaches 00816e30, whose own arm for it (00816f7c..00817330) is recorded as unimplemented, so no slot is pushed |
| `--order moveto=1000,-2000` | refused at parse time, exit 2 |

The last row is the honest answer to the brief's "when the reconstruction supports the
descriptor". It does not: 0046aab0 builds only two descriptors, a named target or the
owner's own world position, and 00816e30's arm for a command that carries a coordinate is
not projected. There is nothing for an authored coordinate to travel through.

### 3. The mission's own orders, and why they are not the AI's

The brief asked for the mission's AI groups to be run from their scripted `AICreate` and
`AISetCommand` bindings. **No installed mission script calls them.** A scan of the 299 `.lua`
files under `Scripts/missions` finds zero call sites for `AICreate`, `AICreateGroup`,
`AISetCommand`, `AIEnable`, `AIEnableGrouping`, `AIMergeGroups`, `AIGetGroupInfo`,
`AIReloadGlobals`, `AIGetTargetWeight` and `AISetQuickSpawnTargetPos`; only the four tuning
bindings appear (`AISetHintWeight` in 32 files, `AISetSpawnSceneUnitsWeightMul` in 31,
`AISetTargetWeight` in 29, `AISetDefendResourcePercent` in 6). `usn_2_java.lua` calls no AI
binding at all. That is what `docs/LUA_BINDING_AI.md`'s own "What the installed scripts
reach" section already says, and it is why both AI-group forwards in the command chain
(0077d787 on the commanded entity and 0071ed54 on the session endpoint's subject) are not
reached: nothing writes `entity+16ch`.

What does issue this mission's orders is its own script. `luaStageInit` calls
`CreateScript("luaInit")`, and `luaInit` is where every order lives. Three things were in
the way and two of them are now gone:

- **The script manager.** `CreateScript` 00898750 registers a script object and the fixed
  step's rows 7 and 8 (00888230 at 00875e55, 00929460 at 00875e64) would run it. All three
  are records, so the executable keeps the name the binding was handed and calls that global
  once, with one fresh table, on the frame the mission enters state 0Dh. The call is the
  executable's; the name is the mission's.
- **`thisTable`.** Milestone 2f created the self table empty and said 00928a00 adds one slot
  per entity and this process creates none. It creates 32 now, so the executable builds the
  32 slots with the `ID`, `Dead` and `Ptr` fields 00928a00 seeds, and assigns each one the
  installed `VehicleClass` row as its `Class`: `docs/MISSION_LUA_SELF_TABLE.md` records that
  `Class` is added later by a per-kind setter through 00b675d0, and that the value is the
  `VehicleClass` row is settled by the scripts themselves, which read `.Class.Type` against
  the literal set those rows' own `Type` keys carry and also read `.Class.Length`,
  `.Class.Name`, `.Class.Height` and `.Class.Width`. 00928a00, 0077e830 and the `Class`
  setter stay records; the slot is the executable's.
- **`FindEntity`.** With the slots built, the entity tail of the nineteen entity-returning
  rows can take its resolved arm instead of the nil arm at 0089903c. Only `FindEntity` is
  resolved, and only over the created instances: 00925a90's own lookup on the scene database
  is a record, and every other entity-returning row resolves its subject from game state
  this process does not own.

With those three in place the mission's own order function runs and **addresses 21 of the 32
created instances through ten bindings**:

| Binding | Row | Instances | Calls |
| --- | --- | --- | --- |
| `NavigatorSetTorpedoEvasion` | 008a3cd0 | 21 | 21 |
| `NavigatorSetAvoidLandCollision` | 008a3b10 | 21 | 21 |
| `SetSkillLevel` | 00895250 | 15 | 15 |
| `JoinFormation` | 00899d10 | 14 | 14 |
| `SetInvincible` | 00897a50 | 10 | 10 |
| `NavigatorAttackMove` | 008a30d0 | 6 | 6 |
| `SetFireTarget` | 0089a8b0 | 6 | 6 |
| `RepairEnable` | 008ad330 | 6 | 12 |
| `SetRoleAvailable` | 008ab850 | 4 | 4 |
| `NavigatorMoveToRange` | 008a2f20 | 1 | 1 |

Every one of the ten is a host record with its own row address, so **no order reaches a
ship**. That is the answer to "how many ships end up under an order and how far they
travel": the scene puts 13 under a `cruise` that latches a zero, the script addresses 21
through bindings that are records, and every ship but the one `--order` drives ends the run
at the coordinates the scene placed it at, to the printed 0.01 m, over 14.5 simulated
seconds.

`luaInit` itself stops part way, and the reason is worth recording: it reached 16 further
bindings in 149 calls and then failed at `scripts/global/commandhelpers.lua:1003` with
`attempt to compare number with nil`, because `luaPickRnd` compares the result of `luaRnd()`
and that binding is a record that returns no value. The first shipped script line that
consumes a binding's **return value** is where a run of this kind ends; the calls before it
are real.

### 4. The precision boundary

`docs/X87_CONTROL_WORD.md` established statically that the CRT startup asks for 53-bit
precision (`__setdefaultprecision` pushes `_PC_53` under `_MCW_PC` at 00c0683c), that no
game code changes the field again, and that the Direct3D 9 device is created without
`D3DCREATE_FPU_PRESERVE` at 00b2aff9, so `d3d9.dll` should drop the field to 24 bits for the
life of the device. Its first uncertainty was that the last step is documented API behaviour
rather than an instruction in the image, and that no run log had been taken. This milestone
takes it: the executable reads `_controlfp_s(&v, 0, 0) & _MCW_PC` once in `WinMain` before
Direct3D exists and once on the first fixed simulation step. The answers are `0x00010000`
(53-bit) and `0x00020000` (24-bit). The static reading was right, and the reads change no
arithmetic.

What that means for this reconstruction is narrower than it sounds. The shipped executable
evaluates 00825f20's `float` expressions on the x87 stack with a 24-bit mantissa, which
rounds each intermediate to single precision; this reconstruction evaluates them with SSE2
`float` under `/fp:strict`, which rounds each intermediate to single precision as well. For
every expression whose operands and result are `float`, the two agree bit for bit, and the
motion path is almost entirely such expressions. The places where they can differ are the
ones that load a `double` constant: the `0.05` at 00d7a270 that 009e1170 compares the
throttle against, and the `double` pairs inside 00825f20. On x87 those comparisons and
products are formed at 24 bits of mantissa with a 15-bit exponent; here they are formed at
53 bits and rounded once. A 24-bit intermediate can only change the answer when the
`double` operand's extra mantissa bits matter, which for a comparison against 0.05 means a
throttle within about 3e-9 of that threshold, and for the `1.0` pairs means nothing at all.
On x87 those comparisons and products are formed at 24 bits of mantissa; here they are
formed at 53 and rounded once. The `0.05` threshold is the only one a trajectory could turn
on, and the gap between it and its 24-bit image is about 7e-10, so a run would have to hold
a throttle inside that band for the two sides to disagree. This run never does: the latched
throttle is exactly 0 and the ordered one exactly 1. **The precision boundary is stated, not
measured against the game**, and the honest form of the claim is that the two sides round
`float` arithmetic identically and differ only in how they carry a `double` intermediate,
which no trajectory here makes decisive.

### What it looks like on screen

Unchanged from milestone 2k, with one difference that is the point of the packet: the ships
no longer move. The capture at in-mission frame 280 is the captain HUD over the cleared dark
blue buffer, the minimap cluster in the top right with its authored `error.tga` island map
and compass ring, the repair wheel in the middle left, the engine telegraph and the damage
bar in the bottom right, the `Artillery`, `Fighter Ace` and `Repair` text runs, and the one
unit marker with its `Unit Name` label and `1254` distance. The sprite bridge holds the same
192 quads. On the minimap, fourteen icons are placed every frame and they now drift only
because the camera ship moves: `Java`'s local position goes from (-0.00977, 0.00000) at
in-mission frame 1 to (-0.01015, -0.00005) at frame 500, which is the camera's own 196 m of
travel and rotation, not the ship's. The captures are the ignored `local/run_2l.png` at the
validation machine's 2560x1440 and `local/run_2l_1024.png` at 1024x768 through an isolated
`--settings-personal-root`; the wide-screen caveat of milestone 2k's follow-up 4 is
unchanged, and at 2560x1440 the minimap cluster still sits half outside the window.

### Host methods

`bsp_game.exe --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 300
--mission-frame-seconds 0.05 --order-frame 0 --order throttle=1,rudder=0.5
--mission-complete-frame 290 --trajectory-csv local/trajectory.csv --screenshot
local/run_2l.png --screenshot-mission-frame 280 --log local/game_run_2l.log --game-root
"<install>"`, exit 0: **375 concrete, 412 unimplemented**. Milestone 2k's published pair for
the same shape of command is 349 and 377.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2l.json` (`command_path_steps`, `cruise_step_steps`,
`session_delivery_steps`, `mission_script_steps`). The counts by group:

| Group | Steps | Concrete | Records |
| --- | --- | --- | --- |
| The command path 0046aab0 to 0071e6c0 | 35 | 18 | 17 |
| The cruise step 009e1170 | 11 | 6 | 5 |
| The session delivery of the three messages | 5 | 0 | 5 |
| The script manager and the entity tables | 5 | 1 | 4 |

### Corrections

1. **Milestone 2h's "the `Command` property, which all 32 author as `Cruise`" is wrong, and
   with it milestone 2i's stand-in.** Thirteen instances author `Cruise`; nineteen carry
   `None`, the default of `properties Command` in `universe/library/commandunit.props`, which
   is that file's `enum CommandType` value 1 and not a command class. Nineteen ships were
   given a throttle by a stand-in for a command they never authored.
2. **`docs/CRUISE_COMMAND.md`'s "both producers ... a descriptor whose kind is 0 and whose
   position is the read-only zero vector at 00F87574" is right for the Lua producer only.**
   The scene queue's targetless branch fills the descriptor with the **owner's own world
   position**: 0046abb7, 0046abc5 and 0046abd8 load owner+fch, +100h and +104h into
   [ESP+20h], [ESP+24h] and [ESP+28h], the kind byte is cleared at 0046abef and
   position_valid is set to 1 at 0046abe0. The two producers agree on the command object and
   the flags, not on the descriptor. Nothing about `cruise` depends on it, because the latch
   reads the ring rather than the descriptor, and `src/scene_deferred_refs.cpp` already had
   the branch right; it is the summary sentence that is wrong.
3. **Milestone 2i's "what that token means is not recovered" is superseded**, and so is the
   stand-in it justified. The token is a latch and it writes no ring.
4. **This packet's brief asked for the mission's AI groups to be run from scripted `AICreate`
   and `AISetCommand` calls. There are none**, in this mission or in any of the 299 installed
   mission scripts; section 3 has the corpus scan. The mission's orders are the `Navigator*`
   family, from the function its stage init hands to `CreateScript`.
5. **`docs/X87_CONTROL_WORD.md`'s uncertainty 1 is closed by observation.** Section 4.
6. **Milestone 2f's "the self table is empty because this process creates no entity" is
   superseded.** It creates 32, and the slots are built.

### Code with no Ghidra function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |

Every address this packet touched already has a Ghidra function and a reviewed ledger name.
No name was added; run-time evidence was appended to 0046aab0, 0077d600, 00816e30, 0071ecf0,
00721a40, 008358d0, 0071e6c0, 00835c70, 00835ac0, 009e1170 and 00a2bd90.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest
case `reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2l.json` checks 40
call rows and reports 0 failures; eight rows are reported as indirect because the native call
goes through a vtable slot.

```
x87 precision before Direct3D: 53-bit (double) (_controlfp_s & _MCW_PC = 0x00010000)
thisTable: 32 per-entity slot(s) built for the created scene instances, 32 of them with the
        installed `VehicleClass` row as their `Class` field
  authored token "Cruise" x13 resolves to command class "cruise"
  authored token "None" x19 resolves to no command class: 0046aab0's case-insensitive
        first-match walk over the registry at 00e19a70 finds nothing and ends the record at
        0046ab13, so no MT_COMMAND is built for those units
  DeRuyter             cruise        16    3     1     1     1   heading     0.000     0.000
  Haguro               None          -1   -1     0     0     0         -     0.000     0.000
the latched pair reaches no order ring: 009dbf90, 009dffb0 and 009e0040 write the AI
        controller block at [state]+8, and the hop from that block to unit+0fc4h /
        unit+0fdch under the unit+61h gate that 00825f20 reads at 008266c1 has no recovered
        writer
  script object luaInit              defined=1 dispatched=1 status=2
        [string "scripts/global/commandhelpers.lua"]:1003: attempt to compare number with nil
  script binding NavigatorSetTorpedoEvasion   008a3cd0 addressed 21 created instance(s) in 21 call(s)
  script binding NavigatorMoveToRange         008a2f20 addressed 1 created instance(s) in 1 call(s)
x87 precision at the fixed simulation step: 24-bit (single) (_controlfp_s & _MCW_PC = 0x00020000)
summary mission commands units=32 resolved=13 issued=13 pushed=13 current=13 latched=13
        with_thrust=0 ai_groups=0 ai_forwards=0 steps=3480
summary mission script orders instances=21/32 bindings=10 entity_resolves=33: every one of
        those bindings is a host record with its own row address, so the mission's own
        orders reach no ship
summary mission world units=32 walked=9280 updated=9280 motion_ticks=9280 simulated=14.50 s
        controlled=DeRuyter moved=196.54 total_path=196.55
host methods 375 concrete, 412 unimplemented
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 154 concrete and 80
unimplemented, a 40 frame title-only run reports 129 and 49, `--vfs-probe fonts/fonts.lua`
exits 0 and `--vfs-probe does/not/exist.lua` exits 3: all four match milestones 2d, 2h, 2i,
2j and 2k exactly. A `--mission-frames 60` run with no `--mission-complete-frame` still ends
on the frame count with `summary mission exit reachable=0`, and the 300 frame run above still
leaves state 0Dh through 004d7970 and exits on the front-end request rather than on the frame
count. The acceptance form `--order-frame 1 --order throttle=1,rudder=1` reproduces milestone
2j's published numbers exactly: `heading -41.253, fwd 16.430, yaw -0.06109` at t = 14 s.

This remains a runtime-validated process, not a game-validated one. What it now proves, that
milestone 2k did not, is that the recovered path from an authored scene command to a weapon
director's command slot runs end to end over the mission's own data, that `cruise` latches
what the ring holds and therefore holds a ship at rest, and that the mission's own order
function runs and names 21 of its 32 ships. It proves nothing about what those orders would
do: the hop from the AI controller block to the order ring has no recovered writer, every
`Navigator*` binding is a host record, and no ship moved under anything but the player's own
`--order`.

### Follow-up packets

1. **`unit_autopilot_pair`**, `docs/CRUISE_COMMAND.md`'s own first follow-up: `unit+61h`,
   `unit+0fc4h`, `unit+0fdch` and the writer that carries the AI controller block at
   `[state]+8` into them. It is now the single step between a latched command and a ship that
   obeys it, and it is why thirteen ships hold station under a command this executable runs
   end to end.
2. **`cruise_speed_setting`**, `*(unit+73ch)+24h` and `+28h`. Its enable is read at three
   sites and cleared at one, and nothing writes the speed through a direct displacement.
3. **The `Navigator*` binding family**: 008a2f20 `NavigatorMoveToRange`, 008a30d0
   `NavigatorAttackMove`, 00899d10 `JoinFormation`, 0089a8b0 `SetFireTarget`, 008a3cd0 and
   008a3b10. They are what this mission's own script calls on 21 of its 32 ships.
4. **The script manager**, 00898750 with 00888230 and 00929460, so a script object is
   registered and run by the game's own scheduler rather than called once by the executable.
5. **`ship_ai_state_machine`**, 00d21598 and 009f3dd0, so 009e1170 runs where the game runs
   it and the `moveto`, `follow` and `stop` states exist at all.
6. **`entity_command_arms`**, 00816e30's 00816f7c..00817330, which is what `--order
   settarget`, `--order moveto` and `--order follow` need.
7. **Binding return values.** The first shipped script line that consumes one stops the
   script; `luaRnd` is the one this mission hits.
8. **`construct_world` 004de610**, unchanged from milestones 2h, 2i, 2j and 2k.

## Milestone 2m: the script's orders as real commands, and the hosts that were waiting on a wiring

Addresses: 008a30d0 (`NavigatorAttackMove`) and 008a2f20 (`NavigatorMoveToRange`, with 008a2bc0
and 008a2d70 as the same body), 0088a810 with 00b67910 / 00b65fb0 / 00888aa0 / 00888760 (the
Lua-to-`SceneCommandTarget` reader), 00899d10 through 0077c8d0 with the availability predicate at
vtable 16Ch and 00905300 / 0075b430 / 0077c2a0, 00895250 with vtable 128h, 008ad330 with vtable
5Ch, 008ab850 with 00888d20 / 004bca50 / vtable 148h / 004c3840; 00836920's stage spine
(00836941, 00836a8b, 00836dc9) with 0071be60 / 0071d810 / 0071d9e0 / 00465080 / 0071ecf0 /
007788b0 / 007788d0 and the stage reset 00835bf0 with 0071c130 / 00822b70, and the
commanded-speed store 00890e6f; the six fan-out rows 00875e3a / 0098bdb0, 00875e44 / 00926700,
00875e55 / 00888230, 00875e64 / 00929460, 00875ec9 / 009273a0 and 00875eda / 00903610; the
mission-load rows 004dfc13 with 004cec60 and 004bb160, 004e0754 with 004218e0 and 00424d00, and
004d30f0 with 004cec60 / 00b67980 / 00b66200 / 004d0640; and the four scene-contents steps
004d0ee0 with 004cb160 / 004caf50, 004c17d0 with 004248a0 / 004239e0, 004ba870, and 0046df00's
weather pass with 00b66bd0 / 00b6a020 / 00b69d40 / 00b67800 / 008f5a00 / 008f2260 / 008f3370.
Packet `cc_exe_2m`, owner `agent/cc-exe-2m`. Sources: `src/game_hosts_script_orders.cpp`,
`include/bsp/game_hosts_script_orders.hpp`, `src/game_hosts_ready.cpp`,
`include/bsp/game_hosts_ready.hpp`, plus edits to `src/game_hosts_commands.cpp`,
`src/game_hosts_units.cpp`, `src/game_hosts_scene_contents.cpp`, `src/game_hosts_fixed_step.cpp`,
`src/game_hosts_lua.cpp`, `src/game_hosts_mission_frame.cpp`, `src/game_hosts_mission.cpp`,
`src/game_hosts_menu.cpp`, `src/game_hosts.cpp` and their headers. Report:
`reports/game_executable_milestone_2m.json`. Ghidra was read-only for this packet.

Milestone 2l watched `usn_2_java`'s own order function address 21 of the mission's 32 created
instances through ten bindings that were all host records, and said that no order reached a ship.
Packet `cc_lua_navigator` then read eight of those ten rows. This milestone runs them, and the
answer is one hop better and still short of a moving ship.

### 1. The mission's orders are real commands now, and where they stop

`GameScriptOrdersHost` is the executable's host for `src/lua_binding_navigator.cpp`. Six of the
mission's ten binding rows run their reconstructed bodies over the created instances; the other
four (`NavigatorSetTorpedoEvasion`, `NavigatorSetAvoidLandCollision`, `SetInvincible`,
`SetFireTarget`) belong to other packets and keep milestone 2l's record.

What one run of `luaInit` produced:

| Binding | Row | Calls | What reached a ship |
| --- | --- | --- | --- |
| `SetSkillLevel` | 00895250 | 15 | nothing: vtable 128h is a record |
| `RepairEnable` | 008ad330 | 12 | nothing: every ship answers `IsKindOf(6)`, so the routed 9Fh message arm runs and the session is a record |
| `JoinFormation` | 00899d10 | 14 | nothing: 0077c8d0's first question is vtable 16Ch |
| `NavigatorAttackMove` | 008a30d0 | 6 | an `attackmove` MT_COMMAND, routed and applied |
| `SetRoleAvailable` | 008ab850 | 4 | nothing: vtable 148h is a record |
| `NavigatorMoveToRange` | 008a2f20 | 1 | a `moveto` MT_COMMAND, routed and applied |

The seven navigator calls build a real command: the fixed command object (`00e08f78`
`attackmove`, `00e08f68` `moveto`), the descriptor `0088a810` read, and the constant flags 1.
`0077d600` builds MT_COMMAND and routes it, and `00816e30` applies it. **That is where they
stop.** `00816e30`'s own arms `00816f7c..00817330` hold `moveto` and `attackmove` and
`docs/CRUISE_COMMAND.md` projects none of them, so the command never reaches a weapon director
slot. Milestone 2l already recorded that boundary for `--order settarget`; this milestone is the
first run in which the mission's own orders reach it.

```
  binding                    unit             command    target           issued   slot
  NavigatorMoveToRange       DeRuyter         moveto     (position)            1      0
  NavigatorAttackMove        Haguro           attackmove DeRuyter              1      0
  NavigatorAttackMove        Jintsu           attackmove Java                  1      0
  NavigatorAttackMove        Yudachi          attackmove DeRuyter              1      0
  NavigatorAttackMove        Samidare         attackmove Java                  1      0
  NavigatorAttackMove        Murasame         attackmove DeRuyter              1      0
  NavigatorAttackMove        Harusame         attackmove Java                  1      0
```

Two things in that table are worth naming. The six Japanese destroyers attack the two Dutch
cruisers by name, which is the mission's own pairing and is read out of the created instances
rather than assigned here. And the Dutch `moveto` carries a position rather than a target,
because its argument is `FindEntity("DRGoTo")` and `DRGoTo` is one of the scene's two
`NavPoint`s, whose creator `004e99b0` is a record: the lookup answers nil, so `0088a810` takes
its ID-absent arm at `0088a8a1` and the descriptor becomes the origin with `position_valid` set.
That is the native's own behaviour for a value that carries no `ID`, not a failure.

**Every ship still holds station.** The run's distance table is unchanged from milestone 2l:
`moved 0.00` for all 32 over 19.5 simulated seconds. The three reasons, in the order they bite:

1. a scripted `attackmove` or `moveto` does not reach a director slot (`entity_command_arms`);
2. the thirteen scene-authored `cruise` commands latch a zero ring, as milestone 2l established;
3. even a latched or commanded value would reach no ring, because the hop from the AI controller
   block at `[state]+8` to `unit+0fc4h` / `unit+0fdch` has no recovered writer
   (`unit_autopilot_pair`).

The AI decision routines that are still records, named: `009f3dd0` and the ship AI state class
family at `00d21598` (so the `moveto`, `attackmove`, `follow` and `stop` states do not exist),
`00816f7c..00817330` (the entity command arms), `00836adc..00836d66` (the director's `follow`,
`attackmove` and `moveonpath` step arms), `00a2bd90` (the AI group forward, unreached because no
script of this mission writes `entity+16ch`), `008162b0` (the command-availability predicate) and
the three setters `009dbf90` / `009dffb0` / `009e0040`, which run and write a block nothing
reads.

### 2. The weapon director's stage ladder, and what it does to a ship with no orders

`00836920` runs once per unit per fixed simulation step. Nothing in it is new: the pre-pass
`00836941`, the `stop` arm `00836a8b`, the idle tail `00836dc9` and the stage reset `00835bf0`
are `docs/UNIT_COMMANDED_SPEED.md`'s routines. Its own caller is the unit update's director
block, which this process does not reach, so the position in the step is the executable's
decision and is recorded as one.

What it decides on this mission is the first thing in three milestones that changes the
commands the ships hold. **Nineteen of the 32 authored `None`** (milestone 2l's correction 1), so
their directors have no primary command and no filled slot, the pre-pass flag is set, the idle
tail runs and `00836e59` finds no commanded speed: all nineteen are issued **`stop`** through
`0071ecf0`, and the slot push takes it. The controlled unit takes the other branch: `00836e45`
reads `unit+184h`, so its idle tail chooses `cruise`. The thirteen ships that author `Cruise`
never reach the tail, because their primary stage is 1 with one filled slot.

```
summary mission director steps=12480 idle_reissues=20 stop=19 cruise=1 follow=0
        script_issues=7 blocked_at_00816f7c=7 commanded_speeds=0
summary mission commands units=32 resolved=13 issued=20 pushed=33 current=33 latched=14
```

### 3. The commanded speed, and the new switch

The pair at `*(unit+73Ch)` `+24h` / `+28h` is no longer a default-constructed record.
`GameCommandsHost` owns one navigator parameter block per unit, the director reaches it the way
the native does (through `[director+24Ch]+73Ch`), the stage reset `00835bf0` ages it against
`00835c28`'s one-second budget, the `stop` arm and the idle tail read it, and the cruise state
reads it at `009e12ac`.

`--order speed=<m/s>` makes the store `luaMW_SetShipSpeed` `00890d30` makes, on the controlled
unit. It is not a throttle, and the log says so: `+24h` is the clamped request and `+28h` is the
mission clock `DAT_00F876A4`. **Neither of the two producers is called by this mission**:
`usn_2_java.lua` calls neither `SetShipSpeed` nor `NavigatorMoveOnPath`, so without the switch
every block stays at the constructor's `-1.0f` and the run reports `commanded_speeds=0`.

With the switch the pair goes active and the run says where it stops: the controlled unit is
player-controlled, so `009e1170` takes the `009e11e8` arm and never reads a cruise field, and the
idle tail's choice for that unit was already `cruise`. So the switch exercises the producer and
the block, and on this mission it changes no trajectory.

### 4. The hosts that only needed wiring

Six fan-out rows, three mission-load rows and four scene-contents steps had reconstructions on
main and were still records. All thirteen now run.

| Row | Native | What it did here |
| --- | --- | --- |
| `refresh_moved_spatial_nodes` | 0098bdb0 | the walk runs over an empty root list: nothing registers a spatial node, 0098a310's callers are the scene graph's |
| `drain_deferred_entity_events` | 00926700 | the queue is empty; 780 passes over 390 steps, because rows 6 and 14 are the same callee |
| `drain_queued_lua_calls` | 00888230 | the deferred call list is empty: its producer 00887560 only runs for a named call made off the main thread |
| `run_due_entity_think` | 00929460 | both think lists are empty: 0088a240 is the `SetThink` binding's |
| `flush_pending_entity_queues` | 009273a0 | both pending lists are empty: 00922fd0 is a record |
| `release_expired_world_objects` | 00903610 | **walks this mission's 32 created instances**, 12480 visits over 390 steps; every counter at entity+6Ch is zero, which is 00903625's skip |
| `reset_network_slots` | 004dfc13 | the recovered branch, which for a local session is 004dfd18 and the single-player reset 004bb160. It was a skipped arm |
| `reset_objective_list` | 004e0754 | the two clears, the tree erase and the avoid-zone rebuild; 00424d00 itself stays a record because it scans a world that does not exist |
| `rebuild_scripted_name_list` | 004d30f0 | **878 function-valued names out of 1178 Lua globals**, the pre-script baseline teardown nils against |
| `preload_record_effects` | 004d0ee0 | clears the handle vector and resolves zero names: record+C70h is a consumer-side read the header pass does not fill |
| `load_avoid_zones` | 004c17d0 | **172939 bytes of the mission's `.nav` parsed into 3 TerrainGridLayers**, 240x240 at 100.0 m per cell |
| `scatter_clouds` | 004ba870 | returns at its own gate 004ba879, because record+C84h is zero |
| `select_weather_descriptor` | 0046df00 | the whole `Weathers` walk on a private Lua state; see the corrections |

Three that the packet brief listed as ready could not be wired, and the report says why for each:
`00871ba0` (the effect acquire needs a `GameplayEffectAcquisitionContext` nothing here builds),
`00922e20` (the 0Ch property-bag holder is a host method of `SceneEntityCreateHost`, not a
routine, and 00922e20 itself has no reconstruction) and `0095c640` (no reconstruction on main at
all: Ghidra still carries `FUN_0095c640`, and the address is leased to
`agent/cc2-scene-traffic-groups`). `004c3840` has a reconstruction and stays an arm the load
never takes, because `004e044d` gates it on `game+1FE4h == 1`.

### What it looks like on screen

Unchanged from milestone 2l, and for milestone 2l's reason: what a mission would draw goes
through `004ca440` and `004ca1f0`, both records, and no ship moves. The capture at in-mission
frame 380 is the captain HUD over the cleared dark blue buffer: the repair wheel with its four
quadrant icons in the middle, the engine telegraph with its `STOP` / `HALF` / `3/4` / `FULL` dial
and the rudder indicator in the bottom right, the damage bar under them, the `Artillery`,
`Repair` and `Fighter Ace` text runs, the one unit marker with its `Unit Name` label and `1254`
distance, and the minimap cluster's authored `error.tga` island map in the top right. The sprite
bridge holds the same 192 quads. The capture is at the validation machine's 2560x1440, where
milestone 2k's wide-screen caveat still puts the minimap cluster half outside the window; it is
written to the ignored `local/run.png` and is not committed.

### Host methods

`bsp_game.exe --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 400
--mission-frame-seconds 0.05 --mission-complete-frame 390 --trajectory-csv local/trajectory.csv
--screenshot local/run.png --screenshot-mission-frame 380 --log local/game_run.log --game-root
"<install>"`, exit 0: **432 concrete, 404 unimplemented**. The same command on this tree before
this packet reports **375 and 411**.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2m.json` (`script_order_steps`, `script_order_indirect`,
`script_order_field_reads`, `director_step_steps`, `fixed_step_rows`, `mission_load_steps`,
`scene_contents_steps`, `still_unimplemented`). The counts by group:

| Group | Steps with a call site | Concrete | Records |
| --- | --- | --- | --- |
| The eight binding bodies | 27 | 19 | 8 |
| The director's stage ladder 00836920 | 13 | 11 | 2 |
| The six fan-out rows | 6 | 6 | 0 |
| The three mission-load rows | 10 | 9 | 1 |
| The four scene-contents steps | 21 | 20 | 1 |

Twenty-four records the run reached before are gone, and sixteen new ones appeared, each a native
call site this executable had never reached: `00816f7c` (the entity command arms), `0077c8fe`
(the availability predicate), `0089539a`, `008ad4cd`, `008aba51`, `0088a88e`, `00836adc`
(the director's step arms), `0071c730` (the stage completion message), `007788b0`, `00424d00`,
`00521ea0`, `006fe530`, `00694a60`, `009e1170`'s stop-state step and the entity-think GC gate.

### Corrections

1. **This packet's brief asked for the script's orders to move the ships. They do not, and the
   reason is one hop further on than the brief assumed.** The bindings run, the commands are real
   and routed, and `00816e30`'s own arm for `moveto` and `attackmove` is projected nowhere, so no
   scripted order reaches a weapon director slot. That block is milestone 2l's follow-up 6,
   `entity_command_arms`, and it is now the first of three steps between the mission's script and
   a ship that obeys it.
2. **This packet's brief listed `0095c640` among the hosts with a reconstruction on main. It has
   none.** `python tools/bsp.py lookup 0095c640` answers `FUN_0095c640` with no ledger name, and
   the address is leased to `agent/cc2-scene-traffic-groups`.
3. **`00871ba0` and `00922e20` are reconstructed and still not wireable.** The effect acquire
   takes a `GameplayEffectAcquisitionContext` (a manager context, a native string storage, a
   name-index host and a scalar-component dispatcher) that nothing in this process builds, and
   `00922e20` is a host method of `SceneEntityCreateHost` rather than a routine. This is the same
   class of answer milestone 2d gave for the scene graph: reconstructed is not the same as
   callable.
4. **This installation ships an empty weather table.** `scripts/datatables/weather.lua` opens a
   `--[[` block on its second line and closes it on line 136, its last content line, so every
   authored entry is inside the comment and `Weathers` evaluates to `{}`. `select_weather_entry`
   therefore matches nothing for any mission on this installation, and the pass writes no shadow
   key. The four `g_Terrain.*` writes a run does perform come from `0046df00`'s own
   console-variable block, not from the weather row.
5. **Milestone 2h's "all 32 author `Cruise`" was corrected by 2l, and the consequence is now
   visible.** Nineteen ships author `None`, so their directors are idle and the recovered idle
   tail issues `stop` to every one of them. No milestone before this ran that tail.
6. **`docs/MISSION_LOAD_HOSTS.md`'s three renamed host methods.** The executable's load walk
   accepts both the old spellings (`reset_network_slots`, `reset_objective_list`,
   `rebuild_scripted_name_list`) and the corrected ones (`erase_native_string_set`,
   `rebuild_avoid_zone_table`, `record_script_function_baseline`), because that rename is landing
   on main while this packet is open.
7. **A 120 frame run now reports 155 concrete and 79 unimplemented**, one more concrete than
   milestone 2l published, and the move is not this packet's: every record that run reaches has
   the same status it had on the tree this packet branched from.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 00835bf0 | 00835c6f | The director's vtable slot 6Ch, which the idle tail invokes at 00836e0b. `docs/UNIT_COMMANDED_SPEED.md` carries both boundaries and the define command (`python tools/ghidra_define_function.py 00835bf0 00835c70`); until it runs, `tools/verify_report_calls.py` cannot check the two rows inside that body, which the report lists under `native_unverified`. |

Every other address this packet touched already has a Ghidra function. No name was added and no
Ghidra annotation was made: the packet is a wiring of reconstructions other packets recovered.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest case
`reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2m.json` checks 79 call
rows and reports 0 failures; five rows are reported as indirect because the native call goes
through a vtable slot.

```
  binding SetSkillLevel                argc=2 phase=luaInit (reconstructed body)
  binding NavigatorAttackMove          argc=2 phase=luaInit (reconstructed body)
JoinFormation stops at its first question. 0077c8d0 asks the follower's vtable 16Ch whether it
        may `follow` the leader, which for MDestroyer is 008162b0; that body was not read by the
        packet that reconstructed the binding, so the host answers the neutral false
summary mission script bindings calls=52 attackmove=6 moveto=1 issued=7 reached_director=0
        units=7 formations=0/14 skills=15 repairs=12 roles=4
summary mission script orders instances=21/32 bindings=10 reconstructed=6 entity_resolves=33
weather pass: SCRIPTS\datatables\Weather.lua carries 0 entry(ies) and 0 sub-scene(s)
  the installed table is empty: this installation's scripts/datatables/weather.lua opens a
        `--[[` block on its second line and closes it on its last
avoid zones: 172939 byte(s) of the scene's `.nav` parsed into 3 TerrainGridLayer(s)
scene record effect preload: 004d0ee0 cleared the handle vector at record+D50h and resolved 0
        name(s)
cloud scatter: 004ba870 read the gate at record+C84h, found 0 and returned at 004ba879
mission load session-slot reset: 004dfc13 tests game+1FE4h and this session is local, so the
        routine takes 004dfd18, the single-player reset 004bb160
mission load scripted-name baseline: 004d30f0 walked 1178 Lua global(s) and recorded 878
        function-valued name(s) into the set at game+1930h
summary fixed step subsystems spatial=390/0 deferred_events=780/0 lua_calls=390/0 think=390/0
        pending_queues=390 expiry=390/12480 released=0
summary mission director steps=12480 idle_reissues=20 stop=19 cruise=1 follow=0 script_issues=7
        blocked_at_00816f7c=7 commanded_speeds=0
summary mission world units=32 walked=12480 updated=12480 motion_ticks=12480 simulated=19.50 s
        controlled=DeRuyter moved=0.00 total_path=0.00
host methods 432 concrete, 404 unimplemented
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 155 concrete and 79
unimplemented, a 40 frame title-only run reports 130 and 48 (see correction 7),
`--vfs-probe fonts/fonts.lua` exits 0 and `--vfs-probe does/not/exist.lua` exits 3. A
`--mission-frames 60` run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`, and the 400 frame run above still leaves state 0Dh through
004d7970 and exits on the front-end request rather than on the frame count. The acceptance form
`--order-frame 1 --order throttle=1,rudder=1` reproduces milestone 2j's published numbers
exactly: `heading -41.253, fwd 16.430, yaw -0.06109` at t = 14 s, so the director's new per-step
ladder changes no trajectory. `--trajectory-csv` writes the same twelve columns.

This remains a runtime-validated process, not a game-validated one. What it now proves, that
milestone 2l did not, is that the mission's own order function issues real `attackmove` and
`moveto` commands through the game's own path over the mission's own ships, that the recovered
weapon-director stage ladder gives every ship with no orders a `stop` and the player's ship a
`cruise`, and that six fixed-step passes, three load steps and four scene-contents steps run
their own reconstructions over this mission's data. It proves nothing about what those orders
would do: three named blocks still stand between a scripted order and a moving ship, and no ship
moved under anything but the player's own `--order`.

### Follow-up packets

1. **`entity_command_arms`**, 00816e30's 00816f7c..00817330. It is now the first blocker: seven
   real commands a run issues stop there. Milestone 2l listed it for `--order settarget`; the
   mission's own script needs the `moveto` and `attackmove` arms of the same block.
2. **`unit_autopilot_pair`**, `docs/CRUISE_COMMAND.md`'s own first follow-up: `unit+61h`,
   `unit+0fc4h`, `unit+0fdch` and the writer that carries the AI controller block at `[state]+8`
   into them. Unchanged from milestone 2l and still the last step.
3. **`ship_ai_state_machine`**, 00d21598 and 009f3dd0, so 009e1170 runs where the game runs it
   and the `moveto`, `follow` and `stop` states exist at all. Unchanged from milestone 2l.
4. **`008162b0`**, the command-availability predicate at vtable 16Ch, which is the whole of
   `JoinFormation`: fourteen calls a run makes stop at it.
5. **The three vtable leaves the small bindings need**: 128h (skill), 148h (role) and the
   `IsKindOf(6)` class this `RepairEnable` answers, plus the delivery of the type-76h, 9Fh and
   4Ch session messages.
6. **`gameplay_effect_acquisition` as the executable can own it**: a `GameplayEffectManagerContext`,
   a name-index host and a scalar-component dispatcher would make `00871ba0` concrete here and
   with it the record effect preload's own acquire.
7. **`scene_property_bag_holder`**: 00922e20 itself, so a created entity's +C0h carries the 0Ch
   holder rather than a record.
8. **`00424d00`**, the avoid-zone table rebuild, which needs `construct_world` 004de610's list at
   `[[00e188a8]+19CCh]+370h`.
9. **`construct_world` 004de610**, unchanged from milestones 2h, 2i, 2j, 2k and 2l.

## Next milestones

1. **Title page.** Phase 6 locale tables 00aa09d0 over the locale file phase 2 can now read
   (`lockit/<language>.lng`, selected by the settings language), the font registry, phase 7 GUI
   startup 00aa06d0, and the GUI layer draw for the logo sequence that game state 1 selects.
   The frame then draws a layer instead of a cleared buffer. Phase 2 and phase 5 no longer
   block this.
2. **Main menu.** Phase 9 game entry 00740840 into game state 2, the front-end screen sets, and
   the input edge test at 004c43c0 through the input polling the input packet connected, so the
   menu responds to a keypress.
3. **Renderer resources, phase 4 00b14a10.** It now has mounted providers to load from. It is
   also what would remove the milestone's own `EnumAdapterModes` substitute: the supported
   resolution table 00f8895c and the antialias level table 00f88968 both come from the renderer
   vector this phase fills.
4. **Loose ends inside phase 2.** The `.mpak` factory 00736b60 and the PAK registry 00736c30 /
   00bd9230 / 00bb40b0; phase 6's resource manager 004c1400 and the two parsers 00736dd0 and
   00736ea0 through 00b80a50; and a cached-load slot on the provider manager so the
   `cachedload` flag 00bd9f90 carries reaches a provider.
5. **Not this packet's to fix.** `apply_options_token` in `src/app_bootstrap.cpp` compares
   option token names case-sensitively where the original compares them case-insensitively;
   see the phase-5 section above.

## Correction from docs/SETTINGS_STARTUP_OWNER.md

Settings startup now retains the initialized complete projected settings object,
loads its language catalog through the mounted VFS, queries the recovered D3D9
resolution/shader/AA operations and writes missing options at the native call
site. The old apply_detected_defaults no-op and token-recapitalization workaround
are removed. --settings-personal-root supplies an isolated personal directory
for both startup language lookup and settings read/write, resolved before the
game-root working-directory change. A640x480 windowed run presented60 frames,
resolved3/3 VFS probes and exited0; the original executable and live options
kept their hashes and timestamps. The milestone remains a clear/present process;
input, GUI and gameplay owners still require integration. Shared native renderer
API ownership and the full capability record remain outside this settings batch.

### Follow-up: persistent input, renderer and locale owners

See [RUNTIME_STARTUP_OWNERS.md](RUNTIME_STARTUP_OWNERS.md). The executable now
retains the input-script Lua state and parsed settings, shares one Direct3D API
and parameter region between settings queries and device creation, and loads
locale tables with the native setter/register/reload order. The old log label
`gui_startup` at00aa06d0 referred to locale reload; fonts/GUI resource loading
at0073bae0 remains unimplemented. Startup process checks do not validate gameplay.
