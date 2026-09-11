# bsp_game.exe, milestones 1 and 2a

Milestone 2a is the current state of the executable; milestone 1 below is the spine it was
built on and is still accurate except where this section corrects it.

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
| `PlatformLoopCallbacks::pretranslate` | `00bec20a` | **unimplemented** | 8 |
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
