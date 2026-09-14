# Material record vector allocation and length error (CJ)

The installed Win32 PE has SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Read-only live analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. `00B145E0` directly calls the length error at
`00B1466E` and the allocator at `00B146E3`; live Ghidra lists no other
callers. Its allocation caller clears EDX, passes the count in ECX, and keeps
EAX as the returned storage. The JSON report pins both complete PE spans,
source/provider hashes, native EH data, and generated-object review.

`00B0D3B0..00B0D405` treats ECX as an unsigned count. Count zero reaches
the native `00BF681B` allocator with a zero-byte request. For a positive
count, native `UINT32_MAX / count >= 300` admits `count * 300` bytes; the
largest admitted count is `14,316,557` (`00DA740D`). The source requests
the **same** native and host byte count through concrete
`singleton_lifetime_allocate`, whose `malloc` / `_callnewh` retry and
`std::bad_alloc` service is paired with `singleton_lifetime_free` / `free`.
It uses the existing source `std::bad_alloc` transport for overflow. The
original overflow constructs a null-message `std::exception` at `00BF6340`,
stamps vtable `00D6923C`, and calls `00BF6885` with ThrowInfo `00E03CC0`.
The source does not claim that binary heap, RTTI, new-handler state or SEH
identity; EDX is explicitly reserved but unused in its Win32 fastcall API.

`STL_xlen_throw_00b135c0` builds the actual 1Ch SBO temporary with
capacity 15, length zero and the first inline byte zero, then assigns 18
bytes from `00CE37E0` (`vector<T> too long`) through the existing counted
assign provider `00408720`. Its original EH handler `00CBC298` loads
FuncInfo `00DF438C` and jumps to `__CxxFrameHandler3` at `00BF6B43`.
FuncInfo magic is `19930522`, MaxState is 1, and its unwind map at
`00DF4384` sends state 0 to -1 via `00CBC290`, which destroys the temporary
at `[EBP-50h]` through `004072D0`. The state 0 store at `00B13605` follows
the successful assign call at `00B135F8`; failure during assignment has no
temporary cleanup. The source arms its `CompletedMessage` owner at that same
boundary, then uses the actual 28h logic-error payload constructor and
`NativeSingletonVectorLengthError` host throw transport already used by
`00BD0590`. Both native length-error bodies are identical after masking only
their handler address and three relative-call displacements; both use
ThrowInfo `00D83F98` and vtable `00D69260`.

The Win32 Release object review found a 111-byte length-error COMDAT with
seven relocations, a 42-byte EH funclet/handler COMDAT with four, and a
92-byte allocator COMDAT with six. Its allocator compares ECX against
`00DA740D`, multiplies admitted counts by `012C`, and passes that byte count
in both request-size fields to the concrete lifetime allocator. The generated
length-error code initializes the three SBO fields before counted assignment,
arms cleanup only after that call returns, then constructs and throws the
existing host catch type. The generated unwind funclet reaches the raw SBO
destructor. The report pins all three section hashes and relocation offsets.
The Win32 build and existing `reconstructed_math` CTest passed.

The source preserves normal allocation bytes, raw SBO initialization and
cleanup timing, and the established source catch type. Generated C++ EH and
host CRT behavior remain distinct from the native `00BF6885` throw, RTTI,
private FH3/SEH frame, and arbitrary native exception interception. These
helpers do not establish completion of their vector-insert caller or game
execution.
