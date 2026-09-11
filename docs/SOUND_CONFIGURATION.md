# Lua sound configuration

`initialize_sound_configuration_00a7ff80` recovers the second stage called at
00A88A8E by the existing audio startup. It reads the sound script, configures
FMOD groups and DSPs, appends listeners and categories, and resolves sound-type
routes. The C++ names are hypotheses, not recovered symbols.

The implementation reuses `SoundSystemState`, `FmodResult`, `GuiLuaHost`,
`NativeString`, `SoundClassDescriptor`, `SoundClassOwnership`, and the canonical
`SoundManagerLevels::classes_98` table. It does not duplicate the mutable enabled
byte or global level at manager+4C. Neither is changed by this stage. Master
channel-group handle +140 is a different field from the master volume setting.

## Evidence and ABI

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, verified by
the repository bridge before each export/query batch. Worker analysis was read
only. The integrator repaired three returning `_free` call sites under the
Ghidra write lock and refreshed exports: 00A81361, 00A813A2 in the main routine,
and the tail of 00A7F160. The former 00A813A7..00A813F8 gap contains temporary
cleanup and the next `SoundTypes` iteration, not a first-record return.

| Address | Native interface / recovered responsibility |
| --- | --- |
| 00A7FF80..00A8145B | `__thiscall(ECX=manager)`, no stack args, RET at 00A8145B length 1; complete initialization policy |
| 00A7DE10..00A7E0C3 | `__thiscall(manager, group-record*, NativeString*, LuaObject*)`, RET 0C at 00A7E0C1 length 3; configure one group |
| 00A7E0D0..00A7E239 | `__thiscall(manager, LuaObject*)`, RET 4 at 00A7E237 length 3; configure System DSP list |
| 00A7C740..00A7D116 | `__thiscall(manager, LuaObject*)`, RET 4 at 00A7D114 length 3, DSP pointer in EAX; five DSP policies |
| 00A7F9F0..00A7FAC1 | `__thiscall(manager+A4, listener-record*)`, RET 4 at 00A7FABF length 3; append listener policy |
| 00A7AE00 | `__thiscall(manager+A4, const char*)`, RET 4, first listener ordinal or **0 on miss** |
| 00A7AE80 | `__thiscall(manager+A4, const char*)`, RET 4, clear/select current listener pointer |
| 00A7B120 | `__thiscall(manager, NativeString*)`, RET 4, group handle or null |
| 00A814A4..00A81554 | scalar-default stores inside 00A81480; constructor fragment only |

The original imports' Ghidra prototypes omit the explicit object pointer. Stack
pushes, rather than those prototypes, establish the FMOD arguments. In particular,
00A7C740 returns EAX, not the pseudocode's `float`, and its numeric parameters are
x87 values narrowed to binary32 before the imported call. Assembly also confirms
that System matching is case-sensitive, via 00B660F0, while DSP type and stored
name matching use CRT `__stricmp` through the existing name-host interface.

## Initialization order and defaults

1. Query `getMasterChannelGroup(system+44, &manager+140)` at 00A8000A.
2. Construct/open the existing Lua owner with mask **1**, base library only,
   at 00A80026..00A80042. Load `sound/soundsetup.lua` with argument 0 through
   00B66CA0 at 00A800A1. This stage does not inspect a success result. The existing
   loader uses an unprotected Lua call; no recovery was added.
3. Read the thirteen globals below in table order. Only Lua type 3 NUMBER writes
   the field. Nil, booleans and numeric strings preserve the current value.
4. Zero a 44h FMOD advanced-settings buffer, set its size word to 44h, get settings,
   overwrite only HRTF min/max/frequency and set settings back (00A80790..00A8082A).
   Their actual buffer offsets are **24h, 28h, 2Ch**. Two intervening PUSHes make
   the uncorrected pseudocode appear to use different locations. Other words
   returned by FMOD survive unchanged.
5. Iterate `Channelgroups`, `Listeners`, `Categories`, then `SoundTypes` using
   Lua's existing iterator order. Do not sort by key, numeric index or name.
6. Destroy the references and close the Lua owner at 00A81441.

| Global | Manager offset | Constructor default |
| --- | --- | --- |
| PlayerSoundBoost | 04 | 1 |
| HRTFMinAngle | 08 | 180 |
| HRTFMaxAngle | 0C | 360 |
| HRTFFreq | 10 | 5000 |
| DopplerConstant | 14 | 2000 |
| DopplerMin | 18 | 0.5 |
| DopplerMax | 1C | 2 |
| SoundDistance1 | 20 | 800 |
| SoundDistance2 | 24 | 1200 |
| SoundDistance3 | 28 | 1800 |
| SoundDistance4 | 2C | 2200 |
| PassDistance | 30 | 200 |
| PassTime | 34 | 3 |

The initial values come from MOVSS stores and image constant bytes, not assumed
Lua defaults. The full 00A81480 constructor is not reconstructed here. +10C is
initialized to -1 at 00A815EF; +110 is not initialized by the visible constructor,
so the projection's zero initialization of that otherwise unread field is a host
convenience until the first listener switch writes it.

## Groups and DSP policy

`Channelgroups["System"]` reads only its `DSP` member, requires it to be a table,
and attaches each nonnull created DSP to the FMOD system. It does not create a
named group and ignores that entry's `Volume`/`Pitch`. Other keys name a new 18h
group record: copy name, create group, set `Volume` (default 1), set `Pitch`
(default 1), then iterate table-valued `DSP`. There is no `addGroup` call.

Both DSP walkers append the DSP handle before calling `addDSP`, passing a null
connection-output pointer. An unknown `Type` returns null and is not appended.
After create, the handle alone gates parameter calls; an error result with a
nonnull handle still configures it. Number-only parameter lookup uses the already
recovered 00B66330 policy. Values are not clamped by the game wrapper.

| Type string (case-insensitive) | FMOD type | Parameter index order and defaults |
| --- | --- | --- |
| Lowpass | 3 | Cutoff=5000, Resonance=1 |
| LowpassSimple | 19 | Cutoff=5000 |
| Distortion | 8 | Level=0.5 |
| Compressor | 17 | Threshold=0, Attack=50, Release=50, Gain=0 |
| SFXReverb | 18 | DryLevel=0, Room=0, RoomHF=0, RoomRolloff=10, DecayTime=1, DecayHFRatio=0.5, ReflectionsLevel=10000, ReflectionsDelay=0.02, ReverbLevel=0, ReverbDelay=0.04, Diffusion=100, Density=100, HFReference=5000, RoomLF=0, LFReference=250 |

The unusual positive `ReflectionsLevel=10000` is proven by 00A7CDBC reading
00CE3D64 = `00 40 1C 46`; `RoomRolloff=10` reads 00CE38B8 = `00 00 20 41`.
These are the game's fallbacks even if the installed FMOD version rejects them.

Every imported result is tested only against **2Bh**, including master/advanced
settings, DSP creation/parameters and group creation/properties/attachment. A
match calls `FMOD_Memory_GetStats` into discarded locals, with two output
pointers in this build. The named 00A7A460 diagnostic used in some DSP paths
ignores its string/ECX and performs the same two-output stats call. Other errors
do not stop the sequence. This is not an error-handling recommendation.

## Listeners, categories and routes

`Listeners` takes names from **values**, allowing null conversion to an empty
name. 00A7F9F0 compares equal-length nonempty names against existing entries but
ignores the comparison results and appends duplicates. Lookup uses the first
case-insensitive match; a miss returns 0. If the returned ordinal differs from
+10C, old +10C moves to +110, new ordinal moves to +10C, and 00A7AE80 selects
the listener. That selector first clears its pointer at +104, then sets pointer
and ordinal +108 on a match; a miss leaves +108 alone. The host represents the
pointer as an ordinal to preserve identity across vector relocation.

`Categories` creates the existing 20h sound class, copies `value.Name` (nil ->
empty), reads numeric `value.Volume` or 1, sets class +08 to the current table
count, and appends through `SoundClassOwnership`. The table retains one reference
after both temporary references drop. No dirty-entry walk or global-level change
occurs here. Classes keep constructor +0C=0 and +14=1.

`SoundTypes` takes a type name from each **outer key**, and maps each inner
**listener-name key** to a **channel-group-name value**. The group lookup checks
stored length before case-insensitive comparison and returns null on miss.
The per-type vector grows through the resolved listener ordinal and fills new
holes with null. A missing listener therefore targets slot 0; repeated mappings
that resolve to the same listener overwrite that slot in Lua iteration order.
Each type record is appended, even with an empty map. The repaired tail proves
all outer entries are processed.

The projection records observed capacity policy: group records grow by 16 with
minimum 16; global DSP handles by 16/minimum 16; group DSP handles by 4/minimum 8;
listener and type tables double/minimum 1. Class storage uses its recovered owner.
The temporary route vector's allocator details and native record copy/destruction
are represented by standard C++ storage; 00A7F160 and the other generic reserve
bodies are analyzed dependencies, not claimed reconstructed by this packet.

## Boundaries and validation

These are new C++ interfaces, not ABI-compatible replacements. FMOD, Lua owner
loading/VFS, Lua 5.1.1 and CRT comparisons remain actual external contracts. No
FMOD or Lua library code was reimplemented. `GuiLua51Host` can execute the
installed script for the fragment entry without another interpreter model.

Native temporary diagnostic strings and their pool traffic are omitted. Native
listener reference counts and group/type NativeString headers are projected as
C++ ownership, while category ownership uses the reconstructed class owner.
There is no FMOD-release policy in this initializer; returned handles remain
owned by the sound system/library and must be handled by its teardown path.

As in native code, group names, DSP `Type`, and inner sound-type names must be
string-convertible. Some native paths dereference the conversion result without
a null check. No invented malformed-input fallback was added. Container counts
and growth arithmetic must fit signed 32-bit range; allocation failures and
corrupted negative counts are outside the projection's equivalence claim.
Missing outer tables follow the supplied established Lua iterator's behavior.

Validation results are recorded in `reports/sound_configuration.json`. Build,
fixture execution, installed-data/FMOD checks and in-game validation are distinct.
The worker's Win32 Release build and both existing CTests passed after all eight
native math seed ranges matched the installed executable. One ignored local
fixture used the real repository Lua 5.1.1 with a recording FMOD host to check
the repaired multi-type loop, duplicate listeners, numeric-string defaults,
advanced-settings preservation, error continuation and final class ownership.
Installed-script/real-FMOD execution is the integrator's separate validation.
