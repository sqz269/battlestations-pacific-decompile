# Actual outer string-vector cleanup wrappers

This packet supplies complete Source compositions for range dispatcher
`0055AC60..0055AC75` and second outer tidy `0055B110..0055B14C`.
Source is `src/native_outer_string_vector_cleanup_wrappers.cpp`; declarations
are in `include/bsp/native_outer_string_vector_cleanup_wrappers.hpp`.
The accepted actual `0055A3A0`/`0055AFF0` cleanup services are reused directly,
with the same concrete raw pool/current CRT boundary. Descriptive Source
names remain hypotheses, and the older host projections remain separate.

The saved analysis is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete cached pseudocode/listings and live
22/61-byte entry envelopes, prototypes, xrefs and incoming instruction
contexts were checked through `bsp.py ghidra`. Neither wrapper creates a
local EH frame, catch, validation guard or allocation model.

Native `0055AC60` has ECX receiver, unconsumed incoming EDX and two stacked
words first/last, RET8. It loads last twice into EAX/EDX, pushes last then
receiver as the range helper's two dead stack words, loads first into ECX
and forwards to `0055A3A0`. Source retains all physical input words and adds
stacked context. The receiver is forwarded without dereference, ownership
validation or invented allocator semantics. The endpoints and metadata are
passed unchanged to the accepted range service. Their Source by-value form
does not reproduce private caller-stack mutation between native loads.

Native `0055B110` has ECX actual16B outer header, unconsumed incoming EDX,
no stacked input, plain RET. Its full21-instruction body matches accepted
`0055AFF0`: captured begin/end range destruction, CURRENT begin free,
unconditional +4/+8/+C clears and no write to +0. Null begin skips end read,
range and free but still clears. The new Source body delegates to that
complete actual service and adds context in EDX.

The full61-byte comparison masks only the two CALL rel32 fields at
offsets15h..18h and1Eh..21h. All53 remaining bytes match.
`0055B124 CALL 0055A3A0` and `0055B12D CALL 00BF65AC` decode to the same
targets as accepted `0055B004 CALL 0055A3A0` and `0055B00D CALL 00BF65AC`.
This is complete
instruction/field/callee equivalence, not similarity inferred from names.
The child uses the accepted actual raw string pool and current `std::free`;
no new callback, host container or replacement stub is introduced.

Both native entries have no established semantic EAX result. The dispatcher
leaves the range child's residue; on an empty range that is the initially
loaded last value. The Source interfaces are void and make no register-residue
promise. Added context changes stack/register usage, so the new signatures
are not original native ABI replacements.

Source inherits exact child propagation: a failing inner cleanup stops later
elements, backing free and later clears. There is no extra catch, retry,
rollback, endpoint reload or validation. Endpoints use the accepted DWORD
wrap/equality schedule, and equal endpoints perform no element work. Raw
header/backing aliases and provider effects are neither banned nor restored.

Historical `string_vector_range_destroy_wrap_0055ac60` and
`string_vector_vector_tidy` in `src/frontend_seg12_17_cc13.cpp` remain
unchanged. They use host providers; the latter's null-begin early return is
not the actual unconditional-clear behavior. Existing bounded settings
projections and their fixtures are also retained separately.

Three incoming range-dispatch calls were inspected. All six unwind references
to `0055B110` are actual tail JMP instructions, despite the generic callgraph
edge label. They derive ECX from parent EBP/member offsets. The report records
the JMP kind explicitly; no Source parent-frame adapter or caller completeness
is supplied by this packet.

Checked native transfers, eight CALLs and six tail JMPs:

```text
0055AC6E CALL 0055A3A0
0055B124 CALL 0055A3A0
0055B12D CALL 00BF65AC
0055B004 CALL 0055A3A0
0055B00D CALL 00BF65AC
006A5DD0 CALL 0055AC60
006A5E63 CALL 0055AC60
006A6777 CALL 0055AC60
00C6DC10 JMP 0055B110
00C6DC80 JMP 0055B110
00C6DD10 JMP 0055B110
00C7F533 JMP 0055B110
00C7F5C3 JMP 0055B110
00C7F5E3 JMP 0055B110
```

Win32 Release and all three existing suites pass; CTest took8.35 seconds.
All14 report transfers and14 documentation claims pass with no failures,
unchecked claims or warnings. Results are recorded in
`reports/oct15_outer_string_vector_cleanup_wrappers.json`. No new tests are
added; existing native math seeds do not exercise these wrappers. Current
Source pool/CRT/C++ behavior does not prove original heap, entry/stack/register,
private-frame, exception/FH3/SEH/fault or cleanup-failure unwind identity,
original-game execution or gameplay compatibility.
