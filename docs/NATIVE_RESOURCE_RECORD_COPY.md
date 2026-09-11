# Actual resource-record copy and destruction

Two complete original entries now have actual-storage C++ implementations in
`src/native_resource_record_copy.cpp`: 4D6F70 (162 bytes) copies the actual 2Ch
record, and 4D45A0 (120 bytes) destroys its owned list and name. They compose the
actual owning-pool alias overloads from commit05cec644 and existing raw string
providers. Existing typed particle/render APIs are unchanged. The cache,
vector append/growth, and resource ownership routes remain separate work.

The source is strict Win32 build-checked and compared with original machine
code in one focused composition. It is not a binary ABI replacement and has
not run in the game. Names are descriptive hypotheses. The audit pins 18 fresh
guarded spans (1,133 bytes), including both full entries, their two eight-byte
unwind actions, FH3 records, and current dependency evidence. The five-byte
BF7680 span is a CRT memcpy bridge preimage, not a full memcpy implementation.

| Entry | Original ABI | Source API |
| --- | --- | --- |
| 004D6F70..004D7012 | ECX destination; stack source; EAX destination; RET4 | `copy_construct_native_resource_record_004d6f70(destination, source, actual_pool, validation)` |
| 004D45A0..004D4618 | ECX record; RET; no specified result | `destroy_native_resource_record_004d45a0(record, actual_pool)` |

Both APIs use `NativeRenderResourceRecord` as actual caller storage: name
length/data at0/4; preserved DWORD+8; list sentinel/count+C/+10; five words at
+14/+18/+1C/+20/+24; raw resource pointer+28. The alias owner is the actual
record address+8. There is no copied list header, synthetic owner, or resource
pointer dereference. Neither entry increments/decrements a resource count or
calls its virtual slots. Existing consumer-specific retention lies outside
these bodies.

4D6F70 compares destination/source identity before zeroing the name. For
different records it resizes the actual destination header through full41DD40
with preservation enabled, then reads the current source length, destination
length/data, and source data in native order for the name copy. Initial name
copy precedes registration of name cleanup. A failure there does not acquire
new cleanup responsibility. The existing C++ string boundary omits a zero-byte
memcpy; no claim is made about observing a zero-length CRT call.

At004D6FCC the native frame arms state0. The source then copies the actual list
through 4D48A0 at record+8, selecting `ActualNativeStringPoolStorage&` throughout
allocation, range rollback, and cleanup. After that call returns, the five
payload loads and stores alternate in increasing address order, followed by
the raw+28 pointer load/store. Volatile accesses preserve that schedule and
overlapping-storage observations. The destination+8 word stays untouched.
Self construction zeroes/abandons the old name and publishes a new empty list,
matching the existing list-copy contract; no assignment cleanup is introduced.

Its FH3 registration atC66228 selects FuncInfoD8F084. The one unwind-map entry
atD8F07C has state -1/actionC66220. That complete eight-byte action loads the
current record from `[EBP-10h]` and tail-jumps to41DD20. The source catch releases
only the current actual name header and rethrows; list-copy ownership is handled
by the existing complete alias chain. It does not add a second list cleanup.

4D45A0 has state0 across the inlined list clear/current-sentinel free/null
sequence. The source composes the complete4D0A10 algorithm for exactly those
operations, including its call to current4D05E0 and post-free sentinel store.
It then captures the record's current name-data pointer, still inside the armed
region. State becomes -1 at004D45E8, before reading name length or invoking the
pool getter/return. For a nonnull captured pointer, the source returns that
pointer with current length+1 through the actual pool. Name fields, payload,
and+28 pointer are left as they stand after callbacks. A null name pointer
skips the length read and return.

The destructor's C65FD8 registration selects FuncInfoD8ED6C and its one map
entry atD8ED64, state -1/actionC65FD0. The eight-byte action again loads the
current record from `[EBP-10h]` and tail-jumps to41DD20. Name cleanup is thus
armed during list cleanup and disarmed before final normal name return. The
source preserves both regions, including the pre-disarm data capture. The EH
actions are dependency evidence owned with this packet, not two additional
claimed full reconstructed entries.

The pool parameter borrows the application's real01090AA8 publication,
01090AA4 return gate, and canonical lifetime domain with
`NativeStringPoolLifetimeBinding`. Existing41DD40/41DD20 operate on actual
eight-byte headers. `ActualNativeStringPoolStorage` calls the current419CC0
getter on every allocation/return and uses complete native BD1120/BD1510 storage
providers. No new allocator or callback provider is introduced. The actual
alias overloads supply the complete insertion/rollback/count/iterator/string
dependencies documented in the prior alias-pool audit. Current archive objects
and relocation references are frozen, including inlined code and unused legacy
overloads; this does not imply every object section executed.

The ignored focused fixture loads the original complete copy/destructor and
their FH3 records, then bridges string/list/pool operations to the exact current
linked library. It compares original and rebuilt executions against the same
real constructed owning-pool layout and normalized allocation trace. Normal
copy preserves+8, copies nonempty name/list/date/pointer fields, and returns the
destination. During normal destruction, a real CRT free observer changes the
name header when the sentinel is freed. Both executions capture and return the
new current buffer; the fixture verifies small-buffer reuse and unchanged
header values.

In the failure path, a real allocation boundary changes the source sentinel
during alias insertion. The existing returning CRT boundary throws from the
post-link validation site. Complete actual list rollback/destruction runs, then
the original record frame executes C66220 exactly once. Original and rebuilt
copies both return the current name buffer and leave uncopied date/resource
fields unchanged. All observed CRT allocations are freed. The full traces
match in170 words. Original4D6F70,4D45A0 andC66220 execute; C65FD0 is verified
statically and is not claimed as a dynamically tested exception path. Original
machine code for every bridged dependency, lazy pool creation, native SEH, and
all possible invalid-storage cases were not exercised. There are no new
permanent tests. Standard `/fp:strict /W4 /WX` Win32 build, both existing CTests,
and all eight seed byte comparisons pass.

The existing string destruction/release API is `noexcept`; failure during lazy
pool creation terminates at that source boundary. This is not asserted as a
native failure rule. Host C++ catches and the existing owning length exception
do not reproduce the original MSVC exception ABI. Native SEH/invalid-memory
behavior and arbitrary external mutation outside the established returning
boundaries remain outside the source interface.

## Primary integration

The unchanged source is registered in main CMake with two distinct raw records;
the older typed particle destructor record remains. Main strict Win32, both
existing CTests and eight fresh seeds passed. Primary checked 65 worker pins,
29 current files, 18 fresh spans (1,133 bytes) and 13 exact main archive objects
with all reviewed code/data/relocation contents. The unchanged fixture linked
only the frozen actual main library and reproduced all 170 trace words. Full
original copy/destruction and the C66220 copy action executed; C65FD0 remains
static evidence. The immutable bundle is `local/resource_record_copy_primary/`.

Primary removed only the returning-free CALL_RETURN override at4D45D4 and
restored the complete saved destructor through4D4617. It preserved prior names
and comments in the journal, applied descriptive string-alias-record names,
saved four annotations including both EH actions, and refreshed the exports.
No global library no-return metadata was changed. The source exception and
original-helper/runtime/gameplay limits above still apply.
