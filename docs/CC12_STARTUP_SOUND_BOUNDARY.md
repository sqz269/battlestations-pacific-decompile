# CC12 startup: boundary before the sound-complete marker

The captured current application exits with access violation `0xC0000005`
before `Phase 5 sound_system_initialize`. The available log does not identify
FMOD as the faulting component. Its last completed record is the settings
summary; Source still has math binding, `SoundServices` construction, raw
singleton operations and a clock read before the first FMOD Event call.
This bounded diagnosis supplies the ordered debugger boundaries and verifies
the DLL/export prerequisites. It changes no Source or configuration and runs
no additional game process.

## Current runtime observation

The primary agent captured `bsp_game.exe` with Python `subprocess`, requesting
three frames, a 960x540 window, installed game data and an isolated settings
directory. `local/cc12_startup_exit_capture.json` in the integrator worktree
records return code 3221225477 (`0xC0000005`), 0.3433566 seconds, and empty
stdout/stderr. Its paired `cc12_startup_captured.log` stops after settings and
contains neither the sound-complete marker nor a run summary. The primary
reports no screenshot and no surviving process. The earlier PowerShell GUI
invocation's shell exit zero is superseded by this actual child exit capture;
it was not a successful startup result.

The sound-complete marker is emitted at `src/game_hosts.cpp:2004`, after both
`std::make_unique<SoundServices>(*this)` and `sound_->core.startup(...)` return.
Only the latter is inside the local FMOD diagnostic catch. An ordinary C++
exception would be eligible for the outer `run_win_main` catch and a
`startup failed` log, but the observed access violation is not evidence of
that C++ recovery path. Absence of a sound marker alone does not prove that
the first FMOD call was reached.

## Ordered boundaries for the next debugger capture

| Order | Source boundary | Contract or observation |
| --- | --- | --- |
| 1 | `game_hosts.cpp:1990`, `bind_legacy_crt_math_runtime` | Binds process-lifetime globals and current `_errno`; the statically supplied pointers are nonnull. This is not proof against earlier heap/state corruption. |
| 2 | `SoundServices::SoundServices`, `game_hosts.cpp:1339` | Dereferences the existing settings process and constructs XLive/sign-in/device/platform services before core sound. `XLiveLibrary::Impl` loads the selected DLL with `LoadLibraryExW`. This is the first explicit foreign loader boundary after the logged settings summary. |
| 3 | `GameSoundRuntime::Impl`, `game_sound_runtime.cpp:61` | Creates Source state/manager and loads `fmodex.dll` in `FmodConfigurationLibrary::Impl`. It constructs resource/callback/clock/Lua contexts and checks the two CRT binding pointers. The Event DLL is still lazy. |
| 4 | End of `SoundServices` construction | Attaches dialog contexts, binds the XLive profile SDK and binds this sound runtime to the existing singleton deletion dispatcher. No successful-construction marker is logged. |
| 5 | `GameSoundRuntime::startup`, `game_sound_runtime.cpp:307` | Requires one-shot startup and sole ownership of the callback binding, then calls `construct_sound_system_00a88770`. |
| 6 | `sound_startup.cpp:28..38` | Constructs/registers the sound base and auxiliary owner using the actual shared lifetime manager, moves the sound registration after the auxiliary registration, and copies the current clock words. All precede FMOD. |
| 7 | `FmodConfigurationLibrary::event_system_create`, `fmod_configuration_library.cpp:90` | Lazily loads `fmod_event.dll`, resolves `_FMOD_EventSystem_Create@4` and calls it with the event-handle output pointer. This is the first FMOD SDK call in this startup sequence. |
| 8 | Remaining `initialize_sound_library_00a8881e_fragment` | Gets the System object, queries drivers and speaker mode when enabled, initializes EventSystem, installs file callbacks and 3D settings, then queries output state. |
| 9 | Resource and Lua initialization | Constructs the resource owner and opens/calls sound configuration through the current VFS/Lua/FMOD services before returning to the marker. |

The raw manager reorder deserves an explicit pre-FMOD breakpoint if the fault
lands in that part of the call stack. `SoundLifetimeManagerView` invokes
`move_native_singleton_object_after_00bd0d70`, whose raw implementation reads
the live manager begin/end/capacity at +4/+8/+Ch, requires both object and
anchor matches, closes the removed slot and reinserts after the anchor.
Neither a copied manager nor a substitute semantic list is supplied. This is
a concrete prerequisite and breakpoint, not a claim that reorder caused the
observed access violation. The clock branch similarly reads the actual
published frame-clock storage before FMOD startup.

The SDK call journal appends a result only after the foreign call returns.
An access violation inside DLL loading or a foreign call can therefore leave
no corresponding journal row; the lack of a logged FMOD result cannot exclude
those calls. Conversely, the same lack does not establish that they ran.

## DLL, ABI and data checks

The captured command specifies the installed game root and no FMOD/XLive
overrides. `game_main.cpp:660` changes the process working directory to that
root before creating the host; `selected_library_path` resolves empty
overrides to an absolute path there. This establishes the paths Source asks
the loader to open. No runtime loaded-module list has yet been supplied, so
actual module selection and successful loading are not claimed.

Static PE checks of those installed files found:

| File | Machine | Bytes | SHA-256 |
| --- | --- | --- | --- |
| `fmodex.dll` | Win32 x86 `014C` | 350544 | `31e7451aef6115b0aec353e4e508ff9f0fc14ea3a8957e5ddb805ede911700e8` |
| `fmod_event.dll` | Win32 x86 `014C` | 238936 | `4b6ae7ba8d3da780c23abb5e72a65ea50e5348a60a1b4aef43196e09ae771890` |
| `xlive.dll` | Win32 x86 `014C` | 3833856 | `71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2` |

Both FMOD PE version resources contain the four-word tuple `0.4.18.4`
(the SDK's 4.18.4 release). The Event DLL imports `fmodex.dll`; both import
`MSVCRT.dll`. All 13 inspected early startup exports are present: the three
decorated EventSystem entries and ten System/memory entries. The three Event
names carry `@4`, `@8` and `@20`, matching the current stdcall argument sizes.
Current bindings use `__stdcall`, 32-bit scalar values and live output
pointers; callbacks are real Source functions with their stdcall signatures.
`SetFileSystem` passes those functions, not integer game-image provenance
addresses. This is a source/PE compatibility check, not a complete live ABI
or DLL dependency-resolution proof.

The math runtime uses process-lifetime bypass words, current `_errno` and
`legacy_crt_87except_00c27489`. The same math-binding call already occurs at
`game_hosts.cpp:1885`, before the logged renderer/settings work, so the later
call is a repeat binding. Core construction rejects missing required CRT
bindings. Startup binds the actual VFS/file callback context before FMOD may
call it, and keeps the library/context alive through teardown. The actual
manager, both registered objects, frame-clock publication, callback outputs
and subsequent VFS/Lua data must remain valid. Static presence of those
bindings does not establish their live contents at the crash.

`docs/AUDIO_STARTUP_DIAGNOSTIC_CC10.md` records the same FMOD DLL hashes and
historical result 61 followed by 78/37. It also records a later successful
finite application run. Those observations concern older executables and
environment snapshots. The current observation is an access violation, with
no captured FMOD result. Applying the historical output-initialization
diagnosis or changing audio options is unsupported by this evidence.

## Evidence and next check

The 21 inspected chain source/header files byte-match the integrator's
`7479af9fd70b41091bc077e9ea98b84dbc45f0bf` snapshot. The worker baseline is
`a8895ad237fd9d19c1c06c5ff4f668ed659df069`, after merging main `7ed2ec2002`.
Read-only evidence is retained in the worker's
`local/cc12_startup_sound_boundary_evidence_20261008/`: 29 files, manifest
SHA-256 `ee6ac2c38c44133146a8b1d595b9e62fa67618a1a6084229c327776b8ace0e2d`.
It contains the paired runtime capture/log, static DLL/export results, source
snapshots and the cited historical documents. The tracked JSON records the
individual hashes and observation limits.

The next discriminating check is the primary agent's debugger capture of the
first access violation: exception address and read/write target, loaded-module
path/base, registers and call stack. If symbols are available, break at
`SoundServices` construction, `GameSoundRuntime::startup`, the raw manager
reorder, and `event_system_create` in that order to identify the first reached
boundary. Preserve the current DLLs and audio options. Once the fault belongs
to a concrete call, inspect that call's inputs and immediate predecessor;
there is no supported Source fix yet.

This packet made no Ghidra query or mutation, expanded no native body, ran no
build/test or additional process, and did not validate sound initialization,
audible playback, frame presentation or gameplay.
