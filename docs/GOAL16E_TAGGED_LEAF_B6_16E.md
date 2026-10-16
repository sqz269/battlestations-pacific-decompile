# Tagged B6 raw leaves

Four new C++ entry bodies reconstruct complete native operations. Every whole
strict C++17 and registered Release/Win32 executable section matches its whole
Native/live/installed-PE body: **four entries, 45 bytes, 19 instructions**.
These are measured raw operations; names are hypotheses, not recovered classes
or semantic field types. No helper, global, allocator or provider is substituted.

| Original address | Observed complete operation | Native / strict / registered bytes |
| --- | --- | ---: |
| 00b663f0 | read raw dword at wrapping ECX+0Ch into EAX | 4 / 4 / 4 |
| 00b6bdd0 | null-aware signed wrapped address difference divided by 12 | 30 / 30 / 30 |
| 00b6d980 | read raw dword at wrapping ECX+38h into EAX | 4 / 4 / 4 |
| 00b6fc60 | return uint32 ECX+1ACh, without dereference | 7 / 7 / 7 |

OriginalABI consumes only ECX and returns full EAX through plain `C3 RET`:
ESP advances four bytes, with no callee argument cleanup or consumed EDX/stack
argument. One-argument `__fastcall` is the new Source spelling for this measured
physical interface; original formal types and convention spelling are unknown.
The exact complete bodies preserve the original registers, defined/undefined
EFLAGS effects, read schedule and fault-time machine effects. There are no
prologues, pushes, local-stack accesses or spills: storage aliasing the incoming
return stack is not silently excluded. x87 state, FIP and SSE state are untouched.
Code placement, call-site adoption, installed entry replacement and game ABI
integration are not established by entry-body identity.

The raw reads use `memcpy`, avoiding invented class/field alignment. Pointer bits
are converted to uint32-width `uintptr_t` before offset arithmetic, retaining x86
address wrap without requiring a fictional complete object extent. Only each
actual four-byte read must have accessible backing storage; there are no wider
loads, typed-pointer subtraction or added reads. Invalid memory is not exercised
by a probe. Original/source code-fetch addresses differ after relocation; no
equivalence of code placement or inaccessible code pages is claimed.

`00b6fc60` uses unsigned address bits for every uint32 input, including ECX=0
(result 1ACh). Its actual instruction is LEA, which preserves flags and does not
read the resulting address. The existing `OceanBeamHost` void callback can model
a caller that ignores this return value. Its inaccurate "empty body" comment
was corrected after checking and claiming that header lease. The callback API,
method name and other identities were preserved.

`00b6bdd0` reads begin at +4 exactly once. Begin zero returns EAX=0 immediately
without reading +8, preserving incoming ECX/EDX and leaving the original TEST
flags. Otherwise it reads end at +8, wraps `end-begin` modulo 2^32, interprets all
32 bits as int32 and divides by twelve with truncation toward zero. Negative
differences, end-before-begin and INT_MIN are admitted; no valid-vector-range
guard or container owner was invented. Final code retains the native IMUL/SAR/
SHR/ADD sequence and register effects, not merely its positive-range result.
The Source bit transfer uses `memcpy`, with no implementation-defined signed
conversion or signed-overflow subtraction.

For the native signed quotient, m=2AAAAAABh satisfies `12*m = 2^33+4`.
The product-high/arithmetic-shift sequence computes floor(x*m/2^33); its sign
correction truncates toward zero. The approximation error has magnitude at most
1/12. The endpoint INT_MIN has remainder eight modulo twelve, so no boundary
exception occurs. Whole instruction identity provides independent physical
evidence for the same computation; no new arithmetic test suite was added.

The inclusive indexed band contains 349 function records and 58 tag-only
candidates. This is metadata, not a completeness claim for the other records.
Existing coverage was qualified rather than inferred from missing ledger rows:

- `00b663f0`: caller comments and abstract argument-count callbacks exist;
  concrete game hosts derive counts from Lua state/model values. They do not
  supply this raw dword-at-ECX+C entry body.
- `00b6d980`: `scene_graph2` has a model `child_count` member and abstract
  `child_count_38`/node-query methods. Those are not this raw +38 entry body.
- `00b6fc60`: weather/water abstractions approximate caller consumption, not
  the whole no-dereference address-return body.
- `00b6bdd0`: the existing script-loader header explicitly uses new interfaces
  and `std::vector<CompressedScriptRecord>`, not native vector layouts. Its
  documented native helper range was left undone. No old named callee record is
  accepted as a concrete provider merely because it exists.

The other 54 tagged B6 candidates are outside this selected four-leaf packet;
their whole coverage, provider wiring and readiness are not asserted. The prior
thirteen deferred B5 scalar-delete candidates retain their individual status;
no common blocker or completed coverage is claimed for all of them.

Unique Source/header/document/report paths were checked absent at own base
`d28c7026a6988cc3beefd90a1f45f67dbb2eef86` and current Main, rechecked immediately
before creation at Main `23ad369e32c1c99388856798f5fa850ed9246d43`. Read-only typed
Ghidra metadata verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
language `x86:LE:32:default`, image base `00400000`, and complete single-range
bodies. Every live byte came through the CLI and matched the installed PE.
There were no Ghidra writes.

The first Source form used C++20 `bit_cast`. Its strict object emitted five
complete executable sections / 52 bytes: four exact entry bodies / 45 bytes
plus a seven-byte library helper. The actual C++17 build failed with C2039,
C2065 and C2275. This full rejected Source/object/log is retained; the helper
was not clipped or counted as a native routine. The first timing wrapper aborted
on that terminating exception before saving its stopwatch. Observed tool command
wall was 9.4324306 seconds, explicitly labeled as tool-wall evidence.

The repair uses a C++17-compatible bit-preserving `memcpy` and keeps the project
standard unchanged. An intermediate strict C++20 memcpy object / 45 bytes was
preserved before the final strict C++17 attempt. Final strict flags are
`/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy`. Both final strict and actual registered
objects contain exactly the four complete entry sections, every symbol offset
zero, no undefined external provider and zero executable relocations. Four
`.debug$F` type-seven metadata relocations target their own function symbols.
Every section/data byte, symbol/auxiliary record and relocation is recorded.
Exactly one real archive member matches the actual registered object byte for
byte. No native-size prefix, tail exclusion, relocation mask or extra helper is
used by the final proof.

The corrected warm `scripts/build.ps1` completed with both existing CTests
passing: 18.8802227 seconds for build plus tests, including 7.47 seconds in CTest.
This interval is separate from failed build/strict attempts and all setup,
qualification, docs, ledger, commit and handoff overhead. No new tests or ad hoc
executable probes were added. No native execution or game validation occurred.

Before any new build, the prior reviewed B5 Source/header/docs/report, all local
evidence, strict object, registered object and full real archive were copied and
hash-verified: 31 files / 94,004,548 bytes. Their seven Source bodies / 21 Source
bytes / seven Native bytes and production pins remain under
`local/goal16e_tagged_b6/frozen_16b/`, with a separate provenance manifest.
The old tracked Source/docs/reports remain unchanged. The warm worktree is kept.

The report pins Source, header, strict/actual objects, archive member, rejection
artifacts and provenance. Full inclusive timing begins at 2026-10-16 05:54:39 UTC
before brief, preservation and index work; the final ignored handoff artifact
extends it through commit, lease release and handoff preparation. Failed builds,
the timing-wrapper repair, all strict attempts and classification overhead are
included. No controlled overall speedup is claimed from the warm build or count.
