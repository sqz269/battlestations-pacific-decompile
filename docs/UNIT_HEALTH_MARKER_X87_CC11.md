# Unit health marker under ambient x87 precision (cc11_health_marker)

Native fragment `00877C00..00877C20` in `00877B90`; original function ABI is
ECX this, stack float request, RET 4. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the standard client before read-only
listing/byte queries. No Ghidra or ledger mutation was performed by this worker.

The marker predicate now executes the native x87 float-load/double-subtract
sequence using the caller's actual control word. This fixes the four data-result
disagreements found by `cc11_health_nan`. It does not change the setter's equality
or clamp rules, getter, callback timing, replication, or host behavior switches.

## Recovered sequence

| Native site | Operation |
| --- | --- |
| `00877C00` | `FLD float [stack stored health]` |
| `00877C04` | Store health to `this+370h`; this remains the surrounding setter's job. |
| `00877C0C` | `FLD float [this+36Ch]` (maximum) |
| `00877C12` | `FSUB double [00D7A210]` (1.0) under ambient x87 PC/RC |
| `00877C18` | `FXCH ST1` |
| `00877C1A` | `FCOMIP ST0,ST1`, compare stored health with maximum-minus-one and pop stored |
| `00877C1C` | `FSTP ST0`, remove the threshold |
| `00877C1E` | `JBE`, skip marker write for less/equal/unordered |
| `00877C20` | Set `this+2E8h=-1` only when the branch did not skip; no reset/else write. |

The operand at `00D7A210` was reread live as
`00 00 00 00 00 00 f0 3f`, double 1.0. Its type does not force the x87 arithmetic
precision to double: the subtraction still uses the active precision control.

The private helper `full_health_marker_00877c00` contains the two float loads,
double `FSUB`, `FXCH`, `FCOMIP`, and `FSTP` in that order. It finishes with `SETA`,
the complement of the native `JBE` predicate: CF=0 and ZF=0. Unordered has CF/ZF
set and therefore produces false. `FSTP` does not change those integer flags.
The returned bool remains the existing request to perform the set-only marker
write; it does not represent the marker slot's persistent contents.

The kernel adds two temporary x87 stack entries and consumes both. It has no
`FLDCW`, control-library call, MXCSR write, algebraic shortcut, or SSE substitute
for the subtraction. MSVC Win32 inline assembly is required, consistent with
existing native kernels in this repository; unsupported targets fail explicitly.
The public header and layouts are unchanged apart from comments.

## Data results and validation

The existing `cc11_health_nan` probe was reused as
`local/cc11_health_marker_probe.cpp`, retaining all 28 input rows and all 12
precision/rounding settings. The reference is the previously reviewed assembly
transcription of the native setter prefix, not the new helper. Its existing
getter/CRT comparisons continue to use the recovered shared CRT assembly.

| Previous witness | Native / corrected marker | Previous double projection |
| --- | --- | --- |
| Current 0, request=max=2^25; PC24 nearest (`007Fh`) | False | True |
| Same values; PC24 upward (`087Fh`) | False | True |
| Current 0, request=max=2^54; PC64 nearest (`037Fh`) | True | False |
| Same values; PC64 upward (`0B7Fh`) | True | False |

At PC24 nearest, `2^25 - 1` rounds back to `2^25`; at PC64, `2^54 - 1` remains
distinguishable. The hardware kernel follows the active control word without
guessing it from initialization or renderer policy.

One strict Win32 build/run of the reused probe passed:

- 336 prefix data and marker comparisons: zero storage/eligibility failures and
  zero marker differences, including the four previous disagreements.
- 672 client/host data and eligibility checks and 240 comparisons with both CRT
  byte paths still pass. The previously corrected NaN storage bits are retained.
- 336 state checks preserve the complete x87 control word and MXCSR control
  bits. Before each source call the probe loads two exact live sentinels; TOP
  and both sentinel values are unchanged afterward. This checks stack balance
  and preservation of older stack entries, not just final TOP at an empty stack.
- Generated optimized assembly retains the exact FP sequence in the standalone
  helper and both inlined setter paths, with no control-word manipulation.

The matrix covers ordinary finite values, signed zeros, a least-positive
subnormal request, infinities, and positive/negative quiet/signaling NaNs in the
current/request/maximum roles represented by the existing rows. It uses PC24,
PC53 and PC64 with all four rounding modes, all x87/MXCSR exceptions masked,
matching x87/MXCSR rounding, and DAZ/FTZ disabled. It is a bounded matrix, not an
exhaustive set of binary32 bit patterns or all floating-point environments.

Command: `cmd /c local\cc11_health_marker_check.cmd`. MSVC Win32 flags:
`/std:c++17 /EHsc /O2 /Gy /W4 /WX /fp:strict`; linker
`/MANIFEST:EMBED /OPT:REF`. The probe returns 0. Original NaN-packet probe/log/
assembly artifacts were preserved; marker artifacts use separate names/folders.
Hashes, native bytes, emitted-assembly locations and counts are recorded in
`reports/unit_health_marker_x87_cc11.json`.

## Retained boundaries

This is a private numeric kernel in the typed setter, not a native ABI entry or
a complete original-routine execution. The original health memory store lies
between the two loads; the projection stores its output before calling the
kernel. The host's existing timing, mutable native object access and callbacks
remain outside this data-result claim.

Unmasked exceptions, trap/resume behavior, x87 status/FIP/FDP and integer-EFLAGS
parity are not established. The kernel executes exception-producing x87
instructions and does not clear flags; the full source routine still uses other
instruction domains for its preceding comparisons and subsequent operations.
The stack claim assumes capacity for the kernel's two temporary entries; stack
overflow/underflow behavior was not separately tested. DAZ/FTZ variants and
independently different x87/MXCSR rounding modes were not tested.

The getter's floor/cap/cache omission, division/spill precision projection,
finite replication overflow limits, and callback ownership/timing limitations
from `UNIT_HEALTH_UNORDERED_CC11.md` remain. The known getter input-fraction-2
case still returns 2 in the typed getter versus 1 in the native getter/cache;
the probe reports it explicitly without treating it as a marker failure.

No new tracked test suite, CMake edit, game run, or policy switch was added.
Full integrated build/CTest remains the primary integrator's responsibility.
