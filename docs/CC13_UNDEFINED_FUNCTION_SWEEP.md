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
| `.rdata` table entries | 1,860 | 1,845 | dword in an `.rdata` function-pointer table beside a known entry, plus a structural start |
| unwind-map entries | 121 | 121 | dword in an unwind-map-shaped `.rdata` record |
| called / pushed | 76 | 75 | rel32 CALL or `push imm32` from code |
| total | 8,837 | 8,821 | |

- A power outage stopped the first run after 2,469 bodies. The resume re-read every candidate
  live (`get_function_by_address`: entry and body end against the planned end) and defined only
  the missing ones, so nothing was defined twice. Same tool and rules as pass 1
  (`tools/ghidra_define_function.py`, explicit `disassemble_bytes` then `create_function` without
  flow discovery), 100 bodies per write-lock hold with a per-batch lease, released between.
- "Non-Returning Functions - Discovered" was checked disabled (`list_analyzers`) before the resume.
- Names: none assigned except the reviewed ledger names the define tool applies on its own (18
  `BSP_`, 14 `thunk_`, 1 `TRIV_` bodies). The handler stubs and table entries are `FUN_`.
- Not defined (15): all in agent/cc13-net2's ranges 0076B630..00786C56 (listed in
  `not_defined` with the holder). One defined body is short: **00726930** (`.rdata` table
  00CFDBE4) ends at 0072693F; the loop at 00726940..00726957 is not instructions in Ghidra even
  after `--recreate` reported 40 bytes disassembled (`get_instruction` at 00726940: none). Needs a
  CodeBrowser look (Clear + Disassemble at 00726940).
- Counts after the forced snapshot (2026-10-12T05:29Z): Ghidra 66,483 -> 75,410 functions
  (includes other workers' concurrent definitions); `bsp brief` untagged `FUN_` 16,370 -> 24,827,
  EH funclets unchanged at 27,955. The handler stubs are therefore counted as untagged `FUN_`.
  bsp counts EH funclets by name prefix only (`Unwind@` / `Catch_All@`, tools/bsp.py:629,
  tools/status.py:29, tools/progress_board.py:68) and had no convention for handler stubs (8
  hand-named `EHHandler_<ctx>_<addr>`). Proposed to the lead, not applied: rename the 6,780 stubs
  `EHHandler@<addr>` and add that prefix to the three counters. `local/df3_rename.py --apply`
  in the cc13-define1 worktree does the rename (locked batches of 500, prior names logged).
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

## Handoff

Done: the analysis scripts are `local/df1_*.py` in the worktree (not committed; `local/` is
ignored). `df1_analyze.py` writes the candidate list, `df1_define.py` leases and defines, and
`df1_report.py` rebuilds the report from live state. To re-run for the leased ranges once they
are released: `python local/df1_define.py <start>-<end> ...` with the pairs from the report rows
whose `defined` is `no`.
