# Actual reader ownership BP

This packet reconstructs buffer destruction `BF0980` (23 bytes), base-reader
destruction `BF09B0` (120), structured-reader construction `BEA150` (101) and
destruction `BE9F10` (120). Their complete 364 bytes match fresh live Ghidra
bytes and the installed PE. The five-byte BF0980 tail is recovered from bytes
outside its current function body. All APIs use actual storage and new C++
interfaces; native ECX holds the owner, with no stack arguments and plain RET.
Only BEA150 has a stable EAX result: the original reader.

The actual 10h base is stream+0 and 0Ch buffer header+4. The 70h structured
reader adds ten 8h strings at +10..+5F, index+60, control+64 and separate name
length/data at +68/+6C. Existing reference and optional-buffer source is reused.
No projected owner, private string pool or automatic reference policy is added.

## Ordinary owner schedules

BF0980 resizes the current actual buffer to zero, then frees **current** data.
It leaves data/capacity words unchanged and has no local cleanup/retry state.
BF09B0 captures stream before arming its state0 cleanup. For a nonnull stream
it decrements captured+4 and reads the current table/slot0 only at zero. It
passes captured stream/table to existing `source_zero_reference`, then clears
reader+0 after a normal return, including a callback replacement of that word.
A null stream skips the clear. Before normal buffer destruction, it changes
state to -1. A failed stream terminal instead runs the buffer destructor once
through the state0 unwind action; the stream word is not cleared on that path.

BEA150 initializes the complete 10h base, then arms base cleanup. It constructs
ten empty strings forward with the exact two DWORD stores of `415270`, then
writes index0, controlFFFFFFFF and both zero name words. It returns the reader.
It does not acquire a stream or allocate strings. After the array iterator
returns, the outer map still contains only base cleanup: no additional array
cleanup state is invented around subsequent scalar/name stores.

BE9F10 captures name data before arming state1. If nonnull, it captures wrapping
name length+1 before resolving the current pool and returning that block. It
then changes to state0 and destroys all ten strings backward, independently
of the path index. Finally it changes to -1 and destroys the base. String
headers, path index and control are not cleared. RawPoolContext is used for
every string return, so an ordinary getter exception can escape.

## Outer EH maps

All FuncInfo records have magic19930522, no try blocks and EH flags1. The
captured reader is the native EH frame's `[EBP-10h]` in these owner funclets.

| Owner | FuncInfo / map | State transition and action |
|---|---|---|
| BF09B0 | E02464 / E0245C | 0 -> -1: CC7810, BF0980(reader+4) |
| BEA150 | E01B58 / E01B50 | 0 -> -1: CC7130, BF09B0(reader) |
| BE9F10 | E01AC4 / E01AB4 | 1 -> 0: CC70C8, BE9F00(reader+10) |
| BE9F10 | same | 0 -> -1: CC70C0, BF09B0(reader) |

BE9F00 invokes the ten-element destructor iterator. A name-return failure thus
unwinds the whole array and then the base. A failure during ordinary array
destruction first invokes the iterator's remaining-element cleanup, then the
outer base action. State advances **before** each outer action, as demonstrated
by FrameUnwindToState at C06A09. A failed normal base/buffer operation is not
retried, because its outer cleanup state was already disarmed.

## Iterator and secondary-failure evidence

The constructor iterator BF7CD1 tracks a current pointer and completed count;
it advances both only after an element constructor returns. Its SEH4 finally
BF7D1E calls BF7C10 ArrayUnwind only when completion is false, passing that
cursor and completed count. Therefore it destroys only preceding completed
strings, in reverse, and never the unfinished string.

The destructor iterator BF7C6E starts at first+80, decrements remaining **before**
each call, and subtracts stride8 to reach the next string. Its finally BF7CB9
passes the current failing-element address and remaining preceding count to
ArrayUnwind. A failed string is not retried. ArrayUnwind decrements the count
and subtracts stride before each cleanup call. Scope tables E02D50, E02D30 and
E02D10 establish these finally/filter connections, including entry state0 and
enclosing state-2. The source models the cursor/count/completion fields with
explicit `__try`/`__finally`, not an assumed C++ container destructor.

ArrayUnwind's filter at BF7C33 tests exception code E06D7363 and calls verified
`terminate` C07A75; other exceptions continue search. This is a secondary C++
failure during remaining-element cleanup, not an ordinary return failure.
The outer FH3 route is BF6B43 -> C07991 -> C069A2 FrameUnwindToState. Its filter
stub C06A24 calls C0695E, which likewise terminates for E06D7363 while performing
native ProcessingThrow bookkeeping. Its separate E0434F4D branch adjusts CLR
bookkeeping and continues search. The source uses an explicit unwind-only SEH
filter for the established C++ termination policy. It does not claim to replace
the original CRT, ProcessingThrow/CLR state or binary FH3 interoperability.
No unconditional noexcept string-release boundary suppresses ordinary failures.

## Read-only repair findings

- BF0980 currently owns BF0980..BF0991 only. BF0992..BF0996 is
  `ADD ESP,4; POP ESI; RET`; BF0997 is padding. The displayed zero-gap count
  applies only to the truncated body. Recover the post-free tail after BF098D.
- EH dispatch stubs CC70D3..CC70DC, CC7138..CC7141 and CC781B..CC7824 have no
  owning functions. Each is a FuncInfo load followed by JMP BF6B43. Their
  associated outer unwind funclets already have correct separate ownership.
- ArrayUnwind lacks BF7C33..BF7C5E (44 bytes). Scope-table filter BF7C33 has
  its return at BF7C56 and terminating arm BF7C57..BF7C5B; handler BF7C5C
  restores ESP and joins the existing epilogue at BF7C5F. Preserve those joins
  when recovering the filter/handler listing and membership.
- Secondary-EH dependency C0695E..C069A1 is a complete 68-byte decoded filter
  with no owning function. FrameUnwindToState lacks C06A24..C06A3D: filter
  C06A24..C06A2D calls it; handler C06A2E..C06A3D restores state/registers and
  joins C06A3E. These are bounded runtime-contract findings, not a CRT rewrite.
- Terminal-identification dependency C07A95..C07A9B is also unlisted/unowned
  inside the captured terminate span. It concerns terminate-handler handling;
  this packet calls the host CRT terminal and does not reconstruct that body.

No Ghidra mutation, ownership repair or no-return change was made. Analysis
and exports used verified `bsp.py ghidra` access to C:/Users/sqz269/bsp.gpr,
/battlestationspacific.exe, x86 Win32 at base00400000. The report separates
function-owned direct/indirect transfers from decoded-only EH instructions.

## Validation and boundaries

Strict MSVC Win32 compilation passed with `/std:c++17 /EHsc /MD /W4 /WX
/fp:strict /permissive-`. One ignored fixture constructed an actual 70h reader,
13 actual pooled strings and a 0Ch optional buffer, then destroyed a real
D642C0 memory stream/backing through `NativeVfsRuntimeBindings`. Object and
byte counters returned to zero. Actual pool reuse verified exact name ->
paths9..0 -> buffer1..0 return order; header/canary checks verified retained
words and storage boundaries. Unused physical/logger services remained empty;
no fixture stream-owner or zero-reference policy replaced the concrete binding.

The fixture uses verified read-only native table data and a frozen registered
BO library from source994e06289522017335e3438a0e499523a8777462, SHA256
02b33f5248e04e595830d66c353883d0b9d66706e37f653a9d8d8f9545c22444.
Library source-before/copy/source-after hashes agree; Lua/zlib support artifacts
are separately frozen. The safe-named probe embeds a manifest and uses a fixed
18000000 image base to leave the native read-only data bands available.

The fixture executes normal owner paths. Getter failure, remaining-element
unwind and secondary termination policies are supported by native listing/EH
evidence and source inspection, not an executed failure fixture. Hardware-fault,
arbitrary native-stack alias, concurrent mutation and gameplay parity remain
unverified. No permanent tests, production runtime, CMake or shared metadata
changes were made. The primary owns the combined integration build.
