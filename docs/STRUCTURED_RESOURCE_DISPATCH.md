# Structured resource factory and dispatch

Addresses: 0071b810, 0071b870, 0071bb40, 00b7e970, 00b7f430, 00b86930, 00b87aa0, 00b88260.

The queued loader's concrete factory produces a 74h-byte game resource with
initial reference count 1, an owned requested name, and vtable `00cfd8cc`.
The root dispatcher visits child records in order and recognizes `Resource`,
`Hierarchy`, and `BoundingBox` case-insensitively. `Resource` children are
decoded through a registered type-parser map, then passed to the resource's
append method. An unknown item type produces an eight-byte fallback object
while its serialized payload is skipped.

This is a bounded structural and ABI reconstruction. It does not identify an
original source class name or supply the missing serialized reader, registered
item parsers, or resource destruction implementation. The enclosing queue route
is documented in [VFS_LOAD_WORKER_DISPATCH.md](VFS_LOAD_WORKER_DISPATCH.md).

## Matched evidence

`reports/structured_resource_dispatch_audit.json` contains complete installed-PE
versus Ghidra hashes, original ABIs, and annotation proposals. Every live CLI
verified `bsp`, `/battlestationspacific.exe`, x86 language, and image base
`00400000`. No Ghidra or shared metadata was changed by this packet.

| Address | Complete matched span | Role and original ABI |
| --- | --- | --- |
| `0071b870` | `0071b870-0071b8ce` | Factory creation; ECX factory unused; name stack; EAX object/null; RET4 |
| `0071b810` | `0071b810-0071b869` | Concrete construction; ECX object; name stack; EAX object; RET4 |
| `00b88260` | `00b88260-00b88318` | Base construction; ECX object; name stack; EAX object; RET4 |
| `00b7f430` | `00b7f430-00b7f5ab` | Root dispatch; ECX manager; parent-handle pointer stack; RET4 |
| `00b7e970` | `00b7e970-00b7eadd` | Resource-item dispatch; ECX manager; child-handle pointer stack; RET4 |
| `00b86930` | `00b86930-00b86945` | Unknown-item construction; ECX object; EAX object; RET |
| `00b87aa0` | `00b87aa0-00b87ada` | Base raw append; ECX resource; item pointer stack; RET4 |
| `0071bb40` | `0071bb40-0071bbf2` | Concrete append/classify; ECX resource; item pointer stack; RET4 |

The last target initially had no Ghidra function definition. Two bounded byte
windows established its branches and final RET4 at `0071bbf0`, followed by CC
padding. Its full 179-byte span was matched to disk and decoded with Capstone.
The three vtable ranges and tag-literal range are matched separately in the
report. Original class names are not inferred from adjacent strings.

## Object construction and observed layout

Factory `0071b870` allocates 74h bytes and calls `0071b810(name)`. Allocation
failure returns null. The constructor calls base `00b88260`, installs concrete
vtable `00cfd8cc`, and initializes three additional list storage triplets.

| Offset | Established initialization or use |
| --- | --- |
| +0 | Base vtable `00d63228`, replaced by concrete `00cfd8cc` |
| +4 | Intrusive reference count initialized to 1 |
| +8/+Ch | Owned native-string length/pointer, copied from requested name |
| +10/+14/+18 | Base raw-item array data/count/capacity, initially zero |
| +1Ch..+40h | Remaining base DWORDs initialized to zero |
| +28h..+3Ch | Six BoundingBox words written by root dispatch |
| +40h | Opaque metric difference written by enclosing loader |
| +48/+4C/+50 | Zeroed storage triplet for list whose object begins at +44h |
| +58/+5C/+60 | Zeroed storage triplet for list whose object begins at +54h |
| +68/+6C/+70 | Zeroed storage triplet for list whose object begins at +64h |

The concrete vtable's append slot+Ch points to `0071bb40`. Initial ownership
of the resource is established by the base constructor; destruction and cache
insertion ownership belong to later contracts.

## Root schema and ordering

`00b7f430` calls renderer global `00f8d394` virtual+50 before traversal and
virtual+54 after normal completion, including an empty root. Their precise
renderer behavior remains external.

The dispatcher begins its parent handle with `00be9a40`, tests remaining data
with `00715bf0`, and obtains each child using `00bea680`. It compares the child's
C-string at +14h with the following literals using CRT `stricmp`:

| Tag | Call | Output |
| --- | --- | --- |
| `Resource` | `00b7e970`, ECX manager, stack `&childHandle` | Appends parsed/fallback item pointers |
| `Hierarchy` | `00b7f100`, ECX manager, stack `&childHandle` | Separate hierarchy branch audit |
| `BoundingBox` | `00b93310`, ECX `&childHandle`, EDX six-float local | Six MOVSS writes to current resource+28h..3Ch |
| Null/unknown | `00be9c40`, ECX `&childHandle` | Skip unknown child payload |

Every child follows the same `00be9ed0(&childHandle)` cleanup path. Children
retain source order. The dispatcher does not clear the destination wholesale:
repeated Resource containers append, and later BoundingBox children overwrite
the prior six words. Reader-handle ownership, serialized byte layout, and
Hierarchy/BoundingBox callee internals are owned by the neighboring audit lanes.
The latter are documented in
[STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md](STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md),
including Item field ordering, repeated-field behavior, and bounding-sphere
updates to the box.

## Resource-item parser selection

`00b7e970` iterates children of the Resource container. The child type's native
string at +10h/+14h is looked up in the manager's registered parser map at +8h
through `00b7df40`. The caller rejects the sentinel and candidates ordered above
the query using empty-string handling and `stricmp`. Full map ordering remains
the helper's contract; the observed call gate is not a license to guess parser
registrations.

For a registered type, map-node+14h supplies a parser object. Its virtual+8
receives `&childHandle` and returns an item pointer in EAX. For an unknown type,
the branch allocates eight bytes and calls `00b86930`; that object has vtable
`00d631c0` and reference count 1. It then skips the child's payload with
`00be9c40`. Allocation-null still flows onward as a null item.

Both branches pass the returned pointer to current resource virtual+Ch. No
local item release follows that call; the local cleanup concerns the reader
handle. The initial reference and validity contract for **registered** parser
results remains external.

## Append and classification

Base append `00b87aa0` compares count+14h against capacity+18h. If full, it calls
`00b872f0` to grow to `max(capacity + 16, 16)`. It stores the raw item pointer
at data+10h[count] and increments the count. There is no AddRef in this body.
Growth and allocation guarantees remain outside the packet.

Concrete append `0071bb40` always performs the base append first. For nonnull
items, it then invokes item virtual+Ch with type tokens, in this priority order:

| Test token | Matching list | Append helper |
| --- | --- | --- |
| Global `00e19b64` | Resource+64h | `0071b9b0` |
| Return of `006f9a80` | Resource+54h | `0071b940` |
| Return of `00721c90` | Resource+44h | `0071b8d0` |

The first match returns immediately, so these specialized lists are populated
by priority rather than independently. A null item is still stored in the base
array but is absent from all three specialized lists. There is no explicit
reference-count operation in the full 179-byte append body. The type-token
names and helper ownership are not recovered here.

## Implementation boundary

A future dispatch projection can preserve these ordered branches with an
already verified reader, actual registered parser callbacks, actual resource
append behavior, and explicit renderer hooks. It must keep the native unknown
item fallback and payload skip, rather than treating unknown bytes as a parsed
object.

An owning resource implementation still needs registered-parser return
ownership, specialized-list helper behavior, and resource/item destruction.
Raw append plus absence of a local release is insufficient to claim verified
teardown. The actual binary-reader and concrete item-parser contracts are also
required; the current source search found no established implementations of
the audited reader/dispatch entry points.

This packet adds no C++, tests, shared metadata edits, or commits. The work is
assembly- and disk-verified, with source implementation, build validation, and
game validation explicitly remaining separate.
