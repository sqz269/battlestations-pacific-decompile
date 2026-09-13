# Actual camera projected axes and borrowed context

Addresses: `00B70EA0`, `00B70FE0`, `00B6FEB0`.

The new `native_camera_axes` module implements all three native bodies over the
actual camera address. These extend the existing complete typed function rows;
they are not newly discovered Ghidra functions. Existing descriptive names are
preserved as hypotheses. No Ghidra mutation is performed by this packet.

| Routine | Inclusive native body | Coverage | Original interface |
|---|---|---|---|
| `BSP_Camera_GetProjectedYAxis` | B70EA0..B70FD6, 311 bytes | complete | ECX camera, no stack arguments, EAX camera+440, RET |
| `BSP_Camera_GetProjectedXAxis` | B70FE0..B71116, 311 bytes | complete | ECX camera, no stack arguments, EAX camera+44C, RET |
| `BSP_Camera_GetContextDepthScale43C` | B6FEB0..B6FEB6, 7 bytes | complete | ECX camera, no stack arguments, EAX current borrowed +43C word, RET |

The axis APIs add their required concrete context in EDX. The original bodies
do not consume EDX as an input. The getter retains its original register and
stack behavior, but this packet does not claim drop-in binary compatibility.

## Actual storage and providers

Pass the actual camera slot/raw node-storage base. A `CameraFrameState`,
`CameraTransform`, or `NativeCameraOwner` companion is a different object.
The existing owner exposes the same raw prefix and `NativeCameraTailStorage`
fields, but these functions do not initialize missing prefix bytes or recover
any additional owner representation.

| Storage | Evidence and required contract |
|---|---|
| +30, +5C, +B0, +F0 | Full raw `refresh_native_camera_world_00b6db70` uses a captured actual parent pointer, validity bits, local64 and world64; it recursively refreshes dirty parents and calls the existing raw matrix providers. |
| +110..+118 | Third world-matrix row: +F0+20, copied into the native temporary before arithmetic. |
| +2F0 | Current cache flags; B71C39 initializes this word to 1. The axes only OR bit100 after successful completion. |
| +440, +44C | Two float3 cache ranges. B71C93..B71CC2 writes Y=(0,one,0), X=(one,0,0); existing tail storage agrees. |
| +43C | B71B26 stores zero. B6FEB0 returns the current raw word without retaining or dereferencing it. B475B6/B475C1 call twice when nonnull, then copy four floats into c71. The optional object's producer and lifetime remain unresolved. |

`NativeCameraAxesContext` borrows actual `0109DD78`, `00D7A24C` and `00CE3800`.
The latter words were verified as `3F800000` (1) and `3F000000` (0.5).
The handler is fixed to the complete `legacy_crt_87except_00c27489` provider.
Its existing `LegacyCrtMathRuntime` must already bind actual `E16BD0` and the
owning runtime's real errno accessor. Construction changes no CRT binding.

The full native listings and concrete current source were checked for all
four immediate providers:

| Provider | Native contract and use |
|---|---|
| B6DB70 | ECX actual camera, RET. Existing complete raw world refresh, including recursive raw parent identity. |
| 4F9B30 | ECX output, EDX left, stack right, EAX output, RET4. Three calls per getter. The first pre-pushed forward argument survives the first call for the second call. |
| 419510 | ECX output, EDX input, EAX output, native RET. Two calls per getter. Source provider adds a stack pointer to the concrete CRT access and removes that added word with RET4. |
| 419440 | ECX input, ST0 result, RET. Source provider adds the concrete CRT access in EDX; its full native spills and CRT square-root path remain in use. |

The report records all 14 owned call sites and eight incoming call sites with
numeric caller/target addresses. There is no unread host callback, absolute
call into the installed game, or replacement FSQRT-only implementation.

## Preserved execution order

Bit100 at +2F0 skips all work, including context reads and world validation.
Otherwise, a clear bit2 in the low byte at +5C triggers the full world refresh.
The routines capture the current forward row and current world-up constant,
perform two crosses and a normalization, then store Y through three native
FLD/FSTP pairs. They cross the captured forward with that actual stored Y,
normalize, and store X through three more FLD/FSTP pairs.

Length is evaluated from the actual Y cache after both stores. The original
FCOMIP/FSTP/JBE sequence skips fallback for length greater than or equal to the
current limit, and for unordered comparison. Ordered length below the limit
reloads the current one word and stores the six fallback words in native order.
The current flag word is then ORed with bit100. Faults preserve the writes that
already occurred; no rollback or synthetic flag publication is added.

Both complete instruction sequences remain explicit in the source. The only
added instructions preserve the context in EBP outside the original 30h local
frame and supply integer pointers to existing providers. No floating-point
operation or spill is added. The borrowed getter is exactly
`MOV EAX,[ECX+43C]; RET`.

## Validation and boundary

Strict source compilation uses MSVC Win32, `/MD /EHsc /W4 /WX /O2 /Oy- /fp:strict`.
The module is deliberately unregistered in this worker's CMake build; its
independent compile and external fixture compile the new source explicitly.
The integrator must register it and replay against the final current libraries.
Current build and call-check results are recorded in the companion report.

Eight guarded live/disk spans agree: 629 owned bytes, 304 bytes from the three
complete original vector kernels and the two four-byte constants (941 total).
Each live read verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. The installed executable's SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The external fixture at `C:/Users/sqz269/bsp-az-camera-axes` copies the six
complete original code spans into its own allocated pages. It verifies each
of 22 explicit relocation operands, changes pages from writable to executable,
and checks their complete postimages after execution. Internal branches are
unchanged. Original vector kernels share the full rebuilt CRT sqrt/C27489 and
raw-world providers with the source side; this is not independent original-CRT
or original-world differential proof.

One focused nine-case fixture compares 20,988 bytes: complete 2,304-byte raw
arenas, both returned axis offsets, optional context pointer, SEH code, x87
control/status, MXCSR and errno. It covers shared-cache hits with null context
and an invalid unused parent, oblique axes, dirty root and two-ancestor refresh,
24-bit and 53-bit round-down x87 precision, degenerate and subnormal inputs,
masked signaling NaN and overflow (actual errno33), and an unmasked signaling
NaN fault (`C0000090`) before cache publication. All nine pairs agree exactly.

Worker replay uses `run.ps1 -Root <worker> -LibraryRoot <library snapshot>
-WorkerSource`. After integration, omit `-WorkerSource` and point both root
arguments at the integrated checkout. The default compiles only the external
probe and links the three current project libraries. Source/library hashes are
pinned before and after each compile/run; fixture files and logs are preserved
with the worker capture.

Private native stack/spill aliasing, invalid memory, cycles, concurrency,
asynchronous constant mutation and all possible floating-point continuation
modes are outside this fixture. The source is a complete raw-storage extension
for these three bodies, not closure of B46A70's remaining lighting/time/service
dependencies. No gameplay, renderer-parent execution or runnable-game claim is
made here, and no repository test was added.

## AZ integration analysis refresh

The integrator saved and read back all 27 AZ original signatures and complete
normal-body ranges, and refreshed exports. CBBBF0 and CBBC10 are ten-byte
analysis-only EH handlers defined under leases and the write lock. Earlier
missing-function observations remain worker capture history. The batch adds
22 complete body records and extends five existing bodies with raw interfaces;
the two EH definitions add no normal-body count. Exact combined validation
follows separately from worker fixture evidence.

## AZ exact merged validation

The exact combined source commit `8e9ab00b59fe1c6f04da21bf39fcc304211b71c9` passed the strict Win32
build, both existing tests and five current-library-only original-byte fixtures.
See `reports/native_system_sources_az_validation.json` for hashes, preserved
captures, case coverage and limits. Earlier pending statements describe worker
capture stages. Full rendering, native ABI and general concurrency remain open.
