# Sound resource loading through installed VFS, Lua and FMOD

Addresses: 00a88770, 00a84740, 00a82ea0, 00a835b0, 00a83450, 00a823f0, 00a82720, 00a85790, 00a85500, 00a85a50, 00a79910, 00beccd0, 00bdd340, 00bd9e80, 00bd9f00, 00bf3a80, 00be5c80, 00bb9d50

`SoundResourceRuntime` connects the cache, asset and cleanup packets to the
existing canonical VFS and the installed FMOD DLL adapter. The previous
`SoundResourceLoadHost` boundary now has a concrete implementation. This is a
typed C++ reconstruction; it is not the native object layout or exception ABI.
Worker evidence is in SOUND_RESOURCE_CACHE.md, SOUND_RESOURCE_ASSET.md and
SOUND_RESOURCE_CLEANUP.md. The machine-readable integration record is
reports/sound_resource_runtime.json.

## Actual services and lifecycle

Cache resolution uses the existing VFS candidate resolver. FSB creation reads
the whole file through the same ordered mounts and memory-stream adapters, then
calls actual FMOD `CreateSound` with the recovered 6Ch create-info structure.
The resource owns the bank, and borrows its first subsound. The FEV branch calls
the installed EventSystem load export and owns its returned EventProject.
The DLL adapter implements the recovered length, mode, defaults, distance,
variation and loop setters; original results remain inspectable.

Cleanup releases the bank and event project through their actual DLL exports.
It then reloads the **current** sound singleton and its +54 resource cache
(`00a79910`, MOV EAX,[ECX+54]; RET), and invokes the recovered name removal.
The singleton and +54 cache must remain published until resource destruction
finishes. Explicit `destroy_owner`/`release_resource` operations precede resetting
the owning pointer, unregistering the singleton, and releasing EventSystem.
The application supplies the shared F8BBE4 accounting word; no missing increment
or balancing adjustment is invented.

The native cache has a conditional reentry behavior confirmed independently:
when a final reference destroys a resource and removes its entry from the same
current cache, both outer clear and name removal subtract its size. Outer clear
then reloads count and may pop the newly last record's storage without releasing
that record's resource. The reconstruction preserves this exact path. This is
not a promise of balanced teardown for arbitrary multiple-entry states.

`FrameClockSoundStartupHost` uses the existing `00bee080` clock sample and copies
its four words. `VfsSoundConfigurationLuaOwner` opens the existing Lua 5.1.1
owner, obtains installed fundamentals through VFS, runs the recovered chunk
loader, and closes reader references before the interpreter. Native fatal Lua
errors remain an explicitly guarded C++ boundary in the existing Lua adapter.

## Load-time platform and file-date operations

`00beccd0` drains all thread messages with PeekMessageA(PM_REMOVE), calls the
required XLive pretranslator, translates/dispatches unconsumed messages, then
invokes current cursor/focus policy with byte 1. It has no frame callback,
WM_QUIT special case or application exit test. Native ECX=platform, RET at
00becd36. The concrete Win32 pump keeps pretranslation and `00becb20` policy as
required application services; the runtime fixture supplies observers for them.

The five cache words from `00bdd340` are a file date: year, month, day, seconds
since midnight, milliseconds. The hidden output pointer and name are two stack
arguments; ECX=manager, EAX=output, RET8 at 00bdd436. It normalizes a copy and
visits existing mounts in their supplied order, without open-only aliases.
`00bd9e80` copies all five provider words. `00bd9f00` stops traversal on **any**
nonzero word; an all-zero result continues to the next matching mount.

Canonical provider bindings now implement their actual +20h operations:

| Provider | Native target | Date result |
| --- | --- | --- |
| Physical directory | 00bf3a80 | Last-write UTC date; zero when manager+78 disables dates or GetFileAttributesExA fails |
| FileStore | 00be5c80 | Five FFFFFFFF words for a cached name; zero on a miss |
| MPKG | 00bb9d50 | Unconditional zero; no archive-entry probe |

Physical lookup copies the name, asks existing +18h path replacement (existence
then build-path), and uses GetFileAttributesExA even if replacement failed. At
00bf3b52, the preceding PUSH means [ESP+48h] identifies attributes+14h, the
last-write FILETIME. FileTimeToSystemTime produces UTC. Native diagnostics
004254b0 are a verified bare RET. Failed FileTime conversion is rejected by
the host because native would consume unspecified SYSTEMTIME words.

The new mount callback reads current `file_dates_disabled` for each visit; only
physical providers use it. Existing mount order, provider ownership and other
callback positions are preserved. Unsupported strings, unavailable provider
operations, native checked-iterator corruption and allocation failures remain
outside the standard-container projection's verified domain.

## Verification and limits

Win32 Release and both existing CTests passed. A focused local fixture checked
actual file time 2021-02-03 04:05:06.007 UTC, ordered zero-result fallback,
date suppression and a real queued thread message. Independent reviews checked
file-date assembly, provider bindings, resource lifetime reentry, runtime
composition and all 12 newly required installed DLL exports.

The installed-runtime probe ran `construct_sound_system_00a88770` with the
canonical owner, actual clock, VFS, installed fundamentals/soundsetup Lua and
installed FMOD DLLs. Its platform pretranslation/focus hooks are explicit fixture
observers. Disabled-sound output was selected. No audio was played.

Observed results:

- 133 FMOD calls, all returning success.
- Installed error.fsb: 2,688 file bytes; bank raw size 2,560 bytes; first subsound
  8,064 PCM frames at 44,100 Hz; duration approximately 0.182857 seconds.
- One initial cache record with two resource references; a missing-name request
  reused the actual fallback bank, appended its alias and retained even though
  clone/load-if-missing were both false. The extra reference was released.
- Explicit owner teardown released the actual bank and emptied the cache.
  Both counters exhibited the recovered unsigned subtraction behavior.
- Installed planes.fev loaded and released as an actual EventProject, reporting
  300,597 bytes from the native before/after FMOD memory accounting in this run.
- Twelve channel groups and 25 sound types configured from installed scripts.

Probe source, command and results are local/sound_resource_probe.cpp,
local/run_sound_resource_probe.cmd, local/sound-resource-probe.log and
local/sound_resource_probe.json. Installed files were read only.

Five missing Ghidra function entries were defined from verified bytes. Internal
free-call gaps in A84450 and A85AC0 were repaired. A842E0, A85500 and A84D00
tails were decoded after clearing false CALL_RETURN overrides, but their saved
function bodies remain short. Physical BF3A80 is also split from its existing
BF3ACC continuation in saved analysis. These metadata limits do not invalidate
the inspected native bytes; complete function-body repair is not claimed.

The current game executable still reports phase-5 sound initialization as
unimplemented in `src/game_hosts.cpp`. Its owner should bind these adapters to
actual application lifetime,
mounts, platform policy and clock, then keep the cache published through cleanup.
No change to that separately owned executable host was made in this packet.
Enabled hardware-audio startup, audible playback, actual XLive/cursor policy,
native binary/exception ABI and gameplay remain unvalidated.

Combined integration build `4c0d029` compiled `bsp_game` and passed both existing
CTests. All 31 reviewed names and evidence comments were saved and read back,
with prior comments preserved. The 32 affected exports were refreshed, including
the analyzed reserve helper whose internal free-call gap was repaired.

After concurrent locale/Lua-reader integration, Win32 build and both CTests
passed again at `033d79e`. The installed sound constructor/FSB/FEV fixture was
rerun against that build and reproduced all reported results, including 133
successful FMOD calls. The platform observer and gameplay limits remain.
