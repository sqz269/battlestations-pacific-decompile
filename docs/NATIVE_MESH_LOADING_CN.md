# Native mesh constructor and aggregate loading (CN)

Addresses: 00b944e0, 00b94710, 00b72b40; compiler supports 00cc2eb0,
00cc2eb8, 00cc2ed0, 00cc2ed8.

CN connects the complete native mesh constructor and ordered field dispatcher to
the recovered native buffers, metadata, subset/material caches, layouts and
instance generators. These source interfaces extend the existing registered
`native_mesh_subset_loading` module. No CMake change is needed.

## Dispatcher and constructor

B944E0 reads the prefix DWORD into the caller's actual pair+4. It processes fields
in their serialized order, reloading the pair's current mesh for each selected
child. LODValue, LODPhases, Subset and Indices use nullable C-string comparisons;
VertexStream, CompressedVertexFormatData, BoundingSphere, BoundingBox and
WeightMapNames use the actual counted node+10 header through425850. Unknown
children skip/detach. The first four comparisons must not silently become counted
comparisons or the later five become ordinary C-string comparisons.

LODValue captures the mesh before reading, preserves the native x87 FSTP32,
FLD32, FSTP32 sequence and passes the resulting bits to the B72710 MOVSS setter.
The other fields invoke their complete native source bodies. Each subset, buffer
or metadata invocation gets a distinct persistent acquired frame. The DFC6B4 FH3
map owns only the current completed child; normal cleanup disarms it before
BE9ED0. Neither this dispatcher nor its caller rolls back completed child effects.

B94710 allocates from the actual mesh pool, runs B73D70 for nonnull storage, and
publishes actual pair[0]. Its sole native exception state returns only the raw
constructor slot through B72F70; it does not destroy a completed mesh. The native
state-1 transition at B94756 precedes parsing. Added host canonical admission is
placed after that transition, so a bind failure preserves the completed creator,
pair publication, stable metadata and any completed unbound companion. The new
`GuiNativeGeometryOwners::create_native_mesh_and_publish` uses the same concrete
pool and constants as its geometry domain. It adds no native reference count or
automatic creator release.

After B944E0, the constructor calls B72B40 and enters the original empty unsigned
loop when count is nonzero. Every iteration reloads the current pair mesh before
incrementing its local index and sampling mesh+58 again. It has no per-element
action, count snapshot, timeout or synthetic completion. The new volatile B72B40
interface preserves these loads; the old GUI convenience getter remains intact.
The return value is a final read of current pair[0].

## Evidence and validation

The two new parent bodies cover557 and131 bytes; B72B40 is the already-understood
four-byte leaf. Saved Ghidra and the installed executable match across692 ordinary
bytes and36 compiler-support bytes. Every instruction owner and all34 direct
transfers were checked. The two missing dispatch handlers were defined/saved under
the write lock without clearing bytes or changing no-return flags. Seven existing
comments are preserved and extended. Descriptive names remain reconstruction
hypotheses, and the report records the original ABI separately from the C++ API.

The tracked Win32 build and both existing CTests pass. One ignored focused probe
executes the full constructor/dispatcher with all nine fields and an unknown tag.
It uses an actual retained-memory reader, native string/material/texture pools,
populated actual caches, D3D9 vertex/index storage, two full subsets with explicit
and default stream0 selection, and generic instance-generator layout readback.
The returned mesh preserves the prefix, LOD/phases, compressed metadata and weight
names, and the final loop performs two iterations over its actual two sections.

A transactional mesh-bind exception proves that pair[0] and the one creator
reference survive after B94756 while pair[1] remains untouched. A separate read
exception on the second outer sphere scalar occurs after both subsets complete:
only the current child is cleaned; the mesh, streams, materials and sections stay
published until explicit fixture retirement. All31 successfully registered
companions retire; the rejected bind adds no registry entry. Caches, hardware tree
and allocator list empty, raw slots return, and device/API final counts are zero.

The cached effect fixture is explicit, as in CM: B407A0 operates on a zero pass
preimage, and B43700 constructs a descriptor whose generic selection name is fixture
input. This does not substitute for B46950/B43B00. Unused cold compiler/state-cache
providers throw if reached. The probe proves this populated-cache composition,
not cold shader loading, original whole-parent execution, native ABI/FH3/SEH
equivalence, concurrent count mutation, or gameplay/render parity.

Frozen sources, libraries, executable/build artifacts and native audits are under
ignored `local/native_mesh_loading_cn/registered/`, with receipts in the tracked
integration report. The default CMake source registration is unchanged.

## Follow-up packets

Live Ghidra callers identify three next wrappers: B947A0 (169 bytes), B94850 and
B94900 (175 bytes each). B947A0's existing evidence describes a10h resource item
with D63738, mesh+8 and prefix+C, retained assignment and local-reference release.
Audit and implement all complete wrapper bodies and connect them to actual native
resource dispatch. The existing `structured_resource_registry` path still models
owning wire values and is not proof of this actual resource-item composition.

Cold material/resource loading still requires the actual descriptor/compiler and
state/pass cache children. Work is published on the agent branch; moving `main`
deltas remain unreviewed.
