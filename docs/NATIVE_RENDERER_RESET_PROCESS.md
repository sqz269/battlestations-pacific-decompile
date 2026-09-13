# Native renderer Reset processing

Addresses: `00B2ABD0`, `00BEC230`; consumed `00B33AD0`, `00B33B00`, `00B21110`, `00B29670`, `00B237D0`, `00B262C0`, `00B23B10`, `00B1FD90`, `00B24460`, `00B26170`, `00B21960`, `00B20C50`; EH `00CBD400`, `00CBD408`, `00DF5C50`, `00DF5C58`, `00BF6B43`.

The complete actual source is `src/native_renderer_reset_process.cpp`, with
its borrowed context in `include/bsp/native_renderer_reset_process.hpp`.
It matches the independently reviewed local draft byte for byte. The source,
current provider hashes and original native evidence are pinned in the JSON.

| Routine | Coverage | Original ABI | Source interface |
| --- | --- | --- | --- |
| B2ABD0..B2AE1C, 589 bytes | complete normal body and represented optional-guard cleanup | ECX actual renderer; no stack arguments; plain RET; no semantic result | `process_native_renderer_device_reset_00b2abd0` |
| BEC230..BEC233, 4 bytes | complete | ECX actual platform; EAX raw HWND from+30; no stack arguments; plain RET | `native_platform_window_00bec230` |

Descriptive names are hypotheses. The raw getter coexists with the existing
semantic `get_platform_window_00bec230(Win32PlatformState&)`; its raw storage
must never be cast to that semantic type. The thin Reset context borrows the
same AW recreation context, actual platform publication0109CF04, pending/lost
bytes0108D4B8/B9, retry DWORD0108D4C4 and render-thread DWORD0108D4C8.

## Native flow and current providers

Entry checks the current optional-guard mode, storing renderer then returned
AL only when entered. State0 arms at B2AC03 **before** real GetCurrentThreadId
at B2AC07. The current authorized-thread DWORD is read after that call.
Wrong-thread return still uses current-mode disarm and full saved-DWORD leave.
There is no separate lifecycle lock in this parent.

B2AC48 reads pending first; B2AC4E then reads lost even if pending is nonzero.
Both zero exits. Cooperative result starts at0, and only captured lost calls
the current device/table+0C TestCooperativeLevel. HRESULT88760868 increments
the actual retry DWORD with wrap, stores it and marks renderer+1D8A. Unsigned
result>10 additionally requires captured platform active+41 and real GetFocus
matching that captured platform's HWND+30. Only then is retry zeroed and full
AW B29670 called. This path has no Sleep calls.

HRESULT88760869 bypasses the second pending read. Every other result, including
initialized0 when lost was zero, rereads pending at B2ACD3 and exits if now0.
The following inline focus test again captures the current platform before
GetFocus and reads HWND+30 from that captured pointer after return. Failure
returns immediately through optional cleanup; it does not enter late fallback.

After focus, B2AD1A captures the immutable Sleep import. Result0 or88760869
performs this sequence:

1. Mark renderer+1D8A, then real Sleep100.
2. Full B237D0 dynamic release and B262C0 resource/binding/cache release, using
   the existing actual renderer contexts and shared synchronization globals.
3. Real Sleep100; freshly reload platform0109CF04 and call raw BEC230; store
   EAX directly to renderer+1A44.
4. Reload current device and table; call Reset+40 with the **actual writable**
   presentation block at renderer+1A28. Only HRESULT exactly0 is success.
5. On success clear1D8A, set1D90=2, call full B23B10 resource restore and B1FD90
   dynamic readiness, set stateA1 from fresh1A38!=0, then full B26170 defaults.
6. FLD renderer196C before reading current renderer profile/slot+F0; FSTP into
   the outgoing float DWORD, then full B21960 gamma. Clear pending, then lost.
7. Real final Sleep100, then current-mode optional cleanup.

Another cooperative result or failed Reset instead reaches B2ADD2: reload the
current platform and call full raw B20C50. Only its AL enables AW B29670; both
true and false predicate paths then execute final Sleep100. The source checks
the complete provider's return value, which is equivalent because B20C50
returns EAX exactly0/1. Completed resource/Reset work is not rolled back.

The gamma selector was freshly captured at D5F198: D5F0A8+F0 contains B21960.
The current complete gamma provider borrows the actual outgoing volatile float
slot and original FP constants. No ordinary C++ floating conversion, host pow,
extra clamp or output normalization is substituted.

## ABI and raw imports

The source COM calls use Win32 stdcall: TestCooperativeLevel has one stacked
DWORD including the device receiver (RET4), and Reset has two (RET8). B24460
has state/value stack slots and native RET8; gamma and guard leave each have
one native stack slot and RET4. BEC230 is exactly `8B 41 30 C3`.

Fresh PE import parsing and live/disk DWORD checks establish:

| Original IAT cell | Exact import | Arguments |
| --- | --- | --- |
| CE2230 | KERNEL32.dll!Sleep | DWORD100, stdcall RET4 |
| CE223C | KERNEL32.dll!GetCurrentThreadId | none |
| CE2310 | USER32.dll!GetFocus | none |

The captured unbound PE values are import metadata, not loaded process API
addresses. Real immutable API bindings remain an explicit runtime condition.

## Exception metadata

CBD400..CBD407 is the existing unwind action: `LEA ECX,[EBP-14]`, then JMP
B21110 at CBD403. Raw handler CBD408..CBD411 is `MOV EAX,DF5C58; JMP BF6B43`,
with the jump at CBD40D. The integrator defined and saved its exact ten-byte function under the write
lock; current readback is FUN_00cbd408, body CBD408..CBD411. This is an
analysis-only dependency, not an additional reconstructed normal-body claim.

DF5C50..DF5C57 is the sole unwind-map pair `(-1,CBD400)`. DF5C58..DF5C7B is the
36-byte FuncInfo with one state. BF6B43 retains its existing library identity.
The only exceptional cleanup is B21110 on the original guard. A skipped guard
is uninitialized; enabling its cleanup later is outside the valid caller
domain. The source does not establish original FH3 frame identity or general
hardware-SEH behavior.

## Producers and corrections to planning notes

There are **two** direct Reset caller functions: B2AEB0 initialization calls at
B2B1D2, and B2B200 BeginFrame calls at B2B229. The earlier readiness note listed
only BeginFrame. Initialization gets the actual thread ID at B2B01A and stores
it at B2B021. It **sets pending to1** at B2B1CC before calling Reset; the earlier
note's description of that instruction as a clear was incorrect. Whole-function
EBX/BL filtering establishes EBX=1 from B2AF41 with no later write before that
store. Mode B29FBC also sets pending1, from EBX=1 at B29F70.

Constructor B32410 clears pending/lost at B327B7/B327BD using BL=0 established
by B32439. Present at B2DB7A is compared with88760868 at B2DB7C, setting lost1
at B2DB83 on equality. Successful Reset clears pending/lost in that order.
B2AEB0 initializes the actual38h presentation block at renderer1A28 via
BF79F0 at B2AEDC, then writes the actual device parameters. No duplicate
presentation owner or private counter is introduced.

All six BEC230 callsites were inspected. Reset and the joystick, keyboard,
mouse and descriptor paths pass the actual platform from0109CF04 and consume
the raw EAX window result; no extra getter arguments are inferred from pushes
belonging to later COM calls. General platform publication/constructor and
complete renderer startup/frame lifecycle remain outside this packet.

## Verification boundary

Eight fresh spans, **671 bytes**, match the installed PE and the verified live
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`: both full bodies, EH
action/handler, map/FuncInfo, the three raw IAT cells and gamma slot. The report
records all22 native Reset CALL instructions, its EH action, both direct Reset
callers and the other five raw getter callers:30 host rows total. Numeric rows
are mechanically checked; import/COM roles and the gamma profile are separately
established by their captured data and original instruction setup.

After this static draft was prepared, the separate `reset_fixture_ax` worker
completed `C:/Users/sqz269/bsp-ax-renderer-reset/fixture_report.json`. Its capture
reports14 parent pairs and14 raw getter pairs,113904 normalized state bytes,
6384 event DWORDs, six real HAL Reset calls, four full AW calls and one native
FH3 exception pair. The companion draft links and hashes that report without
claiming this evidence task reran the fixture. Root also reports an initial
combined build and two passing existing tests.

**The final merged source/library replay is still pending.** Tracked-file
transfer, naming/annotation, integrated build evidence and final validation
belong to the root integrator. These drafts establish neither original binary
ABI compatibility, arbitrary concurrent mutation/SEH, nor gameplay validity.

## AX integration analysis refresh

The integrator saved all four AX original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. Three ten-byte
EH handlers CBCC0E, CBD348 and CBD408 were defined under owned leases and
the Ghidra write lock. Missing-function observations above describe the
earlier worker capture. EH definitions are analysis metadata, not additional
reconstructed normal-body claims. Combined final-commit validation remains
separate from the worker fixture evidence.
