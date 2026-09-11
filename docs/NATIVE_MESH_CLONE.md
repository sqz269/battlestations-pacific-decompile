# Actual GUI Text mesh cloning

`GuiNativeGeometryOwners::clone_mesh_for_text_00b742a0` now performs the
`B742A0 -> B73F50` branch selected by Text flags `26h`. It returns a distinct
actual `BCh` mesh payload from the canonical `0108FFF8` pool, with its one
creator reference registered in the same `NativeRenderActualOwners` domain.
The caller supplies the actual current `D62D60` table (at least five DWORDs);
the implementation checks its `+10` entry is `B742A0` before allocation. A
different profile is a missing implementation, not a substitute mesh route.

The physical successful path is complete for these flags:

- Copy `+0C` with x87 `FLD/FSTP`, then copy all four DWORDs of every active
  `10h` LOD entry and the live `+50` count through full `B72BD0`. Unused LOD
  entries remain untouched. These names are descriptive hypotheses.
- Retain-share the actual index stream and each actual vertex stream through
  the existing mesh setters. Flags `8` and `10h` are clear in `26h`.
- Reload each source section slot and count, allocate a new canonical section,
  and invoke the actual `B85EF0` copy constructor. Its copied resources keep
  their native ownership until replaced or destroyed.
- For each copied section, allocate a new actual `110h` material and invoke
  existing `clone_native_material_00b18b60`. Register its canonical companion,
  publish it in section `+20`, then consume its creator. The material clone's
  parameter count remains zero as established by that constructor.
- Append the copied section to destination `+54`, retain it, and consume its
  creator. The copy appends if destination sections already exist.
- Copy the exact `+80/+94` bytes. When each source flag is nonzero, perform
  individual x87 copies of `+84..90` or `+98..AC`. When clear, preserve the
  corresponding destination payload. The latter payload lives inside the
  storage declaration's otherwise uninterpreted byte range.
- Resize physical destination weight names to zero, reserve the source's live
  count (native minimum capacity one), and independently copy every actual
  eight-byte string header using the same string pool. Counts/data reload after
  callbacks, and the destination count increments only after each copy.

The generic `004CDC20` append helper remains leased to another harness. Its
call-site behavior is composed locally in `native_mesh_clone.cpp`, reusing
actual `00426520`, `00427110`, and `0041DD40` via the existing actual-header copy
fragment. No alternate string vector, ordinary string allocator, generic
helper replacement, or ledger ownership claim is introduced. The append's
single unwind action `C659E0` computes its position and calls bare `401130`;
there is no invented name rollback.

## Creator references and failures

`NativeMeshCloneAcquired`, initially empty, records the mesh immediately after
its construction and canonical registration, then each section/material as it
is constructed. Successful publication retains independently. The associated
creator cell is cleared before its release. On an exception after construction,
the exact partial mesh and any pending creators remain in that caller-owned
state. The caller must consume or abandon those references through the same
domain; restarting the copy would duplicate effects.

This follows `B73F50`'s two constructor-only unwind states: map `DFABD0` points
to `CC1C00 -> B85B20` (return raw section slot) and `CC1C08 -> B17D70` (return
raw material slot). Each is disarmed after its constructor. No native outer
mesh cleanup follows a later failure. The section/material registration adapter
retains the existing transactional host bind contract: a host registration
failure destroys a completed unregistered creator; a failed native constructor
returns only its raw pool slot. Registration is a new host interface, not
native exception equivalence.

Null allocation throws as a diagnostic. Native `B742A0` instead calls `B73F50`
with null destination, and the material/section paths likewise dereference null;
none is a successful empty clone. Valid storage requires distinct live meshes,
LOD counts `0..4`, stream counts `0..6`, valid vector headers/counts, valid
material extents, successful string/backing allocations, and borrowed source
owners/services that survive callbacks. Corrupt-memory behavior, asynchronous
mutation, generic flags8/10 stream-copy branches and flags2-clear retain-only
dispatch are outside this fixed Text interface.

## Evidence and validation

Read-only Ghidra batches verified existing project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Complete `B742A0` and `B73F50` assembly,
`B72BD0`, the string-append assembly and the constructor unwind maps were
inspected. Raw x87 transfers are explicit MSVC Win32 assembly; they preserve
native signaling-NaN quieting and floating-point status behavior instead of
silently replacing those operations with integer copies. The LOD helper uses
raw DWORD stores, as its assembly requires.

Both changed translation units compile with MSVC Win32 `/std:c++17 /W4 /WX
/O2 /MD /fp:strict /EHsc`. Compilation used the section worker's actual new
header in an ignored validation include directory. All fourteen numeric native
call rows matched the live Ghidra listing. The primary integrator registers the new source
and runs the combined build/checks with the section and Model ownership work.
No new test case or test framework was added. No native-byte differential,
in-game, rendering, visual or ABI replacement claim is made for this composed
mesh path. The interfaces are new C++ APIs, not original binary entry points.
