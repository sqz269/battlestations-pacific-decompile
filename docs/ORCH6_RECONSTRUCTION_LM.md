# L/M reconstruction integration

Addresses: 006D1F20, 00953CC0, 0095DC40, 009E52E0, 009F0EA0,
004BB160, 004F1D70, 00469D23, 00469D37.

The separate orch6 integration worktree combined the L obstacle constructor,
generic motion correction and persistent participant owner with the M generic
input and neighbour frame packets. reports/orch6_reconstruction_lm.json
records worker commits, call checks, artifact hashes and exact build/run IDs.

The participant owner now uses authored MaxPlayerNum8 from the loaded scene
header. Its session retains player0 and mission1..7 across frame-owner
replacement and preserves device bytes. The old frame is destroyed before its
borrowed Lua owner is replaced. Single-load runtime is verified; repeat-load
runtime remains open.

The verified generic tick now runs for42 actual generic leaf units whose live
roles0/4 equal8 and whose owned flag634 is0. It uses the corrected final
callback gate and the complete false-role input helper, with the proven RET4
no-op at the final virtual. Its field owners and conditional domain are
documented in UNIT_GENERIC_INPUT_LIVE.md.

The combined fdb806e2 executable passed MSVC Win32 Release and both existing
CTest checks. Its120-frame mission run executed10080 generic ticks with zero
unavailable calls,2400 avoidance queries and1080 cruise-owner reads with zero
unavailable reads. All18557 trajectory rows were finite and the241 Airfield2
samples remained unchanged. Exact executable SHA256:
`a350f0a0c41fb73b323f0b30c55622e1b04e9b7f598b30b4feb3b9b5326b40bb`.

Root review checked71 reported call sites without failure;6 are indirect and
their targets require the separately inspected table or interface evidence.
159 ignored native-fixture artifacts were archived and hash-checked before
worker worktree retirement. The native proofs use explicit external callback
boundaries; none establishes whole-game or binary replacement compatibility.

Neighbour refresh is reconstructed and fixture-tested but is not yet bound to
the running candidate list. Admission, correct world-list membership, timer
and RNG owners, and observer lifetime must precede that binding. The current
runtime registers every unit through006FE620; inspected +130h slots for
LandFort and CommandBuilding instead target006F59B0 and006F5A50. The existing
avoid-box shrink-NaN discrepancy also remains open. Candidate-loop and
admission workers are recovering those adjacent contracts in separate trees.

## Final combined M verification

Commit `bc9aef8837ba2577a26c3168a09d1b2c8e09d3bf` also includes the third M
packet, which corrects the three artillery throw settings to stored reciprocal
rates. Its installed durations 2 / 4 / 2 produce 0.5 / 0.25 / 0.5. The helper
preserves the native widened-0.1f clamp, unordered NaN branch and x87 stores;
240 numerical/FP-state cases and six ordered-prefix cases passed with Lua
callbacks explicitly redirected.

This final combined commit passed Win32 Release and both existing CTests.
Its 120-frame run again produced 18,557 finite rows, 10,080 generic ticks
across 42 units, 2,400 avoidance queries and 1,080 cruise-owner reads, with
zero unavailable generic ticks or cruise reads. The final executable hash is
`3e4701f2f6215622a29cf47243103e0b1183afb0967bc22565172cefea3d1e86`.
Root review now totals 88 call rows with zero failures, including the same
six indirect contracts. The two archive manifests retain 197 artifacts.
The earlier fdb806e2 executable and runtime record remain separately preserved.
