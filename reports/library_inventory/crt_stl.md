# CRT / STL / EH-runtime inventory — battlestationspacific.exe

Read-only survey (2026-09-09). Sources: `exports/bsp/functions.json` (62,191 entries),
Ghidra Function ID bookmarks (category `Function ID Analyzer`, 712 total, VS2005 x86 library
match), `find_code_gaps`, and decompile/xref probes. Nothing in Ghidra was modified.
Machine-readable version: `crt_stl.json`.

## 1. Address ranges

| Range | What | Entries | Named | `FUN_` | Reconstruct? |
| --- | --- | --- | --- | --- | --- |
| `00bf55be`–`00c2ddd0` | MSVC 2005 CRT + C++ EH runtime + libcpmt (locale, ios_base, exception, new, `_Xlen/_Xran`, `_Fiopen`) + undname (`DName`/`UnDecorator`) | 1,039 | 707 FID + `entry` | 322 (+9 thunks) | **No** |
| `00c2ddd0`–`00c2f3e0` | Import jump thunks (`jmp [__imp_*]`: FMOD, D3DX9_40, DINPUT8, XINPUT1_3, XLIVE, POWRPROF, RtlUnwind) | 114 | 114 | 0 | **No** |
| `00c2f3e0`–`00c5df60` | **Not library.** Game/engine code placed after the CRT by link order (Lua `loadlib.c` at the start, then engine code) | 282 | 3 | 279 | Yes |
| `00c5df60`–`00cc8930` | `.text$x` — EH unwind funclets (`Unwind@…`) | 27,369 funclets | — | 0 | **No** |
| `00cc893b`–`00ce2000` | `.text$yc`/`.text$yd` — dynamic initializers for globals and atexit destructor thunks, plus ~92 KB of initializer code Ghidra never turned into functions | 170 | 3 | 167 | **No** (but see §4) |
| scattered `00408a50`–`00c06f0c` | `Catch_All@…` catch-handler funclets (bodies of the enclosing function's `catch`) | 617 | — | — | **No** (part of parent) |

Totals: 29,309 of 62,191 entries are non-targets → 32,882 remain (before subtracting
other libraries, STL instantiations, and the 130 `thunk_` entries elsewhere).

### Boundary evidence

- Start: `FUN_00bf5520` and `FUN_00bf55a0` are game code (they call `BSP_PhysicalStream_Open`,
  `FUN_00bf42a0`, `FUN_00bf5190`). `00bf55be` is a thunk to `operator new`; `00bf55c3` is
  `operator new[]` (size-overflow check → `bad_alloc`); `00bf5695` is `std::_Xlen`
  ("string too long"); `00bf56d4` is `std::_Xran` ("invalid string position"); `00bf576a`
  (`__Tolower`) is the first FID hit. Nothing between `00bf55be` and `00c2dda6` decompiled as
  game code.
- End: `00c2dda6` (`__mbsnicmp`) is the last FID hit; `00c2ddd0` begins the 114 import
  thunks; `00c2f3e0`+ is game code again (`FUN_00c30570` loads `shaderfx/animtextures.lua`,
  `FUN_00c33710` is a polygon/plane clipper, `FUN_00c3cc30` calls `BSP_Vec3d_*`).
- `.text$x`: all 27,369 entries in `00c5df60`–`00cc8930` are named `Unwind@…`; zero other
  functions inside.
- Tail: `FUN_00cd91d0` = `DeleteCriticalSection(&global)`; `FUN_00ce0a00` = tail-jump to
  a global's destructor; `FUN_00cdc720` = global object destructor (vtable reset + list
  unlink). These functions have only DATA xrefs, all from inside the undefined gaps
  (`00cc8955`, `00cc9d3a`, `00ccc560`, `00cce4f0`, `00cce5a1`, `00cd6ffa`, `00cd7f90`),
  i.e. `push offset dtor / call _atexit` sequences in initializer code that Ghidra left as
  orphaned instructions. `operator new` registers `LAB_00ce1122` with `_atexit`, and that
  address is not a defined function at all.

## 2. The 322 unmatched `FUN_` inside the CRT cluster

Fifteen sampled, fifteen are CRT/STL:

| Address | Identified as |
| --- | --- |
| `00bf55c3` | `operator new[]` |
| `00bf5673` | `std::invalid_argument` scalar deleting destructor |
| `00bf5695` / `00bf56d4` | `std::_Xlen` / `std::_Xran` |
| `00bf6477` | `std::bad_cast::bad_cast(const char*)` |
| `00bf681b` | `operator new` (`malloc`/`_callnewh` loop, throws `bad_alloc`) |
| `00bf6ed1` | `__onexit_nolock` atexit-table growth (`_realloc_crt`) |
| `00bf704d`, `00bf7030`, `00bfa910` | CRT `_CI*` math intrinsic dispatchers (`__startOneArgErrorHandling`, `__math_exit`, SSE2 vs x87) |
| `00bf7420` | `_ftol2` |
| `00bf7b93` | `_snprintf` → `_vsnprintf_c_l` wrapper |
| `00bf85b0` | `floor`/`ceil` with SSE2 fast path (`__frnd`, `__except1`) |
| `00bf8b45` | `_splitpath_s` |
| `00c0cd20` | `__updatetmbcinfo` / `setSBCS` code-page table init |
| `00c1ca85` | small `free` wrapper |
| `00c211a0` | `__convertcp` (MultiByteToWideChar/WideCharToMultiByte helper) |

Large unmatched bodies (`00c08748` 2.4 KB, `00c19279` 2.9 KB, `00c2be39` 2.2 KB,
`00c2b50f` 2.1 KB, `00c140dd` 1.7 KB) call only CRT symbols (`write_char`, `_LocaleUpdate`,
`___libm_error_support`, `__alloc_osfhnd`, `___mtold12`, `__invoke_watson`): printf output
engine, libm error paths, `_sopen` helper, `strtod` engine. Treat the whole cluster as
library by range; do not reconstruct anything in it.

## 3. STL components

Inside the cluster (libcpmt, VS2005 Dinkumware):
- locale/ios_base: `00bf576a`–`00bf5f87` (`__Tolower`, `__Getctype`, `_Init_locks`,
  `_Lockit`, `_Deletegloballocale`, `_Setgloballocale`, `facet_Register`, `_Locinfo`,
  `_Locimp`, `locale::_Init`, `ios_base::_Callfns`, `ios_base::_Addstd`, `__Toupper`).
- string.cpp: `_Xlen` `00bf5695`, `_Xran` `00bf56d4`.
- fiopen.cpp: `_Xfsopen` `00bf60bd`, `_Fiopen` `00bf6172`, `00bf622c`.
- exception classes: `std::exception` ctors/operator= `00bf6340`–`00bf63fe`, `bad_alloc`
  `00bf6802`, `bad_cast` `00bf6477`, `invalid_argument` `00bf5673`; `bad_exception`,
  `bad_typeid`, `__non_rtti_object` exist as namespaces.
- new/delete: `00bf55c3`, `00bf681b`, `_callnewh`, `_free` thunks.
- C++ EH runtime: `00bf6885`–`00bf7d1e` (`__CxxThrowException`, `___CxxFrameHandler3`,
  `_CallSETranslator`, `_UnwindNestedFrames`, `_CallCatchBlock2`, `__ArrayUnwind`,
  `eh_vector_*_iterator`) and `00c06b1c`–`00c07xxx` (`IsInExceptionSpec`, `CallUnexpected`,
  `CatchIt`, `FindHandler`, `terminate`, `unexpected`).
- undname: `00c24000`–`00c28000` (`getTemplateName`, `getZName`, `getScopedName`,
  `composeDeclaration`, `DName`, `Replicator`, `pcharNode`).

Outside the cluster (header templates instantiated in game object files, spread over
`00401000`–`00bf5000`; **not** range-bounded):

| Container | `_Xlen` thrower count | Anchor string |
| --- | --- | --- |
| `std::vector<T>` | 220 | `00ce37e0` "vector<T> too long" |
| `std::map/set<T>` (`_Tree`) | 137 | `00ce47bc` "map/set<T> too long" |
| `std::list<T>` | 76 | `00ce38f8` "list<T> too long" |
| `std::deque<T>` | 2 | `00ce78a0` "deque<T> too long" |
| `basic_string<char/wchar_t>` | 8 callers of `_Xlen` | `00408120`, `00408720`, `004c90d0`, `0056f160`, `0067d5a0`, `007b7cc0`, `0098f9a0`, `0098fa90` |

Each xref is a standalone ~0x40-byte `_Xlen()` emitted once per instantiation (verified:
`FUN_0051ba70` builds a `std::string` from the literal, constructs `length_error`, throws).
With 5–10 emitted methods per instantiation, roughly 2,000–4,500 functions in the game
region are STL template code. This was not measured; measure it by taking the callers of
each `_Xlen` thrower and their sibling methods. `BSP_TextQueue_IncreaseCount` (`00bed2be`)
references "list<T> too long", so one `std::list` instantiation has already been
hand-reconstructed — STL detection should run before more targets are picked.

Other FID hits outside the cluster: `00403560` `??_H` vector-constructor-iterator (CRT
COMDAT, library); `0098e2b0`/`0098f0c0` `_String_const_iterator::operator*` (STL
instantiation); `00778840`/`009ddba0` `??1SubAllocator@details@Concurrency` — provisional,
almost certainly FID false positives (VS2005 has no ConcRT).

## 4. `.text$yc` / `.text$yd` tail

170 defined functions plus undefined gaps of 30,373 B (`00cc893b`–`00ccffdf`, orphaned
instructions), 10,675 B, 26,115 B, 8,470 B, 3,812 B, 4,082 B, 2,603 B, 2,187 B, 3,826 B.
None of it is hand-written code, so none of it is a reconstruction target. It is, however,
the complete list of global objects with constructors/destructors (every `push offset
dtor; call _atexit`), which is a cheap globals inventory for the rebuild. It is also not
counted in the 62,191 denominator, so the denominator undercounts this region rather than
overcounting it.

Aside (out of scope): `find_code_gaps` reports 456 gaps ≥ 512 bytes across `.text`, several
in the game region with orphaned instructions (`00414364`, `00430015`, `0043413b`,
`00457156`), so the game-code count in `functions.json` is also slightly low.

## 5. Import-and-match recommendation

1. **Exclude by range, not by name.** Everything in `00bf55be`–`00c2f3e0`, `00c5df60`–`00ce2000`,
   plus the 617 `Catch_All@` funclets: 29,309 entries. That leaves 32,882 entries before
   other libraries and STL instantiations are removed.
2. **FID is already done.** Ghidra's bundled VS2005 x86 FID (libcmt/libcpmt) produced 707
   hits in the cluster. The 322 remaining `FUN_` are small helpers, SSE2/x87 math variants
   and `FID_conflict` multiple-match cases; re-running FID gains little. No VS2005 toolchain
   is installed locally (only MSVC 14.29/14.44/14.51), so a custom FID from `libcmt.lib`
   is not available; if readable names are wanted, derive them by callee/string matching
   against the VS2005 `crt/src` tree, but that is documentation, not reconstruction.
3. **Rebuild links the stock CRT.** The target toolchain is MSVC 19.x; the CRT, EH runtime,
   `operator new`, locale, iostream and undname come from the modern libcmt/libcpmt. Global
   initializers come from declaring the globals. STL container code comes from using
   `std::vector/map/set/list/string` in the reconstructed headers.
4. **Next measurable step** (for whoever owns the STL question): enumerate the 435
   `_Xlen` throwers from the anchor strings above, collect their callers and the callers'
   siblings (same vtable-less small `__thiscall` cluster), and mark those addresses as
   "template instantiation — regenerate, do not reconstruct".
