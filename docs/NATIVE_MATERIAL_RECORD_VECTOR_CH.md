# Native material record vector insertion (CH)

This source reconstructs B145E0 (755 bytes), B150D0 (172 bytes), and B15610
(164 bytes) over the actual vector fields: begin at +4, end at +8 and capacity
end at +Ch. Each record occupies 300 bytes. Names describe recovered behavior;
they are not recovered original symbols.

The original entries use ECX for the vector and pop four, four and one stack
words respectively. The new Win32 fastcall entries add the concrete raw string
pool context in EDX. Naked adapters expose their actual caller argument slots to
the source bodies and preserve RET10h/RET4; B150D0 returns its output pair in EAX.
No private stack-frame, hardware SEH or drop-in binary ABI identity is claimed.

## Insertion and mutation ordering

B145E0 first copy-constructs a complete temporary record, including when count
is zero. Signed wrapped pointer differences use the original IMUL 1B4E81B5h,
SAR 5, sign-correction quotient sequence. Unsigned capacity checks and the
DA740Dh maximum count precede either growth or in-place insertion.

Reallocation chooses the larger of the admissible 1.5 capacity growth and the
fresh size plus count, using the actual B0D230 getter for that final size read.
It saves allocation/cursor in the original count/source argument slots, copies
the prefix, fills the new records, then copies the suffix. Only these three
construction calls are inside native state 1's catch scope. Catch cleanup reads
the reused argument slots, destroys the completed prefix and frees its captured
allocation before rethrowing. Old-record destruction occurs after leaving that
scope. It frees the freshly reloaded old begin pointer, then reads the saved new
allocation and publishes capacity end, end and begin in that order.

For an in-place insertion with fewer trailing records than copies, the source
first copy-constructs the tail beyond the insertion range. Only the subsequent
fill construction is inside native state 3's catch scope. That catch recomputes
cleanup from current count/position argument slots and the current vector end.
Success applies the original single DWORD ADD to the current end, reloads it,
then assigns over the old tail. For a longer tail, it constructs the last count
records at the old end, publishes the returned end, performs backward assignment
and fills the insertion range. Captured pointers/counts and fresh reads follow
the assembly; completed earlier effects remain visible if assignment throws.

B150D0 captures position/begin, derives the original iterator index, applies the
returning invalid-parameter checks, calls insertion and checks the resulting
position against freshly read bounds before returning {vector, position}.
B15610 uses fill construction when capacity remains and publishes captured end
plus 300; otherwise it delegates through B150D0. The source adds no range clamp,
synthetic recovery, rollback or replacement pool.

## Cleanup and concrete source services

The original B145E0 handler CBC37B references FuncInfo DF4540, five states and two
catch maps at DF4564. State 0 unwinds through CBC370 to B10740 destruction of the
completed temporary. States 1/2/3/4 unwind to state 0. Catch entries B1479B and
B1484D implement the two separate cleanup scopes above. Source C++ cleanup uses
the same completed-object boundaries; temporary destruction during unwinding
terminates if it throws, whereas normal temporary destruction may throw.
The source uses its compiler's C++ exception machinery rather than the PE's
private FH3/SEH tables.

Raw CA/CC/CI copy, range and assignment entries and B10740 destruction are called
directly with the actual raw pool context. CJ supplies allocation and the shared
SBO-backed vector length error. Allocation/free and the returning invalid-
parameter path use the existing concrete source CRT domain. Original heap,
new-handler storage, RTTI and invalid-parameter globals are not binary-identical.

Two incorrect CALL_RETURN overrides after BF65AC calls hid the fall-through
bytes B14777..B14779 and B147B1..B147BC. The locked flow-repair tool verified
these bytes against the installed PE, recorded both old overrides, restored
the listing, saved the project and refreshed the export. It changed no global
callee no-return flag. See the separate CH flow report.

## Evidence and validation

The CH report pins all three complete bodies, both EH helpers and the EH tables
against the installed PE and live bsp.gpr program. It records direct callsites,
provider/source hashes, generated-code review and the exact build receipt.
Compilation and the existing math CTests do not establish vector execution or
gameplay behavior. Any focused local probe is reported separately with its
exercised branches and limits. Full diagnostics and material rendering remain
unfinished.
