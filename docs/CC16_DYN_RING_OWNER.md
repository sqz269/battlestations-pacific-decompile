# Dyn ring owner constructor at 00C4D5A0

Packet `codex-dyn-ring-owner` recovers `00C4D5A0..00C4D971` in
`src/cc16_dyn_ring_owner.cpp`, with the 0x50-byte native layout in
`include/bsp/cc16_dyn_ring_owner.hpp`. Names describe observed storage and
behavior; they are hypotheses, not recovered symbols. No ledger, build
registration, Ghidra annotation, vendor body or runtime activation is added.

## Boundary, ABI and evidence

The configured saved project is `C:/Users/sqz269/bsp.gpr`. The `bsp.py ghidra`
client verified live project `bsp`, program `/battlestationspacific.exe`,
language `x86:LE:32:default` and image base `00400000` before the live query
batches. Recovery used all 330 saved assembly lines and live prototype,
callee, xref, epilogue-byte and unwind-data queries. No analysis was mutated.

The live body is exactly `00C4D5A0..00C4D971` inclusive (978 bytes).
`RET 8` at `00C4D96F` ends the body; there is no folded continuation or
unreachable tail in this span. At entry, stack `+04h` is the destination
and stack `+08h` is the input owner pointer. The destination is held in EBP
and returned in EAX. EBX, EBP, ESI and EDI are saved/restored. The source's
reference/CRT/provider arguments are a new C++ interface; binary calling
convention, stack layout and FH3/SEH transport are not supplied.

The body itself contains no x87 instructions. It does not replace or
approximate arithmetic performed by its three required in-house callees.

## Layout and composition

| Offset | Observed storage |
|---|---|
| `+00h..+20h` | the exact `DynRingIndices` view from the initializer packet |
| `+04h` | current anchor, using the leaf view's `opaque_04` word |
| `+24h..+2Ch` | scratch vector of uint32 indices |
| `+30h`, `+34h` | next and previous cursors |
| `+38h..+40h` | vector of 12-byte index-vector records |
| `+44h..+4Ch` | vector of two-dword `{previous, next}` records |

The constructor stores the input owner pointer at `+08h`, zero-initializes
the five buffer records, then directly composes
`initialize_dyn_ring_indices_00c33510` at native site `00C4D605`. The leaf
receives the identical initial 0x24-byte layout through a narrow adapter to
the supplied `DynLiveCrt`. Every allocation/free in this parent and its
existing vector-copy composition uses that same CRT custody. No global
allocator, fake service, fallback callback or guessed owner-data field is
introduced. The cursor words are left untouched until the required
`00C4D370` operation prepares them.

For signed counts of at least three, `00C4D370` prepares a chain. The
constructor then tries `00C41230` first and `00C41190` second, testing full
EAX for nonzero. A successful previous-side result moves `+34h` through
the previous-index array and prepends it to scratch; a successful next-side
result moves `+30h` through the next-index array and appends it. Each
successful extension decrements the unsigned `count - 3` budget. If both
tests fail, a captured cursor pair is appended and extension ends. A zero
budget bypasses both tests and the pair append.

The endpoints are linked in the original write order, scratch is appended
as a deep-copied chain, count receives wrapping `2 - scratch.size`, scratch
size is conditionally cleared, and the anchor becomes `+30h`. The signed
count comparison determines whether the process repeats.

Front insertion grows capacity by `old * 2 + 1`; scratch append, pair
append and outer-chain append use `old * 2 + 2`. Allocation products wrap
in uint32 registers. Existing arrays are copied/freed and replaced in the
listing's order, retaining each placement-address null check. The
in-place prepend is kept literally: a zero scratch size with spare capacity
underflows the backward-shift index. No general empty/overflow/pointer
guard or progress cap was added.

The existing `cc15_dyn_live::vector4_copy_construct_0040d7b0` is composed at
`00C4D8B6` and `00C4D921` using the same CRT. During outer-array growth,
each old child buffer is freed immediately after its copy; its old pointer
is not cleared before the next copy. The old outer buffer is freed only
after all copies complete.

## Exceptions and remaining dependencies

EH handler `00CC82DB` points to FuncInfo `00E04138`; its seven-entry unwind
map begins at `00E04100`. States 4 through 0 clean members in the order
`+44h`, `+38h` (children then outer allocation), `+24h`, `+18h`, `+0Ch`.
The source catch reproduces those observed helper effects using the same
CRT, without clearing fields. The exact unwind tail sites are recorded in
the JSON report; no separate reconstruction credit is taken for those
helper bodies. States 5 and 6 invoke `00401130`, whose complete body is
`RET`, so they add no private-allocation rollback. The source likewise
does not reclaim a partially grown outer array or clear already-freed old
child pointers. CRT free providers retain their nonthrowing CRT contract.

The following bodies remain required typed contracts rather than supplied
implementations: `00C4D370` (ESI receiver, prepare chain), `00C41230` and
`00C41190` (ECX receiver, EAX zero/nonzero tests). The prepare contract also
receives the same CRT, because its native call graph allocates/frees. These
dependencies must be recovered and bound before execution can establish
the parent's geometry behavior. This packet does not claim their bodies,
ABI or arithmetic.

Live xrefs report no references to `00C4D5A0`; the current indexed census
also reports no incoming sites. This is bounded negative evidence, not a
complete dead-chain proof. Recovering this source does not establish
runtime reachability. The primary integrator owns reference/deadness
closure, central metadata, build registration and the common build.

Validation includes instruction-by-instruction source review and a strict
MSVC x86 C++17 object compile with `/permissive- /EHsc /W4 /WX`. The live
call-report verifier checked all 15 body sites and seven unwind sites with
zero failures. The common build and post-integration checks remain with the
primary integrator. No test suite, ad hoc executable, game run, fixture
result or ABI/runtime compatibility claim is included.
