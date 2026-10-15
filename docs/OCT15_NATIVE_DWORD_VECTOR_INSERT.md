# Actual DWORD vector count insertion

`insert_native_dword_vector_count_00697990` reconstructs the complete ordinary
entry with accepted actual allocation, length error and raw DWORD range
services. It adds no provider, container overlay or allocation model. The
established `STL_Vector_InsertN_00697990` identity and older qualified interfaces
remain separate; the new C++ name is a hypothesis.

| Native scope | Coverage | Source representation |
| --- | --- | --- |
| 00697990..00697AF0 and 00697B06..00697BCA | Complete ordinary function, 550 bytes / 216 instructions | Six-formal void fastcall API |
| 00697AF1..00697B05 | Complete native catch effects, 21 bytes / 7 instructions | Lexical new-backing catch |
| 00697AFD..00697B05 | Interior shared-rethrow fragment only, 9 bytes / 3 instructions | Lexical short-tail catch; no whole-function promotion |

Native ECX is the receiver and incoming EDX is unconsumed. The four stack DWORDs
are unused owner, position, unsigned count and value address. All three normal
exits use `RET10h`. The sole caller discards insertion EAX before producing its
own result. The new Source interface retains the physical argument positions
with an explicit unused EDX parameter; it does not promise binary compatibility.
The actual 16-byte header has opaque/begin/end/capacity at 0/4/8/C. Its elements
are unowned four-byte DWORDs, and opaque0 is untouched.

The value is read eagerly into a private DWORD before the capacity and count
checks. Count0 still performs this pointed read and capacity capture. Distances
use x86 `SAR2`; additions, subtractions and scaled addresses wrap in 32 bits.
The length and capacity comparisons are unsigned. Growth is the observed
one-and-a-half candidate, reset to zero on its threshold failure, followed by
the required-size choice. Each native repeated end read is retained. The actual
length service never returns despite its old Ghidra flag; the decompiler's
extraout-register continuation does not justify a Source overflow fallback.

After allocation succeeds, the growth branch captures position and reloads
CURRENT begin. It copies the prefix, fills from the private captured DWORD and
reloads CURRENT end for suffix copy. It then reloads CURRENT begin/end to size
the old allocation, conditionally frees that captured begin, computes both new
addresses and publishes capacity, end, begin in that order. Allocation precedes
the protected state. The state-zero catch frees ONLY saved new backing and
rethrows, with no POD prefix destruction, old-backing free or rollback.

For a short tail, forward copy happens first. CURRENT end is then reloaded for
the remaining fill count. State2 protects fill_n, an actual DWORD read-modify-
write ADD to CURRENT end, a fresh end read and the final in-place gap fill.
Its catch targets the shared rethrow interior and performs no free or rollback.
For a long tail, the copied tail's result is published as end BEFORE backward
move and gap fill. Parent state remains -1 in this branch; a later failure keeps
the expanded end and earlier stores. Neither path becomes a generic vector or
memmove operation.

The parent captures the caller's DWORD VALUE once; range helpers receive its
private ADDRESS and retain progressive rereads and ordered stores. Raw forward
copy/fill_n null-output skips and cursor advances remain callee behavior;
in-place fill has no null skip. The first growth tag has native uninitialized
high bytes but is proved unconsumed by the complete core. Source passes explicit
dead zero rather than evaluating an indeterminate C++ scalar or inventing an
ambient input. The suffix's low-byte-cleared captured value is retained as dead
metadata. Original private argument/prototype aliases are outside this API.

Fresh read-only queries verified `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Both ordinary ranges, the separate catch and the
FH3 handler match the installed PE and all 225 current instruction starts.
The earlier complete dependency audit adds all actual helper bodies and exact
targets. No listing, function-body, flow or script-backend repair was made.
Cached nearest-entry callee summaries conflate the catch and second normal
range; exact typed ranges and decoded sites establish ownership instead.

| Native site and target | Actual effect |
| --- | --- |
| `006979F4 CALL 00556C20` | Accepted owning length error |
| `00697A55 CALL 00553260` | Accepted same-count raw DWORD allocation |
| `00697A7A CALL 00696D40` | Prefix forward copy |
| `00697A8A CALL 00697760` | Fill_n at prefix result |
| `00697AA2 CALL 00696D40` | Suffix forward copy |
| `00697AC4 CALL 00BF65AC` | CURRENT old backing free |
| `00697B28 CALL 00697860` | Short-tail forward-copy dispatch |
| `00697B4B CALL 00697760` | Short-tail remaining fill_n |
| `00697B61 CALL 006970B0` | Short-tail in-place fill |
| `00697B92 CALL 00697860` | Long-tail forward-copy dispatch |
| `00697BA0 CALL 00697440` | Backward copy |
| `00697BB0 CALL 006970B0` | Long-tail in-place fill |
| `00697AF5 CALL 00BF65AC` | Saved NEW backing free in first catch |
| `00697B01 CALL 00BF6885` | Shared rethrow with two zero stack arguments |
| `00C7EBC5 JMP 00BF6B43` | Existing FH3 handler transport; metadata evidence only |

Handler C7EBC0 selects FuncInfo DAB938. Its four unwind entries at DAB918 all
have previous=-1 and action0. The two DAB95C try entries are state0..0 with
catchHigh1 and state2..2 with catchHigh3. Their catch-all records target AF1 and
the INTERIOR AFD respectively. All 140 map bytes match current live/disk data.
No native handler, EBP overlay or additional unwind action is implemented.
Lexical Current C++ catches retain the two distinct effects and use bare throw.

The accepted allocator/throw pin `73f81925` is on main. Its length service owns
the actual 18-byte-message SBO temporary and 28h legacy exception payload; no
generic length_error or second cleanup guard is introduced. Allocation/free,
new-handler state, C++ cleanup/RTTI and exceptions use the accepted current CRT
domain. Reached storage and finite helper termination are preconditions, and
the writable header must survive old-backing free through publication. Native
FH3/exception/heap identity, private spill aliases, asynchronous mutation,
faults/SEH, register residue and game ABI/runtime equivalence are unverified.

The exact Source passed `./scripts/build.ps1` and all three existing suites
(`reconstructed_math`, `native_math_differential`, `tool_tests`) in 8.18s.
Current COFF inspection confirms eager value/capacity accesses before count0,
the raw end ADD and fresh reload, and three four-stack-word returns. Its own
compiler emits four zero-action unwind states, both try regions and a shared
rethrow interior after the new-backing free prefix. Full extraction, symbol
relocations, raw Source tables and an archived object accompany the report;
these are Current C++ compiler artifacts, not a native handler replacement.
All 15 native transfer rows/Markdown claims and diff checks pass. No new tests
were added; existing regression/seed suites do not execute this insertion API.
