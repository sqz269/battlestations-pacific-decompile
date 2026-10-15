# Native nested checked string-vector allocation

Packet `oct15_nested_string_vector_allocation` covers `00553300` and `00556C90`.
Both complete bodies are represented at the accepted actual allocation and
owning Source-exception boundaries. The allocator retains the established
**current-CRT bad_alloc boundary**, which does not reproduce the installed
native bad_alloc payload, catch/RTTI identity, heap or new-handler binding.
Length failure uses the accepted actual legacy-payload owning transport, with
its distinct Source C++ catch/EH identity. No generic `std::length_error`, stub,
allocator callback or new allocation model is introduced.

| Entry | Inclusive body | Original machine interface | Source |
| --- | --- | --- | --- |
| `00553300` | `00553300..00553350`, 81 bytes / 25 instructions | ECX unsigned count, EDX unconsumed, no stack args, plain RET, EAX allocation | `allocate_native_nested_string_vector_00553300` |
| `00556C90` | `00556C90..00556CF8`, 105 bytes / 23 instructions | no consumed input, no normal return | `throw_native_nested_string_vector_length_error_00556c90` |

Existing names `STL_allocator_allocate_00553300` and
`STL_xlen_throw_00556c90` are retained as descriptive library-role names, not
recovered source symbols. Complete pseudocode was followed by both full
listings and raw length-error EH data. Live queries verified project `bsp`,
configured at `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
language and image base before analysis. Both listings have no gaps, the
allocation path reaches plain RET and exception calls have no normal return.
No control-flow repair, extra catch reconstruction or Ghidra mutation was
needed. Full accepted `0042BF40` / `00431F70` listings, Source and boundaries
were already established in this worker stream; original disk bytes independently
compare their relevant complete paths below.

For nonzero count, allocator `00553300` computes unsigned
`FFFFFFFFh / count`, compares it to10h and uses JNC to admit values at least16.
This is exactly `count <= 0FFFFFFFh`. The native TEST/JA branch is unsigned;
high-bit counts are not signed-negative success cases. Zero explicitly sets
ECX0 and still enters the actual allocation path. SHL ECX,4 produces count*16;
maximum admitted request is `FFFFFFF0h`. Source checks the unsigned threshold
before multiplication, adds no host-size cap and preserves zero allocation.
This is backing for actual16-byte nested checked-vector headers; it performs
no element, proxy or header construction and has no header overlay.

The Source entry reserves ignored EDX and returns the complete allocation
pointer in EAX. Pseudocode's void return is not retained. Native outer-copy
caller `0055B150` uses SAR4/SHL4 for its 16-byte range. The allocation instruction
is `0055B1AC CALL 00553300`, and returned EAX is published at `0055B1B6` and
`0055B1B9`. The length-failure instruction is `0055B1A3 CALL 00556C90`, after
comparing count to `0FFFFFFFh`. Full outer
constructor reconstruction is outside this packet.

Actual `singleton_lifetime_allocate` is reused with object byte-domain and
equal native/host byte counts: current malloc, real current `_callnewh`, retry,
and Source `std::bad_alloc`. It does not cache or fabricate a handler and
propagates current new-handler exceptions. Storage belongs to the matching
`singleton_lifetime_free` domain. Allocator overflow uses the same qualified
Source `std::bad_alloc` boundary as accepted8-byte allocator `0042BF40`; the
latter's positive range is `1FFFFFFFh`. Native overflow instead passes an actual
null message slot through `00553335 CALL 00BF6340`, writes `00D6923C`, and
throws through `0055334C CALL 00BF6885` with ThrowInfo `00E03CC0`.
No Source identity claim is made for that native payload,
static CRT or FH3. Original overflow tails `00553325..00553350` and
`0042BF69..0042BF94` are both44 bytes; all36 nonrelocation bytes match, with
only two relative branch displacements masked. Both targets independently resolve to the
same `00BF6340` and `00BF6885` targets; null slot, vtable and ThrowInfo bytes
are not masked.

Length throw `00556C90` directly reuses accepted
`throw_native_string_vector_length_error_00431f70`, which already binds
`native_singleton_length_error_00bd0590`. Each complete original length body
is105 bytes; all89 nonrelocation bytes match. Masks are limited to EH handler
immediate offsets03..06 and CALL displacement offsets39..3C,4E..51,65..68.
The three instructions are `00556CC8 CALL 00408720`, `00556CDD CALL 00411700`,
and `00556CF4 CALL 00BF6885`. Message pointer `00CE37E0`, exact18-byte `vector<T> too long`
message, native payload vtable `00D69260`, ThrowInfo `00D83F98`, initialization
and state-arm schedule are unchanged and unmasked.

The actual no-init legacy SBO temporary starts capacity15, length0 and NUL;
counted18-byte assignment includes actual growth/allocation side effects.
State0 is armed only after that assignment succeeds. Actual `00411700` builds
the28h legacy logic-error payload, whose vtable address word is replaced before
throw. The accepted Source owning class is `NativeSingletonVectorLengthError`,
including actual copy `00411940` and destruction `00411780`. No whole-payload
zeroing, repeated SBO initialization, generic length_error or extra cleanup is
added. The vtable address is an identity DWORD, not a callable host vtable.

Raw handler `00C6D7B8` loads FuncInfo `00D97B84` and jumps to `00BF6B43`.
FuncInfo has magic `19930522`, one state, map `00D97B7C`, no try/catch/IP/ESType
lists and flags1. State0 goes to -1 via action `00C6D7B0`, exactly
`LEA ECX,[EBP-50h]; JMP004072D0`, destroying the completed temporary. Accepted
`00431F70` has the same fields and schedule with handler `00C5EC38`, FuncInfo
`00D8533C`, map `00D85334`, action `00C5EC30`. The complete raw36-byte FuncInfo
tuples and all map/action/handler bytes were independently asserted from the
installed PE. This action is evidence for service equivalence, not a new
standalone Source action or added mutation scope. Source compiler EH, owning
catch type/RTTI and original cleanup-failure identity remain distinguished.

Actual Release COFF confirms unsigned JA against `0FFFFFFFh`, SHL4 and an
unconditional current allocation call on the admitted path, including zero,
followed by pointer EAX/plain RET. Overflow uses the current Source bad_alloc
constructor/throw. The new length entry is one tail JMP to the accepted service.
This is compiled-source inspection and original-to-original byte comparison,
not original-versus-Source byte identity or native exception execution. These
native bodies contain no x87/FP arithmetic; NaNs are not involved. Private
native/Source frame aliases, original CRT/FH3/SEH/catch/RTTI and complete binary
replacement compatibility are outside the Source contract.

`./scripts/build.ps1` passed exact C++ Source and all three existing suites:
reconstructed_math, native_math_differential, tool_tests. No new tests were
added; existing suites are regression/seed evidence, not packet native
differential fixtures. Exact call/caller verification, extraction provenance
and whitespace checks are recorded in the JSON report. No game startup or
gameplay validation is claimed.
