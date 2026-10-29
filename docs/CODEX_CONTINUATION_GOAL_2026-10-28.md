# CC16 continuation goal

User authorization: 2026-10-28, "can you continue? treat this as a goal".
The active goal continues native linking and integration from the CC16 handoff,
with reviewed evidence and publication of accepted batches to `origin/main`.
An isolated OFF landing is progress, not completion of the continuation.

## Baseline and ownership

Accepted main at resumption: `5ca4639f0`, source candidate `381792703`.
The coordinator remains in `agent/cc16-progress`; the dirty old main checkout
is preserved. `bsp.py brief` compares against that stale local main and therefore
overstates unmerged patches; fetch and compare against `origin/main` before
dispatch or publication.

Workers reuse existing streams/worktrees and acquire explicit file/address
leases. Only the primary integrator publishes accepted batches to main.

| Packet | Worker / worktree | Owned source | Initial state |
|---|---|---|---|
| A1 startup domain ON | gameplay integration reviewer / cc9 | renderer application header only | clean candidate `634258460`, build and gates in progress |
| P3a2 Screen/root native providers OFF | GUI reviewer / cc15-gui4 | GUI page application CPP/HPP | bounded implementation, no startup-header or game-host edits |
| Plane reference-freeze OFF | aimfreeze worker / cc13-aimfreezekz | game_hosts_units.cpp and approach_ref_freeze_planes header | review and fresh OFF gates for `4d0350e9e` |

Each worker owns its separate evidence document. Changes to shared host files or
another packet's API require a refreshed contract/lease before implementation.
Vendor-library bodies and readiness remain outside the reconstruction scope.

## Gate schedule

- A1 begins with short reference USN04 and milestone-D USN04 for 1600+ mission
  frames, both exiting 0. Its startup switch is the only switch changed.
- A1 then compares 13 ON runs with the saved accepted OFF executable/logs.
  The plane packet compares 13 fresh OFF runs against that same accepted code.
  Baseline executable SHA-256:
  `1A30FA1D6C67B51BEF537C69AF3179DC8454A0C4DDF608229F97DEB1D0C80BDB`.
- Each matrix admits at most three simultaneous runs through `run_game.ps1`
  with eight slots; the two old GUI run pairs remain present. Every run stays
  at Normal priority. No power changes, hardware-probe commits or installation
  save/profile writes are authorized by this work.
- Pair acceptance includes gameplay, ordered dialog/message/Lua, teardown,
  mission-start frame and first logged RNG draw. Use only existing pointer,
  worker-ID and documented `pair_diff` noise masks. Bounds initialization is a
  symmetric aid on designated pairs, never a default or WINS retime.
- A1 ON additionally needs a serial, strictly alternating OFF/ON 4+4 USN21
  series. All four OFF runs must agree and every ON must match. Reserve a
  light-load window after other game matrices and heavy builds finish.
  Include a separate no-bounds-aid OFF/ON pair.
- P3a2 is kept OFF, built and reviewed first. Its runtime gate is scheduled
  against the next accepted baseline; a switch with no production entry does
  not gain native runtime or GUI playback credit from OFF identity.

Prior root receipts are immutable. New candidates keep separate copied binaries,
hashes, clean source commits and run manifests. Expected startup-domain notes
and provisional type-ID movement must be explained, not hidden by broad masks.

## Continuation order

The GUI sequence remains P3a1 (accepted OFF), A1 (accepted OFF, activation now
gated), P3a2, A2 frontend GUI frame, P3b Text, P3c Icon, then P4 sole ownership.
The final page activation is one switches-only flip after the full chain and
P4 pass together. Startup shader preload and the faithful actual-lighting
instance upload are separate dependency packets, not part of the A1 flip.

Pending historical cc9/cc13 branches remain parked according to
`CODEX_CC16_INTEGRATION_2026-10-28.md`. Uncommitted enum migrations, staged
deletions and ownerless old worktrees are not collected or cleaned.

Current blockers and accepted packet receipts will be appended as the goal
advances. Do not mark the goal complete because a worker finishes or a single
switch lands; continue to the next ready bounded packet and integrate promptly.

## First dependency review

P3a2's full AAA710 call needs distinct widget white/black guards/quartets and
E12FC4 Y-scale; they cannot alias Text's F8BE38 state. Installed PE/Ghidra recovery
established loader-zero widget cells and E12FC4 bits `3F400000` (0.75), with lazy
initialization left in the existing raw property body. Worker state commit is
`590cc866d`, following manager commit `f24b3e21d`.

The manager binding is a once-only borrowed token; unbound page callbacks retain
the existing rejection. Token release requires completed shared-drain
acknowledgment. The coordinator therefore moves that acknowledgment before
page-app reset, while manager destruction stays after page-app reset. The root
manager/ordering build and both existing suites pass. This does not claim that real page
callbacks or the future token borrower have executed.

## First accepted dependency batch

Integrated source `577516563` combines manager `f24b3e21d`, recovered state
`590cc866d` and the coordinator's acknowledgment order. Its Win32 Release build
and both existing suites pass. Saved executable SHA-256 is
`FADF096DF31A59ED556F033994358E4DDEC636C717042FEE431A0898D0339C38`;
the reference XLive DLL remains byte-identical to the accepted baseline.

Reference USN04 (4800 mission frames) and milestone-D USN04 (1600 frames) both
exit 0 and compare with the saved accepted code at `pair_diff` exit 0. Ordered
dialog/message/Lua differences are 0 (762/762 reference, 626/626 milestone-D).
Teardown is identical (248/248 and 4312/4312 lines). Both start at frame 36;
first RNG draws are unchanged (0.4143 reference, 1 milestone-D). Milestone-D
releases 179 preload rows and closes the mission at frame 1600 before final COM
release. Receipts are `local/codex_gui_dependencies_{ref,msd}*` and
`local/codex_gui_dependencies_ordered.json`.

These are dormant dependency APIs, not a switch activation or P3a2 runtime
proof. The real page chain remains OFF. Native page callbacks, bound-token
lifecycle and widget-property execution will be exercised with their borrower.

P3a2 can borrow actual graphics/stream services from the renderer's grid
context and compose a page-owned Model companion domain. Its next dependency
is a genuine raw Lua field resolver: CURRENT source callback signatures add
explicit contexts, whereas BD63B0 expects ECX/EAX fastcall cells. The worker is
implementing a qualified adapter over current canonical slots and actual raw
Lua storage, with nested activation and retained failure frames; no signature
cast or null placeholder is admitted.

## Second accepted batch and common-code freeze

Plane OFF source `93b9a5517`, final evidence `f8a97eb8c`, passes all 13 identity
rows with original masks, Normal priority, exact manifests and no retries.
Ordered events, teardown, start36 and first draws match. Its separate fresh
Win32 build passed three existing suites and lint. The XLive DLL differs from
the preserved reference in six timestamp bytes only; normalizing the two PE
timestamp fields makes both files byte-identical. All new/base freeze flags stay OFF.

Raw resolver source `76fb1eb96` passes Win32 build/two existing suites plus one
focused actual-storage Lua fixture: ECX/EAX bits, CURRENT callback mutation,
nested normal/C++ restoration, real Ptr userdata lookup, balanced tracking/stack,
and completed-frame reuse. The pointer result is captured locally before temporary
cleanup and returned from that captured value. Its adapter remains uncalled;
Original code/nonlocal unwind/game equivalence are not claimed.

The page constructor now retains startup-owned resolver/capability references
through the existing accessors; the latter is a Source CPU capability, not an
Original CRT selector alias. The page and property-composition switches remain OFF.

The shared GUI state also supplies a distinct one-byte readable empty-string view
for F8BC60. Supported PE loader-zero and live Ghidra bytes agree. The bounded
DATA/address references are AA3360/AA334E (find camera store), AA5137/AA5132 (create),
AA4BD5/AA4BC3 (remove), AA2498 (widget-key null-header fallback), and AA2678/AA266D
(texture/atlas resolution). No initializer/store was observed in that census.
No array extent, Original-VA publication or mutable alias is inferred.

Completed startup scene frames now reuse their prepared slot only after outer
completion; incomplete and not-yet-entered slots remain reserved. Heap registry
ownership has left a successful acquisition before reuse. The 256 reservation,
interfaces and native ownership calls are unchanged. These providers are not
entered by the current A1-only production path; direct allocator and GUI/reload
runtime proof remains open. See `CODEX_GUI_SCENE_FRAME_REUSE.md`.

Frozen common OFF source: `6216ea1af`, executable SHA-256
`5461327DE616FEF8485E49A86DCE17901399092B62ACC4621A0A0C4DA06FC926`.
Matching XLive is the byte-identical accepted baseline DLL. The combined build
and both existing suites pass. Common-code reference USN04 and milestone-D1600
exit0, pair_diff0, ordered differences0, teardown differences/length delta0,
start36 and identical first RNG (0.4143 / 1 respectively). Artifacts are under
`local/codex_common_final_off`, with raw logs/comparisons/ordered receipts sharing
that prefix. This snapshot is immutable for A1's latest-common-code comparison.

A1 candidate `634258460` completed its 13-row matrix against source5ca4639f0:
all gameplay/native/summary/ordered/start/RNG/teardown contracts pass. Raw
pair_diff1 is retained for exactly the documented startup note and provisional
type-ID transition19/18 to21/20. Evidence commit is `8330382ab`. It remains
unpublished until a matching common-code ON build passes strict serial USN21
OFF/ON four-plus-four (all OFF self-agree) and the separate no-bounds-aid pair.
Other gameplay runs/heavy builds are held during that final acceptance window.

## Matched activation pair

The A1 gate worker merged the frozen common code into its candidate at clean
`6c8016fa756932161f09d4ccc5617ecd8adf8e51`. Its source/include diff against
`6216ea1af` is only `kNativeRendererStartupCc15Bound: false -> true`.
Win32 Release and both existing suites pass; lint reports zero findings.
The common ON executable SHA-256 is
`799F5FD80A1C35FBB002DE0504A1D6B608044AF14F30C4B124667448B38F3972`.
Both sides use XLive SHA-256
`DDF2C2A86D96F37819147C38991A74EF5C5946CE625E7B4E7914F8382AA13F87`.
The common pair is copied under the cc9 worktree's
`local/codex_a1_activation`; earlier634/5ca inputs remain immutable.

Common ON reference/milestone-D smoke checks precede the approved strict
serial alternating eight USN21 runs and the separate no-aid pair. Root holds
other game/heavy-build work. Any first OFF-self disagreement or unexpected ON
delta stops the series for review, without hidden retries. A1 remains OFF on
main until those results are accepted.

The common ON reference and milestone-D1600 checks have now passed against
matched OFF546: exactly the documented A1 records, ordered differences0,
teardown differences0, frame36 and matching first RNG. The strict serial series
has started. No other game or heavy build is scheduled during it; P3a2 work is
static and remains OFF/unbuilt pending an explicit build window.

## Reviewed dormant Page checkpoint

The first matched strict OFF/ON pair passed its exact documented A1 contract:
raw pair_diff retains the three inventoried startup records, all remaining
comparison fields match, ordered events and teardown match, and startup frame36
and RNG0.6207 match. OFF2 then started serially after provenance checks passed.
The remaining strict repetitions and separate no-bounds-aid pair are pending;
there has been no retry or acceptance of A1 on main.

GUI checkpoint `c5594a19a` (including earlier raw Group `9f8df5d54`) is frozen in
the cc15-gui4 worktree. Root constructor review and independent ownership review
found no source blocker in the complete candidate-vs-common-code diff. It admits
only the genuine childless/type0-key Screen path, and mapped widget keys reject
before AA6560. Private context views retain the canonical registry, pools,
imports and owner families; Page metadata survives native pool return until the
actual Group/outer/camera/resource/sun/ambient companions retire. The review
caught a doc-only count error, corrected in `066a46466` with unchanged source:
AC6825's null B6D890 tail unlinks root
registration but leaves170; the ordinary cold resource count remains4 until
genuine Group B6EE10 removal, absent replacement callbacks or other holders.

This checkpoint is OFF, unbuilt and unmerged. Compilation/CTests and OFF
inertness checks follow the strict A1 window. A focused childless lifecycle
fixture must establish the dormant ownership path independently; OFF parity
cannot do so. Actual FE_initial children still need P3b/P3c and a same-tree raw
Model/Group parenting/lifetime bridge, followed by the joint P4 transfer from
the current GUI bridge. Foreign owners, unsupported lifetime families, repeated
activation and failure/pool-reuse behavior remain explicit open checks.

Tooling-only `b63e0311e` fixes brief's false integration backlog by preferring
the published `origin/main` ref, with local `main` fallback. Two focused Python
tests pass; no C++ or frozen game artifacts changed. The main checkout remains
untouched, and CC16 is fully published at that checkpoint.

The P3b/P3c source map confirms that raw Text and Icon construction/reference/
retirement providers already exist, while their active applications retain
semantic field authority. They cannot be overlaid on the complete raw payloads.
Installed FE_initial has two active Icons and one nonempty Text; its installed
Lua is customized (SHA-256
`3BAB3FC8A29569D0F7DA13CC3C69D3B1232C57772CED58A2E06985700F332CFA`).
Full raw Text content/glyph publication, Icon construction/properties/rebuild,
shared Model/Group parenting/lifetime closure and startup resource order remain.

One bounded independent metadata dependency is dispatched as
`codex-gui-icon-delayed-custody`: a deferred process owner for the single typed
F8BD98 Source backing and existing genuine delayed-load service. Even ordinary
raw Icon destruction calls AB5700 when delayed loading is false. The worker
owns only new process CPP/HPP and its evidence document; root owns the shared
startup/deletion hookup. No getter or raw Icon caller is activated, no queue or
file delivery is fabricated, and no build is allowed during the A1 hold.
Any eventual getter must use the SAME service bound into the process singleton
deletion dispatcher and retain its full provider graph through native drain.

The Icon process dependency is now frozen at `82c09a739` in cc15-gui4. Root
reviewed all three new files: one private null publication, successful-only
preparation over the exact borrowed raw context, and status separating normal
registered-owner retention from incomplete/failed operations. No source blocker
was found. It remains unbuilt, unmerged, unregistered in CMake and uncalled;
root's same-service deletion hookup and later lifecycle proof are still pending.

The raw Model audit found that existing raw parenting kernels already cover
the native hierarchy operations. The generic child suffix still transports
logical Model/scene bindings, so sharing actual counts or raw names cannot make
it compatible with the Page's actual3Ch resource family. A bounded library-only
packet, `codex-gui-raw-model-storage`, now prepares genuine B75030/B750C0/
B75290/B74F20 storage/lifetime/reference providers in new files. It must retain
the single normal decrement epoch, fresh member-unwind epoch, current flags
after destruction and same real pool/node/import/actual-owner domains. This
does not publish a child or make the shared Model/tree/scene transport ready.

A separate read-only A2 audit source-proved genuine input metadata preparation
immediately after shadow success, before73DD12. It retains the same input
contexts and empty SDK trackers, while BOTH the input_runtime_ facade pointer
and actual backend startup remain at73DD8E. It requires the already-live sound,
XLive, clock and platform domain, so it cannot move before73D8CC with factories.
The null-backend cursor guard is genuine; the preceding load pump still runs
XLive and window message callbacks. Full preload execution/failure containment,
factory ownership/retirement and resource order remain separate obligations.
No A2 source change or live trial is accepted.

Raw Model packet `48195196e` is committed in its own
`agent/codex-gui-raw-model-storage` worktree. Root reviewed all three new files
and independently compared the four current assembly exports and existing
state1/state0 unwind evidence. No source blocker was found: normal/member
decrement epochs, callback replacement reloads, late low-byte flags read, same
actual+4/canonical registry and live pool provenance match the qualified
contract. Retired metadata after failure is explicitly not cleanup/pool-return
proof. The packet remains unbuilt, unmerged and uncalled, with CMake/testing and
raw child/scene/tree transport still pending. It adds no body credit.

The follow-on routing audit found no missing native Model parenting/scene/tree
body for the default family: D62DE8 current1C is B6D7B0, current18 B6F310,
current50/54 B6ED80/B6EE10, with real006EF860 type queries and 481's zero
terminal. The shared router/retained record bindings beside Group/camera/sun
remain primary work; one mutable invocation frame cannot support callback
reentry. The one additional missing raw storage entry, B75170 geometry
association, is dispatched in new files as `codex-gui-raw-model-geometry` over
the still-unbuilt 481 source. Its incomplete prerequisite is recorded explicitly.

The Text audit confirms AB9650 always constructs an internal188 shadow Model,
mesh/section and GuiDefault effect graph. A default-only wrapper over the old
Model transport would need replacement before the intended Page integration,
so that wrapper is not dispatched. The useful next boundary is a coherent raw
Model variant through Text sections85, constructor93, retirement118 and
reference122. It must preserve Text188 publication before canonical metadata
admission, actual counts/creator releases, same point-light/node domains and
retained raw Model/frame/reference regions. Existing interfaces fix the legacy
Model family; casts cannot connect 481. Genuine GuiDefault/cache/Lua readiness
and general primary4C/Page retirement remain separate prerequisites. API design
is read-only pending primary review; no Text transport edit is accepted yet.

## Final matched A1 gates and next source batch

The frozen common A1 pair completed all strict four-OFF/four-ON runs. Root read
`common_strict_evaluation.json`: eight valid manifests, exact serial provenance,
all seven row contracts/receipts/remainders passing, OFF self-comparisons pair0,
ON comparisons pair1 only for the exact inventoried A1 records, and ordered0.
All four OFF runs agree; startup frame36/RNG0.6207 and teardown match. The
separate no-aid pair also completed successfully on the same binaries, with the
bounds variable absent and the exact reference trace environment. No failures,
retries or masks changed. Final receipt/doc review precedes A1 publication; the
runtime hold is lifted after these scheduled games.

Geometry packet `d403f5411` is frozen above481. Root reviewed all new source and
the current B75170 assembly: no source blocker, still unbuilt. Its one-shot
frame borrows the four actual DWORD operands; publication/import epochs,
callback replacement, late scalar reads, one sentinel sample and the authored
SSE unordered/store sequence match the qualified Source contract. Compile and
emitted-code/FP evidence remain pending.

Primary approved the concrete Text raw-Model API and dispatched
`codex-gui-text-raw-model-transport` across the eight Text files plus a new
evidence document. It adds explicit raw85/93/118 overloads and a separate typed
raw122 reference, preserving every legacy interface/caller. Model481 and
geometryd403 remain named unaccepted prerequisites. Text188 publication stays
before canonical admission, and host_metadata failure must propagate distinctly
from native constructor cleanup/slot return. Stable raw Model, argument, frame,
reference and disposal regions remain retained. No factory, properties,
nonempty content, Page activation or function-body credit is implied. The
private default-only Model router must use genuine same-domain providers;
general shared Page/primary4C routing remains later work.

A1 evidence branch `010be6972` is now accepted by the integrator and merged.
Root checked the strict aggregate and separate no-aid validator/receipt, found
no remaining scheduled game, and preserved the GUI Text/Element Known Limits
correction. Integrated src/include and CMake sources match the tested `6c8016fa7`
snapshot. The A1 switch is ON; Page construction/drawing, shader preload and
allocator/reload paths remain outside its accepted scope. The reviewed dormant
source dependencies now enter their separate Win32 build/inertness batch.

## Dormant dependency batch accepted

Integrator source `eb18518db8953f215dbf4fa5384d76d555906546` merges frozen
Page c559/066, Icon82, raw Model481 and geometryd403. CMake registers both Model
TUs in bsp_core and Icon process metadata in bsp_game. The initial compile
exposed a missing complete platform-publication header in Page CPP; the include
fix resolved its cascading aggregate errors. MSVC Win32 Release and both CTests
pass. Page chain stays OFF and all new library/service callers remain uncalled.

Immutable executable SHA-256 is
`05F362BB61DC3618D6CB560A0F3F3AEB332E720282CA726F0D62B04A89D2A0D2`,
with the same accepted DDF2 XLive. Root's snapshot/build receipt lives in
`local/codex_gui_dependencies_snapshot`; emitted geometry object disassembly
confirms the authored SSE scalar sequence. No FP execution proof is inferred.

Fresh reference4800 and milestone-D1600 pairs against accepted A1 ON799 pass
exit0/rawpair0, ordered0, teardown0, frame36/RNG0.4143 and1 respectively. The
first baseline survived a worker interruption; root preserved it and continued
the three remaining scheduled runs. Its real slot1/core4 assignment was pinned
before the first candidate, temporarily reserving only a free slot0 without
replacing locks or stopping processes. Full argument/source/DLL/difficulty/env
audits and serial nonoverlap pass. No comparison or game was retried. Receipts:
`cc9/local/codex_gui_deps_inert_20261028`.

The five Model entries are recorded only as qualified fragments, with original
ABI, evidence and runtime limits. No whole-body credit grows. Actual Page,
Model, Icon-service, exception/reentry/reuse and raw Text lifecycles remain open.
Frozen Text e75 has passed root constructor and independent ownership review
within DEFAULT scope; its separate build and acceptance batch follows.

## Raw Text transport build accepted

Dormant dependency batch is published on main at fb071f275. Remote Dyn source
changes merged cleanly and the combined Win32 build passed both existing CTests;
the frozen eb185 gameplay results above retain their exact original provenance.

Frozen Text e75 is integrated at 93a52f36d. Both source reviews passed and the
Win32 Release build passed reconstructed_math and tool_tests. The existing
legacy Text APIs/callers are preserved. Raw85/93/118 and separate raw122 now
compile with the actual Model481/geometryd403 transport. No new test cases or
body credit were added. The private genuine Model router, real GuiDefault/cache/
Lua readiness, DEFAULT factory binding and actual lifecycle/exception/reuse
proof remain explicit next dependencies. Page activation remains OFF.

Workers are preparing the Model-only router, raw DEFAULT factory contract and
early shared procedural-factory lifetime needed for A2. Shared host changes
remain with the primary integrator; no second factory domain is authorized.

Input metadata preparation now precedes the73DD12 position, while the sole
input_runtime_ publication and native backend startup stay at73DD8E. The real
existing clock/XInput/platform/sound graph supplies the same borrowed core to a
future sampler. Win32 Release and both existing CTests pass; no input/gameplay
or shader-loading runtime proof is claimed. See CODEX_A2_INPUT_METADATA.md.

A2 OFF candidate source now binds one early host-owned procedural factory
domain, retained startup sampler/effect/73BF80 preload and borrowed mission
views. Independent review found a stale mission borrow after host owner reset;
both closure paths now detach it. Quiescent retirement precedes native drain,
full renderer closure precedes host acknowledgment, and entered failures keep
the whole graph through immediate process exit. Frozen builds/gates follow;
this source composition does not activate A2. The genuine private Model-only
router e769 is separately merged/registered and Win32 build-tested, still
uncalled with actual DEFAULT lifecycle proof pending.

## A2 startup activation accepted

Corrected ONdfa82aafb and OFF51e6199da differ only in the A2 bool across all
C++/include/CMake sources. Both builds pass the two existing CTests. Corrected
smokes and all four serial reference4800/native-MSD1600 runs exit0; gameplay,
mission ordered traces, startup36 and first RNG match. Root reviewed the frozen
independent predicates, every raw-difference category and ordered edit inventory.
The original failed334 smoke and its distinct artifact remain preserved.

A2 now runs the genuine early BBCB40 factory setup and real73BF80 preload/cache
bracket once, borrowing the canonical Application rows and sharing the same
owners with mission rendering. The switch is ON. Native Page stays OFF. Ref
rawpair1 has only the inventoried startup/cleanup rows and Lua getter diagnostic;
MSD rawpair0 and actual shutdown400/400diff0. The old rawteardown70 is preserved
and explained by one preload row moving to startup.

MSD retains exactly the unchanged accepted error.dds/profileD61948/ref1 residue
with one skipped release/one entry and final COMdevice1/api1. The generic raw
COM-zero failure remains visible; a separate exact Source/baseline qualification
supports this bounded activation. Reference/smoke COM0/0. See
CODEX_A2_STARTUP_PRELOAD.md and the gate directory's frozen evaluation.
This does not close full GUI/visual/interaction, raw Text/Icon/Model/Page
lifecycles, queued-callback failure, allocator/reload or Original ABI proof.

Private Model node/scene/type provider2049 and DEFAULT AA1380 helperba2 now
pass root/independent source review and the integratedcb6ba1195 Win32 build,
including both existing CTests. Both TUs are compiled in bsp_core, with no
production caller, new native body credit or actual DEFAULT lifecycle proof.
The next retained-owner composition must connect the real renderer services and
raw-compatible widget lifetime dispatch; legacy Text-reference dispatch cannot
stand in for the new raw122 companion. Actual creation belongs after genuine
Text type startup in a legitimate cache bracket, with retained frames/providers
through complete renderer drain, failure and surviving Model credits/flags0.

Owner plan9c54471 is reviewed and integrated as a plan. The frame worker owns
new retained-owner files, and the GUI worker owns the separate private DEFAULT
widget-dispatch API. Shared hook review identifies the existing GUI Text opt-in,
early idle metadata preparation, later genuine mission cache callback, retirement
before Font drain and renderer-first metadata closure. No new public test-only
flag is needed. The earlier suspected sampler admission blocker was corrected
against exact source: its gate is layout-only; full-retention metadata/profile
borrows still happen early. Implementation and one genuine lifecycle gate remain
open; A2 stays accepted ON and no full Page/GUI activation follows.

Owner wiring exposed a concrete raw Text admission overconstraint: real VFS
raw/typed wrappers are different objects over the same three canonical native
string cells as the permanent process wrappers. A focused raw-only correction
now checks every cell address and the typed actual-manager mode, retaining
local object identities, legacy guards and all provider lifetimes. No native
allocation/get/free body or class layout/vtable changed. Root and independent
review pass; frozenef69872ae Win32 Release and both existing CTests pass. The
subsequent genuine owner lifecycle remains separate.

The private DEFAULT widget bindingb6b016e68 now passes root/independent source
review and integrateddf7fe8eb0 Win32 build with both existing CTests. It uses
genuine class predicates/list removal and precise constructor/base-prefix custody,
with explicit rejection of unsupported child/timed/scene paths. No raw188 read,
legacy cast, new body credit or lifecycle runtime acceptance is introduced.
Retained-owner0fcd1633 is frozen and undergoing source review before integration.

## One genuine raw DEFAULT lifecycle accepted

Owner0fcd1633 passes both source reviews and Win32 build after adding two
complete-type headers. Shared hooks prepare pure metadata after Text type
startup, create inside the existing real cache callback, retire before Font/
parent drain and verify renderer closure before dropping facade metadata.
The private qualification route uses existing GUI_TEXT and stays OFF in main.

Frozenc0ef2def1/CE52 same-binary GUI_TEXT0/1 pair passes on slot4/core10;
both exit0, requested mission181 and599+1 frames. ON has exactly one nonnull
factory/raw93 completion+raw122 bind, settled actual Text/Model scalar/pool/
late-flags retirement and post-renderer owner closure. Gameplay and ordered
full449/mission299 rows match, start36/RNG1 same. Raw pair1 has only four added
owner-stage rows; raw teardown97 is exactly two inserts, no prior row changed.
Exact existing error.dds/profileD61948/ref1 and COM1+1 remain baseline-qualified
on both sides, with raw failures visible and no new masks. The initial busy-lane
attempt launched no game; the revised fixed-lane harness preserves all lock/
tagged-process/duplicate-log checks and does not stop other processes.

Root reviewed the independent predicates and copied report hashes to the local
gate directory. Route restored OFF atf1bfc515f and existing tests2/2 pass. This
is one real-provider DEFAULT lifecycle fixture; raw content/rendering/Page,
failure, flags0, external credits, reentry, repeated reuse and native ABI remain
open. Next Text transport surface is being audited; Icon host process binding
plan9ce9320 is reviewed as the next independent shared-host hookup.

## Icon process metadata linked

The accepted DEFAULT batch and combined main build are published atf26a111d0.
Icon host candidatec805780d4 then connects the existing permanent process
service after VFS phase2, validates every canonical string/manager cell alias,
and installs only the same existing deletion arm. Central failure guards and
both full-drain branches retain providers on incomplete work; idle/null checks
precede factory facade release and retain binding through fallback destruction.
Independent source review and Win32 build/tests2/2 pass. One frozen60-frame
slot4/core10 idle smoke exits0, validates initial and repeated drain closure,
and ends COM0+0. Detailed receipt/scope:
`docs/CODEX_GUI_ICON_PROCESS_HOST_LINK.md`.

No lazy Icon allocation/registration/retirement or Page activation follows.
Raw Text main geometry and phase-aware nonempty retirement remain the next
disjoint transport/dependency packets; full rendering and native ABI stay open.

## One genuine delayed-Icon singleton lifecycle qualified

Private frozen8849426fb/601BB7CF fixture passes one60-frame slot4/core10 case:
AB5700 actual allocation/publication/registration followed by the same actual
manager-popped D5C4B8/AB57C0 flags1 retirement. Source-enforced independent
metadata checks precede the two records, without post-free payload reads or
manual cleanup. Full parent closure and repeated drain checks pass, exit0 and
COM0+0. Independent receipt/hash/runtime audit accepts this EMPTY singleton
lifecycle only. Raw comparison to the separate idle smoke preserves slots46→47
plus two records; it is not paired gameplay/activation evidence. Private route
restoredOFF677c3024f and Win32/tests2/2 pass. Detailed scope/report hashes:
`docs/CODEX_GUI_ICON_SINGLETON_LIFECYCLE_QUALIFICATION.md`.

Main geometry AB7700 transport is under disjoint source implementation.
Independent retirement review tightens the next packet: header cleanup needs
separate returned progress during unwind; consumed actions alone are not
completion. Owned-member/cached-effect admission cannot expand before genuine
raw acquisition/publication receipts exist. Font borrower plan likewise requires
all-user terminal custody and explicit post-parent-drain unlink; DEFAULT and
AB7700 remain Font-independent. Full P4 activation stays OFF.

Cleanup diagnostics276880ca9 now build with both existing suites passing. The
independent unwind-returned mask is set only after each real helper returns;
both fresh predicates reject stale new progress. Native consumption/call/EH
order and every DEFAULT ownership guard stay unchanged. No new runtime proof.
Widget Model lifetime plana1c2 is dispatched as a disjoint typed shared-schedule
transport; raw main-geometry379 is frozen for independent source review/build.

Raw main geometry379 now passes both source reviews, Win32 build/tests2/2 and
emitted-object inspection (0805D283). One x87 sentinel load/FST/FSTP operand
schedule and SSE zeroXYZ are present; original capture/import/transform epochs
remain explicit. The entry is absent from the game map and has no caller.
It supplies no primary creation/publication, main-section/retirement, Font or
Page activation and no runtime/FP/ABI proof. Concrete primary producer and shader
credit-receipt linkage are under disjoint bounded audits; all ownership gates
remain in force.

Reviewed producer plans a281/1719 are integrated and dispatched in disjoint files:
primary4C allocation/constructor/bind/publication PREFIX with real constructor-
origin credit, and raw AB8CE0 overload with immutable prebound concrete receipt
backing. Neither is an ownership-admission/production-caller authorization.
Baseline widget/shader objects from accepted4ace89234 are retained under
`local/cc16_raw_transport_baseline_objects/` for focused static comparison of
the shared-schedule refactors. This adds no runtime or binary ABI evidence.

Raw widget transport2ee now passes two source reviews and corrected Win32
build/tests2/2. It shares legacy native schedules; concrete new raw routing is
active-base-only and all raw escapes retain without inferred exception cleanup.
Compiler noreturn annotations alone were removed after a preserved /WX warning;
the rejection bodies still throw. One existing empty DEFAULT replay35a4e92f5
independently validates the shared LEGACY path: gameplay/ordered449+299/start36/
RNG1 same, exact accepted Icon diagnostics and baseline error.dds/COM1+1 only.
Private route restoredOFFb95a64a57/build2/2. New raw primary helper remains
uncalled and gains no runtime/ABI/Page credit. See widget evidence doc.

Shader producer transportd13 now passes both source reviews/all5 comparisons and
Win32/build2/2. Its immutable prebound Frame/context enables concrete raw receipt
backing without changing DEFAULT ownership; both ordered SSE helpers are exactly
39-byte-identical in saved before/after objects (static evidence only). No raw
caller/Font export/nonempty retirement is enabled. Primary producerdb55 is frozen
with peer review passed and root review ongoing; all its escapes retain without
inferred ctor-slot EH. Geometry readiness540 confirms genuine constructor74=0
permits a future no-content geometry lifecycle, but Text ownership/routing/full
custody bridges still block it. P4 remains OFF.

Primary producerdb55 now passes both source reviews and Win32/tests2/2. Root
verified actual canonical allocator thunk and publicationAAADC4/138clearAAADC9,
recorded precise diagnostic sites and registered the uncalled TU. No caller,
whole-child/AAA710, runtime or ABI credit follows. Plans eab/540 pin the next
receipt-authorized prefix-only retirement/owner bridge; they preserve DEFAULT
construction/binding and require actual settlement for BOTH Models plus Text.

The prefix retirement adapter and owner halves are dispatched with disjoint
files and an early shared typed-header contract. An independent audit checks the
real external raw-string name stage; no fabricated header or inferred name/EH
cleanup is allowed. Existing DEFAULT behavior remains and no host caller/gate is
added by these source packets. First scope excludes74, Font/content and parenting;
geometry compatibility540 is preparation for a later separate composition.
