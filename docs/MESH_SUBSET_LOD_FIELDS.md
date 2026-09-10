# Mesh subset, LOD, and name fields

The six mesh field readers and ten approved immediate helpers establish the
complete Subset, LODPhases, LODValue, mesh bounds, and WeightMapNames wire
contracts. `include/bsp/mesh_fields.hpp` and `src/mesh_fields.cpp` implement
owning host field values in the existing complete-transfer reader domain.
Effect and texture names are requests for later resource resolution; these
parsers do not construct materials, GPU objects, or drawable geometry.

## Evidence and original ABI

The 2026-09-10 audit used `bsp.py ghidra export/bytes`, whose client verifies
project `bsp`, program `/battlestationspacific.exe`, x86 language and image base
before each batch. The configured project is `C:/Users/sqz269/bsp.gpr`.
All 16 complete function ranges and five supporting data/dispatch ranges
matched the installed PE byte-for-byte. The PE is 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Ranges and their hashes are in `reports/mesh_subset_lod_fields_audit.json`;
this comparison does not establish whole saved-image identity.

| Native address | Calling boundary established from assembly |
| --- | --- |
| `00b941d0` Subset | ECX output-pair context, stack mesh pointer and child-handle pointer; `RET 8`. The pair context is forwarded to the two material child readers but not consumed by them. |
| `00b93710` LODPhases | Stack mesh pointer and child-handle pointer; ECX pair unused; `RET 8`. |
| `00b72710` LODValue setter | ECX mesh, stack float bits; SSE `MOVSS` to mesh+0Ch, `RET 4`. It is not a reader. |
| `00b93590`, `00b935c0` mesh bounds | Second stack argument is child handle; mesh and incoming ECX unused; `RET 8`. |
| `00b93f90` WeightMapNames | Stack mesh and child handle; incoming ECX unused; `RET 8`. |
| `00b73270` LOD append | ECX mesh, stack 16-byte source pointer; `RET 4`. |
| `00b73d50` name append | Adds B0h to ECX mesh and tail-jumps to `004cdc20`, retaining the stack string pointer and `RET 4` contract. |
| `00b93d30`, `00b937a0` material children | First stack argument is material, second is child handle; incoming ECX context unused; `RET 8`. |
| `00b93390` lighting record | ECX reader handle, EDX 68-byte output; plain `RET`. |
| `00b179d0` lighting assignment | ECX material, stack ignored DWORD and 68-byte source pointer; `RET 8`. |
| `00533fa0` section factory | No semantic argument; allocates 60h bytes and invokes `00b857f0`; EAX result, plain `RET`. |
| `00535320` material factory | ECX native counted effect-name string; EAX material, plain `RET`. |

Decompiler signatures omit stack/register inputs in several readers and
misidentify Subset stack cleanup variables. Assembly establishes argument
use, cleanup, x87 stores/discards, and the complete normal returns. No new
incorrect no-return annotation was encountered in these owned ranges.

## Subset wire order and native effects

`00b941d0` first constructs a draw section, then reads the following prefix:

1. Primitive DWORD. Wire values `0,1,2,3,4,5` map to native values
   `1,3,2,5,6,4`; every unsigned value above 5 also maps to 1. The six-entry
   jump table at `00b944c0` was included in the byte comparison.
2. Four more DWORDs, stored unchanged at section+0Ch, +10h, +14h, and +18h.
   The host calls these `range_words`; their downstream draw interpretation
   is not established by this reader alone.
3. One counted effect-name string, using the existing DWORD length and raw
   byte string reader. The native CRT case-insensitive C-string comparison
   checks `soldiers.mshd`; a match resizes the string to 12 bytes and copies
   `soldier.mshd`. Embedded NUL suffixes participate only through the prefix
   comparison; a matching string loses any suffix in this rewrite. Otherwise
   the complete original counted bytes are retained.

The effect name is resolved through global manager `DAT_00f8d394` vtable+48h.
`00535320` allocates 110h bytes and invokes the existing
`BSP_Material_ConstructWithEffect` at `00b18900`, then releases its temporary
effect reference. `BSP_DrawSection_SetMaterial` at `00b864c0` retains the new
material before releasing the old one; Subset immediately drops the factory
temporary reference. Actual effect manager loading is outside this packet.

Subset clears the section's selected vertex streams, then processes named
children in serialized order with CRT case-insensitive C-string comparison:

| Child | Serialized payload and behavior |
| --- | --- |
| `Texture` | Counted texture-name string, then unsigned slot DWORD, then nested children. The native reader first resolves the name through manager vtable+64h with second argument zero, then calls existing `BSP_Material_SetTextureSlot` at `00b189f0`; it releases the temporary texture afterward. |
| `LightingSettings` | Count DWORD, then repeated ignored slot DWORD and 17 float32 values. See below. |
| `BoundingSphere` | Four float32 reads, each immediately discarded with `FSTP ST0`. No section bounds are assigned. |
| `VertexStreamIndex` | One DWORD passed to mesh stream lookup `00b73260`, then append helper `00b85b80`. These operations occur when the child is encountered. |
| Other names | Explicit skip/detach. |

Within Texture, `TextureAddress` reads and discards three DWORDs; unknown
nested children explicitly skip. Texture slot bounds and retain/release
semantics are covered by the existing `docs/MATERIAL_TEXTURE_SLOTS.md`
contract. Parsing records a request and does not itself claim successful
texture resolution or a valid realized slot assignment.

Recognized child handles are released without implicitly seeking an unread
payload tail. Unknown children perform their explicit skip first. This
matches the existing structured-reader convention, including its cursor
consequences for malformed recognized records.

After the child loop, only if section+4Ch (selected-stream count) is zero,
Subset obtains mesh stream zero and appends it. It then rebuilds the section
vertex layout with the mesh, appends the section to mesh+54h/count+58h with a
retained section reference, assigns mesh+60h through `00b85610`, and releases
the temporary section. The typed wire representation does not synthesize
stream zero; the resource/geometry consumer must apply this condition at
the correct point and resolve actual existing stream objects. Selection
lookup, layout reconstruction, and index-buffer assignment remain consumer
contracts; they are not replaced by invented vertex or material defaults.

## LightingSettings

`00b937a0` reads the count, then for every entry reads a slot DWORD,
initializes a 68-byte temporary through `00b17840`, fills it through
`00b93390`, and invokes `00b179d0(material, slot, temporary)`.
The initializer writes float 1 to its first four components, zero to the
next 12, and float 10 to the final component. Constants at `00d7a24c` and
`00ce38b8` were byte-checked. All 17 components are overwritten by complete
float32 reads in this host domain; there is no need to invent defaults for
a missing or truncated lighting record.

`00b93390` performs exactly 17 x87 float32 stores at offsets 0 through 40h
in increasing order. `00b179d0` never reads the slot argument: it sets
material+10Ch to 1 and copies 17 DWORDs to material+38h. Consequently every
entry overwrites the same material record, and the last entry wins.
`MeshLightingRecord` preserves the ignored serialized word and all 17 values
without asserting color/channel meanings from layout alone.

## LOD and mesh bounds

`LODPhases` reads one unsigned count and then, per entry, float32 value0,
float32 value1, unsigned DWORD word0, unsigned DWORD word1. The two float
results are rounded through explicit x87 float32 stores before the 16-byte
record is appended. `00b73270` copies it to mesh+10h+16*mesh[50h], then
increments mesh[50h]. The four records occupy mesh+10h through +4Fh; a fifth
would overwrite the count and later fields. Native code performs no check.
The host parser rejects an append that would make the total exceed four;
this is a stated valid-domain guard, not recovered native error behavior.
Duplicate LODPhases children append, rather than replacing earlier records.

`LODValue` is read by the `00b944e0` dispatch fragment at `00b9454e` through
`00b94570`, which rounds the x87 result through a float32 temporary before
passing it to setter `00b72710`. The setter copies those float bits to
mesh+0Ch. `read_mesh_lod_value_00b944e0_fragment` models that read/assignment
boundary; a native default value is the mesh constructor's responsibility.

Mesh `BoundingSphere` at `00b93590` and `BoundingBox` at `00b935c0` consume
four and six float32 values respectively, issuing `FSTP ST0` for every
result. They do not store bounds, combine coordinates, or call hierarchy
bounds helpers. Their host functions are intentionally consume-only.

## WeightMapNames and host ownership

`00b93f90` has no leading count and no child dispatch. While the node's
remaining payload DWORD is nonzero, it reads a counted string, calls
`00b73d50`, and destroys the temporary string. That adjustor targets the
mesh+B0h vector and calls `004cdc20`, whose eight-byte native string records
are independently allocated/copied with their stored byte lengths. Embedded
NUL bytes survive. Duplicate WeightMapNames fields append in order. The
mesh destructor's string-container cleanup is documented in the separate
mesh object lifetime packet.

Host `std::string` and vectors own all returned values. Subset stores a
sequence of Texture, LightingSettings-entry, and VertexStreamIndex events,
preserving their relative order. Parsing advances input even on failure;
output replacement only after success is an explicit host policy. Inputs
remain attached for the calling field dispatcher to close. No native ABI,
short-read exception behavior, material realization, game validation, or
render equivalence is claimed by these field parsers.

Build and integrated fixture results are recorded by the primary integrator;
the worker's completed evidence is source review and whole-range byte parity.

## Aggregate Mesh wire parser

`parse_mesh_resource_00b944e0` in `src/mesh_resource.cpp` connects these
fields to `mesh_buffers.hpp`. Its owning `MeshResourcePayload` includes the
uninterpreted prefix DWORD, vertex streams, optional latest index payload,
subsets, LOD value/phases, weight-map names, unknown tags, and an encounter
trace of original counted child tags. Its native dispatch anchor is
`00b944e0` (ECX output pair, stack node-handle pointer, RET4). The native
constructor is `00b73d70`; `00b73b60` is an allocation-context entry.

The native constructor initializes mesh+0Ch to float 1.0, index pointer+60h
to zero, and stream/subset/LOD/name counts to zero. These are the exposed
host aggregate defaults. No unspecified constructor bytes become invented
fields or default geometry. The unused native LOD array cells are omitted;
the host phase vector contains only serialized entries.

All nine known child names dispatch as encountered. VertexStream, Subset,
LODPhases, and WeightMapNames append; LODValue and Indices replace their
current values. CompressedVertexFormatData modifies the last already
appended stream and rejects absence of a preceding stream in the host
domain. Repeated compression replaces that stream's retained byte vector.
The aggregate does not postpone compression until future streams exist.
Ordinary recognized fields close without seeking an unread tail. The
buffer helper's special `rope.mvfm` branch explicitly skips/detaches, after
which aggregate close is harmless. Unknown fields retain their tag for
inspection and explicitly skip. Native bounds are consumed/discarded.

This is a decoded wire snapshot. The encounter trace does not preserve
overwritten index/compression objects for replaying native lifetime changes,
and the aggregate does not resolve material requests or construct retained
draw-section stream/index bindings. Output replacement after whole-parser
success and container/size rejection are host policies. The input node
remains attached for its enclosing resource parser to close.

The full `00b944e0` 557-byte body and `00b73d70` 179-byte constructor were
recompared to the installed PE for this aggregation and match. Their earlier
construction/dispatch evidence remains in
`docs/RESOURCE_MESH_PARSER_BOUNDARY.md`. Buffer schemas, resolver boundaries,
and unsupported native inputs are in `docs/MESH_VERTEX_INDEX_PAYLOADS.md`.

## GroupParams dependency audit

Resource-manager registration binds `GroupParams` to parser `00b8eb50`.
Its 110-byte body allocates a 0Ch-byte item, calls the existing reference-base
constructor `00b868b0`, writes vtable `00d634b0`, then calls its +20h slot
with the input node handle. Incoming parser ECX is unused; the handle is
the sole stack argument, EAX returns the item, and the parser uses RET4.
As in other native item factories, allocation-null handling does not prevent
the subsequent virtual dereference.

The +20h vtable DWORD at `00d634d0` points to `00b8e580`. That complete
30-byte method takes ECX item and one stack node-handle pointer, reads one
float, uses `FSTP float [item+8]`, and calls
`BSP_StructuredNode_SkipAndDetach` before RET4. It has no child-name dispatch
and gives the scalar no recovered semantic meaning. Any remaining payload
is explicitly skipped even when it is not empty.

`read_group_params_00b8e580_fragment` in `structured_resource.cpp` models
the float read and explicit skip/detach without constructing a native item.
It commits output only after both operations succeed, an explicit host
failure policy. The parser body, virtual method, and nine-DWORD vtable prefix
were compared against the installed PE and match. No deeper unexamined
field-reader dependency remains on this GroupParams parse path; item factory
and reference-counting behavior remain native-runtime boundaries.
