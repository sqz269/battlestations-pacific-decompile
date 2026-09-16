# Native particle Layer reader

## Body, ABI and evidence

`read_native_particle_layer_00afad00` reconstructs all 1284 bytes at
AFAD00..AFB203. Native ECX supplies the actual 40h Layer; its one stack
argument supplies the actual 1Ch text buffer. RET4 returns AL=1 on every
normal exit, including EOF before the opening brace. The source writes only
Layer+20 (MaxParticles), byte+24 (Sort), +28 (RenderPriority), and binary32
+30/+34/+38/+3C (LODFadeIn/InWidth/Out/OutWidth). Other bytes remain untouched.

The report preserves full installed-PE/live-Ghidra bytes and SHA256 for eleven
spans: the complete body, three repaired tails, six unwind thunks plus compiler
handler, FuncInfo/unwind map, and literal storage. The target is the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The body has 414 decoded
instructions and 53 direct calls. The repaired live listing has 413 instructions;
the jumped-over three-byte LEA at AFADCD..CF is intentionally not repaired.
Prior documentation/comments are retained in the report. `FUN_00afad00` is the
old name; the proposed descriptive name is a hypothesis.

The parent repaired returning-free fallthrough at AFB013..B042 (48 bytes),
AFB06F..B098 (42), and AFB0EB..B0FE (20), after independent full-byte checks.
This worker made no Ghidra mutations. The repairs and old flow metadata are
recorded separately by the parent in the Layer/builder flow reports.

## Genuine dependencies and parsing

The API borrows `NativeParticleParameterBuilderRawContext`, the actual Layer
and text buffer, and the application's same F8C2C8 scratch. The context supplies
the same `NativeStringRawPoolContext` for AF5740, AEE3C0, AF44C0 and the genuine
AFBED0/AFC360/AFC470/AF4110 builder providers. No alternative owner, parser,
runtime parameter pool, private scratch, host string adapter or callback
facade is introduced. Current CRT `_stricmp`, `atol` and `atof` remain explicit
provider boundaries; native literal pointer identity is not preserved.

The reader searches for an exact opening-brace line. It then always performs
one body read, even if the search ended at EOF, and consumes lines until an
exact closing brace or EOF. Empty lines and lines whose token0 is not `Param`
are ignored. There is no field-count check. Token1 becomes the parameter name.

Sort takes token2 and writes whether signed `atol` is greater than zero. Every
other name, including unknown names, takes suffix2 and converts its token0
with `atof`, explicitly discarding ST0. It constructs a real temporary builder,
appends two zero-valued endpoints without initializing kind+C, takes a fresh
suffix2 followed by suffix1, and parses that text. The parser's boolean result
is ignored; its mutations remain. The first builder value supplies recognized
fields. The initial numeric prefix is not used as a multiplier. No conversion
to a runtime parameter object occurs.

MaxParticles and RenderPriority use the genuine AFC1C0 CVTTSS2SI first-value
helper, including its truncation/invalid-conversion behavior. All four LOD
fields call AFC1B0 and preserve the native FSTP-to-binary32-local, FLD-local,
FSTP-destination schedule. For In/InWidth, records are captured between the
reload and destination store, then freed. Out/OutWidth store before invoking
the real builder destructor. MaxParticles captures records after its store;
RenderPriority captures them before its store. Both capture the current name
before zeroing builder+0/+4/+8. These order differences are kept explicitly.

## Ownership and exception schedule

FuncInfo DF303C points to the six-state unwind map at DF3060. The CBB110
compiler handler tail is audited as raw bytes; no function ownership is
manufactured for it. The six existing unwind thunks contribute six verified
tail-call rows in addition to the 53 body calls.

| State | Next | Owned object | Unwind thunk |
| --- | --- | --- | --- |
| 0 | -1 | line | CBB0E0 |
| 1 | 0 | name | CBB0E8 |
| 2 | 1 | initial suffix | CBB0F0 |
| 3 | 1 | builder | CBB0F8 |
| 4 | 3 | outer suffix | CBB100 |
| 5 | 4 | inner suffix | CBB108 |

Keyword and numeric token temporaries have no added unwind ownership. Normal
text cleanup captures the pointer before lowering the state; it returns that
captured allocation through the current 419CC0 getter/BD1510 provider, then
clears the corresponding header at the recovered point. Normal builder free
and zeroing retain state3 until the name state is dropped to0. Final line
cleanup drops to state-1 before return and does not clear the line header.
Normal raw getter failures propagate; a second exception during true unwind
terminates. No validation, bounds policy, rollback or additional cleanup is
introduced for malformed input, short tokens, EOF or failed parsing.

## Verification boundary

Strict Win32 `scripts/build.ps1` and all three existing CTests pass after the
eight seed-byte checks. The focused complete-original/source comparison passes
seven comparisons over four actual-buffer scripts: all seven properties,
Const/Linear/Hermite, ignored unknown names and one failed parse, signed Sort,
integer truncation/overflow, brace/EOF effects, all Layer bytes, cursor and
pool counters, and real singleton-manager drain. The precision input runs
under all four x87 rounding modes; downward/upward LODFadeIn bits are
3F800000/3F800001. The final probe process exits zero.

The fixture uses the complete body with internal branches retained and 53
calls redirected to the same genuine raw-pool/text/builder providers from
this worktree's built library. Eleven literal pointers are relocated to their
verified bytes. Its ten builder constants have independent PE/live hashes
and compile-time bit checks. It uses the explicit normal sqrt bypass and
does not exercise the CRT exception handler. It does not execute native FH3,
allocation/getter failure, hardware faults or secondary exceptions. Source
and current CRT agreement is not exact installed-CRT identity. The new C++
interface does not claim native stack-slot alias identity, drop-in register
ABI compatibility or gameplay validation.
