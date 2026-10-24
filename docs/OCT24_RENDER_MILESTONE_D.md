# Native mission render milestone D

Lane `oct24-renderD`, 2026-10-24. The master runtime switch remains
`BSP_NATIVE_MISSION_SHIP=1`, default OFF. This extends the installed-model
composition documented in [milestone A-C](OCT18B_RENDER_MILESTONE.md).
New adapters add no Original function-body credit or drop-in binary ABI claim.

## D1: simulation-owned poses

The mission/frame owners lend their existing `GameUnitsHost` and
`GameMissionLuaHost` through `native_render_unit_source()`. The renderer asks
for that source each frame; its retained unit pointer is an identity check,
not another owner. `GameNativeMissionUnits::read_transform()` reads the current
canonical entity world rows through `unit_pose()`, after checking the existing
world-valid byte. It never reads the diagnostic census row as a pose, advances
the simulation, refreshes a simulation cache, or writes a simulation field.

Each drawable composes immutable authored asset-local hierarchy matrices with
the current borrowed unit matrix through `00413920`, then calls native node
setter `00B6DB10`. Native transform/cache invalidation remains responsible for
the world matrix and model sphere. Visibility is published with `00B6DA70`;
the actual static-model render slot `D62DE8+20 -> B748E0` tests node `+AC`
before sphere/LOD/geometry submission. No simulation pose is retained in the
renderer. D1 selects the controlled unit and its actual
`VehicleClass[type_id].Mesh`, rather than inventing a diagnostic unit. In this
installation USN04's `Fletcher-class01` uses class 364 and `sims.mmod`; an initial
attempt incorrectly selected by the old diagnostic Fletcher asset path and
stopped before drawing. The corrected selection preserves the installed class.

The initial camera is fitted to the posed native model spheres and uses native
camera look-at/projection/cache operations. It stays a diagnostic camera;
there is no exposed live mission-camera pose bridge. The far plane scales with
the fitted scene radius. Lighting/fog remain the explicit A-C diagnostics.

Original ABI/evidence and provider limits are recorded in
[unit source](OCT24_RENDER_D_UNITS.md) and [model domain](OCT24_RENDER_D_MODELS.md).
In particular, `B6DB10` consumes an actual node plus matrix, `B6DA70` consumes
factor/recurse words (`RET 8`), and the reconstructed interfaces add explicit
source contexts. No new Ghidra mutation accompanies this composition.

## Validation record

The clean control is `origin/main` `515837c10ad11dbc4cdb3fd698323f9aad3c2d2a`.
Before source integration its Win32 Release executable was frozen as
`local/render_D/origin_515837c10.exe`, SHA-256
`1f5eb226c20514cc6590968830b5191b8e65f6b6e41bcf3855c4e79684762797`.
The recorded source status was clean. `./scripts/build.ps1` passed both existing
CTest entries (`reconstructed_math`, `tool_tests`); the complete baseline log is
`local/render_D/baseline_build.log`.

D1's corrected executable is `local/render_D/d1_live_unit.exe`, SHA-256
`6c1f1232525f4ee49c3cbe1d11db0aee7afeef47c28a0d2f29482b53b3dbe9ce`.
Its `local/render_D/d1_live_build.log` records both existing tests passing.
Control and D1 OFF runs use USN04, difficulty 1, 600 application frames,
press-start 30, 181 mission frames at 0.05 seconds, 1280x720, and vsync. Both
processes exit 0. The full comparison
`local/render_D/d1_live_off_pair.json` reports **pair_diff exit 0**: all 21 unit rows,
706 summaries, and 1392 native rows match; no status/count or other unmasked
differences remain. Recognized OS pretranslation noise is reported separately.

Control run: `local/render_D/origin_retry_usn04.log`; D1 OFF:
`local/render_D/d1_live_off_usn04.log`. Their adjacent manifests retain arguments,
environment and executable hashes. The first control attempt failed device
creation with `0x88760868` and exit 4; `query session` showed an active console
session. The unchanged control succeeded on the staggered retry. That failed
attempt is retained as `local/render_D/origin_usn04.log` and is not pair evidence.

The corrected ON run is `local/render_D/d1_live_slot14_on_usn04.log`. It exits
**0**, completes 599 successful presents, and draws the actual installed
`models/ships/us/sims.mmod` through 17 native model nodes. Each command records
100 `B24010` calls and 100 successful returns, then an empty queue and retired
command/context/batch owners. `local/render_D/d1_on_pair.json` compares it to
OFF: **all gameplay rows, 21 unit rows, 706 summaries and 1392 native rows are
identical**, including all native call counts and statuses. The raw tool exits 1
because render/capture diagnostics differ; no masks were added to obtain this.

The landmark pose records show current unit 3 translation advancing from
`(-13499.364258, 0.000955, -13499.602539)` at frame 1 to
`(-13396.840820, 0.740812, -13435.619141)` at frame 180. Forward/pitch rows also
change. Unmodified back-buffer captures are `local/render_D/d1_N001.png`,
`d1_N060.png`, and `d1_N180.png`. Visual inspection confirms textured Sims
geometry and translation through the fixed view; by frame 180 much of the ship
has left that initial view. Black panels in the early capture are the existing
sprite UI. No postprocess or image editing was applied.

Default 14-slot launches repeatedly refilled before the queued ON run could
start. Only this lane's unstarted launcher was stopped; no other runs changed.
The successful short ON run used the supported launcher `-Slots 15` with core
list `2,4,...,30`, tag `slot14`, on the verified 32-logical-processor host. This
lane kept one active run, with device launches more than 60 seconds apart.

The all-model/frame steps are recorded below as they complete. Build/static and
this bounded runtime evidence do not establish Original ABI, lost-device recovery,
or retail-render parity. The original exact `error.dds` shutdown-residue exception
remains unchanged.

### D1 integration refresh

Before publication, `origin/main` was merged and the complete Win32 build/tests
passed again (`local/render_D/d1_integrated_build.log`). A second clean control
was built in the isolated `oct24-renderD-reference` tree at
`3822df776d1ec5b675226d9a023b687f2fd4b9c0`, with a clean source status and both
existing tests passing. Its frozen `local/render_D/origin_current.exe` SHA-256
is `27d1df49772ba965709424aed39ae903d49b1f28d960d3ec9793f4c6e418f65f`.
The integrated D1 executable `local/render_D/d1_integrated.exe` SHA-256 is
`cce62e2bc675366837e75ec51718628ac97794954a99f7ad60c06f4db4a48ec8`.
`origin_current_usn04.log` against `d1_integrated_off_usn04.log` produces
`d1_integrated_off_pair.json`: **exit 0**, again all 21 unit rows, 706 summaries
and 1392 native rows/statuses/counts identical. These are separate from the
earlier pinned ON proof above.

## D3: native frame phase/order

`BSP_NATIVE_MISSION_FRAME=1` additionally selects the existing
`004C6C30 -> 004CA440 -> 004CA1F0` begin/render/finish bodies. The same renderer
and queue perform clear, native camera update, scene traversal/sort/submission,
command execution, diagnostic overlay/capture, and EndScene/Present. The actual
game render phase survives a retained queue; a claimed failure cannot fall
through into a second frame. No second command graph or BeginScene is created.
The shared device bridge still retires foreign sprite textures on recreation.

Each default-OFF switch `BSP_NATIVE_MISSION_TERRAIN`, `_OCEAN`, `_WEATHER`,
`_POST`, and `_TONEMAP` is an explicit no-op with a once-only blocker report,
including when requested. [Frame evidence](OCT24_RENDER_D_FRAME.md) identifies
the missing native owners and the unavailable HUD/audio/shadow/profiler and
cleanup callbacks. These boundaries do not claim recovered multipass ordering.
Terrain is closer than ocean by its retained geometry contracts, but neither
has the complete providers needed for a real optional pass in this step.

A startup borrower can prepare native GUI resources inside the existing cache
bracket. Its later overlay borrows the current queue and original `00B0D0D0`
service `+1D4` context, retires the completed world command companion, and submits
one original eight-byte label through the same `B1F4D0` graph. The renderer does
not own a second GUI pipeline. This seam is provided for the separate GUI lane;
the D3 capture still uses the existing sprite overlay.

`local/render_D/d3_frame.exe` SHA-256 is
`b86f47544fae2f657a37509e99d8a35ad5bfb1734c743ca459f4d567b30745af`.
`d3_build.log` records both existing tests passing. Under the same USN04 arguments
as D1, `d3_off_usn04.log` versus the clean `origin_current_usn04.log` gives
`d3_off_pair.json`: **pair_diff exit 0**. The run with master, frame and all five
no-op switches ON (`d3_on_usn04.log`) exits **0**, completes 599 presents, and
records all five blockers. `d3_on_pair.json` reports identical gameplay, all 21
unit rows, all 706 summaries and all 1392 native rows/statuses/counts. Its raw
exit 1 consists of render/capture diagnostics; no new masks were introduced.
The unmodified ON captures `d3_N001.png`, `d3_N060.png`, `d3_N180.png` are retained
under `local/render_D`. The first was visually inspected and retains D1's textured
live Sims and existing black sprite panels. This does not establish retail
lighting, mission-camera parity, or real terrain/ocean/post rendering.

## D2: all unit and mounted device models

`BSP_NATIVE_MISSION_UNITS=1`, default OFF, selects the complete unit/model
composition beneath the existing master. With it unset the controlled-unit
route remains selectable. Both routes borrow current simulation poses; neither
retains a second pose array. `BSP_NATIVE_MISSION_VIEW_UNIT=<index>` is an optional
diagnostic focus selector. The default focus is the controlled unit. Each frame
fits the camera to its current native model spheres through the native camera
setters, with a minimum 10000-unit far plane. This remains a diagnostic view,
not a recovered mission-camera controller.

The installed USN04 source loads 18 ships and three Val planes, using nine
distinct unit assets. Their genuine model instances total 523 drawables. The
matrix-indexed plane fuselage uses its actual `D63590`/194h owner and ordered
10-node palette; plain palette nodes and the reused cockpit node borrow the
same node/string/type/pool domains. Native material parameter30 refreshes stale
bone world caches before reading `+F0`; no copied palette matrices are added.
Plane creator defaults hide interior cockpit14 and broken-wing items17/16,
preserve exterior cockpit10, and survive whole-unit visibility updates. Pose
animation, rotor blur and damage-part selection are not inferred.

The 406 canonical gun/platform rows produce 379 nonempty physical device models
and 27 intentionally meshless platforms across 20 device asset strings.
Articulation reads the same hierarchy calculation used by firing, including
current unit pose, mount, heading and elevation. It publishes each composed
matrix directly through `B6DB10`. The shared firing calculation preserves its
original preconditions, arithmetic, muzzle offsets, counters and diagnostic
writes. No last-shot pose or live angle copy is used. Runtime unit/gun additions
extend metadata before BeginScene; changed existing identities are rejected.

All instances use the same native preload/material/mesh path. Repeated assets
share immutable native meshes while owning distinct actual model nodes. Native
`B748E0` camera coverage and `B72F80` inclusive serialized LOD-phase selection
remain responsible for LOD. Scene admission now accepts the actual ordinary and
matrix model prefixes through their shared lifetime interface, with canonical
node/scene registries, actual `+04` count and genuine profile checks.

The one existing shader cache stays open for incremental material loading.
Only a ready graph with completed creation and the identical live cache owner
is admitted; it is released once before native singleton shutdown. Later roots
temporarily detach only the genuine outer scene's lighting credit, insert roots
through `B6D890`, then restore that credit. No raw3Ch scene is cast as a logical
scene or replaced with a successful stub.

The larger scene reaches both native batch counts above50. The original
`B1D950` preparation-job branch therefore uses the actual `4C1130` frame owner,
`B0FFB0` preparation descriptor, `BE3020` enqueue and `BE3150` dispatch. Its one
canonical publication/scope set borrows the startup's existing RandomThreads and
the same queue preparation/deletion domains. Native worker count, scheduling,
synchronization mode and manager-owned stop/join are preserved. There is no
serial replacement. See [job binding](OCT24_RENDER_D_FRAME_JOBS.md),
[device source](OCT24_RENDER_D_DEVICES.md), and
[matrix lifetime](OCT24_RENDER_D_BONE_OWNER.md).

Initial probes are retained separately: `d2_probe_usn04.log` stopped at a strict
raw-versus-instance model interface mismatch before drawing; that binding was
corrected. `d2_profile_usn04.log` loaded all models (1790 drawables, 3719 unique
asset sections), then reached batches3714/115 and stopped at the old serial-only
guard. These failed runs are not census or visual proof. The subsequent native
job binding and final runtime evidence are recorded below.

The shared GUI submission API now reports `label_entered_native`. It is false
through all prefix checks and becomes true immediately before `B1F4D0` assumes
ownership, so the GUI borrower can release its label on a pre-entry failure.
Optional group profiles bind before the first submission, through the same
render access and canonical camera/model/matrix domains.
