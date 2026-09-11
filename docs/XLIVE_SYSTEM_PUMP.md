# XLive manager pump and asynchronous helpers

Addresses: 00a409f0, 00a40510, 00a3ed10, 00a3ed60, 00a3ef20, 00a3fa70, 00a3e700.

This packet reconstructs seven complete control-flow bodies through explicit
host operations. Descriptive names are hypotheses, not recovered symbols.
The interfaces are typed C++ projections, not binary replacements for the
native manager or SDK. They do not emulate accounts, storage services or UI.

| Address | C++ function | Original ABI | Evidence |
| --- | --- | --- | --- |
| 00a409f0 | pump_xlive_system_00a409f0 | ECX manager, RET | 00a409f0..00a40acf; clock/x87 listing |
| 00a40510 | pump_online_signin_ui_00a40510 | ECX manager, RET | 00a40510..00a4099f; switch table follows |
| 00a3ed60 | download_online_storage_00a3ed60 | ECX manager, RET | 00a3ed60..00a3ef1d; raw free continuation |
| 00a3ef20 | upload_online_storage_00a3ef20 | ECX manager, RET | 00a3ef20..00a3f0fe; raw free continuation |
| 00a3fa70 | pump_online_achievements_00a3fa70 | ECX manager, byte force in DWORD stack slot, RET 4 | 00a3fa70..00a3fd1f; two raw free continuations |
| 00a3e700 | reset_online_ui_slots_00a3e700 | ECX manager, RET | Five instructions, final RET 00a3e71b |
| 00a3ed10 | build_online_storage_path_00a3ed10 | ECX manager, RET | 00a3ed10..00a3ed59; seven pushed SDK arguments |

All live queries used `bsp.py` against the existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. This worker made no Ghidra writes. False
`_free` no-return metadata leaves four flow gaps; bytes establish the
continuations, recorded in `reports/xlive_system_pump.json` for the integrator.
All seven complete code spans match the installed executable byte for byte;
their lengths and SHA256 values are recorded in that report.

## State ownership and indexed loads

`OnlineSystemState::state` is native +3B0, the six-case sign-in UI flow.
`XLiveSystemPumpState::storage_state_12c` is a separate storage transfer state.
The existing `pending_notifications` vector is the achievement ID queue at
+360/+364/+368; it is reused rather than duplicated under another name.

The profile overlap at +3C0 is exactly
`PlatformManagerFlags::profile_overlapped_3c0`. Session profile polling and the
UI helper therefore observe the same first DWORD. Storage +130 and achievement
+384 resets clear only five DWORDs; words 5 and 6 survive. Message-box setup
clears all seven DWORDs even when +3E8 already indicates visible system UI.
Do not move these objects or release their buffers while DLL operations are
pending. Destruction of the typed state is not an asynchronous cancellation.

The native storage guards read `[manager+8C+index*4]`. The index is +3B4 for
upload/download, +11C for path creation and the successful sign-in-info branch.
Live 00a3ebd0 establishes that +8C is the cached state for user zero and +90 is
the beginning of a 128-byte username; it is not a four-user status array.
`read_manager_dword_8c_indexed` must supply the exact cached bytes, including
the overlap for nonzero indices. Replacing it with an SDK state query changes
the native behavior. The enclosing username/XUID refresh is outside this packet.

New pump field initializers are safe host storage defaults, not claims about
00a40df0 constructor initialization. Other native paths may populate these
fields; the integrator owns their canonical bindings.

## Pump order and clock

00a409f0 drains notifications, pumps the +3B0 UI flow, samples clock virtual
+20 and discards that sample. It tests +12C for upload state 7, then reloads
+12C and tests for download state 3, then pumps achievements with force zero.
It reloads the clock singleton and samples again. The host must preserve both
samples and singleton reloads, including changes during earlier callbacks.

The second timestamp contains signed 64-bit ticks and frequency. Native x87
division spills to a float; subtraction from the previous float stays on x87
until comparison against **double 2.0** at 00D7A308. The source reuses the
canonical timestamp helper and retains the x87 subtraction/comparison. Equal,
negative and unordered differences do not trigger. The image byte 00E0E3EC
is initially 1; the first call captures the current float and clears that byte.
Both this byte and float 00F8ABFC belong to shared process state.

For elapsed time strictly greater than two seconds, readiness requires +8C
nonzero, except byte +119 with +11C zero disables readiness. A nonnull callback
+20 is invoked with **ECX=0 and no stack arguments**. The supplied startup host
must honor that callback boundary. The previous time is updated after the call,
or without a call when gated off; callback exceptions naturally skip that store.

## Storage and achievement behavior

Path creation calls `XStorageBuildServerPath(user,3,nullptr,0,L"DropRates",path,
&size)` with size 0x200 and manager path capacity 0x200 bytes. Only success
sets storage state 1. Upload prepares a nine-byte buffer containing DWORDs
+358/+35C and a zero final byte. Download prepares a zeroed 256-byte buffer and
a zeroed five-DWORD result. Submission accepts only result 0x3E5 as asynchronous
pending; immediate zero still follows native failure states 8/5 respectively.
Pending overlap status causes a progress query. Completed status causes
`XGetOverlappedResult(overlap,nullptr,TRUE)`. Successful download copies the two
DWORDs only when result DWORD zero is 9. Extended error 0x8015C004 selects state
10; other download errors select 5. Upload completion selects 9 or 8.

Achievement submission snapshots `{user,achievementId}` pairs from the current
queue. A pending overlap causes `XGetOverlappedExtendedError`, whose value
overwrites +3A8, even though the operation is still pending. Successful
completion removes one first matching queue entry for each uploaded pair,
preserving later arrivals and remaining duplicates. Empty queues cause an early
return even when a retained batch exists. Force submission can free and replace
an existing pending batch: this surprising native behavior is retained rather
than silently guarded.

Allocation uses host owners for the same replacement/release points. Successful,
valid-sized allocation is the reconstruction domain. Native allocation overflow
and failure eventually fault; the typed version throws on impossible sizes or
allocation failure. It does not reproduce the CRT/STL debug iterator machinery.
Native diagnostic target 004254b0 is a proven RET; those empty calls and their
unused argument computation are omitted. Existing 00a40020 reset is reused.

## UI boundaries and validation limits

Case 1 resolves the exact four localization keys through canonical NativeString
temporaries. Wide results survive the message-box call and unwind in reverse
order afterward, including exceptions. Their typed ownership does not claim
the native wide-string allocator ABI. The host passes a full SDK sign-in-info
structure and returns only its byte +8 flags, the only portion this body reads.
The full notification drain, localization implementation, actual DLL imports,
and account/service behavior remain required host operations, not default stubs.

The Win32 Release build and both existing CTests passed. The ignored local
fixture passed and exercises stable overlap storage, five/seven-word resets, storage
completion, duplicate achievement removal after queue growth, UI progression,
the strict heartbeat boundary and upload-to-download state reload. Such host
fixtures are not native differential or game validation; the integrator owns
the actual DLL runtime probe. No installed DLL was loaded or modified here.

## Integration from docs/PLATFORM_SERVICES.md

The parent now composes these routines through PlatformServices and XLiveManagerRuntime, with original-ordinal SDK forwarding and shared canonical state. Input lookup/reset/tick use one published pointer. The combined build and existing tests passed; installed XLive loader probes failed before pretranslation. Required owner construction, game dependencies and runtime validation remain explicit in docs/PLATFORM_SERVICES.md.
