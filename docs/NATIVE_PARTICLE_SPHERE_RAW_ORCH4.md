# Raw Sphere emitter definition parser

`load_native_sphere_emitter_definition_00b02fd0` reconstructs the complete
`B02FD0..B03894` body: 2,245 bytes and 715 instructions. The original receives
the actual 8Ch Sphere definition in ECX and the actual TextBuffer on the stack,
returns AL true at normal EOF or a closing brace, and uses RET4. This is a new
C++ interface, not an original ABI replacement. The descriptive Ghidra name
remains `BSP_SphereEmissionDefinition_Load`.

## Concrete dependencies and ownership

The parser uses the actual raw pooled strings, TextBuffer, shared F8C2C8 scratch,
parameter builder, and F8D344 parameter pool. AF9D00 and AFA650 are the existing
raw common-parameter and flag implementations. The Sphere properties are
EmittedSpeed at +80, InnerRadius at +84, and OuterRadius at +88. Conversion calls
AFBF60 first, then executes FLD32/FMUL64/FSTP32 against the current percentage
cell. Parsing also preserves the atof result's first FSTP32 and the separate
FLD32/FSTP32 stack-argument store before AF9D00.

Nested Emitter and Particle lines construct the actual 8h name/kind headers and
call the concrete AF9FB0 and B00CE0 factories. AF9F00/AF9F20 publish only after
normal temporary cleanup. Emitter captures the current parent+10 before child
frame allocation. The Particle keyword is extracted again after Emitter loading;
it is a separate native check. Unknown Param lines finish parameter cleanup and
advance to the next line, unlike Cone's additional keyword checks.

The contexts hold typed factory pointers to permit the recursive emitter graph.
The acquired frame retains the native local storage before optional concrete
factory children. Completed children can be replaced. A failed child remains
with its caller and borrowed domains until its existing obligations resolve.
There is no destructor rollback or replay. Existing failed VFS frames currently
have no discharge API and must remain alive for the process lifetime. The own
builder's unwritten kind is supplied to Acquired; a distinct explicit context
word supplies the nested parser's unwritten kind. Neither is invented as zero by
production code.

## Cleanup evidence

Handler CBB658 selects FuncInfo DF36AC, magic 19930522, maxState 15, unwind map
DF36D0, no try/IP/ESType maps, and EHFlags 1. Complete action tails CBB5E0..CBB657
target AEE2A0, 41DD20, or AF4110. State predecessors are
`[-1,0,0,2,2,4,5,0,7,8,9,0,11,12,13]`.

States own line, flag suffix, parameter name, scalar suffix, builder, curve
suffix, curve, emitter name token/name8h/kind token/kind8h, and corresponding
particle temporaries in that order. Command/scalar tokens are not armed. Normal
cleanup disarms before returns, preserves the common/EmittedSpeed captured-name
schedule, and leaves the final line and 8h headers stale. Overwritten parameters
are not returned. Exceptions preserve prior publication; secondary unwind
exceptions terminate. This recovers source cleanup obligations without claiming
native FH3, SEH, fault, or CRT exception identity.

## Validation and limits

All 2,245 body bytes, 130 action/handler bytes, and 156 FuncInfo/map bytes match
the installed PE and live saved program. The report enumerates every direct
call and EH tail. The earlier host-composition ledger record remains under its
distinct symbol; the raw implementation adds a separate record.

One ignored shared probe compares all three complete original emitter parser
bodies with source: 18 pairs cover every common and shape-specific property,
four x87 rounding modes, actual pool allocation/returns, parent owner bytes,
curve payloads, text cursor, nested Sphere/Cone/SmartArea plus Sprite, and EOF
without an opening brace. A source-only missing-factory case verifies state10
cleanup, earlier parameter publication, and rejected replay. A probe-only access
violation was traced to observing native-uninitialized child +30/+38 fields;
the final nested input establishes those fields, and child snapshots compare
only native-defined bytes. Production constructors remain unchanged.

Copied parser bodies bridge 270 direct calls to genuine reconstructed children
and current CRT functions; this does not execute original child bodies or native
EH handlers. Parser-local fixtures do not establish full resource loading,
application context installation, unknown particle-kind record dispatch,
original binary ABI, real asset loading, or gameplay. Final integrated build
and probe receipts are recorded in `reports/native_particle_sphere_raw_orch4.json`.
