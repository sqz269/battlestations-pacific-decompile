# Dyn ring predicates at 00C41190 and 00C41230

Packet `codex-dyn-ring-predicates` recovers the two bodies in
`src/cc16_dyn_ring_predicates.cpp` and reuses the proven `DynRingOwner`
layout from the parent packet. The declarations and required downstream
query contract are in `include/bsp/cc16_dyn_ring_predicates.hpp`. Names are
descriptive hypotheses. This packet changes no earlier source, ledger,
build registration or Ghidra analysis and adds no vendor body.

## Boundaries and original ABI

The configured saved project is `C:/Users/sqz269/bsp.gpr`. Before live
batches, the `bsp.py ghidra` client verified project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and image base
`00400000`. Recovery used the complete saved assembly, live prototypes,
callees, xrefs and full byte spans. The exact body spans are:

| Function | Inclusive span | Bytes | Input and return |
|---|---|---:|---|
| next-side test | `00C41190..00C4122A` | 155 | ECX receiver, no stack arguments, plain RET, full EAX 0 or 1 |
| previous-side test | `00C41230..00C412CD` | 158 | ECX receiver, no stack arguments, plain RET, full EAX 0 or 1 |

The successful deeper paths preserve EBX, EBP, ESI and EDI; early exits
restore whichever of those registers they saved. There are no folded
tails in either span. The C++ uint32 return preserves the whole-value
contract consumed by the parent; this is not an AL/bool binary adapter.

Each body calls `00C377F0` up to twice, short-circuiting on its zero EAX result,
then calls `00C410E0`. The final `NEG EAX; SBB EAX,EAX; NEG EAX` converts
every nonzero uint32 value to one, including values whose low byte is zero.
All six exact sites are in the JSON report.

## Storage, edges and query arguments

The receiver's `+08h` points to the existing input owner record. Its first
dword addresses packed two-float vertex records (stride eight); its second
dword is the count already used by the initializer. This first-dword use is
proven by the two predicates' loads and six float arguments to `00C377F0`.
No new native owner field is guessed or written.

Let `N(i)` read the next-index array at `+0Ch`, `P(i)` read the previous
array at `+18h`, `n` read cursor `+30h`, and `p` read cursor `+34h`.
Each orientation call uses the following ordered vertex indices:

| Predicate | First orientation | Second orientation |
|---|---|---|
| `00C41190` | `P(n), n, N(n)` | `N(n), p, N(p)` |
| `00C41230` | `P(p), p, N(p)` | `P(n), n, P(p)` |

If both orientations pass, the exact `00C410E0` inputs are:

| Predicate | EAX walk start | EBX triangle2 | Stack owner, stop, triangle0, triangle1 |
|---|---|---|---|
| `00C41190` | `N(N(n))` | `N(n)` | `owner, p, p, n` |
| `00C41230` | `N(n)` | `n` | `owner, P(p), P(p), p` |

The source retains the captured initial cursor, cached link pointers and
later cursor reloads at the original stages. All index/address products
wrap in uint32 Win32 addresses. Neither body changes cursors, link arrays,
vertex data or owner storage. The parent performs its cursor advance only
after the corresponding predicate returns nonzero.

## Floating-point and dependency boundary

Neither predicate contains an x87 or SSE arithmetic instruction. The
orientation dependency is directly composed with the existing
`cc13_dyn1::ccw_2d_00c377f0`. That implementation explicitly uses
`x87_sub24`/`x87_mul24`, including 24-bit significand rounding with the
extended exponent for its register intermediates. Four coordinate
differences are narrowed at the original float stores. Its final ordered
comparison returns false for zero or an unordered cross, matching the
listing's `FCOMIP` and `JBE` branch.

Existing `docs/CC13_FIXTURE_PASS_7.md` records 4000/4000 comparisons at the
deciding PC24 precision and 3998/4000 at PC53. This is historical fixture
evidence for the dependency, not a new test run by this packet. These
wrappers add no new arithmetic approximation or hidden PC24 model. A
future runtime binding to a PC53 thread would require a separate precision
decision; this packet establishes no runtime path or FPU-control transport.

`00C410E0` remains a required typed contract, with EAX/EBX plus four stack
arguments retained separately in `DynRingTriangleQuery`. The queried body
is `00C410E0..00C41187`, returns full EAX and uses `RET 10h`. Inspection
establishes read-only receiver/edge/vertex use; its barycentric arithmetic
and walking body are not implemented or credited here. No default result
or fake downstream body is provided. `00C4D370`, the parent's chain
preparation operation, is a separate outstanding dependency and is not
implemented by this packet.

Live xrefs identify only the two parent call sites, `00C4D737` and
`00C4D63B`, respectively. The parent remains without incoming live/indexed
references in the prior bounded census. Source recovery does not establish
runtime reachability or a complete dead-chain proof. The primary integrator
owns parent binding, dependency closure, common build, central metadata,
locked annotations and runtime/deadness evidence.

Validation includes complete instruction review and an MSVC x86 C++17
object-only compile with `/permissive- /EHsc /W4 /WX`. The live call-report
verifier checked all six rows with zero failures. Common build and
post-integration checks remain with the primary integrator. No new tests,
ad hoc executable, game run, fixture result or ABI/runtime compatibility
claim is included.
