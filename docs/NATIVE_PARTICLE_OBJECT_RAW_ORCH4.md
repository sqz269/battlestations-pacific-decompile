# Raw Object particle definition parser

AF8BD0 now has a complete raw overload that composes the existing raw text,
property, builder, runtime-parameter and Object resource bodies. It operates on
the application's actual 98h Object definition and TextBuffer. The earlier
typed-host overload remains separate in `native_particle_object_tracer_loading`.

## Evidence and coverage

Project `C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe` were
checked through standard live queries. This worker made no Ghidra mutations.

| Entry | Complete span | Bytes / instructions | Coverage |
|---|---|---|---|
| AF8BD0 | AF8BD0..AF9094 inclusive | 1221 / 400 | complete |

The full installed PE body equals live Ghidra bytes, SHA256
`180671c6f9dfafcb849630ab66fe660d9e3d7daf0a229e257a806925f7f87a7f`.
The live flow query found zero listing gaps. All 48 direct call rows are
recorded and verified; the one indirect call is AF8D75, through current `+20h`.
The report also preserves the previous name and reconstruction ledger rows.

Native inputs are ECX definition and one stacked TextBuffer. The normal result
is AL true, `RET4` at AF9051. The scan consumes input until an opening brace or
EOF, then performs another read; it returns true on closing brace or ordinary
EOF without imposing additional validation. It clears definition byte `+64h`
before reading. Native EBP preserves the definition; the loop reloads its
original stacked TextBuffer. Private stack/register identity is not the new ABI.

## Raw provider composition

| Native body | Source contract |
|---|---|
| AF5740 | Actual TextBuffer cursor, pooled line and shared F8C2C8 scratch |
| AEE3C0 / AF44C0 | Actual4h pooled token/suffix, same current string publications |
| B015C0 | Raw common properties with caller-retained property/texture/cache child |
| AF9660 | Raw actual Object models, same VFS/manager/factory alias/cache/references |
| AFBED0 / AFC360 / AFC470 / AF4110 | Raw10h builder, endpoints, parse and unwind destruction |
| B00980 / AFBF60 | Raw common parameters and current actual F8D344 parameter pool |
| AF80F0 | Existing raw Size setter: publish `+84h`, cache value at zero in `+88h` |
| 419CC0 / BD1510 / AEE2A0 | Current string-pool lookup, sized captured return and header clear |
| BF7FBF / BFA66C / BF6989 | Current CRT case comparison, atof and free |

The context borrows those same domains. No NativeModelOwner, replacement
resource graph, parser callback or fallback result is introduced. The current
factory alias and canonical resource-container references belong to the raw
AF9660 child. Their existing provider boundaries remain applicable.

Common properties run first. An unhandled `Model` reads token2, captures its
data pointer, captures the current definition profile, then reads current slot
`+20h`. B00CE0's final Object profile is D5DB00. This context requires an
explicit **nine-word readable D5DB00 view through +20h**; it does not read past
the common-property context's existing five-word promise. It dispatches the
captured AF9660 target through a retained raw model child. Unknown reached
profile/target is an explicit source boundary. That boundary owns and cleans
the token, rather than attempting the native unknown call or hardware fault.

Other unhandled properties parse token1 as name, suffix2/token0 as percentage,
and suffix2/suffix1 as curve text. Builder-parser AL is ignored, as native.
B00980 runs first; remaining `RotationSpeed` publishes `+80h`, while `Size`
calls AF80F0. The parser does not return overwritten runtime parameters and
does not roll back earlier publications when a later operation fails.

AFBED0 clears pointer/count/capacity and leaves builder kind `+Ch` untouched.
The acquired constructor requires explicit incoming kind residue, and later
lines reuse the current kind. The same builder storage survives the whole
invocation. Normal cleanup clears pointer/count/capacity, retaining kind.

## Floating point and argument schedule

AF8DF9 atof returns ST0; AF8DFE stores float32. AF8F22..AF8F27 performs a
second FLD32/FSTP32 round trip for the B00980 stack argument. Derived parameter
paths call AFBF60 **before** loading the saved percentage and CURRENT double
D7A358, then FMUL64/FSTP32 at AF8FB7..AF8FC1 or AF9077..AF9084. Small MSVC
Win32 assembly kernels preserve these operations and do not round the scale
to float. AF80F0 supplies its existing native value-at-zero curve kernel.

Stack cleanup confirms two-argument CRT comparisons (`ADD ESP,8`), one-argument
atof/free (`ADD ESP,4`), output/index token and suffix calls (`RET8`), endpoint
insertion with two floats (`RET8`), common name/builder/percentage (`RET0Ch`),
and one-pointer property/model/Size calls (`RET4`).

## Complete exception metadata and retained storage

The report checks the complete ten-byte handler CBAEB0, all 36 FuncInfo bytes
at DF2D28, all 64 unwind-map bytes at DF2D4C and every eight-byte action.
FuncInfo magic is `19930522`, maxState is 8, try-block count/map and IP-map
count/map are zero, ESTypeList is zero, and EHFlags is **1**. The handler loads
DF2D28 then jumps to BF6B43. These are native C++ EH scope facts; arbitrary
hardware faults and the original FH3 transport are not validated by source.

| State | Previous | Header / storage | Action |
|---|---|---|---|
| 0 | -1 | line at native `-44h` | CBAE70 -> AEE2A0 |
| 1 | 0 | common-property suffix `-34h` | CBAE78 -> AEE2A0 |
| 2 | 0 | parameter name `-40h` | CBAE80 -> AEE2A0 |
| 3 | 2 | Model filename `-30h` | CBAE88 -> AEE2A0 |
| 4 | 2 | percentage suffix `-28h` | CBAE90 -> AEE2A0 |
| 5 | 2 | builder `-1Ch` | CBAE98 -> AF4110 |
| 6 | 5 | curve suffix `-20h` | CBAEA0 -> AEE2A0 |
| 7 | 6 | curve text `-24h` | CBAEA8 -> AEE2A0 |

Command and percentage tokens have no unwind state. Normal captured returns
restore the parent state before returning bytes, then clear the native header
after return. Common-parameter completion frees the builder, captures the name,
clears builder words, restores state0 and returns that captured name. Derived
or unrecognized parameters instead use AEE2A0 after restoring state0. Final
line release runs in state-1 and leaves its header stale, matching the body.

`NativeParticleObjectRawAcquired` is caller-retained and immovable. Its headers
and builder precede the optional property and model children. Completed child
frames can be replaced; failure stops the parser and retains the failed child
and all referenced storage. There is no destructor rollback or replay.
The existing **failed VFS resolution frame has no discharge API and terminates
on destruction**. A failure reaching it therefore requires retaining the frame
and referenced contexts for process life. This packet does not invent recovery.

## Validation and limits

- Full `scripts/build.ps1`: Release MSVC Win32 build and existing CTests **3/3**.
- Eight native seeds match; report call check: **48 rows, zero failures**.
- Local probe compiles with `/W4 /WX /EHsc /MD /O2 /fp:strict` and embedded manifest.
- Four copied-original/source AF8BD0 pairs, one per x87 rounding mode, match
  parameter words/segments, defined Object fields, normalized cursor, final
  builder kind and full pooled-byte accounting. Input includes repeated Model
  lines with real empty-VFS resolution, Const/Linear/Hermite curves and unknown
  syntax with explicit kind residue. Model uses an ABI adapter in current +20.
- Three source failure cases cover missing Texture provider, unsupported current
  Model target and a reached unsupported VFS visitor target inside the actual
  raw Model/VFS child. They preserve earlier parameter publication, unwind
  owned headers, retain child frames and reject replay. The last case retains
  its failed VFS frame and all contexts through explicit process exit.

The copied original **parser body** executes; its child calls use the same
existing raw source providers through ABI adapters. It does not execute
original child bodies or native FH3 exceptions. Model success is bounded to
empty VFS/no loaded candidates: successful model acquisition, cache miss and
zero-reference terminals are not newly exercised here. General-register,
private-stack, exact CRT/fault identity and x87 status/EFLAGS are not compared.
No permanent tests were added. No original ABI or gameplay claim is made.
