# CC12 Native mission-clock reset Source

Baseline: `12618f75a00d785a39467500fc952160bd859dc0`.
Worker implementation; **zero new Original credit pending primary review**.

## Implemented boundary

`reset_native_mission_clock_00874640(uint32_t input_float_bits,
const NativeMissionClockResetContext&)` implements the complete ordinary
`00874640..00874690` reset/set sequence over seven borrowed references:

- Writable clock `F876A4`, accumulated value `F876A8`, remainder `F876AC`,
  count DWORD `F876B0` and interpolation value `F876B4`.
- The qualified actual binary64 step operand at `D7A270`.
- The actual current conversion-selector DWORD at `0109EEA4`.

The context owns no storage. All five writable cells must be distinct members
of the same admitted clock state, and all seven referents must outlive the call.
No application caller or common clock owner is bound by this change. The prior
clock-owner and reset-provider audits remain the ownership/readiness boundary.
This is a general input-bit implementation, not a zero-only reset shortcut.

## Address adapter and kernel

The public wrapper constructs a private seven-pointer `ResetAddresses` from
the supplied references. It takes addresses only, without reading clock, step
or mode values, and passes the raw DWORD to one naked MSVC Win32 assembly kernel.
The private pointer layout has size/offset assertions and is neither a clock
owner nor an overlay of the public reference-bearing context or Native globals.

The kernel receives its address block in ECX, ignores incoming EDX, and receives
the raw input DWORD in its stack argument slot. It saves ESI and retains the
address block there across the existing raw converter. Saving ESI moves the
unchanged input slot from `[ESP+4]` to `[ESP+8]`; both original input reads use
that same slot. ESI is restored on the ordinary return. The existing converter
and fallback preserve ESI; there is no new CRT conversion implementation.

The kernel has 28 explicit Source assembly instructions: the 18 Native steps
and ten integer address/save/restore instructions. No emitted byte count or
compiled instruction count is claimed at this worker stage. Its floating and
bit-store order is:

1. `FLD` the input before any output store. Read the original DWORD separately
   with `MOVSS`, duplicate T on the x87 stack, and store the unchanged bits to
   the actual `F876A4` cell.
2. Load actual `D7A270` as binary64; store the same input bits to actual `F876A8`.
3. Execute `FDIV ST1,ST0` and `FXCH`, retaining S and T below the quotient.
4. Load the borrowed mode address into ECX and directly call existing
   `native_crt_truncate_st0_00bf7420`. The existing helper reads the current
   DWORD and consumes only the quotient, leaving S and T on the x87 stack.
5. Store EAX into actual `F876B0`, then signed-`FILD` that same cell. Multiply
   the loaded count by retained S; use `FSUBP ST2,ST0` and `FXCH` for the remainder.
6. Round/store to actual binary32 `F876AC`; subtract that **stored** remainder
   from retained S; round/store the result to actual binary32 `F876B4`.

The Source retains all 12 original x87 operations, three `MOVSS` operations,
the count store and the direct converter call. It adds no floating spill,
mode snapshot, C++ float argument conversion, C++ numeric cast, guard, clamp,
callback, catch, rollback, `noexcept`, control-word change or private timeline.

Using a DWORD input matters for signaling NaNs: the initial x87 load can process
or fault on the input, while the separate `MOVSS` copies still preserve its raw
bits. A C++ float forwarding conversion before that load would not establish
that order. The existing raw converter also preserves the distinction between
its SSE2 signed32 path and x87 signed64 fallback; this reset consumes only EAX.

The step's required bytes are `00 00 00 A0 99 99 A9 3F`, exactly
`0.05000000074505806` (`0x1.99999a0000000p-5`). This equals the promoted binary32
step at `D0DE84`; ordinary binary64 literal `0.05` has different bits. No local
step constant or value conversion substitutes for the supplied actual operand.

## ABI and ownership limits

The private kernel uses `RET 4` for its own DWORD argument. The public function
is the documented new C++ interface with an explicit context; it does not admit
original call-site stack/register compatibility, returned EFLAGS or Windows SEH.
The public wrapper and integer address adaptation can differ in scratch-register
and flag effects. No native drop-in, runtime, game or application validation is
claimed. The operation supplies no initialization, reset/increment schedule,
shared clock publication, synchronization or ownership/lifetime service.

The existing canonical CRT feature-word provider remains available to qualified
callers, but this function only borrows its supplied reference. It does not
acquire that owner, use a hardware-feature query or default the selector. The
existing raw kernels retain their established x87/MXCSR behavior; this change
introduces no handling or normalization of unmasked hardware exceptions.

## Verification and integration boundary

Static Source inspection checked the 28 kernel statements, the original 18
address annotations in order, all 12 x87 operations, exactly one direct converter
call, the seven references and their private address bindings. The report pins
both new Source files, the existing raw converter header/body and the two
inherited audits. The reset/converter/constant byte evidence is inherited from
the primary-replayed readiness audit: four live/PE windows totaling 121 bytes.
No new Native queries, BF7456 queries, build, test, probe or execution occurred.

This packet contains only the new header, implementation, this document and its
report. CMake registration, compilation, complete emitted-kernel/callee review,
ledger credit and any later caller integration belong to the primary integrator.

## Primary integration and emitted review

The integrator registered this unit in bsp_core. The first normal build rejected the single-operand `fmul st(1)` with C2415. The explicit `fmul st(0), st(1)` spelling emits the required `D8 C9`; the failed receipt and old source pin are retained. The corrected normal MSVC Win32 build and all three existing checks passed.

Complete emitted review covers the 49-byte / 17-instruction pointer-only public wrapper, 74-byte / 28-instruction private kernel, all 18 native sites and physical concrete converter/fallback definitions. The kernel retains the 12 x87 operations, three MOVSS operations, current mode read and one raw converter call, then restores ESI and cleans its private DWORD. This admits one new 81-byte Original function. Its new explicit-context API remains unlinked to the game; actual common clock/reset/step authority, Native entry/EH/fault execution and gameplay remain unproved.

Evidence: `reports/cc12_world_tick_reset_frame_primary_review.json`; complete retained artifacts under `local/cc12_world_tick_reset_frame_primary`.
