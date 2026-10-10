# CC12 mesh field application binding Source

`GameNativeRendererApplication::borrow_mesh_field_services()` now returns stable,
noncopyable metadata over the original VFS hierarchy reader and the renderer's
existing geometry, texture-cache and owner domains. It creates no native mesh
operation. This implements the six-point contract accepted in
`CC12_MESH_LOADING_APPLICATION_BINDING_READINESS.md`, on baseline
`2c8f72791db2df19b9c219c5b826f2b45ce09e26`.

The public view contains references to `NativeMeshBufferReadContext`,
`NativeMeshMetadataReadContext`, `NativeMeshTextureFieldContext`, and const
`NativeMeshLightingConstants`. Copy and assignment are deleted. A private
`MeshFieldBinding` owns those metadata contexts and their stable view; it retains
the original hierarchy, read-only mapper, adopted stream dispatcher, empty string
storage, and qualified profile pointers. Its constructor only stores metadata.

Every borrow first requires the renderer's ready phase and a false existing
`requires_process_retention()` result. Only then does it call the original
`GameVfsHost::borrow_hierarchy_services()`. It also obtains fresh raw VFS services
and native owner references, checking the original raw-string pool/publication,
small-return flag, singleton manager publication, VFS publication and all raw VFS
service identities. The existing GUI renderer domain validator and explicit
geometry/cache/texture owner, physical lifetime, renderer publication and
synchronization comparisons establish the renderer side of the same domain.

The mapper qualifies the existing reached finite slots below. These words select
existing Source providers; the code does not call them as host addresses. The
mapped profiles must be the pointers already installed in the renderer's vertex
and index streams. The lighting view retains references to the two original
mapped DWORDs, after checking their bit patterns, rather than substituting local
constants.

| Profile/address | Required extent | Qualified offsets and values |
| --- | ---: | --- |
| `00d5f0a8` renderer | `0x68` | `+38 B317E0`, `+5c B287C0`, `+60 B288B0`, `+64 B319B0` |
| `00d61d6c` vertex | `0x28` | `+10 B49980`, `+14 B49A80`, `+24 B48CE0` |
| `00d61de0` index | `0x30` | `+0c B49B60`, `+10 B49C70`, `+2c B49B30` |
| `00d7a24c` lighting | 4 | `3f800000` |
| `00ce38b8` lighting | 4 | `41200000` |

On later borrows all gates and original-host queries run again. The binding also
checks every saved domain/profile/lighting reference, all context members and the
public view's reference identities. It cannot retarget existing metadata to a
different hierarchy, mapper or renderer domain.

## Physical Source evidence

The normal Win32 worker build ran from 06:47:07 to 06:47:53 UTC on 2026-10-10.
`reconstructed_math` and `tool_tests` passed. The worker has no
`local/seed_reference.hpp`, so its generated CTest configuration contains two
tests. No seed was copied or generated; the integrator's existing seeded third
check remains an integration check. The build retained the existing LNK4006
duplicate `spawn_request_id_matches` warning.

The pre-edit Root build objects, actual Release projects and command tlogs,
06:26 build receipt/log and 737 Source733 context pins were frozen before the
worker build. Fresh diagnostic compilation used the recorded MSVC 14.51 x86
Release options, `/fp:strict`, `/EHsc`, `/MD`, and the actual target-specific
include/external settings. `/Bv`, `/sourceDependencies` and class-layout output
were added for evidence. The first baseline diagnostic compile could not find a
quoted `.inc` file in its copied source directory; that failed attempt is retained.
The corrected attempt adds only the frozen original source directory to the
baseline include search. Ten diagnostic objects and all 50 dumpbin commands then
succeeded. No executable was built outside the normal CMake build.

The full object inventory covers 28 physical COFF objects: ten diagnostics, nine
captured Root objects and nine normal worker objects. It retains 16,216 sections,
50,791 physical symbol records including AUX records, 26,579 relocations and
10,214 function-symbol extents; every extent decoded with zero leftover bytes.
The retained records include raw section headers, all code/noncode/EH payloads,
primary/AUX symbols, relocation physical indices, offsets and addends. A decoded
symbol extent is not assumed to contain every control-flow destination.

All 52 existing `Impl` member rows keep their offsets. The only appended members
are `mesh_field_vfs` at 9,924 and `mesh_fields` at 9,928; `Impl` grows from 9,924 to
9,932 bytes. `MeshFieldBinding` is 92 bytes and its public view is 16 bytes. Complete
old/new member tables, including nested rows, are retained in
`member_layout_review.json` in the packet evidence directory.

The captured Root renderer matches all 760 baseline functions. The normal worker
renderer matches all 792 diagnostic candidate functions. Baseline versus
candidate preserves 747 old functions and adds 32; exactly 13 old extents change:

| Changed extents | Explanation |
| --- | --- |
| Application constructor/destructor, `default_delete<Impl>`, `unique_ptr<Impl>` destructor, scalar-deleting `Impl`, `make_unique<Impl>`, and their two allocation unwind helpers | Eight allocation/sized-delete constants change from `0x26c4` to `0x26cc`; otherwise the complete bodies and mapped relocation edges agree. |
| `Impl` constructor | Stores the original host reference and null metadata pointer after existing members; adds a constructor unwind state. |
| `Impl` destructor | Adds ordinary C++ deletion of the optional 92-byte metadata owner. |
| `drain_singletons` and its catch funclet | Compiler factors the unchanged retention predicate out of line and shares a throw tail. |
| Existing `Impl` constructor unwind funclet `$12` | Its instructions and edge remain unchanged; five trailing `int3` padding bytes move to the newly appended funclet. |

The new borrow's phase comparison is at `+0x0d`. Failure branches at `+0x14` and
`+0x21` reach the common ready/retention error at `+0x599`; the retention call's
REL32 operand is at `+0x1b`, before the original hierarchy borrow at `+0x2e`.
The 135-byte emitted metadata constructor has no call or relocation.

The drain review retains both old/new functions and the complete new shared
section: the 151-byte main symbol branches to section-relative `+0xb2`, which is
the 55-byte catch symbol at `+0x1b`. That throw tail lies outside the main symbol.
The complete 123-byte retention predicate is unchanged; activation/shutdown and
post-drain ordering remain unchanged. The evidence does not mistake the isolated
151-byte main extent for a complete semantic body.

All eight unchanged genuine provider units match complete code and relocation
edges against both captured Root and normal worker objects: GUI geometry, GUI
text renderer, mesh buffers, remaining mesh fields, mesh texture fields, texture
loading cache, resource application, and VFS host. Generated anonymous names and
lambda identities have explicit maps. Two otherwise unused texture-cache lambda
methods are matched by the unique full class/method group with identical complete
bytes and edges. Raw RTTI payload spelling differences are recorded separately,
never counted as byte equality.

Whole noncode comparison preserves all other payloads and edges, except recorded
compiler debug/checksum differences and the owner changes. The 48 `.sxdata`
DWORDs resolve through physical COFF indices to the same ordered handler symbols
under the recorded anonymous-name map. The constructor `.xdata$x` table grows
from 140 to 148 bytes and 13 to 14 unwind states; new funclet `$13` releases only
the optional C++ metadata owner. Eleven added `.debug$F` sections and six added
error strings are fully retained. None of these differences is hidden by code or
EH byte normalization.

The full-file identity audit checks 737 frozen Root context records and 431
compiler-reported tracked dependencies, covering 744 unique tracked paths.
It identifies the two candidate owner files separately, preserves exact Git blob
IDs/raw hashes and labels CRLF/LF-only equality explicitly. The four untracked
Root context artifacts are the library, executable, map and last-test log.
All 752 compiler-reported dependencies are frozen, including toolchain headers.

## Scope and retained limits

This is ordinary Source wiring and build/object evidence. It supplies no full
`NativeMeshLoadingContext`, extra item reference, compiler frame, parser call,
startup call, new native pool/provider, default material, native payload cleanup
or retirement path. Existing consumers still require their persistent failure
frames and must retire native payloads before the shared application drain.
There is no native mesh admission, isolated fixture execution, ABI-equivalence
claim, startup/gameplay proof, visual proof, or newly available historical native
artifact. The new public C++ interface is not an original binary ABI replacement.
No CMake, ledger or Ghidra state changed.

Machine-readable results and immutable evidence pins are in
`reports/cc12_mesh_field_application_binding_source.json`; the complete local
evidence and replayable bundle are under
`local/cc12_mesh_field_application_binding_source/`.
