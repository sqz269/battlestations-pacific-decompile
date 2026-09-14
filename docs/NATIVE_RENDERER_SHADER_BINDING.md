# Actual renderer shader binding

The two raw providers implement B21D10 (vertex) and B21C20 (pixel) for the actual
renderer storage consumed by B28D00. Existing D3D9StateCache methods retain their
projected interfaces; the additional raw APIs are recorded as fragments.

| Entry | Full inclusive range | Cache | Device slot | Counter |
| --- | --- | --- | --- | --- |
| B21D10 | B21D10..B21DF1, 226 bytes | +1770 | +170 | +1BC0 |
| B21C20 | B21C20..B21D01, 226 bytes | +176C | +1AC | +1BBC |

Both native entries consume ECX renderer and a stack logical shader pointer,
then RET4. The new C++ APIs additionally borrow the existing actual
NativeRendererSynchronizationGlobals storage corresponding to 0108D6DC..E3.
No invented renderer, global, cached-state projection or callback adapter is
introduced. Shader pointers are borrowed, with no AddRef/Release operation.

Entry reads current mode before the cached logical pointer. When enabled it
saves the actual renderer and the low-byte result of the existing substantive
B33AD0 guard provider. Cached logical capture occurs after that provider returns;
the exception cleanup state is armed before reading either logical COM field.
Both nonnull logical objects compare old+8 before incoming+8. Equal COM pointers
skip publication and dispatch, preserving the old logical identity, including
when both stored COM values are zero. This is not logical-pointer comparison.

Every other path publishes incoming into the actual cache first. Null-null also
stores zero, then skips device access and the counter. Nonnull incoming captures
current renderer+1A10 device, reloads incoming+8, then reads the captured device's
current vtable and bind slot. The null transition captures current device/table
and sends a null shader. Both slots use actual stdcall(this,shader) dispatch.
HRESULT is ignored, including failure; returning calls increment the CURRENT
counter with DWORD wrapping. A callback's cache/counter/device changes remain
observable; no original cache or counter value is restored.

Normal exit rereads current mode, disarms exception cleanup, then optionally
calls the existing actual B33B00 with saved renderer and the defined result byte.
The native ignored DWORD contains padding that B33B00 does not consume; the
source passes only the defined byte. Skipping entry leaves the native guard
record unwritten. Entry-disabled/exit-enabled transitions are outside the native
valid domain and are not repaired. Entry-enabled/exit-disabled transitions
retain the resulting nesting and lock state, as the current-mode guard dictates.

Exceptional C++ cleanup calls the existing actual B21110 provider, which checks
current mode before reading saved fields. A cleanup exception terminates.
There is no rollback of cache publication, device effects or prior counter work.
Original register/private-frame/FH3/asynchronous-SEH behavior is not claimed by
these source interfaces. Full body bytes, original guard handlers and call sites
are retained in the report; the worker makes no Ghidra mutation. Root owns names,
old-comment preservation, saves and refreshed exports.

The focused fixture compares copied complete original binders with these source
symbols using actual renderer/logical storage, scripted COM slots and a real
tracked Win32 critical section. Its explicit original-handler relocation limits
native execution to normal paths; a separate source-only throwing COM case
checks the C++ cleanup boundary. These are local behavioral checks, not game or
full original exception validation. The archive separates actual library/tool
files from measured loaded runtime DLLs; post-run file hashes do not establish
mapped-image identity.

## Integrated validation at 425b3b48

Both original/source normal binder traces matched with real Win32 guards and current cache/device/counter effects. Source-only C++ cleanup failures passed. Original FH3 exception execution and real shader creation remain unproved. The combined strict Win32 build, eight seed checks and both CTests passed.
The four final-library probes,118 direct/tail rows, seven saved/read-back
annotations and38 live/PE spans are retained in `local/checkpoints/425b3b48/native-renderer-constructor-wave/validation.json`
(SHA256 `64c19259048ada5f8868750e99c793c0b95674ea7f391b124b8ed2e613ae8fe0`). Full parent execution and application/gameplay
validation remain open.
