# A1 startup scene-domain activation evidence

## Immutable packet baseline

Accepted OFF source: `5ca4639f0b` (same code as `381792703`). ON source:
`634258460a6ea47e585fe015694f6d69ba331b2f`, clean before its build and runs.
The only source change sets `kNativeRendererStartupCc15Bound` true in
`include/bsp/game_native_renderer_application.hpp`. No other switch changes.
The domain-only fallback and accepted shutdown correction are the implementation
exercised here. This baseline matrix remains immutable proof; the final matched
common-code gates and integrator acceptance are recorded below.

| Executable | SHA-256 |
| --- | --- |
| Accepted OFF | `1A30FA1D6C67B51BEF537C69AF3179DC8454A0C4DDF608229F97DEB1D0C80BDB` |
| Candidate ON, source `634258460` | `0790B01E94ED395C4ECD178A8D26C0804F15F7EA975C12422E6D8560A7C1E163` |

Artifacts are under
`J:/PROG/battlestations-pacific-decompile-cc9/local/codex_a1_activation/`.
`off_exe/`, `off_logs/`, `off_preservation.json` and the copied coordinator build
receipts preserve the accepted OFF executable/dll and 13-row plus milestone-D
logs. `on_exe/` and `on_build_receipt.json` preserve the candidate and its source
state. Original coordinator receipts were not overwritten.

`scripts/build.ps1` passed the Win32 Release build and both existing CTest suites
(`reconstructed_math`, `tool_tests`). PC24 staged lint passed with zero findings
on the one changed source file. The detailed output is `build.log`.

## Exact comparison contract

All 13 ON runs exited 0. Their plans, frame counts and symmetric bounds aids
are the coordinator's accepted `local/codex_cc16_gate.ps1` matrix, sourced from
`cc15-gui4/local/gui4_a1_jobs.txt`. The ON runner was
`local/codex_a1_activation/run_gate.ps1`, partitioned three ways after the short
USN04 and milestone-D runs passed. It uses `tools/run_game.ps1 -Slots 8`, resets
jitter/native environment flags, and sets the launching process to Normal
priority. At most three matrix game instances ran simultaneously. Actual
Normal-priority process observations are retained in `matrix_normal_priority.json`
and `priority_observations.jsonl`.

Every raw `pair_diff` result is exit **1**, preserved in `pair_<row>.json/.txt`.
It is not relabeled exit 0. The exact non-noise record inventory is:

- OFF only: `native GUI Text lineage ready: text=19 element=18 root=0 parent_match=1 counter=canonical_vfs source_bootstrap=late before_fonts=1`.
- ON only: `native GUI Text lineage ready: text=21 element=20 root=0 parent_match=1 counter=canonical_vfs source_bootstrap=late before_fonts=1`.
- ON only: `native renderer startup domain (A1): registry/gate=# scene_environment=# node=# camera=# viewport=# directional=# pool_01090154=# fog=# types=CD80A0,CD80F0 pool=CD8080 frames=256 reservation=107376 bytes`, with the tool's existing pointer masks.

The evaluator separately requires that this inventory match exactly and that
`pair_diff.is_identical` hold on the remainder after removing just those three
exact records. This preserves all existing parameter, clock, gameplay,
per-entity, native-table and summary checks, with only the tool's established
noise handling. No global tool or noise rule changed; no type-id or GUI pattern
is broadly ignored. The provisional id change was already documented in A1.

Ordered dialog/message/Lua comparisons use only the existing `gui4_subdiff.py`
pointer and worker-thread-id masks. All 13 have zero differing indices, including
length checks. Existing `gui4_teardown.py` comparisons have zero differing
indices and zero length deltas. The expected A1 record inventory above is
retained separately rather than hidden by teardown filtering.

All startup frames are 36, and both sides have the same first logged draw:

| Row | First draw | Teardown lines per side |
| --- | --- | --- |
| ESMP08 | 0.4933 | 638 |
| JM06 | 0.1578 | 241 |
| USN21 | 0.6207 | 413 |
| IJN18 | 0.4640 | 249 |
| BSM01 | 1 | 166 |
| USN12 | 0.1307 | 360 |
| LOMP10 | 0.1119 | 244 |
| BSM09 | 0.0949 | 328 |
| PRCP01 | 0.3605 | 290 |
| USN01 | 0.04169 | 1935 |
| USN02 | 1 | 203 |
| LOMP06 | 1 | 166 |
| USN04 | 0.4143 | 248 |

The aggregate is `matrix_evaluation.json` (13/13 `row_contract_pass`), with
per-row raw reports, `.ordered.json` and `.teardown.txt` files. Every manifest's
exit and executable hash are checked. The evaluator is `evaluate.py`.

## Milestone-D lifecycle

ON USN04 with `BSP_NATIVE_MISSION_SHIP/FRAME/UNITS=1` ran all 1600 mission frames
and exited 0. `on_usn04_lifecycle.log` records the startup domain once (127),
179 preload rows released (30134), Application publication cleared (30143),
lighting retired with an empty registry (30514), mission ship closed at frame
1600 (30528), renderer ownership cleared with the worker joined (30531), and
final COM release (32111). The existing teardown comparison is 4312/4312 lines,
zero differing indices and zero length delta (`msd_teardown.txt`). No lifecycle
failure was suppressed. The final texture reference remains the existing
documented fallback allowance.

## Final matched common-code gates

The original 13-row matrix above remains proof for its exact immutable packet
baseline. Independent prepared GUI dependencies and the independently gated
aim-freeze OFF packet were then frozen together in common OFF source
`6216ea1afb12bc2d702e00a9063e6d263bd8a32f`. Later main metadata/tooling commits
do not change that frozen C++ snapshot. Common ON source is clean merge commit
`6c8016fa756932161f09d4ccc5617ecd8adf8e51`.

`common_source_diff.patch` and the final source audit establish that src/include
differs from the common OFF snapshot only in
`kNativeRendererStartupCc15Bound=false -> true`; CMake configuration sources
are unchanged. The common ON Win32 Release build passed both existing CTest
suites, and PC24 diff lint passed with zero findings on the one changed header.
The build log is `common_build.log`.

| Common executable | SHA-256 |
| --- | --- |
| OFF, saved coordinator build of `6216ea1af` | `5461327de616fef8485e49a86dce17901399092b62acc4621a0a0c4da06fc926` |
| ON, build of `6c8016fa7` | `799f5fd80a1c35fbb002de0504a1d6b608044af14f30c4b124667448b38f3972` |

Both executables use the same supplied XLive DLL, SHA-256
`ddf2c2a86d96f37819147c38991a74ef5c5946ce625e7b4e7914f8382aa13f87`.
Separate `common_off_exe/` and `common_on_exe/` copies and
`common_build_receipt.json` preserve these artifacts. Launcher checkout metadata
can describe the launching worktree even for a copied executable; executable
hashes and the saved build receipts identify the actual OFF/ON source builds.
The old `634258460` artifacts were never replaced or mixed with this common OFF.

Current-code common ON reference USN04 and milestone-D USN04 through 1600
mission frames both exited 0 and pass the exact A1 comparison contract against
the matching coordinator OFF receipts. Raw pair exits remain 1. Reference
ordered lines are 762/762 and teardown 248/248; milestone-D ordered lines are
626/626 and teardown 4312/4312, all with zero differing indices and length
deltas. Startup is frame 36 on both sides, with first logged draws 0.4143
(reference) and 1 (milestone-D). Reports are `common_ref_evaluation.json` and
`common_msd_evaluation.json` with their per-pair artifacts.

## Strict serial USN21 4+4

The approved light-load series ran exactly OFF/ON/OFF/ON/OFF/ON/OFF/ON, one
game instance at a time, Normal priority, `tools/run_game.ps1 -Slots 8`, with
`BSP_SEARCHER_BOUNDS_INIT=1` symmetrically on all eight runs. Death-table and
gunnery RNG streams were enabled; jitter and native mission overrides were
absent. All eight exited 0. Every completed run was checked before the next
launch; there were no failures, retries or hidden reruns.

| Run | Comparison against OFF1 | Raw pair exit | Ordered index differences |
| --- | --- | --- | --- |
| OFF1 | baseline receipt | n/a | n/a |
| ON1 | exact A1 contract | 1 | 0 |
| OFF2 | identical apart from existing noise | 0 | 0 |
| ON2 | exact A1 contract | 1 | 0 |
| OFF3 | identical apart from existing noise | 0 | 0 |
| ON3 | exact A1 contract | 1 | 0 |
| OFF4 | identical apart from existing noise | 0 | 0 |
| ON4 | exact A1 contract | 1 | 0 |

All four OFF runs therefore self-agree, and every ON run matches the same OFF
baseline except the three exact records inventoried above. Gameplay,
per-entity rows, native table and summaries pass the unchanged remainder
assertion. Every compared startup is frame 36 with first logged draw 0.6207;
teardown is 413/413 lines, zero differing indices and zero length delta.

The first game started 2026-10-28T23:20:10.4889118Z; the last game ended
2026-10-29T01:15:41.9534266Z. Individual exact times are retained in
`common_strict_series_order.json` and all eight manifests. The sequence and
every adjacent finish/start boundary were audited as nonoverlapping, preserving
100 ns UTC instants while accepting JSON's omitted trailing fractional zeros.
`common_strict_evaluation.json` and `common_provenance_final.json` both pass.

## Separate reference USN21 pair without bounds aid

After strict acceptance, one OFF/ON pair ran serially with the same common
executables, plan and Normal-priority launch discipline. The environment has
only `BSP_DEATH_TABLE=1` and `BSP_GUNNERY_RNG_STREAMS=1`; the bounds aid is
absent on both sides. OFF ran 2026-10-29T01:17:29.0242591Z through
01:31:03.6172075Z, followed by ON at 01:31:05.5029162Z through
01:44:42.8494445Z. Both exited 0. OFF's receipt passed before ON launched.

The pair passes the same exact A1 contract: raw pair exit 1, unchanged
remainder, ordered dialog/message/Lua 1887/1887 with zero differing indices,
startup frame 36/36, first logged draw 0.6207/0.6207, and teardown 413/413
with zero differing indices and length delta. No masks changed. Full
provenance passes, including the absent bounds variable and nonoverlapping
timestamps. `common_noaid_on_evaluation.json` is the final pair result;
`common_noaid_off_evaluation.json` and `common_noaid_series_order.json`
preserve the preceding receipt and launch sequence. This is a reference-form
pair without the bounds aid, not a full WINS re-time or a default-environment
stability claim.

## Immutable input and provenance audit

The receipt-only hardening was added while ON1 ran; OFF1 and live ON1 were
retrospectively audited and passed. No executable, game argument, order or mask
changed, and no run restarted. Later launches preflight the pinned files before
starting, and each completed run audits its captured inputs before progression.

- Plan: `config/win_orders/usn21_nd1_s46_u21_f.txt`, SHA-256 `d189ab67e44d93f4a45786930b07f3d75e30297c59c4b724c3536b3986dfb00b`.
- Matrix: the copied A1 job file, SHA-256 `9e11e1f2def8d4dd6654d9e9c18df7248c3444915a6b10049fcc67959b176393`.
- Every full launcher/argument vector, source input path, archived input name,
  order/DLL hash, executable hash, pinned difficulty 1 (including effective
  game+6B0h value), exact environment, game root, worktree/source, slot 0 and
  core 2 is checked. Only the expected side-specific executable/DLL paths and
  per-run log paths vary within each series.
- Exact alternating serial order, captured order receipts, completed exits and
  nonoverlapping timestamps are checked; unexpected extra strict/no-aid
  manifests reject acceptance.
- `common_priority_observations.jsonl` contains 20 actual launcher/game process
  observations for the ten USN21 runs, all Normal.

The fixed input data/copies are in `common_input_pin.json` and
`common_pinned_*`; validators are `strict_provenance.py`,
`strict_step_evaluate.py`, `strict_evaluate.py` and
`common_noaid_evaluate.py`. `activation_final_summary.json` combines the
original matrix, common build, current smokes and both final series. The
integrator reviewed the receipts and accepted A1 ON with integrated C++ matching
the frozen common ON snapshot; no worker push was performed.

## Scope limits

The provisional 19/18 -> 21/20 values are GUI Text/Element lineage ids. A1
opens the startup scene domain; it does not establish native GUI page drawing,
startup shader preload, reload, repeated GUI traversal or late-plane drawing.
The common completed-frame-prefix reuse is source/build preparation; its
allocator path is not exercised by the current A1-only gates. Live GUI, reload
and allocator runtime proof remain open. The Known Limits correction is
the same independently cherry-pickable change as `2b3194a77` (main
`261891c4be`) and remains preserved.
