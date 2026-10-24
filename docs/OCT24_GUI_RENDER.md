# Native GUI rendering, oct24-gui

Dispatch: `local/CODEX_BRIEF_oct24.md` in the cc16-progress worktree; the parked
chain is described in `CC15_OBJECT_GUI_A.md`, "Stop: what a faithful bring-up is
missing". No 004C12B0 manager, screen-base manager, front-end manager,
FrontEndFrame, or in-mission interface manager is instantiated by this work.

## Acceptance status

Steps 1--3 pass the bounded USN04 mission runs below. Native pages remain a separate
deliverable. Input remains outside this lane and blocks button dispatch.

| Step | Runtime selection | Current evidence |
| --- | --- | --- |
| 1. Text, image Font, real glyph buffers/effects | `BSP_NATIVE_GUI_TEXT=1` | Visible native Text in USN04; ON game exit0/gameplay identical; OFF pair_diff exit0 |
| 2. Initial GUI texture resources | `BSP_NATIVE_GUI_RESOURCES=1` | ON game exit0/native texture retirement/gameplay identical; OFF pair_diff exit0 |
| 3. Per-widget Model and page cGroup ownership | `BSP_NATIVE_GUI_NODES=1` | Same actual Model under actual cGroup; ON visible/game exit0/normal retirement; OFF pair_diff exit0 |
| 4. One native page and its widget types | `BSP_NATIVE_GUI_PAGE=1` proposed | Source prerequisites below are integrated; warm actual Screen manager, raw Icon/Text composition, one Page lifetime and ten factory arms still block ON |

All runtime selections default OFF. No comparison-tool noise rule is changed.
USN04 is the deterministic comparison mission; USN21 is excluded from single-pair
identity evidence. All launches use `tools/run_game.ps1`, the drain sentinel,
and the coordinator's device-creation staggering rule.

## Step 1: application composition

`GameNativeGuiRenderApplication` borrows the renderer's actual-owner registry,
raw VFS/string domain, canonical node lifetime runtime, material/parameter pools,
and Text process pool/lineage. It composes the existing sampler/effect and
`GameNativeTextApplication` services. `GameNativeGuiNodeApplication` supplies the
Text's sole actual 188h Model slot and its existing current74 geometry path.
The original glyph bodies remain AB9650 -> AB8530/AB7700 -> ABB630 -> ABAED0 ->
ABA8D0 -> AB8400/AB8CE0 -> AB9FD0 (or ABA270), with actual logical/physical HAL
buffers and mapped writes. No scratch glyph-quad draw substitutes for them.

The explicit bring-up text is a standalone, non-localized literal. It is not a
page, and no projected page is copied into a second owner. Context substitution
requires the application's current Lua context and is rejected for this literal
bring-up rather than choosing another page's VM. This is not button/input work.
Its explicit diagnostic position is (0.05, 0.10, 0), inside the corrected native
GUI camera's depth volume; X/Y are authored bring-up content, while Z=0 is the
normal widget plane.

The GUI pass owns one actual 24h scene and one actual 45Ch pooled camera.
AA3E00's existing camera placement, AA2020 orthographic setup and native label
"X" precede B1F4D0 submission through a `GameNativeGuiQueue` supplied by the
existing mission frame. It creates no second queue, clock, BeginScene, EndScene,
or Present. Font effects must be acquired within renderD's existing open shader
cache bracket: `ShaderGraph::create()` is deliberately one-shot.
Startup requires both `BSP_NATIVE_MISSION_SHIP=1` and
`BSP_NATIVE_MISSION_FRAME=1` when Text is requested; an incomplete selection
fails explicitly. The GUI adapter borrows the frame's actual queue/context and
passes AA3E00's consumed label to its overlay API. It owns no frame graph.
The native callback is separate from the existing sprite callback and runs only
in the shared mission frame's post-world scope, before sprite drawing/capture.
Ordinary front-end frames do not submit an unprepared native Text scene.

The adapter uses the shared renderer's explicit entry-observation API. It keeps
label ownership until that API confirms B1F4D0 entered; the native function then
owns its cleanup, including failure. An interrupted GUI/frame is retained until
process exit. No exceptional continuation or native SEH equivalence is claimed.
The immutable material destruction table is admitted during ready startup,
before mission preparation becomes an active retained acquisition, and remains
borrowed from that same renderer through normal drain.
Sampler/effect metadata is also composed in that quiescent phase. Its material
validation retains the admitted canonical table and keeps the existing ready,
layout and domain checks for every invocation; glyph allocation still occurs
only in the mission's open shader-cache bracket.

The Font arm reuses AD55C0 construction, AD4540 unsigned-WORD lookup, AD4AA0
native tree insertion, and the established native tree/string cleanup. In image
mode there is no live `FontData` map. AC3570 selection walks the actual 10h
registry's circular list and reads each Font image's native name cells. Stable
descriptor identity maps the selected image back to custody metadata; descriptor
names do not select the native Font. See `OCT24_GUI_FONT.md` for exact offsets
and ABI limits. Both native Text and the remaining sprite bridge read that
owner's live glyph records and height. The bridge retains its previous draw
ownership until Step 4 explicitly replaces one page.

### Lifetime and ABI boundary

The mission command must retire all borrowed scene/camera/model references before
`retire_before_fonts()`. That method unlinks the actual scene root, transfers the
same standalone layout to the Text application's existing deletion transport,
executes AB8EE0 flags1, then releases camera and scene. Font retirement follows
while the same Text factory still exists. The shared renderer drains its real
cache/singletons, then effect metadata closes while Text services remain alive.
Entered failures retain the whole graph and prohibit replay or normal teardown.

Native entries include AA6640 fastcall, B75030/B71A80 thiscall RET4, AB9650
thiscall RET, AB8400 thiscall RET8, AD4C30 thiscall RET14, AA3E00 thiscall RET8,
and B1F4D0 thiscall with scene/camera/context/name arguments. These Source
interfaces add explicit service parameters; they do not reproduce original
private stack/FH3/SEH timing or the complete raw widget/registry ABI. The GUI
orthographic numerical helper retains its existing double-to-float qualification.

## Run evidence

The frozen clean-origin baseline is commit
`3822df776d1ec5b675226d9a023b687f2fd4b9c0`, not the older `44b94` label in
its artifact names. The worktree-add command fetched origin again; the recorded
`baseline/source.json` is authoritative. Its worktree was clean, Win32 Release
and both existing checks passed, and its immutable executable is
`local/gui_render/baseline/origin_44b94b4b9.exe` (SHA-256
`560C31AC1792BBF574E96C0D325664F939E5CAA26E235E1B183189FDE8480DEE`). The
same baseline commit was merged into this lane before the GUI OFF build.

USN04 uses `--frames 600 --press-start-frame 30 --menu-select USN04
--mission-frames 181 --mission-frame-seconds 0.05 --window-resolution 1280x720
--present-interval vsync --screenshot-mission-frame 180`. The baseline and the
first GUI OFF run both exited 0. `step1_off_pair.json` reports identical gameplay,
21 unit rows, and all 1392 native rows; its literal exit 1 is solely the two
different screenshot paths. An OFF rerun uses the baseline's exact screenshot
argument, with the original image preserved in `baseline/usn04.png`. That rerun
also exited 0, and `step1_off_verified_pair.json` reports literal **pair_diff
exit 0**, no summary/other differences and no native status/count changes. Its
image is copied to `usn04_off_verified.png`. No log filter or comparison noise
rule is changed.

The GUI OFF executable and its source/hash record are frozen under
`local/gui_render/off/`. `step1_off_build.log` records Win32 Release and 2/2
existing CTests passing. That OFF proof predates the final startup/frame wiring.
`step1_wired_build_retry.log` records the wired Win32 Release build and 2/2
existing checks passing; CMake permanently compiles both GUI application TUs.
The first wired build caught use of a C++20 expression in this C++17 target;
the adapter subsequently used ordinary overload substitution; the final shared
renderer API is now a direct call. Final wired OFF/ON results are below.
These build and earlier OFF observations do not establish an ON
draw, pixel correctness, complete raw widget ABI, or page bring-up.

The first wired ON probe (`usn04_text_on_probe1.log`, source `848a7988e`) ran on
slot14/core30 at 10:04:01Z and exited 1 before any Present: a ready-only material
profile getter was incorrectly called from the active mission preparation
callback. The fix captures its immutable view during ready metadata composition;
the renderer's acquisition/retention guard is unchanged. Image Fonts loaded,
but this failed probe proves no native Text draw or normal retirement. The
replacement build also merges renderD's explicit label-entry API and borrows
the startup's existing RandomThreads; it adds no random owner or frame graph.
Probe2 (`69c8546c5`) exposed the same ready-only getter in the effect consumer's
validation, again before any Present. The effect application now admits its
immutable table at ready metadata composition and reuses it for validation.
`step1_effect_view_build.log` records the resulting build/2 checks passing.
The subsequent merge of main `0b56eb178` includes the native whole-material
texture refresh after model loading, which also covers these earlier GUI effect
creations; the combined build and probe are still pending.
`step1_effect_merged_build.log` now records the combined Win32 Release build and
both existing checks passing.

Probes3--5 exit0 with 599 Presents and normal retirement after 561 GUI passes.
The frozen clean final control is `da031b4f3`, Win32 Release and 2/2 existing
checks passing, under `baseline/final_da031b4f3/`. Probe4's `pair_diff` is literal
exit1 with identical gameplay, all 21 unit rows and all 706 summary lines; the
differences are the two native GUI method rows and renderer/GUI diagnostics.
The actual indexed-draw observer confirms 156 vertices/78 triangles, S_OK,
correct HAL buffers, bound shaders/font textures, full viewport and the same
1280x720 backbuffer target. Probe5 records the actual shader bytecode, constants,
declaration, first four vertices and six indices under `shader_trace/`.

These observations do not yet prove visible Text. Native-frame captures have
`_N001`, `_N060`, `_N180` suffixes; the unsuffixed image is the earlier control,
and must not be attributed to an ON run. The actual `_N180` shows the native
ship. The original AA3E00 assembly establishes eyeZ=-10000 and targetZ=0;
the retained helper had reversed them, mirroring Text's X direction/winding.
The caller mapping and diagnostic's former Z=-10000 are corrected together;
the native CCW culling state is retained. Active Font property lookup now uses
the native registry-list authority instead of host descriptor-name selection.
### Final Step 1 capture

Source `aefe9318a297bc12e21cf6bba578e9d8c7fa5971`, clean tree, is frozen under
`local/gui_render/step1_final/`; its executable SHA-256 is
`bd119933b24e638dfc2e5db499885b70d7e1f891a342ccdf20cc7e276ca13b9a`.
`step1_camera_registry_build.log` records Win32 Release and both existing checks
passing. `usn04_text_on_final.log` and its manifest record game exit0, 599
Presents, visible native Text above the controlled ship and normal retirement
after 561 submissions. `step1_final/usn04_text_N180.png` is the inspected native
capture. The native-frame suffix is essential to identify the actual run image.

Flags are SHIP=1, FRAME=1, GUI_TEXT=1, UNITS=0 and GUI_TRACE=0. There is no
diagnostic draw substitute. The label is built through the original GuiText
layout/glyph services, actual logical/physical HAL buffers, material/effect,
scene traversal and indexed draw on renderD's same mission frame.
`step1_text_on_final_pair.json` is literal exit1: gameplay, all 21 unit rows and
all 706 summaries are identical to clean `da031b4f3`; only native GUI method
rows/renderer diagnostics differ.

`usn04_text_off_final.log` uses the same frozen candidate executable with all
native mission/GUI selections OFF and the clean control's exact arguments.
It exits0; `step1_text_off_final_pair.json` is literal **pair_diff exit0**:
1392 native rows, 706 summaries and all gameplay/unit rows agree; no other
masked log differences remain. `step1_final/usn04_off.png` is its capture.
No comparison noise rule or log filter is changed. Final images, source/hash
record, run manifests/logs and pair results are copied to the coordinator's
`local/gui_render/oct24_gui_step1/` before any worktree removal.

This proves the standalone Text path and normal lifetime in this fixture. It
does not prove a full raw Text/Font-registry singleton ABI, page replacement,
GUI manager bring-up, input dispatch, device recreation or game-update stability.

### Step 2: native initial texture entries

Source `7868f79262d9232f4f792904042fd0dc3a7029ac` is frozen under
`step2_final/` (executable SHA-256
`29b5eaf50adc326930e21bb67454ab0986f11366703e8a9e94156c51d5af61d0`).
`step2_build.log` records Win32 Release and 2/2 existing checks passing.
The fresh clean-main control is `2dc6801ff`, built with 2/2 checks and run
through `usn04_origin_step2.log`; its source/hash record is under
`baseline/step2_2dc6801ff/`.

`usn04_resources_off_final.log` exits0 and `step2_off_pair.json` is literal
**pair_diff exit0** against that clean control. `usn04_resources_on_final.log`
uses SHIP/FRAME/GUI_TEXT/GUI_RESOURCES=1, UNITS/GUI_TRACE=0; it exits0 with
native Text still visible (`step2_final/usn04_native_N180.png`).
`step2_on_pair.json` is literal exit1 with identical gameplay and all 21 unit
rows. The native texture method changes UNIMPLEMENTED -> concrete, calls2.
One GUI summary changes: bridge-owned texture count14 ->13, because the native
texture credit is not inserted into the bridge's separate COM ownership list.

The original whiteGui VFS guard rejects the installed path, preserving null
manager+28; this matches the earlier recorded installation behavior. The
transparent entry at+2C resolves and returns the actual renderer64/B319B0
texture reference. Do not substitute another texture for that optional null.
Normal shutdown logs Text retirement before native GUI texture-credit
retirement, then deletes frontend, retires Fonts and drains the same renderer.
The exception gate retains any outstanding graph before provider cleanup.
This does not implement the GUI manager destructor or activate a page.
See `OCT24_GUI_RESOURCES.md` for credit and original-ABI boundaries. Final
images/manifests/logs/pairs/hash record are copied to the coordinator's
`local/gui_render/oct24_gui_step2/`.

### Step 3: same per-widget Model under an actual cGroup

Source `7e51f1643dc9389adeb2eda5cd8378d5cdf32ec3` is frozen under
`step3_final/` (executable SHA-256
`b67c417895c2bc05da975f6dbdf4d3f5ea5acf4de2704c847ae693efeab97061`).
`step3_build.log` records Win32 Release and 2/2 existing checks passing.
The clean-main control is `f3c26eea8`, built with 2/2 checks and run through
`usn04_origin_step3.log`; its source/hash record is under
`baseline/step3_f3c26eea8/`.

The Text widget owns the same actual 188h Model slot from 01090054/B74EB0,
with the 184h generated Model body B75030/B6F5A0 and its original scalar retirement.
NODES=1 adds one actual 18Ch cGroup slot from 010902F4/B8F5E0. The root adapter
owns that creator credit; Text retains its sole existing Model. The same
parenting runtime links them, and the actual 24h scene's root list contains
the Group. This renderer cGroup is distinct from the EC-byte GUI Group widget.
No second widget Model, page, queue, scene owner or geometry is created.

Group service metadata is composed during ready startup. Its environment is
bound in the existing mission-preparation callback after mission configuration,
before any command. B6D890/current50 propagates the scene and B8EF10 traverses
the genuine child through the same command graph. Commands finish before
unroot -> detach Text -> retire Group -> retire same Text -> camera/scene.
Group metadata remains live through renderer drain and closes before nodes.
Interrupted work retains the full graph. NODES=1 requires TEXT/SHIP/FRAME=1.

`usn04_nodes_on_probe1.log` is the accepted first ON run: all three GUI switches
and SHIP/FRAME=1, UNITS/GUI_TRACE=0. It exits0, captures native Text above the
ship (`step3_final/usn04_native_N180.png`), and records 561 submissions plus
explicit Group/Text/pool retirement before resource and Font retirement.
`step3_on_pair.json` is literal exit1 with identical gameplay and all 21 unit
rows; the same expected native/bridge metadata differences as Step2 remain.
`usn04_nodes_off_final.log` exits0; `step3_off_pair.json` is literal
**pair_diff exit0** against the clean control, including 1392 native rows,
706 summaries and no remaining masked differences. The default-OFF direct
Text route is retained. This does not prove a 124h Page or all 18 raw widget classes.
Final images/logs/manifests/pairs/hash record are copied to the coordinator's
`local/gui_render/oct24_gui_step3/`.

### Step 4: integrated source prerequisites, not Page acceptance

The following bounded packets are compiled with `bsp_game`, without activating
a Page route or adding any GUI-chain object. Existing default-OFF bridge
ownership is retained. They do not add retained factory arms for the ten
unsupported classes and must not be described as all18 widget delivery.

* `OCT24_GUI_PAGE_LOOKUP.md`: AA3140 walks the actual88h manager's +14 Page
  vector, invokes current+7C on actual Pages, compares native8B names, and
  reloads the chosen vector slot after callbacks. No getter/manager construction.
* `OCT24_GUI_PAGE_TAIL.md`: AC6806..AC6878 current Page+18, reader+08, same
  node root propagation, actual reader retirement and same-domain Lua close.
  Full raw Page property dispatch remains a mandatory genuine provider.
* `OCT24_GUI_PAGE_AUTHORITY.md`: one existing host Page/tree ownership
  capability and same-widget aliases; explicitly not an actual124h Page or
  native terminal transport. It cannot be a second owner beside a raw Page.
* `OCT24_GUI_ICON.md`, `OCT24_GUI_ICON_LIFETIME.md`, and
  `OCT24_GUI_ICON_DELAYED.md`: deferred real Icon HAL geometry, exact remaining
  raw allocation/state/terminal contract, and raw delayed-singleton producer /
  normal cancellation/drain. The shared deletion arm and same raw Icon state
  authority are not activated; delayed texture insertion/delivery is absent.
* `OCT24_GUI_LINE_STORAGE.md` / `OCT24_GUI_LINE.md`: actual104h Line and same
  +04 companion, raw point-vector operations and deferred native HAL geometry.
  Manager+28 null is preserved by the material path, without a replacement.
* `OCT24_GUI_SCROLLBAR_STORAGE.md`: actualFCh Scrollbar, constructors/copy,
  canonical identity, exact x87 setters and loaded handling. Full AC85A0 raw
  property traversal and genuine child/layout dispatch remain required.
* `OCT24_GUI_SOUND.md`: actual108h Sound, default-base copy with empty strings,
  recovered +2/-2 reference schedule, stop/reload/cleanup ordering and retained
  failures. Genuine audio companions and current stop providers are required.
* `OCT24_GUI_BUTTON.md`: actualFCh Button, default-only raw list registration,
  copy/destruction and canonical identity. Actual process registry/focus producer
  remains required. This supplies no input or button dispatch.
* `OCT24_GUI_WIDGET_LIFETIME_DISPATCH.md`: closed actual GUI Group/ClipBox
  child20/delete04(1), exact native list self-detachment and ClipBox identity
  retirement. This GUI Group is distinct from Step3's renderer cGroup. Genuine
  class-query/type-phase and timed-entry providers remain mandatory.

Independent source review found and corrected a Page-stage identity guard:
the original reader-stage Page pointer must equal the current prefix's Page,
before the tail borrows raw backing. Sound's copy/reference/stop schedule was
independently checked against the original listings. Neither review adds live
Page, native binary ABI, original FH3/SEH or failure-unwind proof.

Every candidate Screen still reaches the actual GUI manager getter during
AC59A0: AC59C5, AC5B32 and conditionally AC5EE6. GUI sent cc15/coordinator the
required warm actual88h AA5D70/F8BC5C borrow contract. It must use the same
native platform/focus, strings, scenes, pools and owner registry through Page
retirement, reread current publication at each call, and reject null without
entering 004C12B0's constructor. No such supplied production borrow is active
in this lane. The same manager's +08 tree/+14 vector, matching store removal,
current20/04 terminal and sole widget storage authority must be composed before
replacing the bridge projection. `FE_initial`'s two Icons and Text still need
full raw state/lifetime composition. The original18-class audit remains in
`OCT24_GUI_PAGES.md`.

Input is out of scope and remains the blocker for button dispatch after visual
Page bring-up. Step4 is incomplete; no native Page ON run or screenshot is
claimed by this prerequisite batch.

#### Final prerequisite regression check

Candidate source `4e28ec0c9657f545ba360a96e7cc9c6a7af526b6` is frozen under
`local/gui_render/step4_source_final/`; `source.json` records clean source and
full binary hashes. The subsequent merge `2c0775cdf` adds only published WINS
plans/documentation; no compiled source or build configuration changes. The
refreshed clean origin/main control is `561d57228c29f2164c8813f9880f66daf3750cc4`,
frozen under `baseline/step4_561d57228/`. Candidate and control Release builds
pass both existing checks; no new tests are added.

* `usn04_origin_step4_final.log`, `usn04_step4_source_off_final.log`, and
  `usn04_step4_source_cumulative_on_final.log` all exit0, with599 presents.
  Launcher manifests preserve exact arguments, flags, slots and binary hashes.
* `step4_source_off_pair.json`: literal pair_diff exit0, all21 unit rows,
  all1392 native rows and all706 summaries identical. Only existing
  pretranslate-count noise remains; no comparison rules are changed.
* `step4_source_on_pair.json`: cumulative delivered Steps1--3 ON, game exit0,
  gameplay and all21 unit rows identical. Literal pair_diff exit1 reflects the
  two native Text/Model rows, concrete renderer64 resource method, expected
  bridge texture ownership14->13 and native renderer diagnostics. It is not a
  Page ON test. Trace and native full-unit rendering remain OFF.
* `usn04_control_step4_prerequisites_N180.png` was visually inspected: visible
  white native Text above the actual mission ship. The actual cGroup contains
  the SAME Text Model;561 Text submissions retire normally, followed by Group
  scene unlink/return and initial GUI resource-reference retirement.

The final N001/N060/N180 and OFF PNGs, logs/manifests, source provenance,
comparison JSON and build logs are copied with SHA-256 verification into the
coordinator tree's `local/gui_render/oct24_gui_source_prerequisites/`. Prior
accepted Step1/2/3 evidence remains in its corresponding copied directories.

Final screenshots belong under this tree's `local/gui_render/` and are copied
to the cc16-progress tree before this worktree is removed.

#### Further raw Page prerequisites

The next source batch adds actual Icon/AnimIcon state storage and matching
canonical identities, Text actual-field/font/shader operations, raw Grid/Curve
storage, full raw AAA710 child enumeration and the AC4C50 Page property schedule.
Their focused evidence is in `OCT24_GUI_ICON_STORAGE.md`,
`OCT24_GUI_ANIM_ICON_STORAGE.md`, `OCT24_GUI_TEXT_STORAGE.md`,
`OCT24_GUI_TEXT_SHADER_STORAGE.md`, `OCT24_GUI_GRID.md`, `OCT24_GUI_CURVE.md`,
`OCT24_GUI_RAW_CHILDREN.md` and `OCT24_GUI_PAGE_PROPERTIES.md`.

GUI's SAME Model environment now selects raw name transport before its first
Model, closing the domain prerequisite for existing full raw Text bodies;
`OCT24_GUI_RAW_MODEL_NAMES.md` records the source boundary. Independent review
corrected a raw scratch header being treated as a C++ NativeString object.
The same actual8B header now reaches raw AA2490/4BCB80 without a second string.
Icon failures keep their canonical binding and retained graph until process
disposition. The Page path executes only each manager getter's genuine nonnull
fast arm and rejects absent publication without constructing A or unwinding
native graph effects. The unrestricted scene-acquisition entry is preserved.

Normal Win32 Release and both existing checks pass in
`raw_widget_wave_build.log`, `raw_page_wave_build.log` and
`raw_page_warm_build.log`; no tests are added. Original names/comments remain,
with saved preimages in `local/ghidra-annotations-20261024T123855Z.json`.
This is source/build evidence; no native Page ON runtime is claimed.

`OCT24_GUI_PROGBAR_STORAGE.md` records another unresolved full18 boundary:
the native Progbar request140 uses the Icon138/13C pool, writes its value at
the pool's138 slab-index word and its default field at the next slot13C.
The exact alias/return relationship is established from source/listings;
no crash/game consequence is claimed. No repaired trailer, larger pool or fake
production lifetime is introduced. Full18 admission, genuine derived providers,
the warm cc15 manager and sole Page retirement still block Step4 activation.
Input remains outside scope and blocks later button dispatch.

The closed raw tree dispatcher additionally admits optional full canonical
Text/Icon children in its SAME registry/lifetime context; Step1's prefix-only
Text identity is excluded. Real current lineage/timed providers remain
mandatory (`OCT24_GUI_WIDGET_TREE.md`). Raw GUI Model9 now has its true11C/120
default allocation, placement copy, retirement and canonical+04 identity;
its copy factory remains unadmitted until genuine partial-loader failure
custody is supplied (`OCT24_GUI_MODEL_STORAGE.md`). Neither is Page activation.
