# Voice manager sound-fade update

Packet `orch4_voice_fade_b`, worker `agent/orch4-fade-20260910b`, 2026-09-11 UTC.
All live Ghidra queries verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was read-only. Descriptive names are
hypotheses pending integrator annotation; existing library names are retained.

## Recovered routine and ABI

`005B8C30` is `void __thiscall(VoiceManager*, float delta)`, with manager in ECX,
one stack float and `RET 4`. Its body is `005B8C30..005B8D94`; last instruction
`005B8D92: RET 4` is three bytes. The verified listing has 105 instructions and
zero gaps. The complete bounded behavior is reconstructed in
`src/voice_fade_update.cpp`, exposed through `include/bsp/voice_fade_update.hpp`.

The existing `VoicePlaybackManager` fields are sufficient: current `+D4h`,
target `+D8h`, rate `+DCh`, callback NativeString `+E0h`. The scalar helper,
actual NativeString copy/destruction, sound manager level application and
thread-safe MissionLua dispatch all reuse existing reconstructions.

## Math and completion

1. Multiply rate by delta through x87 and spill to float (`005B8C4B..69`).
2. Pass current by address, target and that step to `0042AC60` at `005B8C76`.
3. Compare the updated current to the CURRENT target (`005B8C7B..8B`). The
   `FUCOMIP / LAHF / TEST AH,44h / JP` sequence takes completion only for
   ordered equality. Signed zeros compare equal; NaN suppresses completion.

`unit_step_towards_0042ac60` is reused unchanged. It snaps only when step is
strictly greater than `abs(float(current-target))`; otherwise target>current
adds step and the other branch subtracts it. There is no new clamp or absolute
value on the supplied rate/delta. Negative steps can move away from the target,
including moving an initially equal value. A NaN target makes the direction
comparison false and takes subtraction; finite current can remain finite.
A NaN step ordinarily produces a NaN current and suppresses the callback.
These are inherited scalar semantics, not errors normalized by the fade code.

## Callback ownership and reentry

On completion, `0041E870` constructs a temporary from the empty CString at
`00CE3A0C` (verified first four bytes zero). The string wrapper `00449AF0`
compares the callback against this empty header. Its entire body is
`00449AF0..00449B3C`, last instruction `00449B3A: RET 4` (3 bytes), 30
instructions with zero listing gaps. Native ABI is bool/AL, ECX=left
NativeString, stack right NativeString, RET4. It returns the other length's
nonzero test when either length is zero; only when both are nonzero does it
call the existing CRT `__stricmp` at `00BF7FBF` and return result!=0. It does
not require equal stored lengths. No CRT or generic string comparison was
ported here. In this fixed-empty call, the result is exactly callback length
!=0, and the empty temporary allocates no storage.

For a nonempty callback the native sequence is:

| Address | Action |
| --- | --- |
| `005B8CDD` | Log `MoveSoundFade Callback` through `004254B0` |
| `005B8CEA` | Copy the CURRENT callback through `00426060` into an owned local NativeString |
| `005B8CF6` | Arm local cleanup state 0 after successful construction |
| `005B8CFE` | Assign empty CString through `0041E350`, which here reduces to resize(0,preserve=false) |
| `005B8D03/14` | Reload the current game singleton and its MissionLua host at `+1A08h` |
| `005B8D1C` | Call `00887E50` with `(self=null, name=&copy, args=null, first=0, last=-1)` |
| `005B8D21..43` | Release the temporary copy's current buffer using length+1 |

Logging can replace the callback before the copy; the reconstruction does not
capture its bytes early. It uses the actual-header
`copy_construct_native_string_header_00426060`, retaining allocation callbacks
and current-field rereads already implemented there. The original callback is
cleared BEFORE Lua dispatch, so reentry can install a new fade and callback
without the outer call clearing it afterward. The local cleanup guard is armed
only after successful construction and uses
`destroy_native_string_header_0041dd20` for normal and C++ exceptional exits.
If copying throws before construction completes, that guard is not armed,
matching the observed cleanup-state transition.

The concrete `call_named_entry_point_threadsafe` adapter passes the copied
name and an empty argument vector, retaining its existing lifecycle refusal
and worker-thread queuing. Its outcome is ignored, as in the native fade
caller: even a refused/queued call has already consumed the original callback.
The only remaining fade host services are logging and resolving the CURRENT
MissionLua service at that load site; no fake interpreter or success fallback
is supplied. C++ string conversion retains the stored name length.

## Sound publication and singleton reloads

After callback execution AND temporary cleanup, `005B8D48` rereads manager
`+D4h`. `005B8D4A` then loads sound singleton `00F8BBD8`. Ordered equality with
its `+6Ch` skips the update (`005B8D5A..65`); unordered comparison updates it.
When changed, the code stores the new fade value to that owner's `+6Ch`,
reloads the sound singleton at `005B8D73`, reads the reloaded owner's `+4Ch`,
and calls `00A7A440` at `005B8D7F` with that EXISTING value.

Thus the fade changes `SoundSystemOwner::level_6c`; it does not overwrite
`SoundManagerLevels::global_4c` with the fade. Reapplying the existing global
level invokes the recovered FFFF-class dirty propagation. The implementation
holds an explicit volatile reference to the sound singleton slot and performs
both native loads. Callback/cleanup changes to the current fade or singleton
are observed. Each loaded owner must remain valid through its use; no null
guard, cached owner or synchronization policy was invented.

## Integration interface

`update_voice_sound_fade_005b8c30(manager, delta, bindings)` takes
`VoiceFadeBindings{NativeStringStorage&, SoundSystemOwner* volatile&,
VoiceFadeHost&}`. The host supplies `log_004254b0(const char*)` and
`mission_lua_005b8d14() -> MissionLuaHostServices&`. The latter resolves the
current game+1A08 service after callback clearing, not at binding construction.
The parent integrator owns replacing `VoiceLineHost::update_fade_005b8c30`
with a bindings accessor and invoking the recovered free routine from
`begin_voice_sound_fade_005b9760`. No shared voice header/source was edited by
this worker.

## Validation and boundaries

* `./scripts/build.ps1`: MSVC Win32 Release passed; existing
  `reconstructed_math` CTest passed, 1/1. Log: `local/voice_fade_build.log`.
* One ignored fixture, `local/voice_fade_reentry_check.cpp`, compiled with
  MSVC Win32 `/fp:strict` and linked to the built `bsp_core.lib`, passed.
  Logging replaces the callback before copying; the concrete Lua wrapper
  executes a callback whose original field is already empty; that callback
  recursively updates a new fade, swaps sound owner and installs a later
  callback. Temporary cleanup changes the fade and owner again. Assertions
  verify fresh final reads, unchanged `+4Ch` gains, real dirty propagation,
  preserved later callback, restored Lua wrapper state and no live string
  allocations. Unexpected fixture Lua services throw. This is one focused
  behavioral fixture, not native differential or actual interpreter testing.
* No new shared test or test target, Ghidra mutation, missing function or flow
  repair was introduced. Proposed names and previous values are in
  `reports/voice_fade_update.json`.

This is a complete bounded semantic routine, not a native-layout/ABI replacement
or game-validated audio fix. Existing subsystem adapter limitations remain.
Signaling-NaN quieting/payloads, unmasked x87 traps, floating-point status/control
equivalence, asynchronous owner destruction and binary SEH equivalence are
outside the verified domain. No FMOD playback or real Lua engine was exercised.
