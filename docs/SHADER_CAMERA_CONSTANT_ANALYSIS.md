# Camera matrices used by shader constants

Bounded read-only analysis, 2026-09-09. `Client.verify()` confirmed project `bsp`,
program `/battlestationspacific.exe`, x86 LE32 and image base00400000 before each
batch. Raw pseudocode and assembly are under ignored `exports/bsp/functions/`.
No C++, Ghidra state, names, saves or shared metadata were changed.

## Accessor and cache contract

All four accessors take ECX=camera object, have no stack arguments, return an
internal float16 matrix pointer in EAX, and end in plain RET. Bit **set means
valid**, so a zero bit triggers refresh. Descriptive names below are proposals.

| Address / proposed name | Cache/output | Refresh |
|---|---|---|
|00b6fcb0 / BSP_Camera_GetViewMatrix|camera+60h; flags+5ch bit8|Ensure world+f0h valid via bit2, then orthogonal-scaled affine inverse00b63b30|
|00b6fcf0 / BSP_Camera_GetProjectionMatrix|camera+2a0h; flags+2f0h bit8|Build perspective into temporary, copy to+1e0h then+2a0h|
|00b70490 / BSP_Camera_GetViewProjectionMatrix|camera+220h; flags+2f0h bit10h|Ensure view, ensure projection, multiply view * projection|
|00b70510 / BSP_Camera_GetInverseViewProjectionMatrix|camera+260h; flags+2f0h bit20h|Ensure viewProjection, general inverse00b632d0|

Matrix copies use004134f0. The accessors set their corresponding validity bit
only after refresh/copy.00b70490 contains the same inline view-cache refresh as
00b6fcb0.00b6fc70 is another observed accessor with equivalent view-cache logic.
The shader setup00b46a70 also directly ensures bit2 before accessing camera world
matrix+f0h and camera position floats at+120h/+124h/+128h.

## Matrix convention and helper ABI

Matrices are 16 contiguous float32 values. The observed algebra is represented
naturally as row-major storage with row vectors: translation is indices12..14,
world composition is local * parentWorld, and viewProjection is view * projection.
This statement is grounded in multiplication/address arithmetic, not a claim
about every external shader convention or upload transpose. The parent-owned
00b404a0 writer must be considered separately when deciding register orientation.

| Helper | Assembly-backed interface | Behavior |
|---|---|---|
|004134f0|ECX destination, stack source, RET4, EAX destination|Copies16 float slots with FLD/FSTP; not universally bitwise memcpy for exceptional floats|
|00413920|ECX left matrix, stack destination then right matrix, RET8, EAX destination|Full4x4 left*right multiplication|
|00b6d4d0|ECX destination, EDX left affine matrix, stack right affine matrix, RET4|Affine local*parentWorld composition; homogeneous column0,0,0,1|
|00b63b30|ECX destination, EDX source, no stack args, EAX destination|Inverse for orthogonal scaled affine basis; not general shear inverse|
|00b632d0|ECX destination, EDX source, no stack args, EAX destination|General4x4 Gauss-Jordan-style inverse with pivot selection|
|00b642f0|ECX destination, four float stack args fov/aspect/near/far, RET10h, EAX destination|Perspective builder|
|00412e20|One float stack argument, RET4, ST0 float32-rounded result|FSINCOS then sin/cos, float32 spill/reload|

00413920 computes output[r,c] = sum(left[r,k] * right[k,c]). It has x87
intermediates and source float spills; algebraically equivalent reassociation is
not enough for native numerical parity.00b63b30 first copies all16 source slots,
then transposes/divides basis rows by their own squared lengths and replaces
translation with negative translation times the inverse basis. This assumes
mutually orthogonal nonzero axes. It handles axis scales but is not a general
inverse when the basis is sheared. Source/destination alias constraints must be
preserved or explicitly restricted in a new interface.

00b632d0 copies input into a local matrix, initializes destination identity,
selects pivots by absolute magnitude, swaps rows, divides pivot rows and
eliminates other rows. No singular-pivot error return is established; its return
is the destination pointer. Its long pseudocode contains decompiler artifacts,
so a complete numerical reconstruction needs focused assembly/fixture work.

## World refresh dependency

00b6db70 takes ECX=transform object. Parent pointer is+30h, local affine matrix
+b0h, cached world+f0h, flags+5ch bit2. With a parent, it recursively ensures the
parent world is valid, then calls00b6d4d0(destination=this+f0h,
left=this+b0h,right=parent+f0h). Without a parent, it copies local to world.
Finally it ORs bit2. It does not perform hierarchy-cycle detection here.

The current trace does not establish all transform setters/descendant dirty
propagation. Therefore a full scene-camera cache port cannot safely assume
local matrix edits automatically invalidate view, projection combinations or
children. The dependency can be supplied explicitly by an adapter until those
invalidation paths are reconstructed.

## Projection inputs, exact layout and invalidation

Perspective scalar inputs are camera+1c4h (vertical FOV in radians, inferred
from half-angle FSINCOS), +1c8h (aspect), +1d4h (near), and +1d8h (far).
Let s be float32(1 / float32(tan(float32(fov * 0.5)))) and
q be float32(far / (far-near)) with native x87 intermediate precision. The
nonzero matrix entries are:

- m[0] = float32(s/aspect)
- m[5] = s
- m[10] = q
- m[11] = 1
- m[14] = float32(-near*q)

All other slots are zero. This is the conventional positive-Z row-vector
perspective mapping: for positive near/far, post-divide depth maps near to0 and
far to1. That conclusion follows from the entries; it is not a whole-game
handedness/coordinate-system assertion.

Assembly00b642f4 multiplies FOV by live double0.5 at00d7a280 and spills to
float32 before00412e20. That helper uses FSINCOS (00412e25), divides, then spills
and reloads float32 (00412e29..2c). The builder reciprocates and spills s to
float32 at00b6434e. The far ratio is similarly spilled before storage/use
(00b6436d). Generic std::tan or an SDK perspective helper is not bitwise native
proof. Degenerate FOV/aspect/near/far inputs are not validated by these helpers.
Live constants checked:00d7a280=0.5,00d7a24c=1.0,00d7a208 is **negative zero**
(bits80000000), used by the inverse arithmetic. Do not erase signed-zero
behavior while claiming exact FP parity.

The four scalar setters are00b6fbb0(FOV),00b6fbd0(aspect),00b6fbf0(near), and
00b6fc10(far), each ECX receiver, one stack scalar, RET4. Each performs
flags+2f0h &= ffffff41h and writes its scalar, with no equality guard. This clears
bits2h,4h,8h,10h,20h,80h while preserving bit1 and40h and upper bits. Only the
8h/10h/20h meanings are established in this trace.

00b6fd60 is an explicit projection setter: ECX camera, stack source float16,
RET4. It copies the source to both+1e0h and+2a0h, then sets
flags+2f0h = (flags & ffffff4bh) | 8h. Thus the projection cache becomes valid
while combined viewProjection/inverseViewProjection caches become invalid.
A later scalar setter invalidates that supplied projection and causes the
perspective builder to regenerate it. Do not collapse the two projection storage
locations without checking other consumers first.

## Smallest complete next camera slice

The projection builder00b642f0 plus its tiny FSINCOS wrapper00412e20, scalar
setters and projection getter00b6fcf0 form a bounded complete lazy-projection
subsystem. It does not require scene hierarchy or general matrix inversion.
A new typed state can expose those scalar/matrix values and reproduce validity
transitions, with original offsets and ABI documented separately. A focused
native comparison should target the actual FP spills and signed-zero behavior.

View/world and combined caches are a subsequent dependency layer: they require
affine composition00b6d4d0, scaled-orthogonal inverse00b63b30 and multiply00413920,
plus the still-unresolved dirty propagation. InverseVP additionally requires
00b632d0 with explicit singular-input/precision boundaries. A successful shader
matrix upload using supplied host matrices does not establish this camera
subsystem as reconstructed or runtime validated.

Status: requested accessors and immediate refresh helpers exported; ABI,
mathematical layout, cache gates and projection setters assembly-inspected.
No reconstruction, build, native differential execution or game validation was
performed for this document.
