# Native particle bounds reader

## Complete body and ABI

`read_native_particle_bounds_00af4700` implements all 1176 bytes at
AF4700..AF4B97. Native ECX is a captured destination owner; its one stack
argument is the actual text buffer. The function returns with RET4 and has no
defined result. It writes five binary32 fields at destination+7C, +80, +84,
+88 and +8C. Their individual semantic names are not established here.

The full body, four unwind thunks, compiler handler, FuncInfo/unwind map and
six literal strings match the installed PE and live `bsp.gpr` /
`battlestationspacific.exe`. Eight spans preserve their full bytes and SHA256
values in `reports/native_particle_bounds_reader_orch4.json`, along with all
48 body calls and prior Ghidra documentation/comments. Complete disassembly
has 399 instructions, including two jumped-over alignment instructions missing
from the 397-instruction listing: AF47ED..EF and AF485D..5F are both
`LEA ECX,[ECX]`. Neither is a missing execution path or requires flow repair.
`FUN_00af4700` is the previous name; the new descriptive name is a hypothesis.
This worker makes no Ghidra changes.

## Real input and provider contracts

The new source API borrows the actual destination, actual 1Ch text buffer,
`NativeStringRawPoolContext` and the application's shared F8C2C8 scratch.
It uses the complete raw AF5740 line reader, AEE3C0 token constructor, AF44C0
suffix constructor and AF3E90 space-run counter. Every text return invokes the
current 419CC0 getter and BD1510 through the caller's same raw pool, singleton
manager and shutdown-gate cells. No host storage adapter, callback parser,
invented owner, private scratch or replacement container is introduced.

Current CRT `_stricmp` and `atof` remain explicit provider boundaries. Literal
bytes are verified as `BoundSphere`, `{`, `}`, empty, `Param` and `Coords`.
All comparisons are case insensitive; brace comparisons consume the entire
normalized line. Source pointer identity of native literal storage is not part
of this interface.

## Control flow and effects

The first loop reads a line and constructs token0 to test for a null pointer.
For a nonnull token it constructs token0 a second time and compares it with
`BoundSphere`. It stops on a match, a null token (including an empty line), or
EOF. Those conditions all proceed into a separate search for an exact `{` line.
Even EOF from that search is followed by one body read. No early return or
requirement that `BoundSphere` was found is added.

The body consumes lines through exact `}` or EOF. Empty lines are ignored.
For a nonempty line it constructs token0, compares with `Param`, and returns
that temporary before asking the current line's space counter for at least
three fields. It constructs token1 as the parameter name and suffix2 as the
value text. Only `Coords` writes the destination. It takes value tokens0..4
in order, with no five-token validation or null repair. Repeated matching
parameters overwrite those five fields. Cursor effects and all other owner
bytes remain those of the actual called bodies.

Each CRT `atof` result remains in x87 ST0 until a direct FSTP to its actual
binary32 field. The source assembly helper introduces no intermediate C++
double or float object. Stores occur at AF495C, AF49AA, AF49FA, AF4A4A and
AF4A9A, each before its token's current-pool return. The caller's floating-point
environment is retained. No validation, rollback, private bounds object or
whole-result transaction changes the incremental stores.

## Exception ownership

Handler CBAC31 loads FuncInfo DF2A84 (maxState4); unwind map DF2A64 contains:

| State | Next state | Action |
| --- | --- | --- |
| 0 | -1 | CBAC00 clears the current line |
| 1 | 0 | CBAC08 clears the first scan token only when its stored bit1 is set |
| 2 | 0 | CBAC21 clears the parameter name |
| 3 | 2 | CBAC29 clears the suffix |

These are true unwind actions. `BoundsUnwind` uses a `noexcept` destructor,
so a second exception during cleanup terminates. It does not add ownership of
the second scan token, body keyword token or any of the five coordinate tokens.
They may remain unreleased if their normal work fails, just as the table shows.

The first scan activates state1 only after its first token constructor returns;
its bit2 lives only in EBX until normal cleanup. Returning the second token
still has state1; state0 is restored before returning the first token. Name
construction activates state2, suffix construction state3. Normal suffix/name
cleanup captures the pointer before lowering state to2/0, with the corresponding
header zeroed only after successful return. Final line cleanup captures its
pointer before state=-1 and performs no final header clear. A failed normal
return is not retried by the unwind guard. Existing raw getter exceptions may
propagate, and partial destination stores are retained.

## Verification boundary

The ignored focused probe executes a copy of the complete original body and
the source over actual text buffers and the genuine raw pool/manager. Original
direct calls are redirected to the same real text/pool/CRT providers, with
literal pointer operands relocated to their verified bytes. Original internal
branches and EH setup remain present; original exceptions are not triggered.
Four scripts cover ordinary scanning with ignored lines/parameters, the blank
line exit without `BoundSphere`, EOF before a brace, and EOF after coordinates
without a closing brace. One numeric token lies near a binary32 rounding tie; its script runs under all
four x87 rounding modes while retaining the other control-word bits.
The probe compares every destination byte, cursor/extent/auxiliary fields and
pool bump/live/peak counts, then drains the genuine raw singleton manager.

Seed verification, the strict Win32 build and all three existing CTests pass.
The probe passes all seven comparisons, including asserted downward/upward
first-field bits3F800000/3F800001. All52 function-owned call/tail rows verify;
one undefined compiler FH3 tail remains explicitly raw-only byte evidence.
The report records the exact results. No permanent tests are added. Proof excludes native
getter/allocation failure execution, FH3 fault dispatch, CRT locale/numeric
identity, native stack-slot aliases, register ABI, hardware SEH and gameplay.
