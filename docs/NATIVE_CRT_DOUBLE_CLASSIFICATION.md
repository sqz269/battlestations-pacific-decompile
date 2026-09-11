# Native CRT double classification and rounding

This packet preserves three complete native library routines without renaming
their correct Ghidra identities:

| Native entry | Complete range, end exclusive | Bytes | Source entry |
| --- | --- | ---: | --- |
| `__sptype` | `00C12F3E..00C12F99` | 91 | `native_crt_sptype_00c12f3e` |
| `__fpclass` | `00BFA52F..00BFA5C3` | 148 | `native_crt_fpclass_00bfa52f` |
| `__frnd` | `00C28548..00C28559` | 17 | `native_crt_frnd_st0_00c28548` |

The source consists of the complete 112 original instructions. There are no
literal or mutable global dependencies. `__sptype` and `__frnd` have no calls;
`__fpclass` calls only the full source `__sptype`. No host classification or
rounding routine, floating argument adapter, callback or partial math kernel
is used. Future `C19DC0/C19E24` callers are outside this packet.

## Raw argument storage and ABI

The native classifiers consume a binary64 argument in their actual eight-byte
callee argument slot and return an integer in EAX. Both have an actual
unaligned **DWORD** read at `[EBP+0E]`, spanning input bytes six and seven and
the next two caller-storage bytes. The source retains that access width.
`__sptype` reaches the read after its two exact infinity cases; `__fpclass`
reads it at entry.

The public cdecl source interfaces accept `uint32_t low_word`, `high_word` and
`readable_tail_word`. The first two words are the actual binary64 slot bytes;
the third makes the full original read accessible. The caller removes all
three public words. Tail bits do not affect the integer classification because
the native masks discard them, but readable extent is still required. These
are new source interfaces, not original caller ABI compatibility claims.

Inside `__fpclass`, the exceptional-exponent path retains its exact native
`FLD [EBP+8]`, two ECX pushes and `FSTP64 [ESP]` argument preparation. It calls
full `__sptype` with **only that eight-byte temporary**, then discards exactly
those two words. No third argument is inserted into this native internal call.
The child's `[EBP+0E]` read spans the parent's temporary bytes at parent
`EBP-2..EBP-1` and readable saved-frame bytes at parent `EBP..EBP+1`.

The source `__frnd` declaration also accepts two raw words, which its assembly
caller removes. It is declared `void` and is **assembly-only**: its result is
left in ST0, and the caller must consume/pop that result explicitly. An
ordinary C++ call cannot express this output obligation. Entry requires room
for one additional x87 stack value. Its two scratch pushes and pops are
retained; the final ECX value is the high DWORD of the rounded binary64 scratch.

## Native behavior retained

`__sptype` performs integer bit classification without touching x87 state.
Its native results are `1` for positive infinity, `2` for negative infinity,
`3` for the quiet-NaN encoding, `4` for the signaling-NaN encoding, and `0` for
other encodings. The full DWORD access described above is preserved even
though only its low masked bits affect these decisions.

`__fpclass` first tests the exponent through that same full-width storage read.
The all-ones exponent path loads and spills the input through x87 **before**
calling `__sptype`. This can quiet/change an exceptional input or raise a
hardware exception before integer classification. Its returned special codes
are selected from the helper result by the original decrement/branch sequence:
helper `1` gives `200h`, `2` gives `4`, `3` gives `2`, and the remaining path
gives `1`. The source does not replace this with direct input-bit classification.

The ordinary exponent path retains its integer denormal test and sign-derived
codes, followed where reached by the exact zero comparison:
`FLDZ; FCOMP64 [EBP+8]; FNSTSW AX; TEST AH,44h; MOV EAX,ECX; JP ...`.
No host comparison or boolean interpretation replaces the x87 status/parity
test. Returning paths balance their temporary x87 entry.

`__frnd` retains `FLD64`, `FRNDINT`, `FSTP64`, and `FLD64` in order. The actual
x87 control word, rounding mode, status, masks and stack state govern the
operation. The binary64 spill/reload is not elided. None of these functions
installs or restores an invented FP environment or adds an exception handler.
Hardware exceptions can leave the original partial execution effects; this
packet does not promise recovery or translate them into C++ exceptions.

## Evidence and verification

Fresh guarded queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and compared all three complete spans against
the untouched original PE. The source preserves the original function bounds
and all 256 instruction bytes except the one necessary direct-call relocation.

The strict actual Win32 library is built through an ignored extra-source CMake
hook and `scripts/build.ps1`, with both existing CTests and eight freshly
verified native seeds. The whole resulting archive, exact member and complete
source/header inputs are frozen before inspecting the proof. Every COFF
section, symbol and relocation is recorded. Each function's complete bytes
match the original; the sole `IMAGE_REL_I386_REL32` binds `__fpclass` to the full
`__sptype` section in that same actual archive member. Resolving it at the
original section addresses reproduces all 148 original `__fpclass` bytes.

The audit and ignored `local/crt_double_classification/` bundle record exact
build, archive, member, source and original pins. No new runtime suite is added:
the bounded result is complete source/instruction and archive-relocation proof,
plus the existing build checks. It is not runtime verification of FP values,
exception paths, arbitrary native callers, original ABI compatibility or game
behavior. No shared build file, Ghidra annotation, ledger or parent body is
changed.
