# Native GUI rendering, oct24-gui

Dispatch: `local/CODEX_BRIEF_oct24.md` in the cc16-progress worktree; the parked
chain is described in `CC15_OBJECT_GUI_A.md`, "Stop: what a faithful bring-up is
missing". No 004C12B0 manager, screen-base manager, front-end manager,
FrontEndFrame, or in-mission interface manager is instantiated by this work.

## Acceptance status

This is an implementation work log. No native GUI draw or OFF/ON identity is
claimed until its run manifests, literal `pair_diff` exit codes, and screenshots
are recorded below. Input remains outside this lane and blocks button dispatch.

| Step | Runtime selection | Current evidence |
| --- | --- | --- |
| 1. Text, image Font, real glyph buffers/effects | `BSP_NATIVE_GUI_TEXT=1` | Startup/shared-frame wiring built; existing 2/2 checks passed; ON pending |
| 2. Initial GUI texture resources | `BSP_NATIVE_GUI_RESOURCES=1` | Worker built; integration/runtime pending |
| 3. Per-widget Model and page cGroup ownership | `BSP_NATIVE_GUI_NODES=1` planned | Canonical Model composition built; cGroup worker built, integration/ON pending |
| 4. One native page and its widget types | `BSP_NATIVE_GUI_PAGE=1` proposed | Exact 18-class audit in `OCT24_GUI_PAGES.md`; actual Screen manager/page ownership and ten factory arms remain blockers |

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
Its explicit diagnostic position is (0.05, 0.10, -10000), inside the native GUI
camera's depth volume; that content choice is not a recovered widget default.

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
mode there is no live `FontData` map. Stable typed descriptor/name metadata
selects the same image owner; see `OCT24_GUI_FONT.md` for the exact image offsets
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
the adapter now uses ordinary overload substitution. Final wired OFF/ON runs
remain pending. These build and earlier OFF observations do not establish an ON
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
both existing checks passing. The next native probe remains pending.

Final screenshots belong under this tree's `local/gui_render/` and must be
copied to the cc16-progress tree before this worktree is removed.
