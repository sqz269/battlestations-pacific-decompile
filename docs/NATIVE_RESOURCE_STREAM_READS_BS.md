# Native structured-resource stream reads (BS)

Addresses: `00BE42E0`, `00BE4300`, `00BE4620`, `00BF0280`, `00BF0510`.

Five complete ordinary bodies (452 bytes) now operate on actual stream, reader,
native-string and budget storage in `src/native_resource_stream_reads.cpp`.
The context borrows the existing raw string pool and concrete stream dispatch,
plus the mutable empty-string storage binding for native `0109DB64`. Names are
descriptive hypotheses. No stream clone, projected reader, alternate string pool
or numeric-code execution is introduced.

| Address | Original ABI | Preserved behavior |
| --- | --- | --- |
| BE42E0, 26 bytes | ECX stream; optional actual pointer on stack; EAX DWORD; RET4 | Slot34 implementation: one current slot24 read4 into the incoming argument slot. |
| BE4300, 26 bytes | Same | Distinct slot38 implementation with identical instructions apart from address. |
| BE4620, 327 bytes | ECX stream; output header/optional actual on stack; EAX output; RET8 | Read length via current slot38, prepare a space-filled temporary, read once, report actual prefix+payload, construct output and return the captured temporary. |
| BF0280, 27 bytes | ECX reader; budget pointer on stack; EAX scalar; RET4 | Initial local actual contains reader-address bits; current slot34 returns scalar, then current budget is wrap-debited. |
| BF0510, 46 bytes | ECX reader; output/budget on stack; EAX captured output; RET8 | Actual-count storage is the reused output argument slot, initially containing output-address bits; current slot48, then wrap-debit. |

The scalar read-buffer seed is the incoming optional-count pointer, not zero
unless that pointer is null. A partial read leaves unwritten pointer bytes
intact. This is distinct from the earlier font fragment's null-count domain.
BF0510 also clears an otherwise unused local word; it does not clear its reused
output-argument count slot. Original private stack aliases are not supplied by
the new C++ interfaces, but the public pointer-bit seeds are preserved.

BE4620 captures temporary length/data after resizing. It fills the captured
logical count with spaces, requests the original declared length, and copies
using captured temporary fields after raw read. A short payload retains the
declared string length and space-filled tail. Optional actual is published
before output construction. Zero length publishes prefix actual and constructs
empty output. Native BF7680 permits overlap; the source uses `std::memmove`.

Handler CC6BE1 -> FuncInfo E013E8 uses map E013D8: state1 ->0 via CC6BC0
destroys the current temporary at synthetic parent EBP-14; state0 ->-1 via
CC6BC8 tests/clears completed-output flag EBP-18 and destroys the output named
by EBP+4. The flag is set only after the nonzero branch completes output. Normal
temporary return occurs in state0: failure destroys output without retrying the
temporary. Source C++ actions follow this ordering; a second unwind exception
terminates. Native FH3/SEH/CRT identity and hardware-fault behavior are unproved.

Known current scalar targets BE42E0/BE4300 and string target BE4620 dispatch to
their source bodies. Unsupported reached targets throw; arbitrary alternate
stream profiles are outside the demonstrated domain. Raw reads use the existing
dispatch. No length cap, full-transfer check, budget clamp or retry is added.

Strict MSVC Win32 compilation and an actual-memory-stream fixture passed for
0..4-byte scalar reads with nonnull/null count, both scalar bodies, wrapped
budget subtraction, short string `AB  ` retaining declared length4, and empty
string. The same fixture creates and releases a complete native root. It binds
the mutable empty-string cell separately; the null-temporary fallback is not
exercised. Reports retain bytes, calls, EH data and artifact hashes. No original
scalar oracle or game runtime was executed by this fixture.
