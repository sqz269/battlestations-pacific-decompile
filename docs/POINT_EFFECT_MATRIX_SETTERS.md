# Point-effect world and relative matrix setters

`0053D9C0` and `0072AA80` now have complete executable bodies over the existing
`PointEffectInstanceStorage`. The first accepts world space; the second accepts
the effect's parent-relative space. Both use the actual node at effect+110 and
the effect parent at+8C, which is distinct from that node's hierarchy parent.
These helpers close two dependencies of `008680B0`; they do not complete that
constructor or assume successful node allocation, entry creation or insertion.

## Evidence and native ABI

The read-only Ghidra export/query batches verified configured project file
`C:/Users/sqz269/bsp.gpr`, project `bsp`, and program
`/battlestationspacific.exe`; `bsp.py` also checks x86 language and image base
before every live query. The game and saved Ghidra analysis were not changed.
Original function names remain `FUN_0053d9c0` and `FUN_0072aa80` until the
integrator applies the proposed ledger names. The exports are shared ignored
`exports/bsp/functions/0053d9c0` and `0072aa80`.

Both native entries receive ECX=effect and one stack matrix pointer, with
callee cleanup `RET4`. No returned value is required by the recovered callers.
`0053D9C0..0053DA25` contains36 instructions/102 bytes; its final RET starts at
`53DA23`. `0072AA80..0072AB03` contains49 instructions/132 bytes; final RET
starts at `72AB01`. Ghidra's saved no-parameter signatures do not recover the
actual register/stack arguments. Assembly is necessary to establish receiver,
multiply operand order, destination, and callback-sensitive reloads.

The constructor selects `72AA80` for its nonzero transform byte (`86825B`),
and `53D9C0` for zero (`868269`). Live bytes at `D62CBC`, the constructed
`D62C88` node vtable's+34 slot, are `70 E8 B6 00`: concrete target `B6E870`.
At `D62CC8`, slot+40 is `E0 DB B6 00`: concrete target `B6DBE0`.
The pointer test mask `00E1767C` is a nonzero immediate in both bodies. The
native NEG/SBB/TEST sequence implements a null-pointer test; it does not read
a global at `E1767C` or inspect a type token. No numerical constants are added.

## World setter `0053D9C0`

1. Capture node+110 and its current virtual+34, then call it with the original
   input pointer (`53D9C6..53D9D7`). No source snapshot is taken.
2. Reload parent+8C after that virtual call (`53D9D9`). If nonnull, call canonical
   `B6E0D0` on this parent, then canonical `413920` with left=input,
   right=parent inverse-world and a separate64-byte stack destination.
3. Canonical x87 `4134F0` copies the result into relative+D0. If parent is null,
   it copies directly from the original input pointer.

A callback can replace the effect parent or mutate a source matrix aliased to
effect storage. Both changes are visible to the post-callback computation.
Parent inversion can also refresh an aliased source before multiplication.
The original input reference remains live; the code does not retain any owner.

## Relative setter `0072AA80`

1. Canonical x87-copy input into actual relative+D0 (`72AA95`), then capture
   parent+8C (`72AA9A`). Input aliasing the parent's world therefore copies the
   old matrix before a refresh of that parent.
2. For a nonnull parent, refresh its world through canonical `B6DB70` only
   when valid-flags bit2 is clear. Capture the current node's dispatch table,
   then compute relative * captured-parent-world with canonical `413920` into
   a separate stack matrix. Read the captured table's+34 entry and reload the
   current node receiver for the invocation (`72AABB..72AAE1`).
3. For a null parent, invoke the current node+110 virtual+34 with actual+D0
   (`72AAED..72AAF9`). There is no substitute temporary in this branch.

The canonical multiply has no callbacks, globals, branches or FP-control
changes. Therefore the same binding can retain the captured table projection
across it, with no concurrent mutation. Neither branch writes relative after
the virtual call, so callback changes to that storage survive.

## Concrete node dispatch and shared implementation

`SceneNodeAttachment` now includes explicit `set_world_matrix`, the current
virtual+34 on the same scene/transform binding. Its default is null (unbound).
The caller must supply a recovered concrete operation; an unbound operation
throws instead of producing a successful setter. A native `B6F5A0` node should
explicitly bind:

```cpp
binding.scene_attachment.set_world_matrix = set_native_node_world_matrix_00b6e870;
binding.scene_attachment.world_changed = native_node_world_changed_00b6dbe0;
```

Keep these current when an owner's native vtable phase changes. Unknown derived
virtual+34 operations remain required; an integer vtable identity is never
cast to a host callable address. No second node map, reference count, transform,
scene binding, or hierarchy is created. Bindings and all referenced storage
must remain alive through native reentry. Concurrent mutation is excluded.

The concrete node adapter uses the canonical world setter's new overload with
explicit callback context. The previous three-argument API delegates to the
same body, preserving existing users. No TLS, global callback or copied body
is introduced. Native order remains:

1. X87-copy source into node world+F0.
2. Notify the current attachment+A0 virtual+3C when an attachment exists.
3. Derive local+B0 from the current world and hierarchy parent, then invalidate
   descendants through the existing canonical functions.
4. Resolve this same node's **current** scene binding and virtual+40, then call
   it. A first callback can change this override; it is not captured early.
5. Assign node valid-flags=2 only after that callback returns (`B6E8AB`).

Concrete `B6DBE0` clears auxiliary bits30h and may notify the current attachment
a second time. The canonical scene implementation is reused. The node adapter
rejects a nonnull attachment with no actual virtual+3C binding as an invalid
host projection. A missing+40 fails at its late native call point, preserving
the copied/derived matrices and skipping the final valid=2 write.

## Validation and remaining work

Direct MSVC Win32 `/std:c++17 /O2 /W4 /WX /fp:strict /MD` compilation passed for
both the new setters and the canonical callback overload. `scripts/build.ps1`
passed `reconstructed_math` and `native_math_differential` (2/2), after
`ghidra_export.py verify-seeds` matched all existing native seeds. The parent
owns `cmake/startup.cmake` and must register the new source there; the shared
API changes were built by the repository build, and the setter source was
directly compiled and linked into the focused fixture.

One ignored `local/point_effect_matrix_probe.cpp`, linked with
`/MANIFEST:EMBED`, passed actual node/effect backing checks, source+D0 aliasing,
callback parent replacement and inverse-cache refresh, nested setter reentry,
replacement of the current+40 callback, source aliasing parent-world before
relative refresh, callback mutations surviving the relative setter, and
missing+34/+40 failure boundaries. This fixture exercises reconstructed host
operations; it does not execute the original setter bytes or validate native
ABI compatibility. No permanent tests were added and the game was not run.

Full `008680B0` still requires the actual allocation/binding/unwind chain for
the constructed node, the complete `8672A0`/`8670A0` reference-array resize and
cleanup contract, current row virtual+18 creation and output/temporary
ownership (`6FBEB0`, `6CF070`), and real manager `4D1100`/`867500` insertion
including the `866440` lock and `74D780` list operation. Its native SEH cleanup
and final consumed-template release must surround these operations in the
already documented order. See `POINT_EFFECT_INSTANCE.md` for exact constructor
stages and the unwind map. The constructor remains required, not reconstructed.
