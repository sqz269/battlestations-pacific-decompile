# Actual generated instance geometry

`create_native_instance_geometry_00b4c8d0` returns the actual newly allocated
Model address. Model `+180` owns an actual mesh with an actual section, copied
material and registered dynamic logical stream. It composes the existing
native owners; no `GeneratedInstanceGeometry`, `LogicalVertexStream`, semantic
material projection, shadow reference count or private owner registry is used.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00B4C8D0` | `00B4C8D0..00B4CACA` | ECX native string header; EDX generator; stack source mesh, selected section; EAX raw model; `RET 8` | Complete normal body over actual owners; allocation-unwind composition described below |
| `00B556B0` | `00B556B0..00B556B3` | ECX generator; EAX raw declaration; `RET` | Complete |
| `00B556C0` | `00B556C0..00B556C3` | ECX generator; EAX raw layout; `RET` | Complete |

These are new MSVC Win32 source interfaces, not original binary entry points.
Names are descriptive hypotheses. The older `GENERATED_INSTANCE_GEOMETRY.md`
describes a semantic success fragment; its old model/material lifetime limits
do not describe this separate actual-owner composition.

## Native data and order

The full assembly is essential: Ghidra's one-argument decompilation loses EDX,
misidentifies the two stack arguments as locals, and even reports an unrelated
return register. EBP captures the generator at `B4C8EB`; ESI captures the name
at `B4C8ED`. With all saved registers and locals included, `B4C989` loads the
selected section from `[ESP+30h]`, while `B4C9EE` loads the source mesh from
`[ESP+2Ch]`. The final `B4CAC8` instruction is three bytes, `RET 8`.

Model allocation uses the supplied canonical `01090054` pool (`B74EB0` ignores
the apparent size184h and tail-jumps to `B74D00`), then `B75030` constructs its
actual188h slot. Mesh allocation/construction uses `B73B60/B73D70` through
`GuiNativeGeometryOwners`. One x87 `FLD` of the actual `D7A260` storage supplies
both `B75170(0, mesh, sentinel, sentinel)` floats via `FST` then `FSTP`.
The setter re-reads that SAME sentinel address; equal values preserve the
constructor's model178/17C defaults. This is not a capacity calculation.

The current renderer and its table are captured before `B556B0`. The supported
actual renderer profile is `D5F0A8`, with `B287C0` in current virtual5C. That
numeric native target is executed through its reconstructed actual factory,
with count0, flags1000h and generator+10's actual declaration. Real physical
buffer, synchronization, raw renderer/physical registries and device-lifecycle
services remain required by `NativeLogicalVertexOwnerContext`. There is no
success-only stream factory callback. The new stream's creator is registered
canonically before retained publication, without changing its native count.

Mesh stream0 is the CURRENT selected section's `+3C`; it is deliberately not
source-mesh stream0. Mesh stream1 is the generated stream. Only after both
setters does the routine write stream+54=`80000000h` and release its creator.
Material allocation and full `B18B60` cloning read CURRENT selected+20 next.
The actual index pointer is then read from CURRENT source-mesh+60 and retained.

Fresh `533FA0` section storage receives five DWORDs in exact order: selected
`+08`, `+0C`, `+14`, `+18`, `+10`. It does not inherit selected instance count,
depth bias, generator binding or unused fields. `B864C0` sets the clone material;
`B73C60` appends the section to the mesh. `B73260/B85B80` append mesh streams0
then1 to the section; `B556C0/B86650` install CURRENT generator+14 as its
combined layout. Material, section and mesh creator references are consumed
in that order. Every release decrements actual+04 and resolves the canonical
CURRENT terminal only at zero.

The `B55B20` producer writes the two getter fields at +10 and +14. Neither getter
returns +1C/+20. Both native getters are exactly four bytes. B556B0's other
caller is `B1E990` at `B1EA6C`; B556C0 has only the B4C8D0 call site.

## Companion and failure contracts

`NativeInstanceGeometryAccess` borrows one Model environment, the existing
mesh/section/material companion domain, the existing logical-stream context,
the SAME actual string storage, and the SAME sentinel cell. Preparation and
registration callbacks manage host companions only: a prepared `NativeModelOwner`
must bind the exact allocated slot; completed Model/stream registration must
return their one canonical `NativeModelReference`/`NativeLogicalVertexReference`
without retaining or writing any native field. Their actual owner registry is
the same registry used by model geometry, mesh, section and material lifetimes.

The native EH descriptor at `DF8600` has three entries at `DF85E8`, all with
previous state -1. `CBFA80` returns the captured Model allocation through
`B748C0`, `CBFA88` returns the mesh allocation through `B72F70`, and `CBFA90`
returns the material allocation through `B17D70`. There is no cleanup that
destroys completed Model/mesh/material objects when a later step fails.
The source returns the raw Model slot after its failed constructor and retires
only its failed host companion. Existing mesh/material factory code supplies
their native constructor allocation cleanup.

`NativeInstanceGeometryAcquired` records actual creator references immediately
after successful construction and leaves later partial native effects visible
on exception. It is diagnostic caller bookkeeping, not a native transaction.
Model registration failure preserves the live Model; later failure preserves
the completed Model and every still-owned creator. A creator slot is cleared
before a release to prevent retrying a throwing terminal. Successful return
leaves the one Model creator and no temporary creator fields.

The existing companion factories introduce host allocation/registration failure
handling; those failures are not native FH3 evidence. Null allocation is a C++
diagnostic boundary: native Model/mesh/stream/section failures can subsequently
fault, whereas the host factories throw. Native FH3 propagation, unmasked x87
exceptions, corrupt objects, unsupported current renderer profiles and game
execution remain outside the established interface domain.

## Evidence and replay

`reports/native_instance_geometry.json` records call sites, routine boundaries,
coverage, corrections and live/disk hashes. Ghidra was read through the CLI's
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` binding. The
installed original under `I:/SteamLibrary/steamapps/common/Battlestations Pacific`
was read only. The principal constructor, getters, EH descriptor/table/funclets,
renderer5C cell and sentinel comprise eight matching spans,617 bytes. The main
constructor has174 listed instructions and no flow gap. No Ghidra mutation,
annotation, function creation or ledger change belongs to this worker.

The ignored `local/native_instance_geometry` replay is self-contained. Its
`capture.py` verifies installed bytes against live Ghidra and emits the small
original body/fixup header plus observed profile cells. `runtime.hpp` supplies
canonical actual pools and companion registrations, two distinct source streams,
a full actual material, a hidden real D3D9 HAL device, an actual shared16MiB vertex
buffer and a real combined COM declaration. Raw CPU declaration element arrays
and the borrowed layout identity are explicit fixture inputs; this does not
replay the generator's declaration/layout producer or their complete lifetime.

`probe.cpp` compares the complete installed B4C8D0 control flow with the new
source. Original direct callee sites use ABI thunks into the current library's
native owners, so this is a constructor-composition comparison, not an independent
differential proof of every dependency. It compares normalized Model, mesh,
section, stream and cloned material DWORDs, exact counts, raw registries, both
native getters and canonical final Model/stream release. The native EH handler
is not executed. `build_probe.ps1 -RepositoryRoot <checkout> -CurrentLibraryOnly`
links only that checkout's current `bsp_core.lib` after integration; the initial
worker run adds its separately compiled source object because the integrator
owns CMake. The probe uses `/MD /W4 /WX /fp:strict /MANIFEST:EMBED`.

Strict source compilation passed. `scripts/build.ps1` completed with its one
existing test passing. The complete original/source comparison passed all274
normalized DWORDs and both getters. Sequential runs compare the secondary raw
registry's append delta: the existing logical-stream destructor removes its
primary/physical registrations but leaves the secondary raw list entry.
The integrated current-library-only rerun belongs to the integrator after CMake
registration. No installed gameplay, renderer draw submission or visible scene
equivalence is claimed.
