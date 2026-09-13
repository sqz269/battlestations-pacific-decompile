# Gameplay artillery throw rates

Packet `orch6_throw_rates_m` corrects three stores in the existing partial
`load_gameplay_tuning_settings` projection. The Lua keys name durations, but
settings `+38h/+3Ch/+40h` contain reciprocal rates. Existing member identifiers
and the canonical `GameplayTuningSettings` owner are retained for the reconstructed
`0095DC40` consumer. No GameHosts or consumer implementation is changed here.

## Native producer and coverage

The live target was verified as `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through the read-only BSP wrappers.
`BSP_GameSettings_LoadFromLuaGlobals` owns inclusive body
`0083B5E0..00842951`. Its existing name is retained. The only observed direct
caller is `00424BFC` in constructor `00424A10`: `MOV ECX,ESI` at `00424BF5`
supplies the settings object, with no stack arguments. In the loader,
`0083B604` copies ECX into ESI; the full listing has no later ESI write until
the epilogue restores it at `0084294B`. The loader ends with plain `RET` at
`00842951`. Ghidra's displayed no-argument prototype omits this receiver.

The new C++ API is
`store_gameplay_artillery_throw_rate_0083da26(float duration, float& rate) noexcept`.
It implements the arithmetic and output store in the three inclusive ranges
below. It is a source interface, not a binary-compatible replacement or a
complete reconstruction of the surrounding loader.

| Lua key under PlayerArtilleryThrow | Input getter call | Duration fallback | Arithmetic/store range, inclusive | Final float store |
| --- | --- | --- | --- | --- |
| ThrowIncrementTime_HasTarget | `0083DA21` | 12 | `0083DA26..0083DA66` | `0083DA64`, settings `+38h` |
| ThrowIncrementTime_NoTarget | `0083DA96` | 8 | `0083DA9B..0083DADB` | `0083DAD9`, settings `+3Ch` |
| ThrowDecrementTime | `0083DB0B` | 10 | `0083DB10..0083DB50` | `0083DB4E`, settings `+40h` |

Each final `FSTP` is three bytes. The parent row starts at `0083D94A`;
`AfterShot_FireTime` and `AfterShot_WaitTime` remain raw durations stored at
`0083D99D` and `0083D9E3`, settings `+30h/+34h`. The last child-wrapper
destructor is the five-byte call at `0083DB51`, ending at `0083DB55`.
`0083DB56` begins the next row. There are no missing Ghidra functions or
observed flow gaps in these covered ranges.

## Clamp, reciprocal, and input boundary

For the first rate, `DA26` spills the getter's ST0 to float32, `DA2A` reloads
it, and `DA2E` loads double `00D7A3A0`. Its bytes
`00 00 00 A0 99 99 B9 3F` encode the exact widening of float `0.1f`,
`0.100000001490116119384765625`; this is not decimal double `0.1`.
`DA34 FCOMIP ST0,ST1` compares that minimum against the duration, then
`DA36 FSTP ST0` removes the duration. `DA38 JBE` keeps durations at least
the minimum, and also keeps unordered NaNs. The other arm copies float
`00D7A2F0`, bytes `CD CC CC 3D`, into the duration slot.

`DA50 FLD duration`, `DA54 FLD1`, `DA5A FDIVRP ST1,ST0`, and
`DA64 FSTP float [ESI+38h]` calculate and store the reciprocal. The other
two schedules are identical. Negative values, signed zero, and negative
infinity select the minimum; positive infinity produces positive zero.
NaNs follow the native unordered branch and subsequent x87 propagation.
Inline x87 preserves the caller's precision/rounding controls for division
and the final float32 store. The API does not expose or reproduce native
SEH bookkeeping writes, wrapper temporaries, or arbitrary register state.

The called Lua bodies were inspected before interpreting the boundary:

- `00B67800`: ECX is the table reference, two stack arguments are output
  reference and key; `RET 8`. It performs the actual Lua table lookup and
  constructs a tracked `14h` reference. This remains the existing host/library
  contract, including metatable behavior.
- `00B66330`: ECX is the child reference, one float32 stack fallback;
  `RET 4`, result in ST0. Only valid references with exact Lua NUMBER type
  use the numeric value; other types use the fallback. It converts to
  float32 before returning. Numeric strings are not accepted by this getter.
- `00B67700`: ECX is the tracked reference, plain `RET`; releases the
  current reference where populated, then clears its current kind.

`GameplayTuningRowView::number_or` represents these operations as one host
read. Each corrected read, calculation, and output store completes before
the next read, preserving the represented partial output if a later read
fails. Native Lua panic/longjmp handling, wrapper destruction during failure,
and the full loader's SEH unwind are not implemented by this helper. The
existing complete C++ keyed projection is grouped by field offsets and
does not reproduce the full native loader order; its header now says so.
The prior key metadata in `reports/gameplay_settings.json` correctly locates
the getter/final stores, but its `installed` and `loader_default` values are
input durations, not stored rates. This document corrects that interpretation
without claiming unrelated loader coverage.

## Installed data and verification

The installed `Scripts/datatables/shipglobals.lua` has NoTarget `4.0`,
HasTarget `2.0`, Decrement `2.0` at lines 53..55. These produce stored
rates `0.5`, `0.25`, `0.5` at `+38h/+3Ch/+40h` under normal rounding.
The after-shot values at lines 56..57 are both `0.2` and remain durations.
This is inspected installed data and numerical evidence, not a live mission
execution of this loader.

The ignored fixture executes all original 524 bytes `0083D94A..0083DB55`.
Original relative branches and all arithmetic/stores remain intact in the
relocated block. All 16 Lua calls are redirected to explicit input/lifetime
test boundaries with native RET cleanup; a return is appended at `0083DB56`.
It compares the real rebuilt `load_gameplay_tuning_settings` through the
end of the same row. Native SEH byte stores use bounded scratch space;
the fixture does not register a native exception frame.

The comparison passed 240 cases: 20 exact float input patterns across all
four x87 rounding modes and 24-, 53-, and 64-bit precision, with exceptions
masked. Inputs include both zeros, finite values around `0.1f`, subnormal,
maximum finite, infinities, quiet/signaling NaNs, and the three fallbacks.
Every case compares five float32 output words, read-key sequence, x87
control word, complete status word (including TOP), and MXCSR. MXCSR starts
at `1F80h`; DAZ/FTZ variants, XMM register contents, EFLAGS, x87 tag words,
and the complete native register ABI are outside the comparison.

Six additional ordered-prefix cases stop before each of the five getters
or finish the row, using fallback durations. Native fixture `longjmp` and
C++ fixture exceptions compare already-written output prefixes and read
keys; they do not prove native exception/unwind parity. There are no new
tracked tests. `scripts/build.ps1` passed Win32 compilation and both
existing CTest checks. Direct-call verification is recorded in the report.

Preserved ignored evidence is under this worktree's `local/`:
`throw_probe.cpp`, `prepare_throw_probe.py`, `build_throw_probe.ps1`,
`throw_probe.exe`, `throw_probe.log`, original section/constants pages,
target listings, and `throw_rate_build.log`. `throw_rate_manifest.json`
records source, native input, and artifact hashes. The original section hash
is `ba1acedcbf9fe63dc677087fa91310f842fbbcee64c818f9badb42488d0d815d`;
the original executable hash is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
