# Actual shader sampler and state-list lifetime

Addresses: 00b620f0 00b621a0 00b621b0 00b62280 00b623c0 00b623d0 00b62500 00b62590 00b62630 00b62760 00cd7cc0 00ce0d50 00b40cf0 00b56de0 00b56ea0 00b56fc0 00b57b50

This packet supplies the concrete ownership dependency for the existing shader
sampler parser: actual2Ch sampler records, their two pooled0Ch state-list
headers, and the distinct pool at0108FEE4. It reconstructs sixteen full routines
and the33-byte fresh-sampler initialization fragment inB57B50. The full Lua
reader and its publication into descriptorC4 remain separate work. Names are
descriptive hypotheses, not recovered symbols.

## Actual pool

`NativeShaderStateListPool` uses the existing shape-dependent implementation in
`native_material_pools.cpp`, with separately bound actual38h storage and the same
allocator-list domain. This reuse follows verified instruction behavior, not
an assumption that neighboring pool instances are interchangeable.

| Field or region | Recovered layout |
| --- | --- |
| Global / vtable / virtual0 | 0108FEE4 / D62B9C / B62590 |
| Actual pool | allocator-list element00, real24-byte critical section0C, recursion24, table28, count2C, capacity30, first-free34 |
| Slab | 244h bytes,32 slots of10h |
| Slot | 0Ch payload followed by DWORD current slab ID at0C |
| Slab metadata |32 descending unsigned-short free indices at200, freecount at240, final2 bytes untouched |

Construction prepends the actual allocator list, initializes the real critical
section, zeroes table fields, sets first-free=-1, and publishes capacity32 before
allocating128 bytes. Allocation takes the actual lock and increments recursion;
it creates a slab when needed, grows a full table to2*old+2, then consumes the
last free index. These allocating paths have no automatic unwind unlock or
rollback. Returning a slot reads its current+0C slab ID under the lock, computes
its16-byte index with signed SAR4, appends the index and lowers first-free.
The shared division is equal for valid aligned slots in their actual slab.

Trim does not lock. It frees empty slabs, copies the last table entry into the
removed position, decrements count, rewrites all32 moved IDs and retries the
same position. It retains table capacity and recomputes first-free. Destruction
frees slabs and the table without destroying payloads, drains positive lock
recursion, deletes the critical section and unlinks the actual allocator-list
element. Pointer/count/capacity metadata remains stale.

| Full routine | Bytes | Original ABI / role |
| --- | ---: | --- |
| B620F0 |64| ECX fresh244h slab, stack ID, EAX same, RET4 |
| B621B0 |14| ECX pool+28 header, RET; free nonnull table pointer only |
| B623D0 |211| ECX actual38h, EAX same, RET; pool constructor |
| B62500 |138| ECX actual38h, RET; pool destructor |
| B62590 |160| ECX pool, RET; trim |
| B62630 |304| ECX pool, EAX raw slot, RET; allocate |
| B62280 |85| ECX pool, stack slot, RET4; return |
| B623C0 |12| ECX raw slot, RET; fixed-global return wrapper |
| B62760 |10| no args, EAX raw slot, RET; fixed-global allocation wrapper |
| CD7CC0 |22| no args, EAX CRT atexit result, RET; construct then registerCE0D50 |
| CE0D50 |10| no args, RET; fixed-global shutdown |

The static wrappers borrow a persistent canonical companion and use real
`std::atexit`. There is no private exit registry or additional initialization
guard. The storage, list and companion must survive shutdown, with payloads
already destroyed. B62760's previous static-destructor tag and CE0D50's
static-initializer tag were incorrect; these wrappers allocate and shut down.

## State lists and sampler

B621A0 is the full13-byte constructor of a0Ch array header; it writes only data,
count and capacity zero, preserving the physical slot's slab ID. B40CF0 is the
full104-byte reserve routine for8-byte state/value rows. It clamps signed
requests to10, compares signed capacity, allocates, copies ordered DWORDs with
current count and data reads, frees the old buffer, then publishes pointer and
capacity. Row allocation uses DWORD byte-size arithmetic. Valid readable
extents and representable allocation sizes remain caller contracts.

B56DE0 is a77-byte pointer-holder destructor. It captures the pointed header,
reserves zero if capacity is negative, clears count and frees row storage,
returns that same header to0108FEE4, and only then clears the holder. The returned
header's data pointer and capacity remain stale. Null holders are untouched.
The16-byte continuation after B56E15 was previously hidden by a false no-return
annotation; it contains the real pool return and holder clear.

| Actual2Ch sampler field | Meaning / initialization fragment |
| --- | --- |
|00| D621F4; virtual0 B56FC0 |
|04/08| actual name string, zero |
|0C| vertex-sampler byte, zero |
|0D..0F| padding, untouched |
|10| type, untouched until later parser assignment |
|14| texture-source selector, zero |
|18/1C| actual source-name string, zero |
|20| index, zero |
|24/28| two pooled state-list header pointers, zero |

Only initialization storesB57B80..B57BA0 are implemented from the parser in
this packet. That fragment has EAX=fresh2Ch and EBX=0; it writes37 bytes and
leaves7 bytes unchanged. There is no intrusive reference counter. The full
B57B50 reader additionally reads Lua fields, assigns type, allocates both lists
even when state tables are absent, and parses rows; those operations are not
claimed by this fragment.

B56EA0 is the full146-byte sampler destructor: publish D621F4, destroy holders
24 then28, release string18 then04. B56FC0 is the full30-byte scalar destructor:
ECX sampler, stack flags, EAX old pointer, RET4; free the2Ch allocation iff bit0
is set and member destruction returns. Strings keep their stale headers.
`NativeShaderSamplerCallableBinding` supplies an explicit external callable
virtual0 table on this same actual allocation. It restores the numeric profile
and detaches before invoking scalar destruction, using the caller's canonical
pool and string lifetime. Actual descriptorC4 can directly delete the sampler.
The binding adds no reference count or private owner registry.

## Unwind and evidence

Pool constructor FuncInfoDFA328/mapDFA310 has three states: CC14C0 unlinks the
allocator-list element through403970, CC14C8 destroys the section at+0C through
402F70, CC14D3 frees the table header at+28 throughB621B0. Sampler destructor
FuncInfoDF9018/mapDF9008 has only two states: CC0440 destroys name04 and CC044B
source-name18. A failure during state-list cleanup therefore unwinds strings;
it does not resume cleanup of later list holders or free the raw sampler.
Original exception delivery is not runtime-verified here.

Twenty-three current Ghidra byte captures match the installed PE: sixteen
complete routines, the initialization fragment, two vtables and two unwind
code/map groups. Four previously undefined functions were created from verified
extents: B62280, B623C0, CD7CC0 and B621A0. Nine call-return gaps in eight
functions were repaired under the write lock; the two alignment gaps in trim
were left alone. Every reconstructed full function has its complete stored
body. Prior comments were preserved, annotations read back, exports refreshed
and the project saved. Allocator and CRT/library names are unchanged.

The strict MSVC Win32 build and both existing CTests pass. One ignored focused
fixture executes all sixteen full original routines plus the33-byte sampler
initialization block. It compares eight normalized56-byte pool/metadata snapshots
across1026 allocations and33 slabs, tests growth32 to66, all moved IDs, LIFO
reuse, empty trimming and recursion2 shutdown. A580-byte slab preimage comparison
checks untouched payload/padding. Native CRT registration returns the fixture's
observed result7 and invokes the captured shutdown; the rebuilt wrappers execute
real process atexit and verify cleanup afterward.

Ordinary and negative-capacity sampler cases compare44-byte initialization and
destruction snapshots plus actual pool metadata and returned header fields.
They prove both list slots return before source-name/name release, and that
the last returned slot is reusable inside a string-release callback. An actual
descriptor deletes an actual sampler through the production callable binding,
returning both real header slots. Pointer identities and OS critical-section
bytes are normalized; actual string storage uses the established pool. No new
permanent test was added. Frozen source/build/evidence hashes are recorded in
`reports/native_shader_sampler_owner.json`.

## Follow-up packets

Implement the full B57B50 sampler reader and B41830 sampler-table append over
these actual owners; recover B579B0/B567B0 state-registry row publication and
the actual state-definition manager. Field-table allocationB573F0/B419B0 and
full descriptor readerB43B00 also remain. Existing semantic shader Lua parsing
does not yet establish these concrete allocation/publication paths. This packet
is reconstructed, build-tested and fixture-tested; binary ABI, shader rendering
and gameplay remain unvalidated.
