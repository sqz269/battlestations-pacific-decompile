# Actual owning outer-vector Resize at 006A6350

Packet `oct15_outer_vector_resize` adds complete actual Source for
`STL_CheckedStringVectorVector_Resize_006A6350` and its exact parameter-owner
action `00C7F4B0`. The old qualified
`resize_native_input_settings_name_pairs_006a6350` field-storage fragment in
`native_input_settings_vector_storage.cpp`, its fixtures, and its evidence
remain separate and unchanged.

`resize_native_outer_vector_006a6350` uses the actual `NativeStringRawPoolContext`,
accepted full InsertCount `006A5C50`, and accepted full erase `006A5BF0`.
`unwind_native_outer_resize_parameter_00c7f4b0` explicitly receives the owned
inner header and forwards actual `004324A0`. There is no host container,
callback, alternate allocation policy, or unresolved-body substitute.

## Original ABI and consumed owner

Native ECX is the actual 16-byte outer header; incoming EDX is unconsumed. Five
stack words are requested count followed by an owned, by-value 16-byte inner
vector header. The entry has no semantic result and returns with `RET 14h`.
The outer header has opaque +0, begin +4, end +8, capacity +C; its elements are
actual 16-byte inner headers with those same fields. Inner strings are actual
8-byte length/data headers in the accepted pool/CRT ownership domain.

The Source API takes the owned parameter's four words as scalar bits and puts
them into one canonical private header. This transfers the existing owner; it
does not invoke a copy constructor, allocate another inner vector, or create
a second owner. Opaque +0 is copied only as a bit pattern and is never newly
owned, dereferenced, or destroyed. The consumed begin/end/capacity owner must be
valid under the actual cleanup services. Source callers may supply a defined
dead opaque word without evaluating C++ indeterminate data; native callers do
not necessarily initialize that word. The explicit context and private header
are new Source interfaces, not an original by-value stack ABI adapter.

The receiver's begin is read before owner state 0 is armed. All subsequent
admitted paths consume the fill owner, including growth, shrink, equal size,
zero size, and a child/diagnostic C++ exception. Initial malformed-memory faults
and native private-frame aliases remain outside the new Source interface.

## Size decisions and returning diagnostics

Sizes use DWORD subtraction followed by arithmetic shift by four, compared
unsigned. Source preserves wrapping bit patterns rather than host pointer
subtraction or signed container sizes. A null captured begin supplies size
zero without an end read. No capacity field is read by this wrapper.

If the initial size is below requested count, Resize rereads size from the same
captured begin, then separately captures current end. It reports if the initial
begin is above that captured end. A returning diagnostic retains both the
captured size and end. Full actual InsertCount receives receiver again as its
unused owner, that captured end as position, requested minus captured size,
and a borrowed pointer to this function's owned fill header. InsertCount owns
its own private deep copy; Resize continues to own and later destroys the
original consumed parameter.

Otherwise, nonnull initial begin causes a fresh end capture and size comparison.
Resize only enters shrink when requested remains below that reread size.
Its first diagnostic compares initial begin with the saved end. After a
returning report it reloads begin, compares that reload with current end, and
retains the reloaded begin across a returning second report. First endpoint is
that retained begin plus requested times 16. It compares this with current end,
then reads current begin for the lower comparison only when the upper check
passes. Failure of either bound uses the third shrink diagnostic. The endpoint
and original saved end survive a returning report.

Full erase receives a private two-DWORD output pair, receiver as both owners,
that derived first endpoint, and the originally captured end as last endpoint.
The native private pair's second word is preset to the retained begin; it is
overwritten by the accepted child and never used as a synthetic iterator model.
The child's output address result is discarded. There are no additional owner,
null, alignment, self, range, or capacity checks, and no rollback of receiver
changes if a child fails.

## Normal and exceptional parameter cleanup

Normal exit captures the owned parameter's begin with state 0 still armed,
then sets owner state -1. For nonnull begin it captures the parameter's current
end, destroys strings through actual `00432050`, and frees the cached begin. It does not reload
begin after string destruction or clear the private header fields. Null begin
skips end read, range destruction and free. In argument order, the two native
metadata words are parameter-header address and requested count; the accepted
child consumes neither. Source does not invent their stack aliases.
Normal destructor work is outside the C++ owner catch, so a failure there is
not retried through the unwind action.

The exact action at `C7F4B0` is `LEA ECX,[EBP+8]`, followed at `C7F4B3` by a jump
to `004324A0`. That actual cleanup captures the inner string range, destroys
it, frees the parameter's current begin, and clears +4/+8/+C in order. Source
passes the canonical private header explicitly; it does not simulate native
EH EBP. A C++ exception in growth, shrink, or a returning-capable diagnostic
invokes this action and rethrows. Cleanup failure stops later work under the
current C++ service contract; original FH3 handler-failure behavior is not
supplied.

Handler stub `C7F4B8` loads FuncInfo `DAC8F8` then jumps to `BF6B43`. Its 44-byte
metadata region is `[DAC8F0,DAC91C)`: magic `19930522`, max state 1, flags 1,
zero try blocks, and the sole unwind entry `(state0 -> -1, action C7F4B0)`.
Only after these raw maps and action bytes were verified was the packet lease
extended to that action. The handler stub remains read-only native metadata;
it is not a new Source FH3 implementation or an additional passed owned row.

## Complete byte and caller evidence

The complete native body `[006A6350,006A644D)` is 253 bytes and 95 instructions;
disk/live bytes and every refreshed export start agree. SHA256:
`0cd48b052eb368cadc2fc75e2337adc89dd77fb728419fb4542e93b718fa282f`.
The separate owner action is 8 bytes/two instructions,
`8d4d08e9e82f7bff`, with exact `004324A0` target. The complete 44-byte EH map
matches disk/live with SHA256
`f90e07fc2030729e68e47cf567ee431ea7f4d2dd9a48b9c27a635b7a8e90aa9b`.
Live batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 Win32, image base `00400000`. Exports were
force-refreshed and complete typed metadata captured read-only. No Ghidra
mutation or repair was performed.

There are two direct native sites in `BSP_InputSettings_LoadDataTables`
(`006A7BE0`). Their bounded before/after setups were inspected, not that whole
large caller. Each reserves 10h stack bytes for the inner header, writes
+4/+8/+C, pushes requested count from ESI, and places the outer receiver in
ECX. The first copies EBP bits into the three owner fields; the second explicitly
zeros them through ECX. Neither setup writes opaque +0. These are by-value
parameter producers, not a single pointer argument. Native `RET 14h` consumes
the requested count and that parameter. The callee's cleanup/EH evidence also
covers admitted nonempty fill ownership; these observed setups alone are not
a runtime test of that general case.

All ordinary native sites are:

```text
006A63A2 CALL 00BF6713
006A63B3 CALL 006A5C50
006A63D0 CALL 00BF6713
006A63DD CALL 00BF6713
006A63F5 CALL 00BF6713
006A6405 CALL 006A5BF0
006A642A CALL 00432050
006A6430 CALL 00BF65AC
006A8786 CALL 006A6350
006A8F19 CALL 006A6350
```

The owner action transfer is separately declared `kind: tail_jump` in the
JSON report. It is not counted as a native CALL. The four diagnostic sites use
the accepted actual `BF6713` current-SDK adapter. Returning continuation is
preserved, but the original encoded-handler/Watson/native CRT identity remains
separately qualified. `BF65AC` is bound to the accepted current CRT free service.

## Verification and limits

Accepted erase/insertion commit `9c6606552` was merged before the parent build.
The warm MSVC Win32 build passed all three existing suites in 8.85 seconds:
`reconstructed_math`, `native_math_differential`, and `tool_tests`. No new tests
were added. The math differential suite covers established seeds, not this new
Resize entry. All 11 JSON instruction rows passed, including the explicit owner
tail jump and two caller sites. All 10 Markdown call claims passed with no
unchecked claims or warnings. The owned-ledger audit preserved earlier names,
evidence, reconstruction fragments and fixtures, added only two actual Source
function records, and checked the exact CMake append. Index refresh and diff
checks passed; the JSON report records these results.

Build/static evidence does not establish native private-stack/register ABI,
original CRT/FH3/SEH/RTTI/exception identity, cleanup-failure termination,
unrestricted frame aliases, invalid-memory faults, nonlocal jumps, concurrency,
new Resize runtime differential, application wiring, or gameplay equivalence.
The accepted allocation, deep-copy, assignment, transfer, erase, diagnostic,
destruction and current C++ exception service limits remain inherited.
