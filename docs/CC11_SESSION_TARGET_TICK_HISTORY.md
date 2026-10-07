# Session target tick-history consumer

This reconstructs the complete ordinary body `[007833E0,0078348F)`:
175 bytes and 59 native instructions, originally ECX target, one stack DWORD,
RET4. The name describes inferred behavior, not a recovered symbol. The Source
interface uses the existing live target/history storage and actual borrowed
cells; it does not implement the surrounding packet receiver or time producer.

`F876B0` is read exactly once. Its signed value below 10 selects the native
`FLDZ/FSTP` zero argument followed by complete history reset `00783230`.
Otherwise the body executes `FLD F876A8 / FDIV D0DE84` and retains the extended
value in ST0 until the existing `native_crt_truncate_st0_00bf7420` call. The
divisor is the actual native float operand `3D4CCCCD`, not an injected constant.
The shared converter reads the actual `0109EEA4` cell at conversion time:
nonzero uses native double spill/CVTTSD2SI, zero uses the complete BF7456 x87
conversion. No C++ cast, fabricated clock, cached mode or replacement converter
is supplied.

The native arithmetic is preserved as modulo32 SUB/ADD/NEG plus signed
comparisons. It independently normalizes `(captured_step & FFFF) - argument`
and `argument - (converted_EAX & FFFF)`, subtracting 65536 above +32768 or
adding 65536 below -32768, once each. **Both endpoints -32768 and +32768 are
retained.** Narrowing to `int16_t` would change the positive endpoint. A
positive first difference wins and is negated for the sample; otherwise a
positive second difference is used; otherwise the sample is zero. Complete
history append `007831E0` performs the update.

The sole observed caller at `007858C0` reads a 16-bit packet field and uses
`MOVZX EDX,word [ESP+14]` at `00785994`, then calls at `0078599C` when its
three-bit delivery field is zero. Thus the observed producer domain is
`0..65535`. The new entry still preserves the complete DWORD argument and
adds no masking. Out-of-producer-domain fixture inputs are explicitly helper
cases, not evidence that the packet receiver produces them.

The focused probe used two complete Source-constructed targets, with their
actual owned 50-element D54 and three-element D58 histories, buffers and
tracked locks, and destroyed both afterward. Each case seeded a coherent
50-element history of ones with total 50 and valid index 0 or 49. Original
and Source computations received equivalent actual timing/mode cells and the
same native divisor. No native allocator or game process was invoked.

The original comparison executed the complete 175-byte consumer, complete
70-byte append, complete 68-byte reset, and both actual CRT conversion paths.
The contiguous 171-byte CRT copy `[BF7420,BF74CB)` retains the first entry's
28-byte body and complete 117-byte BF7456 fallback at its original relative
offset. The intervening alternate-entry bytes are preserved but never entered
from BF7420's two paths. They are not an additional reconstructed entry.

Only these absolute operands were relocated:

| Original operand site | Original cell | Bound fixture storage |
| --- | --- | --- |
| `007833E3` | `00F876B0` | Original target's actual step cell |
| `00783401` | `00F876A8` | Original target's actual time cell |
| `00783407` | `00D0DE84` | Native divisor storage |
| `00BF7422` | `0109EEA4` | Original target's actual conversion-mode cell |

Five four-byte CALL displacements were relocated: instruction `007833F5`
to copied reset, `0078340B` to copied BF7420, and `00783466`, `00783477`,
`00783485` to copied append. All other consumer/CRT bytes and every history
body byte were checked unchanged. Original conditional branches, stack
arguments and RET4 remain intact. Copies were sealed read/execute before use.

Fresh strict Win32 compilation and execution passed 333 checks over 100
complete-original comparisons:

- 48 cases used `50.0f / native 0.05f` across three x87 precisions, four x87
  rounding modes, two actual CRT modes and empty/three-occupied x87 stacks.
- 44 edge cases covered both CRT modes, signed reset threshold, negative
  captured counters, both wrap endpoints and adjacent values, first-sample
  priority, second-only/zero selection, maximum observed input, and subnormal,
  infinity, NaN, overflow and negative timing operands. Reset cases supplied
  signaling time operands that the selected path must not evaluate.
- Eight helper cases used full DWORD arguments `20000`, `80000000`,
  `7FFFFFFF` and `FFFFFFFF` in both CRT modes, separately from the observed
  unsigned16 producer domain.

Comparisons included both histories and all their samples, primary ownership
identities, preserved outer target/secondary history bytes, unchanged borrowed
cells, x87 CW/SW/FTW and all eight 80-bit register images, MXCSR and all 16
bytes of XMM0. Instruction/data-pointer addresses were excluded. Exceptions
were masked, DAZ/FTZ disabled and sufficient x87 slots available. Exceptional
timing values are valid-storage helper fixtures, not a claim about the native
time producer or whole runtime reachability.

The actual Win32 COFF function is `E0h` bytes. Its only three REL32 external
references are reset at operand offset `2F`, the existing converter at `5A`,
and append at `D5`. COFF disassembly confirms one captured-step load, the
FLDZ/FSTP reset argument, and FLD at `4F`, FDIV at `54`, conversion call at
`59` without an intermediate floating spill. Both normalization chains and
sample priority were inspected in generated assembly and COFF disassembly.

Eight actual translation units and 31 immutable source/header inputs were
freshly compiled with a separately hashed probe and embedded `asInvoker`
manifest. Three support libraries were frozen at main `e7ce15904` before the
authorized registry rebuild. The linked copies and all inputs stayed unchanged
through execution. Main's later `bsp_core.lib` changed, as expected; its new
hash is recorded separately and was not substituted into the probe link.

No tracked tests, Ghidra writes, caller/derived lifetime, socket behavior,
whole native ABI, unmasked fault behavior or gameplay validation are included.
Primary registration, full main build and integration are separate work.
Evidence is in `reports/cc11_session_target_tick_history.json`; reproducible
ignored artifacts are under `local/cc11_target_tick_history_20261007_a/`.
