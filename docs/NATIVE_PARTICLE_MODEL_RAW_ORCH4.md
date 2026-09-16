# Object particle resource provider audit

Addresses: `00AF9660`, `00B80D70` (read-only), plus the provider and caller
sites recorded in `reports/native_particle_model_raw_orch4.json`.

## Result and scope

`BDF4C0`, `4C1400`, and `B80720` have actual source providers. A complete generic
Object-resource composition is not established: one parser registered by the
real resource manager, `SkinedMeshAnimation`, selects unimplemented `B92F20`.
That body reaches unimplemented `B92CD0`; its actual item profile also selects
an unbound deleting destructor `B92FA0`. This packet stops at the requested
bounded audit and adds no raw source companion or successful substitute.

This does not establish a blocker for every input. The generic cache hot path
and the existing Mesh/SkinedMesh/MatrixIndexedMesh/Camera/GroupParams provider
families have concrete bodies. A later implementation can explicitly admit a
proved subset, or complete the missing parser/item lifetime before admitting
all six standard parser families. There is no new gameplay validation here.

## Important ownership correction

AF9660's Object+8Ch array stores **resource containers**, not `NativeModelOwner`
objects. The known factories return raw 44h default or 74h game resources:

| Actual factory / container | Current native dispatch | Existing source |
| --- | --- | --- |
| Game factory `CFD850` | +4 = `71B870` | `create_native_game_resource_0071b870`, then `construct_native_game_resource_0071b810` |
| Default factory | `B88340` | `create_native_default_resource_00b88340` |
| Default container `D63228` | +0=`BD30E0`, +4=`B88760` | `delete_native_resource_container_00b88760` |
| Game container `CFD8CC` | +0=`BD30E0`, +4=`718C20` | `delete_native_game_resource_container_00718c20` |

The listed game factory and container profile words were read live. These
factories construct actual refcount-one containers. B80720 retains cache hits;
AF9660 appends each returned raw pointer without another retain or release.
`NativeResourceContainerReferences` implements their zero-reference terminal
using the same resource manager, hierarchy pool, and item-reference chain.
Its mesh-item adapter, `NativeMeshResourceReferences`, then releases actual
mesh objects through the shared `NativeRenderActualOwners` geometry domain.
`NativeModelOwner` is not an overlay or replacement for the container.

The existing no-context `clear_native_object_particle_models_00af8940` decrements
actual +4 and calls current slot zero as a callable process address. That
overload does not translate numeric original-image profiles. A raw composition
must preserve AF8940's reverse captured-cell order while dispatching the
container terminal through the actual container reference service; silently
treating the container as a canonical model companion is incorrect.
The native clear nulls the captured nonnull cell only after successful terminal
return, then decrements the CURRENT count and resamples it. Failure cannot retry
the already-consumed reference decrement.

## Concrete publication and source contracts

WinMain calls `7175D0` at `8F840B` and publishes its result to `F8D31C` at
`8F8414`. This alias is distinct from the factory owner's `E19B90` publication.
`GameSingletonHost::publish_game_resource_factory_008f840b` implements that
publication; AF9660 must borrow the existing alias cell, not call a new getter
to replace an observed null alias.

| Native provider | Concrete source contract |
| --- | --- |
| `BDF4C0` | `resolve_native_vfs_existing_name_00bdf4c0(current_vfs, actual_mutable_name8h, NativeVfsNameResolutionContext&, NativeVfsNameResolutionAcquired&)` |
| `4C1400` | `get_native_resource_manager_004c1400(NativeResourceManagerContext&)`; borrows actual manager publication `010901C4` and lifetime manager `01090AA0` |
| `B80720` | `load_and_cache_native_resource_00b80720(manager, name8h, factory, NativeResourceLoadCacheContext&, NativeResourceLoadCacheAcquired&)`; actual manager tree/work fields and current factory methods |
| `B80D70` | Complete 17-byte wrapper: capture manager+4; call B80720 with the SAME ECX and original name; no wrapper cleanup or conversion |

`NativeDefaultResourceLoadCacheCalls` already binds actual allocation metric
`BE2700`, actual factory bodies `B88340`/`71B870`, and current VFS open through
`NativeVfsRuntimeBindings`. Unknown targets forward to an explicitly supplied
provider; forwarding is not evidence that the provider exists. B80720's root
dispatch uses the actual parser tree and is similarly selective.

All raw string contexts must share actual pool cells `01090AA8/01090AA4/01090AA0`.
The resource manager, VFS resolver, reader hierarchy and cache contexts must
borrow the application's same actual publications and parser tree. Actual
numeric profile data must be readable as required by the existing resource
providers; numeric function words are dispatch identities, not callable host
addresses. Mesh parser/release chains must share the canonical geometry domain.

## AF9660 complete normal flow and capture order

Full native body `AF9660..AF9BAD`, 1358 bytes, was read in pseudocode and
assembly. ECX is actual 98h Object definition; filename is the sole stack
argument; terminal `AF9BAB RET 4`. The body saves the definition at native
ESP+38h before AF8940. Its local area contains reusable 8h string headers,
two 1024-byte formatting buffers, and the numeric sequence counter.

1. Clear old resources through AF8940; construct stem; call current VFS BDF4C0
   at AF96AE, ignoring AL while retaining its mutations.
2. Find the last dot using an actual pooled `.` header. Capture delimiter data
   and length before the search and use those captures for normal return.
   Split extension and stem through actual substring/copy operations.
3. Scan suffix digits with signed-byte comparisons while index > 0. The byte
   at index zero is not tested. A positive digit count creates the `%%0%dd`
   formatting pattern, parses the starting number using `atol`, and takes the
   prefix substring. Number arithmetic wraps as the native DWORD does.
4. For each numbered candidate, build prefix+number+extension, return the two
   temporary headers in native order, then invoke BDF4C0. False ends the loop;
   true loads and appends the resource. The nonnumbered path builds stem+extension
   and performs one BDF4C0 check before loading/appending.
5. On the explicit-factory arm, test F8D31C first, call 4C1400, then **reread**
   F8D31C and pass that current value to B80720. AF9994/AF9999 and AF9AB8/AF9ABD
   establish the two captures; do not carry the earlier nonnull test value.
   On the null arm call 4C1400, then B80D70, which captures manager+4 itself.
6. Capture returned resource in EDI; grow actual array+8C when count==capacity
   with signed max(2*capacity,1), then reload current count/data, conditionally
   store to the computed nonnull cell, and increment current count. No extra
   retain, resource null filter, or rollback is present.
7. Return completed candidate/prefix/extension/stem strings using their native
   state and captured/current pointer schedule. Existing appended resources
   survive later provider/name failures.

Stack evidence: B80D70 pushes two DWORDs and calls B80720 at B80D79, whose
existing provider contract is RET8; the wrapper ends `B80D7E RET4`. AF9853's
`atol` argument is left on stack while AF986A pushes three `sprintf` arguments;
`AF986F ADD ESP,10h` removes both calls' arguments. Each numbered formatting
call has three arguments and `AF98AC ADD ESP,0Ch`. EDI switches roles between
digit count and loaded resource; EBX is explicitly zero at AF9873 and retained
through the numbered branch. These roles come from the whole function listing.

## Exact missing parser and terminal

The actual manager constructor registers `SkinedMeshAnimation` using
`get_native_skined_mesh_animation_parser_00b7e530`. The current primary profile
`D630D8` has bytes `40 de b7 00 20 2a b9 00 20 2f b9 00`; its parse slot +8 is
`B92F20`, not the already-implemented name getter B92A20.

Complete `B92F20..B92F94` allocates 18h (`B92F3A`), calls the actual refcounted
item constructor B868B0 (`B92F54`), installs profile `D63700` (`B92F59`), clears
+0C/+10/+14, then calls `B92CD0` (`B92F7B`) with ECX=item and the original
handle argument. It returns ESI and consumes the handle with RET4. B92CD0's
full body is `B92CD0..B92E4C`, but its internals were not re-audited here.
Live `D63700` starts `BD30E0`, `B92FA0`, `B922E0`, `B92E90`, `6F9D20`.

Current source searches found no B92F20, B92CD0 or D63700 binding. The complete
available parser adapters cover five of the manager's six standard names:
Mesh B947A0, SkinedMesh B94850, MatrixIndexedMesh B94900, Camera B8B240 and
GroupParams B8EB50. The default adapter forwards all parser calls. The animation
name/singleton implementation alone does not cover parsing, typing or terminal
destruction. This is the specific missing cold-path dependency; no assertion
is made that an arbitrary Object filename necessarily contains animation data.

## Failure-frame requirements for a future raw companion

Use a caller-retained operation owning the actual local string headers and a
distinct, immovable BDF4C0 acquisition for the initial stem and each reached
candidate resolution. Each reached B80720 also needs its own retained cache
frame. Publish child metadata before provider entry; do not reuse one scratch
frame, replay failed children or destroy an unresolved failed resolver.

`NativeResourceLoadCacheAcquired` preserves the native five cleanup states and
does not introduce container/cache rollback. Its nested unresolved resolver
has the existing terminate-on-destruction contract. Other cache failures may be
destroyed after their native cleanup; do not incorrectly import the texture
cache's broader failure-destructor rule. Parser providers such as
`NativeMeshResourceCalls` also retain their own acquisitions separately from
the resource-container ownership and must outlive unresolved children.

AF9660's EH states concern strings, not loaded resource rollback. Complete
native FH3 mappings, unrestricted faults/CRT behavior and actual original ABI
remain separate implementation/verification work. This audit does not derive
new cleanup states from the old host facade's C++ RAII behavior.

## Texture companion review

Read-only review of the primary agent's new
`native_particle_type_texture_raw.hpp/.cpp` found no concrete provider/release
ordering error: current renderer capture, retained cache frame, string cleanup,
record append, one canonical decrement/terminal call, and no invented texture
unwind were present. Same-domain identities remain documented caller
preconditions. This observation applies to the working-copy revision reviewed
on 2026-09-16, not subsequent edits or runtime behavior.

## Validation and prior evidence

Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; read-only queries and current source inspection.
AF9660, AF8940, B80D70 and B92F20 bodies were read completely. WinMain was read
only around its factory publication. Provider internals and destructor bodies
not explicitly listed as complete remain prior dependencies. The call report
is checked mechanically. No C++ changed; no build, native differential, native
EH equivalence or gameplay success is claimed.

Prior evidence: [particle resource facade](NATIVE_PARTICLE_TYPE_RESOURCES.md),
[load/cache](NATIVE_RESOURCE_LOAD_CACHE_BY.md),
[manager lifetime](NATIVE_RESOURCE_MANAGER_LIFETIME_CB.md),
[container lifetime](NATIVE_RESOURCE_CONTAINER_LIFETIME_CE.md),
[construction](NATIVE_RESOURCE_CONSTRUCTION.md), and
[mesh providers](NATIVE_MESH_SUBSET_LOADING_CM.md).
