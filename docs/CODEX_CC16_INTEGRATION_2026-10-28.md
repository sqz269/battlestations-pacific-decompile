# CC16 integration review, 2026-10-28

Addresses: 00737F30 (Application shutdown), 007379A0 (Application retirement).

The user transferred the coordinator handoff in
`local/CODEX_HANDOFF_cc16_2026-10-28.md` and authorized linking/integration.
Review began with a fresh fetch of `origin/main`, at `67aad2827`.
The clean `agent/cc16-progress` worktree was fast-forwarded to that baseline.
The main checkout remains at its existing old revision with unrelated user edits.
Publication uses the handoff's `HEAD:main` workflow from the coordinator worktree.

## Selected batch

- Review and integrate `d7d0820a8`, GUI A1 startup scene domain, with
  `kNativeRendererStartupCc15Bound` still OFF.
- Correct the milestone-D shader bracket lifetime at 00737F30 before the borrowed
  startup Application's preload rows are released. The existing Application
  switch is already ON; this correction needs an actual milestone-D shutdown run.
- Include the A1 singleton/registry construction and teardown table, retaining
  provisional type-ID and future GUI traversal limitations.

This batch does not enable the GUI startup domain or change WINS timings.
ON acceptance still requires the current exact build's 13 rows, strictly
alternating USN21 4+4, ordered dialog/message/Lua comparison, teardown and startup
frame/first-RNG evidence. Historical A1 receipts do not establish all those gates
for the final domain-only fallback.

## Branch triage

- PC24 wave 1 is largely already enabled on main. The files on
  `agent/cc13-pc24w1-sweep1-5` (`b741b72b0`, preceded by `0d2c0c946`) are identical
  to already integrated `0bb88bcd9`; merging the stale branch adds no source work.
- `agent/cc13-trivialslots` (`556ade92a`) has ten files identical to main's
  `0fc1a0be8`. `agent/cc13-torpyaw` (`eee28b487`) and the stub census
  (`282e6e800`) also have patch-equivalent integrated content.
- `agent/cc13-aimfreezekz` (`4d0350e9e`) contains a new OFF packet, with static
  and build evidence but no 13-row OFF identity gate. Preserve it pending those
  gates. Its base freeze dependency (`a8046b1ef`) is on main with switches OFF.
- The dirty `cc13-aimfreeze3` enum conversion is unfinished; several old worktrees
  also contain staged edits/deletions. No uncommitted work was imported or cleaned.
- `agent/cc9-physics1` has the intentionally behavior-changing
  `kMovetoTargetSpeedClassCc13Bound` flip (`a8f4a897c`) and census (`854192d76`).
  The flip has moved/failing rows and lacks
  complete acceptance. S11 experimental receipts have both future switches ON,
  so their differences cannot be attributed to the OFF census alone. Neither
  commit is accepted by this batch.
- Current WINS work still has unresolved JM08, BSM03, USN15, BSM01 and BSM06
  results. Retiming is a separate coordinated wave, not a prerequisite that this
  OFF GUI batch can claim complete.

## Verification receipts

Baseline: `67aad2827`; `./scripts/build.ps1` passed the Win32 Release build and
both existing suites (`reconstructed_math`, `tool_tests`). Baseline executable:
SHA-256 `93C811EE06356F25B41018EEC6DCDBED3A1FE457BDFA419CDFD5C9807BFF3132`.

Integrated source candidate: `381792703`. Its Win32 Release build and both existing
suites passed. Candidate executable SHA-256:
`1A30FA1D6C67B51BEF537C69AF3179DC8454A0C4DDF608229F97DEB1D0C80BDB`.
`pc24_lint.py` reported zero findings across the five changed implementation files.

All 13 default-environment pairs passed: both exits 0, `pair_diff` exit 0,
ordered dialog/message/Lua differences 0, teardown differences and length delta 0,
mission start frame 36 on both sides, and identical first logged RNG draws.

| Mission | First RNG draw, both builds |
|---|---|
| ESMP08 | 0.4933 |
| JM06 | 0.1578 |
| USN21 | 0.6207 |
| IJN18 | 0.4640 |
| BSM01 | 1 |
| USN12 | 0.1307 |
| LOMP10 | 0.1119 |
| BSM09 | 0.0949 |
| PRCP01 | 0.3605 |
| USN01 | 0.04169 |
| USN02 | 1 |
| LOMP06 | 1 |
| USN04 | 0.4143 |

Milestone-D USN04 (`BSP_NATIVE_MISSION_SHIP/FRAME/UNITS=1`) ran all 1600 mission
frames and exited 0. The clean candidate manifest and executable hash agree.
`local/codex_cc16_candidate_usn04_msd.log` records 179 preload rows released
(30133), Application publication cleared (30142), lighting retired with empty
registry (30513), mission graph closed at frame 1600 (30527), renderer closed
with its worker joined (30530), and final COM release (32110). Each closure occurs
once. The remaining texture reference is the documented `error.dds` allowance.
There is no startup failure or unexpected partial-state retention. The bracket's
earlier release is established by the guarded source path and successful row
retirement; it has no separate bracket-release log line. Late-plane drawing
remains unproven by this diagnostic view.

The local runner
`local/codex_cc16_gate.ps1` reads the existing A1 13-row frame/plan/bounds matrix,
uses symmetric bounds aids on the designated rows, enables death/RNG streams,
and launches through `tools/run_game.ps1` with eight slots at Normal priority.
Six comparison lanes were used initially; the first serial baseline launch was
stopped and replaced by the partitioned matrix, and is not an acceptance receipt.
The runner never writes the
installation's save/profile files. Default runs use copied executables in
`local/codex_cc16_base` and `local/codex_cc16_candidate`, with per-run manifests.
The baseline was built and copied before the merge. Launcher checkout metadata
can reflect the later launching checkout; the executable SHA and
`local/codex_cc16_build_receipts.json` identify the actual saved baseline.

`local/codex_cc16_eval.py` reuses `pair_diff.py` and the existing ordered-subsequence,
teardown and startup-RNG rules. Ordered comparison masks only the existing
pointer and worker-thread-ID fields from `gui4_subdiff.py`; no new noise mask was
added. Per-row evidence is `local/codex_cc16_pair_<row>.json`, `.txt` and
`.ordered.txt`; the final `local/codex_cc16_gate_summary.json` accepts 13/13.

Two previous GUI milestone-D runs and the historical inaccessible process 40812
were present at the start of the review and were preserved. Four retained GUI
processes correspond to two bootstrap/child pairs. No new ON flip is accepted
from those incomplete runs.
