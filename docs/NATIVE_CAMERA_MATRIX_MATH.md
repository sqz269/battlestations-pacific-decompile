# Native camera matrix arithmetic

This packet exposes three complete original instruction bodies through raw
matrix-storage interfaces. It promotes the established private assembly in
`camera_inverse.cpp`, `camera_multiply.cpp`, and `camera_affine.cpp` without
changing those files or their semantic public APIs. It does not reconstruct a
camera/node layout, hierarchy, dirty-state getter, or object lifetime.

| Original range (end excluded) | Bytes | Raw entry | Original ABI and incidental result |
|---|---:|---|---|
| `00B63B30..00B63D43` | 531 | `invert_native_camera_scaled_affine_00b63b30` | ECX=destination, EDX=source; RET; EAX=destination |
| `00413920..00413C8A` | 874 | `multiply_native_camera_matrices_00413920` | ECX=left, stack=destination/right; RET8; EAX=destination |
| `00B6D4D0..00B6D623` | 339 | `compose_native_camera_affine_00b6d4d0` | ECX=destination, EDX=left, stack=right; RET4; EAX=right |

Each matrix is an actual 64-byte view. The new C++ interfaces use raw pointers;
they impose no semantic camera overlay or C++ matrix copy. The multiply entry
has an explicit unused EDX argument so its destination and right arguments keep
their original stack positions. The affine entry exposes its incidental right
pointer result. Names describe recovered behavior and remain hypotheses.

The inverse begins with the original `REP MOVSD` of sixteen DWORDs into the
destination. Later instructions read the source again, after those stores and
after subsequent destination writes. No source snapshot is introduced. Unsafe
aliasing therefore keeps the native result and fault/write order. The native
direction flag is not changed; ordinary Win32 callers supply clear DF. Squared
components, norms, reciprocals, scaled-transpose stores, translation dot-product
spills and the final SSE `SUBSS` operations retain their original order. There
is no singular, shear, null, or overlap fallback. The mathematical interpretation
is an orthogonal scaled affine inverse, not a validated general inverse.

Multiplication retains all original x87 register lifetimes and binary32 caches,
including native writes to its stack argument slots. It computes the native
left/right matrix product with sixteen ordered destination stores. The raw
entry does not add guarantees for partial overlap. Composition retains its
interleaved input loads and destination stores, which can corrupt still-needed
input when output aliases an operand. Its last column is set to +0,+0,+0,+1 in
the original positions relative to coefficient evaluation and stores.

All three functions preserve the ambient x87 stack, control/status behavior,
precision-sensitive spills, NaN handling and SSE boundaries by retaining the
complete original instruction bytes. No function catches exceptions, restores
earlier writes, initializes the floating-point environment, or calls a helper.
There are no owner, allocator, virtual, or callback dependencies. Memory must be
valid for the actually reached reads and writes, including native stack space.

The only relocated operands are the inverse's `00D7A208` negative-zero address
and composition's `00D7A24C` positive-one address. Fresh PE evidence places both
in read-only storage. New private read-only DWORDs have identical bits
`80000000` and `3F800000`; their pointer identities and addresses are new. The
object audit resolves both COFF DIR32 relocations to those exact constant
definitions and verifies the full original bodies after substituting only the
two original constant addresses. Multiplication has no relocations.

The evidence report is `reports/native_camera_matrix_math_audit.json`. Five fresh
Ghidra/installed-PE spans total 1,752 bytes: all 1,744 owned code bytes and both
four-byte constants. Every live query uses guarded `bsp.py ghidra` against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The installed executable
SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

An ignored CMake source hook compiles the actual new source into `bsp_core.lib`
under strict MSVC Win32 settings. The existing two CTests and eight native seeds
are required to pass. The complete actual COFF sections are checked against the
fresh original spans, and the library archive contains one byte-identical copy
of the exact object. The library, build object and extracted archive member are
frozen under ignored `local/camera_matrix_math/frozen`; source, provider,
configuration, toolchain-command, build-log and evidence artifacts are pinned.

This packet adds no tests or executable probe. The evidence is full object-byte
verification and the existing build/tests. It does not claim execution of these
new entries, original-byte runtime comparison, linked/runtime byte proof, or
game validation. Complete byte identity after the declared constant relocations
grounds the native arithmetic and alias order; the existing tests do not provide
separate coverage of every raw alias, exceptional value, DF, x87 or MXCSR state.
Ghidra, shared metadata and permanent build configuration remain unchanged.
