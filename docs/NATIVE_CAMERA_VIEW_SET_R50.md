# Actual camera view setter B71490

The complete 55-byte wrapper takes the actual camera in ECX and a stacked
view-matrix pointer, and returns with RET4. Ghidra's inferred no-argument or
single-register prototype misses that stacked input; assembly is the ABI evidence.

The source borrows one existing `NativeCameraOwner`. It masks the same actual
camera flags at +2F0 with FFFFFE4B, captures the owner profile, and calls the
existing raw B63B30 inverse with ECX pointing to a private 64-byte temporary and
EDX pointing to the original live input. It uses the returned pointer as the
world matrix. Only then does it read the current +34 word from the borrowed
captured D62CF0 profile. B71460 is dispatched through the same owner's existing
pose adapter and actual-backed camera views. A separate B70660 call follows;
B71460's own refresh does not replace this second call.

Unknown profiles/targets throw at the post-inverse dispatch boundary, preserving
earlier flag and inverse effects. The wrapper creates no camera, pool, registry,
publication or semantic camera copy. Initialized actual fields, valid input
extent, stable borrowed bindings, native FP environment and clear DF remain
preconditions. The interface adds an owner companion and does not claim original
register/private-stack/FH3/hardware-fault or concurrent-mutation ABI equivalence.

See `reports/native_camera_view_set_main_r50.json` for fresh PE/live instruction
and call receipts, strict Win32 build/three CTests, the focused original/source
method fixture, and frozen compiler/library/module artifacts. The fixture uses
real pool storage and prepared NativeCameraOwner views with initialized method
inputs; camera construction, arbitrary hierarchy/attachment callbacks, active
renderer/application composition and gameplay are separate evidence boundaries.
