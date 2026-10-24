# October 24 board tail and wiring audit

All 1,744 original unknown-identifier rows are accounted for. Four pool bodies
were reconstructed; unsupported completion claims were corrected rather than
counted as implementations. The remaining 179 unknown identifiers are reserved
to cc13 (178) and cc9 (one). There are 352 unfinished target functions after
correction: 322 in the pool and 30 outside it.

The baseline is `515837c10`; the final analysis snapshot is `fdcabdd47`, using
the refreshed index from the shared 2026-10-19 Ghidra snapshot. These are
source-level estimates, including disabled switches. They do not establish
runtime wiring, native ABI compatibility or game validation. No game was run.

## Before and after

| Board KPI | Before | After |
| --- | ---: | ---: |
| target | 20,726 | 20,597 |
| game_done | 20,714 | 20,245 |
| pool_remaining | 12 | 322 |
| game_outside | 316 | 289 |
| unreferenced | 791 | 893 |
| functions with body credit | 24,613 | 24,108 |
| audited_no_body | unavailable | 509 |

Four additions and 473 withdrawn game-body credits explain the change in
`game_done`. The 509 audited no-body addresses include both game and excluded
records. Additional scope exclusions (27) and newly unreferenced rows (102)
explain the target decrease of 129. The larger remaining count exposes old
unsupported completion claims; it does not represent removed working C++.
Of the 322 pool gaps, seven belong to the original twelve and 315 were exposed
by this audit.

| Wiring state | Before | After |
| --- | ---: | ---: |
| wired | 8,217 | 8,228 |
| ready | 1,761 | 1,789 |
| callees_done_no_wired_caller | 10,445 | 9,772 |
| unknown_no_identifier | 1,744 | 179 |
| blocked_on_callees | 94 | 95 |
| total | 22,261 | 20,063 |

The wiring denominator now applies the board's compiler/library exclusions and
no-body status rules. State changes therefore include scope corrections and
better identity resolution; they are not newly enabled runtime paths. Final
file reachability covers 1,855 of 2,785 source files from 30 roots.

## The original twelve pool rows

[OCT24_TAIL_POOL.md](OCT24_TAIL_POOL.md) records the addresses, assembly evidence,
ABI qualifications and owner decisions for every row.

| Addresses | Outcome |
| --- | --- |
| `00407590`, `00407690`, `0040F910`, `0040FA10` | Added independent Mit pool construction and global destruction bodies |
| `00922DD0` | cc9 reservation; `XOR DL,DL; JMP 00922C80` still needs its concrete host implementation |
| `009C7240`, `009C7270`, `009D9CC0`, `009EACA0`, `009EC770` | cc13 segment-67 reservation; no ledger edits here |
| `00A9DF40` | Listbox remains partial: native base/member allocations and SEH are open |
| `00BF3ACC` | Non-callable continuation of the already reconstructed 399-byte parent `00BF3A80`; excluded as an independent target |

The four new bodies preserve pool storage, registry publication, table reserve,
signed lock-depth draining and page/table deallocation distinctions, with
explicit CRT services. They do not claim native register ABI or FH3 transport.
Ghidra names and evidence comments were updated under the write lock, saved,
and re-exported; the previous annotation values were journaled.

`009EC770` already has a complete source body but retains fragment-only credit;
cc13 can decide its ledger backfill. The saved Ghidra split at `00BF3ACC` was
preserved along with its parent evidence.

## All 316 original outside rows

`game_outside` means a target has NULL `functions.segment`. It does not mean
missing implementation. The board also admits bodies without incoming edges;
withdrawing an unsupported body claim can therefore move an address from
outside to unreferenced.

| Original outside category | Count | Disposition |
| --- | ---: | --- |
| Existing implementation records | 255 | Correctly outside the assigned pool |
| Recorded-only or table-only claims | 54 | Withdrew unsupported body credit |
| Explicit partial bodies | 4 | Remain unfinished targets |
| Compiler boundary projections | 3 | Excluded with exact listing evidence |

The 54 include the 53 entries explicitly recorded as not reconstructed plus
render table-only row `00B4B480`. Thirty remain referenced missing-body targets;
24 become unreferenced. The compiler starts `00CB89AD`, `00CC1A01`, `00CC1B36`
are each ten bytes: `MOV EAX,FuncInfo; JMP __CxxFrameHandler3`, with only
parent-prologue/EH references. Their parent cleanup projections remain intact.
Thus the final outside set is 255 existing implementations, four partial
bodies and 30 missing bodies: 289 total.

[OCT24_TAIL_OUTSIDE.md](OCT24_TAIL_OUTSIDE.md) contains the original exhaustive
classification and edge samples; its initial 256/53 split was refined above
when the render audit established that `00B4B480` was table-only.

## All 1,744 original unknown identifiers

[OCT24_TAIL_IDENTIFIER_AUDIT.jsonl](OCT24_TAIL_IDENTIFIER_AUDIT.jsonl) has one
record per original address, including baseline identity, ledger status,
disposition and resolved source/symbol or scope/owner evidence.
[Its metadata](OCT24_TAIL_IDENTIFIER_AUDIT.meta.json) records category totals,
final states and hashes of the actual source, map, tools and manifests.

| Disposition | Count |
| --- | ---: |
| Verified C++ body resolved | 786 |
| No-body completion credit withdrawn | 436 |
| Incomplete or unverified completion credit withdrawn | 20 |
| Classified out of scope | 280 |
| Existing board exclusion | 43 |
| Reserved to cc13 | 178 |
| Reserved to cc9 (`00957740`) | 1 |
| Total | 1,744 |

The 20 incomplete/unverified host claims retain their partial code, contracts
and original qualifications. They lack a verified complete implementation;
this does not assert the absence of every related semantic operation.
Corrections preserve the original ledger fields and add reasons. Audited
statuses `metadata_only_no_function_body_verified_oct24` and
`incomplete_or_unverified_body_credit_withdrawn_oct24` no longer give body
credit, including through stale source-history evidence.

Seven `config/wire_identifiers*.json` manifests hold 754 verified mappings;
743 are used by the explicit-map resolution path, with the rest resolved by
earlier methods. The resolver checks real definitions, evidence, duplicate
addresses and different-address aliases. Shared template implementations
require an explicit evidence flag and retain existing ABI/layout limitations.
Alternate function records, multiline constructor initializers and qualified
or decorated definitions are now recognized. Common method names count uses
only in their own implementation file when they collide; type mentions do not
prove implicit destructor execution.

All identifier work used manifests, ledgers or reporting tools. It changed no
`src/` or `include/` files. The only C++ changes in this lane are the four pool
bodies and their declarations. The low-address packet originally supplied 152
maps; four in cc13's segment-40 band were removed, leaving 148. Its deferred
locale contracts were subsequently audited by the primary.

No reconstruction ledger record in cc13's reserved bands changed against the
baseline: `00728A90..00735EC0`, `008E3DE0..008EE5F0`, and
`00996120..009F6060`. The final cohort has no unexplained unreserved row.

## The original 791 unreferenced rows

The categories are 743 `block_dyn`, 14 game-code, 33 untagged and one trivial.
Of these, 728 have no indexed incoming edges; 63 have only raw unwind/thunk
tail transfers (138 unwind sites and one thunk site). Category samples through
`bsp.py callers` found no omitted ordinary direct, vslot, data or Lua edge.
This establishes incoming-edge status, not root reachability or proof that EH
cleanup cannot execute.

The eight referenced undefined chunks were also checked. `007B3AA0` is a
credible missing-body candidate: a 128-byte chunk referenced by vslot 89 at
`00D05634`, with four outgoing calls, outside the function denominator.
`00529120`, `00530A60` and `006E5F10` already have tail/fragment/reference-model
coverage; `00A82100` is self-call-only; `00BF5752`, `00BF86E0`, `00C2E020`
are excluded CRT/import entries. No unreferenced or undefined body was
reconstructed in this lane.

## Verification and reproduction

MSVC Win32 Release passed after integration, with both configured root CTest
targets passing. The pool worker's build also passed its three configured
targets, including native math differential checks; eight seed matches and
eight direct-call rows were verified. No focused lifecycle execution or
native pool differential test was performed. These checks support compilation
and existing fixtures, not native ABI or gameplay equivalence.

All modified Python tools compile. A focused batch-ledger check verifies that
invalid or duplicate input writes nothing and valid batches check every lease
before mutation. All 754 manifest entries validate against 66 source files
with zero errors. The complete cohort audit passes with zero unexplained
unreserved rows; the outside audit agrees exactly with the frozen board.

Ignored before/after reports are in `local/output/oct24_{board,wire}_{before,after}.json`;
`wire_readiness.json` is the final wiring report. Preserved worker build,
call-check, annotation-journal and original outside/unreferenced cohort evidence
is under `local/evidence/oct24_pool/` and `local/evidence/oct24_outside/`.
Worker worktrees were safely removed through `bsp.py worktree remove` after
integration, with their local inputs archived by that tool.

For exact comparison, use the analysis snapshot `fdcabdd47` and its indexed
Ghidra inputs. Commands run against later ledgers or source report later state:

```powershell
python tools/bsp.py index --if-stale
python tools/progress_board.py --history-ref fdcabdd47 --json local/output/oct24_board_after.json --no-index
python tools/wire_readiness.py
python tools/audit_tail_identifiers.py
python tools/audit_board_outside.py --history-ref fdcabdd47 --board local/output/oct24_board_after.json
```

The remaining owner work is explicit: seven original pool rows, 179 reserved
identifier rows, 352 unfinished targets overall, and the separately identified
undefined vtable candidate. These sets overlap and must not be summed.
