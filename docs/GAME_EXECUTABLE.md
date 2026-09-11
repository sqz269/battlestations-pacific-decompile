# bsp_game.exe, milestones 1 through 2f

Milestone 2f is the current state of the executable, and its section corrects the earlier
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
| `lua_reset_state` | 004e02d0 | 005e2f00 | the `LobbySettings` table, values excepted |
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
rows and reports **51 failures, all of one kind**: every call site of the in-mission frame
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
