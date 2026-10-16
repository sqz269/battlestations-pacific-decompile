# Actual checked DWORD-vector header InsertOne

The whole ordinary function at006988F0 is reconstructed over the accepted
actual00698400 insertion and qualified00BF6713 diagnostic Source services.
Coverage is139 bytes/57 current instruction starts, five actual CALLs,
and no function-local catch, cleanup action or owned temporary.
The established library identity STL_Vector_InsertOne_006988f0 is preserved;
the new descriptive C++ name remains a hypothesis.

## Native ABI and storage

ECX is the raw receiver and incoming EDX is unconsumed. Four physical stack
words hold output pair, iterator owner bits, position and fill-header pointer.
RET10h pops them. EAX returns the actual output address; child698400 EAX is
discarded. The receiver has16-byte header storage; this wrapper directly reads
only DWORD+4 begin and+8 end. It does not dereference an iterator-owner object.

After three pushes, [ESP+14h] captures the entry second word (owner).
After five pushes, [ESP+20h]/[ESP+24h] select position/fill, while
[ESP+18h] selects output later. The raw output pair has twoDWORD words.
No original STL C++ class or drop-in mangling/frame ABI is asserted.

## Exact operation and alias schedule

Capture initial begin once. If it is zero, set index0 without an end read.
Otherwise read initial end once and test the wrapped SAR4(end-begin).
A zero SAR result skips both initial diagnostics even when raw endpoints differ.
For a nonzero result, report if captured begin>end, then independently report
if captured owner is zero or differs from receiver. Both reports may return.
The index still uses the captured initial begin after these reports.

Call actual insertion with receiver, canonical unusedEDX0, captured owner bits,
position, literal count1 and fill-header pointer. This wrapper owns no private
fill copy or allocation; the actual child supplies its full ownership/EH.
The native child EDX contains fill pointer bits but is semantically unconsumed.

After the child returns, capture new begin once and compare it to CURRENT end.
Keep that captured begin across a returning diagnostic. Compute result as
captured new begin+(saved SAR index<<4), with DWORD wrap. Compare against
CURRENT end first. On upper failure, report and skip the lower CURRENT begin
read; otherwise load CURRENT begin and report a lower failure.
Retain result across the last returning report.

Finally store output+4=result before output+0=receiver and return output.
Public output/header/value aliases retain this order. Source parameter/private
frame aliases do not recreate original incoming stack aliases.
No null/endpoint/owner guard, snapshot, self repair or rollback is introduced.

A pre-child diagnostic failure has no insertion/output stores by this wrapper.
Child failure propagates its actual progress. A post-child diagnostic failure
retains child mutations and precedes both output stores. There is no local EH.
Original access-fault/SEH and partial faulting-store behavior remain unproved.

## Direct actual services and provenance

The accepted parent Source pin8180c34d839f8fd356ebfe50545f2a65585af388 is
published in Mainf3cf8a3c6d322b232af943c442ce4d1f3fc38ae1, as Root confirmed.
The initial local sync pointf3f4c75392fb7a0a1b6e7facab21ef3c9a0f05cf
was accepted before that publication receipt; the final dependency sync uses
publishedf3cf. Both exact parent Source/header hashes are recorded.

The actual qualified diagnostic API is invoke_native_invalid_parameter_00bf6713().
Its accepted naked Source entry calls the selected SDK zero-argument
_invalid_parameter_noinfo, with neither noexcept nor noreturn promised.
Its five physical zero words are unused Source padding. This does not establish
the original00BF66EF encoded-global/debugger/dynamic-handler/Watson policy.
No callback/handler replacement, exception conversion or fake CRT body is added.

| Native actual instruction | Source binding |
| --- | --- |
| 00698916 CALL 00BF6713 | invoke_native_invalid_parameter_00bf6713 |
| 00698923 CALL 00BF6713 | invoke_native_invalid_parameter_00bf6713 |
| 00698940 CALL 00698400 | insert_native_dword_vector_header_count_00698400 |
| 0069894D CALL 00BF6713 | invoke_native_invalid_parameter_00bf6713 |
| 00698965 CALL 00BF6713 | invoke_native_invalid_parameter_00bf6713 |

The only current caller is00698980 Append. This packet does not reconstruct
or change it or the configuration owner. Prior append_input_checked_row_storage
and internal insert_row in native_input_configuration_modifiers.cpp remain
a separate bounded interface; their Source/header fingerprints are preserved.

## Verification and limits

BSP verifies saved C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe
before Native batches. Full PE/live139-byte equality and57 exact current
starts match the frozen caller-readiness evidence. No Ghidra mutation/repair.
The required default MSVC Win32 build passed; all three existing suites passed
in14.08 seconds. Five report and five Markdown transfer claims pass. Fourteen
actual CMake COFF checks pass: all221 code bytes/98 starts in six sections
(main159/65, five helpers62/33), all five actual REL32 targets, no padding/EH.
The compiler reduces the saved index SAR4 to SHR4 because its only later use
is SHL4: both produce identical final DWORD byte offset d&FFFFFFF0. This does
not identify native intermediate registers/flags or fault timing.
No new tests were added. CMake registration is one local append
under COORDINATION93-95, without a shared registry lease or queue.

CurrentCpp/raw-access/CRT policy is distinct from original heap/new-handler,
RTTI, FH3/SEH/fault, private argument/frame/register/flag and binary ABI identity.
Static, build and emitted-object checks do not establish native/game behavior.
