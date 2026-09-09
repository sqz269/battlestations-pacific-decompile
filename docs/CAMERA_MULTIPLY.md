# Reconstructed camera matrix multiplication

`include/bsp/camera_multiply.hpp` and `src/camera_multiply.cpp` expose:

```cpp
void multiply_camera_matrices_00413920(CameraMatrix& dst,
    const CameraMatrix& left, const CameraMatrix& right);
```

`CameraMatrix` is the existing float16 alias from camera_projection.hpp. The
operation is row-major `dst = left * right`. This supplies the full matrix
product used by camera view-projection accessor00b70490; camera cache/lifecycle
integration is separate.

## Evidence and original interface

Fresh target verification before this work confirmed project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base00400000. The saved874-byte body at
00413920 hashes to
`a2ae101c6235ce95cd7cffdc28deb5797b9d844dbc809e729db5ac5b134e29d9`, matching the
prior disk-parity evidence in CAMERA_MATRIX_DEPENDENCIES.md. This pass rechecked
the saved bytes; it did not repeat the entire disk comparison or modify Ghidra.

Original ABI: ECX=left, first stack argument=destination, second stack
argument=right, RET8, EAX=destination. The body has290 decoded instructions,
no calls, globals, branches or explicit FP-environment changes. It writes all16
output floats and balances its x87 stack.

The private MSVC Win32 naked helper expresses that same register/stack schedule:
a dummy EDX fastcall argument places destination/right on the stack, after which
the original body loads right into EDX. The typed public wrapper supplies actual
array pointers and discards the helper's returned destination. This is a new
C++ interface, not a binary replacement or a call into the installed game.
The helper consists of source assembly, not executable bytes loaded from disk.

## Numerical and alias behavior

The implementation retains the entire original x87 operation sequence and
scratch layout, including input FLD/FSTP float32 caches, product operand order,
addition order, retained x87 intermediates, and final float32 stores. It does
not replace the operation with a generic dot product, SIMD or compiler-generated
reassociated expression. The source comments mark the input-cache boundaries.
MASM operands are spelled explicitly while retaining native instruction meaning.

The original0x40-byte scratch allocation also uses its now-disposable stack
argument slots as temporary float32 caches after their pointers are captured.
That detail is preserved by the private helper rather than incorrectly sizing
an independent64-byte C++ scratch array.

Exact destination==left and destination==right are supported, including all
three matrices sharing the same object: each left row is cached before its
outputs and each right column is cached before any corresponding first-row
output overwrites it. By completion of the first row all right coefficients
are cached. There is no added whole-input snapshot or altered output-store order.
Arbitrary partial overlap remains outside the API contract because it can
overwrite future uncached left rows/right columns.

The code inherits the caller's x87 control/status state and hardware behavior.
It does not force a precision mode, clear exceptions, validate numbers, or
sanitize NaNs/denormals. Original FLD/FSTP conversions can quiet signaling NaNs;
retaining their order supports investigation of that behavior without claiming
all exceptional inputs have been validated. MSVC Win32 is required; there is
no approximate portable fallback.

## Validation boundary

Parent integration owns CMake, the build, original-byte differential fixture,
metadata and final validation report. This implementation handoff performed no
build/tests and made no shared metadata or Ghidra edits. Build success, native
numerical/alias comparisons and game validation must be reported independently;
source instruction preservation alone is not fixture or runtime proof.

## Native comparison and integration follow-up

The existing D3D9 probe now includes an optional isolated native camera fixture.
Run `python tools/verify_camera_reference.py` before building to audit original
versus saved bytes and emit the ignored header. The probe copies only four
verified helpers and three constants to private memory, relocates four operands,
then changes that allocation from writable to executable/read-only. It loads no
game executable or game DLL. Audit hashes are in `reports/camera_reference_audit.json`.

One rotated/scaled/translated matrix and one perspective matrix compare all16
words bit for bit for inverse, projection and multiplication. Multiplication
also matches with output equal to left, right, or both inputs. These checks
passed in the probe default FP environment; results and limitations are in
`reports/camera_reference_result.json`. No exceptional-value/all-environment
claim follows. The existing generated draw now computes a view from camera
worldZ=-1, multiplies view*projection and uploads the transpose. CenterFF407FBF,
outsideFF000000 and state restore pass. Win32 build and both existing CTests
pass. No new test target was added. Full scene/camera lifecycle and gameplay
remain unported.
