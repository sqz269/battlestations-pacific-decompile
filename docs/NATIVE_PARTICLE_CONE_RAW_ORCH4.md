# Cone emitter parser through the actual raw domains

`load_native_cone_emitter_definition_00b03ec0` reconstructs the complete
`B03EC0..B04823` body: 2,404 bytes, 755 instructions and 94 direct calls.
The existing Ghidra name `BSP_ConeEmissionDefinition_Load` remains a descriptive
hypothesis. The original ABI is ECX actual94h definition, one stacked actual
TextBuffer, RET4 and AL=true on normal completion.

Source: `src/native_particle_cone_raw.cpp`; evidence:
`reports/native_particle_cone_raw_orch4.json`. The earlier host composition in
`native_particle_definition_loading.cpp` and its ledger records remain intact.

## Actual dependencies and lifetime

The context borrows the application's actual string publications through the
raw builder context, actual F8D344 parameter runtime pool, shared F8C2C8 text
scratch and percentage double cell D7A358. These domains must agree with the
typed emitter and particle factory contexts. No host string owner, arbitrary
factory callback, fabricated adapter or unresolved-call stub is used.

Typed factory pointers allow mutually recursive contexts to be wired after
construction. A missing factory is an explicit source boundary only when its
branch is reached. AF9FB0 invokes concrete Cone/Sphere/SmartArea parsers through
current profile+14; B00CE0 invokes genuine raw particle parsers. Emitter
profile+08 slots are record creators and are not substituted for parsers.

An acquired frame is one-shot. Its sparse native token/name/kind headers and
builder storage precede optional factory children inside the opaque Impl.
Completed children can be replaced. A failed factory and its concrete parser
child stay retained, alongside all borrowed contexts, until existing provider
obligations resolve. An Object child's failed VFS frame presently requires
process lifetime. Destruction does not perform native rollback. The parent
builder's incoming kind+C and a reached child's incoming kind are distinct
explicit inputs; neither native initialization nor propagation is invented.

## Complete control flow and numeric schedule

The opening scan reads until `{` or EOF, then always performs the next read.
The body accepts EOF and `}` as successful termination and ignores empty lines.
It constructs and returns each command token through the actual raw pool.

`Param` first calls raw AFA650. If unhandled, it obtains the name, parses the
percentage with atof and performs the original FSTP32. It constructs the raw10h
builder, appends two zero endpoints, obtains both curve suffixes and parses
them through AFC470. Parse AL is ignored. The percentage is copied through
FLD32/FSTP32 for raw AF9D00, preserving that native argument-store schedule.

If the base parser declines, comparisons preserve the original order:

| Name | Definition field | Convert call |
|---|---:|---:|
| InnerEmitSpeed | +80h | B0421D |
| OuterEmitSpeed | +84h | B042A3 |
| MaxAngle | +88h | B042DB |
| InnerDistance | +8Ch | B0433B |
| OuterDistance | +90h | B0436E |

Each branch performs actual raw AFBF60 conversion, then FLD32/FMUL64/FSTP32
using the current D7A358 double, then publishes the returned parameter pointer.
The context's current scale pointer and its double value are read after
conversion, preserving the raw context's pointer-member rebinding contract.
No overwritten parameter is reclaimed.

Base/Inner paths use the original inline free and captured-name return;
Outer/Max use inline free plus AEE2A0 current-header destruction. Distance and
unknown paths call AF4110 and AEE2A0. Unknown parameters continue to the nested
entry checks, while handled parameters proceed to the next line.

`Emitter` and `Particle` construct actual8h name/kind headers: clear both words,
strlen the captured token, call raw 41DD40 with preserve=1, then copy the current
length+1 to the current data pointer. The source uses the existing CRT's
overlapping-safe byte-copy behavior. AF9FB0 receives CURRENT parent+10, parent
and text. B00CE0 receives parent and text. Cleanup of all four temporaries
precedes AF9F00/AF9F20 publication. After an emitter entry the parser still
checks the original outer line for `Particle`, as the native body does.

## Full exception metadata

Live Ghidra identity was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Full body and metadata reads matched the installed
PE. All 10 handler bytes at CBB728, all 36 FuncInfo bytes at DF37A0, all 120
unwind-map bytes at DF37C4 and all 15 eight-byte action tails at CBB6B0..CBB727
were checked. The handler jumps to BF6B43; magic=19930522, max-state=15,
no try blocks/IP map/exception-specification list, EH flags=1.

| State | Previous | Local | Action target |
|---:|---:|---|---|
| 0 | -1 | line -7Ch | AEE2A0 |
| 1 | 0 | flag suffix -68h | AEE2A0 |
| 2 | 0 | parameter name -78h | AEE2A0 |
| 3 | 2 | percentage suffix -60h | AEE2A0 |
| 4 | 2 | builder -1Ch | AF4110 |
| 5 | 4 | curve suffix -58h | AEE2A0 |
| 6 | 5 | curve -5Ch | AEE2A0 |
| 7 | 0 | emitter name token -4Ch | AEE2A0 |
| 8 | 7 | emitter name8h -34h | 41DD20 |
| 9 | 8 | emitter kind token -50h | AEE2A0 |
| 10 | 9 | emitter kind8h -3Ch | 41DD20 |
| 11 | 0 | particle name token -40h | AEE2A0 |
| 12 | 11 | particle name8h -24h | 41DD20 |
| 13 | 12 | particle kind token -44h | AEE2A0 |
| 14 | 13 | particle kind8h -2Ch | 41DD20 |

Command and percentage tokens have no caller unwind state. Normal releases
disarm their ownership first. True unwind invokes current-header AEE2A0 or
41DD20; a second exception terminates. Normal final line return leaves its
header stale, matching the native caller. Earlier definition publications and
factory allocations survive later failures.

## Validation and limits

- Complete live/PE body and EH equality; SHA-256 values recorded in the report.
- `verify_report_calls.py`: 94 call rows checked, 0 failed.
- MSVC Win32 `/c /std:c++17 /MD /EHsc /W4 /WX /fp:strict` passes. The temporary
  include search used the particle-factory worker's stable typed header.
- Whole-cycle build and the shared copied-original/source emitter probe are
  pending integration of the concrete recursive sources; this initial commit
  deliberately has no CMake entry.

This is a new C++ interface. It does not establish the original register/stack
ABI, native FH3/SEH transport, original CRT identity, unrestricted memory-fault
behavior or game validation. No live Ghidra names/comments were modified by
this worker; the integrator serializes annotation/save work.
