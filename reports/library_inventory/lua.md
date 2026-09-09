# Embedded Lua 5.1.1 inventory

Scope: the statically linked Lua interpreter in `battlestationspacific.exe`. The game's `luaMW_*` binding layer, the script-state constructor at `0x00b6a020`, the `DoFile` replacement and the MPAK script loader are game code and are excluded. Companion data: `lua.json` (address map, tables, counts). Ghidra was used read-only.

## Verdict

Stock **Lua 5.1.1** core (`lua_ident` at `0x00d57fe8`; `findfile` calls `luaO_pushfstring` directly, which is the 5.1.1-only source quirk fixed in 5.1.2), compiled *inside the game project* with MSVC 2005 whole-program optimisation, with three deliberate source edits and a replaced `linit.c`:

| Item | Finding |
|---|---|
| `lua_Number` | `double` (`lua_pushnumber 0x00a679d0` is `FLD/FSTP qword`; TValue is 16 bytes with `tt` at +8) |
| `LUAI_MAXCCALLS` / `LUAI_MAXCALLS` | 200 / 20000 (stock) — `luaD_call 0x00a69580`, `growCI 0x00a69370` |
| `LUAI_GCPAUSE` / `LUAI_GCMUL` | 200 / 200 (stock) — `lua_newstate 0x00a68950`, `sizeof(LG)=0x178`, `sizeof(lua_State)=0x78` |
| Pseudo-indices | REGISTRY −10000, ENVIRON −10001, GLOBALS −10002 (stock) |
| Allocator | **Modified.** `l_alloc 0x00a6a1d0`: `nsize==0 → NULL`, else `GameAlloc(nsize)` (`0x00bd1780`), `memcpy(min(osize,nsize))`, then `GameFree(ptr, osize)` (`0x00bd17a0`) when `osize != 0`. `luaL_newstate 0x00a6a260` still passes it to `lua_newstate`. |
| `base_funcs` | **Modified.** 23 entries at `0x00d57c78`; `dofile` removed (the game registers its own `DoFile`). |
| `mathlib` | **Modified.** 26 entries at `0x00d57240`; `random` and `randomseed` removed. |
| Other library tables | dblib 14, strlib 15, syslib 11, iolib 11 (incl. `popen`), flib 9, tab_funcs 9, co_funcs 6, pk_funcs 2, ll_funcs 2, loaders 4 — all stock. |
| `linit.c` | Not linked. The game's script-state constructor `0x00b6a020` calls `luaL_newstate`, `lua_atpanic`, then walks the *stock* `lualibs` table (`0x00d62bb8`) opening each library selected by a bitmask, registers `DoFile`, runs region defines and `Scripts\fundamentals.lua`. A second game-side table `0x00cf8350` (used by `0x006b8740`) lists the libraries without `package`. |
| Code generation | LTCG: internal calls pass `lua_State*` in ECX and the next argument in EDX; only varargs functions (`luaL_error`, `lua_pushfstring`) stay cdecl. `errorlimit`, `enterlevel`, `tofile`, `readable`, `loaderror`, `push_onecapture` are inlined into callers; unreferenced functions (`lua_setallocf`, `luaB_dofile`, `math_random*`) are gone; `loadlib.c` was placed apart from the other objects. |

## Address ranges

| Block | Range | First / last function | Ghidra-defined | Table-referenced entry points Ghidra missed | Real functions |
|---|---|---|---|---|---|
| Main (all of Lua except loadlib) | `0x00a60870`–`0x00a7772f` | `db_getregistry` / `luaK_patchlist 0x00a77670` | 513 | 79 | **592** |
| `loadlib.c` | `0x00c2f3e0`–`0x00c3018f` | `setprogdir` / `luaopen_package 0x00c2ff90` | 14 | 8 | **22** |

Neighbours confirmed as non-Lua: below the main block are import jump thunks (`0x00a607e8`–`0x00a60865`) and game code (`0x00a60624`, wide-string helper); above it is FMOD/sound game code from `0x00a77730`. The loadlib block sits between CRT/`RtlUnwind` thunks (`0x00c2f166`–`0x00c2f25c`) and shader-texture-source game code (`0x00c30190`).

Object order inside the main block (not alphabetical, so this was compiled from the game's own file list): ldblib → lmathlib → lstrlib → loslib → liolib → ltablib → lbaselib → lapi → lstate → ldo → lauxlib → ldebug → lobject → lgc → lvm → ltm → ltable → lstring → lmem → lfunc/lzio → llex → lparser → lundump/ldump → lcode.

## Ghidra gap worth fixing first

Ghidra created no function for C entry points referenced only from `luaL_Reg` tables or other data pointers: 87 such functions were recovered here by reading the tables and decoding the bytes (all `db_get*/set*` at `0x00a60870`–`0x00a60910`, 19 of 26 math functions, `io_*`/`f_*`, `luaB_*`, `luaopen_*`, `gctm`, `ll_require`, `luaopen_package`, …). Several Ghidra function starts near them are also off (e.g. Ghidra has `0x00a62ee0`/`0x00a63010` where the tables say `str_find 0x00a62ec0`, `str_match 0x00a62ed0`, `gmatch 0x00a62fb0`, `gfind_nodef 0x00a63000`; `luaB_corunning` is `0x00a670c0`, Ghidra has `0x00a670d0`). The table addresses are authoritative. The same undercount will apply to every other C table in the binary, so the project-wide function count is a floor, not a ceiling.

Authoritative data tables: `lualibs 0x00d62bb8`, `dblib 0x00d56f80`, `mathlib 0x00d57240`, `strlib 0x00d57398`, `syslib 0x00d576f0`, `iolib 0x00d578a8`, `flib 0x00d57908`, `tab_funcs 0x00d57a80`, `base_funcs 0x00d57c78`, `co_funcs 0x00d57d38`, `pk_funcs 0x00d79840`, `ll_funcs 0x00d79858`, `loaders 0x00d79870`.

## Address → source map

`lua.json` carries 324 entries (282 high, 40 medium, 2 low confidence) with per-entry evidence. Highlights:

- **lapi.c** `0x00a67340`–`0x00a68480`, in source order: `lua_xmove 0x00a67340`, `lua_atpanic 0x00a67390`, `lua_gettop 0x00a673d0`, `lua_settop 0x00a673e0`, `lua_remove 0x00a67430`, `lua_replace 0x00a674c0`, `lua_pushvalue 0x00a67570`, `lua_type 0x00a675a0`, `lua_isnumber 0x00a67600`, `lua_isstring 0x00a67630`, `lua_tointeger 0x00a677a0`, `lua_toboolean 0x00a677e0`, `lua_tolstring 0x00a67810`, `lua_touserdata 0x00a67910`, `lua_tothread 0x00a67930`, `lua_pushnil 0x00a679c0`, `lua_pushnumber 0x00a679d0`, `lua_pushinteger 0x00a679f0`, `lua_pushlstring 0x00a67a10`, `lua_pushstring 0x00a67a50`, `lua_pushfstring 0x00a67af0`, `lua_pushcclosure 0x00a67b20`, `lua_pushboolean 0x00a67bc0`, `lua_pushlightuserdata 0x00a67be0`, `lua_gettable 0x00a67c20`, `lua_getfield 0x00a67c40`, `lua_rawget 0x00a67ca0`, `lua_rawgeti 0x00a67cd0`, `lua_createtable 0x00a67d10`, `lua_getmetatable 0x00a67d50`, `lua_getfenv 0x00a67da0`, `lua_settable 0x00a67e10`, `lua_setfield 0x00a67e40`, `lua_rawseti 0x00a67f00`, `lua_setmetatable 0x00a67f70`, `lua_setfenv 0x00a68010`, `lua_call 0x00a68090`, `lua_pcall 0x00a680e0`, `lua_load 0x00a681f0`, `lua_concat 0x00a683d0`, `lua_newuserdata 0x00a68480`.
- **lstate.c / ldo.c**: `f_luaopen 0x00a68720`, `luaE_newthread 0x00a68870`, `lua_newstate 0x00a68950`, `lua_close 0x00a68a90`, `seterrorobj 0x00a68af0`, `luaD_rawrunprotected 0x00a68b80`, `luaD_throw 0x00a69330`, `growCI 0x00a69370`, `luaD_precall 0x00a693b0`, `luaD_call 0x00a69580`, `lua_resume 0x00a69670`.
- **lauxlib.c**: `luaL_error 0x00a698b0`, `luaL_newmetatable 0x00a698f0`, `luaL_checkstack 0x00a69950`, `luaL_findtable 0x00a69a60`, `luaL_loadbuffer 0x00a6a160`, `luaL_loadstring 0x00a6a190`, `l_alloc 0x00a6a1d0`, `luaL_newstate 0x00a6a260`, `luaL_argerror 0x00a6a290`, `luaL_checktype 0x00a6a450`, `luaL_checkany 0x00a6a4b0`, `luaL_checklstring 0x00a6a4e0`, `luaL_optlstring 0x00a6a540`, `luaL_checkinteger 0x00a6a650`, `luaL_gsub 0x00a6a820`, `luaL_checkoption 0x00a6a950`, `luaL_register 0x00a6aa00`.
- **ldebug.c / lobject.c / lvm.c / lgc.c / ltm.c / ltable.c / lstring.c / lmem.c**: `lua_getstack 0x00a6ab10`, `lua_getinfo 0x00a6b840`, `luaG_runerror 0x00a6b6d0`, `luaG_typeerror 0x00a6b900`, `luaG_ordererror 0x00a6b9f0`, `luaO_int2fb 0x00a6ba50`, `luaO_log2 0x00a6baa0`, `luaO_pushfstring 0x00a6bf60`, `luaC_link 0x00a6d020`, `luaC_step 0x00a6cee0`, `luaT_init 0x00a6d090`, `luaV_tonumber 0x00a6d190`, `luaV_execute 0x00a6ddb0`, `luaH_next 0x00a6eea0`, `luaH_new 0x00a6f380`, `luaS_resize 0x00a6fbb0`, `luaS_newlstr 0x00a6fd10`, `luaM_realloc_ 0x00a70a10`.
- **llex.c / lparser.c / lcode.c / lundump.c**: `luaX_init 0x00a70ba0`, `inclinenumber 0x00a70d60`, `read_numeral 0x00a70ef0`, `read_string 0x00a715f0`, `subexpr 0x00a74ec0`, `addk 0x00a72a40`, `luaU_undump 0x00a759e0`, `fixjump 0x00a75b80`, `luaK_concat 0x00a75e60`, `luaK_code 0x00a76350`, `luaK_jump 0x00a76570`, `luaK_dischargevars 0x00a76650`, `luaK_exp2anyreg 0x00a76a60`, `luaK_goiftrue 0x00a76e90`, `luaK_goiffalse 0x00a76fc0`, `codenot 0x00a770f0`, `luaK_prefix 0x00a77330`, `luaK_patchlist 0x00a77670`.
- **Standard libraries**: every `luaL_Reg` entry of ldblib, lmathlib, lstrlib, loslib, liolib, ltablib, lbaselib, coroutine and loadlib is mapped (see `lua.json`), plus `luaopen_debug 0x00a61620`, `luaopen_math 0x00a61c50`, `luaopen_string 0x00a63910`, `luaopen_os 0x00a64190`, `luaopen_io 0x00a652b0`, `luaopen_table 0x00a66010`, `luaopen_base 0x00a67210`, `luaopen_package 0x00c2ff90`.
- **loadlib.c** (22 functions): `setprogdir 0x00c2f3e0`, `pusherror 0x00c2f470`, `ll_register 0x00c2f520`, `gctm 0x00c2f5d0`, `ll_loadfunc 0x00c2f600`, `ll_loadlib 0x00c2f670`, `pushnexttemplate 0x00c2f710`, `findfile 0x00c2f780`, `loaderror 0x00c2f870`, `loader_Lua 0x00c2f8b0`, `mkfuncname 0x00c2f920`, `loader_C 0x00c2f970`, `loader_Croot 0x00c2f9e0`, `loader_preload 0x00c2fa70`, `ll_require 0x00c2fae0`, `setfenv 0x00c2fcc0`, `dooptions 0x00c2fd10`, `modinit 0x00c2fd50`, `ll_module 0x00c2fdc0`, `ll_seeall 0x00c2feb0`, `setpath 0x00c2ff20`, `luaopen_package 0x00c2ff90`.

Medium/low entries are order-based guesses between verified neighbours (e.g. `lua_insert`, `lua_typename`, `lua_tonumber`, `lua_rawset`, `lua_gc`, `luaO_str2d`, `llex`, the inlined `errorlimit` copies in lparser) and should be confirmed before annotating.

## Recommended import/match approach

1. Do not reconstruct any of the ~614 functions by hand. The build already fetches stock lua-5.1.1 (`build/win32/_deps/lua511-src`). Apply three source edits to match the shipped configuration: `lauxlib.c` `l_alloc` → game allocator wrapper (alloc / copy / free with size); `lbaselib.c` `base_funcs` without `dofile`; `lmathlib.c` without `random`/`randomseed`. Drop `linit.c`; the masked opener at `0x00b6a020` is game code.
2. Use the `luaL_Reg` tables and this map to create the missing Ghidra functions and to name the block; keep the stock names (they are recovered symbols, not hypotheses).
3. Do not expect byte identity with a stock static library: the shipped code is an LTCG build inside the game (register conventions, inlining, dead-function removal, object placement). Match at source level (identity + behaviour) and document the internal ABI (`lua_State*` in ECX, second argument in EDX) where game code calls Lua directly.
4. Anything else Lua-shaped in the binary (`luaMW_*`, `DoFile`, `DoFileOutsideMPAK`, `Scripts\fundamentals.lua` loading, the region defines) belongs to the game inventory, not to this library.
