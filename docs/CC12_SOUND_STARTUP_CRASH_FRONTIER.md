# CC12: current pre-sound crash frontier

The best-supported cause of the 2026-10-09 02:43:24 UTC startup failure is a
repeat of the installed XLive replacement's original-image patch during
`LoadLibraryExW`. The current receipt establishes exit `C0000005` after the
settings summary; it does not contain a current exception stack. The same DLL,
same selection route and an earlier retained debugger capture make this a
specific, evidence-backed recurrence hypothesis, not a new current fault-site
observation. Root's subsequent explicit genuine-runtime run passed the current
sound boundary and three bounded ticks with exit zero. No audio Source change
is justified by this evidence.

## Current observation and call order

The Root receipt is
`J:/PROG/battlestations-pacific-decompile-cc12_resume_integrator/local/cc12_current_startup_observation/receipt.json`.
It identifies Source `0b2eede1007b74f45a1a85622f359a288a4fbd89`, executable
SHA-256 `50f942443e2e73d82bd84b5038e5d8b41c5601897cb0e062d08a8c25b5a13ad1`,
and exit `3221225477` (`C0000005`). Its 7,101-byte log ends with the settings
resolution/language record. The captured command has `--game-root` pointing
to the installed game, and no `--xlive-dll`, `--xlive-dependency`, `--fmod-dll`
or `--fmod-event-dll` override.

The inspected Source chain is unchanged between that revision and worker
revision `bce4e5503`:

1. `game_main.cpp:1179` changes CWD to `--game-root`.
2. After the last settings record, `game_hosts.cpp:1999` binds the same CRT
   math runtime already bound before successful renderer/settings work. The
   binding checks two pointers and atomically stores the runtime address.
3. `game_hosts.cpp:2000` constructs `SoundServices`. Its member order loads
   `XLiveLibrary` before sign-in/device/platform/dialog adapters and core sound.
4. `selected_library_path` defaults to `filesystem::absolute("xlive.dll")`.
   `XLiveLibrary::Impl` reaches `LoadLibraryExW` in `xlive_library.cpp:28` after
   loading the explicit dependency list, which is empty in this capture.
5. Only after successful XLive loading can `GameSoundRuntime::Impl` load
   `fmodex.dll`. The FMOD Event DLL is loaded lazily when startup first calls
   `event_system_create`; it is not loaded by the FMOD wrapper constructor.
6. Only after aggregate construction does `core.startup()` construct/register
   the Sound owner, create/reorder its auxiliary owner, sample the clock and
   invoke the EventSystem entry. The Phase 5 sound marker follows its return.

Thus the log interval includes both constructors and sound startup, but XLive
loading is the first SDK DLL edge. The sign-in/device constructors retain the
module handle rather than invoking their later online/device ordinals.

## Installed DLLs and retained fault evidence

The selected installed XLive is a BSP-specific XliveLessNess adaptation,
3,833,856 bytes, version 1.0.0.0. Its currently verified SHA-256 is
`71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2`, exactly
matching [the prior startup audit](CC12_STARTUP_SOUND_BOUNDARY.md) and
[fixed-patch audit](CC12_XLIVE_FIXED_PATCH_CONTRACT.md). Installed FMOD core
and Event remain version 4.18.4 and match the prior hashes in the report.

The retained earlier `local/cc12_startup_debug3.log` was read directly: it
shows the installed XLive loaded at `698A0000`, a write AV at `69BB9049`
(`xlive+319049`, `MOV [EDI],EDX`), destination `10640F5E`, payload
`EDX=90909090`, and the loader stack through `LoadLibraryExW`. The historical
233,774,274-byte dump still hashes to
`895305456d9c172914155351da0216d50abc908611cdff22270fe823cb28a940`.
The prior bounded DLL audit identifies a five-NOP copy during PROCESS_ATTACH
to executable base plus fixed RVA `640F5E`, with no target-layout guard.
That historical capture reached the fault before any FMOD DLL loaded.

The current captured executable was independently rehashed against its
receipt. Its PE `SizeOfImage` is `4E2000`, so fixed RVA `640F5E` is still
outside its image at any relocation. With preferred base `10000000`, the
predicted target is `10640F5E`, beyond preferred end `104E2000`. These are
file-derived bounds, not a claimed current live mapping or exception address.
During report sealing, the integrator rebuilt the same executable path:
its new hash was `f7aedac68778c38f8b8526db15f900b175d0164dd858b92e39145c0c854ccdf7`
with the same image size. The report preserves the earlier receipt-matching
observation separately from this later file, rather than claiming the mutable
build path still holds the captured executable.
Root subsequently froze the failed executable as `observed_bsp_game.exe` and
its map as `observed_bsp_game.map` beside the original receipt. The frozen
executable's hash was independently verified to match that failed receipt.

## Existing genuine runtime route

Root's `local/gfwl-private-runtime/` already contains all four files from the
previously successful explicit-runtime route. Current hashes match all prior
pins: `xlive.dll`, `msidcrl40.dll`, `ppcrlconfig.dll`, and `xlivefnt.dll`.
The existing options can select that `xlive.dll` with `--xlive-dll` and preload
its matching `msidcrl40.dll` with `--xlive-dependency`. No file replacement,
new fallback, shim patch, stub DLL or Sound reconstruction is required to
perform the integrator's controlled comparison.

The historical system-XLive-only attempt failed with Win32 182 because the
system credential DLL lacked ordinal 43. The matching private dependency
avoided that separate loader failure. The earlier explicit-runtime run passed
sound with 129 FMOD calls and zero errors and completed three bounded ticks
at a compatible window resolution. That was older Source, not validation of
the current executable: the current XLive loader/FMOD adapter/math files match
their old hashes, while `game_hosts.cpp`, `game_sound_runtime.cpp` and the
sign-in source have changed. Successful current startup and later online IPC
behavior require their own observations.

## Supplementary current integrator observations

Root's separate `local/cc12_current_startup_observation/genuine/receipt.json`
records Source `bccbfa0312c2eaf4add2c2717951d3522773c533` plus its explicitly
recorded one-line CMake working-tree change, executable hash `f7aedac...`, the
genuine XLive/dependency paths, and `--window-resolution fit`. It finished at
2026-10-09 02:51:26 UTC with exit zero. Direct inspection of that run's log
confirms both sound/dialog markers, 129 FMOD calls with zero errors, sound
drain with zero FMOD errors, device HRESULT zero, three loop ticks and two
presents. The retained PNG is 1920x1080; this worker checked its hash/header,
not its visual contents. The log still reports 44 unimplemented host methods.

This is a current bounded startup/presentation observation performed by Root.
Because the executable and window-resolution arguments also differ from the
failed receipt, it is not a strict same-binary single-variable experiment or
proof of the earlier current AV instruction. It establishes a usable existing
runtime route for this current Source boundary, not gameplay or original ABI.

Root's separate `debug_receipt.json`/`debug_console.log` were also inspected.
That attempt loaded the installed XLive and stopped at a **second-chance
`C0000008` invalid-handle exception in `ntdll!NtClose`**, before capturing an
AV. It must not be reported as a current replay of the historical `C0000005`
write fault. No current AV dump is supplied by that attempt.

## Scope

This worker changed only this document and its JSON report. It performed
read-only Source, existing log/report, DLL and PE inspection; no launch,
build, executable probe, new test, Ghidra query/mutation or shared metadata
edit. The report distinguishes current evidence from historical runtime
evidence and records complete-file hashes. This packet adds no original
function credit and makes no full-startup, audible-sound, ABI or gameplay claim.
