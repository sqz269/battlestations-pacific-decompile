# CC13 undefined-function sweep (packet cc13_define1)

Worker cc13-define1, 2026-10-12. Input: the `not_code_sites` list of
`reports/cc13_free_flow_repair.json` (docs/CC13_FREE_FLOW_REPAIR.md): 1472 rel32 CALLs to the
`_free` / `operator_new` family (00BF65AC, 00BF6989, 00BF55BE, 00BF9DC8, 00BF681B) that lay in
bytes Ghidra had never made into instructions or functions. Output: per-candidate record in
`reports/cc13_undefined_function_sweep.json`. No C++ was written; nothing here is reconstructed,
build-tested or game-validated. The result is Ghidra listing state only (exported after the
forced snapshot).

## Result

| | count |
|---|---|
| not-code call sites | 1472 |
| sites now inside a Ghidra function | 1419 |
| candidate bodies | 1137 |
| defined by this packet | 1092 |
| defined but body still broken | 1 (00764940) |
| not defined: inside another owner's lease | 42 (47 sites) |
| not defined: not a new function | 2 (0046CCE4 switch cases, 007846EB truncated body in a leased range) |

"Non-Returning Functions - Discovered" was checked disabled (`list_analyzers`) before any write,
and was not touched.

## Method

1. Live containment for every site (`get_function_by_address`): none were inside a function.
2. For each site, the gap between the previous Ghidra body end and the next function entry
   (snapshot entry list, plus functions defined earlier in this packet) was searched for starts:
   - `aligned_after_terminator`: 16-byte aligned, the byte before is INT3/NOP padding, or the
     bytes before end a RET, RET imm16 or JMP rel32 (or the address is the previous body end + 1);
   - `aligned_after_ret` (second pass): an unpadded aligned start after a RET is accepted only
     when the flow from the nearest earlier start in the gap does not reach it;
   - `verified_ref`: a rel32 CALL from inside a defined function, a `push imm32` from inside a
     defined function, an unwind-map-shaped `.rdata` entry, or an unpadded start directly after
     the previous body's RET with rel32 hits.
   Bare byte-scan hits (rel32 or dword) were not accepted alone: in the first pass they produced
   mid-instruction starts such as 004ABB5E.
3. From the nearest accepted start, a Capstone recursive descent bounded by the next entry had to
   reach the site with no conditional branch leaving the gap. The end is the end of the last
   reached instruction; 1046 of the 1070 first-pass bodies are followed by INT3 padding, the rest end
   on a RET exactly at the next function.
4. Containment checked two ways before writing: live lookups at the start, middle and last byte
   of every body (all clear), and the padding/terminator test above. No body overlaps another.
5. Defined with `tools/ghidra_define_function.py` (disk/Ghidra byte comparison, explicit
   disassembly of the range, `create_function` without flow discovery, save) in batches of 80
   under the write lock, released between batches. All bodies are below 4000h.
6. Every first-pass definition was re-read: 1069 of 1070 have a Ghidra body ending exactly where
   planned. The exception is 00764940 (below).

## What the bodies are

- 451 are byte-identical `mov eax,[esp+4]; push eax; call _free(00BF65AC); pop ecx; ret 8`:
  `std::allocator<T>::deallocate` template copies. No rel32 or pointer reference reaches them;
  they are unreferenced COMDAT copies the linker kept. Left `FUN_`.
- Most others are small STL/container helpers with a register `this` (ESI/ECX), scalar deleting
  destructors (`test byte [esp+4],1`), mostly unreferenced. 70 have `.rdata` dword hits and 24 a `push imm32` hit, but names were not
  assigned: no string, Lua or vtable evidence was cheap and certain for any of them.
- CRT (00BF..00C5): four `_output`-family bodies and their `write_string` helpers (00C09133,
  00C0917D, 00C09CAD, 00C09CF7, 00C09D07, 00C103F1, 00C10440, 00C14D91, 00C14DE0); two SEH4
  functions whose scope tables name their `__finally` blocks (00C05738, 00C058C4); two entries of
  the null-delimited initializer tables at 00CE36EC / 00CE370C (00BFE3FE, 00BFE4AF); 00BF5DB1
  (`__EH_prolog3` after `_Locinfo_ctor`); 00C0D5AC (walks `__pioinfo` 0109EDA0 freeing blocks,
  `__ioterm` shape, name not applied). These are not library-tagged yet: the tag ledger's
  categories need a stock-name match this packet did not make.
- The `_output` copies have inline jump tables right after the code (00C09B0D, 00C0AC6F,
  00C11380, 00C156DC, 20h each). The ranges passed to the define tool end before each table,
  because the bridge's `disassemble_bytes` sweeps the whole range linearly (seen at 00764940) and
  would turn the table into instructions.

## Pass 2: referenced bodies image-wide (cc13-define1, resumed by cc13-define2)

Scope approved by the lead: an image-wide gap census with the pass-1 start rules found 42,475
candidate bodies (40,062 candidates, 2,413 rejected). Only the 8,837 with a confirmed reference
were defined; the rest are held. Record: `reports/cc13_undefined_function_sweep_2.json`
(`defined`, `not_defined`, `held`, `rejected`, compact column arrays).

| class | selected | defined | reference that qualified it |
|---|---|---|---|
| EH handler stubs | 6,780 | 6,780 | `mov eax,FuncInfo; jmp ___CxxFrameHandler3` (FuncInfo magic 19930522), `PUSH stub` in the parent's SEH prologue confirmed in the listing |
| `.rdata` table entries | 1,860 | 1,859 | dword in an `.rdata` function-pointer table beside a known entry, plus a structural start |
| unwind-map entries | 121 | 121 | dword in an unwind-map-shaped `.rdata` record |
| called / pushed | 76 | 76 | rel32 CALL or `push imm32` from code |
| total | 8,837 | 8,836 | |

- A power outage stopped the first run after 2,469 bodies. The resume re-read every candidate
  live (`get_function_by_address`: entry and body end against the planned end) and defined only
  the missing ones, so nothing was defined twice. Same tool and rules as pass 1
  (`tools/ghidra_define_function.py`, explicit `disassemble_bytes` then `create_function` without
  flow discovery), 100 bodies per write-lock hold with a per-batch lease, released between.
  The 15 bodies first skipped in agent/cc13-net2's ranges 0076B630..00786C56 were defined after
  that lease was released (`lease check`: unleased).
- "Non-Returning Functions - Discovered" was checked disabled (`list_analyzers`) before the resume.
- Names (lead-approved convention, applied in one locked pass, 500 per lock hold, prior names in
  the worktree's `local/df3_rename_log.jsonl`): the 6,780 handler stubs are `EHHandler@<addr>`,
  the 121 unwind-map entries `Unwind@<addr>` (Ghidra's own funclet convention). Table and
  call/push entries stay `FUN_` unless the define tool applied a reviewed ledger name.
- Counters: bsp classifies EH funclets by name prefix only. `EHHandler@` was added next to
  `Unwind@` / `Catch_All@` in tools/bsp.py (brief), tools/status.py, tools/progress_board.py and
  tools/ledger_backfill.py (the only places in tools/ that use those prefixes). The 8 earlier
  hand-named handler stubs keep their reviewed ledger names and are not matched by the prefix:
  00CB7571, 00CB8A93, 00CBD458 (`BSP_Renderer_DrawDebugLineRecords_EHHandler`), 00CBEF88,
  00CC59A1, 00CC5AF8, 00CC5B18, 00CC5B38 (`EHHandler_<ctx>_<addr>`).
- Counts (`bsp brief`):

  | state | snapshot total | untagged `FUN_` | EH funclets |
  |---|---|---|---|
  | before pass 2 (2026-10-12T03:59Z) | 66,483 | 16,370 | 27,955 |
  | defined, unnamed (05:29Z) | 75,410 | 24,827 | 27,955 |
  | renamed, counters updated (06:01Z) | 75,434 | 17,629 | 34,856 |

  The snapshot totals include other workers' concurrent definitions and renames.
- One defined body is short: **00726930** (`.rdata` table 00CFDBE4) ends at 0072693F; the loop at
  00726940..00726957 is not instructions in Ghidra even after `--recreate` reported 40 bytes
  disassembled (`get_instruction` at 00726940: none). Needs a CodeBrowser look (Clear +
  Disassemble at 00726940, then `ghidra_define_function.py 00726930 00726958 --recreate`).
- Held (31,225): no confirmed reference (unreferenced COMDAT copies, gap-start heuristics, 30
  table hits dropped for lacking structural start evidence). Not defined on purpose; the list is
  in the report for a later pass with stronger evidence.

## Open items

- **00764940** (scalar deleting destructor, vtable slot 00D0361C): an offcut instruction run at
  00764941/42/44/47 blocked the entry instruction, so the function has a 1-byte body. The bridge
  has no clear-code endpoint. Applying `undefined1` over those four bytes removed the offcut
  instructions but left defined data that still blocks disassembly (the old offcut state was not
  better). Fix in the CodeBrowser: Clear Code Bytes 00764941-00764947, disassemble 00764940, then
  `python tools/ghidra_define_function.py 00764940 0076495f --recreate`.
- **Switch cases outside bodies**: no computed-jump references exist, so cases reached only
  through the table are outside the Ghidra body: sites 00C0ABBC (in 00C09D07) and 00C1129A (in
  00C10440), and BSP_SceneEntity_ShouldGenerate 0046C550, whose table at 0046CCE4 is followed by
  case code up to 0046CE90 (site 0046CE48). These need Decompiler Switch Analysis or explicit
  references, not new functions.
- **Leased ranges** (not touched; the holder or a later pass can run the define tool with the
  ranges in the report): agent/cc13-net1 0076AA00-00787040 has 24 bodies plus the truncated body
  at 007846EB (fall-through after the `_free` at 007846E6); agent/cc13-files1 has 16 bodies in
  00BE44C0-00BF5600; agent/cc13-warnings1 has 00978FA0 and 00979570.
- **Neighbouring undefined code**: the gaps hold more undefined functions without `_free` calls
  (e.g. 00931F50..00932050, 00404240, the region after the `_output` tables, the initializer-table entry
  00C0AC8F). This packet only defined bodies that contain a not-code site. An
  image-wide pass using the same start rules would find them.

## Pass 3: RNG call sites outside functions, and four suspect seg-0 bodies (cc13-define3, 2026-10-12)

Input: the 20 `reports/cc13_rng_stream_census.json` rows that `tools/verify_report_calls.py` failed
as "in no Ghidra function" (docs/CC13_RNG_STREAM_CENSUS.md 1.3), and the four segment-0 bodies that
docs/CC13_DYN_LIBRARY.md flagged as possibly running past their INT3 padding. Record of every write:
`reports/cc13_define3.json`. Ghidra listing state only; nothing reconstructed or tested.

- **Method.** Capstone recursive descent from each start over the disk image, bounded by the next
  indexed entry (conditional branches, rel jumps and `jmp [reg*4+table]` followed; every descent
  closed without tail jumps, tables or out-of-gap branches). Containment checked two ways: every start
  is 16-aligned with INT3 before it, every body ends on RET followed by INT3 padding (or, for
  00AF7FB0, a RET directly before the aligned getter at 00AF7FF0); live `get_function_by_address` at
  start, middle and last byte returned no function. "Non-Returning Functions - Discovered" was
  checked disabled and `_free` (00BF9DC8) not noreturn before the write.
- **Defined (20 bodies, end exclusive):** 0047B7F0-0047B808, 0048AEC0-0048AEF7, 006E2410-006E2431,
  006E24E0-006E2504, 006E7C60-006E7C87, 006F23E0-006F2412, 006FCA50-006FCA71, 007B3D60-007B3D9B,
  007B4890-007B48C3, 007EBF10-007EC177, 009B94B0-009B94DF, 009BA2F0-009BA6E6, 009BD6E0-009BD98C,
  009CA730-009CA763, 009D5D60-009D5DCA, 009DA950-009DA9C2, 00AF5A30-00AF5A73, 00AF7F70-00AF7FA6,
  00AF7FB0-00AF7FF0, 00BC7C10-00BC7DA3. One locked `ghidra_define_function.py` batch; re-read live,
  all 20 bodies match the plan exactly. Left `FUN_` (no name evidence was gathered). References:
  006E24E0, 006E7C60 (also CALL from 006E7DC6), 007EBF10, 009BA2F0, 009BD6E0 are in `.rdata`
  tables; the other 15 have no rel32 or dword reference (`scan-bytes`, positive control
  `10 bf 7e 00` = 1 hit). They were defined on the lead's instruction because they draw from the
  RNG; they are structurally functions but may be unreached COMDAT copies.
- **The two "past a short body" sites are separate functions, not mis-sized ones.** 006FCA40 is a
  correct 8-byte adjustor thunk (`sub ecx,10h; jmp 006FCAD0`); site 006FCA6B is in its own body
  006FCA50 after INT3 padding. 00AF7F40 `BSP_NativeParticleModel_ScalarDelete` correctly ends at
  00AF7F5F; 00AF7F60 is a separate 15-byte `fld; fmul [00D7A358]; fstp [ecx]; ret 4` body (no
  reference, not defined), and site 00AF7F92 is in 00AF7F70 (`x == 0 ? 0 : UnitFloat() * x`). The
  census rows were corrected in place (`function`, `name`, `reach`; 00AF7F92 also `confidence`
  `name` -> `band`, `reason`, and `host`: src/native_particle_model_lifetime.cpp cites only 00AF7F40).
- **Seg-0 bodies are correctly sized.** 004019A0 (ends 00401B2D), 00404DC0 (004051BD), 00405D80
  (00405F54), 0040C790 (0040CB8D): for each, the descent end equals Ghidra's body end, the body has
  no internal gap, and only INT3 lies between it and the next entry. No `--recreate` was needed.
  The dyn1 "inside existing bodies" result is probably a stale-index effect (those four starts were
  already function entries when its live check ran); not confirmed from dyn1's planner output.
- **Verify** (`tools/verify_report_calls.py reports/cc13_rng_stream_census.json`): before 759 rows,
  20 failed; after defining, 2 failed (the two rows naming the wrong owner); after the row fix, 0.
- **Snapshot** `--force` at 2026-10-12T08:10Z: function count 76586 -> 76606 (includes other
  workers); index rebuilt, all 20 entries present.
- **Neighbours not defined (no RNG, outside this packet):** more undefined bodies follow several of
  these in the same gaps (after-padding starts 0047B810, 006E2440, 006E2510, 006E7C90, 006F2420,
  007B48D0, 009BA6F0, 009BD990, 009CA770, 009D5DD0, 00AF5A80, plus 00AF7F60 and 00AF7FF0).

## Handoff

Done: the analysis scripts are `local/df1_*.py` in the worktree (not committed; `local/` is
ignored). `df1_analyze.py` writes the candidate list, `df1_define.py` leases and defines, and
`df1_report.py` rebuilds the report from live state. To re-run for the leased ranges once they
are released: `python local/df1_define.py <start>-<end> ...` with the pairs from the report rows
whose `defined` is `no`.
