# Segment 87: the game's Lua script loader around stock Lua 5.1.1 (cc13-scripts1, item e for cc9)

Segment 87 (`00B65BA0..00B6D1B0`) is the game's Lua glue: the state owner (`BSP_LuaStateOwner_*`),
the LuaPlus-style `BSP_LuaObject_*` value API, the `DoFile` replacement, two `DoBuffer` helpers
and a zlib script cache. The embedded library itself is stock Lua 5.1.1 apart from the edits
listed in docs/CC13_EMBEDDED_LUA_LIBRARY.md. Most of the owner and loader was already
reconstructed by earlier packets (docs/LUA_STATE_OWNER.md, docs/LUA_SCRIPT_RUNTIME.md,
docs/NATIVE_LUA_FILE_LOADING.md, docs/NATIVE_LUA_SCRIPT_OVERRIDES.md, docs/LUA_RUNTIME_GLOBALS.md);
this packet names and documents the remaining loader functions and collects, for cc9, every
difference from stock 5.1.1 that could change **which scripts load** or **how they fail**. The
mission machine state (`game+1A08h`, segment 29, docs/MISSION_LUA_HOST.md) is cc9's; it appears
here only where segment 87 code runs on it.

Nothing here edits cc9's Lua host files. Claims about this installation cite file mtimes; the
installation is modded (BSPRM/AlterBSP artefacts).

## Differences from stock Lua 5.1.1 that affect loading and failure

| # | Area | Game behaviour | Stock 5.1.1 | Evidence |
| --- | --- | --- | --- | --- |
| 1 | Panic | `00B669C0` takes `lua_tolstring(L, lua_gettop(L))`, discards it and returns 0; `luaD_throw 00A69330` then calls `exit(1)` (00A69367/00A69369). An error outside every `pcall` ends the process with code 1 and **no message**. | `luaL_newstate`'s panic prints `PANIC: unprotected error ...` to stderr, then `exit`. | listings of 00B669C0, 00A69330 |
| 2 | Error mechanism | C build: `luaD_rawrunprotected 00A68B80` uses `_setjmp3`/`longjmp` (00C034F4/00C03570), not C++ exceptions. A Lua error longjmps over C++ frames without running destructors. | Same when built as C. | listing of 00A68B80 |
| 3 | `dofile` | Removed from `base_funcs`; the global `DoFile` (capital D) is a native C function `00B69E00` registered by owner open (00B6A303), by `00B66C00` and `00B65E20` (borrowed wrappers), and by the unreferenced `00B69F20`. | `dofile(path)` via `luaL_loadfile` (fopen). | docs/CC13_EMBEDDED_LUA_LIBRARY.md; listings |
| 4 | `DoFile` path lookup | The path goes to the VFS manager `0109CEEC` vtable `+4(path, 2)` (archives and mounted folders, alias and case normalisation), not to `fopen`, not to `package.path`. No extension is added and there is no search list. | `fopen` relative to the process CWD. | docs/LUA_SCRIPT_RUNTIME.md; docs/NATIVE_VFS_OPEN_ROUTE.md |
| 5 | Missing file | Null stream, not-open stream or size 0 returns silently: **a missing script is skipped**, the caller continues. | `cannot open <file>` error. | docs/LUA_SCRIPT_RUNTIME.md (00B66CC3/00B66CD4/00B66CE5) |
| 6 | Override variants | After the base file, every existing `stem_<suffix>.ext` from `00BDEF90` runs in the same state, in suffix-registration order (duplicates included). The base always runs first, so a variant **adds to or overwrites** what the base defined. Suffixes are registered only by content-package mounts: `0073BC40` mounts `ContentPackage/%08x.msar` for n = 1..15 and registers suffix `%08x`; `00995690` does the same for Xbox Live content (`XLiveContentGetPath`). This installation has no `ContentPackage` folder and no `*_xxxxxxxx.*` file under `scripts/` or `interface/`, so no variant runs here. | none | listings of 0073BC40, 00BDEF90 docs |
| 7 | Chunk name | `luaL_loadbuffer(buf, size, path)` with the bare path: errors read `[string "Scripts\global\x.lua"]:12: ...`. `DoBuffer` chunks are all named `DoBuffer`; fundamentals is `Scripts\fundamentals.lua`. | `@path`, so `path:12: ...`. | docs/LUA_SCRIPT_RUNTIME.md; 00B65ECE |
| 8 | Compile error | The `luaL_loadbuffer` status is ignored and `lua_call(L, 0, LUA_MULTRET)` follows, so the error **string** is called: the raised error is `attempt to call a string value`, and the syntax message itself is lost. | `dofile` raises the syntax message. | 00B66DA2/00B66DC4; 00B65ED4/00B65EE0 |
| 9 | Run error | `lua_call` is unprotected in `DoFile`, `DoBuffer`, fundamentals and every `00B69D40` caller: a run error propagates to the nearest enclosing `pcall`, or to row 1 (silent exit) when there is none. | `dofile` also propagates. | same |
| 10 | Return values | `DoFile` returns nothing (00B69F17 returns 0 results). | `dofile` returns the chunk's results. | docs/LUA_SCRIPT_RUNTIME.md |
| 11 | `DoFile` argument | `lua_tolstring` then an unchecked `strlen` copy (0041E870): numbers are accepted, an embedded NUL truncates, `DoFile(nil)` or no argument dereferences null (crash). Extra arguments are ignored. | `dofile()` with no argument reads stdin. | docs/LUA_SCRIPT_RUNTIME.md (00B69E60) |
| 12 | Obfuscation flag | `00B69D40`/`00B66CA0` take a flag: when set, bytes up to and including the first `01h` become spaces and the rest are nibble-rotated. In the exported decompiles, 25 calls to `00B69D40` pass a literal 0 (including `DoFile` 00B69E00 and the mission bootstrap) and 14 show no recovered arguments; no call shows 1. A plain-text file run with the flag set would turn into whitespace and do nothing. | none | docs/LUA_SCRIPT_RUNTIME.md; exports grep |
| 13 | Precompiled chunks | `luaL_loadbuffer` accepts `\x1bLua` binary chunks (stock `lundump`), so a precompiled script would load through `DoFile`. None of this installation's 621 `.lua` files is precompiled. | Same. | scan in docs/CC13_DIALOG_TYPE_SCAN.md |
| 14 | Library set | `BSP_LuaStateOwner_Open 00B6A020(mask)` always opens base, then table `04h`, io `08h`, os `10h`, string `20h`, math `40h`, debug `80h`, package `02h` per bit. Of the 51 rel32 call sites, the resolved masks are 1 (base only, 23 sites), 4 (base + table, 9), 41h (base + math, 5) and 65h (base, table, string, math; 2: `00AC672B` and the unreferenced cache runner). No resolved owner opens io, os, debug or package, so `require`, `io.*`, `os.*` and `debug.*` do not exist in those states. The mission machine state is opened by segment 29 (`006B8740`), not here. | `luaL_openlibs` opens all. | call-site scan (`tools/call_push_args.py 00B6A020`, 12 sites unresolved because the mask is in a register or the back-scan misaligned) |
| 15 | `math.random` | `math.random`/`math.randomseed` are absent from `mathlib`. | present | docs/CC13_EMBEDDED_LUA_LIBRARY.md |
| 16 | Disk loaders left in | `loadfile` (`luaB_loadfile 00A66850` -> `luaL_loadfile 00A69F60`, fopen) and `loadstring` remain in base. A script using `loadfile` reads the disk relative to the CWD and bypasses the VFS and the override variants. | same | function names in the image |
| 17 | Bootstrap globals | Each owner open sets `PC = true`, `X360COMP = true/false` from byte `0108FF20`, and `REGION` from `0108FF28` (EU/USA/JAP only), each through `luaL_loadstring` + `lua_pcall` with statuses ignored, then runs the cached `Scripts\fundamentals.lua` unprotected. `0108FF20` has one writer, `BSP_Settings_SetXboxCompatibilityMode 008D44C0`, which also pushes `X360COMP=` into the mission state through `006B8AD0`. In this installation `scripts/fundamentals.lua` (657 bytes, mtime 2024-07-13) defines `Platform`, `PlatformCompatibility` (`XENON or X360COMP` -> Xbox value), `PlatformPcxboxOnly`, `PlatformPs2xboxOnly`, `PlatformPcOnly`; so the Xbox-compatibility setting switches every `PlatformCompatibility(...)` value in the data tables. | none | docs/LUA_STATE_OWNER.md; docs/LUA_RUNTIME_GLOBALS.md |
| 18 | Number precision | The device is created without `D3DCREATE_FPU_PRESERVE` (00B2AFF9), so after Direct3D starts the x87 precision field is 24-bit (measured, docs/GAME_EXECUTABLE.md). Lua numbers stay `double` in memory, but every x87 add/multiply/divide in scripts run after device creation rounds to a 24-bit mantissa. A host built with SSE2 doubles computes script arithmetic at 53 bits: counters past 2^24, accumulated times and printed numbers can differ. | 53-bit | docs/X87_CONTROL_WORD.md; docs/GAME_EXECUTABLE.md |
| 19 | Traceback | No segment-87 path installs an error function: `ExecuteString` and `DoBufferProtected` call `lua_pcall(..., 0)`; the rest are unprotected. The only traceback is the mission machine's handler `008C8390` (cc9's docs/MISSION_LUA_HOST.md). | n/a | listings |
| 20 | Sandboxing | None beyond the library mask. No `setfenv`, no global-table protection, no path filter: any VFS-visible file name can be `DoFile`d. | n/a | absence in the segment |

## Functions named in this packet

All names are hypotheses (`ledger add-name`, applied in Ghidra with evidence comments through
`tools/ghidra_annotate.py`; prior values in the worktree's
`local/ghidra-annotations-20261012T062542Z.json`).

| Address | Name | Original ABI | Behaviour |
| --- | --- | --- | --- |
| 00B65E20 | `BSP_LuaStateOwner_ConstructBorrowedWithDoFile` | `__thiscall(owner, lua_State*)`, RET 4 | owns = 0, state = L, registers `DoFile` (00B65E34 `lua_pushcclosure(L, 00B69E00, 0)`, 00B65E46 `lua_setfield(L, LUA_GLOBALSINDEX, "DoFile")`). No tracked-object bookkeeping, unlike `00B66C00`. |
| 00B65EC0 | `BSP_LuaStateOwner_DoBuffer` | `__thiscall(owner, const char*, size_t)`, RET 8 | `luaL_loadbuffer(..., "DoBuffer")` status ignored, `lua_call(L, 0, LUA_MULTRET)`. |
| 00B65EF0 | `BSP_LuaStateOwner_DoBufferProtected` | `__thiscall(owner, const char*, size_t)`, RET 8, EAX 0/1 | load, then `lua_pcall(L, 0, LUA_MULTRET, 0)`; 1 on either failure with the message left on the stack. |
| 00B69F20 | `BSP_LuaStateOwner_RegisterDoFile` | `__thiscall(owner)`, RET | registers `DoFile` on owner+4. Unreferenced. |
| 00B6BD60 | `BSP_LuaScriptCache_CloseState` | `__thiscall(cache)`, RET | closes owner +24h when +20h set. Unreferenced. |
| 00B6BD80 | `BSP_LuaScriptCache_RunRecord` | `__fastcall(LuaStateOwner*, const Record12*)` | inflate and `DoBuffer`. Unreferenced. |
| 00B6C0E0 | `BSP_LuaScriptCache_OpenAndRunAll` | `__thiscall(cache)`, RET | open mask 65h, run every global then every instance record. Unreferenced. |
| 00B6D0A0 | `BSP_LuaScriptCache_Clear` | `__thiscall(cache)` | close, erase names, free payloads. Reached only from 00B6D400. |
| 00B6D1B0 | `BSP_LuaScriptCache_BuildCompressed` | `__fastcall(names, records)` | read every name and its variants, zlib-compress, push records. |
| 00B6D400 | `BSP_LuaScriptCache_Reload` | `__thiscall(cache, names)`, RET 4 | instance twin of 00B6D3C0. Was not a Ghidra function; defined `00B6D400..00B6D432` (`reports/defined_functions.json`). Unreferenced. |

"Unreferenced" means no rel32 call/jump and no absolute dword anywhere in the image's sections
(`tools/image_ref_scan.py`; positive control: the same scan finds all seven `00B65EC0` call sites, and
`scan-bytes` finds the four `MOV EDX, 00B69E00` loads).

## Live callers of the DoBuffer helpers

- `00B65EC0` (unprotected): `00451E50`, the session receive side of the dialog sequencer: a client
  wraps the mission state (`00B66C00`) and executes the Lua **source text** the host sent (message
  +24h length, +28h text), then reads the resulting `Dialogues` table. The host builds that text
  from strings at 00CE4AD0.. (`Dialogues = `, `{ ["message"]= "` ...). A malformed message ends
  the client process silently (rows 1, 8). Also `BSP_MultiScore_ApplySyncText 00780BED`,
  `BSP_MultiScore_SyncToPeer 00782600`, and a call at `006AF8E7` that Ghidra has in no function.
  These are already hosted as `lua_do_buffer_00b65ec0` in `src/multiplayer_session_cc13.cpp`.
- `00B65EF0` (protected): `BSP_StorageOperation_OpenReadArchive 00BD348A` (hosted in
  `src/storage_backend.cpp`).

## The mission state bootstrap (segment 87 code on cc9's state)

`BSP_Game_OnInitOnce` builds `game+1A0Ch` with `00B65E20` around the mission machine's state
(`[[game+1A08h]+4]+4`, 004DD62C..004DD641) and runs `Scripts\global\luaMW_init.lua` (00CE7CBC)
through `00B69D40` with flag 0 (004DD66C). That call is unprotected and outside any `pcall`, so
a syntax or run error in `luaMW_init.lua` or anything it `DoFile`s is a silent `exit(1)` at
startup. In this installation `scripts/global/luaMW_init.lua` (13549 bytes, mtime 2024-07-13)
calls `DoFile("unlocks.lua")` at line 455; the only file of that name is the game-root
`unlocks.lua` (mtime 2026-09-18, a mod artefact next to `unlocked.json`/`killcount.json`). Whether
the VFS resolves a bare `unlocks.lua` to the root depends on the mount list, which this packet did
not trace; if it does not resolve, row 5 applies and `UnlockedClasses` stays undefined until
something else sets it.

## The script cache

`BSP_Game_OnInit` (004E3CFF) calls `00B6D3C0` (`BSP_LuaGlobals_ResetUnlessLoaded_Provisional`,
segment 88, named by cc13-files1) with a name vector holding `interface/_Common.lua` (00CE81AC).
Unless the names equal the cached ones (`00B6C990`), it frees the old payloads (`00B6CF90`),
stores the names at `0108FF30` and calls `00B6D1B0` into the record vector `0108FF40`:

1. Candidates: each name, then its override variants (`00BDEF90`).
2. Each candidate: `vfs->open(path, 2)`, `size()`, read, release. The returned stream is used
   **without a null test** (00B6D289..00B6D294): if `interface/_Common.lua` cannot be opened the
   game faults at init.
3. `compress` (zlib 1.2.1, `00BC9610`) into `size + trunc(size * -0.001) + 12` bytes (FILD/FMUL
   double `00D62C70` = -0.0010000000474974513 / FISTP with truncation), status ignored; the
   payload is copied to an exact-size block.
4. Record `{payload, compressed size, bound}` pushed by `00B6CEF0`.

The only code that reads these records back, `00B6C0E0`/`00B6BD80`, is unreferenced, as are the
instance methods `00B6D400`, `00B6D0A0`, `00B6BD60` and `00B69F20`. So in this build the cache
changes nothing about which scripts run: it costs one read and one compression of `_Common.lua`
at init, and it adds the init-time fault on a missing file. This installation has
`interface/_common.lua` (8499 bytes, mtime 2024-07-13 08:38); the VFS lowercases names.

## C++

`include/bsp/lua_script_loader_cc13.hpp`, `src/lua_script_loader_cc13.cpp` (registered in
`cmake/startup.cmake`): `compress_bound_00b6d1b0`, `build_compressed_00b6d1b0` (VFS reader and
override appender supplied by the caller; returns false where the original would fault) and
`run_record_00b6bd80` (inflate + DoBuffer shape, unprotected like the original, so it must run
inside a `pcall`). Reconstructed and build-tested; no fixture, not wired into the host, not
game-validated. The DoBuffer helpers themselves were not duplicated: they already exist in the
two host files named above.

## Uncertainty

- Twelve `00B6A020` call sites and the 14 `00B69D40` calls without recovered arguments (plus
  callers that are not exported) were not traced to a value (rows 12 and 14).
- Row 18 is inferred from the measured precision field plus the x87 code shape of the embedded
  Lua (`lua_Number` double on x87); no script result was compared.
- The VFS mount order for root-level names (`unlocks.lua`) was not traced.
- `00B6C0E0` reads the global record vector directly and the instance vector through `this`;
  which class owns the +20h/+24h fields was not recovered because nothing constructs it.

## Handoff

Done: items (b) docs/CC13_HINT_WAITS.md, (d) docs/CC13_DIALOG_TYPE_SCAN.md and (e) this doc;
ten segment-87 functions named (one newly defined) and annotated in Ghidra; cache C++ built.
Not done: the remaining unnamed segment-87 functions are LuaObject helpers called by the
profile, mission-save and class readers (`00B65F70`, `00B660F0`, `00B66160`, `00B662E0`,
`00B664E0`..`00B66740`, `00B672F0`, `00B679F0`..`00B67B40`, `00B680A0`..`00B68220`,
`00B68630`..`00B68AD0`), the `std::sort` instantiation `00B69F50` with its helpers
`00B69380`..`00B69CC0` (8-byte elements, reached from `00B6A390`, called by `0081BA80`), and
the vector helpers `00B6BDD0`..`00B6CEF0`. None of them loads or runs scripts.
