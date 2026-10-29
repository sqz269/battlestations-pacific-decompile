# Dyn ring triangle query at 00C410E0

Packet `codex-dyn-ring-query` recovers `00C410E0..00C41187` in
`src/cc16_dyn_ring_query.cpp`, using the established owner, point and query
views without changing earlier source. The header is
`include/bsp/cc16_dyn_ring_query.hpp`. Names are hypotheses. No ledger,
build registration, Ghidra mutation, native-body credit for another helper,
or runtime activation is included.

## Evidence, boundary and ABI

The configured saved project is `C:/Users/sqz269/bsp.gpr`. The `bsp.py
ghidra` client verified live project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and image base
`00400000` before the live batches. Recovery uses the complete saved
assembly and live prototype/callee/full-byte-span evidence. The inclusive span is 168
bytes, with no folded tail. Its two returns are `RET 10h` at `00C4117A`
and `00C41185`.

The original receives the walk start in EAX and triangle2 in EBX. Its
four stack arguments are `(owner, stop, triangle0, triangle1)`. EBP captures
triangle1, EDI holds the current walk index, and ESI holds the cached vertex
base. EBP, ESI and EDI are saved/restored; EBX is read unchanged without
being pushed. ECX, EDX, EAX and XMM0/XMM1 are scratch. Whole EAX returns
one when no included point is found before stop, or zero when one is found.
The source's reference/provider interface is not a binary ABI adapter.

The only call instruction is `00C41134`, targeting `00C37860`. This query
does not call the CCW helper. The exact site is in the JSON report.

## Walk and native storage

An empty walk (`start == stop`) returns one before reading native owner,
vertex or link storage. For a nonempty walk, the point base is loaded once
from the input owner's first dword. Each tested vertex and triangle vertex
uses stride eight. The query calls the barycentric operation on
`(triangle0, triangle1, triangle2, current point)` in that order.

If the point is not included, the next-array pointer at receiver `+0Ch`
is reloaded and the current index advances through it. Reaching stop
returns one. Address products and additions wrap as Win32 uint32 values.
There is no bounds, cycle, count or progress guard. The query changes no
native owner, cursor, link or point data. In the original private stack,
triangle1's by-value argument slot is reused as `u_out` after EBP captures
its index; the source captures the index and uses private output storage.

## Ambient floating-point instructions

The first two gates are literal `COMISS` followed by carry branches:
negative or unordered `u` rejects the point, then negative or unordered
`v` rejects it. If both pass, x87 `FLD u; FADD v; FLD1; FCOMIP; FSTP`
compares one with the un-stored sum. `JNC` includes the point only when
one is ordered above or equal to that sum. Boundary coordinates are
included. A rejected point advances the walk; an included point returns
zero immediately.

The source preserves that instruction sequence in MSVC Win32 inline
assembly, including the SSE carry/unordered behavior and balanced x87
stack. It uses actual ambient x87 precision, rounding and exception masks,
plus actual MXCSR denormal/exception controls. It neither selects a
precision mode nor inserts a float store after the addition. This avoids
replacing the sequence with a fixed-PC arithmetic approximation. Binary
register state and exception transport are not supplied by the C++ ABI.

## Required barycentric and predicate qualifications

`DynRingQueryCalls::barycentric_00c37860` is a required typed dependency,
without a default implementation. Its four by-value point records match
the original eight stack floats. It must produce all three original
outputs at the original store epochs under the same actual ambient x87
context, preserve control/MXCSR and balance the x87 stack. The unused `w`
output remains part of that arithmetic/exception contract.

Existing `cc13_dyn1::barycentric_2d_00c37860` models double/PC53
intermediates. `docs/CC13_FIXTURE_PASS_2.md` historically classified this
family as worker-reachable when choosing PC53. That source is not silently
composed as an ambient implementation here. Current negative reference
evidence does not establish an inherited PC24 mode or rounding context.

The earlier predicate packet likewise directly composes a qualified fixed
PC24 orientation value model, with historical fixture evidence under its
fixture controls. It does not establish generic ambient-CW equivalence.
Before generic runtime binding, the integrator must replace that CCW
composition with an ambient exact variant or require an explicit matching
FP-context provider. No new default precision inference is made here.

A bounded followup is to audit `00C37860..00C37984` (293 bytes) into a
separate qualified variant, preserving its output-register ABI, all float
store epochs, arithmetic operand directions, precision/rounding context,
NaNs, denormals and zero-denominator behavior. Existing PC53 code must
remain unchanged and no fresh native-body credit should be claimed for an
alternative precision model. A separate ambient CCW variant or explicit
context provider is also required for generic predicate binding.

The parent's chain preparation `00C4D370` is outside this packet. The
query and predicates are downstream of that currently unreferenced parent
chain; bounded negative evidence is not complete deadness proof. The
primary integrator owns dependency/provider wiring, FP qualification,
central metadata, common build and runtime/dead-chain closure.

Validation includes full instruction review and an MSVC x86 C++17 object
compile with `/permissive- /EHsc /fp:strict /W4 /WX`. Object disassembly
confirms both COMISS carry branches and the x87 FADD/FCOMIP/JNC sequence.
The live call-report verifier checked the sole call row with zero failures.
Common build and post-integration checks remain with the primary
integrator. No test suite, ad hoc executable, game run, fixture result or
ABI/runtime compatibility claim is included.
