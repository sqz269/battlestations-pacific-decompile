# Transform and camera dirty propagation

Read-only assembly-backed investigation, 2026-09-09. Each live analysis batch
verified project `bsp`, program `/battlestationspacific.exe` through
`tools/ghidra_export.py` Client.verify. Raw exports remain ignored under
`exports/bsp/functions`. Missing function bodies were decoded from live bytes
with Capstone, without creating functions or changing Ghidra state. Names below
are proposals, not recovered symbols. This document reports analysis, not a new
implementation or runtime test.

## Storage and validity

| Offset | Observed use |
|---|---|
| transform +30h | Parent pointer |
| +34h / +38h | First child / child count |
| +3Ch / +40h | Next / previous sibling |
| +5Ch | Transform validity: bit2 world, bit8 inverse/view |
| +60h / +B0h / +F0h | Inverse world, local, world float16 matrices |
| +E0h / +120h | Local / world translation float3 |
| +A0h | Attached service/object, notified through its virtual +3Ch |
| +138h | Additional flags; transform invalidation clears bits10h/20h |
| camera +2F0h | Separate camera validity flags, including projection8h, VP10h, inverseVP20h |

Matrices use the row-vector local * parentWorld convention. These offsets do
not define a complete ownership-safe native object layout.

## Refresh and descendant walk

`00b6db70` / `BSP_Transform_RefreshWorldMatrix`: ECX=this, no stack
arguments, plain RET. With a parent it first refreshes that parent if its +5Ch
bit2 is clear, then calls affine multiply `00b6d4d0` to write
world = local * parent.world. Without a parent it copies local to world using
`004134f0`. It finally ORs bit2. There is no cycle detection or child traversal.

`00b6da30` / `BSP_Transform_InvalidateValidDescendants`: ECX=this,
no stack arguments, plain RET. Iterate firstChild then each nextSibling. Only
if the child's +5Ch bit2 is set: clear child +138h bits10h/20h, clear child
+5Ch bits2h/8h, and recurse if it has children. An already-invalid child skips
its entire subtree. This depends on a consistent subtree dirty invariant; it
is not an unconditional tree invalidation. The walker does not notify +A0h,
dispatch a virtual callback, or write camera +2F0h. Do not invent such behavior
for cameras reached only through this walk.

## Direct setters and exact order

All setters below take ECX=this, one stack pointer, callee cleanup RET4
(position setters tail-jump to a matrix setter using that same stack slot).

| Address / proposed name | Operations in native order |
|---|---|
| `00b6dab0` / `BSP_Transform_SetLocalPosition` | FLD/FSTP three input floats into +E0h/+E4h/+E8h; replace stack argument with this+B0h; tail-jump virtual+38h |
| `00b6dae0` / `BSP_Transform_SetWorldPosition` | FLD/FSTP three input floats into +120h/+124h/+128h; replace stack argument with this+F0h; tail-jump virtual+34h |
| `00b6db10` / `BSP_Transform_SetLocalMatrix` | Copy source to +B0h; if flags+5Ch & Ah is nonzero, clear +138h bits10h/20h, assign flags+5Ch=0, notify +A0h virtual+3Ch if present, then invalidate descendants if firstChild exists |
| `00b6e870` / `BSP_Transform_SetWorldMatrix` | Copy source to +F0h; notify +A0h virtual+3Ch; derive local through 00b6e7e0; invalidate descendants; call this virtual+40h; finally assign flags+5Ch=2 |

Local matrix copying happens even when its invalidation branch is skipped.
World matrix assignment has no equality or prior-validity guard. Its final
flags assignment occurs after the virtual callback, not before it. Float
copies use x87 loads/stores, so replacing them with a bitwise copy changes
some exceptional-float behavior.

`00b6e7e0` / `BSP_Transform_DeriveLocalFromWorld`: ECX=this, no stack
arguments. A root copies world to local. Otherwise ensure parent inverse +60h
valid: refresh parent world if needed, inverse through `00b63b30`, copy result,
OR parent +5Ch bit8. Then full multiply `00413920` computes
local = world * parent.inverseWorld through a temporary. This inverse assumes
an orthogonal scaled affine basis; it does not establish support for shear.

## Camera overrides

Live table entries at `00d62d1c`, `00d62d20`, `00d62d24`, `00d62d28`
contain `00b713d0`, `00b71400`, `00b71460`, `00b71430` respectively.
Their order agrees with matrix slots +34h(world) and +38h(local) used by the
position setters. Each 34-byte wrapper has ECX=camera, one stack source pointer,
RET4, and performs exactly:

1. camera.flags2F0 &= FFFFFE4Bh.
2. Call its base setter with the original pointer.
3. Call `00b70660` with ECX=camera.

| Wrapper / proposed name | Base call |
|---|---|
| `00b713d0` / `BSP_Camera_SetLocalPosition` | 00b6dab0 |
| `00b71400` / `BSP_Camera_SetWorldPosition` | 00b6dae0 |
| `00b71430` / `BSP_Camera_SetLocalMatrix` | 00b6db10 |
| `00b71460` / `BSP_Camera_SetWorldMatrix` | 00b6e870 |

The mask clears bits4h/10h/20h/80h/100h, preserving projection-valid bit8h.
Position wrappers therefore enter the camera matrix override through the
base position setter's virtual tailcall: the mask and 00b70660 refresh can
occur twice. Omitting that nested dispatch is not a faithful callback trace.

`00b71490` / `BSP_Camera_SetViewMatrix` takes the same interface: clear the
same camera mask, inverse supplied matrix with 00b63b30 into a stack temporary,
call virtual+34h(world setter) with the temporary, then call 00b70660.

`00b70660` / `BSP_Camera_RefreshWorldDirectionAndTarget`: ECX=camera,
no stack arguments, plain RET. Ensure world bit2; copy world indices8..10
(+110h/+114h/+118h) to +1ACh/+1B0h/+1B4h via FLD/FSTP. Check world bit2
again; copy world translation to three float stack slots; add direction to
translation, spill each result to float32, then copy to +1A0h/+1A4h/+1A8h.
These values suggest direction and target; semantic names remain hypotheses.
It does not mark view/inverse valid or write projection flags.

At camera table +40h (`00d62d30`) the target is `00b6dbe0`: clear
+138h bits10h/20h, then tail-dispatch attached +A0h virtual+3Ch if nonnull,
otherwise RET. Adjacent +3Ch target `00b6dbc0` instead clears +138h bits4h,
8h,10h,20h and performs the same notification. Neither clears camera flags.
The attached service callback implementation remains unresolved.

Projection parameter invalidation is separate: scalar setters 00b6fbb0,
00b6fbd0,00b6fbf0,00b6fc10 apply FFFFFF41h (clearing projection bit8 too).
Explicit projection setter 00b6fd60 applies (flags & FFFFFF4Bh) | 8h.
See SHADER_CAMERA_CONSTANT_ANALYSIS.md and CAMERA_PROJECTION_EVIDENCE.md.

## Hierarchy mutation boundary

`00b6e010` attaches a child, sets parent, propagates +A4h scene/service via
00b6d890, performs additional virtual+50h handling, links it as first child
with doubly linked sibling adjustments and increments count. If child world
bit2 was set it clears child world/inverse validity and invalidates its valid
descendants. It calls child virtual+40h afterward.

`00b6d940` removes a child only when child.parent equals this; clears parent,
patches neighboring links/head, decrements count. It does not itself clear the
removed child's sibling pointers or validity. `00b6d820` clears all immediate
children's parent pointers then clears head/count, without dirty propagation.

`00b6e680` reparent combines unlinking, +A0h detach through 00b8f4c0,
+A4h propagation, type/virtual callbacks, attach, conditional self/descendant
invalidation, and virtual+40h notification. `00b6d850` recursively detaches
+A0h services. `00b6d890` also uses scene registration 00b721f0/00b72220.
These ownership and callback paths are unresolved; matrix algebra alone is
insufficient to reconstruct full native reparenting.

A typed graph/cache fragment can faithfully expose the established matrix
refresh and validity operations under an explicit supplied-graph contract.
It must not claim complete camera setter/hierarchy behavior until notification,
registration, and ancestor-driven camera combination invalidation are resolved.
No evidence here establishes an automatic +2F0h invalidation for a camera that
is merely a descendant of a moved transform.

## Byte identity for previously undefined wrappers

SHA-256 covers exactly 34 bytes including RET4, excluding following INT3 padding:

| Address | SHA-256 |
|---|---|
| 00b713d0 | 995d377099e60fb7f84efa29c13d9a1f46b6bc8784064bf4dd54fea9ec580a3b |
| 00b71400 | 9d6c251d4719dc2a48c42b5d5d1906bd235bcfbe361b7db279fa352bbea6319c |
| 00b71430 | 80403d7b83e4bb010b0bc1a34126e1b2ff0c670dd52db4398d93605b3fa1cc81 |
| 00b71460 | 138e48f3a0542e5005f2316f167ad635a3aa8a93df38d634cf5cac436dc59bd9 |

No names were applied and no binary replacement ABI or runtime parity is
claimed by this analysis.
