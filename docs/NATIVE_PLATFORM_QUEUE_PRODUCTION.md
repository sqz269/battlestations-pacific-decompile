# Raw platform text-queue production

The current Source platform can create and append the two-byte message records
on its actual embedded queue. This packet adds BEC7B0, BED290 and BED370 to
`src/native_platform_construction.cpp`, using the same 184h-byte platform owner,
12-byte queue header at +174h, sentinel and canonical Source CRT allocation
domain as the previously adopted constructor and BEC730 retirement. It does not
bind the application, window procedure, message handler or Text singleton.

The earlier typed `TextInputQueue` in `src/text_input.cpp` remains a separate
semantic reconstruction. Its private owner, allocation and exception cleanup
are not substitutes for the raw platform queue. Earlier ledger records and
Ghidra comments are retained alongside these new Source records.

The review and fixture baseline is main commit
`e20f1b31bac3338fe006f4a222fa2119edc75d1b`. The permanent CMake registration
already exists on that baseline. The final normal `scripts/build.ps1` run uses
that registration, with `CMAKE_PROJECT_INCLUDE_BEFORE` absent from the cache.

## Original contracts and Source implementation

| Address | Size | Original ABI | Current Source boundary |
| --- | ---: | --- | --- |
| 00BEC7B0 | 53 bytes | Three stack arguments: next, previous, event pointer; EAX node; RET 0Ch | `__stdcall create_native_text_queue_node_00bec7b0` |
| 00BED290 | 147 bytes | ECX actual queue header, increment on stack; EAX updated count; RET 4 | `__fastcall increase_native_text_queue_count_00bed290`, EDX unused |
| 00BED370 | 50 bytes | ECX actual queue header, event pointer on stack; RET 4 | `__fastcall append_native_text_queue_00bed370`, EDX unused |

Names are reconstruction hypotheses, not recovered symbols. The Source
interfaces and current C++ EH are not Original binary replacements.

BEC7B0 requests exactly 0Ch bytes. It performs three independent pointer tests
before the stores at node +0, +4 and +8. The +4 and +8 tests operate on the
32-bit wrapped addresses; they are not a single conventional null guard. The
payload is one WORD load and one WORD store. Bytes +0Ah/+0Bh remain untouched.
The emitted 53-byte sequence matches Original except for the allocation CALL
operand, which resolves through a real bridge to `singleton_lifetime_allocate`.
No allocator callback or private node owner is introduced.

BED290 captures the volatile DWORD at queue +8 once and tests the unsigned
predicate `(0x7fffffff - captured_count) < increment`. Equality succeeds.
Successful addition wraps modulo 32 bits and stores the result at +8. This
includes the Original behavior for already out-of-range counts; the Source
does not impose a new signed test or saturating bound.

On failure, BED290 builds a legacy SBO string and assigns the 16 message bytes
from `list<T> too long`, excluding the literal's terminator. The completed temporary is
armed only after assignment returns. The original FH3 data has max-state 1,
unwind entry `FFFFFFFF -> 00CC74A0`, and the funclet jumps to 004072D0 to destroy
that completed string. The constructor at 00411700 then builds the exception
owner; the derived vtable is D69260. ThrowInfo D83F98 names destruction at
00411780 and its catchable-type array at D83FD4.

Source uses the existing `NativeAliasListLengthError` transport. Its sole member
is the actual 28h-byte legacy exception owner, with the established native
string construction, copy and destruction providers. Current compiler RTTI,
catch type and EH metadata are new. The original FH3/ThrowInfo catch ABI and
allocation-failure unwinds were not dynamically exercised in this packet.

BED370 captures the sentinel from header +4 before allocation and captures that
sentinel's current previous pointer for the new node. It allocates first, calls
BED290 with increment 1, stores the new node into the captured sentinel's
previous field, then reloads the new node's previous pointer for the final
old-last.next store. The 50-byte sequence matches Original except for its two
Source callee operands. No exception guard owns the new node. A count failure
leaves the allocated node unlinked; adding RAII cleanup would change the
Original failure behavior.

The header DWORD at +0 remains opaque. Only header +4 and +8 participate. The
Source append fixture uses the constructor-created sentinel in the actual
platform's +174h storage; no queue projection, raw clone or 8-byte typed queue
is passed to these functions.

## Evidence and bounded execution

The saved listing and live Ghidra bodies agree with the installed PE for all
three functions. Eight checked spans total 361 bytes: the 250 body bytes,
CC74A0/CC74A8 cleanup and handler, E01FE4/E01FEC unwind data, D83F98 ThrowInfo,
D83FD4 catchable-type array and CE38F8 message bytes. Six decoded calls agree
with their recorded providers. Each live batch verifies project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`.

The standalone candidate and fixture compile as MSVC Win32 with `/O2 /MD
/EHsc /W4 /WX /fp:strict /std:c++17 /Gy`. The executable embeds an asInvoker
manifest. COFF and linked-code checks prove the exact node and append sequences
with only the named rel32 operands changed. The existing 72-byte queue
retirement still matches except for its two real Source free operands. BED290
is compiled C++; its emitted normal path has one count load, unsigned
subtraction/comparison and one wrapped update. It is not a 147-byte binary match.

The focused fixture passes three checks:

1. A real Source CRT allocation holds the full 388-byte platform, constructed
   with the current constructor and actual fixture manager/string-pool cells.
   Source append and copied Original node/append bodies produce equivalent
   complete-owner snapshots and exact sentinel/node links and WORD payloads.
   Only the sentinel address in the owner snapshot is normalized. Explicit
   adversarial aspect preimage bytes are fixture input, not claimed game defaults.
2. Copied Original BED290 and Source run accepted increments on that same
   embedded header: `0+1`, `7ffffffe+1`, `80000000+ffffffff`, and `ffffffff+1`.
   The copied Original normal path retains its bytes. Its throw/handler path
   is not executed or rebound to a synthetic provider.
3. Source append at count `7fffffff` throws the current transport with native
   vtable D69260 and the actual 16-byte counted message. Count and links stay
   unchanged. The actual CRT heap walker identifies the still allocated,
   unlinked 12-byte node by its two links and unique event WORD. Only the
   fixture subsequently frees it. Raw queue and window-base retirement then
   complete, followed by manager cleanup of the recorded null holes.

The fixture does not call the derived screensaver retirement, window APIs,
message dispatch or a game executable. Node-tail padding preservation is
instruction evidence; the fixture does not claim a controlled allocator
preimage for those two bytes. Original exceptional execution, null/wrapped
allocation faults, exact register/EH replacement, application binding, frame
behavior and gameplay remain unproved.

The final normal build passes all three existing tests, including
`native_math_differential`. Core/Lua/zlib hashes are identical before and after
the final manifested probe. The earlier adoption, retirement and window-frontier
captures remain frozen; this packet has its own manifest at
`local/checkpoints/cc14-native-platform-queue-production/final/validation.json`.
The report records exact hashes, commands, provider contracts and limitations.

All three reviewed Ghidra names are retained, earlier comments preserved and
new raw-owner evidence appended through the annotation tool's write lock.
The project is saved, comments read back and the three exports refreshed.

## Remaining window frontier

This closes only the raw queue producer dependency of BED3B0. Full message
handling, BEC3B0's current canonical publication lookup, the raw power/window
configuration owner and explicit application bindings remain separate work.
BECEE0 and BED3B0 do not acquire reconstruction credit from this queue fixture.
