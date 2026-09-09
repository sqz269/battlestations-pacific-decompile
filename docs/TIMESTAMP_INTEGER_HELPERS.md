# Timestamp integer helper audit

The timestamp subtraction routine at `00530890` calls the existing compiler helpers
`__allmul` at `00bf7df0` and `__alldiv` at `00bf7d40`. Their existing library
names are appropriate and were preserved. This document describes their result
bits and ABI; the final section records the integrated native comparison.

## Evidence

The export command and the subsequent memory-read batch each verified project
`bsp`, project file `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Full exported assembly
and decompilation are under the ignored `exports/bsp/functions/<address>/`.
The complete helper bodies, including their three-byte `RET 10h`, matched
installed PE bytes exactly on 2026-09-09:

| Helper | Inclusive range | Bytes | SHA-256 |
|---|---|---:|---|
| `__allmul` | `00bf7df0..00bf7e23` | 52 | `3d0898d998acc8af3ea17032ff56fd1b963c685b072f669c52d4b8267f9d0a78` |
| `__alldiv` | `00bf7d40..00bf7de9` | 170 | `8964dcf0d769cb61dd4354feac00491ae8c1b34eb9627a4c5ec2d4350243d395` |

Both decompilations identify a single Visual Studio 2005 Release library match.
The behavior below follows the actual instructions, independently of that
library identification.

## Calling conventions

Both receive two 64-bit arguments on the stack: at entry, first low/high words
are `[ESP+4]` and `[ESP+8]`, second low/high words are `[ESP+0Ch]` and
`[ESP+10h]`. The result is `EDX:EAX`; the callee pops all 16 argument bytes.
These are compiler helper conventions, not a new C++ public ABI.

`__allmul` preserves EBX using a push/pop only on its general path; ESI/EDI/EBP
are untouched. EAX, EDX, ECX and flags are scratch. `__alldiv` pushes and
restores EDI, ESI and EBX; EBP is untouched. It also rewrites negative operands
in the caller-provided argument slots to their unsigned magnitudes before
returning and popping those slots. Neither routine accesses globals or calls
another function.

## Multiplication

At `00bf7df0..00bf7dfe`, the helper tests whether both high words are zero. If
so, `MUL ECX` at `00bf7e04` directly returns the full 32-by-32 product. Otherwise
it accumulates the low words of `a_hi*b_lo` and `a_lo*b_hi`, then adds that
sum to the high word of the full `a_lo*b_lo` product (`00bf7e1e`). Products or
carries above bit 63 are intentionally discarded; `a_hi*b_hi` contributes
nothing to the retained low 64 bits.

Thus the output is `(a_bits * b_bits) modulo 2^64`. Signedness does not alter
these result bits. There is no checked-overflow branch, trap or saturation.
Using C++ `uint64_t` multiplication is a defined equivalent for the result
bits. Signed C++ multiplication with an overflowing result is not equivalent
because its behavior is undefined.

## Signed division

The high-word sign tests at `00bf7d4b` and `00bf7d67` select modulo two-word
negation (`NEG high; NEG low; SBB high,0`) for each negative operand. EDI counts
negative operands. This representation safely retains unsigned magnitude
`0x8000000000000000` for the most-negative signed input.

For a denominator magnitude with zero high word, the two unsigned divisions
at `00bf7d8b` and `00bf7d93` produce high and low quotient words. The first
uses a zero high dividend; the second uses the first division's remainder.
For a nonzero denominator high word, `00bf7da7..00bf7db1` shifts both
magnitudes right together until the denominator fits 32 bits. The division
at `00bf7db3` forms a quotient estimate. Multiplication and unsigned comparisons
at `00bf7db7..00bf7dd3` compare the estimate times the original denominator
against the original numerator, decrementing the estimate once if necessary.
The carry test at `00bf7dc5` is part of that correction.

Finally `DEC EDI; JNZ` at `00bf7dda` selects result negation only when exactly
one operand was negative. Therefore finite, representable signed division
truncates toward zero. No remainder is returned.

| Input condition | Native result or failure |
|---|---|
| Nonzero denominator, ordinary representable quotient | Signed quotient truncated toward zero |
| `INT64_MIN / 1` | Bits `8000000000000000` |
| `INT64_MIN / -1` | Bits `8000000000000000`, without a division-overflow fault |
| `INT64_MIN / INT64_MIN` | `1` |
| Smaller magnitude divided by larger magnitude | `0`, regardless of sign |
| Zero denominator | Unsigned `DIV ECX` at `00bf7d8b` has ECX zero and raises processor divide error |

The `INT64_MIN / -1` case takes the narrow-denominator path: the first DIV
returns high quotient `80000000`, the second returns low quotient zero, and
EDI is two, so no final negation occurs. The helper contains no signed `IDIV`
that could reject this quotient. For all nonzero signed denominators, the
unsigned DIV operations otherwise remain within their quotient widths.

No SEH handler or divide-by-zero recovery exists inside this helper. The
caller's surrounding exception behavior is not established here. A host
reconstruction should not accidentally replace this with silent zero output;
if it exposes a checked failure instead, that is an explicit interface boundary.

## Defined C++ formulation

For nonzero denominator bits, the following unsigned calculation reproduces
the native quotient bits, including the signed overflow corner case:

```cpp
uint64_t signed_divide_bits(uint64_t numerator, uint64_t denominator) {
    // Caller must enforce denominator != 0, or deliberately implement failure.
    const bool n_negative = (numerator >> 63) != 0;
    const bool d_negative = (denominator >> 63) != 0;
    const uint64_t n = n_negative ? uint64_t{0} - numerator : numerator;
    const uint64_t d = d_negative ? uint64_t{0} - denominator : denominator;
    const uint64_t q = n / d;
    return n_negative != d_negative ? uint64_t{0} - q : q;
}
```

Signed inputs can first convert to `uint64_t`, which is defined modulo 2^64.
Keep subsequent multiplication and negation unsigned. If a signed result is
needed under this Win32 two's-complement target, copy its bit representation
rather than evaluating overflowing signed arithmetic. This reasoning proves
ordinary result-bit equivalence; it does not preserve scratch registers,
argument-slot writes, flags, or the original helper ABI. The focused native fixture below checks these result bits without changing
the helper ABI or exposing a new public arithmetic API.


## Focused isolated native fixture

`tools/verify_timestamp_reference.py` verifies the bsp project and program,
then checks all three complete bodies against the installed PE. Timestamp
subtraction `00530890..00530917` contributes 136 bytes with SHA-256
`225903975901288525f2d40f026081b5620248dd93106930fd06e6217d2b9b42`.
The two helper bodies above bring the isolated payload to 358 bytes.
The audit rejects instructions outside its integer whitelist, absolute or
unexpected memory references, branches leaving a body, incomplete decoding,
and calls other than the two expected helper calls at `005308e4` and
`005308f1`. It emits ignored `local/timestamp_reference.hpp` and
`local/timestamp_reference_audit.json`; the original game is not modified.

`src/timestamp_reference_probe.cpp` allocates private read/write memory,
copies the three audited bodies, relocates only those two relative calls,
then changes the allocation to read/execute and flushes the instruction cache.
It calls subtraction with the native ECX-left / stack-output-right convention
and compares the entire 16-byte output and returned destination pointer with
`bsp::subtract_timestamp_00530890`.

The single compact batch uses four operand pairs for equal-frequency signed
subtraction wrap, unequal-frequency signed truncation, modulo multiplication,
and `INT64_MIN / -1`. Each pair checks a separate output and both exact output
aliases, followed by one all-input/output-identical case (13 comparisons).
The fixture deliberately does not execute the divide-by-zero fault path.
It frees its private executable allocation and does not load or run the game.

Header emission and byte verification succeeded. After integration, the Win32
startup probe executed all 13 native comparisons successfully. Both existing
CTests passed. This validates isolated subtraction result bits and exact aliases,
not full native clock behavior.
