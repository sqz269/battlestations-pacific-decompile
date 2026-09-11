# XLive IPC lifecycle and worker

This packet reconstructs `00A4C030` creation, `00A4BDE0` destruction, `00A4BC80` last-error normalization, the complete `00A4BE50` worker loop, thread entry `00A4C000`, and callbacks `00A4BD40`/`00A4BD80`. Descriptive names are hypotheses, not recovered symbols. Evidence comes from read-only queries of `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and bounded raw disassembly of the installed executable. It supplies the real lifecycle needed by `XLiveManagerOwnerHost`; it does not start any IPC endpoint during validation.

## State, allocation, and creation

`XLiveIpcNativeState` is exactly 2Ch bytes on Win32, with these DWORD slots:

| Offset | Retained state |
|---|---|
| 00 | opaque pipe owner |
| 04 / 08 | worker thread / stop event handles |
| 0C / 10 / 14 | receive buffer / capacity / received count |
| 18 / 1C / 20 | send buffer / capacity / send count |
| 24 / 28 | worker phase / unsigned counter |

The host allocation also stores pointers to its actual pipe/system services. Those pointers are fixed context, not invented native fields, and must outlive every callback and worker. The constructor writes neither native +14 nor +20. The concrete allocator uses uninitialized nothrow allocation; those preimages are preserved until a worker writes them. Buffer allocations use the corresponding nothrow array allocation and matching deallocation. Native BFD012 is a JMP to BFD017, the CRT nothrow-new wrapper; no CRT implementation is ported.

`create_xlive_ipc_00a4c030` rejects null output with E_INVALIDARG. An initial allocation failure stores null output and returns E_OUTOFMEMORY. After a successful allocation it clears all other slots, sets counter=1, clears last error, and creates an unnamed manual-reset, initially nonsignaled event with `CreateEventA(null, TRUE, FALSE, null)`.

It calls A5DE34 with mode0, that event, and the actual +0 output slot. Success sets phase1. A5DF96 then receives `(pipe, 8, &size)`; `size+4` is stored as the send capacity. Arithmetic wraps as a DWORD, followed by an unsigned comparison against 400h. Excessive size or allocation failure clears that capacity and takes failure cleanup. A5DFCE supplies the receive capacity in the same manner. Its nonnegative result is retained in EDI and becomes the constructor's success return value, rather than an invented constant S_OK.

After clearing last error again, `CreateThread(null,0,A4C000,object,0,null)` creates the actual worker. The worker can execute before +04 is written and before the caller's output pointer is published. Only after thread creation succeeds is the owner assigned to output.

Every later failure takes the hidden A4C1BE..A4C211 cleanup: close nonnull pipe, close nonnull event, free/null receive buffer, free/null send buffer, free owner, and **call `_exit(0)`**. It does not return a conventional error or write output first. The `_exit` is visible in raw bytes at A4C20C/A4C20D but hidden from the decompiler after `_free`. No owner/resource RAII cleanup is added on exceptions thrown by required hosts; these native bodies have no C++ unwind scope.

`00A4BC80` returns GetLastError(), substituting 507h only for zero. Creation's HRESULT conversion maps positive errors to 80070000h | LOWORD(error), preserving negative bit patterns; these later failures still exit after cleanup. Null output and initial allocation failure are the only normal failure returns.

## Worker and callbacks

The recovered worker uses the same actual retained buffers and phase field. It does not manufacture completed I/O or protocol output.

| Phase | Native action and next state |
|---|---|
| 1 | Sleep(3A31h = 14897ms), GetSystemTime into discarded stack output, phase2 |
| 2 | Capture send buffer, set phase0, encode into buffer+4 with available capacity-4 and callback A4BD40. Success writes returned length+4 to +20 and returned length into the captured buffer's first DWORD. Callback controls the next phase. |
| 3 | Send the complete frame. Only 8000000Ah is accepted as pending: phase4 and reset EDI=0. Any other result clears phase; negative exits, nonnegative reaches invalid-phase E_UNEXPECTED. |
| 4 | Wait for send with 5000ms timeout. Successful output must equal current +20; mismatch becomes E_UNEXPECTED. Success phase5; failure phase0/return. |
| 5 | Receive into +0C/+10. Only pending 8000000Ah advances to phase6 and resets EDI. |
| 6 | Wait for receive with 5000ms timeout into actual +14. Success phase7; failure phase0/return. |
| 7 | Clear phase; require received count >=4 and frame header == received-4. Decode payload at buffer+4 using A4BD80. A nonnegative result loops with the callback's current phase; failure clears phase and returns. |

An unsupported phase returns E_UNEXPECTED. Successful library calls must define their consumed output counts, and buffers must retain their actual allocations. The projection explicitly rejects absent or shorter-than-four-byte frame storage at the two game-side header accesses. Native arithmetic wrap is preserved; invalid native memory access is not emulated. The protocol provider remains responsible for its actual framing bounds and output contract.

Encode callback A4BD40 is stdcall `(buffer, available*, object)`, RET12. Capacity below8 returns HRESULT 8007007Ah without writes. Otherwise it writes DWORD8, then `counter+27h`, increments the unsigned counter with wrap, sets phase3, and returns0. It does **not** modify the available count. Decode A4BD80 is stdcall `(buffer, bytes, object)`, RET12: require bytes==8, first DWORD==8, and bitwise complement of the second DWORD equal to current counter. Success sets phase1; failures return E_UNEXPECTED without changing phase.

Thread entry A4C000 is stdcall `(object)`, RET4. It runs the complete loop and accepts only E_ABORT (80004004h) as normal shutdown. Every other result calls `TerminateProcess(GetCurrentProcess(),8000FFFFh)`. If that API returns, the original loop result is returned. IAT evidence is CE2228 -> GetCurrentProcess and CE2224 -> TerminateProcess; the IAT stores import-name RVAs A05714/A05700, whose image-base-adjusted names are at E05714/E05700. These are actual kernel32 operations, not XLive forwarding.

## Destruction and concurrency boundary

Null owner is ignored. A nonnull owner always signals its stop event, waits on its worker for exactly1000ms, ignores the result, closes/nulls the thread, closes/nulls the pipe, closes/nulls the event, frees/nulls receive and send buffers, then frees the owner. No extra join, thread termination, retry, or timeout branch is present.

The native one-second wait does **not** establish worker completion. In particular phase1 can sleep14897ms without consulting the stop event, and a still-running worker can access freed state after teardown. This packet preserves that native behavior. Safe use of a host requires independently established worker completion/lifetime before native destruction releases its state; that requirement is a host-domain limitation, not a claim that the native code waits safely. No actual worker, event, pipe, process exit, or process termination was executed by the fixture.

## Named-pipe provenance and remaining operations

`PIPEIPC_*` names are analyst-assigned inventory labels with **medium** `block_pipe_ipc` tags, not recovered vendor/library symbols. Existing comments describe an unidentified, differently aligned block A5DE34..A607E8. Direct function bodies and direct CALL instructions establish that these are linked into the executable rather than IAT thunks or xlive.dll exports; they do not establish third-party authorship.

Independent evidence establishes named-pipe functionality: A5E844 formats `\\.\pipe\%08x`, opens it through CreateFileW, calls SetNamedPipeHandleState, and creates synchronization events. A5F204/A5F230 add48h framing overhead to payload capacity; A5F6C0/A5F7C5 call the game callbacks after that envelope and contain additional state/cryptographic operations, including CryptGenRandom. Exact protocol/framing implementation and library provenance remain unreconstructed. `XLiveIpcPipeHost` requires those genuine operations; it supplies no DLL ordinal guesses, fake handles, alternate named-pipe protocol, or successful defaults. These names do not exempt the bodies from future reconstruction.

Bounded follow-up candidates (not claims of independence or active leases):

| Packet | Address ownership | New files |
|---|---|---|
| Pipe I/O | A5DEAA, A5DEE8, A5DF20, A5DF5E; direct bodies A5E1F7, A5E28B, A5E31F, A5E7E6, A5E815 | `include/bsp/xlive_pipe_io.hpp`, `src/xlive_pipe_io.cpp`, `docs/XLIVE_PIPE_IO.md`, `reports/xlive_pipe_io.json` |
| Framing entry | A5DF96, A5DFCE, A5E055, A5E09E; A5F204, A5F230, A5F6C0, A5F7C5 | corresponding `xlive_pipe_framing` header/source/doc/report |
| Pipe owner | A5DE34, A5DE68, A5E145, A5E710, A5E844; consider E3D0/E557/E6CD after review | corresponding `xlive_pipe_transport` header/source/doc/report |

The framing entry packet depends on EE0D/EE78/EF36/EFA1/F020/F25C/F2A4/F397/FA5A; owner construction/destruction depends on F371/F416 and their children. These are substantive remaining work, not callbacks with default results. I/O and owner packets share the actual30h pipe-owner fields, while framing owns a protocol subobject. The parent must settle one canonical state definition and explicit ownership before overlapping parallel work. Existing caller metadata can include stale flow-derived edges (for example A5DE68 lists E750, while current assembly calls E710); assembly must decide scope.

## Analysis repairs and validation

Three real entries are missing: A4BD40 ends at RET12 A4BD75 (length3, inclusive end A4BD77); A4BD80 ends at RET12 A4BDAF (length3, inclusive end A4BDB1); A4C000 ends at RET4 A4C029 (length3, inclusive end A4C02B). None is an invented interior case label.

False no-return gaps: A4C1EA..A4C1EF after A4C1E5; A4C1FD..A4C202 after A4C1F8; A4C209..A4C211 after A4C204; A4BE2F..A4BE47 after A4BE2A. The latter contains further frees at A4BE36/A4BE3F. The last creation gap contains the **genuinely nonreturning `_exit(0)`** call A4C20D; do not add an invented normal fallthrough from it. Detailed lengths, endpoints, and parent-only repair commands are in the JSON report.

MSVC Win32 Release and both existing CTests passed. One ignored fixture executes the real thread entry synchronously against scripted services: it checks the full callback-driven phase cycle, pending/error behavior, creation output publication, untouched count preimages, fatal cleanup, and destruction after simulated WAIT_TIMEOUT. Its envelope values are scripted test inputs, not a reconstructed authentication protocol. Logs: `local/xlive-ipc-final-build.log`, `local/xlive-ipc-fixture.log`. No DLL, IPC endpoint, OS thread, process-exit, or process-termination behavior was exercised; no game or binary ABI compatibility is claimed.
