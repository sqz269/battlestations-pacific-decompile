# CC12 tick subnode scalar deletion Source candidate

Current Source is registered and build-tested. The complete Native30/11 wrapper maps to39 Source bytes /13 operations with actual publication forwarding, late flags and explicit RET0C. Original ABI and allocator policy remain unproved.

## Integrator compiled review

Root replayed the candidate Native/Source mapping, actual declarations, repository/dependency pins and relevant excerpts before registration. The normal MSVC Win32 build passed all three existing checks. Root reviewed every emitted operation, complete COFF object and indexed relocation graph, actual Core definition and current map/import context. All twelve prior whole objects remain byte-identical, including Legacy20 code/relocations and eight EH payload/relocation contracts. The existing pending-entity provider whole object, its twelve functions and two44-byte EH sections are also unchanged. No new test, probe, storage, caller, production binding or forced retention was added. The immutable worker report below records its candidate stage; current admission is recorded in the primary report.

Current evidence: `reports/cc12_native_tick_subnode_scalar_deletion_primary_review.json`.

## Worker candidate snapshot

This packet adds only `scalar_delete_native_tick_subnode_0071c4d0`, a new
MSVC Win32 naked fastcall Source interface for the complete 30-byte wrapper
at `0071C4D0..0071C4ED`. It is a candidate awaiting the integrator's CMake
registration, normal build and complete emitted-object review. This packet
does not admit Source credit, a Native ABI replacement or gameplay behavior.
The second 30-byte wrapper, `007F1EE0`, remains deferred because its
`007F1E70` cleanup has no admitted raw Source implementation at this baseline.
There is no declaration, stub or arbitrary Original-address call for it.

The candidate uses the raw `cleanup_native_tick_subnode_base_00875b30`
interface admitted on published `main` at baseline
`fbaefa43f0135f71e019bb995c48078c8fa3306f` (the report records the exact Git
baseline). Its admission report records a successful normal build, three
existing checks, a complete 97-byte/33-operation Source body, twelve whole
objects and fourteen positive public Core roots. Those are dependency
results, not build or emitted evidence for this new wrapper. Earlier scalar
readiness and the base cleanup candidate report describe older Source absence
or pending admission; the current base admission supersedes those statements.

## Original evidence and complete schedule

The accepted scalar readiness gate and Root capture supply both complete
30-byte raw windows, eleven instructions each. The saved Ghidra listing has
ten starts: it omits the physical `ADD ESP,4` at `0071C4E5` and `007F1EF5`.
Raw bytes establish both omitted instructions. The previous read-only flow
query was rejected with `Script execution disabled`; it established no flow
properties. This packet changes no listing, flow property or no-return flag.
The companion wrapper is retained only as deferred context; its child body,
the Native free body and all unrelated capture records remain unopened.

| Original start | Owned operation | Candidate treatment |
| --- | --- | --- |
| `0071C4D0` | Push ESI | Same physical operation |
| `0071C4D1` | ESI = ECX | Capture actual receiver |
| `0071C4D3` | Call `00875B30` | Call admitted Source child with actual F/M references |
| `0071C4D8` | Test low byte at ESP+8 against 1 | Same late current flags read |
| `0071C4DD` | Jump to `0071C4E8` if zero | Same selected free/no-free paths |
| `0071C4DF` | Push ESI | Pass actual current ESI |
| `0071C4E0` | Call library-named `00BF65AC` | Call current CRT `free` through its real declaration |
| `0071C4E5` | Add ESP,4 | Retain physical operation omitted from saved listing |
| `0071C4E8` | EAX = ESI | Return actual current ESI, including after free |
| `0071C4EA` | Pop ESI | Read actual current saved word |
| `0071C4EB` | RET4 | Source RET0C consumes flags plus two references |

Two Source pushes are added before the cleanup call. The Source text has
thirteen instructions; instruction encoding, actual import selection,
relocations, symbol/section extent and absence of compiler additions remain
the integrator's compiled review. No numerical emitted size is claimed here.

## Stack placement and aliases

Let `S` be Source entry ESP. ECX is the receiver and the incoming EDX value is
unused placement. `[S]` is the return word, `[S+4]` is the flags DWORD,
`[S+8]` is the actual F publication address and `[S+C]` is the actual M
publication address. After pushing ESI, ESP is `S-4`. The first
`PUSH [ESP+10h]` reads M at `S+C`; after that push the second identical
operand reads F at `S+8`. The child enters at `S-10h` with F/M argument words
at `S-Ch`/`S-8`. Its admitted Source `RET8` returns this wrapper to `S-4`.
The late `TEST [ESP+8],1` therefore reads the current byte at `S+4`.

If selected, the free argument occupies `S-8`, its call enters at `S-Ch`,
and normal cdecl return followed by `ADD ESP,4` restores ESP to `S-4`.
`MOV EAX,ESI` then precedes `POP ESI` from `S-4`. Source `RET0C` leaves ESP
at `S+10h`; Native `RET4` leaves it at `S+8`. The Original zero-argument
cleanup call enters at `S-8`, while the new Source child enters eight bytes
lower. Within the admitted Source child its selected getter enters at
outer `S-28h`; deeper provider frames are outside this wrapper's claim.

The caller supplies genuine, stable publication cells. Their values may
change during the child's admitted getter. This wrapper copies only their
addresses onto the child stack; it does not snapshot publication values or
create private cells. Preserve the outer address words through the two
loads, the child copies through their selected loads, and the admitted
child's saved-register/return backing required by its contract. Added words,
return locations and deeper child frames change possible stack aliases;
no unrestricted alias equivalence to Native placement is asserted.

The flags word is deliberately not captured before cleanup. Its current
low bit after child return controls free, including mutation through valid
aliases. Current ESI after the cleanup is used for the free argument;
current ESI after free is returned. The final POP reads the current saved
word, which may differ under aliases. There is no independent entry-register
snapshot or blanket claim that callee labels prove register preservation.
Normal callable-child stack balance and nonvolatile-register contracts remain
required. The raw cleanup's receiver, parent/list/neighbor and optional
Win32 section/depth backing requirements also remain required.

On the no-free path the last owned flag writer is TEST: CF/OF/SF are zero,
ZF/PF are one and AF is undefined. On the free path it is ADD ESP,4, whose
flags depend on actual ESP. MOV/POP/RET preserve those flags. The wrapper
does not restore ECX or EDX, has no owned writes to EBX/EDI/EBP, and makes no
whole-call DF/x87/SIMD claim. EAX is actual current ESI; a freed address is
historical pointer bits, never proof of object lifetime.

## Actual callable free and remaining boundaries

`<cstdlib>` supplies the real CRT declaration. The selected compiler header
imports C `free` from `<stdlib.h>`; the selected UCRT `corecrt_malloc.h`
declares `_ACRTIMP void __cdecl free(void*)`. For the integrator's current
Release `/MD` configuration, the UCRT `_ACRTIMP` conditional provides the
dllimport declaration. The report pins the actual compiler/SDK headers and
the selected build configuration. The candidate follows the already admitted
`native_allocator_base_cleanup.cpp` use of inline-assembly `call free`.
It neither invents a Runtime provider nor casts an Original address.

The prior allocator admission's linked image selected
`api-ms-win-crt-heap-l1-1-0.dll!free`, with `__imp__free` in its object. That
is historical dependency evidence only. This candidate's emitted operand,
chosen import and build result have not been inspected. Native `00BF65AC`
retains its correct `_free` library name; its actual implementation,
allocation domain and throw/failure policy are unopened. If the late flag
selects free, the actual current pointer must be eligible for the current
Source CRT's free. Matching Native allocation provenance is not established.

There is no owned EH registration, cleanup guard, exception translation or
`noexcept` promise. A child failure does not become a synthetic success or
fall through to the later operations. Native failure/unwind equivalence is
not established. No receiver class, allocation, owner, callable vtable,
profile-to-function binding, `00874F00` consumer, production dispatch,
fixture execution or retained application path is added. The existing
pure-virtual task facade is unchanged.

The machine-readable report contains current input pins, selected Native
byte replay, exact instruction mapping and explicit Source stack deltas.
Validation here is bounded read-only evidence inspection and Source-text
review plus `git diff --check`; no CMake, ledger, Ghidra, build, test or probe
mutation is performed. The integrator owns registration, compiled review,
normal checks, any annotation and any credit admission.
