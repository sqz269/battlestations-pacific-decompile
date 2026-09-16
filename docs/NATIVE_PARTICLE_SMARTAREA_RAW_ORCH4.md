# Raw SmartArea emitter parser B02210

`load_native_smartarea_emitter_definition_00b02210` reconstructs the complete
`00B02210..00B02B24` body: 2325 bytes, 734 instructions, and 90 direct call
sites. It receives ECX=actual 90h emitter definition and one stacked actual
TextBuffer, returns AL=1 on normal completion, and ends in `RET 4` at B02B22.
Names and field interpretations remain descriptive hypotheses.

The report `reports/native_particle_smartarea_raw_orch4.json` preserves the
previous name, reconstruction and Ghidra comment, full byte hashes, every
native call site, every x87 instruction, and all 15 exception actions. All
2666 reviewed bytes match the verified BSP Ghidra program and installed PE.

## Parsing and mutation order

The parser seeks an opening brace with normalized line reader AF5740, then
unconditionally reads once more even if that initial search reached EOF.
It processes nonempty lines until a closing brace or EOF. Both normal endings
return true. There is no transaction or reset of the destination definition.

For `Param`, token 0 is returned before suffix 1 is passed to the concrete
raw flag parser AFA650. If the flag parser declines, token 1 becomes the
parameter name. Suffix 2/token 0 is converted through `atof`, and the x87
result is stored to the local binary32 scalar. The scalar token and suffix
are returned before constructing the actual 10h parameter builder.

The builder gets two zero endpoints. Suffix 2/suffix 1 is passed to AFC470;
its boolean result is ignored. Both suffixes are returned before AF9D00 is
called with the original binary32 scalar, including its native load/store
spill. AF9D00 handles the common emitter parameters with the actual runtime
pool. Otherwise these four case-insensitive names select derived fields:

| Name | Definition field | AFBF60 call site | Scale schedule |
| --- | --- | --- | --- |
| EmittedSpeed | +80h | B0256D | FLD32 / FMUL64 D7A358 / FSTP32 |
| Radius | +84h | B025F3 | FLD32 / FMUL64 D7A358 / FSTP32 |
| RadiusSpeed | +88h | B0262B | FLD32 / FMUL64 D7A358 / FSTP32 |
| RadiusAngle | +8Ch | B0268B | FLD32 / FMUL64 D7A358 / FSTP32 |

Conversion runs before the current double scale is loaded. The multiplier is
stored into the returned actual parameter before its pointer is written into
the definition. Existing field values are overwritten without release. There
is no null conversion guard. Unknown parameter names still construct and
parse the builder, then destroy it without publishing a derived parameter.

The raw context contains one explicit `child_builder_kind`; each child factory
receives that distinct incoming residue. The acquired parser's own explicit
kind initializes only its own builder's unwritten +Ch slot, which persists
across parameter lines. A parent builder mutation is not passed as a child's
native stack residue.

## Nested concrete factories

Non-Param lines test `Emitter`, then independently retokenize and test
`Particle`, including after a successful Emitter branch. Both branches
create native eight-byte name and kind headers from persistent four-byte
pooled token headers. Each inlined construction zeros the header, measures
the captured C string, resizes with preservation, then copies current
length+1 bytes to the current destination pointer if nonnull.

Emitter calls AF9FB0 at B027F5 with kind/name headers, the **current** parent
word at +10h, the parent address as word70, and the original TextBuffer.
Particle calls B00CE0 at B029F2 with kind/name, parent and TextBuffer. The
actual raw factory bodies execute; there is no callback or placeholder body.
Required context pointers are checked only at their reached factory calls.

After each child returns, temporary cleanup runs in reverse order: native
kind string, pooled kind token, native name string, pooled name token. Only
then is the child published through AF9F00 or AF9F20, preserving their current
count/field behavior and lack of a new reference. A cleanup failure leaves
the already created child retained in the factory frame and unappended.

Persistent native headers precede the retained optional factory frames inside
the acquired pimpl. Complete children can be replaced on a later line. Failed
children remain available through typed accessors, with no replay or implicit
rollback. Their existing downstream obligations remain in force; a failed
VFS child currently requires retaining its graph for process lifetime.

## Exception map and normal cleanup

Handler CBB588 loads function info DF35B8; the 15-entry map is DF35DC.

| State | Next | Action | Native local | Cleanup |
| ---: | ---: | --- | --- | --- |
| 0 | -1 | CBB510 | -7Ch line | AEE2A0 |
| 1 | 0 | CBB518 | -68h flag suffix | AEE2A0 |
| 2 | 0 | CBB520 | -78h parameter name | AEE2A0 |
| 3 | 2 | CBB528 | -60h scalar suffix | AEE2A0 |
| 4 | 2 | CBB530 | -1Ch builder | AF4110 |
| 5 | 4 | CBB538 | -58h curve outer suffix | AEE2A0 |
| 6 | 5 | CBB540 | -5Ch curve inner suffix | AEE2A0 |
| 7 | 0 | CBB548 | -4Ch emitter name token | AEE2A0 |
| 8 | 7 | CBB550 | -34h emitter name8 | 41DD20 |
| 9 | 8 | CBB558 | -50h emitter kind token | AEE2A0 |
| 10 | 9 | CBB560 | -3Ch emitter kind8 | 41DD20 |
| 11 | 0 | CBB568 | -40h particle name token | AEE2A0 |
| 12 | 11 | CBB570 | -24h particle name8 | 41DD20 |
| 13 | 12 | CBB578 | -44h particle kind token | AEE2A0 |
| 14 | 13 | CBB580 | -2Ch particle kind8 | 41DD20 |

Each state becomes active only after its producing operation completes.
Native keyword/scalar-token temporaries have no additional state. Normal
cleanup disarms the corresponding state before returning a captured pointer.
For base/EmittedSpeed/Radius/RadiusSpeed, the normal builder cleanup is an
inlined raw free and clears only +0/+4/+8; RadiusAngle/unknown uses AF4110.
Both leave kind+Ch untouched. Final line release first sets state=-1 and
leaves the local header stale, exactly as the original normal path does.

The source catch records the failing native site and state, walks only the
reachable native actions, then rethrows. A second exception while executing
an unwind action terminates. It adds no cleanup for native unowned temporaries,
no child owner release and no destructor-based rollback.

## Validation and limits

Strict MSVC Win32 `/std:c++20 /permissive- /W4 /WX /EHsc /c` passes, and
the integrated concrete cycle passes the full build plus all three existing
CTest checks. The shared copied-original fixture passes 18 complete-body
pairs across the three shapes: all common/derived parameters in four x87
rounding modes, nested three-emitter-plus-Sprite trees, and EOF without braces.
Six pairs exercise SmartArea directly. Nested children use genuine raw
factories and parsers; snapshots include only natively established child
bytes. The final fixture links the integrated core library.

The shared fixture also checks a source-only missing-factory cleanup and
replay boundary. The original FH3 handlers are not executed, and arbitrary
pool-getter failure combinations are not exhaustively exercised. The primary
integrator owns Ghidra annotation updates. These source interfaces do not
establish native calling-convention/FH3/SEH, unrestricted fault/CRT,
concurrent-mutation or gameplay identity.
