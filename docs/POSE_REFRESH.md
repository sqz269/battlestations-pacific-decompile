# Lazy entity pose refresh at 00414DB0

`refresh_pose_00414db0(PoseRefreshView&)` reconstructs the complete 89-byte
routine at `00414DB0..00414E08`. Its descriptive name is a hypothesis. Native
ECX points at the pose owner; there are no stack arguments, and the final `RET`
at `00414E08` is one byte. The native clean path leaves EAX untouched, so the
typed interface makes no return-value claim. This is a new C++ field interface,
not a native object or calling-convention replacement.

Read-only analysis used target-verified `bsp.py` Ghidra commands against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and original PE decoding.
All 28 instructions are defined with complete stored bounds and flow; no missing
definition, final-instruction truncation, or false-no-return gap was found.
The fixture's 89 reference bytes match the installed EXE range; SHA-256 is
`ca7360fd4d5ae3ae386c44374188b3b807eb8a1bb039f508d47f0da2116ab175`.

## Canonical fields and binding

| Native field | Borrowed typed field | Operation |
| --- | --- | --- |
| +3Ch | `PoseRefreshParentSlot parent_3c` | Read before recursion and reload afterward |
| +74h | `CameraMatrix& local_74` | Read as the complete local matrix |
| +C8h | `uint8_t& world_valid_c8` | Any nonzero value skips all work; dirty completion writes 1 |
| +CCh | `CameraMatrix& world_cc` | Write the complete resulting world matrix |
| +10Ch | `uint8_t& derived_valid_10c` | Write 0 after setting +C8h; further meaning unresolved |

The repository already has the canonical `CameraMatrix` float16 representation
and both required matrix operations. `CameraTransform` is a different native
object, with parent +30h and local/world matrices +B0h/+F0h. It is not substituted
for this pose. Existing `SceneEntity`/scene-factory frames are parse/factory
values, while unit-instance/motion projections contain only parts of the pose;
none supplies this complete mutable hierarchy. `UnitShiftedUpdateState` already
borrows the unit's +74h matrix. This view can bind that same matrix and the
owner's actual remaining fields without creating a second pose state.

`PoseRefreshView` owns no matrix, flag, parent pointer, or native object. Its
parent slot borrows either an existing `PoseRefreshView*` field or an actual
`void*` owner field plus `PoseRefreshResolver`. The latter follows the existing
viewport-slot pattern: the resolver performs a pure lookup of a stable view for
a nonnull identity. It must not allocate, copy matrices, retain owners, perform
game callbacks, or supply a fallback view. Null parent bypasses the resolver.
Both constructors require mutable pointer lvalues, preventing accidental
bindings to temporary pointer values. Copies of the slot preserve its field
binding rather than snapshotting the parent.

All storage, views and resolvers must outlive their use. Unsupported nonnull
owner identities must be rejected by the resolver. These lookup/lifetime
obligations are C++ composition boundaries, not additional native game logic.
Byte flags preserve arbitrary nonzero native values on the clean path. A
caller with an older normalized Boolean projection must bind its real byte
storage explicitly rather than reinterpret a Boolean reference.

## Recovered sequence

1. At `414DB6`, read +C8h. When nonzero, return without reading the parent or
   either matrix and without changing +10Ch.
2. At `414DBF`, read parent +3Ch. If nonnull, recurse at `414DC6`. There is no
   separate parent-valid check: the recursive call performs that check itself.
3. At `414DCB`, reload the actual parent field. When nonnull, call canonical
   `multiply_camera_matrices_00413920` at `414DE0` with **local * parent world**,
   writing into disjoint 40h stack scratch. This is the full 4x4 multiplication,
   not the affine camera composition helper. A parent that changed during
   recursion is re-read; the newly observed parent is not separately refreshed.
4. Call canonical `copy_camera_matrix_004134f0` at `414DF1`, copying scratch to
   world for a parented pose or directly copying local to world for a root.
5. Store +C8h = 1 at `414DF6`, then +10Ch = 0 at `414DFD`.

The x87 arithmetic, operand order, intermediate float spills and sequential
FLD/FSTP copy remain in the existing canonical implementations documented by
`CAMERA_MULTIPLY.md` and the camera matrix dependency records. There is no SIMD
substitution, matrix transpose, memoized parent snapshot, or generic affine
shortcut here. Using a separate product before copying also preserves exact
matrix-alias behavior supported by those interfaces. Arbitrary partial overlap
or overlap of matrices with hierarchy/flag storage is outside the typed API.

No notification, descendant invalidation, ownership transfer, allocator call,
or virtual game callback occurs. The function does not set an in-progress flag
before recursion, sanitize floats, change the FPU control word, validate a
matrix, or recheck its own validity after returning from its parent. The valid
stores happen only after matrix completion. The same ordering is retained if
an exceptional matrix operation fails before completion; no C++ cleanup path
marks an incomplete pose valid.

Native dirty ancestry cycles recurse without a cycle guard; the typed function
does not turn such an invalid hierarchy into a successful refresh. A clean
ancestor terminates recursion, even if its own parent is invalid or cyclic.
Reentrant mutation, asynchronous modification and malformed overlapping native
objects are not made safe by this interface. No lock, rollback, early mark or
silently repaired parent is introduced.

## Validation and integration boundary

`python tools/ghidra_export.py verify-seeds` matched all eight existing native
seeds. `./scripts/build.ps1` passed MSVC Win32 Release `/W4 /WX` and both existing
CTests (`reconstructed_math`, `native_math_differential`).

One ignored fixture, `local/pose_refresh_fixture.cpp`, passed through
`local/run_pose_refresh_fixture.cmd`. It relocates the disk-matched 89-byte
native routine into an isolated executable allocation, retaining its native
recursion and field accesses. Only its two matrix calls are redirected through
ABI adapters to the same canonical matrix implementations used by the rebuilt
routine. The fixture therefore compares **pose control flow and state**, not an
independent second implementation of matrix arithmetic. It checks bit-equal
three-level results including a projective coefficient, local-times-parent
orientation, the parent lookup/reload order, dirty validity updates, a nonzero
0x80 clean byte with an invalid parent identity, live typed-parent binding, and
exact matrix aliases. It calls no game process, graphics device, input device,
window-focus API or hardware output.

This implementation supplies the actual operation behind the force-event,
unit-instance, unit-motion and voice-position host boundaries. Their concrete
owners still bind the same live pose fields and parent identities; no leased
scene, renderer, unit or force-event file was changed in this packet. Cycles,
FPU traps, signaling-NaN payloads, partial overlaps, native object allocation,
and native SEH behavior were not exhaustively tested. No gameplay validation
or native object-layout compatibility is claimed.
