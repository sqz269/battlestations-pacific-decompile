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
