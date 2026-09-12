# bsp_game.exe, milestones 1 through 2n

Milestone 2n is the current state of the executable, and its section corrects the earlier
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
commanded-speed store 00890e6f; the seven fan-out rows 00875e3a / 0098bdb0, 00875e3f / 00874de0,
00875e44 / 00926700, 00875e55 / 00888230, 00875e64 / 00929460, 00875ec9 / 009273a0 and
00875eda / 00903610; 0046d51a / 0095c640 (the vehicle-class preload); the
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

Seven fan-out rows, three mission-load rows and five scene-contents steps had reconstructions on
main and were still records. All fifteen now run. Two of the fifteen arrived in the merge of main
this packet took before finishing: packet `cc2_fixed_step_callbacks` made fan-out row 5 (00874de0)
wireable and packet `cc2_scene_traffic_groups` made the vehicle-class preload (0095c640) wireable,
after this packet had already recorded both as unavailable.

| Row | Native | What it did here |
| --- | --- | --- |
| `refresh_moved_spatial_nodes` | 0098bdb0 | the walk runs over an empty root list: nothing registers a spatial node, 0098a310's callers are the scene graph's |
| `run_step_callbacks` | 00874de0 | the list at 00f87680 is empty: its only registrant 00875a80 belongs to the engine's own subsystems |
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
| `register_vehicle_class_preload` | 0095c640 | its three reads go to the live `VehicleClass` global the recovered global-script step loaded, once per registration body |

Two that the packet brief listed as ready could not be wired, and the report says why for each:
`00871ba0` (the effect acquire needs a `GameplayEffectAcquisitionContext` nothing here builds) and
`00922e20` (the 0Ch property-bag holder is a host method of `SceneEntityCreateHost`, not a
routine, and 00922e20 itself has no reconstruction). `004c3840` has a reconstruction and stays an
arm the load never takes, because `004e044d` gates it on `game+1FE4h == 1`.

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
"<install>"`, exit 0: **434 concrete, 402 unimplemented**. The same command on the tree this
packet branched from reports **375 and 411**. Two of the 59 the packet added arrived with the
merge of main it took before finishing, not with its own reading; the corrections say which.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2m.json` (`script_order_steps`, `script_order_indirect`,
`script_order_field_reads`, `director_step_steps`, `fixed_step_rows`, `mission_load_steps`,
`scene_contents_steps`, `still_unimplemented`). The counts by group:

| Group | Steps with a call site | Concrete | Records |
| --- | --- | --- | --- |
| The eight binding bodies | 27 | 19 | 8 |
| The director's stage ladder 00836920 | 13 | 11 | 2 |
| The seven fan-out rows | 7 | 7 | 0 |
| The three mission-load rows | 10 | 9 | 1 |
| The five scene-contents steps | 22 | 21 | 1 |

Twenty-six records the run reached before are gone, and sixteen new ones appeared, each a native
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
2. **This packet's brief listed `0095c640` among the hosts with a reconstruction on main. It had
   none when the packet opened, and it has one now.** `python tools/bsp.py lookup 0095c640`
   answered `FUN_0095c640` with no ledger name and the address was leased to
   `agent/cc2-scene-traffic-groups`; that packet landed while this one was open, and the merge of
   main this packet took before finishing brought `include/bsp/scene_traffic_groups.hpp`, so the
   step is wired after all. The same merge brought `cc2_fixed_step_callbacks`, which made fan-out
   row 5 (00874de0) wireable, and renamed `reset_objective_list` to `reset_avoid_zone_state`
   rather than to the `rebuild_avoid_zone_table` this packet had been told to expect.
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
6. **`docs/MISSION_LOAD_HOSTS.md`'s renamed host methods.** The executable's load walk accepts the
   old spellings (`reset_network_slots`, `reset_objective_list`, `rebuild_scripted_name_list`),
   the ones this packet was told to expect (`erase_native_string_set`,
   `rebuild_avoid_zone_table`, `record_script_function_baseline`) and the one main actually
   carries (`reset_avoid_zone_state`), because that rename landed while this packet was open.
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
`python tools/verify_report_calls.py reports/game_executable_milestone_2m.json` checks 81 call
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
summary fixed step subsystems spatial=390/0 callbacks=390/0 deferred_events=780/0 lua_calls=390/0
        think=390/0 pending_queues=390 expiry=390/12480 released=0
summary mission director steps=12480 idle_reissues=20 stop=19 cruise=1 follow=0 script_issues=7
        blocked_at_00816f7c=7 commanded_speeds=0
summary mission scene contents mode=8 entities=34 generated=34 rejected=0 created=32
        registration_bodies=32 party_class_marks=32 property_groups=22 enum_tables=33
summary mission world units=32 walked=12480 updated=12480 motion_ticks=12480 simulated=19.50 s
        controlled=DeRuyter moved=0.00 total_path=0.00
host methods 434 concrete, 402 unimplemented
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
9. **`load_prop_library` over `008F67B0`.** Packet `cc2_scene_traffic_groups` reconstructed the
   property-group and enum library loader in the same merge that brought the vehicle-class
   preload, so milestone 2h's own reader, the one stand-in that decides the generation gate for
   every entity of every `.scn`, can now be deleted. That is milestone 2h's follow-up 2, and it
   is the largest stand-in this executable still carries.
10. **`construct_world` 004de610**, unchanged from milestones 2h, 2i, 2j, 2k and 2l.

## Milestone 2n: the ship AI controller, and the two hops that are left

Addresses: 009f50e0 with its three gates 009f50e4 / 009f50f2 / 009f50fc and its sixteen steps
009f5106, 009f5156, 009f516c, 009f5186, 009f519e, 009f51ae, 009f51b7, 009f51c6, 009f51d5,
009f51e4, 009f51f3, 009f51fa, 009f5209, 009f5227, 009f5239 and 009f5248; 009f3dd0 with 009f3de2
/ 009f3de6 / 009f3df3 / 009f3e12 / 009f3e1b and 009f3d00 with 009f3d7d / 00779aa0; the eight
state command getters 009dac20, 009dac80, 009dadb0, 009dae20, 009daf50, 009db050, 009e8540 and
009db350 and the two interval getters 009dac30 and 009daa90; 009ed6b0's direct-control arm with
009ed8d1 / 0092d730, 009ed8ec, 009ed902, 009ed95d and 009ed9c1 / 00811940; 009f4d10 with
009f4d2f, 009f4d48 / 00811960 and the three tails 009f0100 / 009ef350 / 009ef910; 00825f2c..
00825f7c with 00811d10; 009f5da0 with 009f5610, 009f5d30, 009f5b70, 009f52f0, 009f65e0,
008053c0, 0071df70, 0071d980 and 00835860; and 00816ea6..0081732e with 00521ea0, 007ac9d0,
0077c8d0 and 00470b80. Packet `cc_exe_2n`, owner `agent/cc-exe-2n`. Sources:
`src/game_hosts_ship_ai.cpp`, `include/bsp/game_hosts_ship_ai.hpp`, plus edits to
`src/game_hosts_commands.cpp`, `src/game_hosts_units.cpp`, `src/game_hosts_mission_frame.cpp`,
`src/game_hosts_mission.cpp`, `src/game_hosts.cpp`, `src/game_main.cpp` and their headers.
Report: `reports/game_executable_milestone_2n.json`. Ghidra was read-only for this packet.

Milestone 2m issued the mission's own `attackmove` and `moveto` commands through the game's own
path and watched all seven stop at `00816e30`'s unprojected arm, and every one of the 32 ships
held station. This milestone runs the ship AI controller over those ships. Four of the nine AI
states are reached, six Japanese destroyers hold an `attackmove` their own mission script issued,
every controller publishes an order into the unit's own order slot every step, and the ships
still do not move, for two named reasons instead of three.

### 1. The controller, 009F50E0, and what installs a state

`GameShipAiHost` owns one controller per created instance and runs
`bsp::ship_ai_controller_step_009f50e0` over `bsp::ShipAiControllerHost` once per unit per fixed
simulation step. Where it runs is the executable's decision and is recorded as one: 009F50E0 has
no caller in the call graph, exactly as 00825F20 and 00836920 do not.

Its three gates all open. `[ai+0B00h]` is the created instance; `unit+5Dh` is the byte milestone
2i holds clear for a live ship; and `unit+61h` is the byte `docs/UNIT_AUTOPILOT_PAIR.md` scanned
`.text` for and found no writer of outside the constructor. All **15680** controller calls of the
validation run passed all three, and none was gated.

**Which state the controller installs is now read rather than guessed.** `009F3DD0` asks the
director for its current command through `0071BE40`, substitutes the `cruise` singleton
`00E08F70` when `unit+184h` is set, and hands the answer to `009F3D00`. That routine had not been
read; this packet read it whole, body 009F3D00-009F3DCE, and it is a plain dispatch:

| test | command | state object (ai offset) | state |
| --- | --- | --- | --- |
| `009F3D04` | `00E08F68` | `+0C5Ch` | `movetopos` |
| `009F3D1A` | `00E08FA0` | `+0C38h` | `land` |
| `009F3D29` | `00E08F88` | `+0BD8h` | `stop` |
| `009F3D38` | `00E08F80` | `+0C64h` | `moveonpath` |
| `009F3D47` | `00E08F60` | `+0BE4h` | `follow` |
| `009F3D56` / `009F3D5D` | `00E08F10` `artillery` or `00E08F78` `attackmove` | `+2254h`, `+217Ch` or `+0C70h` | `kamikaze_attack`, `sub_attack` or `attackmove`, chosen by `[ai+0B0Ch]` and `00779AA0` |
| `009F3D64` | `00E08F70` | `+0BC8h` | `cruise` |
| otherwise | - | `+0BD8h` | `stop`, the default at `009F3DA0` |

The ai offsets are the brain offsets of `docs/SHIP_AI_STATES.md` plus 58h, which is what settles
the table. `009F3DAE` then calls the outgoing state's `vtable[8]`, stores the new one at
`ai+2264h` and calls its `vtable[4]`; both are records. `[ai+0B0Ch]` is `brain+0AB4h` and has no
producer here, so the attack arm always takes the plain `attackmove` object and `00779AA0` is
never reached.

The eight command getters behind `vtable[24h]` were read the same way, each a two-instruction
`MOV EAX,<singleton>; RET`, and so were the two interval getters: `009DAC30` is
`FLD [00CE3958]` = **2.0** for `cruise` and `009DAA90` is `FLD [00CE3850]` = **5.0** for every
other leaf, both multiplied by the 0.05f at 00D0DE84. `kamikaze_attack`'s getter `009DB350`
returns the same `00E08F78` as `attackmove`'s `009E8540`, which is why `009F3D00` separates those
two on `[ai+0B0Ch]` and not on the command.

### 2. Only one of the nine state steps has a body

This is the correction that shapes the whole milestone. `docs/SHIP_AI_STATES.md`'s coverage table
reads "`009E14C0`, `009E1610`, `009E1950`, `009E59C0`: not read. They are named here only as
vtable slots", and the `attackmove` step `009E8820` and the `kamikaze_attack` step `009E2020` were
not read either. **Eight of the nine leaves are records with their own addresses.** Only the
`cruise` step `009E1170` has a reconstruction, and it runs.

That step also moves. Milestone 2l ran `009E1170` once per unit per fixed step; `009F5186` calls
it only on a re-plan tick, so a `cruise` ship steps 245 times in 490 fixed steps and a `stop` or
`attackmove` ship 98 times. The three desired-value setters `009DBF90`, `009DFFB0` and `009E0040`
are no longer records either: they run their reconstructions on the AI control block
`blk = brain+8h`, so the clamped desired throttle, rudder and heading are the values `009ED6B0`
then reads.

`009ED6B0`'s direct-control arm runs whole for every unit every step. One of its inputs stops
being a mystery: the `FDIV [ECX+508h]` at `009ED8EC` takes `ECX = [unit+538h]`, and that field is
`Retardation` (`docs/SHIP_CLASS_FIELDS.md`, loader 00831882..00831998), which this process reads
out of the installed `VehicleClass` row. So `blk+32Ch` in the stopped-direction branch is
`v*v/(2a)` plus `unit+9C8h`: a **stopping distance**, which narrows uncertainty 1 of
`docs/SHIP_AI_STATES.md` for that branch. The moving branch still reads `blk+3E0h`, which has no
producer.

### 3. The order slot fills, and the ring does not

`009F4D10` publishes every step: `00811960` limits the heading target onto the slot's `+44h`, and
`blk+32Ch` and `blk+330h` go to `+40h` and `+48h` with the valid flag set twice. The motion's own
head then promotes it: `00825F2C` finds the flag set, clears it, flips `[unit+0B40h]` and copies
the slot across through `00811D10`. **15680 publishes and 15680 promotions**, one per unit per
step, with no step failing the flag test.

What is missing is the reader. `docs/UNIT_AUTOPILOT_PAIR.md` scanned `00825F20`'s body for
`slot+40h`, `+44h` and `+48h` and found only the promotion block, so whatever turns a heading
target and two distances into the order ring's `+148h` / `+14Ch` has not been located. That is
packet `cc_ai_order_hop`, worker `agent/cc-ai-order-hop`, which was not on main at the end of this
packet's turn, and it is the record `ShipAiOrder::slot_to_order_ring [00825f7c]`.

The same doc also corrects the brief this packet was given, and milestone 2l's follow-up 2 with
it: there is no hop from the AI controller block to `unit+0FC4h` / `unit+0FDCh`, because
`unit+61h` gates the AI off at `009F50FC` and selects that pair at `008266C1`. The two are
mutually exclusive regimes, and the pair is a manual override.

### 4. The mission's own orders reach a director, and put six ships into `attackmove`

Packet `cc_ship_ai_arms` landed on main during this packet's turn and reconstructs
`00816E30`'s arm cascade `00816EA6..0081732E`, which is milestone 2m's follow-up 1. The executable
runs it through `bsp::entity_command_arm_cascade_00816ea6`, so milestone 2m's
`blocked_at_00816f7c=7` becomes **`blocked_at_00816f7c=0` and `reached_director=7`**: the six
`NavigatorAttackMove` calls and the one `NavigatorMoveToRange` call of `usn_2_java`'s own order
function now push a command slot.

The states the validation run ends with:

| state | ships | why |
| --- | --- | --- |
| `cruise` | 13 | the twelve scene-authored `Cruise` tokens, plus the controlled DeRuyter, which `009F3DF3` forces into `cruise` whatever its director holds |
| `stop` | 13 | the idle tail `00836DC9` of milestone 2m, for a ship with no orders |
| `attackmove` | 6 | `Haguro`, `Jintsu`, `Yudachi`, `Samidare`, `Murasame` and `Harusame`, from their own script's `NavigatorAttackMove` |
| `movetopos` | 0 here, 1 with `--order` | the Dutch `moveto` goes to the controlled ship, and the player arm overrides it |

The `movetopos` row is why this milestone adds `--order-unit <name>`: `--order` issued a command
to the controlled unit only, and a command issued there can never change an AI state.
`--order moveto:Java --order-unit Kortenaer` puts `Kortenaer` into `movetopos` for 190 steps with
39 re-plans, which is the fourth state a run reaches.

**No ship moves.** Every unit's distance is `0.00` over 24.5 simulated seconds, and there are now
two reasons rather than milestone 2m's three: the `attackmove` and `movetopos` state steps have no
body to set a desired value with, and the promoted slot has no reader.

### 5. The automatic target selector runs and chooses

`bsp::auto_target_tick_009f5da0` runs beside the controller with the same step delta, so the
selector's own countdown turns 15680 ticks into **812 thinks** over 24.5 s, which is the
once-a-second interval `009F6A20` seeds. `009F5610`'s enable gate opens on the `allowMove` byte
the director constructor `008363E0` sets for a ship, the priority list `009F65E0` builds comes
from the owner's own `IsKindOf` answers over its real class id, and the score is
`009F5B70`'s tier-times-10000 minus distance.

**22 of the 32 ships chose a target**, by name and by the recovered rule: `Haguro`, `Jintsu`,
`Yudachi`, `Samidare`, `Murasame` and `Harusame` all pick `Kortenaer`; `Exeter` picks `Kawakaze`;
`Jupiter` and `Witte` pick `Naka`. The candidate list is the executable's stand-in and is recorded
as one: `009F5D30` walks the recon slot `008053C0` returns for the owner's party and the
intrusive chain at `slot+0DE8h`, which nothing here fills, so the scan is handed the created
instances of the opposing party. That is the same substitution milestone 2i makes for walk 0 of
`004C3CB0`.

**Nothing reaches `00835860`.** The tick's last gate is `0071DF70`, which this packet read whole
(body 0071DF70-0071DFCF): the float at `[director+40h]` must be greater than the 0.0f at
`00D7A218`, and then none of the ten command slots at `director+54h` may answer 1 or 2 from its
`vtable[0Ch]`. `director+40h` has no writer in this process, no recovered producer anywhere and is
not written by the director constructor `008366D0`, so the predicate is a record, its neutral
answer stops every think, and neither `BSP_WeaponDirector_SetFireTarget` nor the `attackmove`
issue `0071D980` is reached. The gun-side ticks of `docs/BOT_FIRE_TARGET.md` and
`BSP_TurningGun_SetTargetAngles` `0085ABA0` are a further boundary behind that one, and this
process owns no gun.

### What it looks like on screen

Unchanged from milestone 2m, and for the same reason: what a mission would draw goes through
`004CA440` and `004CA1F0`, both records, and no ship moves. The capture at in-mission frame 480 is
the captain HUD over the cleared dark blue buffer: the repair wheel with its four quadrant icons
in the middle, the engine telegraph with its `STOP` / `1/4` / `HALF` / `3/4` / `FULL` dial and the
rudder indicator in the bottom right, the damage bar under them, the `Artillery`, `Repair` and
`Fighter Ace` text runs, the one unit marker with its `Unit Name` label and `1254` distance, and
the minimap cluster's authored `error.tga` island map in the top right. The sprite bridge holds
the same 192 quads, 57 of them glyphs. The capture is at the validation machine's 2560x1440, where
milestone 2k's wide-screen caveat still puts the minimap cluster half outside the window; it is
written to the ignored `local/run.png` and is not committed.

### Host methods

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
--mission-frame-seconds 0.05 --mission-complete-frame 490 --trajectory-csv local/trajectory.csv
--screenshot local/run.png --screenshot-mission-frame 480 --log local/game_run.log --game-root
"<install>"`, exit 0: **464 concrete, 428 unimplemented**. Milestone 2m's published pair for the
same shape of command is 434 and 402. Part of that move is not this packet's reading: it merged
main twice, and the second merge brought `cc_ship_ai_arms`, which this packet then wired.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2n.json` (`controller_steps`, `state_selection_steps`,
`cruise_state_steps`, `direct_control_steps`, `publish_steps`, `promotion_steps`,
`auto_target_steps`, `entity_command_arm_steps`, `still_unimplemented`). The counts by group:

| Group | Steps with a call site | Concrete | Records |
| --- | --- | --- | --- |
| The controller 009f50e0 | 20 | 8 | 12 |
| The state selection 009f3dd0 and 009f3d00 | 8 | 3 | 5 |
| The cruise state and its three setters | 7 | 4 | 3 |
| The direct-control arm 009ed6b0 | 8 | 4 | 4 |
| The publish 009f4d10 and the promotion 00825f2c | 10 | 7 | 3 |
| The automatic target think 009f5da0 | 19 | 8 | 11 |
| The entity command arms 00816ea6 | 7 | 2 | 5 |

### Corrections

1. **This packet's brief said the states' steps for `moveto`, `attackmove`, `follow` and `stop`
   are reconstructed. They are not.** Section 2: `docs/SHIP_AI_STATES.md`'s own coverage table
   says those four bodies were not read, and neither were the `attackmove` and `kamikaze_attack`
   steps. Eight of the nine leaves are records.
2. **This packet's brief, and `docs/CRUISE_COMMAND.md`'s follow-up `unit_autopilot_pair`, put the
   last hop at `[state]+8` into `unit+0FC4h` / `unit+0FDCh`. There is no such hop.**
   `docs/UNIT_AUTOPILOT_PAIR.md` settles it: `unit+61h` gates the AI controller off at `009F50FC`
   and selects the pair at `008266C1`, so the two regimes are mutually exclusive. The AI's order
   goes into the unit's own 84-byte slot, and the missing piece is that slot's reader.
3. **Milestone 2l's "009e1170's AI arm runs once per unit per fixed simulation step" is
   superseded**, as `docs/SHIP_AI_STATES.md`'s own correction says. It is a vtable `+0Ch` and
   `009F5186` calls it on a re-plan tick, every 2.0 * 0.05 s for `cruise`.
4. **Milestone 2m's `blocked_at_00816f7c=7` is superseded.** Packet `cc_ship_ai_arms` reconstructs
   the cascade and the executable runs it; the seven scripted commands reach the director.
   Milestone 2m's follow-up 1 is closed, and the three blocks between the mission's script and a
   moving ship are two.
5. **`docs/SHIP_AI_STATES.md`'s uncertainty 1 is narrowed for one branch.** The divisor at
   `[unit+538h]+508h` is `Retardation`, so `009ED8EC..009ED910` builds a stopping distance rather
   than an unnamed one. The branch that reads `blk+3E0h` is unchanged.
6. **`009F3D00` and `0071DF70` had no reading and now have one**, both complete; so do the eight
   `vtable[24h]` command getters, the two `vtable[28h]` interval getters and `00465080`. The
   report's `read_for_this_packet` list carries each with its body range or its two instructions.
7. **A single host-method name cannot carry both dispositions.** `GameHostLog` keeps the first
   disposition it is given for a name, and this packet's own first pass recorded
   `ShipAiState::cruise_step` for the player-controlled arm before the concrete path ran, which
   marked all 3185 calls unimplemented. The player arm is recorded under its own address
   `009E11E8`, which is where milestone 2l already put it.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| - | - | none |

Every address this packet touched has a Ghidra function: `009F50E0` and `009E5770`, the two the
`cc_ship_ai_states` packet listed as undefined, were defined by the integrator in commit
5ee5a720. Two names were added, both for routines this packet read whole and neither of which had
one: `0071DF70` `BSP_WeaponDirector_AcceptsNewTarget` and `00465080`
`BSP_CommandTarget_FromEntity`. Run-time evidence was appended to 009f50e0, 009f3dd0, 009f4d10,
009f5da0 and 009f3d00.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest case
`reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2n.json` checks 49 call
rows and reports 0 failures; eight rows are reported as indirect because the native call goes
through a vtable slot or a register.

```
ship AI controllers: 32 built, one per created instance. 009f50e0 has no Ghidra function and no
        caller, and 009f5da0 is reached only through the derived vtable slot at 00d21b4c, so this
        process runs both once per unit per fixed simulation step, before the motion pass whose
        head at 00825f2c consumes the order slot 009f4d10 published
cruise state step 009e1170 runs on a re-plan tick of the ship AI controller 009f50e0, not on
        every step
the promoted AI order slot reaches no order ring: 009f4d10 publishes the heading target and the
        two distances into unit+0aech - 84*[unit+0b40h], 00825f2c flips the index and 00811d10
        copies the slot across, and no reader of slot+40h / +44h / +48h was found. The last hop
        is packet cc_ai_order_hop, worker agent/cc-ai-order-hop
automatic target selection stops at 0071df70: its first test is the float at director+40h against
        the 0.0f at 00d7a218, and that field has no writer in this process and no recovered
        producer anywhere, so the chosen candidate never reaches 00835860
        BSP_WeaponDirector_SetFireTarget and the attackmove issue at 0071d980 is not reached
  Haguro               attackmove      490       98      490      490    3.1416      26      26 Kortenaer   0071df70 director+40h
  Exeter               cruise          490      245      490      490    0.0000      25      25 Kawakaze    0071df70 director+40h
summary mission script bindings calls=52 attackmove=6 moveto=1 issued=7 reached_director=7
summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5047
        state_steps{concrete=2940 records=2107} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=13 attackmove=6 movetopos=0 other=0
summary mission auto target thinks=812 scans=787 chose=22 fire_target_sets=0 attackmove_issues=0
        blocked_at=0071df70
summary mission director steps=15680 idle_reissues=14 stop=13 cruise=1 follow=0 script_issues=7
        blocked_at_00816f7c=0 commanded_speeds=0
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680 simulated=24.50 s
        controlled=DeRuyter moved=0.00 total_path=0.00
host methods 464 concrete, 428 unimplemented
```

and, from `--order moveto:Java --order-unit Kortenaer --order-frame 5 --mission-frames 200`:

```
player command issued to "Kortenaer": token="moveto" resolved="moveto" outcome=issued current=1
  Kortenaer            movetopos       190       39      190      190    0.0000      10      10 Yudachi     0071df70 director+40h
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
```

Every earlier switch was rechecked on the same binary. A 120 frame run with
`--press-start-frame 30` and no `--menu-select` exits 0 and reports 155 concrete and 79
unimplemented, a 40 frame title-only run reports 130 and 48, `--vfs-probe fonts/fonts.lua` exits 0
and `--vfs-probe does/not/exist.lua` exits 3: all four match milestone 2m exactly. A
`--mission-frames 60` run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`, and the 500 frame run above still leaves state 0Dh through
004d7970 and exits on the front-end request rather than on the frame count. The acceptance form
`--order-frame 1 --order throttle=1,rudder=1` reproduces milestone 2j's published numbers exactly:
`heading -41.253, fwd 16.430, yaw -0.06109` at t = 14 s, so the AI controller changes no
trajectory: what it publishes reaches no ring.

This remains a runtime-validated process, not a game-validated one. What it now proves, that
milestone 2m did not, is that the recovered ship AI controller runs over the mission's own ships
with its three gates open, that the recovered dispatch from a command object to a state puts six
Japanese destroyers into the `attackmove` their own script asked for, that the recovered
direct-control arm and publish fill the unit's own AI order slot every step and the motion head
promotes it, and that the recovered automatic target selector picks a target for 22 of the 32
ships by the game's own priority and distance rule. It proves nothing about what any of that would
do: eight of the nine state steps have no body, the promoted slot has no reader, and the fire
target stops at one unwritten field.

### Follow-up packets

1. **`unit_ai_order_slot_reader`**, packet `cc_ai_order_hop`: `unit+0A98h`, `unit+0AECh`, the
   three fields `+40h` / `+44h` / `+48h`, and `00825F7C..00826D6B`. It is the last hop between a
   published AI order and a moving ship, and every other piece of that chain now runs here.
2. **`ship_ai_state_steps`**: `009E14C0` `stop`, `009E1610` `follow`, `009E1950` `land`,
   `009E5770` `movetopos` beyond its arrival test, `009E59C0` `moveonpath`, `009E8820`
   `attackmove` and its four sub-states, and `009E2020` `kamikaze_attack`. Six ships of this
   mission sit in a state whose step has no body.
3. **`director_new_target_gate`**: `director+40h`, the float `0071DF70` tests first. It is the one
   field between a chosen target and `00835860`, and the director constructor `008366D0` does not
   write it.
4. **`ship_ai_navigation_arm`**, `009ED6B0` `009EDA26..009EF228` with `blk+3D0h` and `blk+3E0h`:
   the bearing-to-waypoint rule, and the producer of the distance the moving branch publishes.
5. **`ship_ai_goal_vector`**: `brain+0B2Ch`, `brain+0B34h`, `brain+0B38h`, `009DE050` and
   `009F4DA0`. Without it `movetopos` has nothing to steer toward.
6. **The recon party list**, `008053C0`'s slot and the intrusive chain at `slot+0DE8h`, so the
   automatic target scan reads the list the game gives it instead of this milestone's stand-in.
7. **The gun side of `docs/BOT_FIRE_TARGET.md`**: `008FFA20`, `008FFF20`, `00902920`, `009030C0`,
   `006DF520` and `0085ABA0`, which is what turns a fire target into a shot.
8. **`construct_world` 004de610**, unchanged from milestones 2h through 2m.

## Milestone 2o: the AI's throttle and steering reach the order ring, and a ship the AI drives moves

Addresses: 009f5248 with 009f4da0 and its tail call 009f50c6 into 009f3f80; inside 009f3f80 the
early out 009f3feb / 009f3ff2 into the epilogue 009f4d00, the write-slot read 009f3ff8..009f402e
with 009f400d and 009f4025, 009f4034 / 0092d730, the heading getter 009f4072..009f407b, the astern
flip 009f4081 / 009f409e / 00438aa0 / 00d7a264, the heading error 009f40bb / 00438b10, the
unprojected spans 009f40ca..009f44e3 and 009f4502..009f4b98, the rudder-law gate 009f44e4 with
009f44f7 / 009da250 / 009f44fc and the second store 009f4694 / 009f46a0, and the hop
009f4b99..009f4d04 with 009f4bb9 / 00d7a270, 009f4bc6, 009f4bd4 / 0092d730, 009f4c12 / 00ce3d78,
009f4ce8 / 0080e190 and 009f4cfb / 0080e170; 009da262..009da280 with 00828f20 and 009da2d1 /
009da2d7; the state steps 009e14c0, 009e5770 and 009e8820 with the selector 009e86f0, the goal
setter 009de050, the path reset 009da4e0 and the heading hold 009e00a0, and their call sites
009e14d7, 009e14ec, 009e14f3 / 0071c4f0, 009e1518, 009e1534, 009e153c / 009e0040, 009e579a,
009e57aa, 009e57d0, 009e57ed / 0071bff0, 009e57f4 / 007adc60, 009e580f, 009e5821, 009e5831,
009e5847 / 00521ea0, 009e58b9, 009e8828, 009e883f, 009e8852, 009e88d8 and 009e88f0; and
00826121 / 00813020, which this executable has run since milestone 2i. Packet `cc_exe_2o`, owner
`agent/cc-exe-2o`. Sources: `src/game_hosts_ship_ai.cpp`, `include/bsp/game_hosts_ship_ai.hpp`,
plus edits to `src/game_hosts_units.cpp`, `src/game_hosts.cpp`, `src/game_hosts_mission.cpp`,
`src/game_hosts_mission_frame.cpp`, `src/game_main.cpp`, their headers and
`tools/verify_report_calls.py`. Report: `reports/game_executable_milestone_2o.json`. Ghidra was
read-only for this packet; two existing names had run-time evidence appended.

Milestone 2n ran the ship AI controller over 32 ships, put six Japanese destroyers into the
`attackmove` their own script asked for, and left every one of them motionless with the last
chain slot unwired and a published order slot nobody read. This milestone wires that slot, and
then runs the real state steps over it: packets `ship_ai_state_steps` and
`ship_ai_class_field_0524` landed on main during this packet's turn and were merged in before
validation. The chain from a state's decision to a hull that moves is now closed end to end
through the game's own routines, four of the nine state leaves run their own bodies, the steering
law runs 6370 times on the ships that ask for a heading, and one ship, driven by a labelled
diagnostic stand-in for the desired throttle no navigation state produces, sails 417.54 units and
turns onto the same profile the ship-motion probe produces for its class.

### 1. Chain slot 16, and why its record costs nothing

`009F5248` calls `009F4DA0`, whose body is a throttle ceiling on `brain+34Ch` / `+350h` that no
packet has reconstructed. That body stays a record. It gates nothing: this packet read the
routine's tail and the exit is a `CALL`, not a `JMP`, and both arms of the `009F4DAF`
`brain+0B38h` test reach it.

```
009f50ba POP EDI ; 009f50bb FLD [ESP+1Ch] ; 009f50bf PUSH ECX
009f50c0 LEA ECX,[ESI+8] ; 009f50c3 FSTP [ESP] ; 009f50c6 CALL 009f3f80
009f50cc POP EBP ; 009f50d0 RET 4
```

`ECX = ESI+8` is `brain+8h`, which is `blk`. So `009F3F80` runs on every controller step whatever
the ceiling did, **15680 of 15680** times in the validation run.

### 2. What the executable projects of 009F3F80, and what it records

`009F3F80` is 009F3F80-009F4D06 and the packet that settled it projected only its head and its
tail. This milestone runs three parts of it and records the rest, which is the milestone's own
boundary and is stated here rather than implied:

| span | disposition |
| --- | --- |
| `009F3FEB..009F3FF2` | the early out on `blk+3F5h`, projected |
| `009F3FF8..009F402E` | the ring slot under the write cursor, projected |
| `009F4034..009F40C6` | the speed, the heading, the astern flip and the heading error, projected |
| `009F40CA..009F44E3` | the ten writers of `blk+1D0h`, one record |
| `009F44E4..009F44FC` | the mode gate and the rudder law's store, projected |
| `009F4502..009F4B98` | the obstacle and reverse-manoeuvre arms, the same record |
| `009F4B99..009F4D04` | the hop, projected |

**Two branch facts settle the wiring, and neither was in `docs/SHIP_AI_THROTTLE_TO_RING.md`.**
First, `009F3FF2 JNZ 009F4D00` is the routine's only early out and `009F4D00` is the epilogue, so
a set `blk+3F5h` skips the ring read, the body and the hop alike. It is the same byte `009ED6B0`
returns on, nothing in this process writes it, and **0 of 15680** steps were gated. Second, every
other path converges on `009F4B99`: the three callee-saved pops at `009F4BBF..009F4BC5` are
unconditional and `009F4D04` is the single `RET`. That is what makes one host method for the two
unprojected spans honest rather than convenient: they can change `blk+1D0h` and `blk+1D4h` before
the hop reads them, but they cannot stop the hop from running.

### 3. The rudder law has a gate, and which states open it

`docs/SHIP_AI_THROTTLE_TO_RING.md` says `009DA250` is the only writer of `blk+1D4h` inside
`009F3F80` and that both stores take its result. True, and incomplete: **neither store is
unconditional.**

```
009f44e4 CMP dword ptr [ESI+1C4h],0
009f44eb JZ  009f4502            ; skip the law when the steering mode is Rudder
009f44ed FLD [ESP+20h] ; 009f44f7 CALL 009da250 ; 009f44fc FSTP [ESI+1D4h]
```

The second store at `009F46A0` sits inside the obstacle-table arm, which this packet records. So
the law runs only for a state that leaves `blk+1C4h` at something other than `Rudder`, and which
states those are is now measured rather than guessed: **6370 of 15680 steps**, which is thirteen
ships every step. The twelve `stop` ships get `Heading` from `009E0040`, and `Kortenaer` gets
`Navigate` from `009DE050`. The thirteen `cruise` ships and the six `attackmove` ships stay at
`Rudder` and never reach it.

The one stand-in that was on this path is gone. `009DA268` loads the divisor from
`[[blk+3FCh]+538h]+524h`, and packet `ship_ai_class_field_0524` found the producer: `00828F20`
derives it as `0.5 * MaxRotAngle / MaxRotAngleChangeRatio`, both keys already read out of the
installed `VehicleClass` row here. All 6370 calls take the derived value, and `00828F20`'s own
positivity gate passed on every one.

### 4. Four of the nine state steps now run their own bodies

Packet `ship_ai_state_steps` projected `009DE050`, `009DA4E0`, `009DFF40`, `009E00A0`, the `stop`
step `009E14C0`, the `movetopos` step `009E5770` and the `attackmove` step `009E8820` with its
sub-state selector `009E86F0`. The executable runs all of them, and the run shows what each state
can and cannot decide:

| state | ships | step | what it produces | does it set a desired throttle |
| --- | --- | --- | --- | --- |
| `cruise` | 13 | `009E1170` | the three setters, from the latched autopilot triple | yes, and it is 0 for every ship of this mission |
| `stop` | 12 | `009E14C0` | `blk+1D0h = 0`, `009E0040` with the ship's own heading, and the avoidance trio at `blk+3ECh` / `+3F0h` / `+3F4h` | yes, and zero is the answer: it is an all-stop |
| `movetopos` | 1 | `009E5770` | a navigation goal through `009DE050`, which forces `Navigate` | no |
| `attackmove` | 6 | `009E8820` | a sub-state selection through `009E86F0` | no |

**This is the finding the milestone turns on.** `docs/SHIP_AI_STATE_STEPS.md` states it as a
reading and the run confirms it as behaviour: a navigation state's whole job is to hand `009DE050`
two floats, and the steering that follows belongs to `009ED6B0`'s navigation arm
`009EDA26..009EF228`, which this executable records. So a navigation state running is not the same
thing as a navigation state producing a desired value, and the run counts both:

```
summary mission ship ai state steps real=4803 goal_sets=98 goal_replans=0 units_with_goal=1
        substate_steps=588 navigate_mode_steps=486
```

`4803` state-step bodies against milestone 2n's 2940, and `245` records against 2108 - the 245 are
the controlled `DeRuyter`'s own player arm at `009E11E8`, which milestone 2n already recorded
there. `98` goal sets is `Kortenaer`'s `movetopos` step on each of its 98 re-plan ticks, `588` is
the six `attackmove` ships' sub-state steps at the vtable slot none of the five leaves has a body
for, and `486` is the number of steps `Kortenaer` spent in `Navigate` mode.

Two things a `movetopos` ship needs are still records, and they are why the goal is `(0,0)`.
`brain+0B2Ch` / `+0B34h`, the AI's goal vector, has no recovered writer anywhere - milestone 2n's
follow-up 5 is exactly this pair - so `009E57D0` reads the zeroes the block carries. And
`009E5847`'s resolve of `director+58h` into an entity is recorded, so the target arm of
`009E5837..009E58DE` short-circuits on the null at `009E584E` and the step never finishes its
command. `attackmove` ends its frame at `009E88F0`, the sub-state vtable `+0Ch`: none of
`009F3240`, `009E23B0`, `009E26C0`, `009F3670` or `007B3DD0` has a reconstruction.

### 5. The ring closes, and the base run still does not move

`0080E190` writes the slewed rudder at `unit + ([unit+97Ch]<<5) + 83Ch` and `0080E170` the
throttle at `+838h`. `00813020` then steps the live pair at `ring+148h` / `+14Ch` toward the read
slot and, in session mode 1, sets the read cursor to the old write cursor, so a hop write on step
N is the motion's input on step N+1. **No wiring was needed for the tick**: `00813020` has one
call site, `00826121` inside `00825F20`, and `src/game_hosts_units.cpp` has run it there once per
unit per fixed step since milestone 2i. This packet's brief expected otherwise.

On the milestone's own command line every ship still holds station, and the reason is now named
per state rather than as a single gap:

```
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=6370
        deadbands=15680 live_pair_changes=0 driven=0
summary mission world units=32 ... controlled=DeRuyter moved=0.00 total_path=0.00
```

Every desired throttle in the mission is zero, so the `009F4BC6` deadband fires on every step and
the hop faithfully carries zero into the ring. For the 25 `cruise` and `stop` ships that is the
recovered answer: nothing ordered them to move. For the seven `movetopos` and `attackmove` ships
it is a boundary, because their states decide a goal or a sub-state and never a throttle. The
publish `009F4D10` and the promotion `00825F2C` still run 15680 times each and are still read by
nothing, and that is no longer on the path to this unit's motion: the published 84-byte slot is
for *other* units, as `docs/UNIT_AI_ORDER_SLOT_READER.md` established.

### 6. `--ai-drive`, and a ship that moves

The milestone keeps `--ai-drive <unit>=<throttle>,<rudder>`, and section 4 says which states still
need it: `movetopos` and `attackmove`, whose steps produce no desired throttle at all. It is **a
labelled diagnostic stand-in and nothing else**: on each re-plan tick of the named unit the
executable calls the two recovered setters `009DBF90` and `009DFFB0` on that unit's own control
block, after whatever its state step did, with the switch's pair in place of a desired value the
state never forms. It engages on `--order-frame`, beside the order that chose the state.
Everything after those two calls is the game's own recovered path: their clamps, `009DFFB0`'s own
mode switch, `009ED6B0`, `009F4D10`, the `009F50C6` tail into `009F3F80`, the hop, `0080E170` /
`0080E190`, `00813020` and `00825F20`.

With `--order moveto:Java --order-unit Kortenaer --ai-drive Kortenaer=1,0.5`:

| quantity | value |
| --- | --- |
| ring hops / writes | 15680 / 15680 |
| `009F4BC6` deadbands | 15194, down from 15680 |
| rudder-law calls | 5884, down from 6370 |
| live-pair changes | 14 |
| ships that moved | 1 of 32 |
| `Kortenaer` straight-line distance | 417.54 |
| `Kortenaer` path length | 417.76 |
| final position, heading | (-32.08, -2083.70), -44.2206 deg |
| final forward speed, yaw rate | 18.2399, -0.034906 rad/s |
| ring slot / live pair at the end | 1.0000, 0.5000 / 1.0000, 0.5000 |

The rudder-law count falls because `009DFFB0` sets `blk+1C4h` to `Rudder`, which closes the
`009F44E4` gate for the driven ship: a state that commands a rudder keeps the rudder it asked for,
which is the recovered rule and not a side effect of the switch. The 14 live-pair changes are the
whole transient: the hop slews the slot by at most `dt * 1.5` (`00CE3D78`), so from the order at
in-mission frame 5 the slot reads 0.075, 0.150, 0.225 and on, one `1.5 * 0.05` step per frame; the
rudder saturates at 0.5 on step 12 and the throttle at 1.0 on step 19. The ring's own slew limits,
8.0 and 2.0 per second, are both faster than that, so the live pair equals the slot from the next
step on and stops changing once the slot saturates.

### 7. The same class gives the same profile

`Kortenaer` is `ShipClasses:PACK3_Icarus=265`, which is `VehicleClass[265] Tribal class 1941`.
Against `bsp_ship_motion_probe.exe --class 265 --steps 490 --dt 0.05 --throttle 1 --rudder 0.5`,
body A (the probe's no-gravity body, which is the one the executable's motion path matches):

| t (s) | d throttle | d rudder | d speed (m/s) | d yaw (rad/s) | d heading (deg) | d position (m) |
| --- | --- | --- | --- | --- | --- | --- |
| 1.00 | 0.000 | 0.025 | -0.6000 | 0.00431 | 0.1405 | 0.520 |
| 4.00 | 0.000 | 0.000 | -0.5974 | 0.00085 | 0.3106 | 2.320 |
| 7.00 | 0.000 | 0.000 | 0.0002 | 0.00000 | 0.4004 | 3.676 |
| 14.00 | 0.000 | 0.000 | 0.0002 | 0.00000 | 0.4004 | 3.872 |
| 24.50 | 0.000 | 0.000 | 0.0002 | 0.00000 | 0.4004 | 4.506 |

**From t = 7.00 s the difference stops growing.** The yaw rates are equal to the printed
precision, the speeds differ by 0.0002 m/s, and the heading offset is a constant 0.4004 deg; the
position separation then creeps from 3.68 m to 4.51 m only because two arcs of the same radius,
offset by 0.4 deg of phase, draw apart as they turn. The whole difference is the transient, and
it has a named cause: the two orders reach the ring by different routes. The driven ship's order
is slewed into the write slot at `dt * 1.5` by the hop; the probe's `--throttle/--rudder` path
issues the full pair into the ring through `00816A40` every step, so the only slew on its side is
`00813020`'s own 8.0 / 2.0 per second over a slot four positions ahead of the read cursor.
Removing the 0.30 s the executable spends before its first non-zero live pair,
`tools/motion_trace_compare.py` reports **no channel left its tolerance over the compared span**,
with peaks of 0.3938 m/s, 0.1998 deg, 2.2397 m and 0.00212 rad/s. Run as-is, without that shift,
it reports peaks of 0.6750, 0.4007, 4.5044 and 0.00433, final deltas of 0.0002, 0.4002 and 4.5042,
and a first divergence in speed at t = 0.20 s. Wiring the real state steps changed none of these
numbers: the driven ship's trajectory file is byte-identical to the one the first pass wrote,
because the drive overrides after the state step and `movetopos` writes no desired value of its
own.

### Host methods

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
--mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java --order-unit
Kortenaer --order-frame 5 --trajectory-csv local/traj_2o_base.csv --log local/run_2o_base.log
--game-root "<install>"`, exit 0: **495 concrete, 443 unimplemented**, and **497 / 442** with
`--ai-drive Kortenaer=1,0.5`. The baseline is not milestone 2n's published 464 / 428: main moved
between the two turns. Measured on this branch's own base commit 45e24d04, with this packet's
changes stashed and the tree rebuilt, the same command line reports **465 concrete, 432
unimplemented**. The packet's move splits cleanly, because the two branches it merged changed no
host binding at all: the ring hop of sections 1 to 3 was **+9 concrete and +2 unimplemented**, and
the state steps of section 4 a further **+21 concrete and +9 unimplemented**.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2o.json` (`ring_hop_steps`, `state_step_steps`,
`ai_drive_steps`, `routines`, `measurements`, `probe_comparison`, `corrections`,
`read_for_this_packet`, `still_unimplemented`). The ring hop, in call order, with `009F50E0` as the
containing function for the first row and `009F3F80` for the rest except the last two:

| Step | Call site | Callee | Disposition |
| --- | --- | --- | --- |
| the throttle ceiling | 009f5248 | 009f4da0 | record |
| the tail into the drive | 009f50c6 | 009f3f80 | concrete |
| the early out on blk+3F5h | 009f3ff2 | - | concrete |
| the ring write-slot read | 009f400d | - | concrete |
| the hull's forward speed | 009f4034 | 0092d730 | concrete |
| the unit's heading getter | 009f407b | vtable +50h | indirect |
| the astern heading flip | 009f409e | 00438aa0 | concrete, not reached |
| the heading error | 009f40bb | 00438b10 | concrete |
| the two unprojected spans | 009f40ca | - | record |
| class+524h, derived | 009da268 | 00828f20 | concrete, 6370 calls |
| the rudder law | 009f44f7 | 009da250 | concrete, 6370 calls |
| the law's own speed read | 009da2d7 | 0092d730 | concrete |
| the hop | 009f4b99 | - | concrete |
| the hop's deadband speed read | 009f4bd4 | 0092d730 | concrete |
| the ring rudder setter | 009f4ce8 | 0080e190 | concrete |
| the ring throttle setter | 009f4cfb | 0080e170 | concrete |
| the ring tick, since milestone 2i | 00826121 | 00813020 | concrete |

The state steps, with `009F50E0` as the containing function for the dispatch and each step's own
body for the rest:

| Step | Call site | Callee | Disposition |
| --- | --- | --- | --- |
| the state's vtable +0Ch | 009f5186 | - | indirect |
| `stop` | - | 009e14c0 | concrete |
| the world-bounds test | 009e14f3 | 0071c4f0 | record, no world object here |
| `stop`'s desired heading | 009e153c | 009e0040 | concrete |
| `movetopos` | - | 009e5770 | concrete |
| the AI goal vector brain+0B2Ch | 009e57d0 | - | record, no producer anywhere |
| the director command slot | 009e57ed | 0071bff0 | concrete |
| the final-leg test | 009e57f4 | 007adc60 | concrete, the absent-list arm |
| the navigation goal | 009e580f | 009de050 | concrete, 98 calls |
| the arrival test | 009e5821 | vtable +2Ch | indirect, contract unread |
| the command target resolve | 009e5847 | 00521ea0 | record |
| `attackmove` | - | 009e8820 | concrete |
| the sub-state selector | 009e88d8 | 009e86f0 | concrete |
| the sub-state's own step | 009e88f0 | vtable +0Ch | indirect, 588 records |

`ShipAiDrive::set_desired_throttle` (`009DBF90`) and `ShipAiDrive::set_desired_steering`
(`009DFFB0`) carry no call site, because no native site calls them here: the state's own vtable
`+0Ch` at `009F5186` is what would, and the switch stands in for the desired value it does not
produce.

### Corrections

1. **Milestone 2n's "the promoted slot has no reader" is no longer one of the reasons the ships
   do not move.** It is still true of the 84-byte slot and it is not on the path to a unit's own
   motion. The hop at the tail of `009F3F80` writes the unit's own order ring, `00813020` steps
   the live pair and `00825F20` reads it. Evidence: `009f50c6`, `009f4ce8`, `009f4cfb`, and the
   drive run's 417.54 units of travel through the same code path.
2. **This packet's brief asked for `00813020` to be run where the game does if the executable did
   not already. It already did.** `00813020` has one call site, `00826121` in `00825F20`, and
   `ShipMotionBinding::tick_order_ring` has run it there since milestone 2i. No wiring was added.
3. **`docs/SHIP_AI_THROTTLE_TO_RING.md`'s chain table, step 5, omits `009F3F80`'s early out.**
   `009F3FEB` / `009F3FF2` jumps to the epilogue on `blk+3F5h` and skips the ring read, the body
   and the hop. Nothing in this process writes the byte, so it never fired; the doc's chain should
   still name it.
4. **The same doc's "009DA250 is the only writer of blk+1D4h inside 009F3F80: both stores take its
   result" needs its gates.** `009F44E4` / `009F44EB` skips the first store whenever the steering
   mode is `Rudder`, and the second is inside the obstacle arm.
5. **This milestone's own first pass said the gate never opens on this mission, and that was an
   artefact of the eight unwired state steps.** With `ship_ai_state_steps` merged, `009E14C0`'s
   `009E0040` call and `009DE050`'s forced `Navigate` open it on thirteen ships, and `009DA250`
   runs 6370 times in 15680 steps instead of 0. The first pass's statement that "`009DA250` still
   has no run-time evidence from `bsp_game.exe`" is superseded: it now has 6370 calls.
6. **The `class+524h` stand-in is gone.** The first pass divided the heading error by `MaxRotAngle`
   and recorded it. `00828F20` is the producer, found by packet `ship_ai_class_field_0524`, and it
   derives `class+524h` as `0.5 * MaxRotAngle / MaxRotAngleChangeRatio`. Both keys are already read
   out of the installed `VehicleClass` row, so all 6370 calls take the derived value and the
   routine's positivity gate passed on every one.
7. **`tools/motion_trace_compare.py` can no longer read the probe's stdout.** Its `_PROBE_ROW`
   regex expects the step number and eight numeric columns; the probe prints nine, because a `y`
   column was added when the Dyn integrator landed, so every row fails to match and the tool exits
   with `no probe trajectory rows matched`. The comparison in section 7 was run after dropping that
   column into a copy; the tool itself was not edited, because it belongs to no packet this worker
   leased. It is follow-up 5 below.
8. **`tools/verify_report_calls.py` rejected a correct row because a recovered name contains the
   word "error".** Its live lookup tested `'error' not in text.lower()` against the whole bridge
   reply, so `Function: BSP_ShipAi_RudderFromHeadingError at 009da250 ... Body: 009da250 -
   009da3a4` was read as a failure and the script reported `callee 009da250 is not a Ghidra
   function`. The test now matches only a reply that begins with `error`, which is the shape the
   client's own exception path writes. `reports/game_executable_milestone_2n.json` still verifies
   at 49 rows checked and 0 failed.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| - | - | none |

Every address this milestone touched lies in a Ghidra function whose body range the bridge
reports: `009F50E0-009F5253`, `009F4DA0-009F50D2`, `009F3F80-009F4D06`, `009DA250-009DA3A4`,
`0080E170-0080E18A`, `0080E190-0080E1AA`, `00813020-008131D5`, `0092D730-0092D76E`,
`00438AA0-00438B0D` and `00438B10-00438B7D`. The state-step boundaries this milestone consumes
(`009E8820` among them) were defined by the integrator in commit 347471cd for packet
`ship_ai_state_steps`, whose own doc carries their table. No name was added; run-time evidence was
appended to `009f3f80` and `009f4da0`.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest case
`reconstructed_math` passes, 1 of 1. No test cases were added.
`python tools/verify_report_calls.py reports/game_executable_milestone_2o.json` checks 25 call
rows and reports 0 failures; the vtable dispatches and `0096515D`'s computed call are reported as
indirect.

```
bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
  --mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java
  --order-unit Kortenaer --order-frame 5 --trajectory-csv local/traj_2o_base.csv
  --log local/run_2o_base.log --game-root "<install>"

summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5048
        state_steps{concrete=4803 records=245} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
summary mission ship ai state steps real=4803 goal_sets=98 goal_replans=0 units_with_goal=1
        substate_steps=588 navigate_mode_steps=486
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=6370
        deadbands=15680 live_pair_changes=0 driven=0
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680 simulated=24.50 s
        controlled=DeRuyter moved=0.00 total_path=0.00
host methods 495 concrete, 443 unimplemented
  ship ai step 10  Yamakaze   state=stop       mode=heading dir=stopped throttle=0.000
        rudder=0.000 heading=-1.5708 target=-1.5708
  ship ai step 10  Haguro     state=attackmove mode=rudder  dir=stopped throttle=0.000
        rudder=0.000 heading= 0.0000 target= 3.1416
```

and, with `--ai-drive "Kortenaer=1,0.5"`:

```
  unit                 state      hops  writes deadband slot_thr slot_rud live_thr live_rud livechg
  Kortenaer            movetopos   490     490        4   1.0000   0.5000   1.0000   0.5000      14
  Haguro               attackmove  490     490      490   0.0000   0.0000   0.0000   0.0000       0
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=5884
        deadbands=15194 live_pair_changes=14 driven=1
  Kortenaer   PACK3_Icarus  0  7  1.000  0.500 moveto  -  -32.1  -2083.7  417.54  1
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680 simulated=24.50 s
        controlled=DeRuyter moved=0.00 total_path=417.76
host methods 497 concrete, 442 unimplemented
```

Every earlier switch was rechecked on this binary. The acceptance form `--order-frame 1 --order
throttle=1,rudder=1` reproduces milestone 2j's published numbers exactly, `heading -41.252617,
fwd 16.429752, yaw -0.061087` at t = 14 s, so neither slot 16 nor the state steps changed a
trajectory the player drives: the hop writes the same slot the standing order refills after it,
and the standing order wins. A 120 frame run with `--press-start-frame 30` and no `--menu-select`
exits 0 and reports 155 concrete and 79 unimplemented, a 40 frame title-only run reports 130 and
48, `--vfs-probe fonts/fonts.lua` exits 0 and `--vfs-probe does/not/exist.lua` exits 3: all four
match milestone 2n exactly. A `--mission-frames 60` run with no `--mission-complete-frame` still
ends on the frame count with `summary mission exit reachable=0`.

This remains a runtime-validated process, not a game-validated one. What it proves that milestone
2n did not is that the recovered chain from a ship AI state's decision to a moving hull is closed:
chain slot 16 runs, `009F4DA0`'s tail enters `009F3F80` on every step, `009F3F80`'s hop writes the
unit's own order ring write slot through the two recovered setters, `00813020` steps the live pair
toward it and `00825F20` turns it into motion; four of the nine state leaves run their own bodies
over that chain, the steering law and the class field behind it run for thirteen ships, and a ship
driven through the chain follows the same speed and turn profile as the reconstructed probe for the
same class, to 0.0002 m/s and 0.0000 rad/s once the transient is over. What it does not prove is
what a navigation state would ask for. `movetopos` and `attackmove` decide a goal and a sub-state
and never a throttle, the goal itself is a record because `brain+0B2Ch` has no writer, the
navigation arm that would turn a goal into a desired value is unprojected, the five `attackmove`
sub-state steps have no body, and the two spans of `009F3F80` that would reshape the desired pair
are records. The value that moved this ship still came from a diagnostic switch.

### Follow-up packets

1. **`ship_ai_navigation_arm`**, `009ED6B0` `009EDA26..009EF228` with `blk+3D0h` and `blk+3E0h`.
   It is now the **first** thing between a navigation state's goal and a moving ship: `009DE050`
   forces `Navigate` and this span is what is supposed to turn the goal into a desired throttle
   and rudder. `src/ship_ai_navigation_arm.cpp` already projects `009EE671..009EEAA2` for the
   probe; the executable needs the rest and a producer for `blk+3D0h`.
2. **`ship_ai_goal_vector`**: `brain+0B2Ch`, `brain+0B34h`, `brain+0B38h`. Unchanged from milestone
   2n follow-up 5 and now measurable: `movetopos` sets a goal 98 times a run and every one of them
   is `(0,0)` because nothing writes the pair.
3. **`ship_ai_attackmove_substates`**: `009F3240`, `009E23B0`, `009E26C0`, `009F3670` and
   `007B3DD0`, the five leaves `009E8450` builds. Six ships of this mission end every frame at
   `009E88F0` with none of them reconstructed.
4. **`ship_ai_obstacle_tables`**: `009F40CA..009F44E3` and `009F4502..009F4B98`, the two
   `2Ch`-stride tables at `blk+81Ch` / `blk+848h`, `009D6B40`, `009D8B90` and `009EC7C0`. Until
   they are projected the executable carries the state's desired pair through `009F3F80`
   unchanged, which is this milestone's own boundary.
5. **`motion_trace_compare_columns`**: `tools/motion_trace_compare.py`'s `_PROBE_ROW`, and the
   probe's own table header. Correction 7; the tool cannot read the probe it was written for.
6. **`ship_ai_throttle_ceiling`**: `009F4DA0`'s body, `brain+34Ch`, `brain+350h`, `009EC7C0` and
   the tuning block at `00424C40`. It does not gate the tail call, so it costs nothing yet.
7. **`director_new_target_gate`**, **the recon party list** and **the gun side of
   `docs/BOT_FIRE_TARGET.md`**, all unchanged from milestone 2n.
8. **`construct_world` 004de610**, unchanged from milestones 2h through 2n, and now also what
   `009E14C0`'s world-bounds test at `0071C4F0` needs.


## Milestone 2p: AI ships move on their own orders

Addresses: 009f516c / 009f1420 with its head 009f1457, 009f146b / 0071eb60, 009f1473 / 009e2fb0
and inside it 009e2fe9 / 00521ea0, 009e2ffd / 006952a0 and 009e300d / 00694a60, 009f1491, 009f14ec
/ 008053c0, 009f14fa / 009dfbe0, 009f1519 / 00922dc0, 009f156b / 009dbcc0 with 009dbcd9 / 00414db0
and 009dbced / 004142e0, the three stores 009f1572 / 009f157b / 009f1584 and the unprojected tail
009f158a; 009f51fa / 009ef230 with 009ef242 / 0092d730, 009ef32f and 009ef334 / 009eb660; the two
alternatives inside 009ed6b0, the station-keeping arm 009eda28 with its gate 009eda34 / 009eda41
and its exit 009ee57b, and the path pick 009ee580 with 009ee5c2 / 009ed3e0, 009ee5f4 / 009e3c00,
009ee600 and 009ee66c / 00815f30, then 009ee671 and 009eeaab; the middle of 009f3f80,
009f40ca..009f4b98, with 009f4174, 009f41a2 / 00419010, 009f42c1 and 009f42e0 / 0042ac60, 009f438c,
009f4406 / 00415550, 009f442a / 009ec7c0, 009f44ae and 009f4878 / 009d6b40, 009f44f7 and 009f4694 /
009da250, 009f4544 / 00415690, 009f4681 / 00438b10, 009f477e / 009d8b90 and 009f47b8 / 0092d730,
and 009f4dbc; the attackmove sub-states 009e88d8 / 009e86f0 with 009e8714, 009e8733, 009e873b /
00852860, 009e87bd / 007b6ee0 and 009e87cd / 009e85b0 with 009e85cd, 009e864c / 0082adc0 and
009e8658 / 004178f0, then 009e88f0 into 009f3240 with 009f328f / 009f3090, 009f332b / 009de050,
009f3360 / 00605070, 009f3375, 009f3429 / 00778890, 009f35e3, 009f3635 and 009f3647 / 009e00a0,
and the four members 009e23b0, 009e26c0, 009f3670 and 007b3dd0; inside 009f3090 the frame state
009f309b / 009f1bc0 with 009f1bf7, 009f1c6a, 009f1cde, 009f1d3c / 00811a30, 009f1db4 / 00bd2f10,
009f1e94 / 00417b10 and 009f1f2d, and the six records 009f30a2, 009f30a9, 009f30b8, 009f30c7,
009f30d6 and 009f30dd; the plan request 009e3780 with 009e3821 / 004218e0, 009e3831 / 00417e40,
009e3851 / 00417580, 009e387b / 004120d0, 009e38a2 / 0041b840, 009e3927 and 009e394c / 00bf681b,
009e3973 / 009d9230, 009e3980, 009e39f5 / 009d9d40, 009e3adb and 009e3af0, with 009ec680 a record;
009e14f3 / 0071c4f0 and its box at 00e188a8; 0071f290 with 0071f2d1, 0071f314, 0071f340, 0071f395
and 0071f39e; 009e57d0, 009e5847 / 00521ea0 and
009e585f in 009e5770; and 009f5ed0 / 0071df70 with 0071df75, 0071df83 and 0071dfae. Packet
`cc_exe_2p`, owner `agent/cc-exe-2p`. Sources: `src/game_hosts_ship_ai.cpp`,
`include/bsp/game_hosts_ship_ai.hpp`, plus edits to `src/game_hosts_units.cpp`,
`src/game_hosts_commands.cpp`, `src/game_hosts_lua.cpp`, their headers and
`tools/motion_trace_compare.py`. Report: `reports/game_executable_milestone_2p.json`. Ghidra was
read-only for this packet; one name was added and two existing names had run-time evidence
appended. Three packets landed on main during this packet's turn and are used here:
`ship_ai_path_planner` at `878325ba`, `ship_ai_approach_update` at `89d4bb77` and
`cc2_director_update_arms`.

Milestone 2o closed the chain from a ship AI state's decision to a moving hull and then reported
that every decision on it was zero: all 98 `movetopos` goal sets were `(0,0)`, the six `attackmove`
ships ended every frame at an unreconstructed sub-state vtable slot, and one ship moved only
because a labelled diagnostic switch stood in for a desired throttle no state produced. This
milestone runs the producers. The goal vector has a writer and the executable calls it, so 31 of
the 32 ships now carry a real goal taken from their own command; the six `attackmove` ships reach a
sub-state body on all 588 dispatches and carry a real approach point taken from that goal; the
navigating ships ask a real planner for a plan; every span of `009F3F80` runs a projection instead
of one record; the world-bounds rule runs at its own call site; and the automatic target think runs
where `0071F290` puts it and answers with the corrected predicate. What did not change is that no
ship moves without the switch, and the milestone's job is to say exactly which record blocks that,
per state, rather than as one gap.

### 1. The goal vector has a producer, and it is the unit's own command

`docs/SHIP_AI_GOAL_VECTOR.md` found the writer milestone 2o said did not exist: `009F1420`, the
brain's pre-pass, rewrites `brain+0B2Ch`..`+0B34h` on every AI sub-tick from whatever command the
entity is carrying. The executable runs it at `009F516C`, which `009F515B` gates on the accumulator
`ai+0B18h` reaching `ai+0B14h`, so it is a sub-tick and not a frame: **5048 bodies in 15680
controller steps.**

The lever is `0071EB60`, read whole for this packet. It is a three-way select on the command mode
at `director+30h`: mode 1 hands back `director+58h`, the descriptor `0071E6C0` assigned into
command slot 0; mode 2 the override descriptor at `director+18Ch`; anything else the lazily built
empty singleton at `00E19B98`. Milestones 2l, 2m and 2n already push those slots, so **every AI
unit of this mission answers mode 1** and the singleton is never returned. That is the whole of the
fifth correction of `docs/SHIP_AI_GOAL_VECTOR.md`, measured:

```
summary mission ship ai goal vector prepasses=5048 refreshes=4466 nonzero_goals=31
        brain_targets=19 path_plan_refreshes=3426 path_picks=3426 path_publishes=0
        station_keeping=0 sector_refreshes=15680 middle_runs=15680 substate_concrete=588
```

| unit | state | 0071EB60 answered | goal | refreshes |
| --- | --- | --- | --- | --- |
| `Kortenaer` | `movetopos` | mode 1, object id 2 | `(-250.0, -3000.0)`, which is `Java` | 99 of 99 |
| `Haguro` | `attackmove` | mode 1, object id 1 | `(250.0, -3000.0)`, which is `DeRuyter` | 1 of 98 |
| `Jintsu` | `attackmove` | mode 1, object id 2 | `(-250.0, -3000.0)`, which is `Java` | 1 of 98 |
| `Yamakaze` | `stop` | mode 1, object id 21 | `(3500.0, -4500.0)`, its own position | 98 of 98 |
| `Houston` | `cruise` | mode 1, position | `(1200.0, -6000.0)`, its own position | 245 of 245 |

Two things the table settles. A command that names no target gets the **owner's** world position in
its descriptor (`0046AAB0`'s empty-target arm), which is why a `stop` or `cruise` ship's goal is
where it already is; a command that names an entity gets `position_valid` clear and the object, so
`009E2FC4` substitutes the zero vector at `00F87574` and `009DBCC0` carries it out through the
target's matrix at `+0CCh`, which makes the goal the target's own world position. And the refresh
counts differ because of the gate `brain+0B28h`: it is reopened every period, then closed again at
`009F14DB`..`009F1504` when the target is on another side and this side's recon does not hold it.
The six `attackmove` ships target the opposing party, `009DFBE0` and `00922DC0` are both records
here, so their goal is written once and then latched, while a ship whose target is itself keeps
refreshing. Both records carry their own addresses.

### 2. `009F3F80` no longer records its middle

Packet `cc_ai_obstacle_tables` projected `009F40CA..009F4B98` operation for operation, and this
milestone runs it in place of the 2o host record, **15680 of 15680** times. Every span of
`009F3F80` now runs a projection. The run measures three things the static reading could not:

| what | value | why |
| --- | --- | --- |
| rudder-law calls `009F44F7` | 9310 of 15680 | 13 ships from milestone 2o plus the six `attackmove` ships, whose approach sub-state writes `brain+1CCh = 3` |
| danger level `blk+0A84h` | 1.0 for every ship on every step | `blk+37Ch`, the ramp's numerator, is written only inside `009EF910`, which no packet has read, so the ratio is `0 / 1` and `009F41A2`'s `00419010(1.0, 1.0, 4.0, 0.0, 0)` clamps to full danger |
| `unit+102Ch` | 1.5 for every ship | `min(2 * danger, 1.5)` from the inlined `009D4FB0` at `009F438C`, which follows from the row above |
| `009EC7C0` calls | 0 | the ceiling runs only when `blk+35Ch` is not `Stopped`, and no ship of this mission ever latches a direction |
| blocked sectors | 0 | section 3 |

The 65-bin cost profile at `blk+4h` runs at `009F44AE` on every step and cannot change a value:
nothing in the recovered chain writes those bytes, so `009D6B40`'s first bin test returns the
throttle untouched. The seven `AutoThrust` keys `009EC7C0` interpolates over are no longer absent -
this milestone loads them beside the turn multipliers milestone 2j already read, out of the
installation's own `ShipGlobals.lua`: `DEG(25)`, `DEG(75)`, `0.5`, `DEG(45)`, `DEG(90)`, `0.75` and
`6.0`, exactly the defaults `docs/GAMEPLAY_SETTINGS.md` tabulates for `+6CCh`..`+6ECh`.

### 3. The obstacle sectors refresh, and nothing is ever blocked

`009EF230` is chain slot 15, one slot before the throttle ceiling at `009F5248` whose tail runs
`009F3F80`, so the twelve sectors at `blk+808h` are always one call fresh when the drive reads
them. The executable runs its two recovered parts - the round robin on `blk+0A18h`, three of the
twelve sectors a frame over four frames, and the braking distance `009EF32F` stores from the hull
speed and `class+508h` - **15680** times. `009EB660`, the scan itself, is a record with its own
address: `docs/SHIP_AI_OBSTACLE_TABLES.md` read it only partially, the swept-arc query between
`009EB6B7` and `009EBECC` is unread, and the neighbour list at `blk+608h` that the read half walks
is empty in this process. So **no sector is ever marked blocked**, and the blocked-sector arm, the
astern latch `blk+380h` and the escape manoeuvre are all unreachable here. That is stated rather
than implied: the middle runs in full, and half of it has no input.

### 4. The arm's two halves, and the one that is not there

`docs/SHIP_AI_GOAL_VECTOR.md`'s fourth correction is that `009EDA28..009EE57B` and
`009EE580..009EE670` are **alternatives**, not a prologue and a tail: the first is gated on
`blk+3A5h` set and `blk+3A6h` clear and leaves through `009EE57B JMP 009EF206`. The executable
implements both sides of that gate. `blk+3A5h` has five writers (`009D5B90`, `009DA0D0`,
`009DA3B0`, `009DDBC0`, `009DE5B0`), all five are records here and two of them are chain steps of
the same frame, so the station-keeping arm was entered **0 times in 15680 steps** and the path pick
ran **3426** times, which is exactly the number of steps that ended in `Navigate` or
`NavigateAstern`.

The path pick runs `009ED3E0`, which asks the planner for a plan and `009E3C00` for a point.
**The plan request is no longer a record.** Packet `ship_ai_path_planner` landed on main at
`878325ba` during this packet's turn and was merged in before validation, so `009E3780` runs: it
latches the goal, asks the avoid-zone manager to push both endpoints clear, allocates a two-node
ship-to-goal graph and leaves the plan in state 1, then answers on every later call whether it is
still the plan for this goal and this pose. **3426 requests, 7 seeds and 3426 accepts** in the base
run, which says the goal drifted past the re-plan tolerance exactly once per navigating unit. The
three zone queries are records, because the avoid-zone singleton `004218E0` is not built here.

What the plan does **not** get is nodes between its two seeds. `009EC680`, the one state transition
per navigation tick that fills them, is leased to `agent/cc-ai-path-search` and is a record, so
`plan+34h` is 0 for every unit; and `009E3C00`'s point-producing tail past `009E3D81` belongs to
packet `ship_ai_path_follower` and is unprojected. The record therefore still comes back with
`node_18 = 0`, `009EE5F9` takes the skip, and `00815F30` publishes **0** times. That matters because
the projected output block `009EE671..009EEAA2` needs that point: it is reconstructed, the probe
exercises it, and in the executable it never runs.

**And the span past it is the one that would move a navigating ship.** `009ED6B0`'s direct-control
block - the ahead/astern latch at `009ED802`..`009ED8A5` and everything that follows it - is inside
`if (blk+1C4h != Navigate)`. A ship that `009DE050` put into `Navigate` therefore never latches a
direction and never gets a desired throttle from that block, and `009F3F80`'s middle answers
`blk+1D0h = 0.0f` on the first line of its throttle arm because `blk+35Ch` is still `Stopped`. The
only code that could break that loop is `009EEAAB..009EF228`, which no packet projects. It is
recorded with its own address and it is this milestone's single most load-bearing gap.

### 5. `attackmove` reaches a body on every dispatch

Milestone 2o recorded all 588 sub-state dispatches. This milestone dispatches them over the table
packet `ship_ai_state_steps` read out of `009E8450`, and **588 of 588** reach a body. All of them
reach the same one, `009F3240` at `state+8h`, and the run says why rather than leaving it to the
reader:

| member | step | selected | why |
| --- | --- | --- | --- |
| `state+8h` approach | `009F3240` | 588 | `009E8733`'s `vtable[5Ch](8)` and `009E8799`'s `(1Ch)` both answer false for a surface target, so `009E87B6` pins the machine here |
| `state+14C0h` engage | `009E23B0` | 0 | `009E85B0` is the only path to it and its first conjunct is `[[unit+538h]+510h]` / `+514h`, which have no producer anywhere |
| `state+14CCh` lead pursuit | `009E26C0` | 0 | reached only from the kind-8 arm with `00852860` open |
| `state+14E0h` tangent | `009F3670` | 0 | the same arm with `brain+0B28h` clear |
| `state+14F4h` initial | `007B3DD0` | 0 | `009E84F8` seeds `state+1500h` negative, so the first selector call moves off it before the first dispatch. Its body is one `RET 4` |

`009F3240` calls `009DE050` 588 times, which is why `goal_sets` rises from 98 to **686** and
`units_with_goal` from 1 to **7**. **Those goals are real.** Packet `ship_ai_approach_update`
landed on main at `89d4bb77` during this packet's turn and was merged in before validation, so
`009F3090` runs its seven-call driver and the first of the seven, the frame state `009F1BC0`, runs
its projection. `009F1F2D..009F1F3D` copies the attackmove destination into the approach point
verbatim - the displacement through `00417B10` needs a zone object at `target+740h`, which has no
producer here - and `009F1CDE` rewrites the planar range to that destination every frame:

| unit | target | approach point | goal range |
| --- | --- | --- | --- |
| `Haguro`, `Yudachi`, `Murasame` | `DeRuyter` | `(250.0, -3000.0)` | 3250.0, 2750.0, 3288.3 |
| `Jintsu`, `Samidare`, `Harusame` | `Java` | `(-250.0, -3000.0)` | 3250.0, 2750.0, 3288.3 |

The other six callees of `009F3090` are records with their own addresses, and the reason is one
chain: `009E76D0`, the ring scan, ranks 60 slots by five weights that four unread scorers
(`009E6400`, `009E5DA0`, `009E6870`, `009E6640`) fill, and it is the caller of `009E5E90`, the only
writer of the commanded heading `nested+120Ch`. That field is in turn the first input of
`009E6A90`, the only writer of the commanded throttle `nested+1210h`. So the two values `009F3240`
forwards to the brain are still records, and the step's own desired throttle,
`brain+1D8h = clamp(sub+1218h, -1, 1)` at `009F3635`, is zero.

**One hazard is worth recording, because it produced a false result before it was caught.**
`009F1BF7` seeds `nested+1210h` with the `9999.0f` sentinel at `00CE4C04` on **every** frame, and
`009E6A90` is the only routine that replaces it. With `009E6A90` recorded, the sentinel survives to
`009F3635`, whose clamp to `[-1, +1]` turns it into **full ahead**: an intermediate build of this
milestone sailed all six ships and reported `total_path=2819.14` in the base run, with no
diagnostic switch anywhere. The read at `009F339A` now recognises the sentinel and answers the
neutral zero, and the run is back to 0 units moved. `009E6A90` cannot simply be run in its place,
because its own first act is `wrap(heading - nested+120Ch)` against the field the recorded ring
scan leaves unwritten. A sentinel is not a value, and a complete routine over an unproduced input
is not a recovery.

**`brain+1D8h` is `blk+1D0h`.** `blk` is `brain+8h`, so the five displacements
`docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md` lists for this step are control-block fields:
`brain+1CCh` is `blk+1C4h` the steering mode, `brain+1D0h` is `blk+1C8h` the throttle hold,
`brain+1D4h` is `blk+1CCh` the requested direction, `brain+1D8h` is `blk+1D0h` the desired throttle
and `brain+1E0h` is `blk+1D8h` the desired heading - which is what the `00605070` wrap at
`009F3360` confirms, because that is the same routine `009E0040` applies to `blk+1D8h`. So the
approach step does produce a desired throttle, and it leaves the block in `NavigateAstern`.

### 6. The automatic target gate, corrected and measured

The think itself now runs where the game runs it. `docs/DIRECTOR_UPDATE_ARMS.md` reads `0071F290`,
the controller vtable's `+0Ch`, whole: seven arms in order, of which arm 3 is the hold countdown
`0071F314` and arm 7 ticks `[controller+38h]->vtable[4](dt)` after the command stepping and before
`vtable[7Ch]`. The director's own vtable `00D21B48` holds `009F5DA0` in slot `+4h`, so that tick is
the automatic target think and `vtable[7Ch]` is `00836920`. Milestone 2n **supplied** that
placement; this milestone takes it from the routine, **15680** bodies, one per unit per fixed step.
Three arms stay records: the path-vector reset at `0071F2D1` has no `+1A4h` path array here, and the
two begin-command arms are gated on the accepted bytes at `+44h` / `+4Ch`, which this process does
not model - which is what keeps `00835C70` from being stepped twice, since milestone 2l already runs
it at its own site. `00836920` is logged where milestone 2m runs it, not repeated here. What this
executable still supplies is where `0071F290` itself runs, because the controller vtable has no
caller in the recovered graph.

Milestone 2n and 2o carried `AutoTarget::director_accepts_new_target` as a record whose note said
the float at `director+40h` must be **greater** than `0.0f` and that the field had no writer.
`docs/DIRECTOR_TARGET_GATE.md` corrected both. The executable now runs the predicate:

* **Rule 1.** `0071DF7C` is `JBE` to the slot scan and the fall-through at `0071DF7E` is
  `XOR AL,AL / RET`, so a hold above `0.0f` is the rejection and at most `0.0f` passes. `00720225`
  constructs the field at `-1.0f` in `00720180`, which `008363E0` calls at `00836403` with `ECX`
  still the director; `0071F314`'s `hold -= dt` runs only while the value is at or above `0.0f`, so
  the sentinel never moves; `00817031`'s `3.0f` is the `cleartarget` arm and this mission never
  issues one. **403 of 403 tested ticks read `-1.0f` and passed.**
* **Rules 2 and 3.** Walk `director+54h` in `1Ch` steps from index 0, stop at the first null, and
  reject when any of those commands answers category 1 or 2 from `vtable[0Ch]`.

What slot 0 carries, which is the question the packet brief asked:

| state | slot 0 | category | 0071DF70 |
| --- | --- | --- | --- |
| `attackmove`, the six Japanese destroyers | `attackmove` | 2 | **rejected at `0071DFC2`**, every tick |
| `movetopos`, `Kortenaer` | `moveto` | 3 | accepted |
| `cruise` | `cruise` | 3 | accepted |
| `stop` | `stop` | 3 | accepted |

So the authored `attackmove` of the Japanese force blocks the automatic think **by design**, which
is the reading `docs/DIRECTOR_TARGET_GATE.md` offered and this run confirms: a queued weapon-run
command owns the unit's fire target and `008358D0` is what sets it. Every other unit's think now
runs to the end, and `00835860 BSP_WeaponDirector_SetFireTarget` is reached **403** times where
milestone 2o reached it 0 times. Ten units never reach the predicate at all, because their scan
found no candidate; the run's table says so per unit rather than printing the field's default.

### 7. `--ai-drive` stays, and this is the state it stays for

The switch is **kept**. Two of the four states this mission reaches now produce their own desired
values and two cannot, and the record that blocks each one is named:

| state | ships | produces a desired throttle | what blocks it |
| --- | --- | --- | --- |
| `cruise` | 13 | yes, and it is 0 for every ship: nothing ordered them to move | - |
| `stop` | 12 | yes, and 0 is the answer: `009E14C0` writes `blk+1D0h = 0` and holds heading | - |
| `attackmove` | 6 | yes, at `009F3635`, and it is 0 | `009E76D0`, the ring scan, with `009E5E90` inside it and the four unread slot scorers behind that. It is the only writer of `nested+120Ch`, which is in turn the only input of `009E6A90`, the only writer of the throttle `009F3635` forwards |
| `movetopos` | 1 | **no** | `009EEAAB..009EF228`, unprojected everywhere, with `009EC680` and `009E3C00`'s tail behind it: `009ED6B0`'s own latch block is skipped for `Navigate`, so nothing on the recovered path can form one |

Two states out of four therefore still need the switch, and one of them, `movetopos`, would need it
even with a perfect goal and a complete path. That is the honest form of milestone 2o's "the value
that moved this ship still came from a diagnostic switch": the goal is no longer the reason, and
neither is the approach point.

### Host methods

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
--mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java --order-unit
Kortenaer --order-frame 5 --trajectory-csv local/traj_2p_base.csv --log local/run_2p_base.log
--game-root "<install>"`, exit 0: **545 concrete, 470 unimplemented**, against milestone 2o's
**495 / 443** for the same command line on this branch's base commit `1f93aeaf`. With
`--ai-drive Kortenaer=1,0.5`: **547 / 470**.

The per-step table with the call site and callee of every row is
`reports/game_executable_milestone_2p.json` (`goal_vector_steps`, `path_steps`, `obstacle_steps`,
`attackmove_steps`, `state_step_steps`, `auto_target_steps`, `ai_drive_steps`, `routines`,
`measurements`, `probe_comparison`, `corrections`, `read_for_this_packet`,
`still_unimplemented`). The goal-vector chain, in call order, with the containing function where it
is not the packet's main routine:

| Step | Call site | Callee | Disposition |
| --- | --- | --- | --- |
| the brain pre-pass, chain slot 6 | 009f516c | 009f1420 | concrete, 5048 bodies |
| the active command descriptor | 009f146b | 0071eb60 | concrete |
| the latch onto brain+0AF8h | 009f1473 | 009e2fb0 | concrete |
| the command target resolve (in 009e2fb0) | 009e2fe9 | 00521ea0 | concrete |
| the observer pair (in 009e2fb0) | 009e2ffd / 009e300d | 006952a0 / 00694a60 | record |
| the kind filter | 009f1491 | vtable +5Ch | indirect, true for a ship |
| the recon slot | 009f14ec | 008053c0 | record |
| the recon holds the target | 009f14fa | 009dfbe0 | record, false |
| the surface-target reopen | 009f1519 | 00922dc0 | record, false |
| the goal position | 009f156b | 009dbcc0 | concrete |
| the target's matrix (in 009dbcc0) | 009dbced | 004142e0 | concrete |
| the unprojected tail | - | 009f158a | record |
| the sector refresh, chain slot 15 | 009f51fa | 009ef230 | concrete, 15680 bodies |
| the sector scan (in 009ef230) | 009ef334 | 009eb660 | record, 3 per step |
| the station-keeping arm | - | 009eda28 | record, 0 entries |
| the path pick | - | 009ee580 | concrete, 3426 entries |
| the path plan refresh | 009ee5c2 | 009ed3e0 | record |
| the plan request | - | 009e3780 | concrete, 3426 requests and 7 seeds |
| the node allocation (in 009e3780) | 009e3927 | 00bf681b | concrete |
| the plan's zone queries (in 009e3780) | 009e3821 / 009e3831 / 009e38a2 | 004218e0 / 00417e40 / 0041b840 | record |
| the path search | - | 009ec680 | record, leased elsewhere |
| the path point | 009ee5f4 | 009e3c00 | record, no node |
| the lateral publish | 009ee66c | 00815f30 | record, 0 calls |
| the navigation arm tail | - | 009eeaab | record |
| the drive's middle | - | 009f40ca | concrete, 15680 bodies |
| the danger ramp | 009f41a2 | 00419010 | concrete |
| the danger step | 009f42c1 / 009f42e0 | 0042ac60 | concrete |
| the throttle ceiling | 009f442a | 009ec7c0 | concrete, 0 calls |
| the throttle profile | 009f44ae | 009d6b40 | concrete, all bins zero |
| the rudder law | 009f44f7 | 009da250 | concrete, 9310 calls |
| the rudder limit | 009f4544 | 00415690 | concrete, gate closed |
| the sub-state selector | 009e88d8 | 009e86f0 | concrete |
| the altitude gate | 009e873b | 00852860 | record, unreachable |
| the engage gate | 009e87cd | 009e85b0 | concrete, always false |
| the readiness pair (in 009e85b0) | - | 009e85cd | record |
| the sub-state's own step | 009e88f0 | vtable +0Ch | indirect, 588 dispatches |
| the approach step | - | 009f3240 | concrete, 588 bodies |
| the approach ring update | 009f328f | 009f3090 | concrete, 588 bodies |
| the approach frame state | 009f309b | 009f1bc0 | concrete, the approach point and the goal range |
| the ring scan and the commanded heading | 009f30d6 | 009e76d0 | record |
| the throttle limiter | 009f30dd | 009e6a90 | record, and its input is the row above |
| the approach's navigation goal | 009f332b | 009de050 | concrete, 588 calls |
| the movetopos goal read | - | 009e57d0 | concrete, no longer a record |
| the movetopos target resolve | 009e5847 | 00521ea0 | concrete |
| the movetopos kind test | 009e585f | vtable +5Ch | indirect, false for a ship |
| the `stop` step's world-bounds test | 009e14f3 | 0071c4f0 | concrete rule, its box is the record |
| the command controller update | - | 0071f290 | concrete, 15680 bodies |
| the think, through the director's vtable | 0071f395 | vtable +4h | indirect, 009f5da0 |
| the auto-target gate | 009f5ed0 | 0071df70 | concrete, 403 passes |

### Corrections

1. **Milestone 2o's `the AI goal vector brain+0B2Ch - record, no producer anywhere` is wrong and
   its 98 all-zero goal sets are explained.** `009F1420`'s head is the producer, the executable
   runs it at `009F516C`, and 31 of 32 units end with a non-zero goal. Evidence: `009f1473`,
   `009f156b`, `009f1572`, and the run's goal-vector table.
2. **Milestone 2n and 2o's note on `0071DF70` reads the compare backwards.** A hold *above* `0.0f`
   is the rejection, and the field has three writers, of which this process reaches `00720225`'s
   `-1.0f`. Evidence: `0071df7c`, `0071df7e`, `docs/DIRECTOR_TARGET_GATE.md`, and 403 passes.
3. **Milestone 2o's "their states decide a goal or a sub-state and never a throttle" is too
   coarse.** `attackmove` does write one, at `009F3635`; it is zero because `009F3090` is unread.
   `movetopos` writes none, and cannot, because `009ED6B0`'s latch block is skipped for `Navigate`.
4. **Milestone 2o's host record over `009F40CA..009F4B98` is gone.** Every span of `009F3F80` runs
   a projection, 15680 times.
5. **Milestone 2o correction 7 and its follow-up 5 are closed.** `tools/motion_trace_compare.py`
   now accepts the probe's nine-column table and still accepts a pre-Dyn eight-column capture. No
   other part of the tool changed, and the comparison below was run with it as it now stands.
6. **`docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md`'s brain displacements for `009F3240` are control-block
   fields.** Not a contradiction, an anchor: `brain+1CCh`/`+1D0h`/`+1D4h`/`+1D8h`/`+1E0h` are
   `blk+1C4h`/`+1C8h`/`+1CCh`/`+1D0h`/`+1D8h`, fixed by the `00605070` wrap at `009F3360` matching
   `009E0040`'s own wrap of `blk+1D8h`. The approach step leaves the block in `NavigateAstern`.
7. **The packet brief's `bsp_ship_motion_probe.exe --class 20` is not `Kortenaer`'s class.** The
   scene's `Type = E ShipClasses : PACK3_Icarus` resolves to 265, which is
   `VehicleClass[265] Tribal class 1941`, and that is what was run - the same class milestone 2o
   used.
8. **Milestone 2o's `0071C4F0` record was on the wrong thing.** The routine's body is read and its
   rule now runs at `009E14F3`; what is missing is the box the world object keeps at `[00E188A8]`
   `+711Ch` / `+7124h` / `+7128h` / `+7130h`, because `construct_world 004DE610` is still a load
   record. Running the four comparisons against a zero box would put every ship of this mission
   outside a world that does not exist, so the record moved onto the operands and the neutral
   `inside` answer stands: **0 of 12** `stop` ships took the navigate-to-origin arm.
9. **Milestone 2n's "the executable decides where the automatic target think runs" is superseded.**
   `0071F290` arm 7 is the call site, and the director's vtable `00D21B48` makes it `009F5DA0`.
   What is still supplied is where `0071F290` runs, not where the think runs inside it.
10. **A sentinel is not a value, and this milestone proved it the expensive way.** An intermediate
    build ran `009F1BC0` without `009E6A90`, and `009F1BF7`'s `9999.0f` seed reached `009F3635`'s
    clamp and sailed all six `attackmove` ships at full throttle for `total_path=2819.14` with no
    diagnostic switch in the run. Section 5 has the fix and the reason `009E6A90` cannot be run in
    its place.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 009e8820 | 009e88f9 | the `attackmove` composite step; boundary defined by packet `ship_ai_state_steps`, unchanged here |
| 007b3dd0 | 007b3dd2 | the empty sub-state step, one `RET 4`; boundary defined by packet `cc_ai_attackmove_substates`, unchanged here |

Every other address this milestone touched lies in a Ghidra function whose body range the bridge
reports. `python tools/verify_report_calls.py reports/game_executable_milestone_2p.json` checks 73
call rows and reports 0 failures; the vtable dispatches are reported as indirect.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest case
`reconstructed_math` passes, 1 of 1. No test cases were added. `origin/main` was merged in before
validation for the three packets this milestone consumes - `ship_ai_path_planner` at `878325ba`,
`ship_ai_approach_update` at `89d4bb77` and `cc2_director_update_arms` - and the two ledger shards
that conflicted, `config/names/00710000.jsonl` and `config/names/009f0000.jsonl`, were resolved by
keeping both sides' records.

```
bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
  --mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java
  --order-unit Kortenaer --order-frame 5 --trajectory-csv local/traj_2p_base.csv
  --log local/run_2p_base.log --game-root "<install>"

summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5048
        state_steps{concrete=4803 records=245} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
summary mission ship ai goal vector prepasses=5048 refreshes=4466 nonzero_goals=31
        brain_targets=19 path_plan_refreshes=3426 path_picks=3426 path_publishes=0
        station_keeping=0 sector_refreshes=15680 middle_runs=15680 substate_concrete=588
summary mission ship ai plan requests=3426 seeds=7 accepts=3426 approach_frames=588
        controller_updates=15680
summary mission ship ai state steps real=4803 goal_sets=686 goal_replans=1 units_with_goal=7
        substate_steps=588 navigate_mode_steps=3426
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=9310
        deadbands=15680 live_pair_changes=0 driven=0
summary mission auto target thinks=812 scans=787 chose=22 fire_target_sets=403
        attackmove_issues=0 accepts=403
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680 simulated=24.50 s
        controlled=DeRuyter moved=0.00 total_path=0.00
host methods 545 concrete, 470 unimplemented

  unit          state       plan_req  seeds  state  nodes  approach  point_x  point_z  goal_range
  Kortenaer     movetopos        486      1      1      0         0      0.0      0.0         0.0
  Haguro        attackmove       490      1      1      0        98    250.0  -3000.0      3250.0
  Jintsu        attackmove       490      1      1      0        98   -250.0  -3000.0      3250.0
```

Every one of the 32 ships reports `moved 0.00` in the base run, which is unchanged from milestone
2o; `local/traj_2p_base.csv` is byte-identical to the file the pre-change binary writes for the
same command line, so nothing this milestone added changed a trajectory. With
`--ai-drive Kortenaer=1,0.5`:

```
summary mission ship ai goal vector prepasses=5048 refreshes=4466 nonzero_goals=31
        brain_targets=19 path_plan_refreshes=2940 path_picks=2940 path_publishes=0
        station_keeping=0 sector_refreshes=15680 middle_runs=15680 substate_concrete=588
summary mission ship ai plan requests=2940 seeds=6 accepts=2940 approach_frames=588
        controller_updates=15680
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=8824
        deadbands=15194 live_pair_changes=14 driven=1
  Kortenaer   PACK3_Icarus  0  7  1.000  0.500 moveto  -  -32.1  -2083.7  417.54  1
summary mission world units=32 ... controlled=DeRuyter moved=0.00 total_path=417.76
host methods 547 concrete, 470 unimplemented
```

`Kortenaer`'s final row of `local/traj_2p_drive.csv` is
`490, 24.499912, -32.079800, 0.0, -2083.696045, -44.220619, 18.239912, 1.0, 0.5, -0.034906`,
identical to milestone 2o to every printed digit: the driven ship's trajectory did not move, which
is the point - the goal, the sub-state and the obstacle middle all run, and none of them reaches
that ship's desired pair.

Against `bsp_ship_motion_probe.exe --class 265 --steps 490 --dt 0.05 --throttle 1 --rudder 0.5`,
through `python tools/motion_trace_compare.py --trace local/traj_2p_kortenaer.csv --probe
local/probe_2p_class265.txt --align-origin`:

| channel | peak &#124;delta&#124; | final delta |
| --- | --- | --- |
| speed (m/s) | 0.6750 | 0.0002 |
| heading (deg) | 0.4007 | 0.4002 |
| position (m) | 4.5044 | 4.5042 |
| yaw (rad/s) | 0.00433 | - |

with the first divergence in speed at t = 0.20 s. Dropping the 0.30 s the executable spends slewing
its first non-zero live pair, the tool reports **no channel left its tolerance over the compared
span**, with peaks of 0.3938, 0.1996, 2.2321 and 0.00212. Both sets reproduce milestone 2o's
published numbers, which is the regression this comparison is for.

The comparison the packet brief asked for, against the probe driving itself to the goal
(`--class 265 --moveto -250,-3000`, which is `Java`'s position from the run log), is a **finding
rather than a match**: peaks of 0.6750 m/s, 44.1620 deg, 110.5314 m and 0.03490 rad/s. The probe's
chain, handed the goal directly, saturates the rudder at 1.0 and turns at -0.06981 rad/s; the
executable's `Kortenaer` holds the diagnostic switch's 0.5 and turns at -0.034906. The difference is
the stand-in, not the motion model, and it measures exactly the gap section 4 names: the probe
substitutes a single path point for `009E3C00`, and the executable plans a graph whose nodes
`009EC680` never fills.

Every earlier switch was rechecked on this binary. The acceptance form `--order-frame 1 --order
throttle=1,rudder=1` reproduces milestone 2j's published numbers exactly, `heading -41.252617,
fwd 16.429752, yaw -0.061087` at t = 14 s. A 120 frame run with `--press-start-frame 30` and no
`--menu-select` exits 0 and reports **155 concrete and 79 unimplemented**, a 40 frame title-only run
reports **130 and 48**, `--vfs-probe fonts/fonts.lua` exits 0 and `--vfs-probe does/not/exist.lua`
exits 3: all four match milestone 2o exactly. A `--mission-frames 60` run with no
`--mission-complete-frame` still ends on the frame count with `summary mission exit reachable=0`.

This remains a runtime-validated process, not a game-validated one. What it proves that milestone
2o did not is that the AI's own decisions are now recovered values rather than records: the goal
vector comes from the unit's own command through `0071EB60`, the six `attackmove` ships reach a
real sub-state body on every dispatch and carry a real approach point taken from that goal, the
navigating ships ask a real planner for a plan, every span of `009F3F80` runs a projection, the
obstacle sectors refresh on the schedule the image keeps, the world-bounds rule runs at its own
call site, and the automatic target think runs where `0071F290` puts it and is blocked only where
the authored commands say it should be. What it does not prove is that any of those decisions can
move a hull. The path search is a record, so the plan has two nodes and no point; the navigation
arm's tail is unprojected; the approach ring scan is a record, so the two commands it feeds are
too; the clearance behind the danger ramp has no producer and the throttle profile has no writer.
The chain runs end to end over a desired pair that is still zero, and the one ship that moves is
still moved by a labelled switch.

The milestone's own cautionary result is in section 5: an intermediate build that ran the approach
frame state without its throttle limiter moved all six ships 2819 units on a `9999.0f` sentinel.
Running a complete routine over an input its producer never wrote does not make the output
recovered, and a run that starts moving is not by itself evidence that it started moving for the
right reason.

### Follow-up packets

1. **`ship_ai_navigation_arm_tail`**, `009EEAAB..009EF228` of `009ED6B0`. Promoted to first place:
   it is the only code that can latch `blk+35Ch` and form a desired throttle for a ship in
   `Navigate`, because `009ED6B0`'s own latch block is inside `if (mode != Navigate)`. Until it is
   projected, no `movetopos` ship can move on its own order however good its goal is.
2. **`ship_ai_path_search`**, `009EC680` with `009E3C00`'s tail past `009E3D81`. `009E3780` now
   seeds a two-node ship-to-goal graph 7 times a run and answers 3426 requests, and the search that
   fills the nodes between the seeds is the only thing between that and a path point. `009EC680` is
   leased to `agent/cc-ai-path-search`; the point source belongs to `ship_ai_path_follower`.
3. **`ship_ai_attackmove_ring_scan`**, `009E76D0` with `009E5E90` and the four slot scorers
   `009E6400`, `009E5DA0`, `009E6870` and `009E6640`. The approach point and the goal range are
   recovered now; the commanded heading `nested+120Ch` is not, and it is also the only input of
   `009E6A90`, which is the only writer of the commanded throttle. Both are what `009F3240`
   forwards to the brain.
   Beside it, the five unread spans of `009F1BC0` (`009F1DBF`, `009F2124`, `009F221C`, `009F2395`,
   `009F270A`) and the four further approach-point stores at `009F2216`, `009F237D`, `009F23B5`
   and `009F26F0`.
4. **`ship_ai_clearance_37c`**, `009EF910` and `009F4D10`, `blk+37Ch` and `blk+33Ch`. The run shows
   the cost of the gap: the danger level saturates at 1.0 for every ship on every step and the
   rudder limit's gate at `009F4514` never opens.
5. **`ship_ai_throttle_profile_producer`**, `blk+4h..blk+45h`, `009D6B40`, `009E04E0`. Both readers
   run now and neither can change a value, because every bin is zero.
6. **`ship_ai_obstacle_sector_scan`**, `009EB660` between `009EB6B7` and `009EBECC`, with
   `009D84E0` and the neighbour list `blk+604h` / `blk+608h`. The refresh runs 15680 times and
   marks nothing.
7. **`ship_ai_unit_armament_component`**, the object at `unit+538h`. Its `+510h` / `+514h` pair is
   the first conjunct of `009E85B0` and is why the engage sub-state is never selected.
8. **`entity_kind_predicate_vtable_005c`**, the slot the goal vector, the selector and the
   `movetopos` step all call with 2, 8, 9 and 1Ch. The executable answers it from the recovered
   chain `006FE530`, and what each literal selects is still a reading from use.
9. **`construct_world` 004de610**, unchanged from milestones 2h through 2o, and now the single
   missing operand of a rule that otherwise runs: the world box `0071C4F0` compares against, and
   the session mode at `[*(00E188A8) + 1FE4h]` that arm 7 of `0071F290` tests.

## Milestone 2q: the scene's cruise ships sail, a moveto ship gets a path, and a finished command comes back through the session

Addresses: 00926110 into 00822c20 with 00823576 / 008f2260, 00823590 / 008f2260, 008235ba and
008235dc / 0080fc30, 008235d5 / 0080d9b0 and 008235f7 / 0092d770; 009ee5c2 / 009ed3e0 and inside
it the arm 009ed4e4..009ed69e, with 009ed4ae and 009ed4d1 / 009d9de0, 009ed523, 009ed588, 009ed5fd
and 009ed692 / 009e3780, 009ed558 / 009d9d40, 009ed59b and 009ed614 / 009ec680, and the swap
009ed5b6 / 009ed5bd / 009ed5c3; inside 009ec680's tree 009ec30b / 00424c40, 009e3058 / 004218e0,
009e3075 / 00417e90, 009e3105 / 00422500, 009e3189 / 0071c4f0, 009e3263 / 00bf681b, 009d5917 /
00417610, 009d5923 / 00423190 and 009e340d; 009ee5f4 / 009e3c00 with its walk 009e3c0d..009e3d96,
009e3dcd / 00811d80, 009e3eae / 0082e850, 009e3ec0, the unprojected corner arm 009e3f1a..009e4222
and the store 009e4223..009e4328; 009ee609 into the output block 009ee671..009eeaa2 with 009ee6ac /
009d9e50, 009ee765, 009ee776 and 009ee8c7, then the tail 009eeaab..009ef226 with 009eeb63 /
004192e0, 009eeb8b, 009eebb0 / 009dbbc0, 009eebc8, 009eebd2, 009eebe0, 009eec41 / 00427eb0,
009eecdb, 009eef0a, 009eef34, 009ef034, 009ef045, 009ef051, 009ef0d6, 009ef112 and 009ef213 /
009de5b0, and the four inlined copies of 009d5240; the per-tick clears 009ed769, 009ed772 and
009ed779; 009e5821 through 009dab10 into 009da590; 009e595c / 00984300, 009e5997 and 009e88c1 / 0071e430 with 0071e463 /
0071d810, 0071e480 / 0071d9e0, 0071d852 / 0071c730, 00721bb7 / 00720850, 00720889, 00720916 /
00521ea0, 0072092a / 006952a0 and 00720b56; 0097e611 and 0097e63a / 0097c8a0.

Packet `cc_exe_2q`, worker `agent/cc-exe-2q`. Sources: `include/bsp/ship_ai_path_refresh.hpp`,
`src/ship_ai_path_refresh.cpp`, `include/bsp/ship_ai_path_point.hpp`,
`src/ship_ai_path_point.cpp`, `src/game_hosts_units.cpp`, `include/bsp/game_hosts_units.hpp`,
`src/game_hosts_ship_ai.cpp`, `include/bsp/game_hosts_ship_ai.hpp`,
`src/game_hosts_commands.cpp`, `include/bsp/game_hosts_commands.hpp`,
`src/game_hosts_scene_contents.cpp`, `include/bsp/game_hosts_scene_contents.hpp`,
`cmake/startup.cmake`. Merged from `main` and consumed rather than written:
`include/bsp/ship_ai_navigation_arm_tail.hpp` (packet `ship_ai_navigation_arm_tail`, `4491d04f`)
and `include/bsp/ship_ai_ring_scan.hpp` (packet `cc_ai_ring_scan`, `6a9c557b`). Report:
`reports/game_executable_milestone_2q.json`. Ghidra was read-only:
no renames, comments, prototypes, function creation or saves. One ledger name was added
(`009D9DE0`) and run-time evidence was appended to two existing names (`009EC680`, `00822C20`).

### 1. Thirteen ships make way, and the speed is the one the mission authored

`00822C20`'s property-bag arm now runs where `BSP_SEntity_InitAll` puts it, at the slot-`0A0h`
call `00926110`, which in this process is the per-entity pass `create_units` makes over the
records the instantiate pass left. It runs before `issue_authored_commands`, which is the order
that matters: the arm writes the ring's live throttle and the latch at `00835E17` reads it.

The scene-contents host keeps what the arm looks up. `0046D5B0` wraps the merged entity bag in
the `0Ch`-byte holder at `entity+0C0h` with kind tag 1, and the executable does not build that
object, but it does keep the two keys the arm reads out of it: `ShipYardLaunch` and `StartSpeed`,
with the authored type letter, so `0082359C`'s compare against zero selects the same arm the
native selects.

All 32 created instances find a `StartSpeed` record, because the `Ship` property group declares
the key and `0046CF40` step 6 merges the group defaults into every bag. Fourteen carry the
authored `12.0`; the other eighteen carry the group's `0.0` and the seed writes two zeroes.

| quantity | value |
| --- | --- |
| units whose bag carries the key | 32 |
| units whose value is non-zero | 14 |
| authored `StartSpeed` | 12.0 m/s |
| `ring+148h` after `0080D9B0` | 0.6571 to 0.7405, one per class |
| argument to `0092D770` | 12.0 m/s for every one of the fourteen |

Twelve of the fourteen then hold 12.0 m/s for the whole run. The two that do not are `DeRuyter`,
which the player controls and whose ring the input path zeroes every step, and `Kortenaer`, whose
authored order is misfiled in the scene (section 2) and which this run's `--order` puts into
`movetopos`; both coast down from the seeded 12.0 m/s and end 34.80 m from where they started.

| run | expected | measured | delta |
| --- | --- | --- | --- |
| 490 steps, 24.5 s | 294.00 m | 294.05 m | 0.05 m |
| 1200 steps, 59.5 s | 713.99 m | 714.12 m | 0.12 m |

`expected` is the ratio `0080D9B0` received times the class `MaxSpeed` times the seconds
simulated. There is no acceleration transient to allow for, because `0092D770` sets the hull's
axial velocity to the same speed the throttle asks for: the ship is already at 12.0 m/s on its
first step. The delta is one frame of the 0.05 s step.

### 2. `Kortenaer`'s `Cruise` is in the wrong block, and the game would drop it too

The shipped `usn_2_java.scn` has fourteen `Command = E CommandType : Cruise` lines and fourteen
`StartSpeed = F 12.0000` lines, and `docs/CRUISE_SPEED_SETTING.md` reads that as fourteen `Cruise`
units. A count that tracks which sub-block each line sits in says otherwise: **thirteen** of the
Cruise lines are inside a `"Command"` sub-block and one, `Kortenaer`'s at line 1104, is inside its
`"CamoColorGun"` sub-block. `Kortenaer`'s own `"Command"` block is empty.

`004E6B30` queues a command only for the `Command` key of the `"Command"` sub-block, so the
shipped game gives `Kortenaer` no order at all, and the executable reproduces that: with no
`--order` on the command line `Kortenaer` is one of thirteen ships the director's idle tail hands
a `stop`. The two counts are therefore different questions with different answers - fourteen
units are seeded with a speed, thirteen are ordered to cruise - and milestone 2n's "thirteen ships
that author `Cruise`" was right about the one that matters for motion.

### 3. The search runs, and the arm that calls it is projected

`009ED3E0`'s second half is now a projection rather than a host contract. The arm
`009ED4E4..009ED69E` is `include/bsp/ship_ai_path_refresh.hpp`; what it does in one pass is
decided by `nav+2FCh`, the "a plan is being computed" byte:

| `nav+2FCh` | what the pass does |
| --- | --- |
| clear, both plans Empty | `009ED523` seeds the back plan and `009ED528` raises the byte, and the same pass falls into the arm below |
| set | revalidate the back plan for states 0, 2 and 3 (`009ED588`, with `009D9D40` first for a Failed one), tick `009EC680` on it at `009ED59B`, and when its state passes 3 swap it into use and clear the byte (`009ED5A6`, `009ED5B6`, `009ED5BD`, `009ED5C3`) |
| clear, a plan in use | revalidate the plan in use at `009ED5FD` and tick `009EC680` on it at `009ED614`; when the revalidation refuses, mark it Failed, reset the back plan and seed it at `009ED692`, and raise the byte again |

The head `009ED3E0..009ED4E2` is **not** projected. It builds two corridor widths, seeded from the
`20.0f` at `00CE3930` and replaced from the unit's group through `00778890`, `0070D400` and
`0070D5D0`, and hands each to `009D9DE0` on each plan. `009D9DE0` is read whole here and named
`BSP_ShipAiPathPlan_CorridorWidthChanged`: it answers "one of the two widths moved by more than
the `25.0` at `00CE3880` since the last pass", and then stores both widths whatever it answered.
Its one consumer is `009ED49A`-`009ED4DE`, which loads `nav+2FCh` into `BL`, ORs both answers into
it and writes it back - so the routine can only **raise** the flag, never clear it. Not projecting
the head therefore costs one thing and only one: a plan is never invalidated mid-search by a
corridor that changed width. Carrying the byte forward is what the image does whenever `009D9DE0`
answers false, which on a constant width is every call after the first.

With the arm in place the four-tick sequence `docs/SHIP_AI_PATH_SEARCH.md` predicts for an open
sea is what the run reports. The avoid-zone manager is not built in this process, so `00417E90`
cannot be asked and its neutral answer is "not blocked" - and on an empty sea that is the only
host call the search makes at all. `00422500`, `0071C4F0`, `00417610` and `00423190` are never
reached, and `009EC280`'s turn ramp reads three zeroes, which costs every route the same.

```
plan refreshes 3426   search ticks 3421   seeds 12   accepted revalidations 3397
front/back swaps 12   plans reaching state 7 (Ready) 7   node_count after the prune 0
```

`node_count` is zero on a Ready plan because `009EC721` clears `plan+34h` at the 3-to-4
transition, which is also why the walk in the next section takes no steps and stops on the head.

### 4. `009E3C00` hands back a point, and it is the goal node

The walk and the store are projected; the corner-rounding arm is not. In call order:

1. `009E3C11`-`009E3C37` seeds five plan fields from the query point and clears the record's
   `+18h` and `+1Ch`.
2. `009E3C3A`: a plan with no head answers with the query point and clears `+20h` and `+21h`.
3. `009E3C6C`-`009E3D0A`: the walk, at most `plan+34h` steps. With the count cleared by the prune
   it takes none and the head is the node it stops on.
4. `009E3D13`-`009E3D5D`: the successor and the node after it, each through `009D5930`.
5. `009E3D8C`: the successor's own `+18h`/`+1Ch`, or its lateral corner record offset by
   `00811D80`'s answer. No node of an unzoned plan carries such a record, so the first branch is
   the one taken.
6. `009E3F14`: **the branch**. A successor that still carries a link and an outgoing leg of at
   least one unit goes into the corner arm `009E3F1A..009E4222`, which is not projected. A two-node
   plan's goal node carries neither link, so the other arm runs: `+20h` is set, the direction is
   `successor - query`, and `009E42BD` raises the advance to `|successor - query|`.
7. `009E4320`: the point is the query point advanced by that distance along that direction, which
   is the successor's own position.

So on this mission the point `009E3C00` hands back is the goal node of the plan, exactly. It does
not depend on `[plan+3Ch]+9C8h`, the unit radius that has no producer anywhere, because
`009E42BD`'s `max` discards it; and it does not depend on `0082E850`'s turn radius, which
`009E3EAE` fetches unconditionally but only the corner arm and its unreachable clearance test read.
**0 of 8319 points needed the corner arm** over the 1200-step run.

### 5. The output block was never gated on the point, and milestone 2p read the jump backwards

Milestone 2p's section 4 says that with no point "`009EE5F9` takes the skip and neither the block
nor the bearing runs". The instruction at `009EE609` is `JZ 009EE671`, and `009EE671` is the
**first** instruction of the output block. A record with no node at `+18h` jumps *into* the block,
not past it; what `+18h` gates is the band computation `009EE60B`-`009EE64C` and the lateral
publish `009EE66C` through `00815F30`, and nothing else. The block runs on every pass the
`009EE59B` / `009EE59F` gate lets through, which is 3426 of the 490-step run's navigation ticks and
matches `navigate_mode_steps` exactly.

With the point now real, `009EE671`-`009EEAA2` writes the trio `009F4D10` publishes:

| unit | state | point x | point z | `blk+32Ch` | `blk+324h` |
| --- | --- | --- | --- | --- | --- |
| `Kortenaer` | movetopos | -250.0 | -2742.6 | 387.49 | -1.8845 |
| `Haguro` | attackmove | 250.0 | -3000.0 | 3250.00 | 3.1417 |
| `Jintsu` | attackmove | -250.0 | -3000.0 | 3250.00 | 3.1415 |

3419 of the 3426 blocks take the bearing at `009EE813`; the seven that do not are the passes
before each unit's plan first reaches a usable state.

### 6. The arm tail runs, and a `movetopos` ship moves on its own order

`009EEAAB..009EF226` was milestone 2p's first follow-up and is now packet
`ship_ai_navigation_arm_tail`, which landed on `main` at `4491d04f` during this packet's turn. It
is merged in and wired here, over a ten-method host and the block fields the output block just
wrote. Three of its four outputs are live in this process:

| output | site | what the run gets |
| --- | --- | --- |
| `blk+344h`, the approach throttle ceiling | `009EEF0A` | `clamp(blk+330h / stopping distance, 0.25, 1.0)` from the cached reference speed and the class `Retardation`, both real here. It clamps to 1.0 at this mission's ranges, and the executable now hands it to `009F4439`'s cap instead of the `009F4DA0` record's 1.0f |
| `blk+35Ch`, the ahead/astern latch | `009EF19C` through the inlined `009D5240` | 1955 of the 3426 tail bodies leave it non-zero. That is what takes `009F43D2`'s `direction == Stopped` arm out of the drive and lets the throttle chain run |
| `blk+2FDh` / `blk+2FEh`, the arrival bytes | `009EF034` / `009EF045` | 0 in this run, and section 7 says why |
| the traffic setback walk | `009EEAAB..009EEEEC` | 0 entries: `blk+604h` comes from `FUN_009E4330`, which no packet has read, and the world entity list at `[[00E188A8]+19CCh]` needs `construct_world` |

The executable also performs the three stores at `009ED769`, `009ED772` and `009ED779` that
`src/ship_ai_states.cpp`'s projection of the direct-control arm leaves out: without the per-tick
clear of `blk+2FDh` and `blk+2FEh` the arrival latch would be a one-way byte instead of a
per-frame answer.

**`Kortenaer` moves on its own order, with no diagnostic switch.** It leaves the seeded 12.0 m/s,
settles at throttle 0.500 with the rudder hard over, and covers 261.14 m in 24.5 s and 844.50 m in
59.5 s. The 0.500 is not a stand-in: it is `ThrustMinSlow` out of the shipped
`ShipGlobals["Navigator"]["AutoThrust"]` block, which `009EC7C0` interpolates to while the heading
error is wide.

### 7. The ship steers, and then the hull runs away sideways

`Kortenaer` does not reach `Java`, and the reason is not in the AI. Over a 3000-step run the
rudder never leaves the stop, the heading rotates at a constant 2.667 deg/s, and the ship's actual
speed grows while its forward-axis speed does not:

| t (s) | speed from the trajectory (m/s) | `fwd_speed` (m/s) | drift angle (deg) |
| --- | --- | --- | --- |
| 15 | 10.84 | 9.12 | 32.7 |
| 90 | 38.71 | 9.04 | 76.5 |
| 120 | 51.11 | 9.01 | 79.8 |
| 135 | 57.71 | 10.05 | 80.0 |

The hull is travelling almost sideways and accelerating. `0092D770` replaces only the component of
the velocity along the body axis and leaves the rest, which is correct; what the shipped game has
and this process does not is anything that resists the rest. The rigid body's linear damping is
zero because `00C37E00` is never called: `docs/RIGID_BODY_INTEGRATION.md`'s follow-up
`ship_hull_body_creation` is still open, so the mass, the inertia and both damping rates keep the
values a freshly constructed body has. Each tick therefore adds thrust along the new heading and
nothing removes the old velocity, and the magnitude integrates upward.

The 490-step run is short enough that the effect is still small (`Kortenaer` is 24.5 s in, with
the drift angle passing 40 degrees), which is why its distance is close to what a ship at 9 m/s
would cover. The effect is a property of the hull, not of the order: the twelve cruise ships hold
an exactly straight course at 12.0 m/s and are untouched by it.

That is also why nothing finishes a command through the AI here. `009E5821`'s
`state->vtable[2Ch]` is recovered - both navigation vtables hold `009DAB10`, three instructions
into `009DA590`, projected as `bsp::ship_ai_path_arrival_009da590` - and `009EF034` now runs. But
`009EF034` needs the ship **stopped and not releasing the stop**, and the release test at
`009EEF14` is `record+21h && blk+3D8h + setback < blk+330h`. `blk+3D8h`, the start radius, comes
from `FUN_009E4330` and is zero here, so `0 < 536` releases the stop on every tick and the latch is
never set:

```
summary mission ship ai arm tail bodies=3426 latched=1955 stops=0 arrival_latches=0
summary mission ship ai command completion events=0 callbacks=0 end_commands=0 queue_advances=0
```

The record has moved twice now: off `009DA590`, then off `009EF034`, and onto `FUN_009E4330`, the
per-ship tuning that fills `blk+3C8h`, `blk+3CCh`, `blk+3D4h`, `blk+3D8h` and `blk+604h`. That is
one unread routine standing between this executable and a `moveto` that ends itself.

### 8. A command does finish, and it goes the whole way round

The completion path is wired end to end and the director's own stage ladder exercises it. `0071D810`
is monotonic and stage 2 is what sends: `0071C730` builds `MT_GAMEUNIT_CLEARCMD` (`5Dh`) with
`+20h = 1` and `+24h = 0`, and because this is a local session the process that sent it is the
process that receives it, so `00721A40`'s `5Dh` arm runs here and takes the `00721BB7` branch into
`00720850`. `00720850` copies slot 0 into the previous-command record, shifts the queue down,
snaps the empty slot to the target's position, and calls `vtable[6Ch]`, whose head `0071C130`
clears the acceptance byte and the stage. Both of `0071D810`'s call sites route, not just the one
the AI reaches.

```
summary mission director completion end_commands=0 stage_raises=2 clear_messages=2
        clear_receives=2 queue_advances=2 restarts=0 command_events=0 event_callbacks=0

  command finished: DeRuyter cleared `moveto` from slot 0 (mode 1); the queue now holds `(none)`
                    and 0071c130 left the stage pair at 0
  command finished: DeRuyter cleared `cruise` from slot 0 (mode 1); the queue now holds `(none)`
                    and 0071c130 left the stage pair at 0
```

Both are `DeRuyter`, and the rule is `00836941`'s: a queue head at stage 1 on a **player-driven**
unit goes to stage 2 with no other condition. The standing re-issue then runs in the same step and
installs `cruise`, because `unit+184h` is set - which is what relatches `DeRuyter`'s authored
throttle after the advance and is why the run reports 14 latched commands where milestone 2p
reported 13. Lengthening the run to 1200 frames adds nothing: the same two completions, and no
third.

What the standing re-issue installs for everyone else is unchanged: **13 `stop`, 1 `cruise`, 0
`follow`**. The choice at `00836E59` reads `*(unit+73Ch)+28h`, the commanded-speed timestamp, and
the `StartSpeed` seed does not touch that pair - `0082352B..008235FB` has no store to
`[unit+73Ch]` - so a seeded ship whose queue empties still gets `stop`, not `cruise`.

The Lua side runs and finds nothing. `00984300` builds its four parameters and looks the channel
named `command` up on the mission event director's map; `009843D8` returns at once when the channel
holds no subscription. `0097E360`, the parser that would put one there, is not reconstructed, and
this mission authors none: `usn_2_java.scn` has no event block and `usn_2_java.lua` registers no
`command` handler. The early return is the native's own, not a substitute for it.

### 9. `--ai-drive` stays for one state out of four

| state | ships | produces a desired throttle | moves in the run | what blocks it |
| --- | --- | --- | --- | --- |
| `cruise` | 13 | **yes, the authored `StartSpeed`** | yes, 12.0 m/s | - |
| `stop` | 12 | yes, and 0 is the answer | correctly not | - |
| `movetopos` | 1 | **yes, through the arm tail's latch and ceiling** | yes, 261.14 m | - |
| `attackmove` | 6 | yes, at `009F3635`, and it is 0 | no | `009E76D0` with `009E5E90` and the four slot scorers. Packet `cc_ai_ring_scan` landed on `main` at `6a9c557b` during this packet's turn and is **not wired here**: `009E6A90`, the routine that turns its commanded heading into the commanded throttle, is only partially projected, so wiring the scan alone would feed a complete consumer from an incomplete producer |

The switch is **kept**, for `attackmove` alone. Milestone 2p needed it for two states out of four
and reported the other two as producing zero; three of the four now produce their own values and
two of them move a hull.

### Host methods

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
--mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java --order-unit
Kortenaer --order-frame 5 --trajectory-csv local/traj_2q_all.csv --log local/run_2q_all.log
--game-root "<install>"`, exit 0: **571 concrete, 483 unimplemented**, against milestone 2p's
**545 / 470** for the same command line on this branch's base commit `f0012d01`. With
`--ai-drive Kortenaer=1,0.5`: **570 / 481**. With no `--order`: **556 / 476**.

`reports/game_executable_milestone_2q.json` carries the per-step table with the call site and
callee of every row (`start_speed_steps`, `path_refresh_steps`, `search_steps`,
`path_point_steps`, `nav_steps`, `completion_steps`, `routines`, `measurements`,
`probe_comparison`, `corrections`, `ai_drive`, `read_for_this_packet`, `still_unimplemented`,
`names_added`, `evidence_appended`, `no_ghidra_function`). In call order, with the containing
function where it is not the packet's main routine:

| Step | Call site | Callee | Disposition |
| --- | --- | --- | --- |
| the unit's init slot (in 00925f20) | 00926110 | vtable +0A0h = 00822c20 | indirect, 32 bodies |
| find `ShipYardLaunch` (in 00822c20) | 00823576 | 008f2260 | concrete |
| find `StartSpeed` (in 00822c20) | 00823590 | 008f2260 | concrete, 32 finds, 14 non-zero |
| the reference speed (in 00822c20) | 008235ba / 008235dc | 0080fc30 | concrete, twice per unit |
| seed the ring throttle (in 00822c20) | 008235d5 | 0080d9b0 | concrete |
| seed the hull velocity (in 00822c20) | 008235f7 | 0092d770 | concrete, 12.0 m/s |
| the plan refresh | 009ee5c2 | 009ed3e0 | concrete, arm 009ed4e4 projected |
| the corridor widths (in 009ed3e0) | 009ed4ae / 009ed4d1 | 009d9de0 | concrete, always false |
| the fresh seed (in 009ed3e0) | 009ed523 | 009e3780 | concrete |
| the back-plan revalidation (in 009ed3e0) | 009ed588 | 009e3780 | concrete |
| the Failed reset (in 009ed3e0) | 009ed558 | 009d9d40 | record, 0 calls |
| the search on the back plan (in 009ed3e0) | 009ed59b | 009ec680 | concrete |
| the live revalidation (in 009ed3e0) | 009ed5fd | 009e3780 | concrete, 3397 accepts |
| the search on the live plan (in 009ed3e0) | 009ed614 | 009ec680 | concrete |
| the re-seed (in 009ed3e0) | 009ed692 | 009e3780 | concrete |
| the turn ramp (in 009ec280) | 009ec30b | 00424c40 | record, three zeroes |
| the avoid-zone singleton (in 009e3040) | 009e3058 | 004218e0 | record |
| the segment test (in 009e3040) | 009e3075 | 00417e90 | record, `not blocked` |
| the detour corners (in 009e3040) | 009e3105 | 00422500 | record, 0 calls |
| the world-bounds test (in 009e3040) | 009e3189 | 0071c4f0 | record, 0 calls |
| the node allocation (in 009e3040) | 009e3263 | 00bf681b | record, 0 calls |
| the corner record (in 009d58f0) | 009d5917 | 00417610 | record, 0 calls |
| the corner metric (in 009d58f0) | 009d5923 | 00423190 | record, 0 calls |
| the node release (in 009e3330) | 009e340d | vtable +0h | indirect, 0 calls |
| the path point | 009ee5f4 | 009e3c00 | concrete, 3419 points |
| the lateral offset (in 009e3c00) | 009e3dcd | 00811d80 | record, 0 calls |
| the class turn radius (in 009e3c00) | 009e3eae | 0082e850 | record |
| the owner radius (in 009e3c00) | 009e3ec0 | field +9C8h | record, discarded by 009e42bd |
| the corner arm (in 009e3c00) | 009e3f1a | - | record, 0 entries |
| the output block | - | 009ee671 | concrete, 3426 bodies |
| the remaining path length (in 009ed6b0) | 009ee6ac | 009d9e50 | concrete |
| the unit heading (in 009ed6b0) | 009ee8c7 | vtable +50h | indirect, 3419 calls |
| the navigation arm tail | - | 009eeaab | concrete, 3426 bodies, 1955 latched |
| the corridor normalise (in 009ed6b0) | 009eeb63 | 004192e0 | concrete, 0 calls |
| the neighbour list (in 009ed6b0) | 009eeb8b | field read | record, count 0 |
| the list indexer (in 009ed6b0) | 009eebb0 | 009dbbc0 | record, 0 calls |
| the neighbour position (in 009ed6b0) | 009eec41 | 00427eb0 | record, 0 calls |
| the approach ceiling (in 009ed6b0) | 009eecdb | - | concrete, clamps to 1.0 |
| the tail's own heading (in 009ed6b0) | 009ef0d6 | vtable +50h | indirect, concrete |
| the tail's turn radius (in 009ed6b0) | 009ef112 | 0082e850 | record |
| the per-ship tuning | - | 009e4330 | record, five fields |
| the arm's last step (in 009ed6b0) | 009ef213 | 009de5b0 | record |
| the arrival test (in 009e5770) | 009e5821 | vtable +2Ch = 009da590 | concrete, always false |
| the arrival latch (in 009ed6b0) | 009ef034 | - | concrete, 0 sets |
| the per-tick arrival clear (in 009ed6b0) | 009ed779 | - | concrete |
| the `finished` event (in 009e5770) | 009e595c | 00984300 | concrete, 0 bodies |
| end the command (in 009e5770) | 009e5997 | 0071e430 | concrete, 0 bodies |
| end the command (in 009e8820) | 009e88c1 | 0071e430 | concrete, 0 bodies |
| raise the queue stage (in 0071e430) | 0071e463 | 0071d810 | concrete |
| raise the override stage (in 0071e430) | 0071e480 | 0071d9e0 | record, no override mode |
| build the clear message (in 0071d810) | 0071d852 | 0071c730 | concrete, 2 messages |
| receive it and clear the slot (in 00721a40) | 00721bb7 | 00720850 | concrete, 2 advances |
| the class name (in 00720850) | 00720889 | vtable +4h | indirect |
| the target resolve (in 00720850) | 00720916 | 00521ea0 | concrete |
| the observer unregister (in 00720850) | 0072092a | 006952a0 | record |
| the stage reset (in 00720850) | 00720b56 | vtable +6Ch = 00835bf0 -> 0071c130 | indirect, concrete |
| the `command` subscription (in 0097e360) | 0097e63a | 0097c8a0 | record, no block authored |

### Corrections

1. **Milestone 2p's "neither the block nor the bearing runs" reads `009EE609` backwards.**
   `JZ 009EE671` jumps *into* the output block; `record+18h` gates only the band computation and
   the lateral publish `009EE66C`. Evidence: `009ee609`, `009ee671`, `009ee66c`, and the run's
   `output_blocks=3426` against `navigate_mode_steps=3426`.
2. **`docs/CRUISE_SPEED_SETTING.md`'s "every `Cruise` unit authors `StartSpeed`" conflates two
   counts.** Fourteen units author `StartSpeed`; thirteen author a `Cruise` inside a `"Command"`
   sub-block. `Kortenaer`'s Cruise line sits in its `"CamoColorGun"` sub-block and its `"Command"`
   block is empty. Evidence: `usn_2_java.scn` lines 1098..1120; a block-aware count gives
   `Command` 13 and `CamoColorGun` 1; with no `--order` the executable puts `Kortenaer` in `stop`.
3. **Milestone 2p section 7's `cruise | 13 | yes, and it is 0 for every ship: nothing ordered them
   to move` is superseded.** Something did order them, and the executable was not reading it. The
   thirteen cruise ships latch 0.657..0.741 and hold 12.0 m/s. Evidence: the run's start-speed
   table, `009e1265`, `00835e17`.
4. **Milestone 2p's acceptance-form numbers no longer reproduce, and the reason is the seed.**
   `--order-frame 1 --order throttle=1,rudder=1` now gives `heading -45.318523, fwd 16.422674,
   yaw -0.061087` at t = 14 s where milestone 2j published `-41.252617, 16.429752, -0.061087`. The
   controlled `DeRuyter` starts at 12.0 m/s instead of at rest, reaches the saturated yaw rate
   sooner and has turned further. `bsp_ship_motion_probe.exe --class 20 --steps 290 --dt 0.05
   --throttle 1 --rudder 1` prints `-41.253 / 16.4298 / -0.06109` at t = 14.00 from rest, which is
   milestone 2j's row to every printed digit: the old figure was the from-rest answer of this same
   motion model, not a different one.
5. **Milestone 2p's `no_ghidra_function` row for `009E8820..009E88F9` is closed.** The integrator
   defined the function; `python tools/bsp.py ghidra proto 009e88c1 --brief` reports
   `BSP_ShipAi_AttackMoveStateStep` with that body, and this milestone's attackmove call site
   verifies against it.
6. **Milestone 2p's `the plan request - 3426 requests` counted refreshes, not requests.**
   `009ED3E0` has four `009E3780` call sites and which one runs depends on `nav+2FCh` and on the
   back plan's state. The same 3426 refreshes now make 3421 search ticks, 12 seeds and 3397
   accepted revalidations. Evidence: `009ed523`, `009ed588`, `009ed5fd`, `009ed692`.
7. **Milestone 2p's follow-up 1 overstates what the arm tail forms.** It says the range "is
   the only code that can latch `blk+35Ch` and form a desired throttle for a ship in `Navigate`".
   It latches `blk+35Ch` and writes the throttle **ceiling** `blk+344h`; it never writes `blk+1D0h`
   or `blk+1D4h`. `009F3F80` forms the ring command from the latch, the ceiling and the heading
   target. `docs/SHIP_AI_NAVIGATION_ARM_TAIL.md` carries the same correction from the other side.
8. **Milestone 2p's `movetopos | 1 | no | 009EEAAB..009EF228` row is closed, and a different row
   opens.** With the tail wired, `movetopos` does produce a desired throttle and the ship moves
   261.14 m on its own order. What it cannot do is finish: `blk+3D8h` is zero because
   `FUN_009E4330` is unread, so `009EEF14`'s release test passes on every tick and `009EF034` never
   sets the arrival latch.
9. **The path record's `+20h` carries the opposite of what its name says.** `009E3D65` and
   `009E419B` set the byte exactly when there is no node after the successor or the outgoing leg is
   under one unit - that is, on the final leg - and `009E3D88` and `009E3F88` clear it when a corner
   is being rounded; `009EE776`'s `SETZ` then makes `blk+338h` true when the byte is *clear*. So
   `more_path_20` in `include/bsp/ship_ai_goal_vector.hpp` marks the last leg and `last_leg_338` in
   `include/bsp/ship_ai_navigation.hpp` is set when the path continues. Nothing is mis-wired: this
   milestone passes the bit through unchanged. Both headers belong to other packets and are not
   edited here; the rename is follow-up 6.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 007b3dd0 | 007b3dd2 | the empty attackmove sub-state step, one `RET 4`; boundary defined by packet `cc_ai_attackmove_substates`, unchanged here and not touched by this milestone |

Every other address this milestone touched lies in a Ghidra function whose body range the bridge
reports, `009E8820` included. `python tools/verify_report_calls.py
reports/game_executable_milestone_2q.json` checks **43 call rows and reports 0 failures**; the
vtable slots, field reads and unprojected spans are reported as indirect and skipped.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings. The existing ctest case
`reconstructed_math` passes, 1 of 1. No test cases were added. `origin/main` was merged in twice:
at `f0012d01` before any change was made, which brought in `docs/SHIP_AI_PATH_PLANNER.md`'s
corrections and the path-search packet, and at `c3bfb005` mid-packet, which brought in
`ship_ai_navigation_arm_tail` (`4491d04f`) and `cc_ai_ring_scan` (`6a9c557b`). The arm tail is
wired and validated here; the ring scan is **not** wired, and section 9 says why.

The full `scripts/build.ps1` could not run to completion: the drive holding every worker's
worktree was down to 0.86 GB free and MSBuild failed with `MSB3191` / `MSB3491` on its `.tlog`
directories across every project. Deleting and re-creating `build/win32` did not help. The three
targets this milestone needs were built one at a time through the same generator and toolchain the
script configures (`cmake --build build/win32 --config Release --target bsp_game`, then
`bsp_math_tests` and `bsp_ship_motion_probe`), each exit 0 with no warnings, and `ctest -C Release`
was then run against the same build directory. That is a machine-state problem, not a code one, and
it is reported here rather than hidden.

```
bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
  --mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java
  --order-unit Kortenaer --order-frame 5 --trajectory-csv local/traj_2q_all.csv
  --log local/run_2q_all.log --game-root "<install>"

summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5048
        state_steps{concrete=4803 records=245} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
summary mission ship ai plan requests=3426 seeds=12 accepts=3397 approach_frames=588
        controller_updates=15680
summary mission ship ai path search ticks=3420 swaps=13 points=3419 corner_arms=0
        units_with_point=7 output_blocks=3426 bearings=3419
summary mission ship ai command completion events=0 callbacks=0 end_commands=0 queue_advances=0
summary mission ship ai arm tail bodies=3426 latched=1955 stops=0 arrival_latches=0
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=15190
        deadbands=9212 live_pair_changes=43 driven=0
summary mission commands units=32 resolved=14 issued=21 pushed=35 current=35 latched=14
        with_thrust=14 ai_groups=0 ai_forwards=0 steps=2940
summary mission director steps=15680 idle_reissues=14 stop=13 cruise=1 follow=0
        script_issues=7 blocked_at_00816f7c=0 commanded_speeds=0
summary mission director completion end_commands=0 stage_raises=2 clear_messages=2
        clear_receives=2 queue_advances=2 restarts=0 command_events=0 event_callbacks=0
summary mission start speed seeds=32 non_zero=14 key=StartSpeed
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680
        simulated=24.50 s controlled=DeRuyter moved=34.80 total_path=3825.35
host methods 571 concrete, 483 unimplemented

  unit          StartSpeed  reference   ratio    axial   expected    moved    delta
  DeRuyter         12.0000    16.4622  0.7289  12.0000     294.00    34.80  -259.20
  Java             12.0000    16.4622  0.7289  12.0000     294.00   294.05     0.05
  Kortenaer        12.0000    18.2628  0.6571  12.0000     294.00   261.14   -32.86
  Electra          12.0000    18.2628  0.6571  12.0000     294.00   294.05     0.05
  Houston          12.0000    16.7194  0.7177  12.0000     294.00   294.05     0.05
  Exeter           12.0000    16.5908  0.7233  12.0000     294.00   294.05     0.05
  Perth            12.0000    16.2050  0.7405  12.0000     294.00   294.05     0.05
  (Alden, John1, John2, John3, Encounter, Jupiter, Witte: 0.6571, 294.05, 0.05)

  unit          state       searches  state  nodes  swaps  points    point_x    point_z  nav_dist  nav_hdg
  Kortenaer     movetopos        480      7      0      7     485     -250.0    -2724.6    536.00  -2.4473
  Haguro        attackmove       490      7      0      1     489      250.0    -3000.0   3250.00   3.1417
  Jintsu        attackmove       490      7      0      1     489     -250.0    -3000.0   3250.00   3.1415

  unit          state        hops  writes deadband slot_thr slot_rud live_thr live_rud livechg
  Kortenaer     movetopos     490     490        0   0.5000   1.0000   0.5000   1.0000      20
  Java          cruise        490     490        0   0.7289  -0.0000   0.7289  -0.0000       1
  Haguro        attackmove    490     490      490   0.0000   0.0000   0.0000   0.0000       0
```

The `--mission-frames 1200` run (59.5 s, `local/run_2q_long.log`) reports the same host-method
counts, `search ticks=8303 swaps=30 points=8319 corner_arms=0 output_blocks=8326 bearings=8319`,
`arm tail bodies=8326 latched=4755 stops=0 arrival_latches=0`, the same two completions and
`total_path=9554.63`, with the twelve holding ships at 714.12 m against 713.99 expected and
`Kortenaer` at 844.50 m. A 3000-step run (149.5 s, `local/run_2q_conv.log`) is what section 7's
drift table comes from; it adds no completion and no arrival latch.

Against `bsp_ship_motion_probe.exe --class 20 --steps 490 --dt 0.05 --throttle 0.7289426
--rudder 0` - `Java`'s own class and its own seeded ratio - through
`python tools/motion_trace_compare.py --trace local/traj_2q_java.csv --probe
local/probe_2q_class20.txt --align-origin`:

| channel | peak &#124;delta&#124; | final delta |
| --- | --- | --- |
| speed (m/s) | 11.8650 | 0.0000 |
| heading (deg) | 0.0000 | 0.0000 |
| position (m) | 24.3489 | 24.3489 |
| yaw (rad/s) | 0.00000 | - |

The whole difference is the probe starting from rest: the first divergence is at t = 0.05 s with
the trace at 12.0000 and the probe at 0.1350, and the probe needs 4 s at `MaxAccel` 3.0 to reach
12.0, losing 24.35 m on the way. Over the steady span alone (both sides trimmed to t >= 4.5 s)
**speed and heading agree to every printed digit**, peaks 0.0000 and 0.0000, and the position delta
is a constant 30.30 m set by aligning on samples 0.05 s apart, moving 0.04 m over 20 s: the two
trajectories are parallel, not drifting. That is the regression this comparison is for, and it is
the first time a cruise ship of this mission has had a straight run to compare.

Every earlier switch was rechecked on this binary. A 120 frame run with `--press-start-frame 30`
and no `--menu-select` exits 0 and reports **155 concrete and 79 unimplemented**, a 40 frame
title-only run reports **130 and 48**, `--vfs-probe fonts/fonts.lua` exits 0 and `--vfs-probe
does/not/exist.lua` exits 3: all four match milestones 2o and 2p exactly. A `--mission-frames 60`
run with no `--mission-complete-frame` still ends on the frame count with
`summary mission exit reachable=0`. The one switch whose published numbers moved is the acceptance
form, and correction 4 shows the move is the `StartSpeed` seed rather than a regression.

This remains a runtime-validated process, not a game-validated one. What it proves that milestone
2p did not is that thirteen ships of this mission move under the speed the mission author gave
them, through the routine the game uses and in the order the game runs it; that a navigating ship's
plan reaches Ready in the four ticks the search predicts and hands back a real path point; that the
navigation arm's output block runs and writes a real heading target and a real distance; that the
arm's tail turns those into a latched direction and a throttle ceiling; that a `movetopos` ship
then moves 261.14 m on its own order with the diagnostic switch off; and that a finished command
makes the whole round trip through the session message and comes back out as a queue advance and a
standing re-issue. What it does not prove is that a ship so moved goes where it was sent.

The milestone's two cautionary results are both in section 7. The first is the hull: giving a ship
a sustained turn for the first time exposed that the rigid body has no lateral resistance, and a
turning ship's speed integrates upward while its forward-axis speed does not. That is not an AI
result and it would have been easy to read the growing distance as success. The second is the
arrival latch: `009DA590` and `009EF034` are both recovered and both run, and the byte is still
never set, because the release test compares against a start radius `FUN_009E4330` would have
written. Running two complete routines over an input their producer never wrote is exactly what
milestone 2p's section 5 warned about, and the honest answer is that they run and answer no.

### Follow-up packets

1. **`ship_hull_body_creation`**, `00C37F40`, `00C37E70`, `00C37E00` and `00C37DE0`, and whatever
   fills the mass, the inertia and the two damping rates. Promoted to first place by section 7: it
   is why the first ship this executable has ever steered accelerates sideways instead of turning.
   `docs/RIGID_BODY_INTEGRATION.md` already carries it as a follow-up; what is new is a run that
   shows the cost.
2. **`ship_ai_nav_tuning_009e4330`**, `FUN_009E4330` with `009E4537`, `009E453F`, `009E4568`,
   `009E4659` and `0082E960`. Five fields - `blk+3C8h`, `blk+3CCh`, `blk+3D4h`, `blk+3D8h` and
   `blk+604h` - and every one of them is an input the arm tail reads and nothing writes.
   `blk+3D8h` alone decides whether a `moveto` can ever end itself, and `blk+604h` decides whether
   the traffic setback walk runs at all. `docs/SHIP_AI_NAVIGATION_ARM_TAIL.md` lists it too.
3. **`game_executable_ring_scan`**, wiring `009E76D0` (packet `cc_ai_ring_scan`, on `main` at
   `6a9c557b`) into `009F3090`'s two call sites. It needs `009E6A90` finished first: only
   `009E6A90`'s head and one tail helper are projected, and it is the routine that turns the scan's
   commanded heading at `nested+120Ch` into the commanded throttle `009F3635` forwards. Wiring the
   scan without it would feed a complete consumer from an incomplete producer.
4. **`ship_ai_path_follower_corner_arm`**, `009E3F1A..009E4222` of `009E3C00`, with `009D6550`,
   `00417EF0` and `00811D80`. The arm that rounds a corner instead of steering at the node. It is
   unreachable on an open sea and becomes reachable the moment `avoid_zone_geometry` lands, because
   a detour node has links on both sides.
5. **`ship_ai_order_consumer_corridor`**, `009ED3E0..009ED4E2` with `00778890`, `0070D400` and
   `0070D5D0`. The two corridor widths, and the only thing that can invalidate a plan mid-search.
6. **`avoid_zone_geometry`**, `004179D0`, `00422500`, `004218E0` and the manager itself. Everything
   the search's zone side needs; until it lands, `00417E90` answers `not blocked` and every plan is
   a straight line.
7. **`ship_ai_arm_final_step_009de5b0`**, `009DE5B0`, `0B67h` bytes, the unconditional last step of
   every arm of `009ED6B0`. The executable now reaches it on every navigation tick and records it.
8. **`ship_ai_path_record_20h`**, the rename in `include/bsp/ship_ai_goal_vector.hpp` and
   `include/bsp/ship_ai_navigation.hpp` that correction 9 asks for. Both headers belong to other
   packets.
9. **`mission_command_event_block`**, `0097E360` and `0097C8A0`, the parser and the subscription.
   `00984300` runs now and has nowhere to deliver; a mission that authors a `command` event is what
   would show the other half.
10. **`ship_class_turn_fields`**, `00828F20` and `class+520h`, the turn radius `0082E850` returns.
    Read by the corner arm, by the arm tail's astern test, by `00836920` and by three sub-state
    steps, and written by nothing this process runs.
11. **`construct_world` 004de610**, unchanged from milestones 2h through 2p, and now also the
    producer of the world entity list `[[00E188A8]+19CCh]` the arm tail's traffic walk needs.

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

## Correction from docs/DIRECTOR_TARGET_GATE.md

Packet `cc2_director_target_gate` (main fe8ae5ae) read `0071DF70` and its writers whole and
corrects milestone 2n's section 5 ("Nothing reaches `00835860`") in three places:

| what milestone 2n said | what is true | evidence |
| --- | --- | --- |
| the float at `[director+40h]` must be greater than the 0.0f at `00D7A218` | the compare runs the other way: `0071DF7C` is `JBE` to the slot scan and the fall-through at `0071DF7E` is `XOR AL,AL / RET`, so a hold above 0.0f is the rejection and at most 0.0f passes | `0071DF7C`, `0071DF7E` |
| `director+40h` has no writer in this process and no recovered producer anywhere | three writers: `00720225` stores -1.0f in the command controller base constructor `00720180` (`008366D0` skips the field, but its base `008363E0` calls `00720180` at `00836403` with ECX still the director; Ghidra's auto-name `CG_array_ctor_helper_00720180` hid it); `0071F314` in `0071F290` (controller vtable `+0Ch`, per frame) does `hold -= dt` while `hold >= 0`; `00817031` in the `cleartarget` arm of `00816E30` stores 3.0f. It is a three-second re-acquisition hold after `cleartarget` | `00720225`, `0071F314`, `00817031` |
| the predicate's neutral answer stops every think | the constructed -1.0f passes the gate, so the hold is not why no think reaches `00835860`. The remaining rule counts the leading occupied slots at `director+54h` (stride `1Ch`, stop at the first null) and rejects if any answers category 1 or 2 (gunnery / weapon-run, the set `008358D0` forces the fire target for) from its `vtable[0Ch]`. What the units hold in slot 0 when the think runs decides it, and needs a run of this executable; if slot 0 carries the authored `attackmove` of the Japanese force, category 2 blocks the automatic think by design and the fire target has to come through `008358D0`'s forced set | `docs/DIRECTOR_TARGET_GATE.md` |

`0071D980` is the override send, `__thiscall(director)(command, target)`, `RET 8`, acting only
while the hold is strictly negative; it builds `MT_GAMEUNIT_SETCMD` with flag byte 0 (installed
as an override through `0071E7F0` in receiver `00721A40` at `00721B36`) where `0071ECF0`
IssueCommand passes 1 (queue). Names: `BSP_WeaponDirector_SendOverrideCommand` `0071D980`,
`BSP_CommandControllerBase_Update` `0071F290`, `BSP_CommandControllerBase_Construct` `00720180`.
The executable's `AutoTarget::director_accepts_new_target` record (`src/game_hosts_ship_ai.cpp`)
still answers with the old reading; replacing it with the hold test and the slot category scan,
and reporting what slot 0 carries, is the next executable milestone's first item.

## Correction from docs/CRUISE_SPEED_SETTING.md

Packet `cc_cruise_speed_setting` (main 677c93cd) read usn_2_java.scn and the game-unit init slot
`0A0h` (`00822C20`, `BSP_UnitInstance_SEntityInit`, partial coverage) and corrects two counts and
one conclusion above: the shipped mission has **fourteen** `Cruise` units and **eighteen** empty
`Command` blocks, not thirteen and nineteen; and the scene record does carry a speed, on the unit
entity rather than in the `Command` sub-block: every `Cruise` unit authors `StartSpeed = F 12.0000`
(207 of the shipped .scn files use the key). `00822C20` reads it out of the entity's scene
property bag at init and seeds the order ring's live throttle at `ring+148h` with
`StartSpeed / 0080FC30(unit)` and the hull body's axial velocity with `0080FC30(unit)` times that
float32 ratio (`008235BF`, `008235BA`, `008235DC`, `008235C3`). The scene's `Cruise` is only
queued at that point, so when `00835E17` latches it the ring already holds the ratio and the
untouched rudder keeps the spawn heading. The executable's cruise ships stand still because its
unit creation skips that seed; wiring it is the next milestone's item.

## Milestone 2r: the hull the game builds, the navigation block's class values, and six attackmove ships steering on their own

Addresses: 00939e2a / 00937c90 with its tail 009399c0..00939c05, 009399d2 / 00414db0, 009399e5 /
00c336c0, 00939a6f / 00c5d580, 00939a89 / 00c31f90 and 00939c05 / 00c37e70; 00826a6d / 00937440
and its tail 00937622 / 009329c0, with 009329ec / 00c37e50, 00932a0e / 00c37e20, the material
index 00932a53..00932a8f and its eight loads 00932a96..00932b3f, and 00932d6b / 0078cf20;
0092d300's own store 0092d3ec..0092d40d and 0092d56c..0092d584; 009f118d / 009e4330 with
009e43cc / 009dfcb0, 009e44c4 and 009e4555 / 0082e960, 009e45f3 / 00bf7030, 009e465f / 00bd2f10
and 009e46a9 / 009e0270; 009ee5f4 / 009e3c00 with 009e3d8c, 009e3dcd / 00811d80, 009e3eae /
0082e850, 009e3ec0, 009e4200 / 004218e0 and 009e4216 / 00417ef0; 009f4d87 / 009ef910 with
009ef97c, 009efa08 / 009ec770, 009efcb8 / 0080e160, 009efd00 / 009d57e0, 009efd4a / 00415d70,
009efdbd / 009dd010, 009efe0f / 009d8860, 009efec3 / 009d8a30, 009f0000 / 00778890 and 009f0038 /
00811a30; 009f51f3 / 009e04e0 with 009e0509, 009e061b / 009da1d0, 009e0631 / 009dc060 and
009e0723 / 00811a30; 009f51e4 / 009f0ea0 and 009f1a25 / 009f0d20; 009ef334 / 009eb660 with
009eb681 and 009ebede / 00424c40, 009eb819 / 004158e0, 009eb8c0 / 009d8160, 009eb8ca / 009d80c0,
009eb8e2 / 009dd540, 009ebdaa / 00415970, 009ebe36 / 009dd010, 009ebf67 / 009d84e0 and 009ec0c1 /
009dc2e0; 009f309b / 009f1bc0, 009f30a2 / 009e7fc0, 009f30a9 / 009e6e80, 009f30b8 / 009e9190,
009f30c7 / 009e74d0, 009f30d6 / 009e76d0 and 009f30dd / 009e6a90, with 009e8080 / 009e6400,
009e81a7 / 009e5da0, 009e5dc4 / 0095eb40, 009e74b7 / 009e6870, 009e7755 / 009e6640 with 009e673e /
00417b10 and 009e6808 / 0041b4e0, 009e7ecb / 009e5e90, 009e6b85 / 009e5e00, and the two reads
009f3314 and 009f339a.

Packet `cc_exe_2r`, worker `agent/cc-exe-2r`. Sources: `src/game_hosts_units.cpp`,
`include/bsp/game_hosts_units.hpp`, `src/game_hosts_ship_ai.cpp`,
`include/bsp/game_hosts_ship_ai.hpp`, `src/game_hosts_lua.cpp`,
`include/bsp/game_hosts_lua.hpp`. Report: `reports/game_executable_milestone_2r.json`. No Ghidra
mutation. Eight packets are consumed from `main` and none of their reconstructions is re-derived
here; what this milestone adds is the wiring, the run and three corrections that the run forced.

### 1. The hull body the game builds, and the one thing it changes

`00939E2A` is the last call of the controller constructor `00939CB0`, and the tail of `00937C90`
at `009399C0..00939C05` is the writer of `controller+2Ch` (`docs/SHIP_HULL_BODY.md`). The
executable runs that tail now, once per created unit, so every hull carries the game's own
descriptor: the mass at `009399F7`, the zeroed inertia diagonal, the angular damping `1.0f` at
`00939A2F`, the row-1 torque lock below a mass of 100.0, `00C5D580`'s body and motion state, and
`00C37E70`'s box inertia at `00939C05`. Two stand-ins go with it. Milestone 2i's `1.0e30f` speed
clamps at `M+18h` and `M+1Ch` are replaced by the descriptor's own `1000.0f`, and the three class
keys that had been pinned at `0.0f` since milestone 2i - `Length` at `class+A0h`, `Height` at
`class+A8h` and `Mass` at `class+B0h` - are read out of the installed `VehicleClass` row, which
this process's own Lua reader had already been parsing two of and throwing away.

```
rigid body: 00c41550 then 00c5b1b0, one substep of the whole 0.0500 s game step. Milestone 2r
builds the hull body through the tail of 00937c90, so "DeRuyter" carries mass 7688.0 (inverse
0.00013007), angular damping 1.0, linear damping 0.0, both speed clamps at 1000.0, torque-lock=0
and physics material 0
```

The measurable effect is exactly one number. Each integration phase multiplies the angular
velocity by `1 - rate*dt` and both phases run, so a commanded yaw rate is scaled by
`(1 - 0.05)^2 = 0.9025` inside the step it is set in. The acceptance form says so to five
decimals: at `t = 14 s` the controlled `DeRuyter`'s yaw rate moves from milestone 2q's
`-0.061087` to `-0.055131`, and `bsp_ship_motion_probe.exe --class 20 --steps 290 --dt 0.05
--throttle 1 --rudder 1` prints `-0.05513` in its class-built-body column. That is the whole of
what the hull body does to this mission.

The collision AABB is the one input it cannot supply. Its producer is the shape attach `00C5C940`
behind `00937D3F..009399BF`, which no packet has read, so no shape is attached, the box span is
zero and `00C37E70` turns the zero inertia into a zero inverse inertia. Nothing in this process
applies a torque, so that decides nothing the run measures; it is the same default the probe takes.

### 2. The drift is not the hull body's, and the probe drifts too

Milestone 2q's section 7 read the sideways runaway as a missing hull body: "what the shipped game
has and this process does not is anything that resists the rest [of the velocity]. The rigid
body's linear damping is zero because `00C37E00` is never called". Both halves are wrong, and the
run says so.

The hull body's linear damping is `0.0f` **in the shipped game**. `docs/SHIP_HULL_BODY.md`
settles it: the descriptor default at `00939295` survives and nothing overrides `desc+60h`. And
`00C37E00` has two callers, neither on a live hull's path: `004462D0`
`BSP_GameDynamicsList_ApplyBuoyancyStep`, which walks the detached-part list that `00934150`
`BSP_UnitParts_DetachPart` and `00935540` add to, and `00824B60` `BSP_UnitInstance_OnWrecked`.
A hull never registers on that list.

`0092D300` is correct as reconstructed, and the drift is what it does. `0092D3EC`, `0092D3F4` and
`0092D40D` form the perpendicular residue `v - current*dir`, and after the `PUSH ECX` at
`0092D544` the three stores at `0092D56C`, `0092D578` and `0092D584` write `target*dir` plus that
residue. The along-axis component is replaced every step; the rest is carried forward. Under a
steady turn of `dtheta` per step the residue settles toward `-target * sin(dtheta) / (1 -
cos(dtheta))`, which is thousands of metres per second, so in practice it grows without bound.

The decisive evidence is the probe, which builds the class hull body and drifts **more** than the
executable does:

| run | drift at t = 150 s | trajectory speed | forward speed |
| --- | --- | --- | --- |
| probe, `--class 20 --throttle 1 --rudder 1` | 80.98 deg | 63.27 m/s | 10.56 m/s |
| probe, `--class 20 --moveto -250,-2724.6` | 73.57 deg | - | 16.39 m/s |
| probe, `--class 265 --moveto -250,-224.6` | 74.50 deg | 66.25 m/s | 18.15 m/s |
| `bsp_game.exe`, `Kortenaer`, before this milestone | 74.35 deg | 66.05 m/s | 18.18 m/s |
| `bsp_game.exe`, `Kortenaer`, after | 78.03 deg | 85.30 m/s | 18.16 m/s |

The probe prints the mechanism itself: `final forward speed 10.5647 (a sustained turn bleeds
speed: 0092D300 rewrites only the axial component, so the lateral one stays)`. Neither
`--moveto` run converges: the class-20 one never gets nearer than 2401 m to a goal 2736 m away,
and the class-265 one, on `Kortenaer`'s own class with `Kortenaer`'s own 336 m goal, ends 1978 m
away having never improved on its starting distance.

**The routine is `009329C0` `BSP_UnitController_ApplyHydroForces`**, body `009329C0-00933BA9`,
the tail call of `00937440` at `00937622`. It is the only code on a live hull's path that writes
the body's linear velocity outside `0092D300`: `009329EC` calls `00C37E50` (set linear velocity)
and `00932A0E` calls `00C37E20` (set angular velocity). It selects a physics material the same way
`00937CF1` does - `class+B0h` against the double `100.0` at `00D7A220` (`00932A53..00932A7E`),
scaled to a `38h` stride by the `LEA`/`SUB`/`ADD` chain at `00932A82..00932A8F` - and then loads
eight floats out of `settings + 4E0h + material*38h` at `00932A96`, `00932AAD`, `00932AC4`,
`00932ADB`, `00932AF2`, `00932B09`, `00932B20` and `00932B3F`. Those are the six record fields
`docs/SHIP_HULL_BODY.md`'s `ship_physics_material_record` follow-up lists as "the readers of
`KozegellenallasiEgyutthato*`", the Hungarian for a medium-resistance coefficient. It also samples
the water height at `00932D6B`. That is a hydrodynamic drag model reading drag coefficients out of
the record this milestone's hull body already selects, and it is unreconstructed. Until it exists,
a turning ship keeps every metre per second of lateral velocity it acquires, in this executable
and in the probe alike.

The drift is worse after this milestone, not better, and the reason is section 5: the clearance
wiring lifts the danger level from 1.0 to 0 and the throttle with it, so `Kortenaer` goes faster
and therefore drifts faster. That is a real result and it is reported as one.

### 3. The navigation block, and what the five fields were worth

`009F118D` runs `009E4330` on the brain record's inline sub-object at `brain+8h`, once per ship.
Nothing in this process builds a brain record, so the executable runs the constructor once per
created unit at registration, which is the only moment it can. Packet `cc_ai_nav_block_ctor`
supplies the body; six host methods supply its call sites, of which `009DFCB0`, `0082E960` twice
and `00BF7030` are concrete and `00BD2F10` and `009E0270` are records with their own addresses.

The values reproduce `docs/SHIP_AI_NAV_BLOCK_CTOR.md`'s published table to every printed digit,
which is the check that the wiring hands the constructor the class row it expects:

```
  unit                 state        len_9c8  turn_3c8  turn_3cc   yaw_3d0  stop_3d4 start_3d8    mass_b0  mat
  DeRuyter             cruise        171.00    269.49    256.02   0.12217     68.40    102.60     7688.0    0
  Kortenaer            movetopos     110.00    261.60    248.52   0.13963     44.00     66.00     1800.0    0
  Haguro               attackmove    203.00    353.71    336.02   0.10472     81.20    121.80    13500.0    0
```

`Kortenaer`'s start radius `blk+3D8h` is 66.0 m where milestone 2q had 0.0, so `009EEF14`'s
release test `record+21h && blk+3D8h + setback < blk+330h` is a real comparison at last. The
arrival latch is still never set, and section 6 says why.

### 4. The path follower, whole

`009EE5F4` used to run milestone 2q's partial projection of `009E3C00`, which stopped at the
corner arm. Packet `cc_ai_path_follower` read `009E3C00-009E432A` whole, so the corner arm
`009E3F1A..009E4222`, the shortcut clearance test and the cursor advance at `009E421F` are code
now. Two of its host methods that milestone 2q recorded are produced values: `0082E850` answers
`class+520h`, whose writer `00828F20` packet `ship_ai_class_field_0524` read, and `unit+9C8h`
answers the full hull `Length` its producers `0081106E` and `0081FA4D` copy from the descriptor
when the class carries no model box.

The run confirms `docs/SHIP_AI_PATH_FOLLOWER.md`'s two-node prediction exactly. Over 500 frames:
**3419 points, 0 corner arms, 0 cursor advances, 0 lateral publishes**. On the open sea the plan
is `head -> goal_node`, the walk does not run, the target is the goal node, the outgoing leg is
zero length so the corner gate fails at `009E3F54`, and `009E41CF` clamps the step floor to the
distance to the goal, which `009E42BF` brings back to exactly that. The published point is the
goal.

### 5. The clearance producer, and the danger level that was never real

`009F4D87` is an unconditional call from the publish into `009EF910`, one chain slot before the
drive whose danger ramp at `009F4168` divides `blk+37Ch` by `unit+9CCh`. Milestone 2q recorded it
and left `blk+37Ch` at the zero a fresh block carries, which made the ramp's ratio zero and pinned
**every navigating ship at danger 1.0** - and with it at `009EC7C0`'s `ThrustMin_Slow` of 0.5 and
at a turn-assist load latch of 1.5.

Run the routine and the field carries the `9999.0f` sentinel `009EF96F` seeds, because nothing
lowers it: the neighbour list `blk+604h` is the zero `009E4659` wrote and nothing appends to it,
and both avoid-zone gates are the zeroes `009E4401` wrote. The ratio saturates and the danger
target is 0.

| column | before | after |
| --- | --- | --- |
| `blk+37Ch` clearance | 0.0 | 9999.0 |
| `blk+0A84h` danger, every navigating ship | 1.000 | 0.000 |
| `unit+102Ch` turn-assist load | 1.500 | 0.000 |
| rudder deadbands over 500 frames | 9212 | 6272 |
| ring live-pair changes | 43 | 1760 |

The throttle profile `009E04E0` at `009F51F3` and the obstacle sector scan `009EB660` at
`009EF334` replace their records beside it, with the neighbour ageing pass `009F0EA0` at
`009F51E4` one slot ahead of the scan so the list is compacted first. Both run over empty inputs
and both say so: the contact-track list at `blk+400h` is the empty one `009E4653` leaves, so the
profile keeps the bypass byte `009E435F` set and `009D6B40` stays a clamp; the neighbour list is
empty and both zone gates are shut, so **47040 sector scans produce 0 marks** and `009D84E0`, the
passing corner, is wired at `009EBF67` and never reached. The one thing that would change either
is `004DE610` `construct_world`: the candidate walk at `009F1877..009F18B8` that feeds
`009F0D20` reads the world object's entity list, and this process has no world object.

### 6. A `moveto` that cannot end, for a reason that is no longer the AI's

With the start radius produced, the release test is real, and the latch is still never set over
500, 1200 and 3000 frames. The reason has moved off the AI entirely.

`--order moveto:Java` names a **ship**, and the goal vector is re-read from that ship's position
every frame. `Java` holds 12.0 m/s on its authored `Cruise`. `Kortenaer` makes 9.1 m/s, because
`009EC7C0`'s `ThrustMin_Slow` floor is 0.5 and its heading error never falls inside the
`HdgDiffValueMin_Slow` window - and its heading error never falls because the hull is travelling
80 degrees off its own bow. So the remaining path length grows:

```
  ship ai step 10   Kortenaer  state=movetopos  ... d32c=   561.54 d330=   561.54
  ship ai step 90   Kortenaer  state=movetopos  ... d32c=   595.50 d330=   595.50
```

and reaches 3045 m over 3000 steps. A goal that outruns the chaser cannot be reached, and the
chaser is slow because of section 2. What this milestone can say is that nothing in the AI is
missing for an arrival any more: the field the test compares against is produced, the latch's
writer `009EF034` runs, `009DA590` behind `state->vtable[2Ch]` runs, and the completion path
`0071E430 -> 0071D810 -> 0071C730 -> 00721BB7 -> 00720850` is wired end to end and still carries
the director's own two `DeRuyter` completions, unchanged from milestone 2q.

`Kortenaer` ends the 500-frame run 518.5 m from `Java`, having started 559.0 m away and moved
258.16 m.

### 7. Six attackmove ships steer on their own, and `--ai-drive` is retired

All six of `009F3090`'s arms that milestone 2q recorded now run: `009E7FC0` at `009F30A2`,
`009E6E80` at `009F30A9`, `009E9190` at `009F30B8`, `009E74D0` at `009F30C7`, the ring scan
`009E76D0` at `009F30D6` and the throttle limiter `009E6A90` at `009F30DD`. The four slot scorers
behind them are packet `cc_ai_ring_scan`'s whole-body projections: `009E6400` at `009E8080`,
`009E5DA0` at `009E81A7`, `009E6870` at `009E74B7` and `009E6640` at `009E7755`. `009E5E90` at
`009E7ECB` writes the commanded heading `nested+120Ch`, and `009E6A90` reads it back and writes
the commanded throttle `nested+1210h`, which is what `009F3240` forwards and `009F3635` clamps.

Milestone 2q's reason for keeping the switch was that `009E6A90` "is only partially projected".
It is not: `docs/SHIP_AI_APPROACH_UPDATE.md` line 299 and its routine table both record
`009E6A90-009E6E78` as complete, and `docs/SHIP_AI_NAV_BLOCK_CTOR.md` re-checked it. That is
correction 4 below.

```
  unit                 state      ring_scan  bearings  winner   hdg_120c   thr_1210 clear_37c  profiles substate
  Haguro               attackmove        98        98       0    -0.5235     0.5000    9999.0       490        8
  Jintsu               attackmove        98        98       0     0.5235     0.5000    9999.0       490        8
  Yudachi              attackmove        98        98       0    -0.5235     0.5000    9999.0       490        8
  Samidare             attackmove        98        98       0     0.5235     0.5000    9999.0       490        8
  Murasame             attackmove        98        98       0    -0.3633     0.5000    9999.0       490        8
  Harusame             attackmove        98        98       0     0.3633     0.5000    9999.0       490        8
```

Every one of the six ends in the approach sub-state, member offset `8h`, `009F3240`:
`set_current_substate` ran six times, once per ship, and all 588 sub-state steps went there. The
winner is slot 0 for all six, because `009E6640`'s probe-space callees `00417B10` and `0041B4E0`
are records so no slot is ever blocked and every slot scores 1.0; what separates the six is
`009E5E90`'s own commit, which gives `Murasame` and `Harusame` a different bearing from the other
four.

They move, and they close:

| ship | moved, 500 frames | closing on `Kortenaer`, 500 frames | closing, 1200 frames |
| --- | --- | --- | --- |
| `Haguro` | 205.15 m | -458.4 m | -877.9 m |
| `Jintsu` | 212.29 m | -469.4 m | -1085.4 m |
| `Yudachi` | 201.19 m | -452.8 m | -770.6 m |
| `Samidare` | 201.19 m | -457.6 m | -1033.2 m |
| `Murasame` | 202.79 m | -436.7 m | -662.2 m |
| `Harusame` | 202.79 m | -447.6 m | -1215.4 m |

Milestone 2q reported all six at 0.00 m. **`--ai-drive` is retired**: the switch is still accepted
on the command line and ignored, with a log line saying so, and the run that passes it reports the
same **602 concrete and 493 unimplemented** as the run that does not.

One arm is wired and not reached, and it is named rather than counted as working. `009E5DA0` at
`009E81A7` sits inside `009E7FC0`'s **mode-0** arm, and every attackmove ship of this mission is
in mode 1 (`mode=1` in the run's approach table), so neither it nor `0095EB40` behind it at
`009E5DC4` is entered. `firepower=0` in the summary is that gate, not a missing wiring; and if the
arm did run, the rating would be 0 anyway, because `0095EB40` walks gunnery device lists at
`unit+394h` / `+398h` / `+430h` that this process does not build.

### Host methods

`bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
--mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java --order-unit
Kortenaer --order-frame 5 --trajectory-csv local/traj_2r_all.csv --log local/run_2r_all.log
--game-root "<install>"`, exit 0: **602 concrete, 493 unimplemented**, against the same line's
milestone 2q baseline of **571 / 483** taken on this branch's merged base. With no `--order`:
**587 / 487**. With `--ai-drive Kortenaer=1,0.5`: **602 / 493**, identical to the run without it,
which is what retiring it means.

`reports/game_executable_milestone_2r.json` carries the per-step table with the call site and
callee of every row (`hull_body_steps`, `nav_block_steps`, `path_follower_steps`,
`clearance_steps`, `throttle_profile_steps`, `sector_scan_steps`, `attackmove_steps`, `routines`,
`measurements`, `probe_comparison`, `corrections`, `ai_drive`, `read_for_this_packet`,
`still_unimplemented`, `evidence_appended`, `no_ghidra_function`, `consumed_from_main`,
`coverage`). In call order, with the containing function where it is not the packet's main
routine:

| Step | Call site | Callee | Disposition |
| --- | --- | --- | --- |
| the hull body build (in 00939cb0) | 00939e2a | 00937c90 | concrete, 32 bodies |
| refresh the unit matrix (in 00937c90) | 009399d2 | 00414db0 | record, not needed |
| copy the transform (in 00937c90) | 009399e5 | 00c336c0 | concrete |
| create the body (in 00937c90) | 00939a6f | 00c5d580 | concrete |
| read back the AABB (in 00937c90) | 00939a89 | 00c31f90 | record, no shape attached |
| set the box inertia (in 00937c90) | 00939c05 | 00c37e70 | concrete, zero tensor |
| the force model (in 00825f20) | 00826a6d | vtable +0h = 00937440 | indirect, concrete |
| **the hydrodynamics (in 00937440)** | **00937622** | **009329c0** | **record: the routine this milestone names** |
| the navigation block (in 009f1160) | 009f118d | 009e4330 | concrete, 32 blocks |
| the steering seed (in 009e4330) | 009e43cc | 009dfcb0 | concrete, window only |
| the turn circle at full (in 009e4330) | 009e44c4 | 0082e960 | concrete |
| the turn circle at 0.9 (in 009e4330) | 009e4555 | 0082e960 | concrete |
| the square root (in 009e4330) | 009e45f3 | 00bf7030 | concrete |
| the random phase (in 009e4330) | 009e465f | 00bd2f10 | record, no stream |
| the sector shapes (in 009e4330) | 009e46a9 | 009e0270 | record |
| the path point | 009ee5f4 | 009e3c00 | concrete, 3419 points, whole body |
| the lateral anchor (in 009e3c00) | 009e3d8c | field +10h | record, none on this plan |
| the order turn limit (in 009e3c00) | 009e3dcd | 00811d80 | record, unreachable arm |
| the class turn radius (in 009e3c00) | 009e3eae | 0082e850 | concrete |
| the owner length (in 009e3c00) | 009e3ec0 | field +9c8h | concrete |
| the avoid-zone manager (in 009e3c00) | 009e4200 | 004218e0 | record |
| the shortcut test (in 009e3c00) | 009e4216 | 00417ef0 | record |
| the turn clearance (in 009f4d10) | 009f4d87 | 009ef910 | concrete, 15680 bodies |
| the category gate (in 009ef910) | 009efa08 | 009ec770 | record |
| the avoidance flag (in 009ef910) | 009efcb8 | 0080e160 | record |
| the static zone block (in 009ef910) | 009efd00 | 009d57e0 | record |
| the static zone clearance (in 009ef910) | 009efd4a | 00415d70 | record |
| the neighbour sweep (in 009ef910) | 009efdbd | 009dd010 | record, list empty |
| the neighbour support (in 009ef910) | 009efe0f | 009d8860 | record, list empty |
| the neighbour closest (in 009ef910) | 009efec3 | 009d8a30 | record, list empty |
| the path fade gate (in 009ef910) | 009f0000 | 00778890 | record |
| the class length unit (in 009ef910) | 009f0038 | 00811a30 | concrete |
| the throttle profile (in 009f50e0) | 009f51f3 | 009e04e0 | concrete, 15680 bodies |
| the avoidance predicate (in 009e04e0) | 009e061b | 009da1d0 | record |
| the track refresh (in 009e04e0) | 009e0631 | 009dc060 | record, list empty |
| the gap length (in 009e04e0) | 009e0723 | 00811a30 | concrete |
| the neighbour ageing (in 009f50e0) | 009f51e4 | 009f0ea0 | concrete, list empty |
| the neighbour append (in 009f1420) | 009f1a25 | 009f0d20 | record, no world list |
| the sector scan (in 009ef230) | 009ef334 | 009eb660 | concrete, 47040 scans, 0 marks |
| the blocked margin (in 009eb660) | 009eb681 | 00424c40 | record |
| the zone segment crossing (in 009eb660) | 009eb819 | 004158e0 | record |
| the avoid-box test (in 009eb660) | 009eb8c0 | 009d8160 | record, list empty |
| the near-box test (in 009eb660) | 009eb8ca | 009d80c0 | record, list empty |
| the ray clip (in 009eb660) | 009eb8e2 | 009dd540 | record, list empty |
| the arc clip against zones (in 009eb660) | 009ebdaa | 00415970 | record |
| the arc clip against a node (in 009eb660) | 009ebe36 | 009dd010 | record, list empty |
| the passing corner (in 009eb660) | 009ebf67 | 009d84e0 | record, wired, not reached |
| the neighbour memory (in 009eb660) | 009ebede | 00424c40 | record |
| the free bearing (in 009eb660) | 009ec0c1 | 009dc2e0 | record |
| the frame state (in 009f3090) | 009f309b | 009f1bc0 | concrete, 588 frames |
| the score reset (in 009f3090) | 009f30a2 | 009e7fc0 | concrete |
| the bearing decay (in 009e7fc0) | 009e8080 | 009e6400 | concrete |
| the class rating (in 009e7fc0) | 009e81a7 | 009e5da0 | record, mode-0 arm not entered |
| the expected damage (in 009e5da0) | 009e5dc4 | 0095eb40 | record, behind the row above |
| the standoff range (in 009f3090) | 009f30a9 | 009e6e80 | concrete |
| the arc score (in 009e6e80) | 009e74b7 | 009e6870 | concrete |
| the avoidance refresh (in 009f3090) | 009f30b8 | 009e9190 | concrete |
| the candidate list (in 009e9190) | 009e9220 | 008053c0 | record, no world list |
| the evade score (in 009f3090) | 009f30c7 | 009e74d0 | concrete |
| **the ring scan (in 009f3090)** | **009f30d6** | **009e76d0** | **concrete, 588 scans** |
| the obstacle probe (in 009e76d0) | 009e7755 | 009e6640 | concrete |
| the probe origin (in 009e6640) | 009e673e | 00417b10 | record, no probe space |
| the probe cast (in 009e6640) | 009e6808 | 0041b4e0 | record, never hits |
| the bearing commit (in 009e76d0) | 009e7ecb | 009e5e90 | concrete, 588 commits |
| the throttle limiter (in 009f3090) | 009f30dd | 009e6a90 | concrete |
| the engagement target (in 009e6a90) | 009e6b85 | 009e5e00 | concrete |
| the commanded heading (in 009f3240) | 009f3314 | field +1214h | concrete, produced now |
| the commanded throttle (in 009f3240) | 009f339a | field +1218h | concrete, produced now |

### Corrections

1. **Milestone 2q section 7's diagnosis of the drift is wrong in both halves.** It says the
   shipped game has something that resists the lateral velocity and that the missing thing is the
   hull body's linear damping, "because `00C37E00` is never called". The hull body's linear
   damping is `0.0f` in the shipped game (`docs/SHIP_HULL_BODY.md`: the descriptor default at
   `00939295` survives, nothing overrides `desc+60h`), and `00C37E00`'s two callers, `004462D0`
   and `00824B60`, are the detached-part buoyancy step and the wreck handler, neither on a live
   hull's path. Evidence: `python tools/bsp.py ghidra callers 00c37e00`; `00447510`'s three
   callers, all part-detach sites; the run's own log line reporting linear damping 0.0 after the
   body is built; and the probe, which builds that body and drifts to 80.98 degrees at 150 s.

2. **The probe does not hold its heading-aligned velocity.** This packet's brief and the reading
   of milestone 2q behind it both take the probe as the converging reference. It is not one:
   under a sustained hard-over turn it reaches 80.98 degrees of drift at 150 s with a trajectory
   speed of 63.27 m/s against a forward speed of 10.56, and neither of its two `--moveto` runs
   closes on its goal. Its own printed summary states the mechanism: `a sustained turn bleeds
   speed: 0092D300 rewrites only the axial component, so the lateral one stays`. Evidence:
   `local/probe_2r_turn.txt`, `local/probe_2r_class20_moveto.txt`,
   `local/probe_2r_class265_moveto.txt`.

3. **Milestone 2q's correction 8 is half closed, and its remainder is not the AI's.** `blk+3D8h`
   is 66.0 m for `Kortenaer` now and `009EEF14` is a real comparison, but the latch is still never
   set because `moveto:Java` names a ship under way at 12.0 m/s that a 9.1 m/s chaser cannot
   catch: `blk+330h` rises from 561.54 m at step 10 to 595.50 m at step 90 and to 3045 m over
   3000 steps. Evidence: the run's nav-block table; the ship-AI sample lines; `arrival_latches=0`
   at 500, 1200 and 3000 frames; the trajectory CSV's `Java` rows.

4. **Milestone 2q section 9's reason for keeping `--ai-drive` rests on a stale coverage claim.**
   It says `009E6A90` "is only partially projected". `docs/SHIP_AI_APPROACH_UPDATE.md` line 299
   and its routine table line 358 both record `009E6A90-009E6E78` as complete, and
   `docs/SHIP_AI_NAV_BLOCK_CTOR.md`'s own section re-checked it against the listing. Evidence:
   those three doc sections; `local/run_2r_aidrive.log` reporting the same 602 / 493 as
   `local/run_2r_all.log`.

5. **Milestone 2q's host rows `the class turn radius | 009e3eae | 0082e850 | record` and
   `the owner radius | 009e3ec0 | field +9C8h | record` are both produced values now.** `0082E850`
   returns `class+520h`, whose writer `00828F20` packet `ship_ai_class_field_0524` read, and
   `unit+9C8h` is the full hull `Length` that `0081106E` and `0081FA4D` copy from the descriptor
   when the class carries no model box - correction 3 of `docs/SHIP_AI_NAV_BLOCK_CTOR.md`.
   Evidence: the run's `len_9c8` column, 171.00 / 110.00 / 203.00, matching the installed
   `vehicleclasses.lua` rows exactly.

6. **`src/game_hosts_units.cpp`'s milestone 2i note that only `class+A0h` has a recovered Lua key
   is superseded, and two of the three keys were already being parsed and discarded.**
   `GameVehicleClassRow` had `length` and `mass` fields filled by `read_vehicle_class_row` since
   milestone 2i; the units host overwrote all three with `0.0f`. `Height` is added to the reader
   here. `VehicleClass[20]` carries `Length = 171`, `Height = 5` and `Mass = 7688`. The keel
   sample point now sits half a hull length astern and 2.5 m below the pose, which leaves
   `00826994`'s gate open as before. Evidence: the installed `vehicleclasses.lua`; the run's log
   line reporting mass 7688.0 and inverse 0.00013007.

7. **Milestone 2q's `the per-ship tuning | - | 009e4330 | record, five fields` row, and its
   section 7's "one unread routine standing between this executable and a `moveto` that ends
   itself", understate what was in the way.** Two more things were: the clearance producer
   `009EF910`, whose absent `blk+37Ch` pinned every navigating ship at danger 1.0 and at the
   AutoThrust slow floor of 0.5, and the order itself, which names a moving goal. Evidence: the
   danger column, 1.000 before and 0.000 after; `arrival_latches` still 0 with `blk+3D8h`
   produced.

8. **`docs/SHIP_AI_RING_SCAN.md`'s reading of `009E5DA0`'s reachability needs a run-time
   qualifier.** The adapter and `0095EB40` behind it are complete and wired, and neither is
   entered on this mission: `009E81A7` sits in `009E7FC0`'s mode-0 arm and every attackmove ship
   here is in mode 1. Evidence: the run's approach table `mode=1`; `firepower=0` in the summary
   beside `ring_scans=588`.

### no_ghidra_function

| Start | End (inclusive) | Note |
| --- | --- | --- |
| 007b3dd0 | 007b3dd2 | the empty attackmove sub-state step, one `RET 4`; boundary defined by packet `cc_ai_attackmove_substates`, unchanged here and not touched by this milestone |

Every other address this milestone touched lies in a Ghidra function whose body range the bridge
reports. `python tools/verify_report_calls.py reports/game_executable_milestone_2r.json` checks
**65 call rows and reports 0 failures**; the one vtable slot (`00826A6D`, the force model) is
reported as indirect and skipped, and the field reads are marked as such.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings, every target built.
`ctest -C Release`: `reconstructed_math` passes, 1 of 1. **No test cases were added.**
`origin/main` was merged in twice: at `a6a17386` before any change was made, and again at
`fcf8b17a` after `docs/COORDINATION.md` and the checklist were read, which brought in
`ship_ai_nav_block_ctor` (`3b4e07a6`). The baseline below was taken on that merged tree before the
first edit.

```
bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
  --mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java
  --order-unit Kortenaer --order-frame 5 --trajectory-csv local/traj_2r_all.csv
  --log local/run_2r_all.log --game-root "<install>"

summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5048
        state_steps{concrete=4803 records=245} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
summary mission ship ai plan requests=3426 seeds=13 accepts=3394 approach_frames=588
        controller_updates=15680
summary mission ship ai path search ticks=3420 swaps=13 points=3419 corner_arms=0
        units_with_point=7 output_blocks=3426 bearings=3419
summary mission ship ai nav blocks=32 (009e4330 once per brain record, 009f118d)
        clearance=15680 throttle_profiles=15680 sector_scans=47040 sector_marks=0
        ring_scans=588 ring_bearings=588 firepower=0 follower_points=3419
        follower_corners=0 follower_advances=0
summary mission ship ai command completion events=0 callbacks=0 end_commands=0 queue_advances=0
summary mission ship ai arm tail bodies=3426 latched=3425 stops=6 arrival_latches=0
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=15190
        deadbands=6272 live_pair_changes=1760 driven=0
summary mission director completion end_commands=0 stage_raises=2 clear_messages=2
        clear_receives=2 queue_advances=2 restarts=0 command_events=0 event_callbacks=0
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680
        simulated=24.50 s controlled=DeRuyter moved=34.80 total_path=5047.64
host methods 602 concrete, 493 unimplemented

  unit          StartSpeed  reference   ratio    axial   expected    moved    delta
  DeRuyter         12.0000    16.4622  0.7289  12.0000     294.00    34.80  -259.20
  Java             12.0000    16.4622  0.7289  12.0000     294.00   294.05     0.05
  Kortenaer        12.0000    18.2628  0.6571  12.0000     294.00   258.16   -35.84
  (the other eleven holding cruise ships: 294.05, delta 0.05)
  (the six attackmove ships, 0.00 in milestone 2q: 205.15, 212.29, 201.19, 201.19,
   202.79, 202.79)
```

Against milestone 2q's own numbers on the same line: `arm tail latched` 1955 -> 3425, `stops`
0 -> 6, `rudder deadbands` 9212 -> 6272, `live_pair_changes` 43 -> 1760, `total_path` 3825.35 ->
5047.64, and `danger` 1.000 -> 0.000 for every navigating ship. The counts that do not move are
the ones that should not: 3426 arm-tail bodies, 3426 output blocks, 15680 ring hops, 15680
publishes and promotions, the same two director completions, and `states cruise=13 stop=12
attackmove=6 movetopos=1`.

The `--mission-frames 1200` run (60.0 s, `local/run_2r_long.log`) reports **597 concrete, 484
unimplemented**, `search ticks=8367 swaps=36 points=8389 corner_arms=0 output_blocks=8396`, `arm
tail bodies=8396 latched=8395 stops=6 arrival_latches=0`, `ring_scans=1440 sector_scans=115200
sector_marks=0`, the same two director completions and `total_path=12925.23`. A 3000-frame run
(150.0 s, `local/run_2r_conv.log`) is what section 2's drift table comes from; it adds no arrival
latch.

Trajectory captures: `local/traj_2r_all.csv` and `local/traj_2r_conv.csv` carry every ship,
`Kortenaer` and `Haguro` included. The probe comparisons are in section 2 and in
`reports/game_executable_milestone_2r.json`'s `probe_comparison`;
`python tools/motion_trace_compare.py --trace local/trace_2r_kortenaer.csv --probe
local/probe_2r_class265_moveto.txt --align-origin` reports peak deltas of 12.97 m/s, 52.29 degrees
and 1125.32 m, dominated by the probe starting from rest against the trace's seeded 12.0 m/s and
by the two sides' different throttle sources. What it establishes is that both sides drift.

Every earlier switch was rechecked on this binary. A 120-frame run with `--press-start-frame 30`
and no `--menu-select` exits 0 and reports **155 concrete and 79 unimplemented**; a 40-frame
title-only run reports **130 and 48**; `--vfs-probe fonts/fonts.lua` exits 0 and
`--vfs-probe does/not/exist.lua` exits 3. All four match milestones 2o, 2p and 2q exactly. The one
published figure that moves is the acceptance form, and correction 1 of section 1 shows the move
is the hull body's angular damping rather than a regression.

This remains a runtime-validated process, not a game-validated one. What it proves that milestone
2q did not: that a ship's rigid body is the one the game's own constructor builds, with its mass,
its material, its torque lock and both damping rates; that the navigation block carries the five
class-derived fields milestone 2q reported as unwritten, to the digit the reading packet
published; that the path follower runs whole and answers the goal node on an open-sea plan; that
the clearance producer runs and takes every navigating ship off full danger; that the obstacle
sector scan and the throttle profile run rather than being recorded; and that six attackmove ships
choose a bearing through a 60-slot ring scan, turn it into a commanded heading and a commanded
throttle, and close between 437 and 469 m on their target in 24.5 s with every diagnostic switch
off. What it does not prove is that any of them goes where it was sent, and section 2 names the
one routine that stands between this process and that.

The milestone's cautionary result is section 2's. Building the hull body was the obvious reading
of milestone 2q's section 7 and it does not fix the drift; the probe that section 7 held up as the
converging reference drifts harder than the executable does. Two runs of a reconstruction that was
never wrong, over an input whose producer is a 4.6 KB unread routine, looked like a bug in the
consumer. That is the same mistake milestone 2q's own closing paragraph warns about, made one
layer up.

### Follow-up packets

1. **`ship_hydro_forces_009329c0`**, `009329C0-00933BA9`, with `0042B260`, `004F9B30`, `0074F930`,
   `00C33650`, `00C35330`, `00C35360`, `00C37E20` and `00C37E50`. The hull's hydrodynamics: the
   drag that removes a turning ship's lateral velocity, the buoyancy that cancels the world's
   gravity, and the reader of the physics-material record this milestone's hull body selects. It
   is the single largest thing standing between this executable and a ship that goes where it is
   sent, and nothing else in the ship AI is now waiting on anything.

2. **`ship_physics_material_record`**, `settings+4E0h + i*38h` and `0083FEE7..008403B7`. The six
   fields of the record whose keys `docs/SHIP_HULL_BODY.md` did not read, and the
   `KozegellenallasiEgyutthato*` coefficients `009329C0` loads at `00932A96..00932B3F`. Follow-up
   1 needs them.

3. **`ship_hull_shapes`**, `00937D3F..009399BF`, `00C5C940`, and the `fizika_%02d` / `hajobelso`
   node walks. Which model nodes become collision shapes, and therefore the AABB the hull's box
   inertia is built from. Carried over unchanged from `docs/SHIP_HULL_BODY.md`; this milestone
   supplies everything else the inertia needs.

4. **`construct_world_004de610`**, unchanged from milestones 2h through 2q and now with three more
   consumers. The world object's entity list at `[[00E188A8]+19CCh]` is what `009F1877..009F18B8`
   walks to fill the neighbour list at `blk+608h`, which is the input the sector scan, the
   clearance sweep and the arm tail's traffic setback are all waiting on. Wiring it would let a
   sector be marked, a clearance be lowered and `009D84E0` be reached.

5. **`ship_ai_moveto_static_goal`**, a diagnostic packet rather than a reconstruction: give
   `--order moveto` a fixed point instead of a unit so the arrival latch can be exercised at all.
   Section 6 shows the AI has nothing missing for an arrival and that the order is what makes one
   impossible on this mission.

6. **`gameplay_settings_ship_ai_block`**, `settings+194h`, `+1D4h`, `+1D8h`, `+214h`, `+218h`,
   `+588h` and the approach tuning block behind `009E7FC0` / `009E6E80` / `009E74D0` / `009E9190`.
   Six of this milestone's records are reads of that one singleton, whose recovered loader
   `0083B5E0` fills only the rudder curve and the AutoThrust sub-table.

7. **`ship_ai_nav_block_seed_defaults`**, `009DFCB0` whole. Carried over from
   `docs/SHIP_AI_NAV_BLOCK_CTOR.md`: only the `blk+3C4h..+3E4h` window is projected, and the rest
   seeds `blk+1C4h..+3C3h`, which includes the shoulders `blk+18Ch..+1B0h` that the clearance
   sweep and the sector probe place their pivots from and that this milestone fills from the hull
   pose with `009DE2F0` recorded.

8. **`ship_gunnery_device_lists`**, `unit+394h` / `+398h` / `+430h` and the walk `0095EBB3`. The
   inventory `0095EB40` rates. Without it the expected-damage rating is 0 whatever bearing it is
   asked about, which is why `009E5DA0` would answer 0 even if `009E7FC0`'s mode-0 arm ran.

## Milestone 2s: a ship goes where it is sent

Addresses: 00826a6d / 00937440 with 00937613 / 00c35330 and 00937622 / 009329c0, and inside
009329c0 the twelve host sites 009329ec / 00c37e50, 00932a0e / 00c37e20, 00932a16, 00932bb0 /
00c31f40, 00932be6 / 00c31f20, 00932c21 / 00c32000, 00932c28 / 00c33650, 00932d6b / 0078cf20,
00933a01 / 0074f930, 00933a52 / 0074f2e0, 00933b01 / 00c35360 and 00933b38 / 00c35330, plus the
unit vtable slot 5Ch at 00932a42 and 00932e2f; 004ddb90's world descriptor and 00c41ad0's copy of
it; the world registry 006fe620 with 006fe623 / 00928560, 00928567 / 00484540 and the five further
00484540 sites 006fe62f, 006fe63b, 006fe647, 006fe653 and 006fe65f, the slot 00cfc3d0+130h they
implement, and the reader 009f1877..009f1a44 inside 009f1420; 0088a810's Vector3 branch through
issue_command_object; and the finish arm of 009e5770 at 009e580f / 009de050, 009e5821 /
009dab10 -> 009da590, 009e58ef / 0041e870, 009e595c / 00984300, 009e5997 / 0071e430 and 009e599e /
009e00a0.

Packet `cc_exe_2s`, worker `agent/cc-exe-2s`. Sources: `src/game_hosts_units.cpp`,
`include/bsp/game_hosts_units.hpp`, `src/game_hosts.cpp` and `src/game_main.cpp`.
Report: `reports/game_executable_milestone_2s.json`.
Ghidra was read-only: no name added, no comment written, no snapshot taken. Every descriptive
name here is a hypothesis, not a recovered symbol.

Milestone 2r ended by naming `009329C0` as "the single largest thing standing between this
executable and a ship that goes where it is sent". Packet `cc_hydro_forces` reconstructed it.
This milestone runs it, and the answer is yes.

### 1. The hydrodynamics, and a hull that stops sliding

`009329C0` runs where the native runs it. `00826A6D` inside the motion tick `00825F20` calls
`00937440`, and `00937440`'s last call, `00937622`, is `009329C0` with the same `dt`. It is a
`CALL` and not a jump: `00937618 FLD dword ptr [ESP+40h]`, `0093761C PUSH ECX`,
`0093761D MOV ECX,EDI`, `0093761F FSTP dword ptr [ESP]`, then the call, then the epilogue
`POP EDI` / `ADD ESP,38h` / `RET 4` at `00937627`. So the hydrodynamics happen inside the motion
tick and **ahead of `0092D300`'s velocity rewrite**, which is why the drag is formed from the
velocity the hull actually had rather than from the one the throttle law is about to impose.

What it stages reaches the body through `00C35360` `AddForce` and `00C35330` `AddTorque`, and the
velocity phase `00C41550` at the end of the same step integrates it. Eleven of the twelve host
methods are calls; the one record is the unit's own vtable slot `5Ch`, whose body no packet has
read, and the host table below says so.

Kortenaer, `--order moveto:Java --order-unit Kortenaer`, 3000 mission frames at 0.05 s:

| | before | after |
| --- | --- | --- |
| drift angle at t = 150 s | 77.7231 deg | 0.0692 deg |
| trajectory speed at t = 150 s | 85.4021 m/s | 18.2533 m/s |
| forward speed at t = 150 s | 18.1600 m/s | 18.2537 m/s |
| peak drift over the run | 85.2854 deg at 132.00 s | 1.4046 deg at 54.95 s |
| position at t = 150 s | (2079.32, -3114.38) | (-124.66, -1909.46) |

The drift angle is the angle in the horizontal plane between one step's own displacement and the
hull's forward axis; the trajectory speed is that displacement over the step. Before, the hull
travelled at 85 m/s along a line 78 degrees off its own bow while making 18 m/s through the water.
After, the trajectory speed and the forward speed agree to four figures and the bow points where
the ship is going. That is the whole of what the drag does. The `before` column is this packet's
own baseline, taken on the merged tree before the first edit; milestone 2r published 78.03 degrees
and 85.30 m/s for the same ship on the same order, and the small difference is the merges between
the two, not a disagreement.

The cruise ships that hold a straight line move slightly **less** than before: 293.81 m over 24.5 s
against 294.05. The forward drag term at `0093324E` is the reason, and it is linear only, because
`KozegellenallasiEgyutthatoNElore` at `+4FCh` is never read.

### 2. The world's gravity, which only balances with the buoyancy

Milestone 2r left `bsp::DynWorldStepConstants` at zero on the reading that "none of them has a
recovered producer". They have one. `004DDB90` builds the 40h-byte world descriptor on its own
stack and `00C41AD0` copies fifteen fields of it into the world; packet `dyn_world_settings` read
both whole and `docs/DYN_WORLD_SETTINGS.md` carries them. Gravity is `(0, -10, 0)` from the double
at `00CE6848` and both sleep speeds are the zero `004DE1C7` / `004DE1CD` store, so no body ever
sleeps.

Gravity is switched on here and not in any earlier milestone because it only balances once
`009329C0` runs. The buoyancy the element list produces at the hull's draft is exactly `mass * 10`,
and `10` is the literal double at `00CE3DC0` that the drag scale uses, equal in value to the world
gravity magnitude. The run reports the balance: `DeRuyter`'s staged force on its first flush is
`(0.00, 76880.00, -922.56)` and its class `Mass` is 7688, so the Y component is `mass * 10` to the
digit and the hull's Y never moves. Turning gravity on without the buoyancy would sink every ship,
and the probe shows exactly that: body B's final Y is `-99999.4844` without `--hydro` and `0.0000`
with it.

### 3. The buoyancy element list is a stand-in, and every figure above is a figure about it

`009329C0` walks the vehicle class's buoyancy elements at `class+52Ch..+530h`. **Nothing in the
exported set writes `class+528h..+534h`**, so `docs/SHIP_HYDRO_FORCES.md` carries the four field
roles as a hypothesis reconciled between the routine's only two readers, and this milestone cannot
do better. The list built here is the one `bsp_ship_motion_probe.exe --hydro` builds: eight
elements spread evenly along the class `Length` in the hull's own `Y = 0` plane, a draft of half
the class `Height`, and a shared coefficient solved so the hull displaces its own weight at that
draft. A class row with no `Length` or `Height` gets no list and the step is skipped rather than
run on invented numbers.

The two sides therefore use the same list, which is what makes the comparison in section 5 mean
anything, and neither side is the game's own list.

**That question has since been answered, after every run in this milestone was taken.** Packet
`ship_buoyancy_elements` landed on `main` at `c6de1e9a` with `docs/SHIP_BUOYANCY_ELEMENTS.md` and
`include/bsp/ship_buoyancy_elements.hpp`: the list is generated by `0082D040` from the model's
`deckline` and `bottomline` nodes through the class descriptor's model-binding virtual `0082FE30`,
its vector base is **`descriptor+528h` and not `+52Ch`**, and the coefficient is solved so the list
sums to `10 * Mass`, which is the same total this stand-in solves for. Adopting the generator is
follow-up 3 below and is a rewiring of one block in `create_units`, not a change to anything else
in this milestone. Until then the list here is the probe's, and every figure in sections 1 and 5 is
a figure about it.

### 4. `00937440` applies its own rudder torque

Milestone 2r said of that torque that "nothing applies it, because the hydrodynamic tail `009329c0`
and the rigid-body solver are external". `00937440` applies it itself.
`python tools/bsp.py ghidra xrefs 00c35330` reports three call sites and one of them is
`From 00937613 in BSP_UnitController_ApplyShipForces`, nine instructions before the call into
`009329C0`; `python tools/bsp.py ghidra callees 00937440` lists `Dyn_Body_AddTorque` among seven.
It is applied here now, in that order, and under the same gate: `00937449 CMP byte ptr [EAX+5Dh],0`
and `0093744D JNZ 00937618` skip the whole torque block and land one instruction before the
hydrodynamic call, so the hydrodynamics run either way and only the torque is conditional. The
argument is the float3 at `ESP+34h..+3Ch` whose address `00937598`'s `LEA` takes, each component
divided by the double `10000.0` at `00CE4BD8`.

It moves nothing, for two reasons that are worth separating. The vector it carries is the zero
vector, because the gain is multiplied by `settings+588h` and that field has no recovered producer.
And the inverse inertia is zero anyway, because the collision AABB's producer `00C5C940` is unread
and `00C37E70` stored a zero, which is also why `009329C0`'s own torque of about 3.0e6 about Y does
nothing. Packet `ship_hull_shapes` owns the AABB and `gameplay_settings_ship_ai_block` owns
`settings+588h`.

### 5. The probe agrees, once both sides have the drag

`bsp_ship_motion_probe.exe --class 265 --hydro --moveto 0,1500 --steps 3000 --dt 0.05`, class 265
being Kortenaer's own, reports a peak drift of **1.3259 deg**; the same run without `--hydro`
reports **82.6785 deg**. The class-20 pair the packet brief names is **1.2196** against
**75.6212**. The executable's own Kortenaer peaks at **1.4046 deg**.

Over the manoeuvre the two sides share, `tools/motion_trace_compare.py` reports:

| channel | peak delta |
| --- | --- |
| heading | 0.6995 deg |
| position | 42.4569 m |
| yaw rate | 0.00413 rad/s |
| speed | 11.8590 m/s, the first step alone |

against milestone 2r's 52.29 degrees, 1125.32 m and 12.97 m/s on the same comparison. The window
ends at the executable's arrival, because the probe has no stop and the two part company after it.
The speed delta is entirely the first step: the executable's Kortenaer is seeded at 12.0 m/s by its
authored `StartSpeed` and the probe starts from rest.

### 6. A fixed point, and a `moveto` that ends

Milestone 2r's section 6 established that nothing in the AI was missing for an arrival and that the
order was what made one impossible: `moveto:Java` names a ship whose position the goal vector
re-reads every frame, and a goal that outruns the chaser cannot be reached.

`--order moveto=<x>,<z>` gives it a point instead, and `--order moveto:<x>,<z>` does the same
thing: a target token that parses as two numbers is read as a world XZ position either way. No
entity of any scene in this game has a comma in its name, so the point form and the named form
cannot be confused.

Milestone 2l refused every `--order <command>=<args>` on the reading that "`0046aab0` builds only
a named-target or owner-position descriptor and `00816e30`'s arm for such a command is not
projected". The first half is true and the second is beside the point, because the mission
script's own navigator bindings do not go through `0046AAB0` at all. The order goes out as
`kCommandObjectMoveTo` with the descriptor `0088A810`'s Vector3 branch builds: `kind` 0, the
`position_valid` byte set, the three floats, a null object. That is the same descriptor
`NavigatorMoveToPos` hands `0077D600` from a mission script, through the same
`issue_command_object` entry milestone 2m wired, so the order takes the game's own command builder
and not a new path. The refusal is narrowed rather than removed: a command with `=` must carry a
comma, and a named target still goes through the colon form and `0046AAB0`.

`--order moveto=0,-1000 --order-unit Kortenaer`, a point 1497.63 m ahead of the hull's own bow, over
2500 mission frames:

| | |
| --- | --- |
| remaining path when the latch fired | 41.33 m |
| time of the latch | 81.00 s, fixed step 1620 |
| distance from the point at rest | 5.00 m |
| distance travelled | 1495.04 m |
| `arrival_latches` | 1 |
| `command completion events` / `end_commands` / `queue_advances` | 1 / 1 / 1 |
| `director completion command_events` | 1 |

The whole finish arm of `009E5770` runs on that one step, in the listing's order: `009E5821`
through `state->vtable[2Ch]`, which both navigation vtables hold as `009DAB10` and which tail-jumps
to `009DA590`, answers true; `009E58EF` assigns the string `"finished"` at `00D09FD8`; `009E595C`
posts it through `00984300` on the event channel named `command`; `009E5997` calls `0071E430` with
the terminal argument **1**; `009E599E` calls `009E00A0`, and the ship-AI sample line for the same
step reads `mode=heading dir=stopped throttle= 0.000`.

The queue does not advance on that step, and `docs/COMMAND_COMPLETION.md` says why: raising the
stage to 2 sends `MT_GAMEUNIT_CLEARCMD` (`5Dh`) and the advance happens when that message is
**received**, in `00721A40`'s `5Dh` arm. The run shows the round trip as a ten-step gap: the state
is still `movetopos` at step 1620 and is `stop` at step 1630, which is `00836920`'s idle tail
re-issuing a standing command the moment the queue empties. The hull coasts the last 36 m under no
throttle at all.

The AutoThrust profile is visible in the approach: throttle 1.000 at 99.78 m of remaining path,
0.978 at 81.53, 0.872 at 72.70, 0.773 at 64.42, 0.680 at 56.68, 0.593 at 49.48, then zero.

### 7. The world's per-class unit lists, producer only

`[[00E188A8]+19CCh]` holds 97 `{count, head, tail}` triples at `registry+18h + id*0Ch`, built by
the vector-constructor iterator at `004CB076` inside `004CB030`. A created unit joins them through
its entity virtual slot `+130h`, and for this class family that slot holds `006FE620`, whose whole
body is six push-backs: `00928560` at `006FE623`, itself
`MOV ECX,[ECX+30h]; ADD ECX,24h; CALL 00484540`, which is id 1; then ids 2, 4, 5, 6 and 7 from the
`ADD ECX,0x30/0x48/0x54/0x60/0x6C` at `006FE62C`..`006FE65C`. `00484540` is the list primitive:
a `{prev, next, value}` node, the head at `list+4h` on the empty branch `00484586`, the tail at
`list+8h` otherwise, and `ADD dword ptr [ESI],1` on both.

Id 6 is the list the ship AI wants. `009F1877` inside `BSP_ShipAi_BrainPrePass` loads `[00E188A8]`,
then `+19CCh`, then the head at `+64h` and the count at `+60h`, and `0x60 == 0x18 + 6*0xC`, which
is the triple `006FE650`'s `ADD ECX,0x60` pushes onto. That settles which of the 97 lists the
neighbour candidate walk reads.

The lists are built and filled: 32 registrations, 192 push-backs, six lists of 32 each. **The
consumer is not wired**, because the candidate walk lives in `GameShipAiHost` and
`src/game_hosts_ship_ai.cpp` is leased to another owner for the whole of this packet's turn. What
is left is named in the follow-up section, and both rules it needs are already reconstructed.

The `+130h` dispatch site itself was **not located**. A byte scan of `.text` for
`call dword ptr [reg + 130h]` (`ff ?? 30 01 00 00`) finds nothing, and neither do the neighbouring
slots `12Ch` and `134h`, which return only `JMP rel32` false positives. `00CFC500`, which is
`00CFC3D0+130h`, is the only reference to `006FE620` in the image, so the slot is cited as data and
the host row carries it as `00cfc3d0+vtable130` rather than as a call site.

### 8. What blocked two of this milestone's four items

`agent/orch6-20260912` held `src/game_hosts_ship_ai.cpp` and `include/bsp/game_hosts_ship_ai.hpp`
for the whole of this turn, first as packet `orch6_game_avoid_zone_runtime` and then as
`orch6_navigation_runtime_d`. It also held `src/game_hosts.cpp` and `include/bsp/game_hosts.hpp`
as `orch6_shared_game_crt` for most of it; those two came free part way through and were claimed
and used, which is why section 6's `=` spelling exists.

What the ship AI host's lease cost:

* **The neighbour list**, milestone item 2. Producer delivered, consumer not. The candidate walk
  lives in `GameShipAiHost` and nothing else can reach the navigation block's list.
* **The `follow` and `land` state steps**, milestone item 4. Neither is wired. Neither state is
  entered on this mission in any case: `summary mission ship ai states` reports
  `cruise=13 stop=12 attackmove=6 movetopos=1 other=0` on every run of this milestone and
  `summary mission director steps ... follow=0`, so both would be records even once bound. Their
  slots are `00D215F8+0Ch` / `009E1610` and `00D21658+0Ch` / `009E1950`, and both reconstructions
  are complete and consumed from main.

### Host methods

In call order. `address` is the native call site and `native` the callee;
`reports/game_executable_milestone_2s.json` carries the same rows and
`tools/verify_report_calls.py` checks each against the live bodies.

| step | address | native | disposition |
| --- | --- | --- | --- |
| the force model | 00826a6d | 00937440 | concrete, from milestone 2i |
| its rudder torque | 00937613 | 00c35330 | concrete, new here |
| the hydrodynamics | 00937622 | 009329c0 | concrete, new here |
| the body's linear velocity | 00932bb0 | 00c31f40 | concrete |
| the body's angular velocity | 00932be6 | 00c31f20 | concrete |
| the body's world transform | 00932c28 | 00c33650 | concrete |
| the unit's category-8 answer | 00cfc3d0+vtable5c | unread | record, indirect |
| the water height | 00932d6b | 0078cf20 | concrete, over the same two record leaves |
| the leak tick | 00933a01 | 0074f930 | concrete, over an empty leak list |
| the leak heeling torque | 00933a52 | 0074f2e0 | concrete, the same empty list |
| AddForce | 00933b01 | 00c35360 | concrete |
| AddTorque | 00933b38 | 00c35330 | concrete, discarded by a zero inverse inertia |
| the disabled path's linear velocity | 009329ec | 00c37e50 | concrete, never reached |
| the disabled path's angular velocity | 00932a0e | 00c37e20 | concrete, never reached |
| the disabled path's gravity bit | 00932a16 | not a call | record |
| the world-list dispatch | 00cfc3d0+vtable130 | 006fe620 | record, indirect |
| the parent entity list, id 1 | 006fe623 | 00928560 | concrete |
| push id 1 | 00928567 | 00484540 | concrete |
| push id 2 | 006fe62f | 00484540 | concrete |
| push id 4 | 006fe63b | 00484540 | concrete |
| push id 5 | 006fe647 | 00484540 | concrete |
| push id 6 | 006fe653 | 00484540 | concrete |
| push id 7 | 006fe65f | 00484540 | concrete |
| the navigation goal | 009e580f | 009de050 | concrete, from milestone 2o |
| the arrival predicate | 009e5821 | 009dab10 | concrete, indirect through vtable +2Ch |
| the `"finished"` string | 009e58ef | 0041e870 | record |
| the `command` event | 009e595c | 00984300 | concrete |
| end command, terminal 1 | 009e5997 | 0071e430 | concrete |
| hold heading and stop | 009e599e | 009e00a0 | concrete |

### Corrections

1. **Milestone 2r's section 2 and its closing paragraph are superseded, not contradicted.** They
   said the hull body was built and the drift got worse, and that the probe held up as the
   converging reference drifts harder than the executable does. Both were true of a process with
   no `009329C0`. With it, both sides converge: Kortenaer's drift at 150 s falls from 77.72 degrees
   to 0.07 and the probe's peak from 82.68 to 1.33 on the same class. Evidence:
   `local/base_traj_conv.csv` against `local/v_traj_conv.csv`; `local/probe_2s_c265_plain.txt`
   against `local/probe_2s_c265_hydro.txt`.

2. **`src/game_hosts_units.cpp`'s note that the world step constants have no recovered producer is
   wrong.** `004DDB90` builds the descriptor and `00C41AD0` copies it; packet `dyn_world_settings`
   read both whole. Gravity is `(0, -10, 0)` from the double at `00CE6848`. Evidence:
   `docs/DYN_WORLD_SETTINGS.md`; `include/bsp/dyn_world_settings.hpp`'s `kDynWorldGravityY`; the
   run's own `gravity_y=-10.0`.

3. **`src/game_hosts_units.cpp`'s note that nothing applies `00937440`'s torque is wrong, and
   `docs/SHIP_HYDRO_FORCES.md`'s "tail call" is loose.** `00937440` calls `00C35330` itself at
   `00937613`, and `00937622` is a `CALL` followed by an epilogue, not a jump. Evidence:
   `python tools/bsp.py ghidra xrefs 00c35330`; `python tools/bsp.py ghidra callees 00937440`;
   `tools/verify_report_calls.py` rejected a `kind: tail_jump` row for `00937622` against the live
   listing.

4. **`src/ship_motion_probe.cpp` line 1387 prints a false statement in every `--hydro` run.** It
   says "B's y falls because `00C41550` adds the world's gravity and nothing here cancels it:
   `009329C0`'s buoyancy is not reconstructed." The buoyancy is reconstructed and it cancels
   gravity exactly: on the same class and the same 3000 steps, body B's final Y is `-99999.4844`
   without `--hydro` and `0.0000` with it. The note predates the switch and belongs under it.
   Evidence: `local/probe_2s_c265_plain.txt` and `local/probe_2s_c265_hydro.txt`, line 641 in each.

5. **Milestone 2r's section 6 and its correction 3 are closed.** They said nothing in the AI was
   missing for an arrival and that the moving goal was what made one impossible. Given a goal that
   does not move, the same AI latches on the first attempt. Evidence: `local/v_run_point.log`.

6. **`bsp_game.exe` on `origin/main` `95f39aa4` access-violates at startup, inside the installed
   `xlive.dll`.** This is not this packet's change and not this packet's file. The Windows
   Application Error log names `xlive.dll`, exception `0xc0000005`, fault offsets `0x0033dbeb` and
   `0x00319049`, on a 40-frame title-only run with no mission arguments. The log stops after the
   settings resolution line, which is the line before the `SoundServices` construction in
   `src/game_hosts.cpp`, and that construction is where `XLiveLibrary` loads the DLL from the
   current directory. `XLiveLibrary` throws when the load fails, so there is no degraded path and
   the executable now hard-depends on a working `xlive.dll`. Commit `2c11966a`, "Wire actual sound
   startup and shared XLive into the application", is the change that made the real DLL load. Every
   run in this milestone therefore passes `--xlive-dll` pointing at a no-op stand-in; the Validation
   section says what that means for the numbers.

7. **`src/game_hosts.cpp` line 1230 stores a dangling pointer.** It calls
   `bind_legacy_crt_math_runtime` with a braced temporary, and `src/legacy_crt_math.cpp` line 271
   stores the address of its argument, which the header itself says must outlive every adapter
   call. The temporary dies at the end of the full expression. It is not the crash above, because
   the binding is only dereferenced from `legacy_crt_87except_00c27489` on the x87 exception path.
   Both files are leased to another owner and were not touched.

### no_ghidra_function

none. Every address cited above lies inside an existing Ghidra function body.
`python tools/verify_report_calls.py reports/game_executable_milestone_2s.json` reports
26 call rows checked, 0 failed; three rows are reported as `indirect` and skipped, and all three
are genuinely indirect: `00826a6d` and `009e5821` are virtual dispatches, and `00932a16` is an
`OR` instruction rather than a call.

### Validation

`scripts/build.ps1` Release Win32 with `/W4 /WX /fp:strict`, no warnings, every target built.
`ctest -C Release`: `reconstructed_math` passes, 1 of 1. **No test cases were added.**
`origin/main` was merged at the start of the turn, taking the branch from `b506e3cf` to
`95f39aa4`, which is the merge of packet `ship_hydro_forces`. The baseline below was taken on that
merged tree before the first edit.

Every run passes `--xlive-dll` pointing at `local/xlive_stub.dll`, a hand-built no-op DLL that
exports the twelve ordinals the startup path calls. It is not part of the build and is not
committed. It answers the way an absent Live service answers: the message pump translates nothing,
the notification queue is empty, the sockets layer succeeds with no work, which is the state the
executable was in before commit `2c11966a`. The baseline run's counters are identical to milestone
2r's published figures to the digit, which is the check that the stand-in changes nothing this
milestone measures: `units=32 ai_owned=31 steps=15680 gated=0 replans=5048
state_steps{concrete=4803 records=245}`, `cruise=13 stop=12 attackmove=6 movetopos=1`,
`arm tail bodies=3426 latched=3425 stops=6 arrival_latches=0`, `sector_scans=47040 sector_marks=0
ring_scans=588`, `total_path=5047.64`, `DeRuyter` 34.80 and `Kortenaer` 258.16 in the distance
table. Runs must also be sequential: `bsp_game.exe` takes a single-instance mutex at `008F8301`
and a second process exits at the error message box with no window.

```
bsp_game.exe --frames 700 --press-start-frame 30 --menu-select USN02 --mission-frames 500
  --mission-frame-seconds 0.05 --mission-complete-frame 490 --order moveto:Java
  --order-unit Kortenaer --order-frame 5 --trajectory-csv local/v_traj_all.csv
  --log local/v_run_all.log --xlive-dll local/xlive_stub.dll --game-root "<install>"

summary mission ship ai units=32 ai_owned=31 steps=15680 gated=0 replans=5048
        state_steps{concrete=4803 records=245} publishes=15680 promotions=15680
summary mission ship ai states cruise=13 stop=12 attackmove=6 movetopos=1 other=0
summary mission ship ai path search ticks=3421 swaps=12 points=3419 corner_arms=0
        units_with_point=7 output_blocks=3426 bearings=3419
summary mission ship ai nav blocks=32 clearance=15680 throttle_profiles=15680
        sector_scans=47040 sector_marks=0 ring_scans=588 ring_bearings=588 firepower=0
        follower_points=3419 follower_corners=0 follower_advances=0
summary mission ship ai command completion events=0 callbacks=0 end_commands=0
        queue_advances=0
summary mission ship ai arm tail bodies=3426 latched=3425 stops=6 arrival_latches=0
summary mission ship ai ring hops=15680 gated_3f5=0 writes=15680 rudder_law=15190
        deadbands=6275 live_pair_changes=1691 driven=0
summary mission world lists registrations=32 pushes=192
        lists{1=32 2=32 4=32 5=32 6=32 7=32}
summary mission hydrodynamics calls=15680 element_steps=125440 submerged_steps=93928
        add_force=15680 add_torque=15680 gravity_y=-10.0 elements_per_hull=8
summary mission director completion end_commands=0 stage_raises=2 clear_messages=2
        clear_receives=2 queue_advances=2 restarts=0 command_events=0
summary mission world units=32 walked=15680 updated=15680 motion_ticks=15680
        simulated=24.50 s controlled=DeRuyter moved=33.43 total_path=5007.14
host methods 618 concrete, 495 unimplemented

  unit          StartSpeed  reference   ratio    axial   expected    moved    delta
  DeRuyter         12.0000    16.4622  0.7289  12.0000     294.00    33.43  -260.57
  Java             12.0000    16.4622  0.7289  12.0000     294.00   293.81    -0.19
  Kortenaer        12.0000    18.2628  0.6571  12.0000     294.00   215.02   -78.98
  (the other eleven holding cruise ships: 293.81, delta -0.19)
```

Against the baseline on the same line, the counts that should not move do not: `steps` 15680,
`replans` 5048, `state_steps{concrete=4803 records=245}`, `states cruise=13 stop=12 attackmove=6
movetopos=1`, `output_blocks` 3426, `arm tail bodies` 3426 and `latched` 3425, `stops` 6,
`ring hops` 15680, `sector_scans` 47040 with `sector_marks` still 0, `ring_scans` 588 and the same
two director completions. The counts that move are the ones the drag moves: `total_path`
5047.64 -> 5007.14, every holding cruise ship 294.05 -> 293.81, `Kortenaer` 258.16 -> 215.02 because
it is turning rather than sliding, `DeRuyter` 34.80 -> 33.43, `search ticks` 3420 -> 3421,
`swaps` 13 -> 12, `deadbands` 6272 -> 6275 and `live_pair_changes` 1760 -> 1691. `host methods`
goes from 605 concrete / 493 unimplemented to 618 / 495: thirteen new concrete methods and two new
records, the unit's vtable slot `5Ch` and the `+130h` dispatch.

```
bsp_game.exe --frames 2700 --press-start-frame 30 --menu-select USN02 --mission-frames 2500
  --mission-frame-seconds 0.05 --order moveto:0,-1000 --order-unit Kortenaer --order-frame 5
  --trajectory-csv local/v_traj_point.csv --log local/v_run_point.log
  --xlive-dll local/xlive_stub.dll --game-root "<install>"
```
The `expected` column of that last row is the straight-run distance a ship at the seeded ratio
would cover in 125 s, and the ship stops 5.00 m short of it because it stopped on purpose. The
extra `stop` in `stops=7` and the extra `stage_raise`, `clear_message`, `clear_receive` and
`queue_advance` over the mission's own two are Kortenaer's.

The `--mission-frames 1200` run (60.0 s, `local/v_run_long.log`) reports **613 concrete, 486
unimplemented**, `search ticks=8379 swaps=24 points=8389 corner_arms=0 output_blocks=8396`,
`arm tail bodies=8396 latched=8395 stops=6 arrival_latches=0`, `sector_scans=115200
sector_marks=0 ring_scans=1440`, the same two director completions and `total_path=12509.55`
against milestone 2r's 12925.23. Every count except `ticks`, `swaps` and `total_path` matches 2r
to the digit. The 3000-frame run (150.0 s, `local/v_run_conv.log`) is where section 1's drift
table comes from: **613 / 486**, `arm tail bodies=20996 latched=20995 stops=6 arrival_latches=0`,
`total_path=31689.48`.

Every earlier switch was rechecked on this binary and every run exits as milestones 2o, 2p, 2q and
2r say it should. A 120-frame run with `--press-start-frame 30` and no `--menu-select` exits 0 and
reports **158 concrete and 79 unimplemented**; a 40-frame title-only run exits 0 and reports
**133 and 48**; `--vfs-probe fonts/fonts.lua` exits 0 with `probes=4/4` and
`--vfs-probe does/not/exist.lua` exits 3 with `probes=3/4`. The unimplemented figures are milestone
2r's exactly. The concrete figures are three higher on both title runs, by the same three that
took the baseline 500-frame run from 2r's 602 to 605 before this packet changed anything: they are
the merges between the two milestones, and none of this packet's own host methods is on the title
path, because that path creates no units.

The drift table of section 1 comes from `local/base_traj_conv.csv` and `local/v_traj_conv.csv`
through `local/drift.py`, and the probe comparison of section 5 from

```
bsp_ship_motion_probe.exe --lua "<install>/scripts/datatables/autoload/vehicleclasses.lua"
  --class 265 --hydro --moveto 0,1500 --steps 3000 --dt 0.05
python tools/motion_trace_compare.py --trace local/trace_2s_kortenaer_to_arrival.csv
  --probe local/probe_2s_c265_hydro.txt --align-origin
```

`origin/main` was merged again after this milestone was committed, taking in packets
`ship_buoyancy_elements`, `ship_ai_settings_block` and the rest of `c6de1e9a`..`e7afcb1a`, and the
500-frame line was rerun on the merged tree (`local/m_run_all.log`, exit 0). Everything this
milestone measures is unchanged to the digit: `hydrodynamics calls=15680 element_steps=125440
submerged_steps=93928 add_force=15680 add_torque=15680 gravity_y=-10.0`, `world lists
registrations=32 pushes=192 lists{1=32 2=32 4=32 5=32 6=32 7=32}`, `arm tail bodies=3426
latched=3425 stops=6 arrival_latches=0`, `states cruise=13 stop=12 attackmove=6 movetopos=1` and
`total_path=5007.14`. The one figure that moves is `host methods`, 618 concrete / 495 unimplemented
to 629 / 486, which is the merged packets' own hosts and none of this one's.

This remains a runtime-validated process, not a game-validated one, and it is now runtime-validated
against a stand-in `xlive.dll` rather than the installed one. What it proves that milestone 2r did
not: that the hull's hydrodynamics run where the game runs them and take a turning ship's drift
from 78 degrees to under a degree and a half; that the world's gravity and the hull's buoyancy
balance to the digit; that `00937440` applies its own rudder torque; that a unit joins six of the
world registry's per-class lists at creation, one of which is the one the ship AI's neighbour walk
reads; and that a ship ordered to a fixed point reaches it, latches, posts a `finished` event into
the mission Lua, ends its command and comes to rest 5 m away. What it does not prove is anything
about the game's own buoyancy element list, which no packet has read, or anything about the
neighbour list, whose consumer this packet could not reach.

### Follow-up packets

1. **`ship_ai_neighbour_walk`**, `009F1877..009F1A44` inside `009F1420`. The consumer this packet
   could not wire. Read list id 6 through `GameUnitsHost::world_list_size` and `world_list_entry`,
   skip self (`009F18BB`), apply the fifteen-metre altitude band (`009F1929`, the double at
   `00CF3F20`) and `ship_ai_neighbour_admission_radius_009f1987`, and hand each survivor to
   `ship_ai_neighbour_list_add_009f0d20`. Both rules and the append are already reconstructed in
   `include/bsp/ship_ai_sector_scan.hpp`; only the walk is missing. It is what the sector scan, the
   clearance sweep and the arm tail's traffic setback are all waiting on, and `009D84E0` is
   unreachable without it.

2. **`ship_ai_follow_land_hosts`**, `009E1610` and `009E1950`. Bind `bsp::ShipAiFollowStepHost` and
   `bsp::ShipAiLandStepHost` beside the existing `stop`, `movetopos` and `attackmove` arms. Both
   reconstructions are complete and consumed from main; only the binding is missing. Neither state
   is entered on this mission, so a mission that reaches one is what would exercise them.

3. **`game_executable_buoyancy_elements`**, adopting `include/bsp/ship_buoyancy_elements.hpp`'s
   generator in place of this milestone's stand-in list. The producer packet landed on `main` at
   `c6de1e9a` after every run here was taken. It replaces one block in `create_units` and changes
   nothing else, and it is what would turn sections 1 and 5 from figures about a stand-in into
   figures about the game's own hull. If the executable holds no model node data, the header's own
   generator over a documented stand-in polyline is the fallback, and whichever is used has to be
   said in the milestone that uses it. Note also that the vector base is `descriptor+528h`, which
   corrects the `class+52Ch` this milestone and `docs/SHIP_HYDRO_FORCES.md` both write.

4. **`ship_hull_shapes`**, `00937D3F..009399BF` and `00C5C940`. Carried over unchanged. Two torques
   now reach the body every step, `00937440`'s rudder torque and `009329C0`'s lever-arm sum of
   about 3.0e6 about Y, and both are discarded by a zero inverse inertia. This is the packet that
   would make them matter.

5. **`game_executable_xlive_startup`**, the crash of correction 6. Until it is fixed, no milestone
   can validate against the installed `xlive.dll`, and the stand-in has to be rebuilt by every
   worker that needs a run.

6. **`ship_ai_moveto_static_goal`** is closed by section 6 and should be struck from milestone 2r's
   follow-up list.

## Correction from docs/SHIP_AI_AVOIDANCE_REQUEST.md

Packet `cc_ai_avoidance_request` read the avoidance request block whole: it is four fields, not
three, each ANDed at its consumers with one of the three authored booleans on the command
controller that `008362A0` names (`blk+3ECh` with `director+240h` torpedoAvoidance, `blk+3F0h` with
`+241h` shipCollisionAvoidance, `blk+3F4h` with `+242h` landCollisionAvoidance) plus `blk+3F5h`,
read by `009ED6B0` and `009F3F80`. `blk+3F5h`, which the milestone 2o section above records as
having no writer, is written by `009E1170`'s arm 1: it is the whole-controller bypass for a helm a
person is holding. The request never changes which ships enter the neighbour list (`009F0D20`
reads none of the four bytes); the filter only sets `node+69h` through `009EAFC0`.
