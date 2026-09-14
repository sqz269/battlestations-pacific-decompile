# Native loading queue owner entries

Packet `orch4_native_loading_queue_owner_bl` reconstructs `004FDE20`,
`004FDBA0`, `00504850` and `00BD1920` as new MSVC Win32 C++ interfaces in
`native_loading_queue_owner.hpp/.cpp`. Baseline:
`96d910d50a2e98317bb45227d830544ef1727a3d`. The [report](../reports/native_loading_queue_owner_bl.json)
records exact installed/live byte spans, call sites, source and fixture hashes.
Descriptive names are hypotheses, not original recovered symbols.

## Storage and borrowed ownership

`NativeLoadingQueueOwnerStorage` is the actual `20h` layout, with compile-time
checks for every offset. Its fields are numeric profile `CEB198` at `+0`,
worker stop DWORD `+4`, published worker job pointer `+8`, actual
`NativeEventOwnerStorage*` at `+0C`, raw job-pointer array `+10`, signed count
`+14`, capacity `+18`, and thread HANDLE `+1C`. Job allocations are actual
`24h` storage; this packet only accesses their state DWORD `+0` through the
raw first pointer. It adds no queue, projected job, callback registry, private
global, thread or deleting owner.

The getter borrows the actual mutable `01090AA0` lifetime-manager cell and
`00E18D4C` loader cell as stable references. It uses existing native manager
getter/registration, allocation/free, actual event creation and tracked
critical-section cleanup. The table DWORD is an original address identity;
it is not a callable source vtable.

## Recovered entries

| Entry / exact normal span | Native ABI | Source behavior |
|---|---|---|
| `4FDBA0..4FDBF8`, 89 bytes | ECX raw owner; EAX same owner; RET | Stamp `CEB198`, zero `+4/+8`, create actual event with CL=1, then store event and zero `+10/+14/+18/+1C` |
| `4FDE20..4FDEDC`, 189 bytes | No inputs; EAX owner; RET | Capture fast publication, otherwise capture first manager's section, acquire/recheck/construct/publish/register, then release captured section and reload publication |
| `504850..504862`, 19 bytes | Two unread name arguments; RET8 | Get current loader, read current array and front pointer, write front state2 |
| `BD1920..BD1951`, 50 bytes | ECX actual event; raw AL result; RET | Load current HANDLE, wait with timeout0 and preserve native status-to-AL mapping |

The constructor leaves `+0C..+1C` untouched until event creation returns.
CL=1 creates an initially unsignaled manual-reset event. Returning-null event
allocation remains a null event; failed CreateEvent retains an actual event
owner with a null HANDLE. Neither outcome gains a success check or rollback.

The slow getter captures manager `+10` before entering. A nonnull section is
entered and its raw DWORD `+18` incremented before state0 begins. It then
rechecks the loader cell. A null publication allocates `20h`, constructs, and
publishes the result before the second manager lookup. Only after that call
does it reread the loader cell for `BD0C30`. Publication during acquisition
skips allocation and registration. A returning-null allocation publishes null
and still performs the second manager lookup and registration call; the
existing registrar validates manager bounds before its null-object branch.
The captured section is decremented and left even if manager publication has
changed. The slow return reads `E18D4C` after LeaveCriticalSection; the fast
return uses its first captured read. Volatile fields/cells preserve this read
schedule; they do not claim C++ interthread atomic synchronization.

The FileStore callback ignores both incoming names. It captures no originating
job and does not read count or stop state. A changed front receives state2;
an invalid/empty domain has no added guard. Added publication references make
the source function a new C++ interface, not a callable native RET8 thunk.

Poll returns `1` for `WAIT_OBJECT_0`; `0` for `WAIT_TIMEOUT`,
`WAIT_ABANDONED_0`, and `WAIT_FAILED`. Only failure additionally calls
`GetLastError`, whose result is discarded. Any other status returns its low
byte unchanged: for example `0x100 -> 0`, `0x123 -> 0x23`. A bool result would
lose that behavior. Upper EAX and original register/flag identity are not the
source return contract.

## Exception evidence and retained state

Constructor handler `C68808..C68811` selects FH3 FuncInfo `D91980` (36 bytes),
whose single unwind-map entry at `D91978` is state0 -> state-1, funclet
`C68800..C68807`. That funclet reloads saved this from `[EBP-10h]` and jumps
to `4F93A0..4F93B0`: unconditionally clear **current** `E18D4C`, then write
`CE3818` to this. The source constructor expresses these exact stores on a
synchronous C++ exception. This is the bounded constructor base cleanup,
not implementation or registration of the full loader deleting destructor.

Getter handler `C68833..C6883C` selects FuncInfo `D919B4` (36 bytes). Its
two-entry map at `D919A4` is state0 -> state-1 through `C68820`, and state1
-> state0 through `C68828`. State0 begins after lock acquisition/depth
increment. State1 begins only after allocation returns, and ends before
publication and second lookup/registration. `C68820..C68827` passes the
saved eight-byte guard `[EBP-14h]` to `411EE0`. `C68828..C68832` frees the
saved allocation `[EBP-18h]`, then POP ECX/RET.

Construction failure therefore clears current loader publication and restores
the base profile, frees the captured outer allocation, releases the captured
guard, and rethrows. Allocation failure only releases the guard. A later
registration failure releases the guard while retaining the publication,
event and outer allocation. Subsequent getters can return that publication
without another registration attempt. No private RAII owner silently deletes
the acquired state, no rollback clears it, and no incomplete destructor is
used to make a fixture appear leak-free.

These C++ catches do not reproduce FH3/SEH tables, original stack-spill
aliasing, hardware-fault unwind, CRT throw identity or provider register ABI.
Initial inspection found both FH3 handler entries without a Ghidra function.
At that inspection `C68828` ended at `C68830`: its decoded `C68831..C68832`
POP ECX/RET continuation had no containing function. Exact live bytes matched
the installed PE. This packet performed no Ghidra mutation, script enablement,
restart, no-return edit or dry-run clearing.

The primary subsequently claimed and repaired the cleanup separately, with
evidence in `reports/native_loading_queue_owner_cleanup_bl.json`. A final
read-only membership check now finds both `C68831` and `C68832` inside
`Unwind@00c68828`; its live listing has all five instructions through RET.
That repair supersedes the initial cleanup-body hole above. Its parser-safe
`void __cdecl(void)` analysis signature does not establish the native inherited
EBP/FH3 callable ABI. The two handler entries remain outside this packet's
definition/annotation work.

## Validation and integration boundary

All 16 recorded normal-body, cleanup, FH3-data and table spans match live
Ghidra and the installed PE. Each live batch used `bsp.py ghidra` and its
project/program verification for `bsp.gpr`, `/battlestationspacific.exe`,
x86 and image base `00400000`. All ten direct CALL/tail-JMP report rows pass
`verify_report_calls.py`; the four Win32 IAT calls are separately recorded.

The new translation unit compiled with `/std:c++17 /W4 /WX /fp:strict /EHsc
/MD /O2` for Win32. An ignored smoke fixture linked it against a frozen copy
of the primary's existing `bsp_core.lib`, whose before/copy/after SHA256 values
agree. It exercised actual raw-manager registration, fast getter retention,
the actual manual-reset event, changing front-job pointers and failed poll.
Fixture teardown explicitly frees its own test allocations; it does not claim
to be the native loader's missing lifecycle.

A second ignored fixture instruments service calls only in its own copy of
the source translation unit. It checks initialization timing, manager changes
during event construction, publication before registration, final read after
Leave, publication during Enter, constructor failure, retained publication
after registration failure, and eight raw wait-status values. These are
source behavior checks with fixture providers, not original instruction
execution. Production exposes no injection hooks. No permanent test added.
The worker baseline build was started then interrupted at the primary's
direction; it did not include the unregistered new source. The primary must
register this translation unit and run the integrated Win32 build/checks.

Production use remains gated. `CEB198[0]` is actual `50ACE0`; no deleting
dispatcher is installed. Getter/callback wiring must wait for actual
`509FD0/50ACE0/5092E0` and their drain/FileBlock/resource ownership domain.
The update/worker/resource-loading bodies and worker exit lifetime remain
separate dependencies. This packet is reconstructed and focused fixture
tested; it is not a binary replacement or game-validated loader.
