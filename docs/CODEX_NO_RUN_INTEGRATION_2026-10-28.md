# CC16 integration without new game runs, 2026-10-28

The user requested an integration attempt using review, existing acceptance
receipts and compilation, with no new game launches. The remote was refreshed
before selection and publication. The reviewed baseline was `5ca4639f0`.
The handoff's proposed GUI A1 OFF and Application shutdown batch was already
published; the stale local `main` reference caused the misleading unmerged count.

## Accepted documentation corrections

- `docs/SHIP_AI_OPEN_ITEMS.md` section 325 incorporates the historical build-base
  caveat from `f9e4b33860`. Its USN15 timing rows now identify branch results
  rather than claiming a later main revision. The worker's build provenance and
  lua72's regression report are attributed as historical evidence; this audit
  does not establish a new timing or cause for the regression.
- `docs/CC13_MODE3STEER.md` incorporates the remaining clarification from
  `7c26febef`: Ashigara's own-group escorts fail the preceding `009D8010` gate
  before the distance comparison. The supporting trace is already in
  `docs/CC13_DRIVE32C.md` section 3. The existing settings+1C8h correction and
  later sections are preserved.

Both changes were reviewed independently. This batch changes documentation only.

## Existing acceptance evidence reused

`381792703` to `5ca4639f0` changes three documentation files. Source and build
inputs are identical. The receipt audit checked the saved executable hashes,
all 26 default-row manifests, 13 pair JSONs, the aggregate gate summary, and the
milestone-D manifest and raw retirement log.

| Evidence | Verified result |
|---|---|
| Baseline saved executable, `67aad2827` | SHA-256 `93C811EE06356F25B41018EEC6DCDBED3A1FE457BDFA419CDFD5C9807BFF3132` |
| Candidate saved executable, `381792703` | SHA-256 `1A30FA1D6C67B51BEF537C69AF3179DC8454A0C4DDF608229F97DEB1D0C80BDB` |
| Default-environment pairs | 13/13 identical; exits and pair comparisons 0 |
| Ordered events and teardown | Zero differences and zero teardown length delta |
| Startup | Mission start frame 36 and matching first logged RNG draws |
| Milestone-D USN04 | 1600 mission frames, exit 0, 179 preload rows released, clean Application/mission/renderer retirement |

Receipts remain in the cc16-progress worktree under
`local/codex_cc16_build_receipts.json`, `local/codex_cc16_gate_summary.json`,
`local/codex_cc16_pair_<row>.json`, and
`local/codex_cc16_candidate_usn04_msd.log` with its manifest. Some baseline
manifests name the later launcher's checkout; their saved executable hashes
match the independently retained baseline build receipt, as explained in
`docs/CODEX_CC16_INTEGRATION_2026-10-28.md`.

## Fresh compilation

`./scripts/build.ps1` succeeded on `5ca4639f0` in cc16-progress. Both registered
suites passed: `reconstructed_math` and `tool_tests`. The resulting game
executable's SHA-256 is exactly the accepted candidate hash above. Probes were
built; this audit launched no game or rendering probe.

The build log is retained in both the original build worktree and this audit's
`local/codex_no_run_build_20261028.log`, SHA-256
`EA8FEA513C627A28851B8F7A89A592BB19E8F3DBADBDFE8064BD13BAD3BD9334`.
`local/codex_no_run_audit_20261028.json` records the source baseline and scope.
The selected documentation corrections leave the tested source/build inputs
unchanged; no further build is required for them.

## Remaining peer work

| Candidate | Disposition at the reviewed baseline |
|---|---|
| `4d0350e9e`, plane reference-freeze OFF | Missing required 13-row OFF identity gate; storage/mask changes also occur outside the switch. Another coordinator now owns fresh gate work. |
| `52f52c743` / `d98cadc27`, subtitles OFF/ON | Only a historical USNOS pair, pair comparison exit 1, and callbacks unexercised; required acceptance remains open. |
| `a8f4a897c` / `854192d76`, physics flip/census | Behavior-changing flip has moved/failing rows. Census evidence does not isolate its changes from the experimental switches. |
| `b34f7946ff`, USNOS investigation queue | Superseded by the completed investigation in SHIP_AI section 82. |
| `21685af19f`, old landing-task queue | Substantially superseded; includes work implemented since the old handoff. |
| `95621dc99`, partial session-mode note | Superseded by SHIP_AI section 129's constructor/caller census. |
| Stale PC24/trivial-slot/yaw branches | Previously reviewed integrated content; no wholesale stale-branch merge. |

GUI A1 ON, P3a2 and fresh plane-reference gates are owned by a separate active
coordinator. This audit used its own worktree and explicit documentation leases
after detecting that concurrent continuation. Their worktrees, changes and
running processes were preserved. Gameplay switches and WINS timings were not
changed by this batch. Historical OFF acceptance does not establish GUI ON
acceptance or late-plane drawing.
