# Camera plane-set initialization

The two reconstructed entries write the existing `CameraPlaneSet` storage used
by the native camera constructor. Its sixteen records are twenty bytes each:
four float coefficient words followed by one flags DWORD. Count is a separate
DWORD at `+140`, giving a total extent of `144h` bytes. The APIs borrow that same
storage and the actual live constant word at `00D7A24C`; neither constructs a
replacement plane owner nor touches camera cache flags.

`initialize_camera_plane_records_00b652d0` captures the live constant once and
writes all sixteen records in increasing address order. Each record receives
`{positive_zero, captured_word, positive_zero, positive_zero, flags=1}` in that
store order. Coefficients use the original `MOVSS` schedule, so even a signaling
NaN payload is copied without quieting it or raising a floating-point exception.
The byte extent is exactly `140h`: count and surrounding memory are untouched.
The installed constant contains `3F800000` (one), but the API retains its live
raw-word binding.

`construct_camera_plane_set_00b659d0` performs this sequence:

1. Call the record initializer on the destination.
2. Reload the same actual `00D7A24C` word after all sixteen records are written.
3. Store six in destination count `+140`.
4. Write a temporary matrix in row order, with the second captured word in all
   four diagonal positions and positive zero elsewhere.
5. Call the existing `extract_camera_frustum_00b653f0` with that matrix.
6. Call the shared `assign_camera_frustum_planes_00b658e0` with flags seven.
7. Return the original destination pointer.

The matrix is identity for the installed constant. Extraction retains the
established x87/SSE operation order, normalization, and signed-zero results.
Six-plane assignment retains its forward `FLD/FSTP` coefficient stores, followed
by each flags store. It leaves count unchanged. The first six final records
therefore have flags seven; the remaining ten retain the initialized values and
flags one. No dirty-cache getter or hardcoded set of six planes substitutes for
these two existing helpers.

The assignment helper was previously embedded in `get_camera_frustum_00b70710`.
The primary agent extracted it into the shared frame module and made that getter
delegate with flags seven. This packet calls that public helper and does not
duplicate its numeric loop. The two shared frame files were supplied for local
compilation only; their hashes are recorded in the audit, and they are excluded
from this packet's commit.

## Original ABI and evidence

Both native entries receive destination in ECX, return that exact pointer in EAX,
and use `RET 0`. The `00B652D0` decompilation initially reported a void return;
the assembly sets EAX from ECX before the loop and preserves it through `RET`.
The new typed interfaces add an explicit live-constant binding, so they are not
drop-in native ABI replacements. Names are descriptive interpretations, not
recovered original symbols.

`00B652D0` spans `[00B652D0,00B65314)` and `00B659D0` spans
`[00B659D0,00B65A75)`. Both were inspected through their final return. Twelve code
spans and three data spans were compared between bounded live reads from
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and the installed
PE. Each query verifies the configured project/program first. The audit records
individual SHA-256 hashes, data values, dependency ownership, and fixture
relocations. The additional code captures support only the existing extraction,
assignment, vector-length, and finite CRT square-root path; this packet claims
only the two initializer addresses.

## Validation and limits

One focused Win32 sequence matches the original installed instructions across
five checkpoints, ninety-three words each: all sixteen records, count, sixteen
canary bytes on each side, x87 status/control, MXCSR, and the live constant. It
checks both native return pointers, count preservation, six active planes,
remaining records, one-to-two live-constant changes, and signaling-NaN bit
preservation in the record-only initializer. All 465 compared words match.
The native fixture executes the original CRT square-root path without replacing
it with a stub and keeps the installed zero-filled CRT mode word unchanged.

The new source and the primary-supplied frame helper pass MSVC Win32
`/std:c++20 /W4 /WX /fp:strict`. The existing repository build and its existing
test pass (1/1). Compiled object inspection additionally confirms the constructor
calls the record kernel, reloads the borrowed constant, then writes count before
the matrix stores and extraction/assignment calls.

Constructor comparisons use finite positive constants one and two with x87
control `027F` and MXCSR `1F80`; signaling-NaN coverage is confined to record
initialization. Native CRT exceptional diagnostics, other floating-point modes,
concurrent constant mutation between loads, and game execution were not tested.
The reused extraction helper retains its existing finite-square-root scope.
Camera owner composition, Ghidra annotation/export, ledger entries, and CMake
registration remain primary integration work.
