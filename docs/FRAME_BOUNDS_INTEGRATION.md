# Frame, camera and model ownership integration

The installed mesh diagnostic now uses the recovered model sphere, scene
attachment, batch preparation, camera frame, retained targets and model teardown
paths together. These are Win32 C++ interfaces over actual retained host objects;
they are not native-layout binary replacements or a runnable game rebuild.

| Path | Implemented behavior | Verification |
| --- | --- | --- |
| Model bounds and scene | Shared transform/cache bits, transformed local sphere, type-gated scene registry and ordered reference replacement | Native arithmetic evidence, focused fixtures and actual installed model upload |
| Batch preparation | Actual queued pointer array, native key fields and comparator selection | Native permutation/key comparisons and retained installed material/texture identities |
| Command ownership | Queue/context/group cleanup, root/hierarchy unlink and required generated-model virtual18 | Callback/reentry fixture plus actual installed group destruction |
| Camera frame | Same camera and renderer caches, viewport/scissor, Clear, inverse VP/frustum, clipping counts and ambient conversion | 38 native arithmetic comparisons, COM ordering and three signaling-NaN copy checks; installed device state readback |
| Targets | Retained surface groups, default color, null/identity rules, all four color slots, depth and sRGB state | Real device identities/counters and independent failure/publication fixture |
| Model lifetime | Point-light backlinks, child release, byte44 self-release gate, retained geometry/scene cleanup and physical storage disposal | Corrected point-light fixture and three actual diagnostic model allocations disposed |
| Material dispatch | Fourteen camera modes, special/alternate/LOD branches, override iteration, geometry/state/upload/draw orchestration | 21 native selector cases selecting 24 program/override identities; strict build. Full constants backend remains required |

The shared affine point kernel now lives in `camera_affine.cpp`. Model bounds
and entry depth use that same native x87 schedule. The effect owner is retained
across its compiled mode passes and clones; descriptor priority/pipe fields and
construction serials come from their actual producer steps. The diagnostic
texture domain acquires `error.tga` before the effect serial, increments a
texture serial only after successful physical construction, and preserves that
identity when the retained image is recreated or its lowercase name is reused.
This domain starts with explicit zero counters; native global cache history is
not inferred from it.

The model `+164` list is **point lights**, separate from retained geometry `+180`.
The point-light constructor/vtable/type descriptor and reciprocal unlink callers
establish this, as do the two constant consumers. `GeneratedModelPointLightLinks`
shares the actual borrowed owner identity, position/radius, color and reverse
model pointers. The initial geometry-backlink interpretation was corrected
before final annotations. The diagnostic has no point lights, and its instance
writer reads the same empty list used by lifetime cleanup. A focused fixture
exercises nonempty borrowed light backlinks while retaining geometry separately.

Each model's host control holds its physical allocation until native virtual18
reaches terminal release. Terminal disposal clears the control pointer before
deleting the model; subsequent shared-owner reset cannot delete it again. Actual
`RenderCommand` batches and groups own the generated entries and use recovered
group teardown. Runtime associations and scene owners outlive these controls.
The probe does not execute the full command renderer through placeholder hooks.

The installed GPU check produces two queued entries sharing the physical
instance pool, distinct material clones and the same effect. It verifies stream,
index, declaration, frequency, texture and constant readback, target/camera
state, three model disposals and restoration of prior device state. The final
readback has 2,499 nonblack pixels and 54 colors. Visual inspection shows a very
dark diagnostic silhouette. These checks establish the listed data/state paths;
they do not establish original-game visual parity or final-display correctness.

Independent camera review corrected three x87 boundaries: clip coefficients
quiet signaling NaNs in the cache while COM receives the original input;
plane-set copies process all sixteen records; enabled camera commands spill
depth before calling the Clear helper even when flags are zero. Existing COM
checks and a small local signaling-NaN case passed after these corrections.

Ghidra work stays in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The integration report records annotation preimages,
flow repairs, missing definitions, refreshed exports and original-PE hashes.
Raw decompilation stays in ignored `exports/`. Code and data byte agreement is
identity evidence, not execution equivalence.

Remaining dependencies include the actual `00B46A70` scene/system gatherer,
complete `00B42350` material builder and its dynamic/bone/skin owners, diagnostic
and material callbacks, effect-plane construction, job/cache/type initialization
and physical engine allocation policies. `MaterialEntryOperations` requires
those operations explicitly. `config/parallel_work.json` contains three disjoint
material-builder packets, with shared buffers and execution order retained by
the integrator. Full startup/world execution and gameplay validation remain open.

See `reports/frame_bounds_integration_validation.json`,
`reports/frame_bounds_integration_native_identity.json`,
`reports/frame_bounds_integration_annotations.json` and the per-module audits.
