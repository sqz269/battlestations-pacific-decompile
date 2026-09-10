# Model world bounds

`ModelBounds` stores the local and cached world spheres for a model. The getter
receives that model's existing `CameraTransform`, so transform changes and sphere
cache validity share the recovered native flags. This is reconstructed behavior
behind a new C++ storage interface; it is not a binary replacement for a native
model object, its constructor, or its vtable.

The implementation reuses the public `transform_point_004142e0` kernel promoted
by commit `4234a7f`. It also calls `refresh_camera_world_00b6db70`; neither affine
arithmetic nor transform cache state is duplicated.

## Native functions and layout

| Address | Proposed name | Original ABI |
| --- | --- | --- |
| `00B6E8C0` | `BSP_Model_GetWorldSphere` | ECX model; EAX model+13C; plain RET; virtual+48 |
| `007C1180` | `BSP_Sphere_TransformAffine` | ECX source sphere; stack destination, matrix; EAX destination; RET8 |
| `007BB620` | `BSP_Matrix_GetMaximumBasisLengthSquared` | ECX matrix; x87 ST0 result reloaded from float storage; plain RET |

Names are descriptive hypotheses. Register and stack arguments come from
assembly, because the decompiler omits register inputs and the sphere transform's
return pointer. The original model stores local XYZ/radius at `+08`, world-valid
flags at `+5C`, world matrix at `+F0`, auxiliary flags at `+138`, and cached world
XYZ/radius at `+13C`.

`get_model_world_sphere_00b6e8c0(transform, bounds)` first tests
`transform.auxiliary_flags & 0x30`. Any nonzero result is a cache hit. In
particular, either `0x10` or `0x20` alone accepts the cache even when world-valid
bit `2` is clear. A cache hit returns the existing sphere without touching flags
or refreshing world state.

On a miss, the getter refreshes world only if bit `2` is clear, transforms the
local sphere into temporary storage, copies four floats into the world cache with
the native FLD/FSTP order, then ORs `0x30` into the auxiliary flags. This preserves
other auxiliary bits and leaves world refresh responsible for its own flags and
parent traversal. The temporary and final copies are material: cache validity is
published only after all four values have been stored.

## Scale and center arithmetic

For each of the first three matrix rows, `007BB620` computes `x*x`, then `y*y`,
combines `ySquared + xSquared` in x87, computes `z*z`, adds it, and stores the
completed sum to float. There are no intermediate float stores inside one row.
The fourth elements and translation are ignored. The decompiler's reordered
expression is not the instruction order.

FCOMI/JC chooses between the first and second stored row sums. FCOMIP/JC then
compares that selection with the third. Equality retains the earlier row. Carry
also covers unordered comparisons, so ordinary C++ `max` is not an equivalent
replacement. The reconstructed kernel retains the original x87 sequence and
these carry branches. This is the native maximum-row-length rule; it is not the
matrix's largest singular value and no shear correction is added.

`007C1180` calls the register-input CRT square-root wrapper on the selected sum.
It stores the square-root result to float, reloads it, multiplies by the source
radius, and stores that product to float **before** calling the affine point
kernel for the center. The source radius keeps its sign; there is no absolute
value or clamp. Final XYZ and radius copies use FLD/FSTP. The C++ leaf supports
the source and destination being the same float4; matrix/destination overlap is
outside its interface.

## CRT environment boundary

The supported numerical domain is finite inputs and finite stored intermediates
with round-to-nearest, masked exceptions, and x87 control word `0x007F` or
`0x027F`. These select 24-bit and 53-bit arithmetic precision respectively. The
former is the renderer's observed post-D3D9 environment. The implementation does
not change the caller's control word.

The CRT audit establishes why FSQRT is sufficient for this domain:

1. `00BF7030` stores the input as double without popping ST0, calls exponent
   classifier `00C08418`, and enters `00BF704D`. A finite nonnegative operand
   reaches FSQRT at `00BF706C`.
2. `00BF704D` saves the control word. For a word other than `0x027F`, helper
   `00C083A5` installs `(savedCW & 0x0300) | 0x007F`. Both supported words are
   therefore unchanged. Direct exit `00C0842E` restores the saved word as needed.
3. If global `0109DD78` selects `__math_exit` (`00C0843B`), `0x027F` exits
   directly. Under `0x007F`, a set inexact status bit routes code `8` through
   `__startOneArgErrorHandling` (`00C08347`) and `__87except` (`00C27489`). The
   result is stored as double in this path; a 24-bit FSQRT result is exact there.
4. `__87except` maps code `8` to mask `0x10`. `__handle_exc` (`00C13364`) sees
   saved control bit `0x20` and calls `00C138CE(0x20)`, whose FLDPI/FSTP-double
   sequence sets the already-set inexact flag. It does not alter the saved
   square-root value and reports the condition handled, bypassing `__raise_exc`.
5. `__ctrlfp` (`00C138A7`) restores the saved word. Code `8` explicitly bypasses
   user matherr. `__set_errno_from_matherr` (`00C13545`) changes errno only for
   codes `1` through `3`, so it does nothing here. The saved result is reloaded
   unchanged and the original word remains in force for the float scale store.

Thus both normal CRT exits have the same finite numeric result and control word
as the reconstructed FSQRT path. This is not a reconstruction of general CRT
math error handling. NaNs, infinities, negative square-root inputs, unmasked
exceptions, other precision/rounding control words, and complete x87 status-word
or instruction/data-pointer equivalence are not claimed. Existing correct CRT
library names are retained; the audit does not propose renaming those helpers.

## Local-sphere producer

The model owner must supply the real local sphere and pair `ModelBounds` with
the correct shared transform. If the local sphere changes, it must clear both
auxiliary bits `0x30`; recovered transform setters already invalidate these bits
when transform state changes. Zero-initialized C++ storage is not a recovered
native constructor default.

`StructuredHierarchy.sphere` is an available upstream value: the hierarchy
reader uses `read_sphere_00b932e0` for its four serialized floats. A model owner
may provide that value when its model resource contract identifies it as the
local bound. This getter does not infer a sphere from vertices or silently
substitute the model's translation. Mesh payload handler
`consume_mesh_sphere_00b93590` reads and discards four floats; it does not populate
this cache. Hierarchy ownership, model cloning, and installation of the owner
adapter are separate integration work.

## Evidence and validation

`reports/model_world_bounds_audit.json` records the installed PE hash, verified
byte ranges, native ABIs, original names/prototypes/comments, proposed names and
plate comments, and CRT branches. All sixteen recorded code ranges match the
installed PE and the saved Ghidra image. Every live query/export verified project
`bsp`, program `/battlestationspacific.exe`, language and image base through the
repository CLI. This comparison does not establish whole-image identity.

MSVC Win32 compiled `model_bounds.cpp` with `/O2 /MD /W4 /WX /fp:strict`. A local
native comparison executed the unchanged `007BB620` PE bytes against the
reconstructed kernel for one precision-sensitive matrix and a NaN carry-branch
variant, under both supported control words. All four comparisons matched
bitwise and preserved the control word. The finite case returned `0x4B800000`
under `0x007F` and `0x4B800001` under `0x027F`, demonstrating that precision is
observed rather than normalized away. This is a maximum-basis differential,
not execution of the complete native CRT chain or model object.

`scripts/build.ps1` completed and both existing CTests passed after seed-byte
verification. At the worker handoff, CMake does not yet include the new source;
the separate compile/link above validates it. The primary integrator owns CMake,
the real model/probe adapter, shared ledgers, Ghidra annotation application and
export refresh. No gameplay or visual validation is claimed by this packet.
