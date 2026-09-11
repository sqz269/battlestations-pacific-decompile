# Native renderer gamma discovery

The complete gamma setter at `00B21960..00B21B3A` is 474 bytes. Its renderer/guard/ramp schedule is recoverable, but **an original-equivalent full source interface remains blocked by the power helper**. No complete raw pow provider exists in the pinned checkout. A modern host `pow` could support an explicitly different, restricted numerical interface; its existence is not evidence that it reproduces the original WORD ramp or floating-point side effects. The one independent exact source packet proposed below is the 67-byte CRT double loader C083D5.

This is read-only discovery. No source, Ghidra, ledger, installed game, monitor gamma, window, focus, build or runtime operation was changed or exercised. [The audit](../reports/native_renderer_gamma_discovery.json) pins the two-file packet and ignored `local/gamma/` evidence in worktree `J:/PROG/battlestations-pacific-decompile-native_renderer_gamma_discovery`.

## Extent, ABI and concrete storage

Fresh live Ghidra program bytes and the installed PE agree on the full body, SHA-256 `395daf39c6ea3f1871fd11d6aea7b1d6e5d9d2005f6d876b39fd3ffc26ef894a`. The preceding B218C0 pixel-constant function ends with `RET 0Ch` at B2195D, exactly before B21960. Gamma ends with `RET 4` at B21B37, followed by six CC bytes and the next DrawPrimitive function at B21B40. Ghidra has no function beginning at B21960. The index's “enclosing candidate B218C0” is a nearest-start artifact, not evidence that gamma belongs to that function. Original table cell D5F198, renderer D5F0A8 +F0h, contains B21960 and has a direct data reference to it.

Original ABI: ECX is actual renderer storage, with one float in the actual callee argument slot at entry ESP+4; `RET 4`. The routine saves EBX/EDI and temporarily ESI in the generation path. It has no recovered useful return value. After its ordinary prologue and two saved registers, the requested slot is `[ESP+62Ch]`, guard bytes are +10h/+14h, EH state is +624h, and the ramp begins +1Ch. These are original stack locations, not fields in any semantic camera/renderer/options object.

The consumed renderer fields are cached requested float +196Ch, current device +1A10h, enabled byte +1B53h, and calibration byte +1B54h. Only raw current bytes/pointers are justified. B2C8E0's pinned GetDeviceCaps/writer prefix stores `(Caps2 >> 20) & 1` at +1B54h and `(Caps2 >> 17) & 1` at +1B53h. B32410's pinned XOR EBX and later MOV-byte stores initialize both flags to zero. No cache-zero initialization assumption is established here. Existing `renderer_capabilities.cpp` provides a typed capability result; it does not justify casting that companion as raw renderer storage. `renderer_startup.hpp` only declares abstract `renderer_set_gamma(float)`; there is no existing gamma implementation to reuse.

## Guard, input and current device order

1. Test current global byte 108D6DCh. If enabled, write the captured renderer to guard+4, call complete B33AD0, then write returned AL to guard+0. Entry exceptions occur before arming. If skipped, native still loads EBX from the uninitialized saved-renderer cell; no initialized substitute is justified.
2. Read current renderer +1B53h **before** writing EH state 0. A disabled flag skips all cache/input/ramp/device work and takes the current-mode cleanup route. It does not skip the optional guard entry.
3. FLD cached +196Ch, FLD the actual requested callee slot, duplicate/exchange x87 stack values, and compare cached versus requested with FUCOMIP. FSTP/LAHF/TEST AH,44h/JNP retains one requested value on x87 and skips only ordered numerical equality. Signed zeros compare equal; unordered comparisons proceed. The equal path pops its remaining requested value before cleanup.
4. For changed/unordered input, FADD double 10 to the retained first x87 input. **Then separately reread the current callee slot with MOVSS and store those raw bits to cache +196Ch**, before FDIV by 20 and the normalization float spill. A fixed requested-value snapshot would erase this distinction: computation uses the first x87 load, while cache publication uses the later raw slot read. An exception can occur before or after publication.
5. Generate all 256 ramp entries. Only after generation, compare current calibration byte +1B54h, capture current device +1A10h, load its current vtable and +54h selector, restore ESI, and form the ramp address. The earlier calibration comparison supplies flags 0 or 1; it is not reread after device/table loads. Call actual `SetGammaRamp(device, swap_chain 0, flags, &ramp)` using stdcall. This method returns void. No HRESULT or rollback policy exists.
6. Test current synchronization mode before setting state -1. If enabled, pass the saved DWORD at guard+0 and the EBX-captured guard renderer to complete B33B00. Normal cleanup is disarmed before this call. The callee ignores the saved result but rereads current mode and renderer+4 lock according to its existing raw contract.

The skipped-entry/uninitialized-guard boundary is the same as existing raw synchronization: an asynchronous transition that creates enabled cleanup after skipped entry is outside a valid source domain. No new mode normalization, stable lock/device snapshot, or whole-body RAII guard should be invented.

The exact EH record is handler CBCDDB -> FuncInfo DF5374, one state, map DF536C `{previous=-1, action=CBCDD0}`. CBCDD0 is the complete 11-byte `LEA ECX,[EBP-618h]; JMP B21110`, using the original guard record. The existing full B33AD0/B33B00/B21110 providers and `NativeRendererSynchronizationGlobals` satisfy the guard dependency. The state is armed across cache/ramp/device work. It does not add cache rollback, ramp cleanup, or an x87 control-word restoration on an interrupted conversion. A future C++ exception interface must state its native SEH/stack-layout limits separately.

## Exact outer arithmetic schedule

| Literal | Type/value | Original little-endian bytes |
| --- | --- | --- |
| CE3DC0 | double 10 | `0000000000002440` |
| CE3D88 | double 20 | `0000000000003440` |
| CE4B48 | double 255 | `0000000000e06f40` |
| CE5F90 | double 65535 | `00000000e0ffef40` |
| D7A24C | float 1 | `0000803f` |

These are all absolute floating-point literal reads in the gamma body. FLDZ, FLD1 and XORPS supply the other zero/one values directly. Literal addresses and access order must be represented by actual borrowed constants or an explicitly declared relocation contract.

The first input undergoes x87 FADD10/FDIV20 without an intervening float spill, then FSTP float. Lower clamping compares x87 zero to that float using FCOMIP: ordered negatives select XORPS positive zero; nonnegative or unordered values take the upper path. Upper COMISS against float 1 and JBE retain less/equal or unordered values; strictly greater selects 1. A generic clamp does not capture NaN quieting, exception masks, or MXCSR behavior.

Reload the chosen float, double it with FADD ST0,ST0, use FLD1/FDIVRP for the reciprocal, and spill the exponent to float32. For each integer i=0..255, FILD i / double255, spill coordinate to float32, reload it, then FLD the float32 exponent. At B21A57 the pow entry receives **ST1=coordinate, ST0=exponent**, not C stack arguments. It must consume both and return one result in ST0. Spill the returned result to float32, reload it and multiply by double65535; there is no final float spill before integer conversion.

Increment i, FNSTCW the current word, OR only rounding-control bits with 0C00h, FLDCW, and FISTP signed DWORD. Store the low WORD into **blue, green, restore the saved control word, then red**, in that order. The loop uses its earlier signed integer comparison and fills exactly 256 entries per channel. There is no saturation, output clamp, or nearest-integer conversion. Unmasked faults can interrupt before the explicit control-word restoration; the EH action only destroys the renderer guard. The body does not normalize precision control, MXCSR, sticky FP status or thread errno.

For ordinary masked arithmetic, input <=-10 reaches zero normalization and an infinite reciprocal; input >=10 reaches exponent 0.5; input zero reaches exponent 1. These observations do not determine missing pow exceptional behavior or justify replacing the ramp with `i*257`. NaNs, infinities, signed zero, denormals, altered precision/rounding and masked versus unmasked conversion require the original instruction/helper contract, not host casts.

## Power provider readiness

BFEB10 is a complete 84-byte register/x87 entry. It checks current DWORD 109EEA0, then `(MXCSR & 1F80h)==1F80h`, then `(x87 CW & 007Fh)==007Fh`. The masks do **not** constrain rounding bits, x87 precision bits, DAZ or FTZ. The initial image contains zero at 109EEA0, but pinned CRT instructions C0AC8F/C0AC9B modify it; the initial image is not a runtime policy.

If all tests pass, BFEB10 tail-jumps to C19260. This complete 25-byte wrapper aligns its stack to 16 bytes, exchanges operands, spills/pops coordinate then exponent as doubles, and calls C19279. The 2,872-byte SSE2/table-driven core has no current source implementation. Its full bytes are pinned for extent/identity and its entry, return paths and direct call were inspected; its coefficient-table extents and complete polynomial/error semantics were not reconstructed. Its direct error provider C0F0E4 (`___libm_error_support`, 634 bytes) is also unimplemented, with `_errno` BFFB8B and C04FDE dependencies. Complete `legacy_crt_87except_00c27489` is not a substitute for that distinct route.

Otherwise BFEB10 allocates 14h bytes, exchanges operands, spills/pops coordinate as double, stores exponent as double while keeping it on x87, loads the exponent high DWORD into EAX, and calls BFEB6D. That complete 453-byte fallback changes/restores the current x87 control environment, classifies operands and uses FYL2X plus C08390 for ordinary positive values. It also reads current 109DD78, passes actual string E15858 (`"pow"`) and operation 1Dh to error helpers, and has zero/negative/nonfinite routes. Its direct named dependencies are BFED32, C08330, C08390, C083A5, C083D5, C0842E, C08479 and C19E24; no complete public raw fallback provider exists. Half at E15850, positive-infinity80 at E166D0 and negative-NaN80 at E15C70 are pinned. The original SSE2 core's additional table data remains an explicit prerequisite.

`system_camera_axes.cpp` privately implements instruction tails corresponding to C083A5/C0842E and a camera dispatch beginning at C08347. The pow route calls C08330 and needs its own complete input/error schedule. Those camera helpers and the concrete LegacyCrtMathRuntime binding must not be relabeled as full pow closure. No generic error callback, FSQRT-style arithmetic substitute, or stubbed unreachable branch is proposed here.

Disk-only inspection confirms current SysWOW64 `ucrtbase.dll` exports `pow`, `powf` and `_errno`; the installed SDK declares `double __cdecl pow(double,double)`. This is a callable host numerical service, but no original/host pow byte, result, FP-status, errno or matherr proof was performed. A restricted translation could use fixed host double pow with explicit x87 float spills and conversion, for example finite cache/request values and a requested range strictly inside the zero-normalization boundary, masked exceptions and fixed rounding settings. Even a conservative interval such as [-9,9] avoids domain/overflow endpoints but still admits inexact arithmetic and integer-boundary sensitivity after multiplication/truncation. Calling host pow changes the promised provider contract; it cannot currently close B21960 as original-equivalent. Such a deliberately different numerical interface would require separate approval and a narrowly stated result/FP contract.

## One ready independent source proposal

Propose `native_crt_x87_double_load1`: only **C083D5..C08418 [67 bytes]**, with four new `native_crt_x87_double_load` source/header/doc/audit files. The retained original library name is `__fload_withFB`. It is unleased at the recorded check. No implementation or extra address lease is included in this discovery.

Suggested declaration: `std::uint32_t __fastcall load_native_crt_double_x87_00c083d5(void* unused_ecx, const void* actual_double);`, implemented as a naked complete original sequence. Original input is EDX pointing to eight readable bytes; EAX and flags carry classification while one x87 value is pushed. Assembly callers must explicitly consume ST0/flags; this is not an ordinary C++ double-return API. Finite and exceptional routes reread actual high/low words at their native positions. The nonfinite route builds a ten-byte temporary and FLDs it before the final current high-word reload; a snapshotted double or ordinary conversion is not equivalent.

This helper has no calls, absolute relocations, globals, allocation, profile or lifetime dependencies. Its entire 67-byte COFF body can be compared directly to the freshly pinned original, with no new broad test suite. It supplies one exact prerequisite and does **not** by itself close either pow core or the gamma parent. Full renderer initialization remains blocked until the genuine remaining power/error dependencies are resolved or an explicitly different numerical contract is accepted.


Primary verified all 50 sealed worker files, 63 additional report pins and 38 freshly guarded spans (4,862 bytes). Full gamma remains blocked by genuine x87/SSE2 power and error providers; no host pow substitution or display/runtime action. Exact C083D5..C08418 loader source approved separately after full primary live disassembly. Immutable discovery evidence: `local/gamma_discovery_primary/`. No source, original-body execution or gameplay claim is added by this review.
