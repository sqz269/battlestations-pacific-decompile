# Generated instance geometry

`00b4c8d0` creates a model wrapper around a new mesh with one generated draw
section. Its second vertex stream uses the renderer's existing shared dynamic
vertex buffer. The allocator does not allocate a private buffer sized for an
instance group. The C++ fragment implements that retained geometry contract;
model construction and the full native material clone remain explicit inputs
to the enclosing integration.

The complete `00b4c8d0..00b4caca` span, dynamic stream constructor, material
clone, relevant vtables, startup buffer instructions and float sentinel match
the installed PE. Exact ranges and hashes are in
`reports/generated_instance_geometry_audit.json`. Ghidra project `bsp` and
program `/battlestationspacific.exe` were checked by the CLI before each query.
No Ghidra annotations or flow overrides were changed by this packet.

## Calling convention and construction

The decompiler loses register inputs and misidentifies several locals. The
assembly establishes ECX as the argument forwarded to model constructor
`00b75030`, EDX as the generator, and two stack inputs: source mesh followed by
selected draw section. It returns the new model wrapper in EAX and uses `RET 8`.
At `00b4c989`, `[ESP+30h]` is the selected section; at `00b4c9ee`, `[ESP+2Ch]`
is the source mesh. These are not stack-local pointer aliases in the C++ API.

The function allocates `184h` bytes and invokes `00b75030`, then allocates and
constructs a `BCh`-byte mesh through `00b73b60`/`00b73d70`. At `00b4c94c` the
x87 load reads `00d7a260`, whose bytes are `00 00 80 bf` (`-1.0f`). It pushes
two copies into `00b75170(0, mesh, -1.0f, -1.0f)`. That method retains the mesh
at wrapper `+180h`; the two sentinel values preserve its existing `+178h` and
`+17Ch` defaults. There is no per-instance floating-point capacity calculation.
Dry-run flow inspection of the three allocation call sites found `NONE`.
The main allocator has no direct CRT free call or hidden free-return tail.

The concrete wrapper vtable is `00d62de8`: virtual `+48h` points to `00b6e8c0`
and `+50h` to `00b6ed80`. Their attach/position behavior belongs to the upload
packet. The geometry fragment returns an owning typed geometry, not the native
wrapper or an ABI-compatible replacement.

## Shared instance stream

`00b4c973..00b4c987` obtains generator `+10h` through `00b556b0` and calls
renderer virtual `+5Ch` with `(0, 1000h, declaration)`. The table at `00d5f0a8`
has `00b287c0` in slot `00d5f104`. The renderer constructor writes this table
at `00b3243b`; application initialization calls that constructor at `0073da88`.

Factory `00b287c0` calls logical-stream constructor `00b4bc00` with the order
`(count, declaration, flags)`. For `(flags & F000h) == 1000h`, the constructor
retains renderer `+1974h`, registers the raw logical pointer on that physical
wrapper through `00b4b1e0`, and sets stream `+5Ch` to `FFFFFFFFh`. It retains
the declaration at `+68h`, stores count zero at `+64h`, and original flags
`1000h` at `+60h`. The private `CreateVertexBuffer(count * stride, ...)` branch
is not taken. See `docs/MESH_REGISTERED_BUFFER_BACKING.md` for that distinct
mesh-static branch and the renderer's additional raw-object registries.

At `00b4c9aa`, the allocator overwrites the base tag with `80000000h`. The
temporary stream reference is released only after the mesh retains stream 1.
The shared pool is already initialized by renderer startup: the instructions
at `00b2b095..00b2b0d9` install renderer `+1974h`, create a `1000000h`-byte
vertex buffer with usage `208h`, FVF zero and DEFAULT pool, and attach it with
wrapper flags `1000h` and capacity `1000000h`. The capacity is **16 MiB shared
across logical streams**, not 4096 vertices or a count per generated model.

The C++ fragment takes that existing wrapper and validates both its metadata
and `IDirect3DVertexBuffer9::GetDesc`. It does not create, resize, lock, clear
or rewind the pool. It registers the fresh stream and starts with count zero,
offset `FFFFFFFFh`, flags `1000h`, and tag `80000000h`. The existing dynamic
lock path updates the logical count/offset during upload. Buffer capacity and
frame rewind remain renderer/upload responsibilities.

## Copied and retained geometry

Stream 0 is selected-section `+3Ch`, not an assumed source mesh stream zero.
The generated mesh and its section retain that same logical stream identity.
The source mesh's index pointer at `+60h` is retained as-is, including null.
No vertex or index bytes are copied by this function.

Only the five source section DWORDs at `+8h`, `+Ch`, `+10h`, `+14h`, `+18h`
are copied. The write order is `+8h`, `+Ch`, `+14h`, `+18h`, `+10h`.
Fresh section construction leaves instance count `+1Ch` and depth bias `+34h`
zero. The caller subsequently sets the instance count through `00b85590`.
Source section count, bias and other fields are not inherited.

The section retains the material clone through `00b864c0`; the mesh retains
the section through `00b73c60`. `00b85b80` then appends retained streams in
mesh, instance order. Finally, `00b556c0` returns generator `+14h`, and
`00b86650` retains that prebuilt combined layout at section `+50h`. Layout
construction belongs to generator `00b55b20`; this allocator neither appends
declarations nor infers a stride. For the building generator, the known
instance declaration is nine `uf44` elements with 144-byte stride.

The function drops the temporary material, section and mesh references before
returning the model. The host geometry uses shared ownership to preserve those
reachable objects without reproducing exact intrusive reference totals. Its
instance stream unregisters from the physical buffer only when the final
geometry or state-cache owner releases it. `D3D9StateCache` must outlive all
such references; unbind/invalidate its bindings before teardown. Callers must
not replace the registered stream's physical owner while it is registered.

## Material-clone follow-on contract

`00b4c9c3..00b4c9e3` allocates `110h` bytes and calls `00b18b60` with source
section material `+20h`. The full clone span `00b18b60..00b18cf0` was inspected
in assembly and compared with the PE; ABI is ECX destination, stack source,
EAX destination, `RET 4`. It is not a plain memory copy or the same material
object. The observed transfer is:

- Copy source `+8h` and `+Ch`, copy byte `+10Dh`, and retain `+Ch` only when
  the copied byte is nonzero and that pointer is non-null.
- Iterate signed source WORD count `+34h`; copy and retain the pointers from
  `+10h`, growing the destination count. The constructor zeros nine pointer
  slots through `+30h`; malformed counts above that capacity remain outside
  a future bounded clone interface.
- Set destination byte `+10Ch` to one; copy 17 DWORDs from `+38h` through
  `+78h` using `REP MOVSD`.
- Retain source effect pointer `+7Ch`, and copy DWORDs `+104h` and `+108h`.
  Destination `+100h` is initialized to zero rather than copied.

The material's owner kinds, complete destructors and cached regions remain
unported. `GeneratedInstanceMaterialClone` therefore requires an explicit
already-cloned owner and its effect queue metadata. A host diagnostic owner
can exercise allocation/upload but does not prove the native clone path.
No placeholder native clone implementation is supplied.

## Validation boundary

The packet contains reconstructed C++, complete principal byte comparisons and
assembly-backed ownership evidence. Build and installed-renderer integration
are performed by the primary integrator after merging. No new test suite was
added. Native ABI compatibility, complete model/material lifetime and gameplay
validation are not claimed. Invalid inputs and allocation failures use checked
HRESULTs and output rollback as new interface policy; native null-allocation,
diagnostic and exception behavior remains outside this success domain.
