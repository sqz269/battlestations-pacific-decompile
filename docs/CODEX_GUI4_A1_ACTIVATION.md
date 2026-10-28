# A1 startup scene-domain activation evidence

## Immutable packet baseline

Accepted OFF source: `5ca4639f0b` (same code as `381792703`). ON source:
`634258460a6ea47e585fe015694f6d69ba331b2f`, clean before its build and runs.
The only source change sets `kNativeRendererStartupCc15Bound` true in
`include/bsp/game_native_renderer_application.hpp`. No other switch changes.
The domain-only fallback and accepted shutdown correction are the implementation
exercised here. This baseline matrix is complete; ON publication is still pending.

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

## Remaining common-code gate

Independent prepared GUI dependencies landed while this immutable matrix ran.
The coordinator will supply a matching latest-common-code OFF artifact after
integrating the independently gated prepared dependencies and aim-freeze OFF
packet. Rebuild a separate ON executable over that same code, preserving these
original artifacts. Repeat only proportionate build/lifecycle checks unless a
new interaction requires additional rows; this 13-row matrix remains proof for
the exact `5ca4639f0b` packet baseline.

ON publication requires a coordinator-approved Normal-priority serial
OFF/ON/OFF/ON/OFF/ON/OFF/ON USN21 series on matching common code, with symmetric
bounds aid. All four OFF runs must agree and all ON runs must match under the
exact documented A1 contract. A no-bounds-aid OFF/ON check is also required.
Neither series has started in this baseline record. GUI Text ids remain
provisional; this activation does not establish native GUI page rendering,
startup shader preload, reload, repeated GUI traversal or late-plane drawing.
