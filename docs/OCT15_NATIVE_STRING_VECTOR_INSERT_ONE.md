# Actual pooled-string vector single insertion

This packet reconstructs the complete `00450180..00450208` wrapper against
the accepted actual `0044F410` count-insertion service and the existing
`00BF6713` invalid-parameter service. Source is
`src/native_string_vector_insert_one.cpp`; its contract is
`include/bsp/native_string_vector_insert_one.hpp`. The older qualified
checked-storage append fragments remain separate and unchanged. Descriptive
Source names are hypotheses, not recovered symbols.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete cached pseudocode and native listing,
all137 entry bytes, live prototype and the sole incoming call were checked
through `bsp.py ghidra`. The native wrapper has no local EH frame or compiler
catch. It propagates child/provider failures without adding cleanup.

Native inputs are ECX actual10h vector, EDX unused and stacked
output/owner/position/value; RET10h, EAX output. Source retains those physical
words and adds an actual `NativeStringRawPoolContext` after them. Its extra
stack word changes cleanup and does not promise original binary ABI or
private-stack alias identity. The wrapper calls `0044F410` at `004501D0` with
EDX=value-header address, loaded by `004501C1`; the Source child receives those
same bits as its unconsumed EDX parameter, not the entry's ignored EDX.

The exact capture/read schedule is retained:

1. Capture original begin at +4. Null begin skips the preliminary end read.
   Otherwise capture end once and calculate SAR3(end-begin) with DWORD
   subtraction. If that size is zero, offset0 is used without owner validation
   or position subtraction; this includes nonzero byte differences below8.
2. Only the nonnull/nonzero-size branch compares captured begin/end, invokes
   the invalid-parameter service if reversed, then checks the captured owner
   for nonnull identity with the header. Returning handlers continue. The
   offset is SAR3(position-original begin), preserving that original begin
   even if a handler changes the vector. No pre-insertion position bounds or
   alignment check is introduced.
3. Forward owner/position/count1/value to the accepted actual `0044F410`.
   Its temporary value construction, actual pool allocation, storage alias
   behavior and current C++ exception cleanup are inherited. The wrapper
   neither snapshots the string nor creates a host vector or allocator.
4. After child return, capture new begin and compare it with CURRENT end.
   A returning invalid handler does not replace the captured begin. Calculate
   the result from that begin plus offset*8 using wrapped DWORD arithmetic.
5. Compare result with a fresh end; only if result<=end, read a fresh begin
   and compare the lower bound. Either invalid result invokes one provider
   call, whose normal return still permits output publication. No header
   reload/revalidation is inserted after that provider returns.
6. Write output+4 position, then output+0 header owner, and return the original
   output pointer. Other output fields are untouched. Output may alias header
   fields, the value header or other supplied storage: no overlap ban or
   temporary iterator copy changes those two ordered stores.

The Source reads/writes use actual DWORD field accesses with the required
volatile ordering. Pure address/SAR helpers add no field loads, iterator model
or validation. The private native stack write at `004501E2` is not exported
as a Source memory side effect; aliases into compiler-private argument/spill
slots are outside the new by-value Source interface.

Returning diagnostics are intentional. The accepted
`invoke_native_invalid_parameter_00bf6713` uses the current SDK CRT policy,
has no `noreturn` or `noexcept` promise, and supplies no invented handler,
exception conversion or fallback allocator. Handler mutations remain visible
only at the field reads the listing actually performs. Source exceptions
escape immediately, preserving earlier child mutations and skipping later
checks/stores; there is no wrapper rollback or catch.

The sole Ghidra xref is append `004505B6`. Its physical call supplies a
private output iterator plus owner/header, current end and value. This is
interface evidence; it does not promote the old append projection to a new
complete actual-storage implementation.

Exact checked CALL instructions:

```text
004501A6 CALL 00BF6713
004501B3 CALL 00BF6713
004501D0 CALL 0044F410
004501DD CALL 00BF6713
004501F3 CALL 00BF6713
004505B6 CALL 00450180
```

Win32 Release and all three existing suites pass; CTest took7.79 seconds.
All six report rows and six documentation call claims pass live verification,
with no failures, unchecked claims or warnings. Results are recorded in
`reports/oct15_native_string_vector_insert_one.json`.
Existing native math seeds do not exercise this wrapper. No new test or
native differential/handler-injection fixture is supplied. Source/build proof
does not establish original stack/CRT/exception/FH3/SEH identity, fault
addresses, original-game execution or gameplay behavior.
