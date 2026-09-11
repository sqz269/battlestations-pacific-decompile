# Native mesh owner storage and lifetime

`NativeMeshStorage` is the actual BCh-byte payload of a C0h mesh-pool slot,
not `MeshResource` or `GeneratedInstanceGeometry`. `NativeMeshReference` borrows
the atomic at that exact storage's +04 and dispatches destruction through its
current D62D60 profile. A native model's +180 can therefore hold the raw mesh
identity whose +54 is the draw-section pointer array used by native consumers.
No extra count, mesh graph, or adapter pointer occupies a native owner field.

The storage and direct C++ bodies are in `include/bsp/native_mesh_owner.hpp`
and `src/native_mesh_owner.cpp`. The separate `NativeMeshPool` packet supplies
the concrete canonical 0108FFF8 pool. Physical deletion calls that same pool's
B72DA0 return method; it never individually frees the BCh mesh object. The
mesh owner header consumes the concrete pool header rather than declaring a
second allocator interface or implementing a fallback heap pool.

## Evidence and layout

`reports/native_mesh_owner.json` records 24 live Ghidra byte spans matched to
the installed PE, including the native vtable, constants, EH table and unwind
thunks. Each `bsp.py ghidra bytes` call verified
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The installed binary at `I:/SteamLibrary/steamapps/common/Battlestations Pacific/`
has SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The executable and Ghidra analysis were read only in this packet.

| Offset | Actual storage |
| --- | --- |
| 00, 04 | Current native vtable token and actual intrusive atomic |
| 08, 0C | Observed word and LOD bits |
| 10..4F | Four 10h-byte phase records; constructor writes only the first two DWORDs of each |
| 50 | LOD phase count |
| 54, 58, 5C | Draw-section pointer data, signed count, signed capacity |
| 60 | Optional actual index-stream owner |
| 64..7B, 7C | Six inline stream pointers and signed active count |
| 80, 84..93, 94 | Constructor's two byte flags and four zero DWORDs |
| 81..83, 95..AF | Uninterpreted bytes preserved by construction |
| B0, B4, B8 | Eight-byte string-header data, count, capacity |
| BC | Pool-chunk index, outside the object and preserved |

B73D70 reads CE4ADC/CE4970 before its first writes, publishes CEB130,
sets +04=1, then publishes D62D60. It initializes only the observed fields;
all unused stream slots and unknown bytes retain the allocation preimage.
The D7A24C LOD constant is read at its later store point. The constructor
allocates nothing and has no throwing child calls or constructor unwind.
Its final RET is at B73E22, length one, end-exclusive B73E23.

## Owners, array growth, and callback ordering

B73B70 assigns index+60. B73BB0 first extends the stream high-water count,
writing null to every newly exposed slot, then assigns +64+4*index. Both
methods return immediately for equal identities; otherwise they publish the
incoming pointer, increment its actual+04 if nonnull, and decrement the
captured old owner's actual+04 if nonnull. Canonical lookup happens only on
the zero transition, through `release_native_render_actual_owner`. A missing
or mismatched companion is an error. The methods have no rollback on error.

B73C60 appends a nonnull actual section. When count equals capacity it grows
to max(2*capacity,4), stores the section, increments count, then increments
section+04. B72E10 reserves at least four slots and grows only; B73840 zeroes
newly exposed slots and decreases count without destroying pointed objects.
This array uses the existing ordinary BF55BE/BF6989 allocation boundary,
separate from pooled mesh-object storage. Spare allocation cells are not
initialized by reserve.

B73C10 releases each nonnull-assumed section using a signed loop, reloading
data and count after each callback, then resizes count to zero. It neither
frees backing nor clears dangling pointer cells. B73CB0 resizes to zero and
frees backing without resetting the data pointer or capacity.

B73E60 performs this full sequence:

1. Publish D62D60; capture, release and then clear optional index+60.
2. Capture the stream end from +7C after the index callback. Visit each slot
   up to that captured end, loading the current pointer and releasing it when
   nonnull. Do not clear stream slots or count. A stream callback may change
   later cells or +7C without moving the captured end.
3. Run B73C10, which uses the live section count instead of a captured end.
4. Destroy weight names in reverse order, decrementing count before each
   string return, then ordinary-free the current B0 backing pointer.
5. Resize the current section vector to zero again and free its current
   backing pointer, without releasing section references again.
6. Publish intermediate D5C104 and execute BD30F0's CEB130 base-table store.

String returns use the existing explicit `NativeStringStorage` boundary with
captured data and length+1, preserving each old element header. The no-grow
count-zero fragment of 427110 is reconstructed; arbitrary string-vector
growth through 426520 remains outside this module. Full 427880 is available
in its valid-header domain. Native singleton acquisition and pool policies
remain represented by the supplied storage implementation.

The destructor's EH handler is CC1BF1, with FuncInfo DFABAC and unwind map
DFAB94: state2 -> CC1BE3 / strings at mesh+B0, state1 -> CC1BD8 / section
backing at mesh+54, state0 -> CC1BD0 / AA6E10 intermediate/base tables. Normal
code lowers the state before each member destructor. The C++ cleanup guard
reproduces this order on lookup failure and does not retry index, stream or
section reference releases. The failing index remains uncleared; unvisited
streams and sections are not released by the unwind member destructors.

B74280 calls the complete body and returns the physical slot through
canonical 0108FFF8 only when flags&1. It returns the original address even
after return. `NativeMeshReference` requires current D62D60 slots
BD30E0/B74280 and dispatches this scalar deletion with flag1; its retirement
callback runs afterward with no subsequent storage/companion access. Binding
one canonical companion and preserving its dependencies are caller duties.

## Saved-analysis continuation handoff

These continuations were verified from live bytes and the installed PE, not
invented to satisfy compilation. This worker made no Ghidra writes.

| Call site | Excluded continuation | Final instruction / end-exclusive |
| --- | --- | --- |
| B72E5C -> BF6989 | B72E61..B72E6A is a nine-byte gap in the saved listing; it restores ESP, publishes data/capacity, pops EBX | B72E6C RET4, length3 / B72E6F |
| B73CBD -> BF6989 | Listing stops at the call; bytes B73CC2..B73CC7 restore ESP, pop ESI and return | B73CC6 RET, length1 / B73CC7 |
| 42788D -> BF6989 | Listing stops at the call; bytes 427892..427897 restore ESP, pop ESI and return | 427896 RET, length1 / 427897 |

`bsp.py ghidra flow` reports the B72E61 gap. For the other two helpers it
reports no interior gap because their saved function bodies end at the calls;
their excluded tail instructions still require the integrator's repair.
B73E60 itself currently decompiles through its complete B73F4D RET,
length one, end-exclusive B73F4E.

## Validation and limits

`scripts/build.ps1` passed with the concrete pool header overlay. After seed
verification, both existing CTests passed: reconstructed math and native math
differential. One ignored `local/native_mesh_probe.cpp` was compiled as a
manifested MSVC Win32 executable and passed:

- All 256 byte-fill preimages produce identical C0h storage after the copied
  installed constructor and reconstructed constructor, including untouched
  bytes and the trailing pool word.
- Installed index/stream assignment, five-section growth through capacities
  four/eight, and complete normal destruction agree in fields and actual
  reference counts. Native code uses relocated verified spans with observed
  Interlocked and ordinary allocation/free boundaries.
- A native callback scenario agrees on index clear-after-release, captured
  stream end, reloaded later stream cells, live section count, skipped owners,
  resulting native bytes, and exact zero-callback order.
- A host lookup-failure fixture checks reverse string returns, member backing
  cleanup, final base table, preserved failing index, and no repeated or
  unvisited reference releases. This is not a native exception-runtime test.
- The concrete pool worker's source overlay supplies a real C0h slot. A
  canonical mesh companion borrows its +04, destroys and returns it on zero;
  the next allocation from that same concrete pool reuses the exact address.

The source overlay comes from packet `orch2_native_mesh_pool_m`; it is not
duplicated in this commit. Root integration must merge that packet alongside
this owner. No GUI source was changed. Draw-section construction/material
retention, renderer services, complete mesh field loading, and binding GUI
geometry into a native model remain their own integration work. These are
new C++ interfaces with original ABI evidence, not binary replacement entry
points or game/render validation. The valid domain requires nonnegative
vector headers, valid spans, indices below six, live aligned actual owners,
and representable allocation sizes/capacity arithmetic; corrupt-header and
native allocation-failure behavior are not broadened into recovery promises.
