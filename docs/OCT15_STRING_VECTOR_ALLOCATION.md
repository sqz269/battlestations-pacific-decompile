# Native checked string-vector allocation

Packet `oct15_string_vector_allocation` covers `00432090`, `0042BF40`, and
`00431F70`. All three complete bodies are reconstructed at the established
current-CRT and owning Source-exception boundaries. Those boundaries do not
reproduce the installed game's static CRT, original RTTI/catch identity, or
FH3 binary exception ABI. Existing library-role names are retained, including
`STL_NativeStringVector_Buy_00432090`, `STL_allocator_allocate_0042bf40`, and
`STL_xlen_throw_00431f70`; they are descriptive hypotheses, not recovered symbols.

| Address | Inclusive body | Original machine contract | Source entry |
| --- | --- | --- | --- |
| `00432090` | `00432090..004320D6` | ECX actual header, stacked unsigned count, unused EDX; EAX result; RET4 | `buy_native_checked_string_vector_00432090` |
| `0042BF40` | `0042BF40..0042BF94` | ECX unsigned count; no consumed EDX/stack input; EAX allocation; RET | `allocate_native_string_vector_0042bf40` |
| `00431F70` | `00431F70..00431FD8` | no input; no normal return | `throw_native_string_vector_length_error_00431f70` |

The fastcall declarations reserve unused EDX explicitly. Buy exposes full EAX,
since the original writes only AL on positive success. No source routine in
this packet is claimed as a drop-in binary replacement. Live Ghidra queries
verified project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language and image base before this analysis.
Complete pseudocode followed by assembly was read for these three routines and
accepted `00BD0590` / `00BCFEB0`. All 28/25/23 listed instructions respectively
have no flow/listing gaps. Ghidra and the installed game were not modified.

Buy uses the producer/consumer-established **16-byte checked-vector header**:
opaque/proxy word at +0, begin at +4, end at +8, capacity end at +C. This is not
the different `NativeStringVectorStorage` 0Ch data/count/capacity layout. The
existing checked assignment/erase services use these same raw offsets and
explicit native context. This entry introduces no overlay, proxy constructor,
element construction or ownership convenience wrapper.

The original captures count before clearing +4, +8, +C in that order, leaving
+0 untouched. Count zero still performs these three stores, then returns full
EAX zero without allocating. A count above unsigned `1FFFFFFFh` throws only
after the stores. Positive admitted count allocates count*8 bytes, then publishes
begin, end=begin, capacity end in that order. It does not destroy old elements,
free an old buffer, or restore the old fields after a failure. Raw DWORD stores
retain the native publication schedule without evaluating pointer preimages.
The actual header must remain live and writable through real new-handler and
exception activity; aliases into private implementation frames/spills are not
admitted. No malformed pointer/heap storage is made safe by this API.

Capacity end uses unsigned 32-bit address addition, including native wrap,
rather than undefined out-of-array C++ pointer arithmetic. The success result
is `(capacity_end & FFFFFF00h) | 1`, retaining the upper 24 native EAX bits.
Callers testing AL receive the original success result. `004320BF` explicitly
zeroes EDX before the allocator call; the Source call supplies null for that
ignored register word. The original count is not reloaded from the vector.

Standalone allocator `0042BF40` uses unsigned `TEST/JA`, then for positive count
checks `FFFFFFFFh / count >= 8` with DIV and unsigned JNC. This is exactly the
admitted range `count <= 1FFFFFFFh`; the maximum still requests `FFFFFFF8h`
bytes. Zero takes the allocation path with zero bytes. Source retains both
facts and adds no host-size guard. It reuses actual `singleton_lifetime_allocate`
with equal native/host byte counts and the established object byte-domain:
malloc, real current `_callnewh`, retry, then Source `std::bad_alloc`.
The overflow path uses the same qualified Source `std::bad_alloc` boundary as
accepted pointer allocator `00BCFEB0`, whose corresponding stride is 4 and
limit is `3FFFFFFFh`. Native overflow instead constructs the base through
`00BF6340` with a null message slot, writes native vtable `00D6923C`, and calls
`00BF6885` with ThrowInfo `00E03CC0`. Those original payload/RTTI/handler and
static-CRT identities are not claimed for the Source exception. No invented
allocator callback or synthetic native throw metadata is introduced. Successful
storage belongs to the same current-CRT domain as `singleton_lifetime_free`.

The length throw is different: it directly reuses the accepted owning
`native_singleton_length_error_00bd0590`, after comparing both **complete**
native bodies and raw EH evidence. Each builds the actual no-init legacy SBO
temporary with capacity15, length0 and initial NUL, then assigns exactly18
bytes `vector<T> too long` from `00CE37E0` through `00408720`. This includes the
actual SBO growth/allocation side effects. State0 is armed only after counted
assignment succeeds. `00411700` constructs the actual 28h legacy logic-error
payload; its first word is replaced with `00D69260`, and both originals throw
with identical ThrowInfo `00D83F98` through `00BF6885`. There is no generic
`std::length_error` substitute, repeated SBO initialization or extra cleanup.

Raw original disk evidence independently confirms the match. Both bodies are
105 bytes; all89 nonrelocation bytes match. The only masked bytes are the
handler immediate (offset03..06) and three CALL displacements (39..3C,
4E..51,65..68). Each CALL independently resolves to the same target, respectively
`00408720`, `00411700`, `00BF6885`. New handler `00C5EC38` loads FuncInfo
`00D8533C`; accepted handler `00CC5478` loads `00DFF4BC`. Both jump to
`00BF6B43`. Each FuncInfo has magic `19930522`, maxstate1 and an unwind map
(`00D85334` / `00DFF4B4`) taking state0 to -1. Actions `00C5EC30` / `00CC5470`
are each `LEA ECX,[EBP-50h]; JMP004072D0`, destroying the completed SBO
temporary on unwind. Raw bytes and full extraction/mask provenance are in the
JSON report; ignored audit script/result are under `local/`.

The owning Source transport is `NativeSingletonVectorLengthError`, including
the actual legacy payload, copy service `00411940`, and destructor `00411780`.
The legacy vtable address is an identity DWORD, not a callable host vtable.
Source RTTI/catch type and compiler EH replace original FH3, ThrowInfo and
native `std::length_error` catch identity. This is the same explicit boundary
as the reused accepted service. These three native bodies contain no x87 or
floating arithmetic; NaN behavior is not involved.

`./scripts/build.ps1` passed the exact source and all three existing suites:
reconstructed_math, native_math_differential, tool_tests. No new tests were
added; those suites are regression/seed evidence, not packet-specific native
differential coverage. Actual Release COFF was inspected: Buy captures count
before the three ordered zero stores, preserves ordered allocation publication,
returns full EAX with AL1/RET4, and the throw wrapper is a single tail JMP to
the accepted service. Standalone allocator sends zero to the actual allocation
service and retains unsigned max comparison. This is static compiled-source
inspection, not byte identity with the original or original exception execution.
Call-report verification and whitespace checks are recorded in the JSON report.
No native fixture, original ABI compatibility, game startup or gameplay result
is claimed.
