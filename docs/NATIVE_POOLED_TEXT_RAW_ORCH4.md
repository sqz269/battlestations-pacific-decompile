# Raw pooled text prerequisites

## Scope

Ten `NativeStringRawPoolContext` overloads now provide the parser-facing actual
four-byte text owners. Existing `NativeStringStorage` overloads and their host
exception behavior remain unchanged. No new source file or CMake entry is needed.

| Entry | Complete bytes | Native interface and operation |
| --- | ---: | --- |
| AEE1E0 | 50 | stack nonnull text, RET4; release strlen+1 bytes |
| AEE2A0 | 55 | ECX header, RET; release captured pointer then clear |
| AEE2E0 | 90 | ECX destination, stack source header, RET4/EAX destination; copy construct |
| AEE340 | 116 | ECX header, stack text, RET4/EAX count; printable token prefix |
| AEE3C0 | 272 | ECX line, stack output/index, RET8/EAX output; construct token |
| AF4450 | 111 | ECX header, stack text, RET4/EAX count; printable suffix including TAB |
| AF44C0 | 272 | ECX line, stack output/index, RET8/EAX output; construct suffix |
| AF5660 | 92 | ECX destination, stack nullable text, RET4/EAX destination; construct |
| AF56C0 | 123 | ECX destination, stack source header, RET4/EAX destination; assign |
| AF5740 | 258 | ECX text buffer, stack output, RET4/AL boolean; read normalized line |

The 1439 primary bytes and six complete EH thunk/table spans match live
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and the installed PE.
The report preserves all span SHA256 hashes and bytes, exact direct/tail call
rows, existing names, full prior documentation and function comments.
All names remain descriptive hypotheses; correct library names are retained.
No Ghidra metadata is changed by this packet. Exported gaps AF56FD..FF and
AF579D..9F are three-byte `LEA ECX,[ECX]` alignment instructions, jumped over by
the preceding branches; no missing executable tail requires repair.

## Raw ownership and current reads

Every allocation and nonnull return invokes the genuine current 419CC0 getter
over the borrowed 01090AA8 pool and 01090AA0 raw manager publication cells,
then BD1120 or BD1510. Returns pass the current 01090AA4 gate. Large blocks and
disabled small returns still call the getter first. No `noexcept` host storage
adapter, cached pool, private allocator or injected provider callback intervenes.
A getter failure during normal work propagates. The four-byte header is a raw
pointer, not the eight-byte length/data `NativeString` header.

Construction publishes the allocation before reloading a source header.
Assignment returns the old destination before reading the source; a null
source retains the now-returned pointer, and self assignment may reuse that
storage. Clear overwrites the current header only after the captured pointer's
return succeeds. Prefix/suffix functions retain count capture, publication,
CRT `strncpy` padding and a current-header reload for the final NUL. BF9280
remains the genuine CRT dependency: overlapping byte ranges are outside that
source provider's contract; unsafe header and publication aliases are not
repaired. No new check is added.

Token selection precedes skipping spaces: token0 of a leading-space line is
null. Suffix0 instead copies the leading spaces. Both construct output storage
without releasing an old output. Signed byte classification is retained.

AF5740 borrows the application's shared F8C2C8 scratch storage. It captures the
initial data pointer, scans with current cursor/extent, captures the scan end,
and trims signed leading bytes <=20h. The copy loop reloads the current data
pointer before its byte and the current cursor after each scratch store.
Interior signed bytes <20h are discarded. It writes the scratch NUL before
incrementing the current cursor and constructing the temporary from scratch.
This matters when scratch aliases cursor/data/header bytes. Capacity, bounds,
negative cursors, reentrancy and unsafe aliases receive no new policy.

## Exception evidence

These are genuine native unwind actions, not explicit catch handlers. Source
uses `noexcept` guard destructors for active unwind actions; a second exception
terminates. Normal destruction is explicit after disabling its unwind state,
so its first getter exception may propagate.

| Body | Handler and function info | Unwind map |
| --- | --- | --- |
| AEE3C0 | CBA651 -> DF2300, maxState2 | DF22F0: state1 -> state0, CBA630 clears temporary; state0 -> -1, CBA638 conditionally clears completed output |
| AF44C0 | CBABD1 -> DF2A0C, maxState2 | DF29FC: state1 -> state0, CBABB0 clears temporary; state0 -> -1, CBABB8 conditionally clears completed output |
| AF5740 | CBACF8 -> DF2B4C, maxState1 | DF2B44: state0 -> -1, CBACF0 clears temporary |

Token/suffix temporary cleanup covers both prefix/suffix assignment and output
copy. Once the output copy returns, native sets the output flag and state0
before normal temporary release. A failure in that release must unwind the
completed output. The two raw guards reproduce that transition. AF5740 enables
temporary cleanup only after its constructor succeeds. After output assignment
it captures the temporary pointer, disables state0, and performs the inlined
normal return without a second clear or cleanup on failure. The seven other
helpers have no local EH cleanup frame.

## Validation boundary

The focused ignored probe executes copies of all ten original bodies with
their original internal calls/branches and one shared scratch binding. The ten
bodies and internal direct calls are relocated together, and the three F8C2C8
operands bind that same scratch used by source. Only getter,
pool allocate/return and CRT strncpy calls are redirected to the same genuine
providers used by source. It compares tokens/suffixes, line filtering/cursors,
scratch aliasing of the current cursor, actual small-ring self assignment,
large allocations, disabled small returns, pool counters and raw manager drain.
The probe passes, including the scratch-alias result B/current cursor66, and
links with `/MANIFEST:EMBED`; no permanent tests are added. Plain strict Win32
build and both existing CTests pass. `verify_report_calls.py` verifies39
function-owned direct/tail rows. Three undefined compiler FH3 handler tails
remain explicitly raw-only, with full PE/live-byte and FuncInfo evidence; no
function ownership is invented for them.

This packet establishes raw source composition and bounded normal-path
original/source evidence. Native allocation failures, FH3 dispatch, secondary
unwind failure, CRT byte-range overlaps, hardware SEH, binary ABI compatibility
and game execution are not validated by this fixture. See the report for build,
probe and exact-call verification results.
