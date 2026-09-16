# Raw particle resource parser

`parse_native_particle_resource_00af4ba0` reconstructs the complete 2,574-byte
body at AF4BA0..AF55AD: native ECX actual 90h resource, stacked actual 1Ch
TextBuffer, RET4, AL result. The descriptive name is a hypothesis.

## Concrete storage and dependencies

The context borrows the application's existing raw builder/string publications,
recursive emitter factory, shared F8C2C8 text scratch and actual mutable
0109EEA4 conversion feature cell. All reached emitter/type contexts must use
the same actual string, parameter pool, owner and provider domains. No parser,
factory, construction, preparation or resource callback replaces these bodies.

The acquired invocation retains the native EBP-8Ch..-20h words, followed by the
EBP-1Ch builder, before its optional typed emitter-factory child. Its explicit
incoming builder kind supplies AFBED0's unwritten +Ch slot; the context supplies
a separate nested-parser builder kind. A completed child can be replaced on a
later Emitter line. Failed children and borrowed contexts must remain alive
until their provider obligations resolve; failed VFS frames currently require
process lifetime. There is no replay or destructor rollback.

## Parsing and publication

The parser rewinds the buffer, rejects an empty first read or a first token
other than `ParticleSystem`, and searches for an exact opening brace. A valid
header reaches bounds reading and recursive preparation even at EOF. Empty
and unknown lines are consumed without rollback.

Direct properties write the native offsets, with signed `atol > 0` for flags.
`PerPixelNormal` is first written, its token is returned, and only then is the
flag cleared. ColorBurn stores atof's ST0 directly; MaxParticleSize and the
two LOD distances retain the native binary32 spill/reload. Other parameters
still perform the discarded initial atof and genuine builder parse even when
their names are unknown. Builder parse failure does not undo its mutations.

MaxEmitters calls AFC1B0 and immediately consumes its live ST0 through the
existing BF7420 hardware helper, reading actual 0109EEA4 at conversion time.
FrameRate uses the distinct AFC1C0 CVTTSS2SI body. The source introduces no
float-to-integer cast or intermediate floating-point store in MaxEmitters.

Emitter names come from token 1 and kinds from the final token, with actual8h
strings passed as ECX kind/EDX name and native stack words resource, zero and
text to AF9FB0. The resulting owner is published only after all four parent
temporaries have been returned. Layer allocation, AFAB90 construction and
AFAD00 reading precede Layer publication. Both inline arrays use the native
unguarded count/index updates. Layer reading runs even for a null allocation.

## Cleanup evidence

The complete live/installed handler and action bytes at CBAC40..CBACE8 and
FuncInfo/map DF2AA8..DF2B43 agree. FuncInfo magic is 19930522, maximum state
15, map DF2ACC, flags 1, and no try, IP or ESType entries. Previous states are
`[-1,0,1,1,3,4,0,6,7,8,0,10,11,0,13]`.

States 0/1 own line/name; 2 owns the first suffix; 3 owns the builder; 4/5 own
outer/inner suffixes; 6..9 own emitter token/name/token/kind. State 10 frees
the captured Layer allocation. States 11/13 conditionally clear the Layer
token via mask bit 1; 12/14 conditionally destroy its name via bit 2. The
normal body reaches state 13 after construction and disarms allocation cleanup
before Layer reading. State 14 exists only in the recovered map. Cleanup
exceptions terminate during C++ unwind; normal cleanup exceptions propagate
through the then-current native cleanup state.

## Validation and limits

The strict MSVC Win32 build and all three existing CTests pass after native
seed verification. The final probe links the built `bsp_core.lib`, including
this parser's CMake registration; it does not separately compile the parser.

The report records 2,899 live/installed bytes, all 762 body instructions and
120 direct call sites. The temporary manifest-embedded Win32 probe compares
the complete copied AF4BA0 body against source. Original calls bridge to the
same genuine raw string, builder, factory, Layer, bounds and preparation
implementations; only read-only string literal addresses are relocated.

Fifteen comparisons cover eight numeric runs across both feature-cell modes
and four x87 rounding modes, six bad-header/EOF/malformed-curve cases, and a
Layer plus all three emitter kinds with Sprite children. Observations include
the full normalized resource image, current text cursor, x87 status, child
content and raw pool accounting. Unwritten Layer padding is excluded. A
separate source-only child-failure case verifies state 9 cleanup, retained
factory owner, preserved earlier property writes, absent publication, and
replay rejection.

This is a new C++ interface. Copied-body fixtures do not establish original
register ABI, FH3/SEH, unrestricted faults, arbitrary provider rebinding,
installed CRT equivalence or gameplay compatibility. Child bridges validate
the parent schedule against the same concrete child implementation; they are
not independent copied-body proofs of every recursive descendant.
