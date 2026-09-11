# Sound constructor sequencing and callable FMOD startup

Addresses: 00a88770, 00bd0d70, 00a7a460, 00a81480, 00a858f0, 00a88650, 00a7d410, 00a7b750, 00a79930, 00a79970, 00be41a0

`construct_sound_system_00a88770` now connects the recovered normal constructor
sequence through its final Lua configuration call. The old `start_audio` entry
covered the FMOD stage only. The new sequence uses the canonical system,
configuration, class and singleton lifetime owners, with required clock,
resource-loader and Lua-owner interfaces. There are no fallback implementations
for those game services. This is a typed reconstruction, not a replacement
178h native object or original exception ABI.

## Native order and interfaces

The native constructor receives ECX=this, a disabled byte followed by two unused
stack words, returns this in EAX and executes RET 0C at 00a88aa6. It constructs the
base, sets the derived vtable/enable flag and +174, constructs the global auxiliary
tree, reorders lifetime registrations, copies four clock words to +118..124,
initializes FMOD, constructs the resource owner at+54 and loads Lua configuration.
These calls occur in the original order; base state is not reset between stages.
The raw/unhandled speaker-mode branch leaves the previous layout untouched.

`00bd0d70` is a lifetime ordering operation: remove the first object match, then
insert it after the first remaining anchor match. Both searches require valid
matches. Erasure closes the pointer-array gap, preserves other null holes and
retains capacity. Native __thiscall(manager,object,after) ends in RET 8 at 00bd0f47.
The implementation reuses the existing checked insertion logic. Bounds-handler
calls may return: captured base pointers at 00bd08db and 00bd0925 survive those
callbacks. Independent review corrected an extra shared bounds check and those
two captured-pointer reloads. No new foreign-owner or null-this recovery is added
by the typed method.

`FmodConfigurationLibrary` also implements the startup interface, loading the
caller-specified Event DLL when needed. It calls the installed stdcall C exports
for EventSystem creation, initialization, update and release, and the System
settings/query functions. The original four callback addresses remain provenance;
`SetFileSystem` receives actual reconstructed function pointers. Unknown hook
provenance is rejected before making a DLL call. Library results remain visible,
including errors ignored by the recovered native caller. The memory checkpoint
00a7a460 calls the actual two-output-pointer FMOD memory query and emits no log.

## Verification

The Win32 Release build and both existing CTests passed. A focused singleton
fixture exercises moving a registration to middle/end/after a hole and checks
order, count and retained capacity. The keyboard/mouse worker additionally
matched 6272 exact native query results, and file callback read/seek bodies were
executed against actual memory/physical stream adapters. Full device creation
and hardware input were not exercised.

A local runtime probe executed the recovered disabled-sound FMOD stage using the
unchanged installed `fmodex.dll` and `fmod_event.dll` (FMOD version 0x41804). It
then applied the installed 6559-byte soundsetup.lua with actual Lua 5.1.1 and the
real FMOD configuration interface. FMOD successfully created a stream from a
44144-byte PCM WAV fixture through the reconstructed open/read/seek/close
callbacks, using the existing real PhysicalFile implementation. The fixture is
under local/; the game installation was read only. No sound was played.

Observed: 117 successful library calls; one open and matching close; 12 reads,
13 seeks and 17570 bytes read; 12 groups and 25 sound types configured. The probe
released the stream and EventSystem before restoring its file context. It uses
an explicit loose-file fixture resolver and the already-loaded Lua fragment;
this does not verify native VFS mount resolution or unprotected Lua loading.
Source/commands/logs are local/sound_event_file_probe.cpp,
local/run_sound_event_file_probe.cmd, local/sound-event-file-probe.log and
local/sound_event_file_probe.json. Exact hashes and results are retained in
reports/complete_sound_startup.json.

## Analysis repairs and remaining work

The primary defined 16 verified function entries missing from Ghidra's saved
analysis. Calls still resolve through the existing BSP project; no game binary
was modified. The false no-return call at 00a85a2d was cleared and the 24-byte
constructor epilogue decoded. Ghidra's stored function-body listing still omits
that tail even though decompilation now reaches return. The bridge's arbitrary
Java script facility is disabled, so the attempted supported body extension
made no mutation. The incomplete metadata and original bytes remain recorded
in the dedicated repair/extension reports; full body repair is not claimed.

The full constructor sequence is build/evidence reviewed. Its resource loader
00a84740 remains a required meaningful game dependency, including cache mutation
and asset ownership; no loader stub was used for the runtime probe. The owner
packet supplies a required failure-cleanup boundary 00a85500 after path/options
unwind. Complete resource/manager teardown, original object/exception ABI,
audible playback, the enabled hardware-audio startup branch and gameplay remain
unvalidated. Next asset work follows 00a84740 through 00a82ea0 resolution,
00a835b0 creation and 00a854c0 cloning, with 00a85500/00a84c90 cleanup.

Integration build at `3d04e83` also compiled the concurrently added `bsp_game`
target and passed both CTests. Main integration `c33924f` adds only coordination
documentation after that build. All 31 reviewed names and evidence comments were
saved and read back, preserving previous comments; affected exports refreshed.
This does not establish that the game executable runs through sound startup.
