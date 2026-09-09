# Orthogonal scaled camera inverse

`invert_camera_affine_00b63b30` reconstructs the complete 531-byte routine as a
typed CameraMatrix operation. The internal Win32 assembly kernel preserves
the original register and stack schedule, including the forward 64-byte copy,
float32 spills after squared components/norms/divisions, x87 translation dot
products and final SSE subtraction from negative zero. It contains assembled
instructions, not executable bytes copied from the game or calls into its image.

The public C++ API takes distinct, nonoverlapping destination and source arrays.
The original register ABI is ECX=destination, EDX=source, EAX=destination result,
plain RET. The kernel uses that ABI internally; the public wrapper returns void.
No native camera object layout, dirty cache or ownership is introduced.

The upper three source rows must be mutually orthogonal and have nonzero
length for this operation to be an inverse. Each row is transposed and divided
by its squared length, then translation is transformed and negated. The original
last-column words at indices 3,7,11,15 are retained; affine inputs require these
to be 0,0,0,1. General shear/perspective matrices are not supported inverses.
There is no validation, epsilon or singular fallback, matching native arithmetic.

Both distinct matrices must contain 64 readable/writable bytes as appropriate.
In-place operation is outside the interface: original later source reads can
observe overwritten destination elements despite the initial copy. No alias
normalization or temporary input snapshot is added.

The private negative-zero word is exactly 80000000h, replacing native global
00d7a208 without depending on original addresses. Neither x87 control word nor
MXCSR is changed. Numerical behavior therefore retains the caller's rounding,
precision, denormal and exception configuration; the ordinary clear direction
flag ABI requirement applies to REP MOVSD. This preserves instruction semantics
but is not a claim of all-environment native fixture or gameplay validation.

Evidence and alias/precision analysis are in CAMERA_MATRIX_DEPENDENCIES.md.
Original/saved 531-byte SHA-256 is
`619b87a228444a7f6e3e78d8d26e1cbd2851baa6bbcbc3362ea9b39c2ff06d5c`.
Parent integration owns build and focused reference checks. No new tests,
Ghidra metadata edits or project save accompany this implementation.

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
