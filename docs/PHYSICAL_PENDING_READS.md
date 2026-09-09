# Physical overlapped submission and explicit completion pump

Addresses: `00bf43b0`, `00bf46b0`, `00bf41c0`, `00bf3da0`, `00bf4240`, `00befa40`

`PhysicalPendingReads` owns real Windows overlapped file reads and dispatches
successful completions only when its caller pumps them. Accepted immediate
`ReadFile` success remains queued. It copies completed bytes through the existing
`memory_stream_from_bytes_00befa40_fragment`, allowing callbacks to retain an
independent memory stream after staging storage and the file handle are released.

This is a new Win32 C++ owner, not a native physical-provider object or ABI.
The native submission takes ECX provider and four stack arguments
`(firstName, secondName, callback, flags)`, returns acceptance in AL and uses
RET 10h. Native pump `00bf46b0` takes ECX provider, no stack arguments and RET.
The typed host accepts an already resolved physical ANSI path as a separate
argument; `PhysicalDirectory` and VFS selection remain the parent's integration.
First and second callback names are copied unchanged and never replaced by that
physical path or a traversal suffix.

Evidence is the complete byte-verified assembly in
[VFS_PENDING_DISPATCH.md](VFS_PENDING_DISPATCH.md) and
[VFS_PENDING_LIFETIME.md](VFS_PENDING_LIFETIME.md), with their corresponding
audit reports. This packet used capped `tools/bsp.py lookup` queries for the
current submission/pump/copy names and cached verified assembly. It did not
modify or freshly analyze Ghidra.

## Submission and accepted ownership

`submit_00bf43b0_fragment(path, first, second, flags, callback, error)` returns
true only after `ReadFile` either succeeds immediately or reports
`ERROR_IO_PENDING`. It does not invoke the callback. It requires a callable
callback and strings without embedded NUL. Empty names are otherwise retained;
an empty physical path reaches the real Windows open failure.

The read gate is the observed `(flags & 1) == 0 && (flags & 0xE) == 2`.
Both 2 and 0x32 are accepted, as are other values satisfying that native gate.
Unconsumed flag bits acquire no invented meaning. The OS route is:

- `CreateFileA`: `GENERIC_READ`, `FILE_SHARE_READ`, `OPEN_EXISTING`, null
  security/template, `FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING`.
- `GetFileSizeEx`: require a successful size in `1..INT32_MAX`, the bounded
  domain supported by the existing memory-copy helper. Empty, high-DWORD and
  larger sizes return the host `ERROR_NOT_SUPPORTED` result.
- Allocate uninitialized `logical_size + 0x20000` staging bytes, then round the
  usable address upward to a 0x10000 boundary. Round the read count upward to
  the same 64-KiB multiple. The supported size domain avoids DWORD overflow.
- Allocate a separate zero-initialized 20-byte Win32 `OVERLAPPED`, with zero
  offset and no event. Pass it to `ReadFile`, with null synchronous actual-count
  output, exactly as the native submission does.

There is no buffered or synchronous fallback if this OS route fails. The
reported error comes from the failing operation or an explicit host input/
size/allocation guard. Rejected work is not queued. The temporary open handle
is released on rejection; any earlier operation error remains the primary
reported error.

The queue holds separately allocated request objects. Each owns its callback,
two names, handle, original staging allocation and `OVERLAPPED`; its aligned
pointer refers into that allocation. Queue capacity and all request/name/
callback allocations are prepared before starting `ReadFile`. Once accepted,
insertion cannot allocate or throw. A C++ exception therefore cannot discard
an operation that was just handed to Windows.

Native reserve relocates records and copies names while preserving I/O pointer
values. The host instead moves `unique_ptr` entries, keeping request and name
addresses stable through growth. That is an explicit safe lifetime extension;
it does not claim native borrowed name wrappers survive callback submission.
The owner is noncopyable and nonmovable.

## Pump ordering and truthful completion counts

`pump_00bf46b0(report, error)` makes one nonblocking pass through the live queue.
It first checks `OVERLAPPED.Internal == 0x103`. Such entries remain pending.
Other entries call `GetOverlappedResult(..., FALSE)` with an actual-count output.
No OS completion flag or transferred count is synthesized.

Terminal successes use the original logical size for memory copying. The host
checks that the returned count is at least that size and no greater than the
requested rounded read size. A short result reports `ERROR_HANDLE_EOF`; an
impossible count above the request reports `ERROR_INVALID_DATA`. Neither copies
uninitialized staging bytes nor invokes the callback. A larger initialized
result within the rounded request still copies only the original logical
prefix, matching the native copy count; its full actual count stays in the report.

The native pump lacks this initialized-length guard. It also frees requests
after any failed `GetOverlappedResult`. The host keeps an unexpectedly incomplete
request owned if the error is `ERROR_IO_INCOMPLETE`/`ERROR_IO_PENDING` or its
status still says pending, and reports that observation as an error rather
than pretending the request is terminal. Ordinary pending status is not an
error. Actual counts from failed results are marked invalid in completion
records; an initialized output variable is not claimed as an OS byte count.

For successful copying, callback order is:

```text
callback(shared_memory_stream, owned_first_name, owned_second_name)
release the pump's temporary stream reference
free original staging allocation
free OVERLAPPED
close handle
erase completed request
```

The callback may retain the shared stream. Name references are borrowed until
that request is erased. Copy names if they need to survive callback return.
Callbacks can submit another request: growth keeps the active request stable,
and erasure uses the current queue base. Erase preserves relative order and
rechecks the same index. The loop uses the current count, so an appended request
may be examined and completed in that same outer pump call. Submission itself
still never dispatches inline. Later ready entries can complete while earlier
ones remain pending; this is not strict FIFO completion.

`PhysicalReadPumpReport` exposes `completed`, `succeeded`, `failed`, `remaining`
and one owned record per terminal request, including both names, logical/
requested/transferred counts, count validity, callback invocation, outcome,
primary error and separate handle-close error. The pump continues after
terminal I/O, length or copy failures and returns false with its first error.
These errors have no invented native failure callback. Unexpected incomplete
observations are counted separately and keep their resources/pending count.

## Exceptions, serialization and shutdown

All access must be serialized by the caller. There is no creator-thread check:
the audited native pump can run from the frame path or a loading-thread stop
wait. This class does not establish cross-thread synchronization. Recursive
pumping returns `ERROR_BUSY` without modifying the supplied report. Callbacks
must not mutate the active report or destroy the owner. The caller must keep
the owner and callback dependencies alive through the entire pump return,
including destruction of the completed request's callback object.

If a callback throws, the pump records `callback_threw`, releases its temporary
stream reference, cleans up and erases that terminal request, updates counts,
restores the non-pumping state, then rethrows the original exception. It does
not invoke that callback again. Remaining requests stay owned. Allocation
exceptions before callback invocation leave that completed request queued for
a later retry; the report's `remaining` count and pump guard are restored.
Exceptions are a defined host policy, not recovered native exception behavior.

`begin_shutdown(error)` stops further submissions, including submissions from
later callbacks. It performs no wait, cancellation, dispatch or resource
discard. It returns `ERROR_BUSY` during an active pump. The caller must continue
calling the explicit pump, handling errors/exceptions and retaining the owner,
until `pending_count()` reaches zero. A normal error result can coexist with
successful draining; inspect the report and remaining count.

**Destruction requires an empty queue and no active pump.** Violating that host
precondition calls `std::terminate`. The destructor cannot silently wait, cancel,
discard callbacks or free buffers still owned by Windows. It is therefore also
the caller's responsibility to drain accepted work during its own exceptional
unwinding. No automatic cancellation or bounded shutdown-time guarantee is
claimed. Native cancellation, unmount/shutdown invariants and FileStore cleanup
after a physical error without callback remain separate integration questions.

## One installed-file scenario and validation boundary

The parent separately authorized `probe_physical_pending_reads(physical_path)`
in `src/physical_pending_reads_probe.cpp`, called from the existing D3D9 probe
with installed `scripts/datatables/inputs.lua`. One ordinary `std::ifstream`
read supplies its independent expected bytes. Flags 2 submit the first request;
it must be queued with no callback before pumping. Its callback validates
initialized memory, cursor zero and both names, then submits the same physical
file with flags 0x32 and different names. The current names must survive that
append, and the second callback must not run inline in submission.

After the first callback has submitted the second request, the probe begins
shutdown and drains explicitly. It accepts either same-pump or later completion
of the second OS read. Reports must show two successful completions, correct
rounded requests and actual counts, and no hidden errors. After destroying the
drained queue, retained streams must still contain the independent expected
bytes with separate backing/cursors. No fixture changes the installed file.

The polling phase has a 15-second deadline. A timeout or exception that leaves
pending requests prints an error and exits the probe process explicitly through
`std::_Exit`; it does not destroy an active queue or invent cancellation. Errors
on fully drained paths return false normally. This is a probe process failure
policy, not the library's shutdown mechanism.

The complete MSVC Win32 build, both existing CTests and full D3D9 probe pass.
The installed source was 71,090 bytes: both requests rounded to131,072 bytes,
both completed with71,090 actual bytes, and two callbacks retained correct
independent copies after shutdown. The observed run used two pumps; timing is
not a requirement. No installed file was changed and no test target was added.

Independent review caught an aliased error-output bug: a callback can reuse the
DWORD supplied to pump for another operation. The pump scope guard now restores
`error=report.first_error`, including exception exits. The same fixture's second
callback deliberately reuses that DWORD for a rejected shutdown-during-pump
call; the successful pump must still return ERROR_SUCCESS.

See `reports/parallel_provider_validation.json`. Manager dispatch, FileStore
pending integration, frame/stop-loop wiring and original-game behavior remain
incomplete. Error/cancellation and unusual filesystem cases are not exercised
by this successful-read scenario. This is a host interface, not native ABI.
