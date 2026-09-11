# Native node world refresh

The complete `00B6DB70..00B6DBC0` body (80 bytes, end excluded) is exposed as
`void __fastcall refresh_native_camera_world_00b6db70(void* actual_node)`.
ECX is an actual original-layout node; there are no stack arguments and no
context or callback parameter. It uses the complete raw matrix copy at
`004134F0` and affine composition at `00B6D4D0`. The canonical hierarchy integration described below supplies the raw parent
words through the existing node binding; semantic matrix APIs remain separate.

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

The canonical hierarchy migration is now integrated. `NativeNodeStorage`
stores actual node addresses at +30/+34/+3C/+40. `CameraTransformLink` reads
and writes those same DWORDs and resolves current companions through the
existing `SceneAttachmentRuntime` binding vector. Its immutable raw identity
comes from `NativeNodeBinding`; there is one graph and no temporary swizzling.
`NativeCameraOwner` uses this binding path. The raw entry takes the actual
prefix address, not the address of the C++ owner or transform companion.

All traversed semantic nodes require stable live same-runtime bindings.
Preconstruction registration touches no backing words. Exact companion
retirement does not inspect dead backing, and stale identities cannot retire
a replacement at a reused address. Link reads still require live backing;
the runtime must outlive its companions. Host copy/move assignment validates
all four source links and destination domains before publishing any fields.

The primary host sequence covers nonnull reparent, reentrant prepend, current
sibling reload, full raw frustum/inverse VP, assignment failure without partial
publication, protected backing retirement and address reuse. See
`NATIVE_NODE_RAW_HIERARCHY.md` for the verified canonical integration. Camera
tail fields were explicit fixture inputs; full camera owner constructor and
destructor replay, unrelated owner fields and original object ABI remain open.

The following original-body evidence predates that host migration and retains
its narrower historical scope.

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

In the original world-body packet no new test or runtime fixture was added: complete instruction-byte equivalence
and the complete provider proof establish the raw recursion and alias order.
The existing tests do not validate this new raw entry against a nonnull current
companion-backed owner. No original/linked/runtime execution, complete hierarchy
migration, camera-owner compatibility, renderer behavior or game validation is
claimed. Ghidra, shared metadata and permanent build configuration were not
changed. Function names remain descriptive hypotheses.


## Primary main-object validation

The raw world entry is registered in main. Strict MSVC Win32 compilation and
both existing CTests passed; eight native seeds matched. The primary checked
48 worker artifact pins and the compiler pin, then twenty current files:
nineteen literal matches and one existing file with only CRLF/LF differences.
Nine fresh guarded spans matched all 772 bytes.

The actual main library
`6eb2fce9d811380365d3031d51de8b82a0f98bcf7a997c3695c7c231295d9d7a`
and three exact archive members were frozen. Complete original/main COFF
verification covers the 80-byte world routine and both matrix providers,
522 bytes and 181 instructions, after only three direct-call operands and
one equal read-only constant operand. No new runtime or linked fixture is
claimed here. The subsequent canonical hierarchy migration and host runtime checks are
recorded in `NATIVE_NODE_RAW_HIERARCHY.md`; this original body proof is unchanged.

The read-only bundle is `local/camera_world_primary/`, manifest SHA256
`0f27423f70503ae4f803ff3c10da72b0e9100aa834824aa9751b9ec63292fea7`.
The existing Ghidra name and comments were preserved, evidence was appended
and saved, and the export and complete source record were refreshed.
