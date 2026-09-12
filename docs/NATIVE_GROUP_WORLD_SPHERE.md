# Native Group world sphere aggregation

Addresses: 00B8E980, 00B8E9F0, 00B8EBE0, 00B8F100.

`aggregate_native_group_world_sphere_00b8ebe0` computes the lazy dynamic sphere
in the existing `NativeGroupOwner` allocation. It consumes the same attachment
descriptor at +178/+17C and writes the same node sphere at +13C. It does not walk
the separate +34 hierarchy or create a second group, child array or sphere.

| Routine and inclusive body | Original ABI | Coverage |
|---|---|---|
| B8E980..B8E9E6 | ECX output XYZ, EDX source XYZ, stack length output; EAX output, RET4 | Complete normalization/length helper |
| B8E9F0..B8EB1D | ECX destination sphere, stack source sphere; EAX destination, RET4 | Complete sphere merge |
| B8EBE0..B8EC7A | ECX Group, RET | Complete dynamic aggregation over the established owner projection |
| B8F100..B8F18B | ECX Group; EAX Group+13C, RET | Full contract analyzed; final getter composition belongs to the integrating PointLight provider packet |

All live wrappers verified `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. All four entries already had complete containing
bodies; no missing-code definition or Ghidra mutation was required. Three new
descriptive names are hypotheses, not recovered symbols. The existing
`BSP_Group_GetWorldSphere` name is retained. Annotation application and final
combined build remain the integrator's work.

The constructor B8F5E0 initializes the actual +178/+17C/+180 descriptor, sets
+175 to 1 and +138 bit 2, and sets the initial local sphere radius from CE4970.
The existing registration B8F460 and actual-array append store the same
`CameraTransform` companions in that physical descriptor. This established
C++ projection uses companion pointers for native raw-node pointers; it is
**not** a drop-in native pointer ABI. Each reached companion resolves through
the existing scene runtime to its actual raw node identity before +138 is read.
The diagnostic `GeneratedModelAttachmentLinks.models` vector remains unused.

Aggregation first stores positive-zero XYZ and the live CE4970 word into +13C.
Installed CE4970 is `0x501502F9`, approximately 1e10; it is not zero, -1 or an
inferred empty-radius policy. It then captures count and begin, derives the end
once, and reads each current element from that captured allocation. Before the
first seed only, child +138 mask 3 gates the current +48 call. After the first
seed, **every** remaining child invokes current +48 and merges, even if its mask
3 is zero. The seed uses ordered FLD/FSTP float copies, not integer copies.
Completion ORs the current Group +138 with 0x30. It does not retain a child,
change count, notify an enclosing owner or refresh the Group transform.

Current-child +48 remains a required dispatch service. Verified table
D634F8+48 contains B8F100, and Model table D62DE8+48 contains B6E8C0. No new
type-dispatch table is supplied by this module. The parent adapter binds the
same PointLight population runtime and current sphere service. The callback
can reenter and change count, cache flags or later elements; it must keep the
captured allocation and reached owners alive. Concurrent changes and malformed
array extents are outside the valid C++ adapter domain. Checks do not clear
native state or convert an unresolved callback into a default sphere.

B8F100 tests +175 before choosing static versus dynamic work. A nonzero byte
and clear mask 0x30 call B8EBE0 at B8F118, then OR that same mask again. The
static branch refreshes the actual world transform with B6DB70 if needed,
uses existing affine sphere 007C1180, copies the
result to +13C and ORs 0x30. Both cached branches return the same +13C pointer.
The static branch retains the existing affine kernel's finite-input/CW007F or
CW027F boundary; that limitation is not imposed on the new merge helpers.
The other direct aggregation caller B8F0F0 writes +175=1 before tail-jumping to
B8EBE0. Its wrapper is caller evidence, not a new implementation in this packet.

The merge helper keeps the original x87 stack, binary32 spills, forward source
reloads and branch flags. It computes spilled coordinate differences, then
B8E980 calls existing `camera_vector_length_00419440` with actual CRT access.
The length is stored to its output before source components are reloaded;
aliasing the length/output/source therefore matters. Positive ordered length
divides each source component; zero, negative and unordered length write three
positive zeros. The returned length remains the actual spilled value.

Merge containment uses two FCOMI branches. Unordered comparisons take the
expansion path. If the source contains the destination, only the first source
component is cached from before length; later source components/radius are
reloaded after earlier stores. Expansion reads **double** D7A280 (installed 0.5),
spills the new radius, spills its delta, spills each directional product, then
adds those stored products to the original center. Replacing the double load,
changing operation association, or calculating directly in SSE changes native
results. The new C++ arguments borrow live CRT and D7A280 pointers; the numeric
access adds no native state and the functions do not change the control word.

One ignored local probe compiles the actual source with MSVC Win32 `/W4 /WX
/fp:strict` and `/MANIFEST:EMBED`. It extracts original B8E980..B8EB1D machine
code from the installed PE (414 bytes; SHA256 in the report), relocates the
internal call, live half pointer and existing length dependency, and compares
against the reconstructed functions. **93,600** comparisons passed across all
12 x87 precision/rounding combinations, overlapping source/output/length
storage, randomized words, signed zeros, subnormals, infinities and NaNs.
Result bytes, x87 exception flags, callback counts and preserved control words
agree. Both sides share the existing reconstructed length/CRT dependency, so
this validates the new helpers, not independent original-CRT equivalence or
unmasked exception dispatch. MXCSR and x87 condition-code parity were not part
of the comparison.

The same focused executable constructs real Group owners from the existing
18Ch pool, registers actual attachments through B8F460 and verifies the empty
sentinel, first qualifying seed, ungated trailing merge, captured end versus
current element mutation, unchanged references, empty diagnostic vectors and
actual owner/pool teardown. It does not prove the final PointLight getter
composition, rendering, gameplay, or a native ABI replacement. No permanent
test suite or executable runtime claim was added.
