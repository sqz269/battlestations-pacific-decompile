# Actual instance group upload and generator records

Addresses: 00b1e990, 00b556f0, 00b55780, 00b6dc50, 00b72110, 00b85590

`upload_native_instance_groups_00b1e990` covers the complete normal upload body
over the existing actual command, group, source pointer cells, generated models,
logical streams, retained 28h output entries and batches. It uses the current
canonical model/scene, profile and physical mapping domains from collection and
rendering. The older `instance_upload.cpp` interface remains a separate checked
value projection; this implementation does not use its entries, vectors, queues,
generator hooks, preflight guards or exception cleanup.

## Routine coverage and original interfaces

| Address and inclusive end | Coverage | Original interface | New source binding |
| --- | --- | --- | --- |
| `00B1E990..00B1EB7D` (494 bytes) | complete normal body | ECX command; no stack args; RET | Adds `NativeInstanceGroupUploadAccess&` |
| `00B556F0..00B55777` (136 bytes) | complete | ECX generator unused; stack entry, destination; RET8 | Exact raw generic writer; EDX unused |
| `00B55780..00B55B16` (919 bytes) | complete | ECX generator unused; stack entry, destination; RET8 | Exact raw building writer; EDX adds actual float `00CE3978` address |
| `00B6DC50..00B6DC56` (7 bytes) | complete | ECX node; EAX node+164; RET | Raw existing point-light descriptor alias |
| `00B72110..00B72113` (4 bytes) | complete | ECX outer scene; EAX [scene+1C]; RET | Existing canonical `SceneResource*` publication |
| `00B85590..00B85599` (10 bytes) | complete | ECX section; stack DWORD count; RET4 | Raw section+1C store |

These are new MSVC Win32 C++ interfaces, with six complete routine bodies and no
new partial function fragments. Ghidra originally has no function at `B556F0`;
the verified original byte range ends with RET8 at `B55775`, before eight INT3
bytes. The report records this missing definition for primary coordination.
Worker analysis/export was read-only; no names, comments, prototypes, function
definitions or saved program state were changed.

## Live traversal and submission order

Canonical layouts come from `NativeRenderCommandStorage`,
`NativeRenderGroupStorage`, `NativeRenderPointerArrayStorage`,
`NativeModelTailStorage`, `NativeMeshStorage`, `NativeMeshSectionStorage` and the
existing raw entry/model renderer. This source declares no competing layout.
The command's initial group slot is captured once. Each group is captured from
that actual slot; category is an unsigned DWORD visiting zero then one. A zero
group count skips the entire category. At the outer loop edge, command count
and base are reloaded and compared with the incremented captured slot.

For an active category, model+00's current table is captured before the
outer-scene lighting getter, then group+1C/20 is reloaded for attachment.
`B72110` consumes no stack words: the earlier PUSH0 belongs to model virtual50's
second argument. The checked current target is `B6ED80`, called with the same
actual model binding, current scene resource and `false`. This uses the full
existing scene detach/reload/publish/retain/release/register body. Its existing
`SceneResource`/registry companion remains a source service domain, not a newly
established binary scene-resource layout.

Category one passes the current source range and native signed DWORD pointer
distance as ideal to complete `sort_native_render_pointer_slots_00b1dce0`.
It supplies the existing raw `B51AB0` comparator, whose material order comes
from entry+4 -> section+20 -> material+7C -> effect+B0, compared signed ascending;
ties compare entry+14 by x87 descending. The sorter tests AL only and operates
on original four-byte pointer cells. Its insertion, median/ninther, equal-band,
recursive and heap paths already exist in `native_render_pointer_slot_sort.cpp`;
no vector or replacement sorting implementation was added. The new composition
adds no finite-value guard and does not claim a total ordering for NaN depths.

Map selects current model geometry0 and stream1, reloads group instance count,
and dispatches the current B49980 target with `(count,0,0)`, RET0C. Actual mapping
publishes physical base cursor and lock result through the existing raw owner.
The source pointer cursor is then captured. Each iteration reloads the binding
and generator before current virtual8 dispatch with `(entry,destination)`, RET8.
After the write it reloads binding/generator again, calls B556B0, reads the
current declaration+CC stride and advances by wrapping DWORD arithmetic. The
source end is recomputed from live count then live base before comparing the
incremented captured cursor. It is not a captured vector size or instance count.

Unmap reacquires current model geometry0/stream1 and current B49A80 target. It
does not reuse the earlier mapped stream. Geometry0 and section0 are obtained
again; B85590 stores the freshly reloaded instance count. The current camera is
read through command+28 -> context+8. Full raw B51A20 receives eight original
stack words `(0,section,geometry,model,camera,1-or-half,0,555h)`, RET20, preserving
the original output pointer and untouched sort words+20/+24. Its depth path uses
the existing full native camera-view/model-world-sphere/point kernels.

The parent span B1EACC..B1EB1B has one compiled `detail::initialize_upload_output`
adapter, explicitly not another original function. It retains MOVSS alpha,
FLDZ, camera/model reloads, FST depth without popping zero, the alpha
MOVSS/FLD/FSTP crossing, output reload and final FSTP leading argument. This
preserves the original peak x87 occupancy; ordinary C++ float arguments would
not preserve that sequence.

After entry initialization, category one reloads output and batch1. Category
zero reloads the section's effect, obtains its unsigned AC selector via B17300,
then reloads output and command+0C+selector*4. Full B51CB0 appends that raw pointer
to actual batch storage, growing through existing B51B50. Both categories can
target the same batch. There is no selector-zero assumption, retained source
entry, output clone, second queue or private owner registration.

## Concrete generator dispatch

Producer `B44FD0` requests `uf44uf44uf44.mvfm` and publishes table D61BFC, whose
actual +8 is B556F0. Producer B450D0 requests nine `uf44` records and publishes
D61C1C, whose +8 is B55780. Both call B55B20, which produces declaration+10 and
combined layout+14. The current table entries and their installed/live bytes
were checked. The actual logical vertex profile is D61D6C (13 DWORDs, map/unmap
slots B49980/B49A80); D61CC0 is a separate element-size table, not a vtable.

Both writers refresh entry+0C's raw world matrix through existing B6DB70 if its
flag byte lacks bit2. The generic writer emits twelve sequential transposed
world FLD/FSTP32 pairs, retaining forward overlap effects. Building performs the
same twelve copies, then RELOADS entry+0C before aliasing its point-light array
at+164. It keeps the original signed count clamp to three, unsigned presence
branches, raw list-base reloads and ordered 1EC/184 light field reads. Missing
records follow the original zero stores. Final writes replace output+6C with
entry visibility, +7C with unsigned count converted through FILD and conditional
FADD of actual float 2^32, and +8C with B179F0(0)'s diffuse alpha.

The complete assembly is retained in source with instruction addresses. The
building writer saves its added EDX constant pointer outside native scratch and
adjusts only the affected original argument offsets and final stack removal.
It does not stage output or substitute a semantic light list. Existing raw
world refresh, point-light descriptor and material diffuse providers receive
the actual original objects. Other current generator/model/map targets are
outside this concrete profile domain and never dispatch a successful no-op.

## Verification and limits

`local/native_instance_group_upload/capture.py` verifies the project/program
through `bsp.py ghidra` for each read, compares complete spans to the installed
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
and generates byte headers, fixups, assembly and hashes. The report records
23 matching spans totaling 3,206 bytes, including both generator profiles and
the actual 13-word logical stream profile. Existing seed verification passed.

The focused ignored Win32 fixture copies the six complete routines, comparator
and all ten original sorting kernels to isolated memory, fixes direct edges,
and changes those code allocations from RW to RX. Callees outside this set use
the current library's concrete providers with original argument adapters.
It extends the preserved AU geometry fixture's actual pools, canonical owner
associations, actual strings, combined COM vertex layout and real 16 MiB D3D9
dynamic vertex buffer. It does not load or call the running game's image.

Both complete upload runs compare 2,266 normalized/output DWORDs: 48- and
144-byte declarations, two generated models, 44 raw input entries, a 42-entry
category-one sort with repeated depth classes, two full raw output rows,
physical cursor/depth/dynamic-lock counters, readback of actual COM buffer
regions, distinct and shared batch routing, and canonical final cleanup. The
building case enters a real previous-scene zero callback that changes category
and source counts, camera publication and the outer group end before mapping.
The callback runs once in each original/source execution. A further nine
focused generator rows compare 600 DWORDs covering every light-presence branch,
the negative-count unsigned conversion and generic forward aliases. Fixture
point-light value rows are raw borrowed test data, not proof of light lifetime.
The same compiled argument adapter also matches the copied original parent
span with seven incoming x87 values: native peak occupancy nine raises masked
stack fault, both status words are `0841`, visibility is `FFC00000`, and all ten
output DWORDs match. Both declaration modes exercise this concrete FP edge.

`scripts/build.ps1` passed the existing Release Win32 build and both checks.
Owned source and fixture compile with `/std:c++17 /W4 /WX /fp:strict /MD`; the
probe embeds its manifest. Worker mode adds the freshly compiled unregistered
AV object to an ignored copy of the current archive. Default `run.ps1` compiles
only `probe.cpp` against the supplied current library directory; primary
integration owns that final registered-library run and CMake registration.

Valid producer-created storage, live canonical companions and supported current
profiles are required. Existing providers retain their published domain limits.
There is no upload preflight, lock-error conversion, exception rollback or
unmap-on-exception; partial effects survive failure as the native schedule
permits. Hardware faults, FH3/SEH equivalence, malformed container safety,
binary detour compatibility and application/gameplay rendering are not proved.
