# XLive pipe I/O

`src/xlive_pipe_io.cpp` reconstructs the normal control flow of the pipe
read/write submission, completion and server-connect operations. Descriptive
names and the existing `PIPEIPC` grouping remain analyst hypotheses, not evidence
of library authorship. `Win32XLivePipeIoHost` forwards the actual Windows APIs;
the reconstruction supplies no protocol, endpoint or asynchronous-success stub.

| Address | Native ABI | Operation |
| --- | --- | --- |
| `00A5DE84` | stdcall(owner, timeout), RET 8 | Validate owner, wait for connection |
| `00A5DEAA` | stdcall(owner, buffer, bytes), RET C | Validate owner/buffer/length, submit read |
| `00A5DEE8` | stdcall(owner, DWORD output*, timeout), RET C | Validate owner/output, finish read |
| `00A5DF20` | stdcall(owner, const buffer, bytes), RET C | Validate owner/buffer/length, submit write |
| `00A5DF5E` | stdcall(owner, DWORD output*, timeout), RET C | Validate owner/output, finish write |
| `00A5E16E` | stdcall(pipe, OVERLAPPED*, DWORD pending*), RET C | ConnectNamedPipe and already-connected event signaling |
| `00A5E1F7` | thiscall(owner, buffer, bytes), RET 8 | ReadFile submission |
| `00A5E28B` | thiscall(owner, const buffer, bytes), RET 8 | WriteFile submission |
| `00A5E31F` | thiscall(owner, DWORD output*, timeout), RET 8 | Wait and GetOverlappedResult |
| `00A5E750` | thiscall(owner, timeout), RET 4 | Start/continue server connection wait |
| `00A5E7E6` | thiscall(owner, DWORD output*, timeout), RET 8 | State-1 read completion |
| `00A5E815` | thiscall(owner, DWORD output*, timeout), RET 8 | State-2 write completion |

Every native result is in EAX. These C++ signatures explicitly add the required
OS host; they are not drop-in stdcall/thiscall replacements. The public wrappers
reject null and `FFFFFFFF` owner pointers first (`80070006`), then null
buffers/outputs or zero submitted lengths (`80070057`). Timeout zero is accepted.
Internal bodies add no public argument guards.

All twelve spans were independently checked against the live saved program and
the installed executable. `A5DE84` has no Ghidra function start: it begins after
the close wrapper and ends at `A5DEA9`, with a three-byte `RET 8` at `A5DEA7`.
The other eleven starts exist. There are no false no-return gaps. Exact final
instructions, full bytes, disassembly and hashes are in
`reports/xlive_pipe_io.json`; this packet performs no Ghidra writes.

The only transport state is the canonical 30h `XLivePipeNativeState` from
`xlive_pipe_transport.hpp`. Its actual OVERLAPPED occupies `+0C..+1F`; the event
at `+1C` aliases the separately owned completion event `+24` after successful
transport creation. The stop event `+28` is borrowed. Submission clears only
OffsetHigh `+18`, then Offset `+14`. Internal `+0C`, InternalHigh `+10` and hEvent
`+1C` retain their preimages until the operating system changes them. No local
OVERLAPPED, event reset, zeroed prefix or shadow state is introduced.

Read/write require state `+20 == 3`. If count `+2C == 2`, a zero-time stop-event
wait aborts only on result zero; even WAIT_FAILED proceeds. After clearing
offsets, SetLastError(0) precedes ReadFile/WriteFile. The transferred-count local
begins with the owner pointer DWORD because the native prologue uses PUSH ECX;
its API output is discarded. Immediate API success and ERROR_IO_PENDING (997)
both set state 1/read or 2/write and return `8000000A` (E_PENDING). Other failures
retain current state: last-error zero becomes 507h, positive signed errors become
`80070000 | low16(error)`, and negative DWORD results pass through unchanged.

Completion waits directly on `&native.completion_event_24`, so the adjacent
`+24/+28` handles and current `+2C` remain authoritative. WaitForMultipleObjects
uses wait-all false and the caller's timeout. Timeout returns E_PENDING before
any count reload; otherwise the returned index is compared against the current
count after the call. Index zero performs SetLastError(0), then
GetOverlappedResult with the same OVERLAPPED, actual caller output and wait=false.
Nonzero transferred bytes return S_OK; zero bytes return `80004005` (E_FAIL).
ERROR_IO_INCOMPLETE (996) returns E_PENDING; other errors use the conversion
above. API writes to the output survive failure, and timeout does not clear it.
Index one calls CancelIo on the current pipe, discards its result and returns
`80004004` (E_ABORT). Out-of-count indices, WAIT_FAILED, abandoned waits and other
results yield `8000FFFF` (E_UNEXPECTED), without a last-error query. The read/write
completion wrappers reset state to 3 on every result except exact E_PENDING,
including errors and cancellation.

The connect helper has a deliberately different error contract. It first sets
the supplied pending DWORD to zero and clears last error, then calls
ConnectNamedPipe. Immediate success returns zero. Failure with last error zero
returns **raw positive 507h**; most other errors are also raw DWORDs. Pending 997
sets the output to 1 and returns zero. Already-connected 535 clears last error
again and signals the current OVERLAPPED hEvent; only failure of that SetEvent
uses HRESULT conversion. Consequently a positive connect result can describe an
error. Callers must retain this native distinction.

`A5E750` accepts only nonzero mode and state 3/start or state 0/continue. Starting
captures the pipe argument, clears offsets and calls the connect helper. A
negative result or zero pending flag returns immediately, preserving a positive
raw error. A pending connection waits on the canonical handles. Timeout stores
state 0 and returns `800705B4`. Completion index zero returns S_OK without
GetOverlappedResult or a store of state 3; thus a later successful continuation
after timeout leaves state 0. Stop cancellation returns E_ABORT. This behavior is
preserved even though it can prevent a subsequent read/write from accepting the
owner. CancelIo is not followed by a completion drain in any of these functions.

Transport creation, protocol lifetime/encoding, buffer ownership and worker
threads remain separate canonical dependencies. The owner, actual OVERLAPPED,
events and pending buffers must remain alive for their native asynchronous use;
these operations do not establish completion merely by canceling. This packet
does not add a new lifetime or endpoint policy.

Validation: Win32 Release C++17 `/W4 /WX /fp:strict`, both existing CTests and
all eight native seed checks pass. The focused fixture is an
ignored synchronous recording program: it checks actual prefix/output aliases,
preimages, immediate-success E_PENDING, incomplete/zero-byte results, cancellation,
count reload, mixed connect errors and state-0 timeout continuation. It opens no
endpoint, starts no thread/process and invokes no real I/O API. Build/fixture
results do not establish live pipe or SDK behavior. The dependency integration
briefly registered an absent `xlive_manager_owner.cpp`; the integrator removed
that inherited registration for this worktree. Its real owner registration must
be retained when merging into a checkout that contains the owner source.
