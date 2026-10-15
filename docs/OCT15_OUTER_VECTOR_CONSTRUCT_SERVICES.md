# Actual outer-vector construction services

`native_outer_vector_construct_services.cpp` provides complete admitted
Source effects for `00557250`, `006A3350` and `006A4C00`, with catch
`006A33AA` and proven no-op placement action `00C7F290`. Existing frontend
`Fe3InputHost` reconstruction at `00557250` remains a separate projection.
No old Source/header or earlier packet was changed. Retained library names
and new descriptive names are hypotheses, not recovered symbols.

## Zero, limit, allocation and publication

`00557250..00557298` is73 bytes/29 instructions, without local EH. Native
ECX is the actual16B header, EDX is unconsumed and the single stacked word
is unsigned count; both returns are `RET4`. Source reserves ignored EDX and
returns the complete defined DWORD EAX. The sole native caller confirms
its low byte is meaningful: `006A679E CALL 00557250` is followed by
`TEST AL,AL` at `006A67A3`.

Count is captured before the header stores. The helper zeros +4, +8, +C
in that order, leaves opaque +0 untouched, then tests count. Zero returns
EAX0 with no allocator call. It does not destroy/free any old backing or
initialize a proxy. These ownership-loss effects are not repaired.

The unsigned comparison admits at most0FFFFFFF. `00557276 CALL 00556C90`
binds to the accepted actual owning length-error transport, preserving the
real legacy28h payload,18-byte message, actual SBO/logic-error construction
and cleanup domain. No generic `std::length_error` is substituted.
`0055727F CALL 00553300` binds to the accepted actual16B allocator, with
count in ECX and EDX0. The allocator's current CRT/heap/new-handler and
Source bad-allocation qualifications remain explicit. Although that child
can allocate zero bytes, this helper's zero path skips it entirely.

On success, compute DWORD `storage + (count <<4)`, publish capacity +C,
then begin +4, then end +8. Only afterward native `MOV AL,1` changes the
return: EAX is `(storage_bits & FFFFFF00h) |1`, not an all-bits boolean1.
Source retains this result and ordered raw x86 stores, without undefined
C++ pointer arithmetic or an invented allocation callback.

## Exact existing range-copy reuse

The new core has normal ranges `006A3350..006A33A9` and
`006A33CF..006A33E3` (111 bytes/42 instructions), and a separate catch
`006A33AA..006A33CE` (37 bytes/15 instructions). Its whole148-byte envelope
matches the accepted `00559E00` envelope after masking **only** the verified
own EH-handler immediate and three independently verified rel32 transfer
displacements. Every branch, field-free cursor instruction, stack operation
and return byte otherwise matches. Complete raw EH tables and actions were
separately compared, rather than inferred from the masked bytes.

Native inputs are ECX first, EDX captured last and four stacked words:
destination, owner metadata, dead tag3 and dead tag4. It returns destination
end in EAX with `RET10h`. Source retains those words and adds actual raw-pool
context, giving `RET14h`. The complete accepted
`copy_native_nested_string_vector_uninitialized_range_00559e00` is directly
reused; it is not a callback, borrowed assignment, raw8B copy or host vector.

`006A3397 CALL 00557BC0` uses the current source address and current nonnull
destination to construct an actual16B inner owning checked header. Each
header has opaque +0, owning begin +4, end +8 and capacity +C; its elements
are actual8B owning pooled-string records. Source endpoint values are
captured, contents are read by the actual constructor on each iteration.
Forward overlap and actual child self-alias effects remain unchanged.

Placement state1 is armed before the destination null branch. Null skips
only construction. After success or skip: advance destination by16, return
to state0, publish completed destination, then advance source by16 and test
captured last. All arithmetic uses DWORD wrap and equality. There is no
range/self/null repair, contents snapshot or extra outer allocation free.
The child preserves opaque +0 according to its established actual contract.

## Prefix catch and placement action

`006A33BA CALL 00559DC0` tidies the captured completed prefix, without a null
skip. The accepted core composes actual `00432440` after the previously
verified complete cleanup-schedule equivalence: captured begin/end,
actual string-range destruction, CURRENT begin free, unconditional ordered
+4/+8/+C clears, +0 preserved. No replacement callback or unaccepted service
is introduced. The failing current child is excluded; prior completed copies
are tidied in order. A skipped null slot still belongs to that prefix, so
safe cleanup requires every accessed prefix header to meet actual Tidy
preconditions. Failure during cleanup stops later work.

`006A33CA CALL 00BF6885` receives two zeros and does not return. Accepted
Source uses its real C++ prefix catch and `throw;`; native FH3/SEH/RTTI,
fault and nested-cleanup-failure identities are not asserted.

The native handler's `00C7F2A6 JMP 00BF6B43` follows loading FuncInfoDAC4CC.
Its full tuple is `(19930522,3,DAC4B4,1,DAC4A0,0,0,0,1)`. Unwind states are
`0->(-1,0),1->(0,C7F290),2->(-1,0)`. Try record
`(0,1,2,1,DAC490)` names catch-all record `(40,0,0,6A33AA)`.
These match acceptedD982B8/D982A0/D9828C/D9827C structurally with the actual
address bindings retained.

The complete17-byte/seven-instruction action at `00C7F290..00C7F2A0` reads
parent words at EBP-1C/-20, pushes them, performs
`00C7F298 CALL 00401130`, then `ADD ESP,8; RET`. The actual target is one
byte, `C3`/`RET`. It matches acceptedC6DAC0 after only the verified rel32
field is masked. Eliding this action adds no destructor/free/stub.
Catch and action are parent-bound Source scopes delegated to the accepted
complete core, not standalone native ABI entries.

## Dispatcher and validation

`006A4C00..006A4C24` is37 bytes/14 instructions. ECX is owner metadata,
entry EDX is unused, and its **three** physical stack words are first, last,
destination; native cleanup is `RET0Ch`. `006A4C1C CALL 006A3350` forwards
owner, destination bits and owner with only low byte cleared as the three
dead metadata words. Its EAX child result survives. Native
`006A67B5 CALL 006A4C00` immediately publishes EAX as destination end at
`006A67BA`. Source adds context, hence `RET10h`.

Accepted0055A950 has **five** physical stack words and `RET14h`, with two
additional ignored owner words. All four changed stack-load displacements,
the changed return immediate and the verified child rel32 are recorded in
the report. This packet supplies the actual three-word dispatcher instead
of pretending those original ABIs are interchangeable.

The existing `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` were
verified before analysis/export. Pseudocode preceded full assembly; typed
metadata confirms all normal/catch boundaries. Fourteen complete live-byte
spans equal the installed PE32 image without masks. Capstone5.0.7 matches
all178 current listing starts across eight owned/comparison scopes, and16
CALL/JMP targets were independently checked. Comparison masks apply only to
the explicitly proven address/ABI differences described above. No x87 or
other floating-point arithmetic occurs in the three helpers or local scopes.

Exact registered Source passed `./scripts/build.ps1` and the three existing
suites in7.68s. JSON10/10 and Markdown10/10 call/jump checks pass, with zero
unchecked claims or warnings; staged diff checks pass. No new tests were
added. Generated COFF confirms clear order,
unsigned limit, actual allocator/owning throw, capacity/begin/end publication,
exact DWORD result, complete-core tail reuse and dispatcher tags/return.
The report records native spans, commands, typed capture, masks and object
hashes. Added context, current CRT/Source owning exception transport and C++
catch/private-frame differences remain explicit; no original ABI/FH3/SEH,
fault, native fixture execution, binary replacement or gameplay claim.

The short `oct15_outer_vector_construct_cmake_merge` lease was queued and
claimed at its first/unleased turn. The standard append-union merge of
published main was committed separately as `9f1e374fb`, then only this Source
registration was appended. The lease was released before the build; root's
main promotion was next. The five-address lease remains held for review.
