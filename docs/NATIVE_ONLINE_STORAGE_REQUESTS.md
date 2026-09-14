# Actual online storage path and request entrypoints

Addresses: 00a3ed10, 00a3f4a0, 00a3f500

This packet adds three complete normal bodies to the exact 3F0h
`NativeOnlineManagerStorage` from `native_online_notifications.hpp`. The
earlier A3ED60/A3EF20 transfer bodies in `native_online_storage.cpp` remain
the invoked implementations; no projected `XLiveSystemPumpContext` or
`XLiveOwnerAllocation` is used. Names are descriptive hypotheses. Analysis
used the live program `/battlestationspacific.exe` in
`C:/Users/sqz269/bsp.gpr` and the installed PE at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.

| Body | Win32 original ABI | Inclusive end | Coverage |
| --- | --- | --- | --- |
| A3ED10 path producer | ECX=captured owner, RET | A3ED59 | complete normal body |
| A3F4A0 upload request | ECX=captured owner, two stacked DWORDs, RET 8 | A3F4FA | complete normal body |
| A3F500 download request | ECX=captured owner, RET | A3F52F | complete normal body |

A3ED10 reads `owner+11C` into EAX once. It uses that value as the index for
the **DWORD** gate at `owner+8C+4*user`; when that word is not 2 it returns
without calling the SDK or changing the state. On the permitted branch it
initializes a stack-local byte capacity to 200h and calls the seven-argument
`XStorageBuildServerPath(user, 3, nullptr, 0, L"DropRates", owner+154,
&capacity)`. The target is thunk A4D578 (`JMP [CE26CC]`), XLive ordinal 5344,
Win32 `__stdcall` with 28 stack bytes, as corroborated by the prior installed
SDK import report. The SDK owns any write to the borrowed 200h-byte UTF-16
path span `+154..+353`. The body writes state 1 at +12C **only** if the SDK
return is zero. It does not clear a partly written path on error, copy the
output capacity into the manager, or return the SDK error. The UI pump at
A40937 calls A3ED10 after a successful sign-in-info query and an indexed
state-2 gate; it sets UI state +3B0 to 6 whether the path builder succeeds
or fails. The raw UI pump and manager lifetime wiring remain separate.

A3F4A0 reads its first stacked DWORD, writes it to +358, snapshots +12C,
then reads and writes its second stacked DWORD to +35C. Its CMP/SBB/AND
sequences implement the identity even for zero; there is no clamping or
conversion. If the captured state is 1, 9, 4, 5, 8 or 10 it calls the
actual raw A3EF20 upload body with the same ECX owner. That body prepares
the **current** two DWORDs into a nine-byte payload (two little-endian
DWORDs, one zero byte) only for its narrower state set 1/9/4/5/10. Thus
the wrapper's state-8 call intentionally performs no transfer. The wrapper
always reaches diagnostic 4254B0 after its conditional call; the inspected
callee is a single `RET`, and its argument is discarded by the caller, so
there is no log callback or observable queue side effect to provide.

A3F500 snapshots +12C and calls the actual raw A3ED60 download body only
for states 1/4/9/5/8. The download body prepares only in states 1/4/9,
so requests in 5/8 intentionally do nothing. A3F500 then reaches the same
empty diagnostic target. Neither request entrypoint allocates or copies
data itself beyond the two +358/+35C writes; all buffer, overlap, result,
failure and pending policies are retained in the invoked raw transfer
bodies. They need the same loaded XLive module, matching game-CRT allocation
pair, and stable captured manager/buffer/path through pending completion.

The live xref to A3F500 is A46B2A inside function A46930, which reloads
F8ABE8 before the call, following its selected-user/sign-in-state and XUID
checks. The live xref to A3F4A0 is A48E53. The seven byte disk/live
window A48E51..A48E57 is `52 52 E8 48 66 FF FF` (two pushes, then call),
but the incoming A48E31 branch pushes ESI and jumps directly to A48E52,
so the first argument is either ESI or EDX by path; A48E52 pushes EDX
as the second. Ghidra reports **no containing function** at A48E53. A stale cached
caller graph assigned this call to A48450, whose proven body ends A488CF;
that attribution is corrected here. No enclosing start/end, ECX owner
provenance, or upstream caller semantics are claimed for the unassigned
region. It is reserved for a separate listing/control-flow investigation.

These are reconstructed source bodies, not drop-in binary replacements or
game-validated behavior. The path must be built before a real transfer, and
the raw UI pump, selected-user path, A48E53 upstream dispatch, manager
lifetime, and game event validation remain for full event closure. The
local fixture uses only in-memory SDK callbacks; no network request, upload
or external send was made. Initially, another worker leased the shared
`cmake/startup.cmake` registration. After that lease cleared, one serialized
transaction registered both the inherited `native_online_storage.cpp` and
this packet's `native_online_storage_requests.cpp` in `bsp_core`. The default
`scripts/build.ps1` Win32 Release build compiled both sources and linked
`bsp_core.lib` and `bsp_game.exe`. The configured `reconstructed_math` CTest
passed (1/1); this worktree has no `local/seed_reference.hpp`, so the optional
native differential CTest is not configured here. The two reports' live
call-site checks passed at 18/18 and 7/7 respectively. This proves build and
fixture behavior, not game event validation or an actual XLive request.
