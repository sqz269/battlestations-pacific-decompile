# Structured-resource hierarchy and bounds

Addresses: `00b7f100`, `00b93310`, `00b7eb90`, `00be99d0`, `00bf02c0`,
`00b7d220`, `00b7d160`.

The `Hierarchy` branch consumes `Item` children and produces flat records with
numeric parent/resource fields, an owned name, matrix storage, flags, a sphere,
and a box. This parser does not build or validate a parent-pointer tree. The six
box floats are ordered **minimum X/Y/Z, maximum X/Y/Z**; the sphere branch
computes the same slots as center minus/plus radius. Both branches preserve
encounter order, so the last sphere or explicit box replaces the box output.

Seven complete code spans (1,306 bytes) and six data spans (79 bytes) freshly
match the installed PE. Every live read/export verified project `bsp` and
`/battlestationspacific.exe`. Exact lengths, hashes, ABIs, preserved comments and
annotation proposals are in
[structured_resource_hierarchy_bounds_audit.json](../reports/structured_resource_hierarchy_bounds_audit.json).
Raw evidence is under `exports/bsp/parallel_structured_hierarchy_bounds/`.
This packet adds no C++, tests, shared metadata, or Ghidra mutations.

The parent dispatcher and its resource-item branch are separately covered by
[STRUCTURED_RESOURCE_DISPATCH.md](STRUCTURED_RESOURCE_DISPATCH.md). Its
`00b87aa0` resource-item append is a different call from the hierarchy-record
append `00b87ae0` discussed below; their ownership contracts must not be merged.

## Hierarchy container: 00b7f100

ABI: ECX resource manager, stack pointer to the input reader-handle wrapper,
RET 4. The manager is preserved in EBP and the wrapper pointer in ESI.

At `00b7f11f/00b7f188`, `00715bf0(&parentHandle)` supplies the loop predicate in
AL. Each iteration calls `00bea680(&parentHandle, &childHandle)`, checks the
child name pointer at child-object `+14h`, and compares it case-insensitively
with the byte-verified literal `Item`. A match calls `00b7eb90(manager,
&childHandle)`; null/unrecognized names call `00be9c40(&childHandle)`.
Both normal branches then call `00be9ed0(&childHandle)` before repeating.

The normal-path wrapper lifecycle is established at these call sites. Reader
allocation, shared-stream ownership, payload bounds, error propagation, and
unwind cleanup still depend on the reader helpers; this packet does not claim
their full implementations. No temporary node is retained directly by the
hierarchy container after its child-cleanup call.

## Item record: 00b7eb90

ABI: ECX manager, one reader-handle-wrapper pointer on the stack, RET 4.
The function calls `00b87a90` with ECX equal to `0x84` and treats returned EAX
as storage for a 132-byte record. That helper is currently misclassified in the
inventory as a static-destructor stub; its allocator implementation was not
expanded here. The following record writes and field dispatch are direct
assembly evidence, independent of that inaccurate name.

| Record offset | Native field/tag | Initialization in this body | Field effect |
|---|---|---|---|
| `+00h` | `Parent` | `FFFFFFFFh` | `00be9a00` result overwrites this dword. Numeric meaning/index validation is not established. |
| `+04h/+08h` | `Name` length/data | Zero/zero | `00bea010` returns a temporary name; allocate/copy its stored-length bytes into this owned wrapper, then destroy the temporary. |
| `+0Ch..+4Bh` | `Matrix` destination | No writes here | `00b936e0(&childHandle, record+0Ch)` fills this area. Matrix read/layout semantics remain a separate helper contract; do not invent an identity default. |
| `+4Ch/+50h/+54h` | `Resource` dword array/count/capacity | Zero/zero/zero | Append each `00be9a00` result; no pointer lookup or resource retain appears here. |
| `+58h` | `Flags` | Zero | `00be9a00` result overwrites it; individual flag meanings are unknown. |
| `+5Ch/+60h/+64h/+68h` | `BoundingSphere` center/radius storage | `(0,0,0,1e10)` | `00b932e0` fills the sphere, then `00b7d220` computes and writes the box. The sphere-reader body is not audited here. |
| `+6Ch/+70h/+74h` | Box minimum XYZ | `(-1e10,-1e10,-1e10)` | Replaced by sphere-derived or explicit box values. |
| `+78h/+7Ch/+80h` | Box maximum XYZ | `(1e10,1e10,1e10)` | Replaced by sphere-derived or explicit box values. |

The two float constants were freshly decoded from disk-matching bytes:
`00ce4adc = D01502F9h = -1e10`, and `00ce4970 = 501502F9h = 1e10`.
No matrix initialization is present between allocation and child parsing;
what the allocator supplies for that region remains unknown.

`Parent`, `Resource`, `Matrix`, and `Name` use explicit null-name checks followed
by CRT `stricmp`. The last three tags use string helper `00425850`, whose
existing audited nonnull contract also delegates to `stricmp`. Thus recognized
nonnull tag spelling is case-insensitive. Unknown children call `00be9c40`.
Every normally completed child is then cleaned up by `00be9ed0`.

For the resource-value array, count equal to capacity requests
`max(capacity + 4, 8)` through `00b7d640`, writes the next four-byte value, and
increments count (`00b7ecae..00b7ecde`). Order and duplicates are preserved.
The reserve helper's allocation/failure behavior is outside this packet.
The appended values are raw numeric results, not retained resource pointers.

Repeated `Parent`, `Name`, and `Flags` fields replace their prior values.
Repeated `Matrix` fields call the same helper with the same destination; its
internal write policy remains unreviewed. Each `Resource` adds a value. Each
sphere dispatches the sphere reader and then replaces box storage from the
resulting sphere values. An explicit box changes only the box, so a later
box need not match the retained sphere. Neither branch checks radius sign,
minimum-versus-maximum order, finiteness, or parent/resource index validity.

After consuming all children, `00b7ee70..00b7ee78` loads the current resource
from manager `+24h` and calls `00b87ae0(resource, record)`. It passes the same raw
record pointer and performs no local record destruction afterward. That is a
storage handoff boundary, not proof of a particular owning container: append,
reference counting, exception cleanup, and final record/name/array destruction
remain gated on `00b87ae0` and the resource destructor. The null-allocation
branch has no graceful error return; valid record storage is a necessary domain
assumption for interpreting subsequent field writes.

## Explicit box values: 00b93310

ABI: ECX reader-handle-wrapper pointer, EDX destination for six floats, no stack
arguments, RET. It preserves those inputs, calls `00be99d0` six times, and uses
`FSTP float` stores at destination offsets `0,4,8,0Ch,10h,14h` in that order.
No count field, retry, validity check, coordinate transform or value sorting is
performed in this 66-byte routine.

The Item branch passes destination `record+6Ch`. The separate dispatcher branch
uses a six-float local and copies it into current resource offsets
`+28h/+2Ch/+30h/+34h/+38h/+3Ch`. These are two storage locations consuming the
same six-value schema, not proof that their enclosing objects share a layout.

## Sphere conversion: 00b7d220 and 00b7d160

`00b7d220` takes ECX output box, one sphere-pointer stack argument, EAX output
box, RET 4. It calls the leaf through a temporary six-float output, then copies
all six values using x87 float loads/stores.

The leaf `00b7d160` takes ECX output box and EDX sphere, has no stack arguments,
returns the output pointer in EAX, and ends in RET. Complete assembly establishes:

```text
output[0..2] = (sphere[0]-sphere[3], sphere[1]-sphere[3], sphere[2]-sphere[3])
output[3..5] = (sphere[0]+sphere[3], sphere[1]+sphere[3], sphere[2]+sphere[3])
```

This grounds center XYZ and radius as the sphere's four input components, and
minimum XYZ followed by maximum XYZ as the box's six output components for a
nonnegative radius. It uses x87 arithmetic and float stores; it does not clamp
a negative radius, normalize coordinates, or reject nonfinite values. For a
negative radius the nominal minimum and maximum can be inverted. Exceptional
x87 values/control-word behavior is not a tested bitwise-parity claim.

## Float reader, byte accounting and short reads

`00be99d0` is a 15-byte adapter, ECX pointer to a reader handle, no stack
arguments, RET, result in x87 ST0. Let `node = *handle`. It forwards the object
pointer stored at node `+8` as ECX and passes `&node[+20h]` on the stack to
`00bf02c0`. That object is itself a wrapper: `00bf02c0` dereferences its first
dword to obtain the actual stream before invoking stream virtual `+44h`.

`00bf02c0` takes ECX wrapper, one byte-accounting pointer stack argument, RET 4,
and returns ST0. It creates a local actual-count dword, passes its **nonnull
address** to stream virtual `+44h`, rounds/reloads the returned value through a
32-bit float temporary, and subtracts the reported actual count from the passed
dword at `00bf02df`. The decrement is consistent with remaining-byte accounting;
this body does not seek or advance a cursor itself. The underlying stream read
owns position advancement.

There is no precheck of node `+20h`, clamp of the requested scalar to that value,
test for four transferred bytes, retry, error branch, or protection against
underflow. The actual-count local is not initialized by this wrapper before
the virtual call, so its valid domain requires the callee to write that count.
The six-float caller also has no early-stop condition. A shortage in the node's
remaining budget alone does not stop the read in these inspected wrappers;
whether a particular underlying stream enforces a separate bound is not known.

Existing [STREAM_SCALAR_READERS.md](STREAM_SCALAR_READERS.md) establishes that
the known memory/physical stream vtables map `+44h` to `00be4360`: one four-byte
read followed by x87 `FLD`, with no byte swap. Complete reads therefore interpret
x86 little-endian float32 data. That prior scalar audit is a dependency, not a
newly exported function in this packet; arbitrary alternate stream vtables
retain their own virtual-method contracts.

The same scalar audit identifies a material malformed-input distinction:
`00be4360` reuses its incoming actual-count-pointer argument slot as the read
buffer. With this packet's **nonnull** pointer, bytes not overwritten by a short
read retain numeric pointer-address bits. The font parser's existing host
adapter instead uses a null pointer and starts from zero. Reusing that adapter
would silently change structured-resource short-read behavior. Full scalar
reads can share decoding; truncated reads cannot claim parity through zero-fill.

## Supported reconstruction boundary

The schema and sphere-to-box arithmetic are concrete for valid complete input.
A host decoder may require six complete floats, valid initialized output storage,
bounded collections, and explicit record ownership. Those are rejection/lifetime
policies absent from these native bodies and must be labeled accordingly.
Do not promise malformed-input, cross-stream, native ABI or render parity.

Remaining dependencies are hierarchy append `00b87ae0` and destruction,
allocation `00b87a90`, dword-array reserve `00b7d640`, reader iteration/skip/cleanup,
integer reader `00be9a00`, string reader `00bea010`, matrix reader `00b936e0`, and
sphere reader `00b932e0`. Parent/resource values' graph semantics and flag bits
also remain unproven. The current packet supplies an exact field layout and
numeric boundary without inventing those missing behaviors.
