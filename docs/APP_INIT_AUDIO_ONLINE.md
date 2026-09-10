# Application init, audio and online phase

Addresses: 00a88770, 00a79230, 00a40df0, 00a87060, 00a7a460, 00a77ef0, 00a40020

Packet `app_init_audio_online`. Covers the sound-system bring-up, the streamed-dialog definition
table and the XenonSystemManager / Games for Windows LIVE bring-up reached from
`cSkeletonAppMidway::Init` (`BSP_Application_Initialize`, 0073d410). The Bink and DirectSound
imports next to these calls are out of scope.

Reconstruction: `include/bsp/audio_online_startup.hpp`, `src/audio_online_startup.cpp`.
Machine-readable form of everything below: `reports/app_init_audio_online.json`.

## State reached

| Address | Ledger name | State |
| --- | --- | --- |
| 00a88770 | BSP_SoundSystem_Initialize | exported, analyzed, reconstructed, build-tested |
| 00a79230 | BSP_DialogStreamTable_Load | exported, analyzed (object layout only) |
| 00a87060 | BSP_DialogStreamTable_Parse | exported, analyzed, reconstructed, build-tested |
| 00a40df0 | BSP_XLiveSystem_Initialize | exported, analyzed, reconstructed, build-tested |
| 00a40020 | BSP_XenonSystemManager_ResetSignInState | exported, analyzed, reconstructed, build-tested |
| 00a7a460 | BSP_SoundSystem_ReportFmodMemoryFailure | exported, analyzed |
| 00a77ef0 | BSP_DialogChannelRecord_Copy | exported, analyzed |

None of it is fixture-tested, ABI-compatible or game-validated. The C++ is a behavioural model
with injected hosts, not a drop-in binary replacement.

## How this was read

The pseudocode for 00a88770 is not usable on its own: it carries register inputs (`unaff_ESI`,
`unaff_retaddr`), reuses the first argument slot as scratch after the `operator new` at 00a887b4,
and turns the speaker-mode jump table into a `switch` on the wrong value. Every branch, argument
order and offset below comes from the disassembly. 00a40df0 decompiles cleanly and was checked
against the listing only for the `XLIVE_INITIALIZE_INFO` field offsets and the `RET 8`.

## 00a88770 sound system bring-up

`__thiscall(ECX = object, byte sound_disabled, dword, dword)`, `RET 0xc`. The object is
0x178 bytes (`PUSH 0x178` at 0073dac8). Two of the three stack dwords are never read; the caller
at 0073daee..0073dafd pushes `SETZ` of `DAT_00f889a4 == 0` plus two zeroes.

00a887b1 stores the negation of the byte argument into +0x70, so +0x70 means **sound enabled**.
`DAT_00f889a4` has no writing cross-reference in the image, so what sets it is unresolved; it sits
in the same globals block as `DAT_00f889a8` and `DAT_00f889b0`, which the settings step at
0073db4x consumes.

### Object fields written

| Offset | Value |
| --- | --- |
| +0x000 | vtable 00d5b44c |
| +0x044 | `FMOD::System*` |
| +0x048 | `FMOD_EVENTSYSTEM*` |
| +0x054 | `operator new(0x18)` with ctor 00a858f0 |
| +0x070 | sound-enabled byte |
| +0x118..+0x124 | four dwords copied from `DAT_01090ab0` vtable +0x14 |
| +0x158 | `getDriverCaps` minfrequency, then overwritten with 1 |
| +0x15c | `getDriverCaps` maxfrequency, then overwritten with 440000 (0x6b6c0) |
| +0x170 | game speaker layout, 1..6 |
| +0x174 | 0 |

The base constructor at 00a81480 owns everything else.

### FMOD linkage and calling convention

`fmodex.dll` is imported by its mangled C++ names, all carrying the `QAG` qualifier, for example
`?setSpeakerMode@System@FMOD@@QAG?AW4FMOD_RESULT@@W4FMOD_SPEAKERMODE@@@Z`. `QAG` is a public
`__stdcall` member, so `this` is pushed as the first stack argument and the callee cleans up. That
matches the call sites exactly: the system pointer is the last thing pushed before every `CALL`.

The event system is different. `FMOD_EventSystem_Create` is the C import at 00c2df80, and the two
event-system calls dispatch through a function table at offset 0 of the returned handle rather than
through an import:

- 00a88844, slot 7 (+0x1c), one out-pointer. The value it writes into +0x44 is the `this` of every
  later `FMOD::System` import, so this is `getSystemObject`.
- 00a88931, slot 0, four arguments `(0x200, 0x10, NULL, 0)`, made after the system is fully
  configured and before anything is played, so this is `EventSystem::init(maxchannels, flags,
  extradriverdata, eventflags)`.

The slot indices are what is proven. The public FMOD C++ headers declare these classes with
non-virtual methods, so the mapping from slot to documented name is behavioural, not structural,
and the exact FMOD Ex version is unknown. `0x10` is `FMOD_INIT_HRTF_LOWPASS` in the FMOD Ex 4.x
headers; treat the flag name as provisional and the value as proven.

### Call sequence

1. 00a8881e `FMOD_EventSystem_Create(&this->event_system)`.
2. 00a88844 slot 7, `getSystemObject(&this->system)`.
3. Only when +0x70 is set:
   - 00a8886d `getNumDrivers(system, &count)`.
   - 00a88890 seeds a stack `FMOD_SPEAKERMODE` slot with 1 so it survives a machine with no drivers.
   - 00a888a0..00a888be `getDriverCaps(system, id, &caps, &this[0x158], &this[0x15c],
     &control_panel_speaker_mode)` counting **down** from `count - 1` to 0 inclusive. Results are
     never tested, so the values that survive belong to driver 0.
   - 00a888c8 `setSpeakerMode(system, control_panel_speaker_mode)`.
4. 00a888eb, 00a888f1 unconditionally: +0x158 = 1, +0x15c = 440000. The queried frequency range is
   discarded. The hypothesis that these are a min/max frequency clamp is not proven.
5. Only when +0x70 is clear: 00a88904 `setOutput(system, 2)` = `FMOD_OUTPUTTYPE_NOSOUND`. This is
   the whole disabled-sound path.
6. 00a88931 slot 0, `init(0x200, 0x10, NULL, 0)`.
7. 00a88961 `setFileSystem(system, open 00a7d410, close 00a7b750, read 00a79930, seek 00a79970,
   blockalign 0)`.
8. 00a88994 `set3DSettings(system, dopplerscale DAT_00ce69c8 = 0.3f, distancefactor 1.0f,
   rolloffscale 1.0f)`.
9. 00a889b6, 00a889d8, 00a889fa `getDriver`, `getOutput`, `getSpeakerMode` into stack slots.

**Driver selection**: there is no `setDriver` anywhere in the routine, and the driver index and
output type read back in step 9 are written to stack slots that are never read again. The driver
enumeration exists only to pick a speaker mode.

**Channel groups**: none are created here. `getMasterChannelGroup`, `getAdvancedSettings` and
`setAdvancedSettings` appear in the follow-on stage 00a7ff80, called at 00a88a8e, which is a large
Lua-driven configuration pass (it also calls `BSP_LuaReference_GetFloatOrDefault`). That stage is
outside this packet.

### Speaker-mode mapping

00a88a19 bounds the value at 7 and jumps through the eight-entry table at 00a88aac:

| `FMOD_SPEAKERMODE` | Target | +0x170 |
| --- | --- | --- |
| 0 RAW | 00a88a63 | not written |
| 1 MONO | 00a88a25 | 1 |
| 2 STEREO | 00a88a2d | 2 |
| 3 QUAD | 00a88a35 | 3 |
| 4 SURROUND | 00a88a41 | 4 |
| 5 5POINT1 | 00a88a59 | 5 |
| 6 7POINT1 | 00a88a4d | 6 |
| 7 SRS5_1_MATRIX | 00a88a59 | 5 |

### Failure path

Every FMOD result in the routine is compared against `0x2b` and nothing else. `0x2b` is
`FMOD_ERR_MEMORY`, which the string at 00d5ab94, `Out of sounjd memory:` (misspelt in the shipped
image), confirms. On a match the code loads `DAT_00f8bbd8` into ECX, pushes 00d5ab94 and calls
00a7a460. That routine is `__stdcall(const char*)`, `RET 4`, and it ignores both the argument and
ECX: its whole body calls `FMOD_Memory_GetStats` into two discarded stack locals and returns. So in
this build the diagnostic never reaches a log, and no FMOD failure ever aborts initialisation or
changes the control flow. Every other FMOD error code passes unnoticed.

## 00a79230 and 00a87060 streamed dialog table

00a79230 is `__thiscall(ECX = object)`, `RET 0`, on a 0x234-byte object (`PUSH 0x234` at 0073db02).
It is the streamed-dialog manager constructor; the load is one step inside it.

| Offset | Value |
| --- | --- |
| +0x000 | vtable 00d58f78 |
| +0x004 | `operator new(0xc)`: count 1 plus one 8-byte element (ctor 00a77d10) |
| +0x008 | `operator new(0x5c)` with ctor 00a78150(0) |
| +0x00c | `operator new(0x5c)` with ctor 00a781c0(0) |
| +0x010..+0x1bf | 0x12 records of 0x18 bytes (ctor 004c87d0, dtor 004c87f0) |
| +0x1c0..+0x207 | 0x12 dwords (ctor 00a778c0, dtor 0052e020) |
| +0x208 | -1 |
| +0x210 | 0 |
| +0x214, +0x215 | bytes 1 and 0 |
| +0x218..+0x224 | 1.0f each (DAT_00d7a24c) |
| +0x228, +0x230 | 0 |
| +0x22c | the reference-counted definition table below |

The definition table is `operator new(0x20)`, vtable written as 00ceb130 then immediately as
00d58f80, refcount 1. Its layout: +0x08 vector data, +0x0c count, +0x10 capacity, +0x14 volume
(1.0f), +0x18 running channel total, +0x1c loop byte. Any previous table at +0x22c is released
through `InterlockedDecrement` on its +0x04 before the new one is stored.

### Data source

The path is the literal `sound/streamed_dialogs.def` at 00d58f88, built as a 26-character native
string at 00a79365 and handed to 00a87060 with the definition table in ECX. **This is not Lua**: it
is a plain whitespace-delimited text file read through a shared tokenizer.

The tokenizer object is 0x828 bytes, created by `operator new` at 00a8707e and constructed by
00bef2e0. Token text lives at +0x001, the previous token at +0x401, a cached-token flag at +0x801,
end-of-stream flags at +0x804 and +0x805, the caller-visible end-of-file flag at +0x806, and the
source stream at +0x824 (read through its vtable +0x24). 00bee8e0 is an idempotent **peek**: it
returns early while +0x801 is set. 00bee800 is **advance**: it copies the token to +0x401 and
clears +0x801. Values are taken with 00bef020 (string) and 00bef170 (float), which consume.

### Grammar

```
Channels
  <name> mono|stereo|51
  ...
end
Volume <float>
Loop
```

Keywords are matched with `_stricmp`, so they are case-insensitive. `mono`, `stereo` and `51` map
to format indices 1, 2 and 3, and the channel count comes from the four-entry table at 00e12ef0,
whose entries 1..3 are 1, 2 and 6.

### Record layout

Records are 0x14 bytes in the vector at table +0x08. 00a77ef0 places each one and 00a872c0 patches
the running total in afterwards.

| Offset | Field |
| --- | --- |
| +0x00 | name length |
| +0x04 | name pointer |
| +0x08 | channel count, 1 / 2 / 6 |
| +0x0c | first channel, the value of table +0x18 before this entry |
| +0x10 | format index, 1 / 2 / 3 |

The vector grows through 00a78dc0 with a `max(1, capacity * 2)` policy.

### Two recovered hazards

Both are faithful readings of the listing, not decompiler artefacts.

1. A top-level token that is none of `Channels`, `Volume` or `Loop` is never consumed, so the outer
   loop at 00a870f0 spins forever. The loop only terminates on end of file, on an empty token, or
   through one of the three keywords.
2. Inside a `Channels` block, a format token that is none of `mono`, `stereo` or `51` is also not
   consumed. The record is appended with a channel count of 0 and the next iteration reads that
   token as the next channel's name.

The reconstruction reproduces the structure but returns `DialogParseOutcome::unrecognized_token`
instead of spinning on case 1. Case 2 is reproduced exactly.

## 00a40df0 XenonSystemManager and Games for Windows LIVE

`__thiscall(ECX = object, dword, dword)`, `RET 8`, on a 0x3f0-byte object (`PUSH 0x3f0` at
0073dc50). The caller pushes 00735510 and 00735520 into +0x20 and +0x24. Those two are not
registered functions: each is a 14-byte stub `PUSH 6 | PUSH 4; PUSH 0x8001; PUSH ECX;
CALL 00a6d3f2; RET`. Both call the same target, which Ghidra places inside the function starting at
00a6d380, a routine that ends in `luaD_call`. What the stubs actually do is an open question.

Right after the call, 0073dc87 stores 00737d60 into `(*DAT_00f8abe8) + 0x18`, which belongs to the
next packet.

### XLiveInitializeEx

The 28-byte block built at 00a40ef1..00a40f41 and passed at 00a40f46:

| Offset | Value | Provisional name |
| --- | --- | --- |
| +0x00 | 0x1c | cbSize |
| +0x04 | 0 | |
| +0x08 | `BSP_D3D9Renderer_GetDevice` (00b1fef0) on `DAT_00f8d394` | pD3DDevice |
| +0x0c | `DAT_00f8d394 + 0x1a28` | pD3DPresentParameters |
| +0x10 | `GetUserDefaultLangID()`, a WORD | wLanguage |
| +0x12, +0x14, +0x18 | 0 | |

The offsets and the values are proven by the stores. The field names follow the published
`XLIVE_INITIALIZE_INFO` layout and are provisional. The second argument is the literal
`0x20029900`, an SDK version token; it is recorded as a literal rather than decoded.

The `HRESULT` is discarded: nothing branches on it.

### Rest of the sequence

1. 00a40eae log ` ...  XenonSystemManager Start Initialization  ...` (00d2435c) through 004254b0.
2. 00a40f46 `XLiveInitializeEx`.
3. 00a40f50 log ` ...  XenonSystemManager XLiveInitialized  ...` (00d2432c).
4. 00a40f5e 00a4c250 on the sub-object at +0x3ac. It returns `E_INVALIDARG` (0x80070057) for a null
   target, otherwise the result of 00a4c030 and a masked write-back. Purpose unresolved.
5. 00a40f63 `XOnlineStartup()`.
6. 00a40f72 `XWSAStartup(0x0202, &wsadata)` into a 400-byte `WSADATA`. 00a40f77..00a40f82 checks the
   low word byte by byte and calls `XWSACleanup` unless it is exactly 2.2.
7. 00a40f94 `XNetSetSystemLinkPort(XSocketNTOHS(0x0c02))`. 0x0c02 is 3074, the Xbox LIVE system link
   port, byte-swapped into network order.
8. 00a40fd4 `XNotifyCreateListener(qwAreas = 0x2f)` (pushed as `0, 0x2f`) into +0x1c. 0x2f is
   `SYSTEM | LIVE | FRIENDS | CUSTOM` plus bit 0x20; the individual bit names come from the public
   `XNOTIFY_*` set and are provisional. 00a40fd9 and 00a40fe0 reject both 0 and -1 before logging
   ` ...  XenonSystemManager XNotifyCreateListener  ... SUCCEED ` (00d242ec).
9. 00a40ff4 `BSP_XenonSystemManager_ResetSignInState` (00a40020).
10. 00a40ffb 00a409f0, one pump of the manager's state machine. It reads the platform timer through
    `DAT_01090ab0` vtable +0x20 (a pointer to a `{counter, frequency}` pair), branches on the state
    word at +0x12c being 7 or 3, and invokes the +0x20 stub when +0x8c is set and the sign-in gate
    at +0x119 / +0x11c allows it. Not modelled.

### Sign-in state handling

00a40020 is the reset path and is also reused outside initialisation. It calls the stub at +0x20
only when +0x8c is non-zero; clears +0x119, +0x11a and sets +0x11c to 1 unless +0x3bd is set; sets
+0x124 to -1; empties the four-byte-stride vector at +0x364 / +0x368 by moving its end back to its
begin; sets the state word at +0x3b0 to 0 and logs ` ...   XenonSystemManager  ChangeState To %d`
with 0; then sets +0x3b4 and +0x3b8 to 1. The meaning of each individual sign-in slot is inferred
from this routine and from 00a409f0 only, so all of them are named after their offset.

### When XLive is absent

There is no fallback. `xlive.dll` is a plain static import (the thunks at 00a4d46a..00a4d5de and
00c2f1c0.. jump through the ordinary import address table, not a delay-load stub), so the process
cannot start without it, and 00a40df0 does not check any XLive return value. Nothing in this
routine degrades gracefully.

## Callers and callees

- 00a88770: called from 0073dafd. Calls 00a81480, 00bf681b, 00a88650, 00415350, 00bd0d70,
  `DAT_01090ab0` vtable +0x14, the FMOD imports listed above, 00a7a460, 00a858f0, 00a7ff80.
- 00a79230: called from 0073db2b. Calls 00a778d0, 00bf7cd1, 00bf681b, 00bf55be, 0041dd40, 00bf7680,
  00419cc0, 00bd1510, 00a87060, 00a78150, 00a781c0.
- 00a87060: called from 00a793ac. Calls 00bf681b, 0041dd40, 00bf7680, 00bef2e0, 00bee8e0, 00bee800,
  00bef020, 00bef170, 00bef220, 00bf7fbf (`_stricmp`), 00419cc0, 00bd1120, 00bd1510, 00a78dc0,
  00a77ef0, 00bf6989.
- 00a40df0: called from 0073dc7c. Calls 00a3f530, 004254b0, 00b1fef0, `GetUserDefaultLangID`,
  00a4d5de, 00a4c250, 00a4d5d8, 00a4d5d2, 00a4d5cc, 00a4d494, 00a4d5c6, 00a4d5b4, 00a40020, 00a409f0.

## What remains

- The event-system function table. The slot-to-name mapping is behavioural; the FMOD Ex version and
  the provenance of the vtable dispatch (the public headers are non-virtual) are unresolved.
- `DAT_00f889a4` has no writing cross-reference. What turns sound off is unknown.
- +0x158 = 1 and +0x15c = 440000: values proven, meaning not.
- The four dwords copied from `DAT_01090ab0` vtable +0x14 into sound system +0x118..+0x124.
- 00a7ff80, the Lua-driven second stage that owns the channel groups and advanced settings.
- The 0x18-byte object at sound system +0x54 (ctor 00a858f0) and the 0x10-byte object built at
  00a887b4 (ctor 00a88650), which is never stored into the sound system by this routine.
- The two stubs at 00735510 and 00735520 and their shared target inside 00a6d380.
- 00a4c250 / 00a4c030, the sub-object at XenonSystemManager +0x3ac.
- 00a409f0, the state-machine pump, and the meaning of the individual sign-in slots.
- The 0x18-byte records at dialog manager +0x10 and the two 0x5c-byte objects at +0x08 and +0x0c.

## Divergences in the reconstruction

- `load_dialog_stream_table` returns `DialogParseOutcome::unrecognized_token` where the original
  spins. Everything else about the loop is reproduced.
- The vector growth policy is left to `std::vector` instead of 00a78dc0.
- The two sub-object allocations in 00a88770 and the follow-on call to 00a7ff80 are not modelled.
- FMOD, XLive and the tokenizer are injected interfaces, so nothing is initialised for real.
