# Actual outer string-vector copy constructor

Packet `oct15_outer_string_vector_copy_construct` covers the complete ordinary
`0055B150` body and separate `0055B216` cleanup catch. Its Source entry is
`construct_native_outer_string_vector_copy_0055b150`. Earlier settings-storage
projections and their fixtures remain separate.

The target was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little endian, image base `00400000`.
All 71 ordinary instructions and five catch instructions were inspected.
The complete 215-byte span matches the installed executable and a fresh saved
Ghidra byte response; all 76 disk instruction starts match the saved listings.
There are no listing gaps or analysis repairs.

The native entry has ECX destination, unused EDX, one stacked source pointer,
RET4 and EAX destination. The Source fastcall interface explicitly reserves
unused EDX and adds stacked pool context, changing cleanup to RET8. Both outer
and inner checked-vector headers are 16 bytes: opaque +0, begin +4, end +8,
capacity +C. Actual pooled-string elements inside the inner vector are eight
bytes. The opaque destination word is never initialized or overwritten here.

The constructor captures source begin and wrapped SAR4 element count before
zeroing destination +4, +8 and +C in that order. Zero count returns immediately,
with no allocation, iterator diagnostics or element access. Nonzero count is
checked unsigned against `0FFFFFFF`, then the actual 16-byte allocator supplies
backing. Destination begin, end and capacity are published in that order.

Next it captures CURRENT source end and compares CURRENT source begin with
that captured end. Only after those reads and comparison is state0 armed. A
reversed range is reported through the accepted returning invalid-parameter
service. It then captures CURRENT source begin, compares it against freshly
loaded source end and reports again if reversed. A returning report does not
replace either captured endpoint. CURRENT destination begin is loaded after
the reports. The actual nested-copy helper receives those captured endpoints,
destination begin, destination owner and two dead tags formed from the captured
first address with its low byte cleared. Returned end is published only after
successful completion.

There is no self-copy guard. A nonempty source identical to destination yields
the native clear/allocate/empty-copy sequence and loses ownership of the old
backing. A snapshot, self-assignment shortcut or rollback would change behavior.

Raw FuncInfo `00D985E0` has magic19930522, two state entries at `00D985BC`,
both with state-1/action0, and one try record at `00D985CC`: low0, high0,
catchHigh1. The catch-all handler record `00D985AC` has attributes40, no type or
displacement, and entry `0055B216`; EHFlags is1. The catch reads its saved
destination and tidies its CURRENT fields, then rethrows. Length/allocation
failure occurs before this scope is armed. The nested child handles its own
completed prefix; the parent end remains its initial allocation cursor until
the child returns. No double prefix cleanup or outer free is invented.

The complete conditional-placement caller was inspected as incoming interface
evidence. The five other live caller xrefs were inventoried without promoting
their behavior or completion status.

```text
0055B1A3 CALL 00556C90
0055B1AC CALL 00553300
0055B1CE CALL 00BF6713
0055B1DE CALL 00BF6713
0055B1F9 CALL 00559E00
0055B219 CALL 0055AFF0
0055B222 CALL 00BF6885
006A683C CALL 0055B150
```

The Source composes accepted actual allocation, owning exception, nested copy,
cleanup and invalid-parameter services. Taken raw storage, ownership, current
pool publications and CRT contracts must be admitted. Private-frame aliases
are excluded. Its current C++ catch/rethrow does not reproduce original
FH3/SEH/RTTI, cleanup-failure unwinding, fault or register identity. No original
function execution, binary replacement or live game behavior is claimed.
Validation is recorded in `reports/oct15_outer_string_vector_copy_construct.json`;
no new tracked tests are introduced.
