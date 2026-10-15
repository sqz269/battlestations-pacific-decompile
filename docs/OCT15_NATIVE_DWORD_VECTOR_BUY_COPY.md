# Native DWORD-vector Buy and forward copy

This packet provides complete raw-storage Source for `00557200` and
`00697080`. It retains the established library identities
`TMPL_Vector_ConstructN_00557200` and `STL_CopyDwords_00697080`; these are
descriptive hypotheses, not recovered symbols. The new interfaces use the
actual accepted allocation and length-error services. No generic container,
callback allocator, pointer-count adapter or typed-array bridge is involved.

The corresponding implementation is
`src/native_dword_vector_buy_copy.cpp`, with declarations in
`include/bsp/native_dword_vector_buy_copy.hpp`. The JSON report contains full
owned bytes, every native instruction start, bounded caller bytes and all six
verified transfers. These two bodies do not complete `00697BD0` assignment.

## Buy: 00557200

The complete current body is `00557200..00557246`: 71 bytes and 28 instruction
starts. Native ECX is the receiver, incoming EDX is unconsumed, and the one
stack word is an unsigned element count. The body ends with `RET4`. The receiver
is the actual 16-byte storage header: +4 is begin, +8 is end, and +C is capacity
end. The opaque word at +0 is not written.

The body captures the count before writing zero to +4, +8, then +C. A zero
count returns full EAX zero without allocation. An unsigned count above
`3FFFFFFF` enters the accepted length-error service. Otherwise the same count
is passed in ECX to the accepted allocator with EDX zero. There is no element
width conversion at that call boundary.

Normal allocation return is published to begin, then end. Capacity end is
computed and stored as the wrapped DWORD sum `begin + count*4`. Native
`MOV AL,1` retains the upper 24 bits of that computed capacity end, so the new
Source returns `(capacity_end & FFFFFF00) | 1`. Its `uint32_t` return exposes
those register bits; it does not establish the original C++ return type.
The containing assignment caller tests AL only.

Buy does not release a previous allocation, retain old field values, construct
elements or install a rollback. Length-error and allocation failure occur
after the three clears. The accepted allocator's current new-handler may
reenter or modify memory: its side effects are not rolled back, and a normal
return still performs the three publication writes in native order. Source
does not add an exception guard or a second cleanup around the length helper.

The direct bindings are
`allocate_native_dword_vector_00553260(count,nullptr)` and
`throw_native_dword_vector_length_error_00556c20()`. They are complete accepted
Source in `native_dword_vector_allocation.hpp/.cpp`. Their raw allocation
domain is the accepted current `malloc`/`_callnewh`/free domain. Their owning
legacy length-error payload and completed-message cleanup are already handled
by the accepted full helper.

## Forward copy: 00697080

The complete current body is `00697080..006970AE`: 47 bytes and 21 instruction
starts. Native cdecl takes three stacked words `(first,last,destination)` and
uses plain `RET`; callers remove `0Ch`. It captures last, first and destination
even for an empty range, then precomputes the returned DWORD pointer bits as
`destination + SAR2(wrapped(last-first))*4` before touching data. Arithmetic
right shift is reproduced explicitly; neither typed-pointer subtraction nor
an element-count conversion replaces it.

For a nonempty range it captures the displacement `destination-first`. Each
iteration reads the current source DWORD, stores it through
`displacement + current_source`, then advances source by four with DWORD
wrapping until it equals the captured last. The loop returns the independently
precomputed pointer, rather than deriving its result from a final cursor.

Reads are progressive. A right-overlapping destination can therefore change a
later source DWORD. No temporary snapshot or memmove is introduced. Unlike
`00696D40`, this body does not skip accesses when the destination is null, so
that accepted helper is not a valid whole-body delegate. The Source uses
explicit ordered four-byte machine loads and stores for this body. It does
not add null, alignment, distance or overlap validation.

## Caller evidence and transfer accounting

The following sites are fresh current instruction starts with verified owning
bodies, default/effective targets, no flow override, and matching disk/live
REL32 bytes. The four `00697BD0` rows are bounded producer evidence, not a
whole-parent Source or ownership claim.

| Native transfer | Evidence role |
| --- | --- |
| `00557226 CALL 00556C20` | Length error after all three field clears |
| `0055722F CALL 00553260` | Same unsigned count; EDX zero |
| `00697C1D CALL 00697080` | Shortening copy; caller removes 0Ch |
| `00697C86 CALL 00697080` | Existing-capacity prefix; caller removes 0Ch |
| `00697CAF CALL 00BF65AC` | External growth frees old backing before Buy |
| `00697CCD CALL 00557200` | External growth recomputes count and tests AL |

The producer spans are `00697C1A..00697C27` (14 bytes, six starts),
`00697C7E..00697C93` (22 bytes, nine starts), and `00697CAA..00697CD5`
(44 bytes, 18 starts). The growth span proves that the parent frees old begin
before it reloads/recomputes source count and calls Buy. The parent, rather
than Buy, owns that free. These spans include caller argument setup, relevant
reloads, stack cleanup and the AL test. They do not cover the whole parent.

Owned coverage is 118 bytes, 49 starts and two CALLs. Bounded producers add
80 bytes, 33 starts and four CALLs. The union is 198 bytes, 82 starts and six
CALLs. Every selected byte equals the original PE and current Ghidra memory;
all selected instruction starts were checked against current typed metadata
at modification 1711. Neither owned body has a local native EH frame, catch,
cleanup action or external tail JMP. No body or flow repair was required.

## Existing Source and verification boundary

The existing `bsp::cc13_fe3::word_vector_construct_n_00557200` in
`frontend_seg12_17_cc13.cpp` and its `Fe3TemplateHost` interface remain intact.
That Source uses the previously qualified callback services and a bool result.
The existing `bsp::cc13_seg29::copy_dwords_00697080` in
`settings_seg29_cc13.cpp` remains intact in its well-formed typed-array domain.
Its Source, fixtures and reconstruction records are preserved. Historical
assignment/storage fragments are also unchanged. This packet adds distinct
actual-service/raw-DWORD Source records rather than replacing their provenance.

The MSVC Win32 build and all three existing suites passed in 7.67 seconds.
The report and this document each pass all six transfer claims. No new test
cases or native execution were added. The existing suites cover established
math, seed differentials and tools; they do not execute these new routines in
the original process.

A focused Release COFF check passed seven checks: Buy is 165 bytes/64
instructions with physical RET4, the same-count allocator call, three clears
before the branches, ordered publication and the upper24-preserving EAX
result. Copy is 100 bytes/43 instructions with plain RET, SAR2 before data
access, progressive load/store/increment order and the independent return.
These counts describe the newly compiled Source, separately from the original
118-byte/49-start bodies. They establish Source layout/order, not native binary
or private-frame identity. The frozen Source/header hashes accompany the
primary's eight-check precommit review in the JSON report.

The new interfaces do not reproduce arbitrary aliases into original private
argument/frame slots, ambient registers, fault ordering under invalid machine
addresses, native SEH/FH3 transport, original static-CRT heap/new-handler state,
RTTI/exception identity, or original binary ABI. The accepted Current C++/CRT
services define allocation and exception behavior. Compilation, current-body
and byte proof do not establish installed or gameplay behavior.

`00697BD0` remains outside this packet. Its actual erase/clear children were
accepted separately; whole assignment Source still awaits this packet's final
acceptance and separate authorization. `00697DC0` and `00697F40` belong to the
separate insertion/append packet. No claim of those entries is made here.
