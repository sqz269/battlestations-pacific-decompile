# OCT18B library binding B: sorting instantiations

Audit date: 2026-10-18 PDT. Base: `3d581c328`, worktree
`agent/oct18b-libB-sort`. Scope is library binding and substitution review;
there are no host changes, reconstructed bodies, game runs, or Ghidra writes.

The session-browser tie order is an implementation dependency, not an observed
modern-STL defect. A source-family probe found **zero identity-order differences
in 640 cases** between the repository's preserved MSVC9 sort and the installed
modern MSVC `std::sort`. Nevertheless, the browser uses the resulting order both
for visible rows and for last-matching-name selection, so an arbitrary future
sort implementation is not justified by the standard's unspecified tie order.
The existing preserved template provides a small source-level way to pin it.

## Evidence and meaning of the classifications

The six sampled rows below group related helpers rather than crediting every
heap/partition routine separately. They cover seven original introsort entry
points. Counts are **bind-safe 1, layout-bound 1, behaviour-sensitive 3,
unreached 1**. The categories apply to the stated use range, not all template
arguments or every possible input to an address.

* **Bind-safe:** the audited caller's value ordering can use the stated stock
  source operation, with its adapter and explicit bound. This is not binary ABI,
  allocator-failure, or original exception-path certification.
* **Layout-bound:** an algorithm binding already exists, but native element
  storage, accessors, or ownership still require the existing adapter.
* **Behaviour-sensitive:** input identity order, nonstandard comparison, or an
  uncovered branch is relevant to state/output; do not replace by an arbitrary
  conforming sort on the strength of its name.
* **Unreached:** no external static reachability was found for the stated
  producer in this sample. Computed runtime dispatch is not ruled out.

`config/target.json` selects `C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86:LE:32, image base `00400000`. The live count
matched the 76,774-function snapshot. A read-only typed-flow capture verified
the target and was accepted at modification 982; retained as
`local/oct18b_sort_target_identity.json`. Original image SHA-256:
`B682A82C52F81F957B2C70222077305A933F72481686C88843077F714B956DD6`.

Original bodies were read through `bsp.py show`; register-ABI and floating
comparison questions were checked against assembly/disk disassembly. Caller
queries were supplemented by both existing scanners described below. Current
source and installed MSVC headers were inspected locally; no web assumptions
or live original-sort execution are used.

## Sampled families

| Family and original entry | Original concrete consumer | Current body/binding | Classification |
| --- | --- | --- | --- |
| Session records, `00574A50`, A0h stride | `005789C0`, call `00578ECB` | `src/menu_screens_2.cpp`, `session_browser_build_rows_005789c0`, modern `std::sort` | **Behaviour-sensitive:** equal-key row order influences display and duplicate-name selection; no observed mismatch in the bounded probe. |
| Rewards rows, `00625D10`, 60h stride | `0062C2C0`, call `0062D43B`, immediately after `0062A0B0` | `src/ingame_gui.cpp`, `rewards_sort_score_rows_00625d10`, `std::stable_sort` | **Bind-safe for the concrete <=8-row scoreboard caller:** original uses stable insertion sort. Generic >32 input is a different contract. |
| Widget-pointer twins, `00AA4E00` / `00AAA480` | `00AA5A00` at `00AA5AFD`; `00AAAED0` at `00AAB3F1` | `include/bsp/gui_widgets_cc13.hpp::msvc9_sort`; `src/gui_widgets_cc13.cpp::sort_widgets_by_name`; `src/gui_render_order.cpp::collect_sorted_children_00aa5a00` | **Layout-bound:** already pins the original algorithm and pointer identity permutation; keep native-name/accessor and storage adapters. |
| AI defend records, `00A28300`, 34h stride | `00A28A60`, call `00A290F5` | `src/gameplay_seg69_70_cc13.cpp::ai_defend_sort_records_00a28300`; `src/game_hosts_ai.cpp` caller | **Behaviour-sensitive:** asymmetric threat/weight relation and a known >32-record gap. |
| Render-stat records, `00B148E0`, 12Ch stride | `00B151E0`, call `00B1527A` | `src/scene_renderer_cc13.cpp::stats_overlay_00b151e0`, modern `std::sort` | **Behaviour-sensitive, diagnostic priority:** in-place identity order and diagnostic row placement; no observed mismatch in the bounded probe. |
| Lua distance pairs, `008C9E10`, 8-byte stride | `008CCEB0`, call `008CD2D7`; producer itself has zero external refs found | `src/cc13_left55_seg56c.cpp::lua_sort_by_distance` delegates through `Left55Seg56cHost::sort_scored_entries_008c9e10` | **Unreached in the static sample:** retain the explicit binding; equidistant identities affect a returned Lua array if wired. |

### Session browser: ties are part of a concrete consumer contract

`0056A870` supplies six keys and no secondary session-ID comparison:
case-insensitive wide name; unsigned game mode at +54h; descending max players
at +2Ch; descending unsigned free slots (+2Ch minus +28h); descending +34h;
and descending unsigned `16 * max_players + players`. Several unrelated
sessions can therefore compare equal. Duplicate names, case variants, equal
capacity, equal population and the same game mode require no corrupt data.

`00578E83..00578ECB` checks the range, calculates `(end-begin)/A0h`, and passes
the key, count, ECX=first and EDX=last to `00574A50`. That body inserts through
32 elements and otherwise partitions with heap fallback. The current source's
comment that ties have no particular order is insufficient as a parity argument.

The sort feeds an optional complete reversal and then row creation in order.
At `005796E0` the original appends a row to listbox +30h. At
`00579755..00579786` it tests equal name length and `_stricmp == 0`; at
`005797B1` every match writes screen+26Ch. There is no first-match break. Thus
the **last equal-name row** becomes the selected-row pointer when rebuilding
from the previously selected name. An altered permutation can change both the
visible row order and which duplicate-name row gets this pointer. This is a
conditional implication, not a demonstrated divergence in today's MSVC.

Recommendation: preserve the existing comparator and use the already-present
`msvc9_sort` through a small source adapter if exact identity ordering is wanted.
Estimated effort: 15-30 minutes plus focused validation; no new vendor body is
needed. Review comparator/locale fidelity separately from sorting permutation.
The source-family probe covers the actual `SessionJoinRecord` game-mode key;
it does not certify all six keys, names/locales, allocation failures or all
input arrangements.

### Rewards: the original caller establishes the useful bound

`00625D10` is an introsort, not a general stable-sort function. It uses insertion
helper `00623410` for counts <=32, partition `00624E40`, and heap helpers for
larger/depth-exhausted ranges. The signed integer score is at row+50h.
`00623457`, `00623497` and `006234B1` move a new row only past strictly lower
scores (`JLE` stops, `JG` continues), preserving equal-score order.

The original `0062A0B0` clears the vector and considers player slots 0..7,
appending at most one row per slot. Its final loop increments the slot and exits
on `7 < slot`. The concrete `0062D43B` sort follows this builder. Therefore the
large-input path is dormant for this audited scoreboard use, and modern
`stable_sort(score descending)` has the needed equal-score value ordering here.
The generic source function accepts arbitrary vector sizes; its >32 behavior
must not be advertised as the original generic sorter. The source-family probe
demonstrates that generic large-input distinction.

Recommendation: retain the current stock operation for this bounded caller,
document/enforce the bound at its public use contract if widened later. Estimate
10-20 minutes for the documentation/guard decision; no reconstruction needed.

### GUI twins: preserve the binding that already exists

The original element is a 32-bit widget pointer. The name predicate uses a
widget vtable +28h native-string accessor and case-insensitive comparison
(`00AA2420` and twin `00AA7810`). `GuiWidgetNameLess` deliberately fetches the
right name before the left. Equal names need not mean equal widget identities.
`00AA5A00` copies child-list payload pointers at widget+68h into a snapshot,
then sorts; `00AAAED0` also uses the twin during property description.

The existing `msvc9_sort` carries median selection, partition, heap and insertion
operation order. `GuiSnapshotChild` swaps a widget/name pair, so a pure name
lookup can preserve pointer permutations. This is an adapter claim, not native
pointer-vector or NativeString layout compatibility. `std::stable_sort` would
be inappropriate for generic >32 equal-name inputs; the probe demonstrates it.
Recommendation: keep this binding. Adapter/contract review is 15-30 minutes;
there is near-zero new reconstruction effort.

### AI defend: do not normalize this relation into a stock comparator

The insertion helper `00A25750` reads new-record threat +4h and weight +8h.
Assembly `00A25800..00A25820` and `00A258C3..00A258D7` moves the record only
when its threat is above zero and its weight exceeds the predecessor's weight;
unordered comparisons stop movement (`JBE`). The existing <=32 source model
preserves that insertion sequence.

This is not an unrestricted strict weak ordering. For finite keys, let
`A=(positive threat, weight 1)`, `B=(zero threat, weight 2)`,
`C=(positive threat, weight 3)`. A and B compare equivalent, as do B and C,
while C precedes A. Treating the relation as a general stock `std::sort`
predicate would therefore require an extra input invariant that is not proved.

The original caller sorts at `00A290F5` before traversing records. Its fallback
anchor scan keeps a candidate only on strict smaller squared distance, so the
first equal-distance record wins. It subsequently deducts group resources from
that record and issues defend/patrol commands. Ordering can affect game state.
The current source returns `nullopt` above 32 records, and `game_hosts_ai.cpp`
then retains construction order. This is an explicit uncovered branch; actual
>32-record gameplay occurrence was not tested or proved here.

Recommendation: keep the <=32 sequence and label the uncovered range. Recover
the remaining native partition/heap comparison behavior before widening it;
do not replace by an ordinary weight-only sort. Estimate 0.5-1 day for bounded
recovery and focused branch evidence, longer if exact x87/NaN behavior is needed.

### Render statistics and Lua distance pairs

For render statistics, `00B0DAA0` compares the native string at +124h: empty
sorts before non-empty; otherwise `_stricmp < 0`. No identity tie-break exists.
The current source sorts `host.records_690()` in place and iterates it into
diagnostic rows with incrementing y/column positions. Distinct effect records
with equal/CaseFold-equal sort keys are not excluded by the comparator. Modern
`std::sort` matched the preserved template for the sampled `RenderStatsRecord`
keys. The current header records the original debug-print sink as a bare RET;
this is a diagnostic/array-order concern, not proof of a visible native-game or
gameplay difference. Recommendation: pin the existing family if diagnostic
identity order matters, below session-browser/AI priority; 15-30 minutes.

For Lua distance pairs, `0088A720` assembly compares only the float at pair+4;
the dword at +0 is an index carried with it, not a tie-breaker. Positive finite
equidistant points naturally tie. `008CCEB0` sorts and creates a Lua table with
`table[i+1]=objects[sorted[i].index]`, so identity order would be exposed if used.
Its current C++ body keeps the explicit sort host binding; it does not silently
substitute a modern sort. `bsp.py callers 008cceb0` and its rel32/absolute-dword
census both returned zero. It is also absent from the queried
`config/lua_bindings.json`. Recommendation: park pending a concrete registration
or caller; binding-adapter review would be 30-60 minutes once a consumer exists.

## Original family versus installed stock source

The probe used VS 18 Community MSVC **14.51.36231**, `_MSC_VER=1951`,
`_MSVC_STL_VERSION=145`, `_MSVC_STL_UPDATE=202604`. The relevant installed files
are `include/algorithm` and `include/__msvc_heap_algorithms.hpp` under that
toolset directory. Older installed 14.44 headers were inspected initially;
the decisions and executable probe use 14.51.

| Mechanism | Original/preserved family | Installed 14.51 stock source |
| --- | --- | --- |
| Small range | insertion sort through 32 | `_ISORT_MAX=32`, `_Insertion_sort_unchecked` |
| Pivot | middle, first and inclusive last; ninther if inclusive endpoint difference >40 | same median positions and threshold in `_Guess_median_unchecked`; no differing threshold/pivot rule found |
| Budget | `ideal/2 + (ideal/2)/2` | `(ideal>>1)+(ideal>>2)`, same for valid nonnegative counts |
| Recursion/fallback | smaller half recursed; heap when ideal expires | same control shape |
| Insertion/movement | preserved generic helper uses rotate; some native element instantiations use copied temporary plus backward assignments | moved temporary/backward assignments; different move/copy, exception and allocation operations |
| Partition | preserved helper includes self-swaps | stock skips two self-swap cases; stock median also avoids a redundant comparison; side-effectful callbacks require separate review |
| Heap | original copies and chooses child through strict comparison | same observed child/tie rule, modern move operations and overflow guards; not a native-ABI binding |
| `stable_sort` | sampled native entries above are introsorts | stable insertion <=32; buffered divide/merge above 32; this changes tied identities in the probe |
| `partial_sort` | no separately recovered candidate established in this sample | heap-select then sort selected prefix; cutoff-equivalent identities require their own proof |

`find Partial`/`find Stable` found no sorting entry with those explicit names;
there is no current `std::partial_sort` source call in the searched tree. This
is discovery evidence, not an all-binary absence claim. A merge-like candidate
`00499E70`, tagged `STL_List_Sort_00499E70`, takes two list objects and a
comparator and has three byte-census callsites (`0049F789`, `0049F7CB`,
`0049F82E`). It has no current reconstructed body in its lookup. Its tag alone
does not prove an original `std::stable_sort` instantiation; element/comparator
and enclosing-caller ownership were deferred in this worker packet. Primary
follow-up traced the driver to landing-troop disembark `004A0D20` at `004A2BFC`,
using `0047B9F0` on value+8h; the current host method is explicitly unreached.
See S7 in [the combined audit](OCT18B_LIBRARY_BINDING_AUDIT_B.md) for that
additional decision, outside this worker's six-row count. The resource-dump function
`00B811C0` is an adjacent **qsort** to current `stable_sort` substitution, not an
STL stable-sort example; its tie behavior belongs with the qsort audit.

## Caller census and reproducibility

Both scanners read the installed original image. `callsite_census.py` scans
`.text` for E8/E9 and `.text/.rdata/.data` for literal target dwords;
`image_ref_scan.py` additionally checks absolute dwords in every PE section.
For the seven introsort entries they agree on **29 rel32 CALL sites, zero JMP
sites, zero absolute-dword hits**. All seven have a concrete direct game-caller
site. This is an exact-target byte census, not complete computed-dispatch proof.

| Sort entry | `bsp.py callers` owners | All rel32 sites (recursive sites grouped) | Absolute dwords |
| --- | --- | --- | --- |
| `00574A50` | self; `005789C0` | self `00574AF1/00574B04`; `005750E9`; game `00578ECB` | 0 |
| `00625D10` | self; `0062651E`; `0062C2C0` | self `00625DA7/00625DB4`; `006265E4`; game `0062D43B` | 0 |
| `00AA4E00` | self; `00AA5A00` | self `00AA4E6B/00AA4E7E`; `00AA5258`; `00AA5726`; game `00AA5AFD` | 0 |
| `00AAA480` | self; `00AAAED0` | self `00AAA4EB/00AAA4FE`; `00AAA598`; game `00AAB3F1` | 0 |
| `008C9E10` | self; `008CCEB0` | self `008C9E6D/008C9E7A`; `008CACE3`; game `008CD2D7` | 0 |
| `00A28300` | self; `00A28A60` | self `00A28397/00A283A4`; `00A28704`; game `00A290F5` | 0 |
| `00B148E0` | self; `00B151E0` | self `00B14977/00B14984`; `00B151B4`; game `00B1527A` | 0 |

The extra byte hits must not be dropped because they are missing from Ghidra's
call graph. The scanner labels an enclosing owner by nearest preceding known
start; that is not a function-boundary proof. For example `005750E9` is a real
call in a sort wrapper (disk disassembly shows the A0h count calculation and
`RET 14h`) despite the preceding inventory owner being labelled tree-destroy.
Other extra sites remain wrapper/ownership candidates; no caller-closure claim
is based solely on those preceding names.

Reproduce the scans from the worktree root:

```powershell
python tools/callsite_census.py 00574a50 00625d10 00aa4e00 00aaa480 008c9e10 00a28300 00b148e0
python tools/image_ref_scan.py 00574a50 00625d10 00aa4e00 00aaa480 008c9e10 00a28300 00b148e0
python tools/callsite_census.py 008cceb0 00499e70
```

Retained ignored artifacts: `local/oct18b_sort_callsite_census.txt`,
`local/oct18b_sort_image_refs.txt`, `local/oct18b_sort_boundary_callers.txt`,
`local/oct18b_sort_family_probe.cpp`, `local/run_oct18b_sort_family_probe.ps1`,
and `local/oct18b_sort_family_probe.log`. The probe compiles Win32 with
`/link /MANIFEST:EMBED` and uses the **actual repository** `SessionJoinRecord`,
`RewardsScoreRow`, `GuiSnapshotChild` and `RenderStatsRecord` classes. Each gets
16 deterministic repeated-key arrangements at lengths
0,1,8,31,32,33,40,41,64,129 (160 cases each). It compares identity sequences
after preserved `msvc9_sort`, stock `sort`, and stock `stable_sort`.

Result: all four classes had `modern_identity_diffs=0/160`;
`stable_identity_diffs=80/160`, first difference at length 33. This is a
source-family comparison over pure finite/ASCII keys, **not original machine
code execution, whole-algorithm proof, ABI validation or game validation**.
No NaN comparator was passed to a stock sort. Source SHA-256:
`7D6830AC6E5835323BD7DB279866AF8C1C507D7ADDC8474DCE2AA6860DB614A0`;
probe EXE SHA-256:
`767924B759FA50A05A25ECC28D077B01C517860513DC9B28327211712447F6C7`.

This report and four name-ledger evidence additions are committed. Existing
names/evidence are preserved for `00574A50`, `00625D10` and `008C9E10`;
`00A28300` receives an evidence record under its existing Ghidra/tag name,
`STL_IntroSortLoop52_00a28300`. These are audit notes, not newly reconstructed
bodies. The primary integrator owns final audit tags, Ghidra comments and
integration; this worker made no live Ghidra edits.
