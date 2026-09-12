# Application sound runtime composition

`bsp::game::GameSoundRuntime` owns the canonical sound state, configuration,
levels, classes, manager, auxiliary sample cache, installed FMOD library,
resource/sample runtimes, bank and event virtual adapters, spatial contexts,
retained-reference shutdown, and optional event-query lock lifetime. It connects
existing recovered implementations through a reusable pimpl interface. This is
application integration code with no original image address or native ABI.

The caller supplies the existing VFS mounts and registrations, Lua file/runtime
services and globals, frame clock, string storage, resource-load event policy,
singleton domain, CRT exception bindings and alternate-engine destruction
service. The runtime creates no mounts, private singleton domain, independent
Lua runtime, replacement platform policy or alternate engine. The DLL paths are
explicit; omitted Event DLL path means `fmod_event.dll` beside the supplied core.

## Startup and ownership

Call `startup(!settings.audio.enabled_24)` exactly once. The boolean reaches
`00A88770` unchanged: true selects the recovered no-sound branch; false runs the
installed FMOD device enumeration and enabled initialization path. It does not
force a mute policy. `owner()` requires completed startup; `summary()` and the
FMOD call records remain inspectable after shutdown.

Startup binds the actual VFS open/read/seek/close adapters before invoking
`construct_sound_system_00a88770`, including its shared shutdown context. The
recovered order remains base registration, auxiliary registration, lifetime
reordering, four-word clock sample, FMOD initialization, resource manager/error
FSB load and installed Lua configuration. Resource assets remain distinct from
actual 7Ch samples; channel contexts use `SoundSampleRuntime` for their sample
reference lifetime.

`GameSoundRuntimeWords` supplies writable, initially zero storage for the ID and
resource-accounting globals and the null-data fallbacks. `F8BBEF` aliases byte 3
of `F8BBEC`; these are live storage projections, not substituted immutable
strings. The alternate word can be supplied externally; otherwise it is owned
and initially null. A published alternate requires a real update callback.
Alternate deletion delegates to the required `SoundAlternateShutdownHost`.

The caller's singleton deleting callback routes `owns_registered(pointer)` to
`delete_registered(pointer, flags)`. The dispatcher recognizes the manager,
sample cache and lazy event-query lock and honors scalar-delete flag bit 0.
Manager/cache allocations release their `unique_ptr` ownership before native
scalar deletion; explicit runtime shutdown instead retains their inert C++
storage until runtime destruction. Neither path shuts down the caller's domain.

## Teardown and failure policy

Normal shutdown invokes `00A882C0`: alternate unregister/delete, configured DSP
and group release, retained samples/channels, active entries, two fresh listener
updates, resource manager destruction, EventSystem release, base storage cleanup
and manager unregistration. It then invokes the recovered `00A886C0` cache
destructor and unregisters any owned event-query lock. The current manager and
its resource-cache word remain available through resource final release. Native
dead `+44/+48/+54` words are not dereferenced after destruction.

The native constructor's base-only exception unwind is described in
[SOUND_STARTUP_UNWIND.md](SOUND_STARTUP_UNWIND.md). The application wrapper adds
an explicit recovery policy for surviving external allocations: close Lua,
remove an incomplete registration if present, temporarily publish the surviving
manager solely for resource-cache final release, release EventSystem, then
destroy the auxiliary cache. This recovery does not invoke the derived manager
destructor after failed construction and does not re-register the failed owner.
If auxiliary registration failed before its factory returned, its published
pointer may already be dead; recovery only removes that pointer value and never
reads or dispatches through it. A second cleanup exception terminates rather
than unloading a DLL with live assets. A failed explicit manager teardown also
prevents later silent DLL disposal.

The process-global FMOD callback binding permits one active runtime and rejects
an existing foreign binding. Restoration occurs only after EventSystem teardown.
Host exceptions are caught outside the installed DLL, preserve the first
exception, and return file failure. `rethrow_file_error()` exposes that exception
on the game thread; startup checks it automatically. Successful native callback
behavior is unchanged. Callers serialize the runtime and release external
sample/channel references before shutdown. All borrowed services outlive it.

## Evidence and ABI boundaries

| Existing implementation | Original call boundary | Evidence |
|---|---|---|
| A88770 ordered construction | ECX owner, disabled plus two unused stack words, EAX owner, RET 0C | [COMPLETE_SOUND_STARTUP.md](COMPLETE_SOUND_STARTUP.md), [SOUND_STARTUP_UNWIND.md](SOUND_STARTUP_UNWIND.md) |
| A84740 resources and A84D70 samples | Typed reconstruction around their separately documented native objects | [SOUND_RESOURCE_RUNTIME.md](SOUND_RESOURCE_RUNTIME.md), [SOUND_SAMPLE.md](SOUND_SAMPLE.md) |
| A882C0 manager destruction | ECX owner, RET; A883B0 takes flags byte, RET 4 | [SOUND_SHUTDOWN.md](SOUND_SHUTDOWN.md), [SOUND_RETAINED_CHANNELS.md](SOUND_RETAINED_CHANNELS.md) |
| A886C0/A88750 sample cache destruction | ECX owner, RET / flags DWORD, RET 4 | [SOUND_SAMPLE_CACHE_SHUTDOWN.md](SOUND_SAMPLE_CACHE_SHUTDOWN.md) |
| A7D410/A7B750/A79930/A79970 file callbacks | Win32 stdcall open/close/read/seek | [COMPLETE_SOUND_STARTUP.md](COMPLETE_SOUND_STARTUP.md), `sound_file_callbacks.hpp` |

Descriptive names remain hypotheses. This packet makes no new Ghidra mutations
or native address claims. The underlying analysis is the saved
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the seed-byte verification
passed before the Win32 native differential checks. This interface is not a
drop-in native 178h object, original vtable, exception ABI or binary replacement.

## Verification

Win32 Release built with `scripts/build.ps1`; both existing CTests passed. Shared
MSBuild worker reuse caused initial directory-access failures; disabling node
reuse with `MSBUILDDISABLENODEREUSE=1` resolved them without changing source or
directory permissions.

One local probe uses unchanged installed assets and DLLs, a caller-provided VFS
root, real Lua 5.1.1, clock and CRT bindings. Its platform event methods are
explicit fixture observers. Three normal runs cover disabled output with
explicit shutdown, disabled output with shared-domain scalar deletion, and
enabled hardware startup. Each creates seven configured classes, loads the
error FSB, creates an actual sample-backed nonspatial channel and loads the
installed `planes.fev` through the runtime's FMOD callbacks. Channel, sample,
cache, file and tracked string ownership are balanced on shutdown. The domain
case also creates and destroys the actual event-query lock.

Disabled runs made 168 FMOD calls each. Enabled startup found eight drivers,
reported output value 9 and made 177 calls. All three had zero initialization
errors. Each retained one raw FMOD `Channel_Stop=0x25` result from `00A7BF40`'s
unconditional destruction call on the deliberately never-started channel; the
native caller ignores it. No result is converted into success. Missing Event
DLL and an injected VFS failure while loading soundsetup.lua both propagated
their original error and left no live owned publication or tracked strings.

Probe source/runner/log are retained under `local/game_sound_probe.cpp`,
`local/run_game_sound_probe.ps1` and `local/game-sound-probe.log`. Counts and input
hashes are recorded in `reports/game_sound_runtime.json`. The probe did not play
sound or validate gameplay. Application phase-5 wiring remains separate: its
singleton host is being migrated to the native lifetime representation, and the
shared registration boundary must be connected before this semantic-domain
composition can be installed there. Alternate-engine streaming dependencies and
audible playback remain separate work.
