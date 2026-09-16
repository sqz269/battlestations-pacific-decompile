# Raw Sprite and Floating particle definition parsers

This packet adds genuine raw overloads of `B08AC0` (Sprite) and `B07D60`
(Floating). They compose the existing raw pooled-text, parameter-builder,
runtime-parameter, common-property and common-parameter bodies. They use the
application's actual definition and TextBuffer, same string publication cells,
shared `F8C2C8` scratch and same `F8D344` parameter pool. They introduce no
replacement owner, parser callback or fallback success path.

The older `NativeParticleTypeLoadingBindings` overloads remain available. Their
typed property-provider interface is separate from these raw overloads.

## Coverage and ABI

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` was
verified by standard live queries. Both complete PE bodies equal live bytes.
No Ghidra mutation or listing repair was needed by this worker.

| Entry | Complete body | Bytes / instructions | Coverage |
|---|---|---|---|
| B08AC0 | B08AC0..B08F55 | 1174 / 378 | complete |
| B07D60 | B07D60..B08202 | 1187 / 381 | complete |

Both native entries receive ECX actual definition and one stacked actual
TextBuffer; normal return is AL true and `RET4`. Sprite captures TextBuffer in
EBP and reuses the stack argument as flag/percentage storage. Floating reloads
its original stacked TextBuffer during the read loop and stores percentage in
its separate local `-38h`. The source uses explicit stable storage; native
private-stack aliasing and general-register identity are outside its new ABI.

The report records all 93 direct calls with exact containing function, call
site and callee; `verify_report_calls.py` verifies them against the live listing.
Existing saved names are retained as descriptive hypotheses. Previous name and
reconstruction records are preserved in the report before ledger updates.

## Genuine provider contracts

| Body | Existing source composition |
|---|---|
| AF5740 | Raw TextBuffer normalization, current cursor and actual shared scratch |
| AEE3C0 / AF44C0 | Actual4h pooled-token/suffix constructors, same raw string publications |
| B015C0 | `NativeParticleTypePropertyRawContext` and caller-retained acquired child |
| AFBED0 / AFC360 / AFC470 | Raw 10h builder construction, endpoint insertion and partial parser mutation |
| AFBF60 | Actual F8D344 pool and fixed CRT segment storage through runtime raw context |
| B00980 | Raw common parameter properties and original float stores |
| B08870 | Raw Sprite Size publication and curve-bound kernel |
| 419CC0 / BD1510 / AEE2A0 | Current actual string-pool lookup, sized return, header clear |
| BF7FBF / BFA66C / BF6989 | Current CRT case comparison, atof and free boundaries |

`NativeParticleSpriteFloatingRawContext` borrows those contexts. Their string,
runtime, numeric and CRT domains must be the application's same domains.
Scale pointer members remain live and are captured after runtime conversion,
at their original use point.

## Producer layout and publication

No new definition layout is inferred. Existing producers supply the actual
definition. Sprite alone clears byte `+64h` at entry; Floating preserves it.
Common properties run first. Unrecognized common properties are parsed as
parameter name, percentage and builder text, with ignored builder-parser AL.
Common parameters run next. Remaining `InitialRotation`, `RotationSpeed` and
`Size` publish actual runtime parameters at `+80h`, `+84h` and `+88h`.
Sprite Size invokes `B08870`, which also stores its bound at `+8Ch`; Floating
Size performs only its direct `+88h` publication. Existing parameters are not
returned or rolled back by these parser bodies.

The same builder storage is reused across lines. AFBED0 clears only its
pointer/count/capacity and leaves kind `+Ch` unwritten. The acquired-frame
constructor requires an explicit initial kind residue; subsequent calls keep
the value last written by the actual parser. Unknown curve syntax does not
silently receive a new kind. Default endpoints are appended before parsing.

The parser scans until an opening brace or EOF, then performs another read.
It returns true at a closing brace or ordinary EOF; it adds no brace validation,
success callback, replay or rollback policy.

## Floating-point and stack schedule

`atof` feeds one x87 FSTP32 at `B08C60` / `B07EF9`. A later FLD32/FSTP32
round trip supplies the stacked percentage to `B00980` at `B08D83..B08D88`
and `B08022..B08027`. Derived parameter paths call AFBF60 first, then load
the saved percentage, multiply by CURRENT double `D7A358` and FSTP32 into
the actual runtime parameter. The new raw overloads preserve these operations
with small MSVC Win32 assembly entries. They do not round the scale to float.

Native stack cleanup confirms argument counts: comparisons use `ADD ESP,8`;
atof/free use `ADD ESP,4`; token/suffix methods consume output/index with
`RET8`; builder endpoint insertion consumes two floats; B00980 consumes
name, builder and percentage (`RET0Ch`); parser and Size consume one pointer.

## Retained storage and exact unwind map

The caller retains `NativeParticleSpriteFloatingRawAcquired`. Its actual4h
header slots and 10h builder precede an optional property-acquired child, so
the storage remains alive through child teardown. A completed property frame
can be replaced without allocation. A failure stops the parser and retains
its child, including any failed texture/cache frames, until their existing
obligations are resolved externally. No destructor performs rollback.

Both native FH3 maps have the same transitions. Floating map `DF3AB8` is
referenced by `CBB928`; Sprite map `DF3B6C` by `CBB9B8`. Both 56-byte maps
were compared against live Ghidra bytes.

| State | Previous | Owned cleanup | Floating / Sprite action |
|---|---|---|---|
| 0 | -1 | line header | CBB8F0 / CBB980 |
| 1 | 0 | common-property suffix | CBB8F8 / CBB988 |
| 2 | 0 | parameter name | CBB900 / CBB990 |
| 3 | 2 | percentage suffix | CBB908 / CBB998 |
| 4 | 2 | builder through AF4110 | CBB910 / CBB9A0 |
| 5 | 4 | curve suffix | CBB918 / CBB9A8 |
| 6 | 5 | curve text | CBB920 / CBB9B0 |

The command token and percentage token are deliberately unarmed. Normal
captured-pointer returns happen with the parent state already restored, then
clear the corresponding header after return. A failing percentage-token
return does not acquire invented token cleanup. Builder-construction failure
is still in state 2; constructor-owned cleanup remains inside AFBED0.

Successful common parameters and InitialRotation free builder storage, capture
the name, clear builder pointer/count/capacity, set state 0 and return the
captured name. RotationSpeed/Size/unknown paths free and clear the builder,
then invoke AEE2A0 on the name. All leave builder kind unchanged. Final normal
line cleanup disarms state 0 before return and leaves its header stale. A
second C++ exception during equivalent unwind terminates.

## Validation and boundaries

The full Release MSVC Win32 build and all three existing CTests passed.
`verify_report_calls.py` checked 93 rows with zero failures. Strict standalone
`/W4 /WX /fp:strict` compilation passed, as did eight copied-original/source
parser pairs and two source failure cases. The ignored probe relocates both
complete original parser bodies to
the same genuine reconstructed providers used by source. Original exception
handlers are guarded; these comparisons cover normal control/data/x87 flow,
not original FH3 transport or original child bodies/CRT execution.

The fixture compares Const/Linear/Hermite output, an explicitly seeded builder
kind on unknown syntax, percentage conversion, Sprite bound/Floating size,
normalized text cursor and pool accounting across all four x87 rounding modes.
Source-only missing-property-provider failures check retained property frames
and line/suffix cleanup. They do not prove a failing real texture-cache load.

Original register/stack ABI, private frame aliases, native EH/SEH, unrestricted
faults, CRT identity and concurrent mutation remain outside this interface.
No installed-game, `bsp_game` runtime-log, rendering or gameplay claim is made.
