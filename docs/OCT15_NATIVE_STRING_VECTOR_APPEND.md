# Actual pooled-string vector Append at 00450540

Packet `oct15_native_string_vector_append` completes a new Source reconstruction
of the actual ordinary Append entry
`STL_CheckedNativeStringVector_Append_00450540`. Its established library name is
preserved. The earlier bounded `append_checked_native_string_storage` fragment
in `native_checked_string_storage.cpp` and its fixtures remain separately
qualified; this packet does not replace or widen their evidence.

The new entry is `append_native_string_header_vector_00450540` in
`src/native_string_vector_append.cpp`, declared in the corresponding new header.
It uses the actual `NativeStringRawPoolContext`, accepted uninitialized fill,
and accepted InsertOne. It introduces no allocator, string, iterator, callback,
validation policy, or ownership model.

## Original ABI and storage

The native entry receives the vector header in ECX and one borrowed string-header
pointer on the stack, and returns with `RET 4`. Incoming EDX is unconsumed and
the native EAX value has no semantic return contract. The actual 10h vector
header retains its opaque word at +0 and has begin/end/capacity pointers at
+4/+8/+C. Elements are actual 8-byte NativeString length/data headers, governed
by the accepted raw-pool services. These offsets are shared with the fully
reconstructed actual InsertOne/InsertCount and their allocation, copy, fill,
destruction, and range services; they are not a host STL layout assumption.

Both size and capacity calculations use DWORD subtraction followed by arithmetic
right shift by three; their comparison is unsigned. Source uses explicit
32-bit wrapping arithmetic and sign extension rather than pointer subtraction
or a signed host-container count. It preserves raw accesses and does not add
alignment, null-header, range, or overflow validation.

## Instruction and dataflow order

The initial begin is captured first. A null begin supplies size zero without
reading the initial end, skips capacity entirely, and takes the slow path. For
a nonnull begin, the initial end supplies size and capacity is calculated using
the same captured begin. Strict unsigned size-less-than-capacity selects the
fast path. Thus an allocated empty vector with spare capacity uses the fast
path, while a full or overfull count uses the slow path.

The fast path captures the current end after those tests, then constructs one
actual NativeString there through `fill_native_string_uninitialized_0044ce20`.
Only successful completion publishes captured end plus eight at header +8.
There is no post-construction end reload, no increment of a potentially changed
current end, and no begin/capacity publication. A borrowed value inside the
vector is passed directly to the actual child; this wrapper makes no private
prototype copy.

The slow path captures the current end after the tests and reports through
`invoke_native_invalid_parameter_00bf6713` if the original begin is above that
captured end. A returning handler continues with that same captured position.
Actual `insert_native_string_header_vector_one_00450180` then receives the
receiver again as owner, that position, and the borrowed value. Its own
validations, loads, InsertCount composition, and output ordering remain active.
Append discards the private two-DWORD iterator result. It does not bypass
InsertOne by calling InsertCount directly.

The complete native dependency sites are:

```text
00450589 CALL 0044CE20
004505A3 CALL 00BF6713
004505B6 CALL 00450180
```

CE20 receives ECX=captured end and EDX=1, plus four physical stack words in
logical order: value, owner, duplicate value, and scratch tag. Only the first
two stack words are consumed by its accepted actual service. Append writes only
the low tag byte to zero; its high 24 bits remain unspecified. The Source
interface omits those two dead words and does not evaluate indeterminate C++
data. On the slow path the same native private eight bytes hold InsertOne's
output pair. Source supplies an ordinary private two-word scratch pair and a
defined zero for InsertOne's unconsumed EDX. It does not reproduce the native
scratch overlay or original argument-slot aliases.

## Ownership and exception boundary

This wrapper has no native EH registration, cleanup state, catch funclet, owned
prototype, or rollback action. Its borrowed value remains owned by the caller.
The fast child owns its completed construction prefix and its existing failure
cleanup contract; a failure escaping that child skips end publication. The slow
child retains the accepted InsertOne/InsertCount temporary, growth, prefix,
and in-place exception behavior. Source adds no wrapper catch or cleanup.

These are current C++ service compositions. They do not reproduce the original
FH3 runtime, SEH dispatch, binary frame, invalid-pointer faults, nonlocal jumps,
or cleanup-failure termination behavior. The current invalid-parameter adapter
has its separately qualified current-CRT policy; it is not proof of the
original native CRT encoded-handler/Watson implementation. Returning-handler
continuation is preserved within that declared Source domain.

## Evidence and caller contracts

The complete body is `[00450540,004505C3)`: 131 bytes and 53 instructions.
Disk bytes, live Ghidra bytes, and every saved assembly instruction start agree.
SHA256 is
`b78e4c1224ccfaca9b23e5da76842bdc791be527bdac8df87c0897e2bc390c3d`.
Read-only live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 Win32, image base `00400000`. Complete current
typed metadata was captured for the entry, its three dependency sites, and all
150 direct references. No Ghidra mutation or repair was performed.

All 150 reference setups were inspected with bounded before/after instruction
contexts. They comprise 149 actual CALL instructions and one tail jump across
61 containing functions, all individually identified in the JSON report. Their
receivers are actual vector headers; values are borrowed pointers to local,
member, existing-element, or helper-produced NativeString headers. This audit
establishes the consumed call setup, not the complete behavior of those 61
caller bodies.

The special queued-value setups at `008ED33C`, `008ED354`, and `008ECAEE` leave
the value pointer below a subsequent map key argument. The intervening actual
map lookup at `008EC460` returns with `RET 4` at `008EC545`, consuming only its
key and leaving the queued value for Append. Its returned vector is placed in
ECX. There is still only one consumed stacked argument to Append.

The eight live/disk bytes `83c104e908f89aff` at `00AA0D30` are the complete
localization forwarder: add four to ECX, then jump to Append at `00AA0D33`.
It leaves the inherited stack intact. Ghidra records `CALL_RETURN` on that
instruction: default flow is an unconditional jump, effective flow is a call
terminator. Accordingly the report labels it `kind: tail_jump`; it is not
counted as a native CALL. Its prior metadata is retained without mutation.

## Verification and limits

The warm-cache MSVC Win32 `./scripts/build.ps1` passed all three existing suites:
`reconstructed_math`, `native_math_differential`, and `tool_tests` (9.48 seconds
total). The native differential suite covers established math seeds, not this
new Append entry. No new tests were added. The existing unrelated duplicate
`spawn_request_id_matches` linker warning remained.

The report verifier passed all 153 rows: the three dependencies, 149 ordinary
caller sites, and the explicit tail transfer against current native instructions
and owners. Its three Markdown claims passed with no unchecked claims or
warnings. The owned-ledger audit preserved all earlier name fields/evidence and
the prior fragment; only new actual Source evidence and its distinct function
record were added. The index refresh and `git diff --check` passed. Build and
static native evidence do not establish a
new Append runtime differential, drop-in binary ABI, original CRT/FH3/SEH
compatibility, private-stack alias equivalence, concurrency semantics, or game
validation. The explicit pool context is an added Source parameter.
