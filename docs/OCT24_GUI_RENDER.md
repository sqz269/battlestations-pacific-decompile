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
| 1. Text, image Font, real glyph buffers/effects | `BSP_NATIVE_GUI_TEXT=1` | Source composition; integration/build/runtime pending |
| 2. Initial GUI texture resources | `BSP_NATIVE_GUI_RESOURCES=1` | Worker built; integration/runtime pending |
| 3. Per-widget Model and page cGroup ownership | Separate page/node bring-up selection pending | Existing bodies composed; worker build pending |
| 4. One native page and its widget types | `BSP_NATIVE_GUI_PAGE=1` planned | Existing runtime/dependency audit underway |

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

Pending. Final screenshots belong under this tree's `local/gui_render/` and must
be copied to the cc16-progress tree before this worktree is removed.
