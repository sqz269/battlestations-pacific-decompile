# Type5 V6 isolated-path TEXT peer: three blocking defects

The V6 relocation is **not ready for further execution**. The independent
passive review found three runtime path bindings left in Main. Source credit
is **0**. This report reviews the UNADOPTED V6 proposal; Root's later adoption,
materialization and failed prepare attempt are separate Root-owned evidence.

| Defect | Fixed V6 evidence | Consequence |
| --- | --- | --- |
| Fixed family was not relocated | `complete_guard.py:11` keeps `EXPECTED_FAMILY=.../battlestations-pacific-decompile/local/type5p1`; line 43 requires exact equality with the running family. | The isolated `...-cc12_type5_fixture_execution/local/type5p2` is rejected before downstream stages. Root separately reported this rejection in its prepare process; the peer did not replay that process. |
| Guarded Native tool was not relocated | `guarded_tool_entry.py:41` keeps Main's `tools/bsp.py`; lines 42–47 enforce that argv path and require its selected inventory row. | The isolated BSP argv is rejected. The selected inventory has no mutable Main tool row to support the old literal either. |
| Include selection and ENV were not relocated | `include_roots[0]` in both the proposal and concrete selection remains Main's `include`. The complete 16-key root/effective profiles also retain that prefix in `INCLUDE`. | The recipe derives the isolated include from `F.parents[1]` and rejects the selected effective ENV at line 197. The Main include directory is outside the frozen dependency selection. |

The last defect is internally consistent within the stale selection:
`external_dispatch.ps1:49` and `complete_guard.py:72` derive the same Main
`INCLUDE` as the selected profiles. It becomes inconsistent with the recipe's
`ROOT`, `INC`, and derived subprocess environment. Successful environment
inspectors cannot detect this by comparing only requested and actual maps.

The concrete Root selection inspected as data was
`type5select02/current_dependency_selection.json`, 10,722,896 bytes,
SHA-256 `7f0ddecb4cf51286c63a4ebeffe5d38ed616c1d14b9e00625656f69da92e4b5d`.
It was Root-adopted and carries the same three unresolved relationships.
The worker captured it without modifying it or treating it as authorization
for execution.

The selected data otherwise preserves the claimed derivation:

- Eleven of twelve payloads are byte-identical to sealed V5. The external
  driver differs only by its two declared family/tool literal replacements.
  This preservation includes the two stale Python literals above, so it is
  evidence of derivation, not proof of a correct relocation.
- The materializer differs only by its declared family/log replacements.
  Parsing both old/new PowerShell texts yields zero syntax errors. Driver
  throw/`Assert-*` counts remain 57/25; materializer counts remain 29/13.
  Ordered rejection and assertion texts are identical. The unchanged Python
  bytes preserve the Root-recorded recipe/helper assertion counts 108/62.
- All 51 scopes match the declared rebase exactly, with membership predicates
  unchanged and historical `local`/`exports` references retained. The peer
  independently checked all 12,051 original/frozen pairs: 24,102 files had
  matching sizes and hashes. All 51 current scope sets match the 14,512
  recorded memberships.
- All 46 tool-module paths resolve to isolated `tools`. The only 147 original
  inventory members under Main belong to historical `local/dup57p2` (130)
  and `local/type5guardReview01` (17). No mutable Main source, module, build,
  config or header path is an inventory member. There are 2,024 isolated
  include files and zero Main include files. The unselected Main include
  reference survives in selection/profile data instead.
- All six reviewed C++/header pins match the isolated originals, their
  frozen inventory records, and current Main. No source was modified.
- The all-twelve-payload literal survey finds the preserved historical
  `AUDIT=...reference_readiness/local/rfa`, the two correct isolated driver
  literals, and the two incorrect Python runtime literals. Historical audit
  and prior-source references must retain their existing identities.

Root's three actual inspector records report closed, exit-zero processes
and exact requested/actual maps: external host 14 keys, root Python 16,
effective Python 16. The peer compared those private maps as data with the
three selected profiles; no private values are published. External equals
base; root equals effective. Derivation from the selection matches effective,
but derivation from the recipe differs in exactly `INCLUDE`. Root's own TEXT
before/after records are identical and its process closure reports exit zero,
no timeout. These are Root-owned observations, not worker runtime evidence.

The held proposal retains an inherited initial-file count of 12,063; the
concrete selection uses 12,051, agreeing with the fresh capture. The peer
does not adopt the proposal's empty inventory or null Root receipt fields.

The peer's fresh ignored family is `local/t5isolatedPeer01`. It contains 50
fixed data copies, validation sources/results, and one retained own-utility
parser failure corrected in a separately named source before a successful
passive review. Fixed originals and copies matched at capture and review
bookends. The peer makes no full worker runtime-closure claim. Its seal has
57 artifacts, exactly 58 recursive files including the seal, SHA-256
`41857c02febd2ef72d99f343872c30f230ea4f7f84f87caea3f7fd0ac05d524e`.
V5 selected payloads match its sealed ledger; no whole-V5 rehash is claimed.

The next proposal must jointly correct the fixed family, guarded BSP literal,
selection include root, and both complete 16-key profiles in a fresh namespace.
Keep every rejection branch, use a fresh exact future family, preserve consumed
families, and obtain new Root admission/receipts. No Native query, compiler,
provider, original-PE or target execution was performed by this peer. The
initial absence of `type5p2` was observed before Root's separate materialization;
it is not asserted as the final filesystem state. No C++ build was appropriate
for this two-file documentation/data packet.
