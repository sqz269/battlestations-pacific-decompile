# Deleted shard rows and merges; four shader-band listing repairs (cc13-tombstone1)

Status: tooling change in Python only (no C++ touched, so no build), checked on scratch repositories
and on a replay of the faulty merge. Listing repairs applied to Ghidra and exported. Nothing here is game-validated.

## 1. What went wrong in `044466966`

`docs/CC13_STL_PROBABLE_REVIEW.md` section 9.4 reports 19 tag rows that cc13-stlreview1 dropped
(`009042B0`, the 15-row `00968710` cluster, `0096BF70`, `0096CE00`, `0096ED00`) coming back after
merge `044466966` (origin/main into agent/cc13-sweep9x). Tracing the merge:

| shard | base `11a736da9` | ours `cef29989a` (sweep9x) | theirs `02e21a51f` (origin) | merge result |
|---|---|---|---|---|
| `config/tags/00960000.jsonl` | 232 lines | 285 (+53 appended) | 214 (53 rows rewritten, 18 deleted) | 285 |
| `config/tags/00900000.jsonl` | 95 lines | 152 (+57) | 94 (rows rewritten, 1 deleted) | 152 |

The result has exactly as many lines as ours: every row origin deleted came back. No `.gitattributes`
union driver is involved (the file sets only `text`/`eol`), and git's own text merge never brings back
a deleted line. `tools/merge_resolve.py` had **no handler for `config/tags`**: the generic
same-spot and diff3 pure-insertion fallbacks both refuse when the base region of a conflict is
non-empty, so they refused. The worker then resolved by hand "per address (origin's reviewed rows
kept where origin changed a row, this branch's new rows kept otherwise)", per its merge message. A
per-address rule with no base comparison cannot see a deletion: an address origin removed is simply
absent on one side and present on the other, and "keep it otherwise" restores it.

The other two shard families were already safe:
- `config/names`: three-way by address (`o == b` takes theirs, which may be absent, so a deletion
  holds).
- `config/reconstruction`: line-level; a side that deleted or rewrote base rows is kept whole and
  only the other side's new rows are replayed, so a deletion holds. It refuses when both sides
  rewrote anything.

## 2. Design

New `three_way_rows(base, ours, theirs, path)` in `tools/merge_resolve.py`, used for
`config/tags/*.jsonl` conflicts. Three-way, no format change, so `bsp.py index`, `ghidra_tag.py` and
`ledger.py` are unaffected. Tombstone rows were not used: every reader of the shards would need to
learn to skip them, and the merge base already carries the information.

- A row is one line, matched by canonical JSON (`sort_keys`), so key-order differences do not
  count as edits. The emitted line is the original text, so no unrelated churn.
- Rows are a multiset. Per row: both sides agree → that count; only one side changed the count →
  that side; both added it (base 0) → once. A base row present on one side only is therefore
  deleted. A repeated base row whose count both sides changed differently → refuse.
- Conflict check per address: if the two sides' changes at an address differ and, taken together,
  include both a deletion and an addition, refuse. That covers delete-versus-edit (an edit is a
  delete plus an add) and delete-versus-new-row (for example a review drop against a sweep re-tag
  of the same address). Deletions alone on both sides compose; additions alone union (current
  behaviour for pure appends, including two different new rows at one address, as at `00A44840`).
- Order: if base, ours and theirs are each sorted by address the result is sorted (stable, ours
  first within an address); otherwise ours' order with theirs' new rows appended. 15 of 136 tag
  shards are not address-sorted on main, so re-sorting them would make a large diff.
- A refusal prints the address on stderr and leaves the file conflicted, as the other handlers do.

## 3. Tests

`tools/merge_resolve_tags_check.py` (about 100 lines, no framework) builds a throwaway git
repository per case, makes a real conflicting merge and runs `resolve_all`:

| case | outcome |
|---|---|
| both sides append rows at one spot | PASS, resolved, union of both appends |
| one side deletes 3 rows, the other appends next to them | PASS, resolved, rows stay deleted (old tool: refused) |
| one side deletes a row, the other edits the same row | PASS, refused and reported (`00961400 deleted on one side and changed on the other`) |
| deletions + an unrelated edit + appends on both sides | PASS, resolved, deletions held, edit and appends kept |
| both sides delete different rows at one address | PASS, resolved, both deletions held |

Replay of the real merge (base `11a736da9`, ours `cef29989a`, theirs `02e21a51f`) through
`three_way_rows`: `00960000.jsonl` resolves to 267 rows and `00900000.jsonl` to 151 rows (285-18 and
152-1), and none of the addresses origin dropped come back. No address in that merge had a
delete-versus-edit conflict, so the merge would have resolved mechanically.

## 4. Scan of origin/main for other resurrected rows

Two independent checks, each run twice on 2026-10-12 against origin/main, the last time at
`d0ed99869`, same results:

1. History scan (`local/tomb_scan.py`, not committed): walk every non-merge commit touching
   `config/tags`, `config/names` and `config/reconstruction` oldest first and remember each row's
   last add/delete event (canonical JSON). A row whose last non-merge event is a deletion yet which
   exists at the target can only have come back through a merge. Control: at `044466966` the scan
   finds exactly the 19 known rows (all tags). At origin/main: **0 rows** in all three families.
2. Decision scan (`local/tomb_decisions.py`): 1752 addresses carry an `upgrade_shape`,
   `upgrade_named`, `reviewed_game` or `drop_game` decision in `stlr_decisions.txt`,
   `stlr2_decisions.txt` or `stlr3_decisions.txt`; each of those removed the `stl_probable` row
   (`stlr_review.py`). **0** of them still carry an `stl_probable` row on origin/main. The st1
   decision files (`st1_decisions.txt`, `st1_retag_006d.txt`) are group renames with no drop
   decision, so they define nothing to re-drop.

Nothing was re-dropped: the 19 rows were already removed again by cc13-stlreview2. One duplicate
remains and is not a resurrection: `00A44840` has two `stl_instantiation` rows with the same name
from two independent adds (stlreview1 `cc13_stlreview1_named_instance`, cc13-sweepA1 `cc13sA1s`).
It was left alone, as section 9.4 of the review did.

## 5. Who benefits

- `tools/integrate_workers.py` calls `merge_resolve.resolve_all` directly.
- The lead's `J:\PROG\battlestations-pacific-decompile-cc13\local\cc13_land.ps1` runs
  `python tools/merge_resolve.py $tree` after a failed `git merge` (both `Merge-OriginMain` and each
  worker merge), with the working directory set to the cc13 tree. It picks up the new handler once
  the cc13 tree has merged this branch; until then a tag-shard conflict throws there, as before.
  The script then stops on a refusal (`throw`), so a delete-versus-edit tag conflict now stops the
  land instead of being unioned.
- Worker merges of origin/main: when the tool refuses a tag shard, do not union it by hand
  (note appended to `docs/COORDINATION.md`).

## 6. Listing repairs (from `docs/CC13_SHADER_VTABLES.md` section 4)

Applied with `tools/ghidra_define_function.py --delete-inside` (Ghidra write lock; bytes checked
against the disk image; explicit disassembly, no flow discovery), event log
`reports/cc13_tombstone1_defined.json`. Names and evidence comments through
`tools/ghidra_annotate.py --apply` (prior values in the tree's `local/ghidra-annotations-20261012T145938Z.json`).
Reference counts from a scan of the image for CALL/JMP/Jcc rel32 and absolute dwords (positive
control `00B483A0`: 2 hits, `00B48471` and `00B489EE`); all four entries have 0.

| entry | before | after | name (provisional) |
|---|---|---|---|
| `00B40A10` | no function; `FUN_00B40A20` body `00B40A20-00B40A2E` (loop head) | `00B40A10-00B40A2E`; `FUN_00B40A20` deleted | `BSP_MaterialPassSlab_StampSlabIndex_Unreferenced` |
| `00B47E80` | no function; `FUN_00B47E90` body `00B47E90-00B47E9E` (loop head) | `00B47E80-00B47E9E`; `FUN_00B47E90` deleted | `BSP_NativeVertexDeclarationSlab_StampSlabIndex_Unreferenced` |
| `00B405E0` | undefined code; `BSP_Float4Block_At_Unreferenced` `00B405D0-00B405DD` ends before it | `00B405E0-00B406A9` | `BSP_Float4Block_SetMatrixRowsAt_Unreferenced` |
| `00B48460` | undefined code | `00B48460-00B48475` | `BSP_HardwareLayoutRecordQuad_PushAssign_Unreferenced` |

Bodies (disk bytes, `bsp.py disasm-raw`):
- `00B40A10` / `00B47E80`: `__thiscall(this, value)`, `RET 4`. `MOV EDX,[ESP+4]; LEA EAX,[ECX+88h|0D0h];
  MOV ECX,20h`, then 32 stores of EDX stepping `8Ch|0D4h`: the dword at `+88h|+0D0h` of each slot.
  "StampSlabIndex" is the shader doc's name; what the stored value means is not established.
- `00B405E0`: `__thiscall(this, int index, const float *matrix, int rows)`, `RET 0Ch`; destination
  `this+4+index*10h`. rows 4 → `00B404A0` `BSP_ShaderConstants_WriteTransposedMatrix`; rows 3 →
  `00B40440` `BSP_Float3x4_FromMatrix44Transposed`; rows 2 → inline copy of matrix columns 0 and 1
  (`m[0],m[4],m[8],m[12],m[1],m[5],m[9],m[13]`) via a 20h stack buffer and `REP MOVSD` of 8 dwords;
  other counts write nothing. The shader doc listed the arguments as `(i, rows, matrix)`; the
  stack reads (`[esp+0Ch]` at entry is the count) give `(index, matrix, rows)`.
- `00B48460`: `__thiscall`, stack arguments forwarded; `n = [this+30h]`, `[this+30h] = n+1`,
  `JMP 00B483A0` `BSP_HardwareLayoutRecord_Assign` with `ECX = this + n*0Ch`. No bounds check.

Checks: `bsp.py ghidra proto 00BF9DC8` read `void __cdecl _free(void * _Memory)`, body
`00BF9DC8-00BF9E55`, before and after. 'Non-Returning Functions - Discovered' was not touched.
`bsp.py snapshot --force` then ran (function count 76706 → 76707, call graph and index rebuilt), and
`lookup 00B40A20` now reports the enclosing `00B40A10`. `tools/verify_report_calls.py` does not
apply: it reads JSON call-site reports, and this doc has none.

## 7. Open

- `config/reconstruction` still refuses whenever both sides rewrote any base record anywhere in a
  shard; `three_way_rows` could replace that rule, but its "two competing new records at one
  address" refusal would need keeping. Not changed here.
- The other `cc13` docs that cite `FUN_00B40A20` / `FUN_00B47E90` as mis-splits are now historical.
