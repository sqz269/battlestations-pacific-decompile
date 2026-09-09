# Affine camera composition

`compose_camera_affine_00b6d4d0(destination,left,right)` reconstructs the native
affine composition kernel with typed CameraMatrix arrays. Native ABI is
ECX=destination, EDX=left, one stack pointer=right, RET4. EAX holds the right
pointer at exit; it is not a destination return contract. The public API is void.

With row-major storage, the upper 3x3 output is the ordinary product of the
upper 3x3 inputs. Translation is left translation multiplied by the right 3x3
plus right translation. Input indices 3,7,11,15 are not read. The output last
column is explicitly +0,+0,+0,1 even when inputs contain non-affine values.
This is affine composition, not a substitute for general 4x4 multiplication.

The internal Win32 naked fastcall kernel reassembles the original instructions
in order, including each x87 product/sum and final float32 FSTP store. The first
pair of product operands varies across elements; it is intentionally not
normalized into a shared dot-product helper. Translation adds the right-hand
translation after the three-product sum, before the final float32 store.
The three zero-column writes use XORPS/MOVSS; the last-column one uses exact
bits 3F800000h in a private constant replacing original global 00d7a24c. No game
memory is executed or read by this implementation.

Destination must occupy a distinct, nonoverlapping 64-byte range from either
input. The routine does not snapshot rows or columns. Destination==left can
overwrite left[0] before the next output component rereads it; destination==right
can overwrite right[0] before later rows reread it. There is no alias repair or
validation. Left and right can alias each other when destination is separate.

The routine inherits the caller's x87 control word/MXCSR. It preserves x87
extended intermediates until native stores and does not use generic C++ float
arithmetic or FMA. No claim is made that singular, NaN, infinite or non-affine
inputs acquire new validation or useful camera semantics. The wrapper does
not handle dirty flags, parent transforms, cache lifetime or hierarchy cycles.

Before analysis, project bsp and /battlestationspacific.exe were verified.
The full 339-byte native body and original one literal matched saved-image and
disk bytes. Build and focused reference checks are delegated to parent
integration. No new tests, Ghidra metadata edits or saved project changes were
made in this implementation pass.

Native body SHA-256: `a046a9c272194648d838b69a1246fb01f53ecb02857485f0c6de279e5bf20a49`.

Parent integration follow-up: this helper now drives recursive world refresh
and a two-node camera hierarchy in the existing shader draw. The isolated native
fixture compares all16 affine result words exactly for one scaled/rotated input.
Build, both existing CTests and full D3D9 probe pass. See `CAMERA_TRANSFORM.md`,
`reports/camera_reference_result.json` and `reports/camera_transform.json` for
validation scope. Dirty edit behavior and gameplay remain incomplete.
