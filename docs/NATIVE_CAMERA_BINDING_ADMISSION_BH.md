# Camera host binding admission overloads, BH

Addresses: none. This is host composition over the already reviewed camera
owner/reference and registry APIs. It adds no native body, call row, original
ABI implementation, native reference operation, viewport allocation change,
ledger edit or Ghidra mutation.

The approved `docs/NATIVE_COCKPIT_REGISTRY_ADMISSION_BH.md` contract now has its
two narrow camera entry points:

```cpp
NativeCameraOwner(void*, std::size_t, NativeCameraEnvironment&,
    SceneAttachmentRuntime::BindingAdmission&&);
NativeCameraReference(NativeCameraOwner&, NativeCameraCompanionDisposal,
    GeneratedModelLifetimeRuntime::BindingAdmission&&);
```

Each class has one private common constructor taking an admission pointer. The
legacy public constructor delegates with null; the new public constructor
delegates with the address of its rvalue-reference argument. At the existing
bind site, a nonnull pointer selects the admitted registry overload, and null
selects ordinary binding. The new overload never completes an ordinary bind
first. No class retains a token or adds a field.

Owner preparation still validates the actual slot/environment, establishes typed
node/tail lifetimes while preserving the slot preimage, and creates the existing
borrowed views. Its existing try/catch encloses the selected scene bind. Bind
failure ends the camera tail and node typed lifetimes and rethrows before any
native camera constructor has run. The caller still owns raw-slot return.
Abandoned successful preparation still forgets the scene association and ends
typed storage. The overload does not change those responsibilities.

Reference construction still borrows actual +04 through `RenderCommandReference`
without retaining or initializing it. It checks the same live owner, explicit
disposal callback, positive actual count and current supported profile before
the selected lifetime bind. The private target constructor performs all
validation and registration. A failure in that target does not complete a
`NativeCameraReference`, so its bound-phase destructor is not invoked. Both
public delegating bodies are empty; no throwing step follows successful target
construction. Registry rejection leaves a named supplied token active.

Native scene forgetting, pool return, lifetime unbind and companion disposal
remain in their existing terminal order. No viewport code or native constructor,
destructor, store, publication or retain/release sequence changes.

| Static review boundary | Current evidence |
| --- | --- |
| Common owner path | `src/native_camera_owner.cpp:149-174`: delegates, original initializers, selected scene bind, original catch cleanup. |
| Common reference path | `src/native_camera_reference.cpp:19-37`: delegates, actual count reference, original validations/profile checks, selected lifetime bind. |
| Preconstruction identity | Existing `src/native_node_construction.cpp:315-320` forms the actual node key and binds references without reading uninitialized scene state. |
| Count ownership | Existing `include/bsp/render_command_queue.hpp:16-29` borrows the supplied atomic count; it neither adds a retain nor creates a replacement native count. |
| Failed reference construction | All throwing validation/bind work is in the private target; the empty public delegating bodies add no post-target failure point. |
| Unchanged lifecycle | Diff contains constructor delegation and bind selection only in the two sources; teardown and viewport/native operations are unchanged. |

`docs/WORKER_VERIFICATION_CHECKLIST.md` was applied to the current host callees,
identity producers and evidence limits. Its native-boundary and call-site rules
add no native claims here; the report has zero numeric native call rows.

## Verification and source boundary

Base commit: `9ea01f7c0b80ac93d69b13f020b0acd7822076e7`, branch
`agent/orch3-registry-admission-impl-bh`. Verification ran on that complete worker
tree plus exactly these four source/header changes, before the final evidence
commit. The report records their SHA-256 hashes, and the separate final manifest
compares committed file bytes with the build inputs.

`scripts/build.ps1` passed for MSVC Win32 Release, including the existing strict
core flags `/W4 /WX /fp:strict`. This was a whole-target build using the previous
packet's fresh matching-header build directory; affected targets were rebuilt.
No new translation unit was linked against libraries from an earlier runtime
layout. Both existing CTests passed: `reconstructed_math` and
`native_math_differential`. All eight prerequisite seed ranges matched, with
the exporter verifying configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before its read-only byte batch.

All 2,593 source/header/dependency/build-input hashes matched before and after
the build. Frozen source/header inputs, three resulting libraries, exact build
recipe, source overlay, seed evidence and full build/CTest logs are retained
outside the repository. Read-only archive:
`J:/PROG/bsp-evidence/orch3-camera-binding-admission-bh-20260913.zip`.
SHA-256:
`D48ACCA8D8C828ED176653B055CE00ADCBEAB561205A6D375A0779FEF8819796`.
The separate committed-source manifest is
`J:/PROG/bsp-evidence/orch3-camera-binding-admission-bh-20260913.manifest.json`.
The archive recipe pins this worker's absolute paths. To verify a selected
current tree, run seed verification and `scripts/build.ps1` in that root and
record its own input/library hashes and logs; the frozen worker libraries are
not the current combined tree's fixture inputs.

No repository test or external fixture was added for this packet. The parent
owns one coupled current-library camera/helper fixture and the combined-tree
build; those checks remain required before promoting that composition. Existing
CTest success is not camera allocation, native exception/unwind, constructor
runtime or gameplay validation. Prepared persistent cockpit/viewport companions
and their publication/lifetime contracts remain separate work.
