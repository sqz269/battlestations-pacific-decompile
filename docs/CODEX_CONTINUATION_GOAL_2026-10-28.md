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
manager/ordering build and both existing suites pass; smoke gates are queued
after a matrix partition frees a slot. This does not claim that real page
callbacks or the future token borrower have executed.
