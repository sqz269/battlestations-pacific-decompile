# Actual shader descriptor lifetime

Addresses: 00b43700 00b458a0 00b46930

The effect loader allocates a `0x110`-byte descriptor at `00b45ee0` and publishes
it in actual effect `+C4`. This packet reconstructs its complete constructor,
member destructor and scalar deleting destructor in
`native_shader_descriptor_owner.hpp/.cpp`. Descriptive names are hypotheses,
not recovered symbols. The parser and program builder remain separate work.

| Routine | Complete verified extent | Original ABI |
| --- | --- | --- |
| `00b43700` constructor | `00b43700..00b437e8`, 233 bytes | ECX fresh110h, EAX same, RET |
| `00b458a0` member destructor | `00b458a0..00b45df2`, 1363 bytes | ECX descriptor, RET |
| `00b46930` scalar deletion | `00b46930..00b4694d`, 30 bytes | ECX descriptor, stack flags, EAX old address, RET4 |

## Storage and initialization

The vtable at `00d61a44` has scalar deletion `00b46930` in slot zero. There is
no intrusive reference count: descriptor `+04` is parser-written PipeID and
`+08` is Priority. Both survive construction. The actual constructor initializes
22 eight-byte string headers: `0C,28,34,3C`, fourteen starting at `48`, then
`E8,F0,F8,100`. It zeroes four12-byte array headers at `B8,C4,D0,DC`.
The remaining bytes survive, including `14..27`, word30, flags44..47, RTCount108
and10C..10F. Field roles not needed for lifetime remain opaque in the C++ type.
The original array constructor calls CRT `BF7CD1`, stride8/count14, with actual
empty-string constructor415270 and destructor41DD20; these are not new library
implementations. C++ actual string construction has no throwing operation.

## Direct ownership and member cleanup

The destructor first publishes D61A44, then walks these three arrays in order:

| Header | Entry ownership | Traversal |
| --- | --- | --- |
| C4 | Direct current callable virtual0 with flags1 | Capture current slot address, call, clear that captured slot |
| D0 | Heap allocation starting with actual8h string | Release string, free object, clear captured slot |
| DC | Same string-first ownership | Same sequence |

Each loop uses an unsigned index and reloads the current count after every
iteration; it reloads the data pointer for the next entry. A callback that
replaces the table does not redirect the pending captured-slot clear. No
reference-count operation is inserted. Null entries skip calls and stores.
The sizes and payload meanings of the pointed objects remain external contracts.

Member destruction then runs in reverse order: strings100/F8/F0/E8, headers
DC/D0/C4/B8, fourteen mode names in reverse, then3C/34/28/0C. Each nonnull string
returns its captured buffer with length+1 to the same supplied actual pool;
its header is left stale. Header destruction sets count0, frees the array, and
leaves the pointer and capacity stale. B8 entries are plain8-byte rows: there is
no string or child destruction within them.

A negative capacity triggers reserve before count clearing. DC/D0 reserve ten
4-byte entries, C4 six4-byte entries, B8 ten8-byte rows. Allocation precedes a
copy with current count/data reloads, followed by old-buffer free, then pointer
and capacity publication. This allocation can therefore happen during teardown.
The supported host extent is nonnegative count with readable backing storage;
the native pointer-header resize also contains malformed negative-count writes
before the current array pointer. That invalid-memory domain is not exposed by this API.
There is no additional allocation rollback. Scalar deletion frees actual110h
storage iff flags bit0 is set; flags2 does not free it.

## Unwinding and callable host composition

Constructor FuncInfoDF7AD0 points to mapDF7AB0: four states clean the strings at
0C/28/34/3C through CBF130/CBF13B/CBF146/CBF151. Destructor FuncInfoDF7EA4 points
to mapDF7EC8: states12 through0 have previous-state links11 through-1 and member
thunks CBF4C0..CBF559. The map covers the reverse member sequence above, not the
three direct child loops. On a throwing child deletion, remaining child entries
are not visited; header and string members still unwind. On failure during a
member cleanup, only earlier members unwind. The object allocation itself is
not freed when member destruction throws.

The nine dependent helper bodies were analyzed and captured, not added as
independently reconstructed functions: B42240, B41F40, B34BF0, B34C30,
B40CF0, B34620, B34680, B34760, B34A00. Their paths provide the same header and
array behavior during unwind. Existing actual string-pool and shared heap
implementations supply the storage lifetimes.

`NativeShaderDescriptorCallableBinding` is explicit host metadata. It temporarily
installs a callable virtual0 table on the same110h descriptor, carrying only the
binding and its string lifetime. It adds no counter or private owner registry.
Its callback restores D61A44, detaches metadata and invokes the reconstructed
scalar destructor. Actual effect B41B10 can therefore directly delete its C4
descriptor. The binding must live until this call or explicit detach; neither
binding destruction nor detach implicitly destroys the descriptor. After a
throwing deletion the raw object remains the caller's responsibility.

## Evidence and validation

Eighteen current Ghidra byte captures match the installed PE: three full packet
bodies, the vtable, the16-byte empty-string constructor, two unwind code/map
pairs and nine dependent helpers. Four false call-return gaps were repaired
under the write lock in B458A0/B46930. The unrelated10-byte alignment gap was
left alone. B458A0's **stored** Ghidra body still ends at B45B14; the verified
tail B45B15..B45DF2 is734 additional bytes. Reconstruction and the native fixture
use the full1363-byte capture. No Java body extension was attempted in this
packet. Saved names/comments explicitly retain this evidence boundary.

The strict MSVC Win32 build and both existing CTests pass. One ignored focused
fixture executes all three full original routines with ordinary and negative
capacities, comparing normalized272-byte constructor and destructor snapshots
and27 ordered callbacks per run. It verifies scalar/padding preimages, live
count growth, table replacement, captured-slot clearing, reverse string cleanup,
stale headers and flags1/2. String calls use the existing actual pool. CRT array
iteration is a normal-path library adapter over the established actual string
operations; original SEH delivery is not executed. The fixture also exercises
existing actual effect B41B10 through the production descriptor binding, and one
controlled rebuilt child exception through the thirteen-member cleanup path.
No permanent test was added. Source, build and evidence hashes are pinned in
`reports/native_shader_descriptor_owner.json`.

## Follow-up packets

Recover actual descriptor parserB43B00 and full effect loaderB45EE0, including
unresolved string/name helper4BCB80, mode program builderB3C3A0 and actual shader
payload construction. The existing semantic parser/builder fragments do not yet
establish those actual ownership paths. This packet is reconstructed, build-tested
and fixture-tested; it is not ABI-compatible or game-validated.
