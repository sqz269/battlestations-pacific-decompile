# Embedded Lua 5.1.1 library: complete identification (cc13-lualib1)

Packet: partition segment 73 (`00A62660-00A7A3F0`) plus the Lua part of segment 72
(`00A60870-00A62660`), worker cc13-lualib1, 2026-10-12. Goal: give every function of the
statically linked Lua interpreter its original Lua source name so that it leaves the
reconstruction target. No C++ was written: library code is not reconstructed, and the host
build already links the stock lua-5.1.1 source fetched into `build/win32/_deps/lua511-src`.

Status vocabulary: everything here is **exported and annotated** (names in the ledger and in
Ghidra). Nothing is reconstructed, build-tested or game-validated by this packet.

## Version

Lua **5.1.1**, as `reports/library_inventory/lua.md` already established:

- `lua_ident` at `00D57FE8`: `$Lua: Lua 5.1.1 Copyright (C) 1994-2006 Lua.org, PUC-Rio $`.
- `base_open 00A67100` stores `_VERSION = "Lua 5.1"`.
- `luaU_header 00A759A0` writes the 5.1 bytecode header (ESC "Lua", version byte 0x51, format 0).
- The 5.1.1-specific llex.c helpers `buffreplace 00A70ED0` and `trydecpoint 00A70EF0`
  (locale decimal point retry) are present.

This installation's modified scripts do not touch the interpreter and play no part here.

## Method

1. Preprocessed every lua-5.1.1 `.c` file (except `lua.c`, `luac.c`, `print.c`) with the
   local MSVC (`cl /E`), parsed each function's ordered calls, function-pointer references and
   string literals (`local/ll1_src.py`, worktree-local, not committed).
2. Disassembled the PE block `00A60870-00A7772F` with Capstone from the disk image, collecting
   per function the ordered direct calls, pointer references and string immediates
   (`local/ll1_bin.py`). Function starts came from the index plus starts the index lacked:
   rel32 call targets, aligned code after INT3 padding, and code after jump tables.
3. Seeded with the existing `lua511` tag names, aligned each named function's binary call
   sequence with its source call sequence (Needleman-Wunsch), and ran a constraint pass
   (callee set, caller set and strings of a binary function must fit the source function,
   allowing LTCG inlining). Both are only candidate generators.
4. Reviewed every candidate and every pre-existing name file by file, in address order,
   against the call shape, strings, struct offsets and the source order of each file
   (LTCG keeps the per-file order with a few moves). Assembly was read wherever the call
   shape alone did not decide (e.g. `luaD_growstack`, `luaT_gettmbyobj`, `luaG_checkopenop`,
   `checkArgMode`, `unmakemask`, `addintlen`, `lua_getallocf`/`lua_setallocf`, `getbinopr`,
   the `luaK_exp2RK` body, `propagateall`).

Code-generation notes that matter for reading these bodies (already in the inventory, confirmed):
LTCG passes `lua_State*` in ECX and the next argument in EDX; `luaL_error`, `lua_pushfstring`
and `luaO_pushfstring` stay cdecl. Small statics are inlined into some callers and still kept
out of line (see "Out-of-line copies").

## Result

- The Lua block holds **665 functions**: the 592 Ghidra already had plus the 73 this packet
  defined. Every one now carries its Lua 5.1.1 source name (or, for four CRT wrappers, its CRT
  name); none is left raw.
- **411 names** written to `config/names/00a60000.jsonl` and `00a70000.jsonl`: 244 from manual
  review, 133 aligned/constraint matches that were reviewed, 31 inventory `__prov` names
  re-checked and promoted to plain names, and 3 spelling fixes to the source names
  (`classEnd` -> `classend`, `seterrorobj` -> `luaD_seterrorobj`, `luaM_realloc` -> `luaM_realloc_`).
- **73 functions Ghidra never defined** (69 Lua, 4 CRT inline wrappers), defined with
  `tools/ghidra_define_function.py` (record `reports/defined_functions.json`):
  00A60990 getthread; 00A60E10 auxupvalue; 00A61D10 posrelat; 00A621E0 check_capture;
  00A62210 capture_to_close; 00A635C0 addintlen; 00A63970 gmtime; 00A63980 localtime;
  00A63990 mktime; 00A639A0 time; 00A63B90 getboolfield; 00A63BE0 getfield; 00A64310 tofile;
  00A64340 newfile; 00A64800 getiofile; 00A65200 createmeta; 00A65A90 sort_comp;
  00A666E0 luaB_pairs; 00A66720 ipairsaux; 00A66770 luaB_ipairs; 00A66CB0 luaB_newproxy;
  00A66ED0 luaB_auxwrap; 00A672B0 getcurrenv; 00A67660 lua_isuserdata; 00A680D0 f_call;
  00A68440 lua_getallocf; 00A684D0 aux_upvalue; 00A686F0 freestack; 00A687A0 preinit_state;
  00A68A80 callallgcTM; 00A69270 restore_stack_limit; 00A69A40 libsize; 00A69B50 emptybuffer;
  00A69CA0 luaL_addstring; 00A69DA0 luaL_ref; 00A6A230 panic; 00A6A380 tag_error;
  00A6AA20 currentpc; 00A6AB90 getluaproto; 00A6AD30 info_tailcall; 00A6AE10 precheck;
  00A6B5A0 isinstack; 00A6C0E0 removeentry; 00A6C5C0 checkstacksizes; 00A6C810 propagateall;
  00A6F430 getfreepos; 00A70020 unlinkupval; 00A70450 DumpBlock; 00A70C40 txtToken;
  00A70ED0 buffreplace; 00A72220 anchor_token; 00A72250 error_expected; 00A722F0 check;
  00A72320 checknext; 00A723C0 str_checkname; 00A72420 codestring; 00A727A0 searchvar;
  00A727D0 markupval; 00A729B0 enterlevel; 00A729D0 enterblock; 00A729F0 leaveblock;
  00A72F00 closelistfield; 00A73A20 getunopr; 00A73D00 expr; 00A73D10 block_follow;
  00A75010 error; 00A75040 LoadBlock; 00A75B60 isnumeral; 00A75BE0 getjump;
  00A75C00 getjumpcontrol; 00A75E40 dischargejpc; 00A75F20 freereg; 00A76E20 jumponcond.
  `luaB_newproxy` (loaded by `base_open` at `00A671E9`) and `propagateall` sat behind the jump
  tables of `luaB_tostring` and `propagatemark`, so no padding scan saw them. `block_follow` is
  defined over its code only (`00A73D10-00A73D31`); its tables follow.
- Tags: the same 411 addresses carry rows in `config/tags/00a60000.jsonl`/`00a70000.jsonl`
  (category `lua511`, the four time wrappers plus `difftime` as `crt_unmatched`), all pointing
  to the new evidence id `cc13ll1`. `reports/library_inventory/lua.json` gained the same
  entries (680 mapped addresses), so a `build_tag_ledger.py` regeneration keeps them.
  Only the two bands this packet owns were touched; `write_tags` was not used.

### Corrections to the earlier inventory

The existing tags had these swaps or misreadings (all fixed; old names are kept in each
record's evidence):

| Address | Old tag | Now | Deciding evidence |
|---|---|---|---|
| 00A626B0 | check_capture__prov | end_capture | 'invalid pattern capture' scan (capture_to_close inlined) then `match` |
| 00A62710 | capture_to_close | match_capture | 'invalid capture index' then the length and byte compare |
| 00A64840 | aux_lines | g_iofile | `io_input` passes "r" to it; newfile/fopen/fileerror inline |
| 00A66BB0 | luaB_tostring | (kept) | its Ghidra extent swallows `luaB_newproxy 00A66CB0` |
| 00A68150 | lua_cpcall__prov | f_Ccall | builds the C closure; `lua_cpcall` is 00A681C0 |
| 00A68460 | lua_getallocf__prov | lua_setallocf | stores ud and frealloc; `lua_getallocf` is 00A68440 |
| 00A68C70 | (aligned luaD_growstack) | luaD_reallocstack | realloc + `correctstack`; `luaD_growstack` is the 32-byte 00A68D50 |
| 00A6C1F0 | luaC_freeall__prov | luaC_separateudata | called by `lua_close`/`atomic`; `luaC_freeall` is 00A6CBC0 |
| 00A70EF0 | read_numeral | trydecpoint | localeconv + 'malformed number'; `read_numeral` is 00A70FD0 |
| 00A72280 | new_localvar__prov | errorlimit | the two "has more than %d %s" formats only |
| 00A72550 | indexupvalue__prov | new_localvar | 'local variables' limit then `registerlocalvar` |
| 00A72650 | errorlimit_inlined__prov | indexupvalue | 'upvalues' limit, upvalue vector growth |
| 00A72A40 | addk | pushclosure | OP_CLOSURE via `luaK_codeABx`; `addk` is 00A75F60 |
| 00A72FA0 | luaY_checklimit__prov | listfield | `expr` then 'items in a constructor' |
| 00A73C00 | chunk__prov | subexpr | `luaK_prefix`/`luaK_infix`/`luaK_posfix` recursion |
| 00A74EC0 | subexpr | chunk | statement loop; `luaY_parser` calls it |
| 00A759E0 | luaU_undump | LoadHeader | 'bad header'; `luaU_undump` is 00A75B00 (referenced by `f_parser`) |
| 00A75080 | Load_helpers__prov | LoadChar | one-byte LoadBlock |

Also: `00A6A340`/`00A6A380` are `luaL_typerror` and `tag_error`; the earlier automatic guesses
`pushstr`, `Arith` and `freeexp` at 00A68D50/00A6D130/00A76B00 were `luaD_growstack`,
`luaT_gettmbyobj` and `luaK_exp2RK`.

## Game-specific changes to the core

Comparing every function against the stock source turned up nothing beyond what the inventory
recorded, with one correction:

- `l_alloc 00A6A1D0` (game allocator wrapper), `base_funcs` without `dofile`, `mathlib`
  without `random`/`randomseed`, and the replaced `linit.c` (game constructor `00B6A020`)
  are as documented in `reports/library_inventory/lua.md`. These, the `luaMW_*` bindings and
  the `DoFile` replacement are game code and stay reconstruction targets.
- **`lua_setallocf` is present** (`00A68460`, unreferenced); the inventory listed it as dropped.
- No other edits: every one of the 665 bodies has the stock call shape, strings and limits
  (the limits seen while reading: LUAI_MAXCCALLS 200, LUAI_MAXCALLS 20000, MAXSTACK 250).
- `os_execute 00A63A10` is the stock body; the host-side `os.execute` guard
  (`include/bsp/lua_exec_guard.hpp`, owned by cc9) is a host addition with no counterpart here.

### Out-of-line copies and dead code

The linker kept unreferenced COMDATs, so several statics that LTCG inlined into every caller
also exist out of line with no callers: `auxupvalue`, `getthread`, `enterlevel`,
`propagateall`, `lua_setallocf`, `posrelat`, `addintlen`, among others. Their names are the
source names; nothing calls them.

`loslib` also carries five unreferenced copies of the MSVC 8 `time.h` static-inline wrappers
(`difftime 00A63950` -> `__difftime64`, `gmtime 00A63970` -> `__gmtime64`,
`localtime 00A63980` -> `__localtime64`, `mktime 00A63990` -> `__mktime64`,
`time 00A639A0` -> `__time64`). They are CRT, tagged `crt_unmatched`, not Lua.

## Not library code in segment 73

- `00A77730-00A7A3F0`: sound/dialog streaming code, already named and reconstructed by
  other packets (`BSP_StreamedDialog*`, `BSP_SoundChannel_*`, ...). Untouched.
- `00A776B0`, `00A776D0`, `00A776F0`, `00A77700`, `00A77710`, `00A77720`: six small C++
  `thiscall` accessors (`ret 4` setters/getters on +10h/+20h/+4Eh/+50h) right after the Lua
  block, with no Ghidra function and no callers found by a rel32 scan (00A77700 has two
  absolute references, which suggests vtable slots). Left for the owner of that class.

## Handoff

Done (2026-10-12, 02:42-03:04 UTC): every Lua block function is named in the ledger, tag
shards and inventory, and in Ghidra. The 73 functions were defined
(`reports/defined_functions.json`), all 411 renames and plate comments applied through
`tools/ghidra_annotate.py` under the write lock, project saved; prior values are in the
worktree's `local/ghidra-annotations-*.json`. Spot checks: `luaB_newproxy` body
`00A66CB0-00A66D92`, `luaB_tostring` now ends at `00A66C91`, `block_follow` `00A73D10-00A73D30`.

One deviation: `Arith 00A6DC00` (lvm.c static) is recorded as **`lvm_Arith`**, because the
Ghidra bridge rejects the single-token name `Arith` (`name_quality`). The inventory keeps the
source name `lvm.c:Arith`, so a `build_tag_ledger.py` regeneration would propose `Arith`
again and hit the same rejection.

Open items:

1. The six accessors at `00A776B0-00A77720` need an owner (vtable search for 00A77700).
2. Exports in `exports/bsp` still carry the old names until the next `bsp.py snapshot`
   (not run here: Ghidra was busy with cc13-flowfix1's image-wide body repair).
3. The bookmarks `ghidra_tag.py` sets ("Inventory: lua511") were not applied for the 73 new
   functions; `python tools/ghidra_tag.py --category lua511 --apply` adds them if wanted.
4. The `loadlib.c` part of the inventory (`00C2xxxx`) is outside this packet and was not
   re-checked.
