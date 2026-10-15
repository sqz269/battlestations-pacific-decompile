# Native nested checked string-vector uninitialized copy

Packet `oct15_nested_string_vector_uninitialized_copy` covers helper
`00559E00`, separately owned catch `00559E5A`, and dispatcher `0055A950`.
The complete normal and completed-prefix cleanup effects use accepted actual
`construct_native_string_header_vector_copy_00557bc0` and
`tidy_native_string_header_vector_00432440`. There is no new vector, allocator,
copy callback or substitute ownership model. These are **16-byte checked-vector
headers**, not the 8-byte string-header range copied by `0044D0C0`.

| Entry | Exact native scope | Original machine contract |
| --- | --- | --- |
| `00559E00` | normal `00559E00..00559E59` and `00559E7F..00559E93`, 42 instructions | fastcall ECX first, EDX last; stack destination, dead_owner, dead_tag3, dead_tag4; RET10h; EAX completed destination end |
| `00559E5A` | catch `00559E5A..00559E7E`, 15 instructions | parent-frame catch, no independent callable API; completed-prefix Tidy then rethrow |
| `0055A950` | `0055A950..0055A974`, 14 instructions / 37 bytes | ECX dead_owner; five stack words dead_first_owner, first, dead_last_owner, last, destination; RET14h; EAX helper result |

The min/max helper span includes the separately identified catch: 148 bytes,
57 combined listing instructions. Both live identities and exact containing
functions were checked before assigning calls. Complete pseudocode was followed
by complete helper/dispatcher/catch listings, child constructor and Tidy
listings, plus the raw EH handler, maps and state1 action. Live queries verified
project `bsp` at configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language and image base before analysis/export.
There are no listing gaps or required control-flow/no-return repairs. Ghidra
and the installed game were not changed. Existing descriptive names
`TMPL_NativeStringVector_UninitCopy_00559e00`, `STL_TagDispatch_0055a950`, and
`Catch_All@00559e5a` are retained; they are not recovered source symbols.

Each destination slot has opaque/proxy+0, begin+4, end+8 and capacity+C. The
accepted actual constructor establishes +4/+8/+C using the source's actual
8-byte inner-string records and pool/backing ownership. It preserves the
destination +0 preimage; this wrapper does not initialize or copy that word.
It does not allocate or free the outer destination range. The caller supplies
actual admitted construction storage, source headers and the accepted actual
raw-pool context, including current-CRT backing ownership. No host header
overlay, whole-record memcpy or borrowed uninitialized assignment is used.

The helper captures first, last, initial destination and current completed end.
For each source cursor it arms native state1 around the current placement
construction, uses the actual constructor service only when destination is nonzero, then
advances destination by10h, resets state0, publishes completed end, and advances
source by10h. The loop compares equality against captured last. All address
arithmetic is native DWORD wrapping through unsigned Source words, without
out-of-array C++ pointer arithmetic. Empty returns the initial destination and
does no element work. The child return value is ignored; EAX is the advanced
destination cursor at ordinary completion. Native caller `0055B1F9` publishes
that return at `0055B1FE`, confirming the semantic result.

Null destination skips only its constructor. It still advances and belongs to
the completed prefix; for a single input element and destination0 the native
normal result is address10h with no header read or write. If a later child
fails, the catch tidies every prefix address **without a null skip**. A skipped
null slot is consequently not a safe cleanup object merely because the normal
path admitted the skip. Every executed cleanup access must be admitted. No
null/fault repair is added. Forward overlap and the child's lack of a self-copy
guard are also retained: a completed destination may change a later source,
and the child may overwrite/leak preexisting fields where native does so.
No class-safe copy or ownership normalization is promised for such storage.

Catch `00559E5A` reads initial destination from parent [EBP-18h] and completed
end from [EBP+8]. It tidies in ascending10h steps and then calls native
`00BF6885(0,0)` to rethrow. The failing current header is excluded; the actual
child constructor retains its own cleanup behavior. Catch cleanup does not
free the outer range. If one Tidy fails, later cleanup and the final original
rethrow are not reached. The new Source C++ catch/rethrow expresses this ordered
scope without claiming native cleanup-failure unwind identity.

The raw EH handler at `00C6DAD1` loads FuncInfo `00D982B8` then jumps to
`00BF6B43`. FuncInfo has magic `19930522`, three state entries, unwind map
`00D982A0`, one try map `00D9828C`, no IP/ESType lists, flags1. The try entry is
low0/high1/catchhigh2, with one HandlerType at `00D9827C`: attributes40h, type0,
catch-object displacement0, handler `00559E5A`. Unwind state0 is (-1,action0),
state1 is (0,action`00C6DAC0`), state2 is (-1,action0). The state1 action loads
the current destination from [EBP-1Ch]/[EBP-20h], passes both words to
`00401130`, removes8 stack bytes and returns. Actual `00401130` is **one RET**.
This proven no-op has no allocation/free effect; Source does not invent a
placement-deallocation service or stub for it. Private original/Source frames
and original FH3/SEH dispatch remain outside the new interface.

The catch's actual `00559DC0` takes its header from the stack and ignores the
owner in ECX. Both complete `00559DC0` and accepted `00432440` capture begin,
capture end, pass the header twice to actual `00432050`, free CURRENT begin,
and always zero +4,+8,+C, with +0 untouched. Their complete contracts differ in
physical entry/cleanup, not these effects. This packet directly reuses
accepted `00432440`, as coordinated with the independently owned new actual
`00559DC0` wrapper packet. The older `Fe3TemplateHost` projection in
`frontend_seg12_17_cc13.cpp` has callbacks and an early return for null begin,
which would preserve stale end/capacity; it is not used or modified here.

Dispatcher assembly recovers **five physical stack words**, unlike its
pseudocode's single formal parameter. It forwards helper(first,last,destination,
incoming_ECX,destination_bits,incoming_ECX&FFFFFF00h). Only the low local tag
byte is zeroed; its upper24 bits retain ECX. The first and third original stack
words are ignored; all three helper metadata words are proven dead, including
the owner passed only to the ignored ECX of Tidy. Metadata needs no fabricated
owner object or valid pointer identity. Dispatcher forwards EAX unchanged.
Its final POP ECX restores the modified tag, not the original ECX; this ambient
register fact is not a new semantic Source result.

New fastcall Source entries retain original register/stack word positions,
reserving ignored EDX for the dispatcher, and append the actual context.
Source helper consequently returns RET14h rather than native RET10h; Source
dispatcher RET18h rather than RET14h. Actual Release COFF confirms constructor
call/null skip, native32 advances, completed-prefix catch/Tidy/rethrow, EAX
completed end, and both new cleanup counts. Invariant Source parameter reloads
and original private frame aliases are outside the declared domain. Source
C++ catch, child CRT/diagnostic/zero-copy boundaries, native catch/RTTI/handler
identity, and fault/unwind behavior are distinguished explicitly. No wrapper
contains x87/FP arithmetic; NaNs are not involved.

`./scripts/build.ps1` passed the exact new source and all three existing suites:
reconstructed_math, native_math_differential, tool_tests. No new tests were
added. Static original PE/EH assertions and actual compiled-output inspection
are evidence, not packet-specific native execution or original ABI proof.
The JSON report records exact call checks, extraction provenance and whitespace
checks. No native exception execution, game startup or gameplay result is claimed.

Exact native CALL sites:

```text
00559E47 CALL 00557BC0
00559E6A CALL 00559DC0
00559E7A CALL 00BF6885
0055A96C CALL 00559E00
00C6DAC8 CALL 00401130
0055B1F9 CALL 00559E00
```
