# CC12 mission-clock reset provider readiness

Baseline: `4652b343c640eb3f4f955f9323ae657e44752379`.
Read-only composition audit; **zero Source changes and zero new Original credit**.

## Decision

The complete ordinary `00874640` body has a concrete existing conversion
dependency. A bounded Source reset pair is ready to implement over explicitly
borrowed cells. No new converter, private clock, global value, virtual callback
or link stub is needed. This is readiness for the reset operation only:
`CC12_WORLD_MISSION_CLOCK_OWNER_READINESS.md` still leaves the common mission
clock owner, write/reset wiring and application lifetime unqualified.

The older `00BF7420` ledger points to `gui_group_bounds.cpp`. Current callable
Source is `native_crt_truncate_st0_00bf7420` in `native_render_batch_keys.cpp`,
publicly declared in its matching header and already registered in `bsp_core`.
`gui_group_bounds.cpp` now calls that shared body. Its zero-mode target is the
complete public `native_x87_truncate_st0_00bf7456`, in the same Source file.
Neither the old GUI wrapper nor `instance_sort`'s binary32 numeric projection
should be substituted for these raw entries.

## Concrete converter contract

| Entry | Input and selection | Result and stack effect |
| --- | --- | --- |
| `native_crt_truncate_st0_00bf7420` | Assembly caller supplies the operand in ST0 and the actual `0109EEA4` address in ECX; the helper reads the DWORD at the call. | Nonzero selects `FSTP double` then `CVTTSD2SI EAX`; zero tail-transfers to the complete raw `BF7456` helper. Consumes the supplied ST0. |
| `native_x87_truncate_st0_00bf7456` | Receives the same extended ST0 operand under the current x87 environment. | Executes the existing duplicate/FISTP/reload/residual/correction sequence, returning signed-integer bits in EDX:EAX and consuming its input. |

The reset consumes **only EAX** and subsequently interprets that stored DWORD
with signed `FILD`. It does not use the fallback's high EDX word or replace its
result with signed32 saturation, a C++ cast, `trunc`, or a zero-input shortcut.
The two selector branches can differ for invalid/range inputs and in floating
status effects; the selector must remain live at the original call point.

Both current raw Source paths preserve the deeper x87 values. The SSE2 path
pops only ST0. The fallback duplicates its input and consumes both temporary
values on every return path: the residual path pops its difference, and the
zero/indefinite shortcut performs both `FSTP` stores. Thus reset's retained step
and original loaded input survive below the converted operand. The helpers do
not modify ESI, EDI or EBX; an address adapter can keep its borrowed context in
a properly saved callee-preserved register. A normal C++ call expression does
not supply this unusual ST0 ABI; the call must remain inside x86 assembly.

`native_dyn_convex_support.cpp` already uses this same public converter while
retaining deeper x87 values. That is an existing composition pattern, not new
validation of the proposed reset. The full `BF7456` Source and its existing
qualified ledger contract were inspected; no new live `BF7456` body capture,
fixture, reconstruction or address claim was made.

## Complete reset order and retained x87 values

`00874640..00874690` is 81 bytes / 18 instructions. Original input is one
binary32 DWORD at `[ESP+4]`; ECX is unused; the body ends in `RET 4`.
Let `T` mean the x87 value produced by the initial float load, `S` the exact
binary64 step, and `Q = T/S` under the current x87 environment. The input DWORD
bits are separate from `T`: a masked signaling-NaN load can alter the x87 value
while the subsequent `MOVSS` copies still retain the original input bits.

| Native point | Required operation | x87 stack after the operation, top first |
| --- | --- | --- |
| `874640..87464C` | `FLD` input, `MOVSS` original bits to XMM0, duplicate ST0, then copy XMM0 to `F876A4`. | `T, T` |
| `874654..87465A` | Load exact binary64 `D7A270`, then copy XMM0 to `F876A8`. | `S, T, T` |
| `874662..874664` | `FDIV ST1,ST0`, then `FXCH`. | `Q, S, T` |
| `874666` | Call the existing raw converter with the actual current mode address. | `S, T` |
| `87466B..874676` | Store EAX to actual `F876B0`, signed-`FILD` that same cell, then multiply by retained S. | `count*S, S, T` |
| `874678..87467C` | `FSUBP ST2,ST0`, `FXCH`, then round/store the remainder to actual binary32 `F876AC`. | `S` |
| `874682..874688` | Subtract the **stored** binary32 remainder from S; round/store to actual binary32 `F876B4`. | Original incoming stack depth restored. |

Preserve every load, duplicate, arithmetic operation and explicit spill/reload.
Do not carry an unrounded remainder through the last subtraction, replace the
stored-count `FILD` with a different numeric conversion, spill S/T around the
raw converter, or replace the generic body with five assignments for zero.
The first `FLD` precedes the clock stores; an unmasked fault there must not be
preceded by eager clock writes. This body changes none of the stepping byte,
buffer index or previous-buffer word and supplies no reset/increment schedule.

The actual step operand is `00 00 00 A0 99 99 A9 3F` at `D7A270`:
`0x1.99999a0000000p-5`, exactly `0.05000000074505806`. It equals binary32
`CD CC 4C 3D` at `D0DE84` promoted to double. Ordinary binary64 literal `0.05`
has different bits. Borrow the qualified actual step operand rather than using
that literal or a newly rounded intermediate. Both exact operands were checked.

## Minimal proposed Source pair

Proposed files only; this packet creates neither:

- `include/bsp/native_mission_clock_reset.hpp`
- `src/native_mission_clock_reset.cpp`

A sufficient non-owning public context and new C++ interface are:

```cpp
struct NativeMissionClockResetContext {
    volatile float& clock_00f876a4;
    volatile float& accumulated_00f876a8;
    volatile float& remainder_00f876ac;
    volatile std::uint32_t& step_count_00f876b0;
    volatile float& interpolation_00f876b4;
    const volatile double& step_00d7a270;
    const volatile std::uint32_t& conversion_mode_0109eea4;
};
void reset_native_mission_clock_00874640(
    std::uint32_t input_float_bits,
    const NativeMissionClockResetContext&);
```

All five writable references must designate their distinct actual cells in the
same admitted clock state. The step and mode references must designate the
qualified existing operands and outlive the call. This aggregate owns no cells,
initializes no state and does not bind current independent host clocks together.
The caller must establish the common owner's identity and lifetime; the earlier
clock audit did not establish that application composition.

Use raw input bits so a C++ float forwarding conversion cannot quiet a signaling
NaN or round/change the argument before the Native first load. Existing raw-bit
camera and bit-cursor APIs supply precedents. Prepare only borrowed addresses
before entering one contiguous MSVC Win32 assembly kernel, with a stable DWORD
argument slot for the original two input reads. Keep the existing direct raw
converter call within that kernel. Integer address glue must not introduce
extra floating operations, value snapshots, mode reads, guards or callbacks.
An implementation can pack addresses privately; it need not assume a native
layout for the reference-bearing C++ context.

## Existing actual conversion-mode provider

`GameNativeCanonicalDataOwner::mutable_crt_data().feature_word()` returns the
canonical volatile DWORD at `0109EEA4`. Its owner is intentionally retained for
process lifetime. Current `game_main.cpp:1036` borrows this exact reference and
passes it to Dyn convex startup at line 1119. The Dyn process subsequently
returns the same borrowed conversion reference and rejects a different binding.

This is a concrete Source provider for the selector, conditional on its existing
canonical initialization succeeding. Read its current DWORD in the converter;
do not cache a bool or substitute `IsProcessorFeaturePresent`, a private host
feature field, or a presumed startup value. No live selector value was read or
assumed here. The canonical CRT owner does not supply the five mission-clock
cells or make that separate owner ready.

## ABI, control state and validation limits

The proposed public function is a new C++ ABI with an explicit context and raw
input word. Original `RET 4`, incoming scratch registers and returned EFLAGS
are not automatically reproduced by a C++ wrapper. Integer prologue/epilogue
instructions can change flags even when floating instructions match. Any future
claim about consumed caller flags or a drop-in call boundary needs emitted-code
and caller validation; this audit makes neither claim.

Reuse the existing hardware converter under the caller's x87 control word and
MXCSR; do not change precision, rounding or exception masks. The selector word
and those thread control states are separate inputs. No catch, rollback,
synthetic success or owning cleanup is proposed around partial stores. Hardware
faults and Windows SEH/unwind behavior are not established by the new C++
interface, and volatile access does not provide synchronization or lifetime.

Verification: four live-Ghidra/installed-PE windows total **121 bytes** (81-byte
reset, 28-byte selector/SSE2 body, eight-byte step and four-byte cross-check),
14 Source/build-file pins plus two inherited audit pins, JSON decoding and diff
checks. No Source, GPR, ledger or CMake mutation; no build, test, probe or runtime
execution. The reset is ready for bounded Source implementation, not implemented,
ABI-compatible, shared-owner-qualified or application-ready by this packet.
