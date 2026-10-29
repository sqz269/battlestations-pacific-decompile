# Weapon-accuracy RNG observation correction

My `048da36338fedeaef286d3abe10a05183262a268` review incorrectly reported
first RNG draw `1` for MSD1600 and USN02. I copied the inherited substring
regex into the supposedly independent audit and accepted a feature flag as a
sample. I withdraw those claims and that review's unqualified recommendation
that no accuracy-specific activation evidence gate remained unmet.

This is an additive correction. The original review, frozen inputs, evaluators,
saved pass fields/results, raw logs and masks remain unchanged. Their old
values are pinned in the JSON alongside the corrected interpretation. No
source/header/shared-ledger/Ghidra edit or main-guard activation is performed.

## What the old observer actually matched

Both `acr86/inputs/evaluate_accuracy.py` and `gacc_now/inputs/evaluate_accuracy.py`
use `draw=([-0-9.]+)` after the MissionLoad marker. The old independent
`start_probe` copied the same regex. It also matches the suffix of
`lof_expiry_draw=1`, so the old per-row `start_rng=true` does not establish a
real sample when no plane event exists.

`src/game_hosts_gunnery.cpp` prints that field from
`kAaLineOfFireExpiryDrawBound ? 1 : 0`. It is a feature-enabled flag, not a
sample or a draw-consumption count. The real source-format line is emitted by
`src/game_hosts_units.cpp`: `plane death mode: unit=... t=... draw=... mode=...`.
Its value comes from `death_mode_draw_00bd2f10(1,unit,0,1)` except the explicit
water/destroy bypass sentinel `-1`. A sentinel must not count as a sampled RNG
event, but remains in full event-sequence comparisons.

The full producer/logger file blobs are identical at the historical shared-OFF,
current shared-OFF and private-ON source epochs: gunnery
`2f2755d134501d48888ef4b3496db532df4432c3`, units
`d3834be26e182d44b98dba5592144a9273a32662`. The correction follows the actual
logged field and producer, not a guessed numeric range alone.

## Reclassified historical and current matrices

All 54 `acr86` and 65 `gacc_now` frozen file hashes/sizes still match.
All twelve raw logs were independently rescanned with `\bdraw=` and the
complete source-format predicate. The table gives OFF/ON counts separately.

| Matrix/case | Old probe | Genuine sampled events | Correct first logged sample |
| --- | --- | --- | --- |
| acr86 reference USN04 | frame 36 / .4143 | 51 / 51 | 93.60s, .4143 |
| acr86 MSD1600 USN04 | frame 36 / 1 | 0 / 0 | unobserved |
| acr86 hot USN02 | frame 36 / 1 | 0 / 0 | unobserved |
| gacc_now reference USN04 | frame 36 / .4143 | 51 / 51 | 93.60s, .4143 |
| gacc_now MSD1600 USN04 | frame 36 / 1 | 0 / 0 | unobserved |
| gacc_now hot USN02 | frame 36 / 1 | 0 / 0 | unobserved |

Each reference's first line is A6M Zero #1.2|.-II, powerlost, at t93.60,
draw0.4143. Its last sample is t202.26/draw0.1295. All 51 source-format
records match OFF/ON, with no sentinels. The old substring matcher finds one
additional summary flag in every reference and only that flag in MSD/USN02.
The erroneous summary is at historical lines 26677/35260 and current
lines 26678/35261 for MSD/USN02 respectively.

Frame36 remains a separately observed mission-start marker. Equal absence of
an event is not an RNG witness. These original MSD1600 and USN02 rows stay
partial/unmet for the nonempty RNG-event gate; no later run relabels them.

Even a real reference event is only the first **logged plane-mode sample** in
that run. The formatter rounds draws to four decimal places and time to two.
The existing `BSP_GUNNERY_RNG_STREAMS=1` measurement mode keys generators by
purpose/unit/index; other draw sites are not comprehensively logged. Neither
first logged sample nor full logged sequence proves the global first RNG draw,
float-bit equality, all-stream state/consumption parity or native RNG history.

## Evidence that the regex correction does not discard

The unchanged pair checker was rerun read-only on all six OFF/ON pairs. Pair,
ordered and teardown differences remain zero, with the inherited masks and
exclusions unchanged. Ordered counts are 762/626/584 for reference/MSD/USN02.
Historical teardown counts are 251/4312/226; current counts are 253/4314/228.
Original evaluator `passed` and `start_rng` fields remain stored as originally
written; this correction marks the RNG part false/unobserved where appropriate.

Each USN02 side still has 1,789,747 actual accuracy calls, 584,640 completed
post-return ratings and 10,107 ring bearings. The observed outcome is still
failed at 130.05s, Game Over/Alden. That is valid hot/comparison evidence,
without a genuine logged RNG sample. No WINS retime or outcome reinterpretation
is introduced. Existing build, final-call/canonical-cell and narrow FP fixture
proofs retain their original qualified scope; this packet neither re-audits nor
enlarges them. In particular, fixture 8c2 remains the original post-spill window
with weight 1/direct converter, masked PC24/PC53 and synthetic modes0/1 only.

## Natural hot witness selection before any new run

Retained `off409` USN21 logs identify a natural full 80k-frame hot workload:
8,753,634 accuracy calls, 2,405,880 completed ratings, 441 samples and seven
non-sampled sentinel records per side. Its first sample is489.35s/.4819,
but first actual accuracy/completed-return markers occur only between mission
frames 29965/29966. The first subsequent sample is1502.39s/.0493. The proposed
12k prefix was therefore inadequate and was withdrawn before execution.
The later32k proposal was also superseded before freezing/running when an
earlier natural witness was found. No USN21 prefix run/pass is claimed.

Independently checked IJN18 full 64k logs have first accuracy line 5490 and
completed-return line 5511, after mission 73 and before 74. Their genuine Recon2
sample at 67.05s/.4640 is line 14964, after mission 1340, thus after an actual
completed return. Both sides have one sample, zero sentinels, 3,087,856
accuracy calls and459,960 completed ratings. The source block calls0095EB40,
then logs completion, increments the counters, and returns its result. The
first-log mechanism emits only on the first completed call. Those blocks match
at the historical409 and current5c/c8 epochs.

These are historical full-run observations, not current-guard A/B results or
inherited short-run proof. They justify choosing a new, separately frozen
IJN18 prefix rather than altering an existing plan or forcing a death/event.

## New strict witnesses and approval boundary

Root reviewed and authorized two new 4800-mission/5100-total prefixes at step .05:
native-mode USN04 and hot IJN18. Both are distinct added cases. The latter
copies the exact old `ijn18_l67_i18_d.txt` bytes (SHA256
`8324d67e28d82ae234cf4bd00e482605eff7b1bf04c16c401787ca79cae493c8`),
with unchanged DEATH_TABLE/RNG_STREAMS/SEARCHER_BOUNDS_INIT flags, difficulty1
and stop-after-end40. The240s budget exceeds the observed sample time by 172.95s.
The native case retains its original native-frame/ship/units flags and no plan.

The clean named runtime host is pinned OFF at
`0ff658233116db29ce600aea90095dd1f1cd1264`; its tracked diff/status are empty.
It uses exact copied current-frozen OFF/ON binaries and the common DLL, current
reviewed launcher, unchanged original evaluator/checkers and owned-identity
external watchdog, Normal priority, Slots8 and at most one owned logical run.
The two freeze hashes are `5630cc12308a85bcf286a47056b33476d14179ef036e888572891de07033a41b`
and `7f20acdc13317f9869cac8e6c71a7c0de1b09229ecdd6dc23fd29a92141fae34`.
Frozen pending-creation flags remain immutable. Root's later timestamped
execution approval is a separate receipt, not a rewritten historical approval.

Each side must have a genuine non-sentinel sample. IJN18 must also have actual
accuracy and completed-rating counters greater than zero, a real completed-
return marker, and at least one genuine sampled event **after** that marker.
Full normalized source-format sequences, including sentinel diagnostics, must
match. All existing start/settings, pair, ordered, teardown, file/argument/env,
process/slot/priority and four-hour UTC watchdog gates also remain mandatory.
The unchanged controller's matrix.done alone is not acceptance: the additional
strict sampled-event checker must pass. Missing evidence is inconclusive/fail;
no posthoc budget, seed, source, trace, event-forcing or mask change is allowed.

Native OFF/ON is serialized first. IJN18 starts only after both original and
strict native gates pass. The JSON carries actual terminal receipts as they
complete; preparation or authorization alone is not runtime proof. Even a
successful new matrix establishes a bounded rounded logged projection only.
Old absent-event rows and global/all-stream claims remain unproved.

Both authorized new pairs completed with original and strict gates passing:

| New case | Samples OFF/ON | Accuracy calls per side | Completed ratings per side | First sample / after return |
| --- | ---: | ---: | ---: | --- |
| Native USN04 4800 | 51 / 51 | 0 | 0 | 93.60s / .4143; hot return not required |
| IJN18 4800 | 1 / 1 | 2,843,254 | 432,180 | 67.05s / .4640, after the completed-return marker |

All four logical games exited zero. Original pair, ordered, teardown, start,
settings and provenance checks pass, and full source-format sequences match.
Actual runs used slot0/core2, Normal, with eight real held-handle parent/child
captures, correct parent chains, exact start+14400s UTC deadlines and no
watchdog actions. Original inputs and both reviewed freeze/plan/criteria hashes
remain unchanged. No additional rerun was started.

This result is confined to the exact copied 5c-OFF/c8-private-ON executables
and clean 0ff runtime host. It is **not current-main ON acceptance**. Later
C320/C5/profile-cleanup, live locale and BDD600/MainSection integrations are
outside these executable receipts; frame/result source has changed. Root must
separately refresh critical fingerprints, final actual code and current OFF/ON
builds, and preregister any required candidate run. This packet does not
silently extend the old binaries' result to that new context.

## Publication obligation

Shared main at the audited 1e10d9de4 remains OFF; c8 is a private ON candidate.
The uncommitted/private 8383 activation report/name drafts must be qualified
before publication. They must not repeat first-draw1, treat equal absence as
an RNG pass, inherit a shortened-run result, or promote the new projection to
global RNG proof. Root owns any final reviewed guard/publication decision.
This correction supplies no new body/vendor credit or original-ABI claim.
