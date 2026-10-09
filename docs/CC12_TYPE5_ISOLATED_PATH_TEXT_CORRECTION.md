# Type5 V7 isolated-path TEXT correction

The fresh V7 worker proposal corrects all three V6 path defects. It remains
**Source credit 0, UNADOPTED**, with null Root authorship/receipt pins, empty
execution inventory/membership arrays, and an explicit execution hold. The
fixed future family is
`J:/PROG/battlestations-pacific-decompile-cc12_type5_fixture_execution/local/type5p3`.
No Root files or selected functions were changed or executed by this worker.

The sealed proposal is in worker `local/t5isolatedFix02`. Use
`candidate_TEXT_v7`, `proposed_selection_template_v7_corrected.json`,
`derivation_v7_corrected.json`, `proposed_execution_scopes_v7.json`, and
`proposed_materializer_v7.ps1.txt`. The earlier selection/derivation files in
that family retain an own metadata serialization failure and are superseded
by the explicitly named corrected files.

| Corrected binding | V7 result |
| --- | --- |
| `complete_guard.py:11` | `EXPECTED_FAMILY` is the exact isolated `type5p3` family. |
| `guarded_tool_entry.py:41` | The permitted BSP tool is the isolated checkout's `tools/bsp.py`. |
| `external_dispatch.ps1:6` | The driver requires that same `type5p3` family; its already isolated tool literal is preserved. |
| Selection `include_roots[0]` and both 16-key Python profiles | All use the isolated `include` directory, agreeing with the recipe's `F.parents[1]` derivation. |
| Four complete private profiles | `TEMP`/`TMP` use the fresh comparison proposal's `type5select03/temp`; external/base remain 14 keys, root/effective remain 16. |
| Materializer | Family and materialization-log prefix use `type5p3`; rejection logic is preserved. |

Nine of twelve payloads are byte-identical to V6, including the complete
recipe, helpers, probe, and six reader modules. Each changed payload has
exactly one intended literal replacement. All ten Python ASTs are equal after
normalizing the two changed string constants. All 170 original assertions
(recipe 108, helpers 62) and all 364 calls counted under `need`, `require`,
`_type5_need`, and `_dependency_require` retain their ordered ASTs. The full
AST comparison also preserves the other branches and calls. PowerShell parsing
reports zero errors, identical parameters, and identical ordered rejection
texts: driver 57 throws/25 `Assert-*` calls; materializer 29/13.

The review covered runtime consumers as well as literal changes:

- `F`, `ROOT=F.parents[1]`, the fixed guard family, driver family, recipe
  source/header/config/build metadata paths, and guarded BSP path agree on
  the isolated checkout/future family. All 46 selected tool-module paths have
  corresponding entries in the recorded isolated inventory.
- Seven include roots, three library roots and both binary roots agree with
  the 51-scope data. The V7 scope change only replaces the candidate-text
  category/path; historical membership predicates and evidence paths remain
  unchanged. The six C++/header identities still match their reviewed pins.
- Selection-derived and recipe-derived effective environments agree exactly.
  Full original/new map comparison proves that external/base changed only
  `TEMP`/`TMP`, while root/effective changed only `INCLUDE`/`TEMP`/`TMP`.
  Complete key sets and all other values remain equal. Private values are
  retained only in ignored local evidence.
- The all-payload path survey preserves the historical `AUDIT` family, exact
  MSVC/SDK roots, original game binary path, system CRT paths and BSP Ghidra
  project identity. These paths were reviewed as text; no original binary,
  Native query, compiler or provider was executed.

Root independently generated `type5select03` proposals. Comparison of 18 fixed
files as data found all twelve payloads and the materializer byte-identical,
all four private maps key/value-identical, and matching runtime selection
fields. The worker did not snapshot Root's entire changing namespace or reuse
Root's process results as worker execution evidence.

There is a separate Native tool dependency hold. The isolated `bsp.py` takes
`ROOT` from `ledger.ROOT`, whose value is derived from the isolated module path.
Its `client()` reads isolated `config/target.json`. However, module initialization
also calls `workspace.exports_dir()`: without `BSP_EXPORTS_DIR`, that function
uses `git rev-parse --git-common-dir` to find Main. The allowed `ghidra count`
route conditionally reads shared `exports/bsp/snapshot.json`. All allowed
Ghidra routes call `connect(required=False)`, which conditionally opens isolated
`local/bsp_index.sqlite`. Those inputs/process relationships are not closed by
the V6/V7 scope rebase alone.

Root confirmed that hold and is preparing a distinct `type5select04` proposal
changing the existing `BSP_EXPORTS_DIR` value to a privately frozen historical
snapshot directory, plus an explicit membership check for the absent optional
index. The worker independently confirmed that all four V6/V7 maps already
contain this key and point it to Main exports: the Git fallback was already
bypassed by those maps. The confirmed uncovered read is the shared snapshot,
alongside the optional index/absence relationship. Counts remain 14
external/base and 16 root/effective; no new key is needed. Fresh values,
`TEMP`/`TMP`, observations, snapshots and Root receipts remain necessary.
The earlier 15/17-key plan recorded in the sealed comparison data is superseded
by this verified correction. This worker's fixed V7 comparison proposal is not
admission evidence for that later variant. No tool behavior or allowed Native
route was changed here, and no full Native-route runtime proof is claimed.

Own analysis dependencies were captured before optional Python imports:
2,102 files, 3,588 memberships, 16 scopes. Before/after checks covered 2,134
original/copy pairs (4,268 files), exact memberships, private ambient ENV and
PowerShell identity. The isolated own AST process closed with exit zero and no
timeout; its 43 loaded file origins matched the before-capture inventory.
Prior Root V6 selection, failed `type5p2`, and peer namespaces were rehashed
with exact membership: 12,107, 29 and 58 files respectively. The selected V5
seal was captured as data; a whole-V5 rehash is not claimed.

Two own utility failures are retained: a failed freeze caused by a PowerShell
variable-name collision was sealed in `local/t5isolatedFix01` (2,138 files),
and a null-field serialization failure caused an own AST review to exit one.
The latter was corrected in new metadata/tool files, preserving the original
attempt and all candidate/private-profile bytes. Both own review attempts have
explicit closure records; no selected payload was imported by either.

The final proposal seal has 2,210 artifacts, exactly 2,211 recursive files,
SHA-256 `090b743b90bee3d2b0f02b853eba1b8634c071cd27e487bd9ffde09c6b905e2d`.
Its contents and membership were verified after writing. `type5p3` remained
absent at final validation. Source credit, selected-function executions,
Native queries, compiler/provider/original-PE/target executions remain zero.
No C++ change or build was part of this packet.
