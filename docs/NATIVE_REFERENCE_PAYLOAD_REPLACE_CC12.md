# CC12 physical reference payload replacement (008F0310)

The worker implementation, one-process focused fixture and current Main Win32
build/three checks passed. **Source 0 / ready 0** remain pending Root's independent
fresh fixture, shared CMake integration/build and publication. The new consumer
was compiled in the four-TU fixture; the unchanged Main CMake list does not yet
include it. No native class, caller, private heap, failure/EH or game behavior is
admitted by these results.

## Whole body and public contract

The complete native range is `[008F0310,008F0340)`: **48 bytes / 16 instructions**,
SHA-256 `c4c713091d8c81c8083a9c53eba6d356e55cb459b8c619ac703ead3796b04abf`.
The descriptive C++ name is a hypothesis. The Win32 MSVC naked padded-fastcall
interface is:

```cpp
std::uint32_t __fastcall replace_native_reference_payload_008f0310(
    void* destination_ecx, void* unused_edx,
    const char* borrowed_text_stack, std::uint32_t raw_scalar_stack);
```

ECX is actual readable/writable eight-byte payload storage. Native entry ESP+4
contains a borrowed text pointer and ESP+8 contains full scalar bits; incoming
EDX is unused. The body reads old +4 while live, frees a nonnull old allocation
through the actual matching current free, executes ADD ESP,4 and clears +4.
It then loads the text argument into ECX, calls the admitted physical duplicate,
stores the actual new pointer/null at +4, reads the scalar argument into EAX,
stores it at +0, restores ESI and returns with RET8.

**Full EAX is the scalar on every successful path, including null text.** The
owned allocation is observed through payload +4. No source-payload argument,
phase, vtable or other class field is involved. Successful writes after the
duplicate occur in pointer-then-scalar order. Normal EBX/ESI/EDI/EBP survive.

Exactly 40 bytes remain literal. Only CALL operands `[12,16)` and `[31,35)` bind
the actual freshly compiled current canonical free and admitted physical
duplicate57, respectively. The relocated original whole48 and Source use those
same actual functions. The fixture independently rechecks the runtime original's
40 literal bytes and both actual targets. No branch/prefix substitute, original
private free, legacy CDECL duplicate, fake object/vtable or provider callback is
used. The duplicate's existing admitted allocation adapter is unchanged.

## Ownership and successful-call domain

Old +4 is null or a sole-owned actual result of the matching current duplicate.
Payload, borrowed text and old allocation are disjoint and outside active call
frames/argument slots. Borrowed text is null or remains readable through NUL with
representable length+1. Text that aliases the old allocation would be freed
before duplication and is excluded. Current CRT requires DF clear.

The consumer frees a real old current copy once. The caller reads a nonnull new
pointer from +4, observes its complete n+1 bytes while live, and frees it once
before payload reuse/disposal. Borrowed text is not freed, scalar EAX must not be
freed, and retired old bytes are never read. The allocator actually reused the
old address for both native and Source nonnull replacements in this fixture;
no old/new pointer inequality is required. There is no `noexcept` promise or
guarantee about OOM, exceptions, unwind cleanup or native owner/destructor life.

## Saved analysis and nested ABI

Root repaired CALL_RETURN to NONE at 008F031B once, decoded the genuine ten-byte
ADD/clear gap `[008F0320,008F032A)`, preserved complete prototype/comments/labels,
saved under the lock and refreshed the export. Root's 20-file repair seal is
`147bb521452219953ea701a1e21c8a2be0f2d0e52857587a4b396bf368c073f3`.
Worker preparation confirmed all 16 actual saved starts against installed/live
48-byte disassembly. Cached signature count14 is historical. The worker did not
replay repair, enable scripts, mutate callee NoReturn or query completed consumers.

Let T be the actual first/text argument-slot ESP before outer CALL; scalar is
at T+4. Outer entry is T-4 and saved ESI is T-8. Optional old-free argument,
entry and return are T-12/T-16/T-12; ADD4 restores T-8. Duplicate entry is T-12,
its saved EBX/ESI/EDI are T-16/T-20/T-24 and size argument is T-28. Memcpy
destination/source/count are T-40/T-36/T-32; memcpy enters T-44 and returns
T-40. Its final ADD16 yields T-24, duplicate RET0 yields T-8 and outer RET8
yields T+8. Both actual argument slots and values were captured before CALL.

Null text leaves ECX zero and EAX equal to scalar; arithmetic mask `0x8C5`
expects `0x44`, excluding XOR's undefined AF. Incoming EDX survives only when
old +4 was null; real old free is volatile. For nonnull text, EAX is still the
scalar, ECX/EDX are volatile and all six arithmetic flags under mask `0x8D5`
derive from `ADD32(T-40,16)`. DF0 was checked before and after. There is no
blanket full-EFLAGS, FP, MXCSR or segment-state assertion.

## Fresh fixture and physical provider evidence

Four fresh TUs compiled once: new consumer, unchanged admitted physical duplicate
and size adapter, unchanged canonical allocation/free, and newly authored RET8
probe. No BSP archive, old object, old helper execution or old process was used.
The Win32 /MD /O2 /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL- fixture uses
duplicate /Oi-, /OPT:NOICF and an embedded asInvoker manifest.

The seven entries were three native/Source raw pairs (old-null/text-null,
real-old/text-null, real-old/nonempty), plus ordinary Source old-null/nonempty.
Four actual fresh duplicate calls established old ownership after all gates;
four actual in-consumer frees completed. Three new copies were observed through
the full eight bytes `9d2bf1658347da00`, including NUL, and caller-freed once.
Old setup text was all seven bytes `72657469d35100`. Copied observation size
equals the entire chosen meaningful string including its terminator.

Scalar seeds were 91B2C3D4, E5F687A9 and 9ABCD123 for paired raw cases, and
FEDCBA98 for the ordinary call. Full scalar EAX was checked on every path.
The 88-byte capture contains both actual text/scalar argument slots and values.
All seven 48-byte before/after payload records and complete 48-byte borrowed and
old-text records were checked, including eight payload bytes, surrounding guards,
input preservation, GPR/ESP and three matched actual raw caller frames. Recorded
flags were 582, 582, 518, 582, 582, 518, with only justified masks asserted.

All **38 complete code spans** were derived from current COFF/map/linked evidence
and gated before zero old-setup/consumer/target-free entries, then afterward.
They include consumer48/16, duplicate57/30, adapter61/17, allocator90/34, free6/1,
ordinary caller22/6, new raw caller157/57, **whole main4895/1250**, all retained
probe helpers and actual import thunks. The actual security-cookie helper includes
all 14 bytes/four instructions and its failure tail JMP. The actual map-owned
stack probe includes all 43 bytes/19 instructions. No main/helper prefix is used.

Actual I386 UCRT malloc/free/_callnewh and distinct VCRUNTIME memcpy were checked
through real IAT/export resolution, MEM_IMAGE, module/export RVA, mapped versus
physical NT path, volume/file ID, full file size/SHA-256 and actual ASLR-relocated
physical export prefix32, before setup and after the cases. Identical prefixes
alone do not identify a provider. Observer activity is separate from target
counters. The old-free counts follow gated complete control flow, actual nonnull
owned inputs and successful return; no interposed hook manufactures those calls.

## Build, provenance and integration boundary

The worker baseline is Main `c700966eb7b3f4a5a37fb0b90a2f09cdc8aec1ee`.
`scripts/build.ps1` completed its current Main Win32 build and all three existing
CTest entries: reconstructed_math, native_math_differential and tool_tests.
Current CMake includes the earlier copy consumer and excludes this new replacement
TU. The four-TU fixture provides this new implementation's compile/link/ABI
evidence. Root still owns shared CMake registration, a fresh independent fixture,
combined build and Source publication.

One precompiler guard failed before any compiler: its plain text split matched
its own `def launch():` string literal. The original recipe and failed attempt
remain immutable. A separate fresh build-continuation helper selected the actual
launch FunctionDef through Python's syntax tree, verified exact
`prelaunch_review.json` / `accepted_for_single_process` before the first compiler,
and compiled the four fresh TUs once. Exact COFF symbol matching was already used
before substring fallback. No preparation or accepted process was repeated.

All actually consumed fixture sources, headers, libraries, tools, provider files,
objects/executable and the supplemental build helper passed post checks. The
readiness65+older1212 union and Root20 repair family deduplicate to **1,297 prior
artifacts**, all unchanged with original hash associations preserved. Unconsumed
moving dispatch/CMake generation metadata was frozen separately. Provider file
checks and current-provider fixture evidence do not establish original private
CRT compatibility.

The family is `local/cc12_reference_payload_replace_worker20261008a/`.
Its immutable 105-artifact core seal is
`6681949664db6bd02c0aafd2da97f15a7bb580bc12bc07a7ef5f2971df75f06b`.
The final recursive manifest adds build logs, metadata, failures, helpers and
handoff, excludes only itself and is verified against exact inventory. Source
admission stays zero until Root completes its independent work. Native production
caller transport remains unobserved; native class/owner/destructor, private heap,
OOM/EH/failure, World integration, startup and gameplay remain unadmitted.

## Root independent source validation and integration

Whole [008F0310,008F0340)48B16 reference-payload replacement in the qualified successful current-owned string domain. Only CALL operands[12,16) and[31,35) bind actual canonical free and admitted physical ECX/RET0 duplicate57;40 other bytes literal. Actual ECX writable disjoint8-byte raw payload, padded unusedEDX, two physical text/scalar stackDWORDs and RET8; fullEAX=scalar on all paths, actual owned child from+4. Nonnull old child is a genuine currently owned duplicate, freed inside the body before new text copy and scalar store; borrowed text must not alias retired old storage. Root NEW local/ref48p2 uses four fresh compiler TUs with all33 retained symbols/31 distinct full TU bodies, including full5039B1287 Main and157B57 raw capture, plus18 complete map-owned cookie14/stack43/cold-delete16+5/import helper spans:49 actual runtime byte gates before0 target/setup entries and after all frees. Independent raw COFF/map/PE/packed-gate parser checked268 distinct physical relocation operands via274 symbol-level checks; actual weak auxiliary mode1 NOLIBRARY fallback distinguished from mode3 ALIAS, defined target selection proved by same map address/full extent/full linked bytes. Cookie separate failure destination and cold EH/OOM execution remain unadmitted despite gated cold code. One new process/seven entries, four actual old8-byte setup copies/inside frees and three new10-byte copies/result frees:seven real allocations/frees. All six88-byte serialized captures and all seven full before/after48-byte destination/new-borrowed48/old-borrowed48 snapshots independently decoded; newE358B79A4FC261D83500 all10 bytes includingNUL, old borrowedF6A17D38CB952400 all8 bytes includingNUL, guards preserved. Actual old setup bytes are compared while live by the complete gated Main, not separately serialized; no retired reads or old/new address inequality required. Distinct derived R/Q/T: Q=R-28,T=Q-4; textQ/scalarQ+4, RET8 Q+8, nonnull last ADD16 flags from actual memcpy-returnQ-40; null XORmask8C5 excludes undefinedAF. Full scalarEAX, nonvolatiles/DF0 verified; incomingEDX checked only null/no-old-free, provider volatile outputs otherwise unasserted. Four actual loaded I386 heap/copy providers independently verified by actual IAT/export/MEM_IMAGE/physicalNT/fileID/full DLL SHA and ASLR-adjusted32-byte prefixes before/after. Exact119 artifacts+seal120 and20984 prior pins preserved, including stopped ref48p1 exact76 before any target, old flow-repair/worker/Rootfamilies and failed parser logs; no old accepted helper/stage/object/process or flow-repair replay. Combined current Main Win32 build/all3 existing checks passed after concurrent physical-pool/property-bag C++ integration, with Source48 CPP/HPP unchanged. Original private CRT/heap/owner/class/destructor/EH/failure/fullclone/native caller/game ABI/startup/gameplay remain unadmitted; descriptive name is a hypothesis.

The erroneous negated descriptor is ignored; sizes are established by sealed probe constants, complete gated setup comparisons and independently decoded borrowed/copy bytes.
