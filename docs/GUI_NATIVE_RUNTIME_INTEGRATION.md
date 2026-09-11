# GUI ownership and geometry integration

Addresses: see the39 reviewed entries in `reports/gui_native_runtime_integration.json`.
Validated source commit: `6cb873a521972962d9dac61a0c8160ec988269b1`. Later main changes are outside that build claim.

The GUI owner now retains the existing layout tree and actual pooled model nodes.
Base-property publication precedes children; derived Icon/FrameBox property reads
follow the child traversal, then loaded hooks run. Layout destruction retires the
same model lifetime and permits a queued reference to defer physical pool return.
Actual raw geometry element bounds, visibility and local transforms are composed.

Icon and FrameBox own parsed states and texture references and share the existing
stream/material/geometry infrastructure. Their construction/load callbacks retain
native ordering and require actual external mesh, material, clip and color services.
The plain cGroup root constructor and full node reparent sequence are recovered;
reparenting preserves both world callbacks and assignment-before-registration.

Win32 Release and both existing math CTests passed; all8 seed byte comparisons
matched. Integrated focused fixtures passed actual model-slot lifetime/bounds,
installed `_Mouse.lua` with10 Icons/14 states/4 U flips, and all four authored
`_highlight.lua` FrameBoxes. Each54-vertex output matched relocated original code,
including a nonunit-atlas case, with controlled128x64 texture dimensions. The
root constructor matched four entire396-byte dirty allocations using a shared
reconstructed base; four original child-prepend cases matched with documented
external-call substitutions. Full-parent callback order and attachment guards
were checked with a host fixture. No new permanent tests were added.

The existing executable completed60 frames and exited0 with six fonts loaded.
SHA256: `598E017F9EE792B56B719FDE49BB10A9F8EB0474B7B524F6555FB59FE6CD90A5`. Original game executable and personal
options retained their hashes, lengths and modification times; settings were
isolated under `local/gui-k-personal-j`. This is a startup regression check,
not GUI drawing or gameplay validation.

Saved Ghidra project: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All39 names/comments were read back,13 previous
comment fields preserved, and39 exports refreshed. Missing entry definitions
were recorded under the write lock. Two erroneous FrameBox inventory tags were
removed with prior values preserved; the existing trivial-body category was kept.

Remaining work is explicit: actual cGroup allocation/lifetime/type bindings,
raw MeshObject ownership, material/clip services and application GUI composition.
The next workers own those independent pool, group-owner and mesh-owner packets.
Icon animation/delayed-loading and widget ABI/copy/destruction parity remain open.
Reconstruction names are hypotheses, and these new C++ interfaces are not drop-in
binary replacements. Validation artifacts and source hashes are in the report.
