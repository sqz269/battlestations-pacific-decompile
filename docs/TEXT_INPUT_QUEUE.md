# Platform text-event storage

`TextInputQueue` reconstructs the sentinel builder00bec710, node builder00bec7b0,
count check00bed290, append00bed370, destruction00bec730 and successful platform
pop00bece90. This is the list dependency used by the platform constructor and
message handler at platform+174h. STARTUP_WINDOW_HANDOFF and TEXT_INPUT_CONSUMER
record the call sites, native ABIs and surrounding policy.

Nodes contain next/previous pointers and a two-byte event, with native size12.
The first event byte is the low character or virtual-key value; the second is
zero for WM_CHAR or one for selected WM_KEYDOWN events. Payloads and padding
of the sentinel are left uninitialized. Append allocates first, checks the
unsigned maximum count, then links at the tail. Pop copies the first event,
unlinks/frees its node and decrements count. Destruction resets sentinel links
and count before freeing the detached nodes, then frees/clears the sentinel.

The destructor and erase exports omit instructions following calls to free.
Complete raw disassembly establishes the loop and count decrement; the port
does not follow that truncated pseudocode. `reports/text_input_queue_audit.json`
records complete disk/live byte matches for the five storage routines; the
consumer handoff records pop/erase evidence. All batches verified `bsp` and
`/battlestationspacific.exe` before analysis. Confirmed functions have descriptive
names and preserved evidence comments in the saved project.

## Interface and validation limits

The new class is a typed owner, not the platform/list/iterator ABI. Its `size`
and `front` accessors are host inspection conveniences, not additional recovered
functions. Native count checking throws a native length_error; the port uses the
current C++ exception type with the same text and unsigned arithmetic. Original
allocator/SEH layout is not replicated. Host RAII frees an unlinked allocation
if the subsequent length check throws. Host pop returns false on an empty queue;
native calls the invalid-parameter handler. The private-node interface omits
native iterator debug checks and iterator return storage.

The existing platform probe passed a two-event character/key FIFO sequence and
left a node for nonempty destructor cleanup. Build and both existing CTests
passed. This is a focused host check, not a native queue differential test or
proof that arbitrary allocation failures are equivalent. No new test target
was introduced.

Window input enabling/clearing00a965a0, full message filtering, clipboard flag
handling, text-owner callback/fallback processing and actual native window
lifecycle remain separate dependencies. In particular, native enable clears the
queue even if the requested state equals its previous state. These behaviors
must be implemented before connecting this storage to a real text field.
