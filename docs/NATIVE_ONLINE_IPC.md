# Native online IPC endpoint

This packet reconstructs the actual 2Ch endpoint used by manager initialization A40DF0 and destruction A3F9D0. The caller supplies its existing DWORD slot at manager+3AC. No manager is allocated or projected here, and no service pointers are appended to the endpoint. Descriptive C++ names remain hypotheses; these are source interfaces rather than drop-in register-ABI replacements.

| Original routine | Native ABI | Coverage |
|---|---|---|
| A4C250..A4C27B initialize slot | ECX=DWORD output slot, RET, signed EAX | Complete |
| A4C030..A4C224 create endpoint | ECX=output pointer, RET, signed EAX | Complete, including hidden cleanup and exit |
| A4C280..A4C28E close handle | ECX=handle, RET or tail jump | Complete |
| A4BDE0..A4BE4A destroy endpoint | ECX=endpoint, RET | Complete, including hidden free tail |
| A4BC80..A4BC8F last error | No arguments, RET, DWORD EAX | Complete |
| A4BD40..A4BD77 encode callback | stdcall(buffer, available*, endpoint), RET12 | Complete |
| A4BD80..A4BDB1 decode callback | stdcall(buffer, bytes, endpoint), RET12 | Complete |
| A4BE50..A4BFDF worker loop | ECX=endpoint, RET, signed EAX | Complete seven-state loop |
| A4C000..A4C02B thread entry | stdcall(endpoint), RET4, DWORD EAX | Complete |

Read-only BSP queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. All nine complete byte spans match the installed PE. A4C030 is called only by A4C250, itself called by A40DF0. A4C280 is called by A3F9D0 and tail-jumps to A4BDE0. A4C000 is the data operand passed to CreateThread; its sole loop call targets A4BE50. The loop passes A4BD40 and A4BD80 as callback addresses. These callers and exact RET cleanup establish the interfaces.

## Raw storage and creation

`NativeOnlineIpcStorage` aliases the existing exact `XLiveIpcNativeState` layout, not the larger projected `XLiveIpc` owner. The producer requests precisely2Ch bytes. Fields are pipe+00, thread+04, stop event+08, receive pointer/capacity/count+0C/+10/+14, send pointer/capacity/count+18/+1C/+20, phase+24 and counter+28. Creation leaves +14 and +20 untouched, clears the other fields, and sets counter1.

A4C250 rejects null slot, calls creation using a local output, then writes that pointer for a nonnegative result or zero for a negative result. It does not destroy the old slot. A4C280 ignores zero and FFFFFFFF; closing a valid handle never clears the caller's retained slot.

Creation rejects null output with80070057. Initial nothrow allocation failure writes null and returns8007000E. It clears last error and creates an unnamed manual-reset, initially nonsignaled event. Pipe open receives mode0, that current event and the endpoint's actual +00 output slot. After success, phase becomes1. Send and receive capacity calls each receive channel8 and the same local DWORD. Each capacity is added to4 with unsigned wrapping; a result above400h or failed allocation clears the corresponding capacity and takes fatal cleanup. The nonnegative receive-capacity result is preserved as the final success result, including nonzero success.

CreateThread receives null security, zero stack/flags/id, the actual raw endpoint parameter, and the reconstructed stdcall entry. It can execute before the constructor writes +04 or publishes output. Thread failure converts GetLastError into an HRESULT; zero becomes507h before conversion. Every failure after the first allocation closes/non-null-clears pipe, event and both buffers, frees the endpoint, then calls `_exit(0)`. Output remains unwritten. No exception unwind or invented error return is added.

The memory pair is separate from the projected system allocator. Native BFD012 jumps to BFD017, whose nothrow-new wrapper calls BF681B; BF6989/BF65AC are matching CRT frees. The default source binding uses the selected MSVC CRT's nothrow scalar new and the same module's free. Its installed `new_scalar.cpp` uses malloc and new-handler retry; `new_scalar_nothrow.cpp` catches exceptions and returns null. A replacement allocator requires a matching release function. This does not establish equivalence between every CRT version's allocation-failure policy.

## Worker, framing and shutdown

Phase1 sleeps3A31h milliseconds, calls GetSystemTime into a discarded local, and sets phase2. Phase2 captures send capacity minus4 and the current buffer, clears phase, and invokes encode with buffer+4, a local size, callback and actual endpoint. Successful encode writes returned size+4 to +20 and returned size to the captured frame header. The callback writes DWORD8 and counter+27h, increments the current unsigned counter, sets phase3 and leaves the available count unchanged. Available below8 returns8007007A before any write.

Phase3 submits the current send pointer/count. Only8000000A advances to4 and resets the retained status to zero; all other statuses clear phase. Phase4 waits5000ms, compares the returned size with current +20 on success, and selects phase5 or zero. Phase5 starts a receive with current pointer/capacity; only8000000A advances to6. Phase6 waits5000ms into the actual +14 DWORD and selects phase7 or zero. Phase7 clears phase, requires count>=4 and header==count-4, then calls decode with the retained buffer payload and actual endpoint. Successful decode preserves the callback's current phase; failed decode clears it. Invalid phase/framing yields8000FFFF. Negative operation results otherwise return unchanged.

Decode requires exactly8 bytes, marker8 and complement of the second DWORD equal to current counter; success sets phase1. The source adds no buffer-capacity check absent from native instructions. In particular it does not inherit the projected loop's `require_frame_buffer` exception. Buffer accessibility remains an actual provider/owner precondition; wrapped arithmetic is preserved.

The thread accepts only80004004 as normal shutdown. Any other loop result invokes TerminateProcess(GetCurrentProcess(),8000FFFF); if that API returns, the original loop result is returned. Destruction signals the current event, waits exactly1000ms, ignores the wait result, closes/nulls thread, closes/nulls pipe/event, frees/nulls receive and send buffers, then frees the endpoint. Loads before null stores follow the listing. A timeout is not proof of thread exit: phase1 can sleep14897ms. The native behavior can free storage still used by a worker; this reconstruction adds no longer join or new cancellation behavior.

## Concrete source service binding

PIPEIPC names are analyst labels for linked executable bodies, not DLL imports. No missing DLL or ordinal is invented. `make_win32_native_online_ipc_runtime` accepts the existing substantive `ReconstructedXLivePipeServices`, whose protocol, framing, transport and I/O implementations call real Win32/Crypto APIs. It combines those services with the existing real Win32 system host and the matching memory pair. Opaque child pipe pointers retain that service implementation's ownership and are never reinterpreted as raw2Ch endpoints. The dependency contracts and existing limitations are recorded in `XLIVE_PIPE_SERVICES.md`, `XLIVE_PIPE_TRANSPORT.md`, `XLIVE_PIPE_PROTOCOL.md`, `XLIVE_PIPE_FRAMING.md` and `XLIVE_PIPE_IO.md`.

The application supplies verified protocol tables and canonical protocol/frame globals, including shared key and acquisition lock, before starting endpoints. The endpoint constructor itself creates no alternate protocol, success stub or fake worker. This packet does not certify the projected child pipe owner as binary-layout-compatible or re-prove its cryptographic/environmental parity.

The native thread parameter contains no room for a service context. The source binding therefore installs one immutable process-wide runtime pointer before any real endpoint creation. This is an explicit source-host adaptation for the original fixed code/global environment. Binding the same final runtime address again succeeds; binding a different address throws and leaves the original binding intact. The runtime, pipes, tables, global owners and system dependencies must remain alive until process exit. There is no unbind operation, owner identity map, per-endpoint context or hidden thread payload allocation. Moving/copying the runtime after binding or changing its services while workers run violates this boundary. Entering the thread before binding terminates; it does not pretend to succeed.

Capacity and sent-byte local preimages are explicit runtime inputs. The sent DWORD persists across loop iterations, matching the native single stack slot. Concrete providers normally overwrite successful outputs; these preimages keep partial-output observations defined without inventing zero-filled native stack storage. Endpoint +14/+20 preimages instead come directly from the actual allocation and remain intact until native writes.

## Verification and retained limitations

The retained one-file fixture is `local/native_online_ipc/ipc_probe.cpp`; its build uses `/MD /W4 /WX /fp:strict /link /MANIFEST:EMBED` and a safe executable name. It redirects copied native calls and IAT entries to the same deterministic memory, pipe and system providers as the source, preserving actual endpoint and buffer addresses. It compares full endpoint snapshots and call order across creation, worker-before-publication, wrapped allocation size, initial allocation failure, later fatal cleanup, normal loop/framing and timeout teardown. Fatal `_exit` is intercepted by a probe-only jump across frames with trivial locals. It also runs the real reconstructed thread using CreateThread, observes E_ABORT termination with WaitForSingleObject/GetExitCodeThread, and checks repeated binding and sentinel/null slots. No pipe endpoint, account write, network request, real process exit or real termination is invoked.

At the worker's audit, 24 direct numeric call rows were checked and three remained true failures: A4BE36->BF6989, A4BE3F->BF65AC and A4C20D->BFBDBB fall in saved-body gaps. Full read-only listings and disk bytes establish A4BDE0's missing A4BE2F..A4BE47 tail (A4BE48..A4BE4A remains an existing epilogue) and A4C030's missing A4C209..A4C211 tail. The root must repair those saved function bodies under its write lock and rerun validation. This packet performs no Ghidra mutation or save and retains the failed rows. Build/fixture results and final immutable artifact manifest are recorded in the report; they are not game/runtime parity claims.
