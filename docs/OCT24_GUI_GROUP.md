# Native GUI renderer-side cGroup prerequisite

This packet adds one optional renderer-side cGroup above the existing standalone
Text Model. It constructs no page, cGuiLayer, Screen, 004C12B0 manager, front-end
manager or interface manager. It is a prerequisite for Step 3 in
`OCT24_GUI_RENDER.md`, not completion of Step 4 or button/input dispatch.

`GameNativeGuiGroupApplication` borrows the renderer's canonical node lifetime,
actual-owner registry, original group pool010902F4, VFS raw-name domain, live
type descriptors and constant/table views. Its constructor only composes these
services. `create_root(name, same_text_model_reference)` allocates one actual18Ch
slot with B8F310, reserves actual-owner admission, constructs B8F5E0 through the
same raw name header/domain, creates one NativeGroupReference over actual+04 and
publishes it to the same actual-owner registry. The borrowed Text remains owned
by its existing widget/Text application. The supplied SAME
NativeNodeParentingRuntime executes B6E680 to parent/unparent it; no second
transform, hierarchy, node registry, count, Model or attachment array is created.

The caller submits the root's actual `group_owner().storage.node` and canonical
reference in place of the standalone Text root. After all queued references and
the actual scene/root registration retire, `detach_text()` executes B6E680 null
through that same runtime. Text can then follow its existing AB8EE0/AB75A0
retirement. `retire_root()` requires no remaining children/scene/root and the
sole creator credit. B8EEC0 -> shared B6F310 -> B8F8C0 flags1 destroys the actual
group and returns its slot; the terminal callback unbinds the exact registry
association. All entered failures retain the same persistent slot/name/owner/
reference frame; replay and partial cleanup are prohibited.

## Render and bounds evidence

The configured saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live `ghidra ensure --status`, `ghidra count` and
`ghidra proto 00B8EF10` confirmed that program before analysis. The live count
was 77629, matching the saved snapshot. The existing project config, exported
listing/pseudocode and live table/disassembly queries were used; no re-import
or game-install mutation occurred.

Live bytes at D634F8 confirm its +20 target B8EF10 and +48 target B8F100. B8EF10
is native thiscall with ECX=actual group; context, unused LOD, visibility and
flags are the four original stack words, RET10h. The 110 reachable instructions
from B8EF10 through B8F08C were inspected in the listing. The apparent gap
B8F016..B8F01F contains only `lea esp,[esp]` / `lea ecx,[ecx]` alignment padding
and is bypassed by the native unconditional jump.

`render_native_group_children_00b8ef10` adds EDX=the established render access,
retains the native18h local frame, and keeps its two private words outside that
frame. The original caller's context slot is still used for the projected-depth
denominator and computed LOD. The original incoming flags slot still receives
the per-child x87 visibility product after flags are captured in EBP. Native
FLT32 spill/reload order, integer ABS, x87/SSE NaN branches, strict184 threshold,
cull-camera child mask, current group48/child20 slot reads and post-callback next
reads remain in native order. Actual table translation is integer-only while
an x87 value is live. Unknown slots retain the existing qualified application
dispatch after required spills; missing profiles fail explicitly.

The B8F100 static arm B8F12F..B8F18B is instruction-identical to the existing
raw Model B6E8C6..B6E922 cache/transform/copy arm. It reuses that complete raw
body, including world refresh and ordered x87 copies. Byte175 dynamic selection
uses the existing B8EBE0 aggregation over the SAME canonical Group owner and
actual178/17C descriptor. Current child48 calls dispatch raw Model/Group bodies
or the existing application service. The dynamic arm retains the repository's
existing CameraTransform companion-array projection; it is not a newly proven
raw native pointer/FH3/SEH ABI. No precomputed or fake bounds replace it.

`NativeRender20Profiles` appends optional group table/environment bindings.
Both existing constructor overloads default the new final argument to null;
legacy callers retain their existing supported domain. NativeTracelineRenderAccess
layout/offsets are unchanged. A selected Group must use the same actual node,
table and owner domain. Integration must pass `&group_application.groups()` to
the scene traversal's existing profiles and admit D634F8 with58h bytes in
GameNativeMissionModel::profile. No native numeric address is called as a host
function pointer.

## Validation and boundary

`scripts/build.ps1` completed Win32 Release successfully, including compilation
of the new application TU and modified raw traversal, and linked bsp_game.exe.
Both existing CTests passed: reconstructed_math (0.36s), tool_tests (12.69s),
2/2 overall (13.10s). Evidence is `local/oct24_gui_group_build.log`. These
checks do not execute the new Group path. No new tests were added.

The new root composition was compiled through a temporary private CMake TU
admission, restored byte-for-byte before commit. Final registration and the
default-OFF `BSP_NATIVE_GUI_NODES=1` runtime selector belong to the integrator.
This packet performs no game launches, no OFF/ON comparisons and no screenshot
or visual/native ABI/gameplay claims. Runtime group construction, queued draw,
successful teardown and post-integration behavior remain unverified here.

Both existing Ghidra names were retained and their evidence comments appended
under `ghidra_annotate.py --apply`'s write lock. The saved project's old comments
are recorded in `local/ghidra-annotations-20261024T091518Z.json`; the project was
saved and both affected exports refreshed. Address-to-name and additional
source-function records are in the corresponding B80000 ledger shards.
