# Actual outer string-vector InsertCount at 006A5C50

Packet `oct15_outer_string_vector_insert_count` reconstructs complete general
insertion into an actual checked vector of actual checked NativeString vectors.
The established library name
`STL_CheckedStringVectorVector_InsertCount_006A5C50` is retained. This is a new
Source binding to `NativeStringRawPoolContext` and accepted actual services,
separate from the historical qualified field-storage fragments and fixtures in
`native_input_settings_vector_storage.cpp`.

The new public entries in `native_outer_string_vector_insert.hpp/.cpp` are
`insert_native_outer_string_vector_count_006a5c50` and the explicit temporary
owner action `unwind_native_outer_insert_value_00c7f470`. The two catches and
normal cleanup are represented by the parent's C++ scopes. No fake native
frame, generic container, callback, allocator model, or replacement assignment
is introduced.

## Native ABI, storage, and prototype

Native ECX is the actual 16-byte outer header. Incoming EDX is unconsumed.
The four physical stacked words are unused owner, insertion position, unsigned
count, and borrowed pointer to an actual 16-byte inner vector header. The
function returns with `RET 10h` and has no semantic return value. Both actual
headers retain opaque +0 and use begin/end/capacity at +4/+8/+C. Outer elements
are 16-byte inner owning headers; their strings are actual 8-byte length/data
headers using the accepted pool publication/gate/manager domain.

The accepted actual inner copy constructor `00557BC0` deep-copies the borrowed
prototype into private 16-byte storage before the first receiver-begin read.
Opaque +0 remains uninitialized and unconsumed. Native state 0 arms only after
that first begin read. Capacity is calculated before testing count, so even
count zero constructs and destroys the prototype and reads capacity when begin
is nonnull. A constructor failure propagates through that child's own owner
scope, before the parent's temporary cleanup is armed. Prototype aliases to an
existing outer element retain the saved deep copy through subsequent transfers.

All counts use wrapping DWORD subtraction and arithmetic shift by four, with
unsigned comparisons. The length threshold is `0FFFFFFF`; no pointer-distance,
host-container, alignment, owner, null-header, position, self, or overflow
validation is added. The consumed position/count/header/backing accesses must
be admitted by the actual services' contracts.

## Growth and ownership transfer

The initial begin supplies the capacity calculation and each repeated size
calculation before allocation; current end is reread at each native size site.
Length checking precedes the capacity comparison. Growth uses capacity plus
its logical half, selects zero if that addition exceeds `0FFFFFFF`, and then
raises the candidate to a freshly reread size plus count if necessary. The
allocator is actual `00553300` with 16-byte elements and the accepted current
CRT/new-handler/owning-throw contract, not a new allocation callback.

After allocation, receiver begin is read again for the prefix. Actual
`006A3200` constructs an empty inner header at each destination and transfers
begin/end/capacity through ordered field swaps. It does not deep-copy existing
inner strings or assign whole headers. Opaque +0 stays at its physical slot.
The growth schedule is prefix transfer, inserted uninitialized deep-copy fill
through `006A5170`, and suffix transfer. Suffix uses the current receiver end.
The completed pointer advances only after a successful phase. The final suffix
return is ignored. Earlier transferred source headers are not restored.

State 1 protects only these construction phases and the subsequent old-begin
capture. Catch `006A5DC5` destroys the previously completed new range through
actual `0055AC60`, frees the new base, and rethrows. Each failed child separately
cleans its own completed partial phase. The parent does not include that partial
phase twice. There is no rollback of transfers already made in the old backing.

State 0 resumes before old cleanup. The captured old begin and freshly read end
supply the new element count and old range destruction through `0055A3A0`.
Free then reloads the receiver's current begin. A failure in this cleanup does
not reenter the new-backing catch. Publication is capacity, then end, then begin;
the new end is new base plus the recalculated size/count, not the ignored final
transfer result. The temporary remains owned until normal cleanup is disabled.

## Both in-place branches

The capacity-sufficient path captures current end and computes the unsigned
tail count from end minus position. It retains that end independently of later
receiver loads.

If tail is shorter than count, `006A4B50` transfers the existing tail to
position plus count times 16. It then rereads current end and derives the
remaining uninitialized-fill count. State 3 arms only for the ensuing fill and
end-publication schedule. On success it adds the original count times 16 to
current receiver end, reloads the result, and subtracts that byte count to
obtain the assignment endpoint. State 0 resumes before actual assigned fill
`006A2A00` fills the existing gap from position to that endpoint.

Catch `006A5E4E` reloads current receiver end and destroys
`[position + original_count*16, current_end + original_count*16)` through
`0055AC60`, then rethrows. It neither caches the earlier end nor restores the
already transferred tail. The fill child handles its own partial construction
prefix. A failure in the later assigned fill does not enter this catch.

If tail is at least count, `006A4B50` transfers the final count headers to the
captured old end and its returned endpoint is published immediately. Actual
`006A4B80` then swaps the preceding range backwards into that captured old end.
Finally `006A2A00` assigns copies of the saved prototype into
`[position, position + count*16)`. There is no local range catch or rollback
around the backward swaps or assigned fill. This is general middle insertion,
including empty ranges and the append endpoint; it is not an append-only port.

## Owner cleanup and original EH metadata

Normal `006A5EA8` is a fragment inside the parent, not a callee or separate
function. It captures temporary begin with state 0 still armed, then disables
owner cleanup. For nonnull begin it captures temporary end, destroys actual
strings through `00432050`, and frees the cached begin. It does not reload begin
for that free or clear the private fields. Its two native metadata stack words
to `00432050` are unconsumed by the accepted actual child.

The owner action `00C7F470` instead executes `LEA ECX,[EBP-28]` and jumps to
actual `004324A0`. Source receives that private header explicitly and composes
`destroy_native_string_header_vector_004324a0`: captured range destruction,
current-begin free, ordered +4/+8/+C clears. It does not substitute outer tidy
`0055B110`. Normal destructor work is outside the parent's C++ catch, so an
exception there is not retried through the unwind action.

Native handler stub `00C7F478` loads FuncInfo `00DAC850` and transfers to
`00BF6B43`. It is read-only metadata evidence outside this packet's Source
coverage. The 148-byte owned-function EH structure is `[00DAC830,00DAC8C4)`:
magic `19930522`, max state 5, flags 1; state 0 unwinds to -1 through `C7F470`,
and states 1..4 unwind to 0 without an action. Two catch-all descriptors have
adjectives `40h`, null type and object displacement. Their try maps are
`(1,1,2)->6A5DC5` and `(3,3,4)->6A5E4E`. Both handlers issue native
`__CxxThrowException(0,0)` after cleanup; Source uses bare C++ rethrow.

Source retains these scope boundaries and partial ownership schedules. Original
FH3/SEH dispatch, native frame and private spill/argument aliases, EH payload
identity, cleanup-failure termination, and invalid-memory faults are not
reconstructed. A current C++ cleanup failure stops the remaining work and can
replace the active exception according to these C++ scopes; no native EH
runtime equivalence is claimed. Normal and exceptional earlier writes and
transfers remain visible; no retry, restoration, or synthetic owner is added.

## Complete native evidence and caller

The contiguous parent/catch bytes `[006A5C50,006A5EE4)` comprise 660 bytes and
254 instructions; disk/live bytes and all refreshed export instruction starts
agree. SHA256:
`5d23a4d9e34fa749980461669d86250a6006d8fd348e10edbd9733d7c76b455f`.
The live parent has three complete normal ranges totaling 591 bytes, with
separate 34-byte growth and 35-byte short-tail catches. Complete typed metadata
confirms `6A5EA8` has no function start and belongs to the parent. Legacy
snapshot graph/enclosing hints that extend catch bodies into normal code are
not current ownership evidence. Exports were force-refreshed read-only; no
Ghidra mutation or repair was needed.

The owner action is another 8 live/disk bytes and two instructions:
`8d4dd8e928307bff`. Its target is `004324A0`. The complete 148-byte EH structure
matches live/disk with SHA256
`34bbe53dcf11ea216e659f74af405b9e8c20e7d4b56de87e25294869bb4dd8da`.
All live batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 Win32, image base `00400000`.

The sole direct caller is actual outer Resize `006A6350`. Its complete
95-instruction body shows `006A63B3` passing receiver again as unused owner,
captured end position, desired minus separately captured current size, and a
pointer to its owned by-value 16-byte inner prototype. Resize retains ownership
of that parameter and cleans it after this call. This parent receives a
borrowed pointer, independently deep-copies it before reading receiver begin,
and owns only its private copy. The earlier qualified Resize fragment is not
replaced or widened by this packet.

The complete native dependency sites, including both catch bodies, are:

```text
006A5C7D CALL 00557BC0
006A5CBC CALL 00556C90
006A5D1D CALL 00553300
006A5D40 CALL 006A3200
006A5D50 CALL 006A5170
006A5D6C CALL 006A3200
006A5D99 CALL 0055A3A0
006A5DA2 CALL 00BF65AC
006A5DD0 CALL 0055AC60
006A5DD6 CALL 00BF65AC
006A5DE2 CALL 00BF6885
006A5E0A CALL 006A4B50
006A5E27 CALL 006A5170
006A5E44 CALL 006A2A00
006A5E63 CALL 0055AC60
006A5E6C CALL 00BF6885
006A5E82 CALL 006A4B50
006A5E90 CALL 006A4B80
006A5EA0 CALL 006A2A00
006A5EC3 CALL 00432050
006A5EC9 CALL 00BF65AC
006A63B3 CALL 006A5C50
```

The direct `006A3200` physical tag3 repeats the destination and tag4 has only
its low byte set to zero. Their values are unconsumed. Source preserves the
duplicate destination and supplies a defined zero dead tag. Dispatchers retain
their accepted independently computed results and dead-slot qualifications.
Source does not recreate original ambient EDX/EAX residue, stack cleanup and
argument-slot overlays (including the stacked backward-swap and fill words).

## Verification and limits

Accepted dependency commit `08c2e2bca` was merged before the parent build. The
warm MSVC Win32 `./scripts/build.ps1` compiled the new parent without new C
warnings and passed all three existing suites: `reconstructed_math`,
`native_math_differential`, and `tool_tests` (8.76 seconds total). The existing
unrelated duplicate `spawn_request_id_matches` linker warning remained. No new
tests were added. The native differential suite covers established math seeds,
not this new parent.

All 23 JSON instruction rows passed: 21 owned calls, one explicit owner tail
jump, and the sole caller. All 22 Markdown call claims passed with no unchecked
claims or warnings. The owned-ledger audit preserved all earlier fields,
evidence, reconstruction rows and qualified fragments; new records cover only
the five leased addresses. It explicitly records `6A5EA8` as a fragment, without
creating a function name there. Only the exact parent registration was appended
to CMake. Index refresh and final diff checks are recorded in the JSON report.

The Source interface has an explicit context and different private frames.
Build/static evidence does not establish a new parent native runtime
differential, original CRT/FH3/SEH or binary ABI compatibility, unrestricted
alias/fault/concurrency behavior, or gameplay validation. Current allocation,
length-error, raw-pool copy, assignment, transfer and cleanup service limits
remain inherited explicitly.
