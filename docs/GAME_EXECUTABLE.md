# bsp_game.exe, milestones 1, 2a and 2b

Milestone 2b is the current state of the executable, and its section near the end of this
file corrects the two earlier ones. Milestone 1 is the spine it was all built on.

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
