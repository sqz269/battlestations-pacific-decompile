# Native node world refresh

The complete `00B6DB70..00B6DBC0` body (80 bytes, end excluded) is exposed as
`void __fastcall refresh_native_camera_world_00b6db70(void* actual_node)`.
ECX is an actual original-layout node; there are no stack arguments and no
context or callback parameter. It uses the complete raw matrix copy at
`004134F0` and affine composition at `00B6D4D0`. Existing semantic APIs and
companion-backed camera owners are unchanged.

| Native offset | Required contents |
|---|---|
| `+30` | Actual original-layout parent node address, or zero |
| `+5C` | Current flags DWORD; mask `0x2` marks a valid world matrix |
| `+B0` | Local matrix, 64 bytes |
| `+F0` | World matrix, 64 bytes |

The function captures the parent pointer once in EDI. For a nonnull parent it
tests mask `0x2` in the parent's low flags byte and recursively refreshes that parent
when missing. It then uses the captured parent address plus F0 as the right
matrix, composes local B0 into own world F0, and ORs the current own flags DWORD
with 2 after composition returns. A null parent takes the complete x87 matrix
copy from B0 to F0, followed by the same flag update. The function does not test
its own valid bit before recomputing. It does not reload the parent field after
recursion, detect cycles, validate pointers, snapshot matrices, or roll back
partial writes on an exception.

The original incidental EAX is the captured parent-world pointer after
composition, or the node's own world pointer after root copying. The naked body
preserves that result, both original RET instructions, and ESI/EDI saves. The
public void interface does not call it a world-matrix getter result. All
floating-point work and unsafe alias/store effects belong to the actual complete
raw matrix providers. They retain the original x87/SSE instructions and the
affine provider's equal-bit read-only positive-one constant at a new address.

The raw world body has no unresolved helper: direct recursion, raw copy and raw
composition are complete. Its callers must still supply an actual raw parent
chain. A matching set of scalar offsets alone is insufficient for that contract.

The existing representation boundary is concrete:

- `NativeNodeStorage` is the actual 174h prefix used in pool slots, but its +30,
  +34, +3C and +40 fields are `CameraTransform*` companion pointers.
  `CameraTransformBacking` and `CameraTransform` hold references to those same
  pointer fields. `NativeNodeBinding::transform_backing` binds them directly.
- `NativeCameraOwner` contains a `NativeNodeBinding` and exposes a
  `NativeNodeStorage&` prefix. Its companion is not an original node address.
  The current owning class must not be passed to the raw entry or cast into a
  raw parent chain merely because its matrix and flag members occupy the right
  offsets. A root with a zero parent does not establish general compatibility.
- Original constructor stores `B6F5E8..B6F5F6` zero +30/+34/+38/+3C/+40; later
  `B6F683` selects local B0, `B6F696` clears flags, and `B6F70E` selects world F0.
  Current construction mirrors those initial values, which do not reveal the
  nonnull representation mismatch.
- Original `B6E01D` stores the actual parent ECX-derived address into child+30
  before root/scene calls. Its later stores publish actual child/sibling
  addresses. Current `prepend_native_node_child_00b6e010` instead assigns
  `child.parent = &parent` where parent is a companion. `B6D940` similarly
  proves raw-address unlink stores. The current implementations preserve their
  semantic callback order but do not make their published words raw addresses.

A bounded integration plan, **not implemented in this packet**, is to make the
existing prefix hierarchy words canonical raw addresses and adapt companion
access at the representation boundary. Keep the same physical prefix, flag and
matrix storage; introduce no second parent graph or duplicate owner. Replace
the four `CameraTransform*&` link references with explicit accessors/proxies that
load/store the actual raw word and resolve companions through a stable identity
association when a semantic consumer needs one. A companion-to-raw conversion
uses its bound storage address, never the companion address.

The existing `SceneNodeAttachment::pointer_key` already records the actual slot
identity. `SceneAttachmentRuntime` currently exposes `resolve(CameraTransform&)`
and a light-key resolver; it does **not** expose a general raw-node-key resolver.
Adding and reviewing that reverse lookup, including bind, unbind and
`forget_destroyed_binding` lifetime behavior, is a named missing prerequisite.
It must use the existing binding's lifetime and identity, with no invented
allocator, ownership reference or caller-selected world callback.

The corresponding writers and traversal consumers must switch together before
mixed hierarchies can use the raw getter. The concrete initial review set is
construction/backing in `native_node_construction`, prepend/reparent
`B6E010/B6E680` in `native_node_parenting`, unlink `B6D940`, descendant invalidation
`B6DA30`, root propagation `B6D890`, and the existing destruction/scene child
walks. These implementations are present under companion contracts; their raw
hierarchy integration is named but incomplete. Preserve parent publication
before callbacks, current child/sibling reloads after callbacks, detach order,
and stable binding retirement. Do not temporarily swizzle shared +30 words to
call this routine: recursion, reentry and exceptions would observe a mixed
representation. Keep current semantic callers on their existing path until
the canonical-word/accessor migration and nonnull reparent/detach checks close.

Fresh evidence consists of nine guarded live-Ghidra/installed-PE spans totaling
772 bytes: the complete world body, both complete matrix providers, their one
constant, and bounded constructor/hierarchy-writer evidence. The analysis target
is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; each live query goes
through guarded `bsp.py ghidra`. Installed executable SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The actual strict Win32 library build and two existing CTests are required to
pass, with all eight native seeds verified. The full 80-byte owned object body
and complete 442 bytes of reached matrix-provider object bodies are compared
to the originals after only their direct-call/constant relocations. Exact
actual library members are extracted and frozen with the archive and objects
under ignored `local/camera_world/frozen`. The audit report pins the source,
provider and boundary files, compiler command, binary evidence and build logs.

No new test or runtime fixture is added: complete instruction-byte equivalence
and the complete provider proof establish the raw recursion and alias order.
The existing tests do not validate this new raw entry against a nonnull current
companion-backed owner. No original/linked/runtime execution, complete hierarchy
migration, camera-owner compatibility, renderer behavior or game validation is
claimed. Ghidra, shared metadata and permanent build configuration were not
changed. Function names remain descriptive hypotheses.
