# USN04 execution coverage census

The reconstruction reference run reached **1,740 identified mapped game-function
addresses**, including **342 observed only through inline sites**. It completed
4,800 mission frames and exited 0. Original mission coverage remains unknown
after two guarded DynamoRIO startup failures. The brief's function-entry
breakpoint fallback is being implemented. A short native motion capture and its thread histogram are separate
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

The frozen census table contains 21,770 game-classified addresses, including
unknown unreferenced rows outside the board's 20,892 reconstruction target.
Those are different denominators. All 1,740 observed body mappings belong to
the board target; none comes from its excluded unreferenced addresses.

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

Both original attempts used the native direct USN04 scene argument from the
earlier baseline. This differs from the reconstruction menu path; equivalence
remains deferred. Neither attempt reached a valid mission clock or SceneInit.

The pinned SDK CREATE/import-shim backend passed private token, explicit-handle,
job, mutation-denial, Documents, floating-point, CALL5 and authenticated-stop
fixtures. Both real attempts passed the 69,700-object access audit, inherited
69,700 protected handles and admitted entry at base `00400000`; neither used a
debugger. The original CALL and continuation executed under DR while native
entry memory retained the disclosed `EBFE` barrier. All owned jobs were empty
before parent lock release. Both inventories verified all 17 installed runtime
files and four protected options/save files unchanged.

| Original attempt | PID | Result |
| --- | ---: | --- |
| live01 | 98252 | Startup exit `C0000005`; complete 328,332-row BB file, no mission clock. |
| live02 | 99608 | Same exit; bounded pass-through logger observed an AV in `nvd3dum.dll` at RVA `001B0285`, reading `00000040`; no mission clock. |

The observed NVIDIA module is version `32.0.16.1742`, SHA-256
`617c9af288fb28023f29cca57963c219aa117647df0fea75c144765efc048fe1`.
The observation does not establish a driver defect or the initiating cause.
The final missing `causticstexturesource` warning also occurs in the successful
native baseline. DR's kernel child-process limits differ from classic locked
mode's debug-event child rejection, which remains a compatibility confound.

Excluding the GPU DLL from instrumentation can miss callbacks into game code.
The pinned Windows retakeover hook is unimplemented, so such an exclusion
cannot justify negative execution claims. See the [pinned hook implementation](https://github.com/DynamoRIO/dynamorio/blob/cronbuild-11.91.20749/core/win32/native_pe.c#L52-L62)
and [native-call coverage issue](https://github.com/DynamoRIO/dynamorio/issues/978).
No driver, asset or power setting was changed.

Stock drrun lacked the guard's restricted creation/handle contract; suspended
attach failed before entry, PIN's official download returned 403, and stock
TinyInst requires a pre-entry adapter. The remaining documented fallback uses
the already established classic guarded debugger to record one first hit per
frozen game-function entry. It will preserve unknowns for unhit functions and
leave dynamic BB/call counts unavailable. No original was traced outside the
guard. `COVERAGE_ORIGINAL_GUARD.md` contains the backend and fixture evidence.

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
