# USN04 execution coverage census

The reconstruction reference run reached **1,740 identified mapped game-function
addresses**, including **342 observed only through inline sites**. It completed
4,800 mission frames and exited 0. Original-game coverage is still unknown:
the guarded tracer's independent entry-admission path is being tested on private
fixtures. A short native motion capture and its thread histogram are separate
evidence and are not substituted for an original basic-block trace.

`config/coverage_census_summary.json` supplies the board's `bsp_live=1740` and
`orig_live=null`. Null displays as **Not measured**, rather than zero. This is
an observed mapped lower bound, not a count of behaviorally correct functions.
The source build is frozen at `9c79eedff6231a0140c6dc401f5305e8526bc4fd`;
it is not a claim that every later Main commit was executed.

## Tracer and build provenance

The official [DynamoRIO release](https://github.com/DynamoRIO/dynamorio/releases/tag/cronbuild-11.91.20749)
was downloaded into ignored `local/tools/dynamorio/`, without an installer or
system PATH change. Version: `cronbuild-11.91.20749`, published 2026-10-24.
The 467,145,305-byte Windows ZIP SHA-256 is
`9ef2d8f990a6acb4d303385fec0105bd45e54d36e92269494ac8d45e966c3844`,
matching its official release asset digest.

`BSP_COVERAGE_PDB` defaults OFF. ON adds `/Zi` and
`/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO` to separate Win32 Release builds.
The same-commit normal/coverage builds completed with exit 0. All 36,087 common
map function-name RVA sets and all section RVAs match; `.text` size is identical.
The 11,109 differing text bytes are confined to matching relocation operands;
nonrelocated text bytes match. Data/debug layout differs, so neither byte-
identical executables nor identical complete images are claimed.

| Artifact | SHA-256 |
| --- | --- |
| Coverage EXE | `128fba34c171517a605d0188c00fac76f8edf020b9a6dbad51da6d192c9c6d93` |
| Coverage PDB | `6545e1ea9eefd884b969264f15e74b8f714eaac90bdab4268746089a403e61b4` |
| Normal EXE | `dd486dbf0ea2cb426c7374b68561382c5d1840e9239e1cdd9f3aed02bedeb461` |
| Original EXE, not yet coverage traced | `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |

The full PDB is 92,614,656 bytes, GUID
`dcb824dc-35b7-495b-88c3-b1528a05f247`, age 1. Offline attribution verified
that identity against the exact EXE. Details: `COVERAGE_BUILD.md`.

## Actual reconstruction runs

All runs used `tools/run_game.ps1`, Normal priority, shared slot/DRAIN protocol,
the byte-copy private installed game data, per-case private settings/Documents,
and exactly the two reference BSP variables `BSP_DEATH_TABLE=1` and
`BSP_GUNNERY_RNG_STREAMS=1`. Source runtime hashes (17) and options/save files
(4, including timestamps) remained unchanged. Read-only profile IO rejects
writes; checkpoint binding is OFF; mission-result storage flush is record-only.
Existing real product/save directories were asserted before bootstrap calls.
The actual traced children loaded the exact frozen `xlive_stub.dll`.
See `COVERAGE_BSP_RUNS.md` for the write audit, commands and process receipts.

| Run | App / mission frames | Wall seconds | Exit | Mission entry / first tick, app index |
| --- | ---: | ---: | ---: | ---: |
| Smoke, untraced | 620 / 300 | 8.370 | 0 | 38 / 39 |
| Smoke, traced | 620 / 300 | 74.451 | 0 | 38 / 39 |
| Reference, untraced | 5000 / 4800 | 72.308 | 0 | 38 / 39 |
| Reference, traced | 5000 / 4800 | 171.016 | 0 | 38 / 39 |

The long arguments were exactly `--frames 5000 --press-start-frame 30
--menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05`.
This produced 240.00 simulated seconds. Source-verified completed-render ordinals
recover entry/first-tick app indices; matching starts are measured, not inferred
from identical command arguments. All runs drained naturally; no old process
was stopped and no owned game process survived.

Long traced bootstrap PID 92948 and game child 20456 are both included.
Their module tables bind the frozen executable at base `10000000`.
The whole-run union includes startup/menu blocks. Drcov has no per-block game
timestamp, so the collection cannot be trimmed into a mission-only set.

Combat outcomes differ: the untraced long run reported 695 dispatched hits,
57 deaths and 12,859 damage; traced reported 810, 56 and 13,494.6. The cause is
not established. Fixed mission delta and matching starts therefore do not
establish deterministic gameplay equivalence.

## Attributed reach and uncertainty

The actual logs use drcov file version 3, module table 5. Version 3 offsets are
relative to a segment; version 2 offsets are relative to its containing module.
The strict parser handles both separately. Both complete long BB tables contain
176,460 raw main-module records, deduplicated to 104,884 distinct blocks in the
bootstrap+child union. Child alone contributes 104,559; 1,003 are shared and
325 are bootstrap-only. Repeated block records are not function call counts.

The PDB scan covered every one of 2,346,688 observed bytes, with no API or
inventory errors. Qualified/source identity and native-address suffix evidence
exclude bare foreign leaf-name collisions such as `std::next`. Earlier
provisional 2,191 and 2,173 mapped-name counts are superseded. A source-body
review removed 433 apparent credits carried only by mismatched identities.
For example, a HUD adapter named with0059CB90 is not the declared frontend
body; legitimate ship/flight-body and host-HUD-body hits remain. A native
address suffix creates a candidate only; confirmed body source or exact
qualified identity is required, and a known source mismatch rejects it. The separate smoke count is 1,571.

| Frozen game-address state | Count |
| --- | ---: |
| Identified executed mappings | 1,740 |
| Of those, function-ledger mappings | 1,740 |
| Observed only via inline sites | 342 |
| Matched identity, not observed in this run | 1,989 |
| Execution attribution remains unknown | 18,041 |

Uncertainty buckets overlap: 74,085 unresolved PDB site records (23,635 unique
names), 284 ambiguous executed candidates (275 without another definite hit),
85 inventory candidates without source confirmation, 206 source-identifier-
unknown wire rows and 16,408 rows without a verified matching body/inline record.
Missing records are not definitely labelled optimized away, removed or ICF-folded.
No host/adapter suffix, source-wiring flag, OFF switch, vendor body or shared-RVA ambiguity alone
receives game execution credit. Source-wiring states remain labels from the frozen analysis, not execution proof.

| Board category | Identified executed body addresses |
| --- | ---: |
| ai | 203 |
| core | 152 |
| files | 95 |
| gui | 127 |
| lua | 141 |
| mission | 48 |
| net | 13 |
| other | 27 |
| render | 371 |
| scene | 177 |
| sound | 77 |
| units | 309 |

Host/adapter candidate reach is separate:1,233 records mention882 candidate
addresses;471 have no independent positive body hit. These hints overlap
other audit buckets and are not reconstruction-body execution credit.

## Original side and wiring frontier

Original startup is planned through the same native direct USN04 scene path as
the earlier baseline. It differs from the reconstruction menu path; equivalence
remains deferred. Matching game-time endpoints will be recorded if guarded
original tracing passes admission.

Stock drrun does not preserve the current restricted-root creation and explicit
handle-list contract. Suspended attach failed the private pre-entry fixture;
PIN's official download returned 403; stock TinyInst needs a custom entry
adapter. A private SDK CREATE/import-shim fixture subsequently passed token,
handle/job and mutation-denial checks. The full admission fixture also passed
Documents/tracer readiness, a fail-closed entry barrier and preserved floating-
point state. Production integration and authenticated trace-stop flushing are
still being tested on private fixtures. No original was
traced outside the guard. See `COVERAGE_ORIGINAL_GUARD.md` for exact evidence,
the rejected topology, later bounded work and remaining conditions.

With no attributable original BB trace, all original execution cells remain
null. The original-yes/rebuilt-no wiring frontier, original-no/rebuilt-yes and
both-no cross-tables are unavailable, not empty measured results. Current
cross-cells are original-unknown/rebuilt-yes 1,740; unknown/no 1,989;
unknown/unknown 18,041. One mission's unobserved functions are not reached in
this supplied USN04 run, not dead code.

## Artifacts and limits

The immutable detailed result is in the main checkout's ignored
`local/coverage-census-20261030/coverage_census_bsp_only.json`; its SHA-256 is
`bc7da4a9c4bdc2dc500c1603d182a088f7cb32a8f01aa9258c79fb4bf55d4e99`.
The compact `bsp_census_summary.json`, final manifests, compressed trace inputs,
frozen EXE/PDB/map/stub, index/readiness and run/safety receipts are retained
there. Working output is `local/output/coverage_census.json`.

`coverage_census.py` is reusable, preserves explicit unknowns and keeps the
future entry-count observation interface separate. Current drcov data supplies
block reach only. Parser/PDB behavior and API checks are documented in
`COVERAGE_ATTRIBUTION.md`; synthetic fixtures are not mission observations.
Coverage proves execution reach, not correctness, ABI compatibility or parity.
