# Installed sound configuration verification

Addresses: 00a7ff80, 00a7c740, 00a7de10, 00a7e0d0, 00a7f9f0, 00a7ae00, 00a7ae80, 00a7b120

The recovered sound configuration policy now has a concrete Win32 FMOD boundary
in `src/fmod_configuration_library.cpp`. It loads the caller-supplied DLL with
`LoadLibraryExW` and calls its C exports. It returns library results unchanged
and records each call/result; it does not implement FMOD algorithms. Systems
must be released before unloading the library, and calls must be serialized by
the owner. Missing exports and DLL load failures are explicit C++ errors.

The original executable pushes the object pointer as an explicit first stack
argument to its FMOD imports. The adapter uses the installed DLL's stdcall C
exports, including the two-output-pointer Memory_GetStats form used by this
build. Original object addresses are never invoked in the reconstructed process.
See `SOUND_CONFIGURATION.md` for native calling conventions, state offsets and
uncertainties in the game-owned policy.

## Installed-data result

The isolated Win32 probe loaded the unchanged installed `sound/soundsetup.lua`
(6559 bytes) with the repository's Lua 5.1.1 interpreter and used the unchanged
installed `fmodex.dll` (reported version 0x41804, FMOD 4.18.04). It created and
initialized a real FMOD System with 512 channels, flags 0x10 and explicitly
selected no-sound output. It queried the master channel group, ran the recovered
loaded-Lua fragment, updated the system, read state back and released it.

The result contained 12 channel groups, 10 group DSPs plus one system DSP,
3 listeners, 7 retained sound classes and 25 sound-type routing tables.
All 42 DSP parameter calls and every other recorded FMOD operation returned 0.
Live advanced-setting readback was HRTF [120,180,10000]; group volume/pitch
readbacks matched the script, including Silent 0.5, AirSpecialInUnderwater 0.7,
Distance3 0.9 and Distance4 0.8. The two-pointer memory-stat query, update and
system release also succeeded. Exact names, routes, results and installed-file
SHA256 hashes are retained in `reports/installed_sound_configuration.json`.

## Reproduction and evidence limits

Local reproduction: `local/run_installed_sound_configuration_probe.cmd`, source
`local/installed_sound_configuration_probe.cpp`, output
`local/installed_sound_configuration_probe.json`. The command links against the
normal `build/win32/Release` libraries and embeds a manifest in
`sound_configuration_runtime_probe.exe`. The executable name avoids Windows'
legacy installer-name heuristic. The installed game tree was read only.

This verifies the real FMOD C boundary and the loaded-script configuration
fragment. The probe deliberately uses the existing protected Lua host load
entry; the original base-library owner, VFS chunk loading and unprotected error
policy were not exercised by this probe. The full sequence retains those
explicit interfaces. No audio was played. Audible DSP effects, full manager
construction/teardown, native object/exception ABI, device startup and gameplay
are not established by this result.
