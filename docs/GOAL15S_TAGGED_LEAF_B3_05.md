# GOAL15S tagged B3 leaves, packet 05

Twelve complete Source bodies close the twelve uninspected entries in the
original 79-candidate screening inventory. They cover 62 Native bytes and 56
actual registered Source bytes: eight entire machine entries are identical
(35 bytes), and four complete Source entries have explicit qualifications
(27 Native / 21 Source bytes). This is inventory accounting, not a claim that
the whole B3 address band or its indirect callers are reconstructed.

Names describe operations and remain hypotheses. The old `TRIV_body_*` tags,
existing library names, and older Source interfaces are preserved. No class or
semantic field identity is invented. The candidate index's missing FUNCTION
records were screening hints; full Source/header searches and whole actual
bodies established the missing coverage individually.

| Native entry | Complete Source operation | Native / Source bytes | Coverage and evidence class |
| --- | --- | --- | --- |
| `00B32910` | zero dword result, pop 4 | 5 / 5 | complete; exact entire COMDAT |
| `00B33A90` | store dword at +8, expose resulting EAX value | 10 / 6 | complete; qualified new input interface |
| `00B33E60` | read dword at +18h | 4 / 4 | complete; exact entire COMDAT |
| `00B33E70` | read dword at +1Ch | 4 / 4 | complete; exact entire COMDAT |
| `00B33E80` | zero byte result | 3 / 3 | complete; exact entire COMDAT |
| `00B33F30` | empty body | 1 / 3 | complete; RET versus RET 0 encoding qualification |
| `00B33F40` | empty body | 1 / 3 | complete; RET versus RET 0 encoding qualification |
| `00B34070` | empty body, pop 8 | 3 / 3 | complete; exact entire COMDAT |
| `00B34080` | zero dword result, pop 8 | 5 / 5 | complete; exact entire COMDAT |
| `00B34550` | read dword at +4 | 4 / 4 | complete; exact entire COMDAT |
| `00B34560` | load child address at +24h, read child dword +4 | 7 / 7 | complete; exact entire COMDAT |
| `00B347C0` | child word plus index times 8 modulo 2^32 | 15 / 9 | complete; qualified new input interface |

`include/bsp/native_tagged_leaf_b3_05.hpp` contains the exact ready Source names
and prototypes. Each entry is a whole ordinary C++ body. The genuinely empty
Native bodies justify the empty Source bodies; these are not unresolved stubs.
The module has no calls, providers, unresolved externals, absolute game-global
bindings, added guards, speculative class types, or extra test framework.

## Original physical contracts and new Source interfaces

`00B32910` and `00B34080` execute XOR EAX,EAX and RET 4/8. `00B34070`
executes RET 8. The stdcall Source envelopes choose one/two opaque uint32
words to express this cleanup. Original does not read those argument cells or
consume ECX/EDX, and the code establishes neither their semantic types nor a
receiver. The return instruction still reads the return address. A zero-word
result exposes the concrete EAX result rather than a recovered language type.

The three raw getters consume ECX only and return a four-byte value in EAX
through plain RET. `00B34560` first reads four child-address bytes at owner+24h,
then four bytes at `(child_bits + 4) modulo 2^32`. Its Source uses unsigned
address arithmetic and memcpy; there is no aligned typed record or requirement
that an entire hypothetical parent/child structure be readable. Only the
touched ranges need valid stable backing. Aliases between readable ranges are
permitted. `00B33E80` is XOR AL,AL followed by RET: upper EAX remains its full
incoming preimage, ECX/EDX are preserved, and XOR8 defines the same flag effects
in the actual strict and registered Source objects. The uint8 interface does
not turn upper EAX into a semantic result.

`00B33F30` and `00B33F40` are exactly one Native RET. Ordinary void cdecl Source
emits the entire three-byte RET 0 instruction. On a valid return stack and
executable instruction backing these have the same return-address read,
ESP+4, register and flag effects. They are not byte-identical, and their code
fetch extents differ. The audit retains both whole extents rather than a
one-byte Source prefix. Neither body consumes an argument or receiver.

`00B33A90` loads a dword from entry ESP+4, stores it to ECX+8, and RET 4. EAX
contains the stored value; ECX, EDX and flags are preserved. The new meaningful
two-argument fastcall interface receives base in ECX and value in EDX. Its
whole Source body stores EDX to base+8, moves EDX to EAX, and RET. The value and
single four-byte destination store agree after mapping the supplied value to
the Original stack word. No receiver/class identity or Original EDX input is
claimed. Original performs a stack-cell read that Source does not; cleanup and
stack alias relationships differ with the new calling boundary. At a failed
store Original has already defined EAX while Source has not. Same normal
store/result behavior therefore does not establish Original ABI, SEH context,
or fault equivalence. The Source accepts any valid writable destination range;
it adds no class, alignment, nonalias or value validation rule.

`00B347C0` reads child address at owner+24h, reads child word0, then reads index
at entry ESP+4. Native computes `(word0 + index*8) modulo 2^32` using LEA and
RET 4; ECX becomes word0, EDX becomes index, and flags are preserved. The new
fastcall interface receives owner in ECX and meaningful index in EDX. Its
whole Source reads owner+24h then child word0, computes the same unsigned
formula with LEA, and RET; ECX remains owner and EDX remains index. Both input
words admit all 32-bit bit patterns, including wrap and signed-negative index
representations. Only the two actual data ranges require stable readable
backing, with arbitrary aliasing between them and no store. Native's later
stack-index read, cleanup, intermediate registers and fault context are not
preserved by the new interface. There is no index-bound, null or empty guard.

## Existing interfaces and caller evidence

Fresh live xrefs show `00B00950` calls `00B33E60` at `00B00953`.
The caller preserves ECX in ESI, calls without argument pushes,
compares signed EAX against the DXT value and 15h/16h, then selects existing
texture operations. Existing `RenderMiscHost::texture_format_00b33e60` is a
named but unimplemented virtual host boundary; its consumer remains unchanged.
The new raw uint32 getter preserves the concrete four-byte representation and
does not claim to bind that host or recover its production class. The report
records the declaration/caller identity so it cannot be mistaken for a prior
complete Source body. These leaves themselves have no incomplete dependencies.

Fresh data refs for six entries are checked against their installed PE
pointer words. The report records every actual reference. No-ref observations
for the other entries do not prove them unused or close indirect dispatch.

## Whole-body verification and inventory

The receipt in `reports/goal15s_tagged_leaf_b3_05.json` retains every Native
byte/instruction and every whole actual Source executable COMDAT, symbol
offset zero, SizeOfRawData, flags, relocation count, hashes and evidence class.
Every nonempty executable section is accounted for, including the longer
empty-body encodings. There are no executable-section relocations or undefined
providers. The strict standalone object and actual registered CMake object are
checked separately; their entire per-entry Source bytes agree. Standalone
compiler options are labeled as such, and actual CMake flags come from its
CL.command tlog. The actual Core archive member is checked. Source-only
arithmetic/dataflow reasoning establishes the two changed-interface semantic
results; it does not claim a module-wide byte match or Original ABI adapter.

Fresh typed live metadata, live bytes and installed PE bytes establish actual
Native extents, not entry spacing. Target verification uses saved bsp.gpr,
`/battlestationspacific.exe`, i386, base 00400000. Ignored artifacts are retained
under `local/goal15s_tagged_leaf_b3_05/`, including audit.py, strict/registered
objects and dumps, live typed flow, xrefs/bytes, caller listing, build log,
compile command provenance, report checks and final handoff timing.

All 79 screening candidates are now inspected: 53 complete Source entries
cover 251 Native bytes, comprising 46 exact entries / 194 bytes and 7 qualified
entries / 57 bytes. Twenty-six earlier deferrals remain; zero candidates are
uninspected. The report lists every deferred address and actual contract.
Several deferrals were earlier family/capacity choices or byte-gate failures,
not unresolved semantics: raw getters, identity, plain/cleanup returns,
byte-zero, stack setters and five three-dword zeroers can receive subsequent
qualified Source work. Two actual-global readers need storage binding; three
memory ADD decrements need RMW/flags/alias review; the SSE copy needs its full
stack/overlap/fault contract; and one FLD32 return needs x87 qualification.
No earlier deferral is silently counted complete or revisited for Source here.

The registered `scripts/build.ps1` result, two existing CTest suites and report
checks are recorded in the report. No new tests or runtime probes are added.
Full qualification-to-precommit timing includes all discovery/rejections,
Main merge, cache rebuild, evidence capture, documentation and review overhead;
`handoff_timing.json` separately extends that measured interval through commit,
lease release and handoff preparation. Build duration is not total effort.
There was no game execution, installation, Ghidra mutation, or drop-in claim.
