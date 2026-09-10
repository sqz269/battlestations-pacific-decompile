# Retained command to material dispatch

Command vtable 00D5E5E0 slot0 is 00B1D950. The C++ orchestration now preserves
its readiness/frame gates, captured camera, retained targets, two batches,
serial or two-job preparation rule and diagnostic X reset. Required scene,
material and job operations remain explicit dependencies. See
RENDER_COMMAND_EXECUTION.md.

Batch preparation 00B51DF0 reads configuration through 00B1CB30, builds the
native unsigned64 material/texture/depth key for batch0, and selects the native
material/depth comparator for the other batch. It sorts the actual retained
queue pointer array. Camera render mode selects material programs independently
of this batch index. See RENDER_BATCH_PREPARATION.md and MATERIAL_SORT_METADATA.md.

Batch execution 00B55550 checks frame activity, clears the cached effect-owner
identity0108FBF4 and dispatches each entry through material virtual14. The
concrete 00B45360 selector and 00B44750/00B43410 draw orchestration are implemented
around required actual owner/constant operations. All fourteen camera modes,
the alternate/special passes, final-LOD skip and live override boundaries have
focused native selector coverage.00B42350 remains an incomplete material
constant builder; compilation is not a full frame proof.

Queue/context/group ownership and hierarchy unlink are implemented in
RENDER_COMMAND_QUEUE.md; generated-model point-light/geometry/scene cleanup is
in GENERATED_MODEL_LIFETIME.md. Native allocator, pool, SEH, job-service and
complete renderer/world lifetimes remain distinct open work.

FRAME_BOUNDS_INTEGRATION.md records the installed diagnostic camera, target,
upload, sort and model cleanup checks. The earlier analysis-only
reports/render_batch_evidence.json remains historical evidence, superseded for
implementation status by reports/frame_bounds_integration_validation.json.
