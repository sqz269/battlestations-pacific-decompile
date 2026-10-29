# Dyn ambient x87 orientation and barycentric providers

This packet adds ambient instruction variants for two bodies already represented
by `src/cc13_dyn1.cpp`. It earns **zero new native-body credit**. The old fixed
PC24 CCW and double-intermediate barycentric interfaces and their callers remain
unchanged. New source is `src/cc16_dyn_ambient_math.cpp`, with public declarations
in `include/bsp/cc16_dyn_ambient_math.hpp`. Names describe recovered behavior;
they are not recovered symbols.

The private naked helpers preserve every native instruction and its encoding.
The public cdecl wrappers push raw argument words, supply register outputs, and
read the full integer CCW result. They do not approximate x87 operations using
`double`, select a precision mode, normalize exceptional inputs, or skip `w`.
These are compiled source providers, with no activation or runtime admission.

## Evidence and native ABIs

Read-only `bsp.py ghidra` queries verified the current project/program on every
batch: `bsp`, `/battlestationspacific.exe`, `x86:LE:32:default`, image base
`00400000`, using `C:/Users/sqz269/bsp.gpr` in `config/target.json`.
The wrapper's `Client.verify()` checks project name, program path, language and
base before the live query. Contiguous assembly and bytes were read for both
bodies; no decompiler assumptions were used for their instruction schedules.

| Native span | Size / instructions | Original calling convention |
| --- | --- | --- |
| `00C377F0..00C37857` | 104 bytes / 35 | Six stack floats `(x0,y0,x1,y1,x2,y2)`, full `EAX=0/1`, `RET 18h` |
| `00C37860..00C37984` | 293 bytes / 111 | `EAX=u_out`, `ECX=v_out`, `EDX=w_out`; eight stack floats `(x0,y0,x1,y1,x2,y2,px,py)`; `RET 20h` |

CCW uses `SUB/ADD ESP,8`; barycentric uses `SUB/ADD ESP,10h`. Neither body
touches EBX, EBP, ESI or EDI, accesses external data, or calls another function.
CCW overwrites EAX; barycentric only reads its three integer registers. The
maximum added x87 depths are respectively four and seven; each balances its
added entries on normal return. Existing x87 entries below them are retained
when enough entries are free. Stack overflow, pending exceptions and exceptional
exits follow the actual instructions, rather than a software recovery policy.

CCW's caller `00C41190` was independently inspected through its first call at
`00C411C1`: it pushes six raw vertex words, then tests full EAX. Current caller
queries name `00C41190` and `00C41230` for CCW; barycentric's indexed references
include `00C41070` (undefined code), `00C410E0`, and `00C4D050`. None establishes
an inherited precision or rounding mode.

## Storage epochs and operand direction

Offsets below are relative to ESP **after** the native local allocation.
Arithmetic symbols in this section explain dataflow only. Every operation uses
the original x87 direction, ambient arithmetic precision/rounding, and extended
exponent range. Each float store has the original independent conversion to
binary32; no algebraic substitution is used by the source.

For CCW, `a=(x1-x0,y1-y0)` and `b=(x2-x0,y2-y0)` are first stored as float:

| Instruction | Destination | Epoch |
| --- | --- | --- |
| `00C37801 FSTP` | `[ESP]` | `a.x` |
| `00C37812 FSTP` | `[ESP+4]` | `a.y` |
| `00C3781E FSTP` | `[ESP+14h]` | `b.x`, replacing the x1 argument |
| `00C37826 FSTP` | `[ESP+18h]` | `b.y`, replacing the y1 argument |
| `00C37841 FSTP ST(0)` | register pop | Pops the retained zero after comparison; no memory rounding |

`00C3782A..00C37839` computes `b.y*a.x - a.y*b.x` in registers. Neither product
nor the difference is spilled. `FLDZ; FXCH; FCOMIP ST(0),ST(1)` compares that
cross value against zero. `JBE` returns zero for ordered nonpositive values and
for unordered values when execution continues with invalid masked. `FCOMIP`
signals invalid for quiet as well as signaling NaNs; it is not `FUCOMIP`.

For barycentric, `a=triangle1-triangle0`, `b=triangle2-triangle0`, and
`d=point-triangle0`. Dot-product labels denote the stored values
`A=a.a`, `B=b.a`, `C=b.b`, `P=a.d`, and `Q=b.d`.

| Instruction | Destination | Epoch |
| --- | --- | --- |
| `00C37871 FSTP` | `[ESP]` | `a.x` |
| `00C37882 FSTP` | `[ESP+4]` | `a.y` |
| `00C3788C FSTP` | `[ESP+1Ch]` | `b.x`, replacing x1 |
| `00C37896 FSTP` | `[ESP+20h]` | `b.y`, replacing y1 |
| `00C378A2 FSTP` | `[ESP+8]` | `d.x` |
| `00C378AA FSTP` | `[ESP+0Ch]` | `d.y` |
| `00C378C3 FSTP` | `[ESP+24h]` | `A`, replacing x2 |
| `00C378DB FSTP` | `[ESP+1Ch]` | `B`, replacing b.x |
| `00C378EB FSTP` | `[ESP+14h]` | `C`, replacing x0 |
| `00C37905 FSTP` | `[ESP+2Ch]` | `P`, replacing px |
| `00C37913 FSTP` | `[ESP]` | `Q`, replacing a.x |
| `00C37934 FSTP` | `[ESP+1Ch]` | `D=C*A-B*B`, replacing B |
| `00C37953 FSTP` | `[ESP+1Ch]` | `u=(C*P-Q*B)/D`, replacing D |
| `00C3796D FSTP` | `[ESP+1Ch]` | `v=(A*Q-B*P)/D`, replacing u |
| `00C3797D FSTP` | `[EDX]` | `w=(1-u)-v` |

Each dot product performs two register products and one register addition before
its listed store. In particular, B adds its `b.x*a.x` product into the retained
`b.y*a.y` product; P adds the `a.x*d.x` product into the `a.y*d.y` product; Q adds
the `b.y*d.y` product into the `b.x*d.x` product. The exact opcode directions,
including NaN operand precedence, remain the encoded native directions.

`00C37930 FSUBRP ST(3),ST(0)` forms the determinant product minus the square.
`00C37947 FSUBP ST(3),ST(0)` forms `C*P-Q*B`.
`00C3794F FDIVP ST(4),ST(0)` divides that unspilled numerator by a loaded D.
The numerator and quotient receive no extra spill before `00C37953`.
`00C37967 FSUBRP ST(3),ST(0)` forms `A*Q-B*P`, followed by
`00C37969 FDIVP ST(2),ST(0)` using the retained float-loaded D. Its numerator
and quotient likewise remain in registers until `00C3796D`.

The u spill is reloaded before **non-popping** `00C3795B FST [EAX]`.
The v spill is reloaded before **non-popping** `00C37975 FST [ECX]`.
Those retained u and v values feed `FLD1; FSUBRP ST(2),ST(0); FSUBP`, forming
`(1-u)-v` with no intermediate float store. All three output stores execute even
when the caller ignores w. There is no denominator guard or clamp.

## Ambient context and alias contract

The providers do not execute `FLDCW`, `F[N]INIT`, `F[N]CLEX`, state save/restore,
or any MXCSR operation. Arithmetic follows the actual x87 precision control
(including PC24/PC53/PC64 as selected), all four rounding modes, exception masks,
pending exceptions, and hardware handling of reserved control settings. Existing
status is not cleared: normal sticky exceptions and condition-code changes are
observable. Float loads, arithmetic, float stores and comparisons occur in the
same order, including every NaN quieting/selection and denormal, underflow,
overflow, divide-by-zero, invalid or inexact opportunity. Signed-zero outcomes
come from those instructions. A zero determinant still divides; masked results
can continue through infinities/NaNs, while an unmasked exception interrupts at
the corresponding relocated instruction.

No SSE instruction occurs in either body or either inspected wrapper. MXCSR
remains unchanged, and its DAZ/FTZ, rounding and exception controls do not replace
the x87 controls for these operations. The ring query's separate COMISS gates
still depend on its actual MXCSR; this packet supplies no SSE gate or state reset.

Output pointers must address writable float storage. Exact pointer equality and
partial overlap retain native store order: u, then v, then w. For equal pointers,
the last successful store wins; w still uses retained u/v, not pointer reloads.
The naked body also retains its native scratch/argument overwrites. The public
C++ wrapper creates private copies of those slots and does not expose their
addresses. Native aliases into the callee's stack, return-address corruption,
native caller cleanup, integer flags/register transparency, and fault/exception
transport require a separate binary adapter. Public arguments are by value, so
aliasing output pointers with caller input objects does not make them aliases of
the helper's private input slots.

The public API is ordinary cdecl C++; the native entrypoints are not exported.
Code/data addresses in x87 last-instruction/data-pointer state and hardware trap
contexts refer to the relocated helpers and their stack slots. They cannot equal
the original image addresses. Consequently byte identity proves the arithmetic
and storage schedule, not binary admission or original-address exception frames.
Callers remain responsible for passing unmodified float bit patterns and the
intended ambient context into the public entry. No precision is inferred from
deadness or the absence of native references.

## Compile and object evidence

MSVC Win32 compiler `19.51.36244.0` compiled the final source with
`/std:c++17 /EHsc /W4 /WX /fp:strict` in both `/O2 /Ob2` and `/Od /RTC1`
configurations. `/c` produced objects only; no linker, executable, probe or game
ran. `/RTC` instrumentation is locally disabled around the wrappers so it cannot
introduce an extra helper call; the pragma is restored afterward. EBP-based
argument addressing is likewise explicitly scoped and restored.

The local COFF inspector compared each helper symbol against the exact span of
the installed PE on disk, read-only. Both configurations match all 104 and 293
bytes respectively, with zero differing offsets and zero relocations within
either body. Capstone counts 35 and 111 instructions. Native-span SHA-256:

- `00C377F0`: `a76903724f146a68fc4815a7b131b3cea18247e7aef33fb9a3b1b5f6e5d83450`
- `00C37860`: `a1b1f99bfff1b431e96c393b5035896b0c51414ca59c4492c390835f701ac92c`

In both final objects, the CCW wrapper has 15 instructions and the barycentric
wrapper 16. Each contains only integer/frame operations and exactly one call to
its helper. Neither contains any FP/SSE instruction, conversion, precision
control operation, extra arithmetic, or additional runtime call. The wrappers
push the six/eight words in reverse native order and load EAX/ECX/EDX from the
three public output-pointer arguments. Native `RET 18h/20h` restores those pushes.

Ignored evidence is under `local/cc16_dyn_ambient_math/`: `release/debug.obj`,
the compiler listings, and their `.inspection.json` records. The inspection
script only reads the object and installed PE; it does not execute either. No
new tests or native differential fixtures were needed after exact-byte proof.

## Dependency closure and remaining integration

These leaf variants have no external-data or call dependencies. They can replace
the fixed-PC CCW composition in the sibling ring predicate packet, and supply
the required `DynRingQueryCalls::barycentric_00c37860` provider while preserving
all outputs and the actual ambient context. The sibling ring source/documents
were read-only interface evidence; their acceptance state is not changed here.

The primary integrator must register the new source, bind the ring providers,
review the final emitted target object under its own build configuration, update
shared ledgers/Ghidra annotations as appropriate, and build the integrated tree.
This worker changes no CMake, ledgers, existing callers, Ghidra annotations,
installation, saves or profile. No game, ABI-adapter, fixture-execution,
activation, or gameplay claim is made.
