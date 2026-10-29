# Retained Bink runtime library dependency

`BinkRuntimeLibrary` owns one explicit installed Win32 DLL load and resolves the eleven existing `BinkImports` entries plus the two factory sound exports before publishing either borrowed table. It creates no decoder, installs no sound system and calls no imported Bink function. This packet is authored and strict-object-compiled; independent admission and production composition remain with the integrator.

Base: `6bb21d924cf205d53002ea50501988c8d0dea195`. Owned source is limited to `include/bsp/bink_runtime_library.hpp` and `src/bink_runtime_library.cpp`. CMake, startup/factory/renderer composition, existing types, locale/VFS providers and guards are unchanged.

## Exact installed metadata

The explicitly inspected `I:/SteamLibrary/steamapps/common/Battlestations Pacific/binkw32.dll` is 178,440 bytes, PE32/x86 (014Ch/010Bh), SHA-256 `7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724`. Fresh `dumpbin /headers /exports` and the installed executable import metadata confirm all thirteen names below. This is file metadata evidence; the DLL was not loaded or executed.

| Required decorated export | Ordinal | RVA |
|---|---:|---|
| `_BinkOpen@8` | 44 | `0x0000D690` |
| `_BinkClose@4` | 16 | `0x0000F850` |
| `_BinkSetSoundTrack@8` | 65 | `0x0000D320` |
| `_BinkSetVolume@12` | 67 | `0x000101E0` |
| `_BinkDoFrame@4` | 25 | `0x0000F340` |
| `_BinkCopyToBuffer@28` | 20 | `0x0000E750` |
| `_BinkNextFrame@4` | 43 | `0x0000F3C0` |
| `_BinkWait@4` | 70 | `0x0000FAC0` |
| `_BinkGetRealtime@12` | 33 | `0x0000FF30` |
| `_BinkGoto@12` | 40 | `0x0000F660` |
| `_BinkPause@8` | 49 | `0x0000FC40` |
| `_BinkSetSoundSystem@8` | 64 | `0x0000CEA0` |
| `_BinkOpenDirectSound@4` | 45 | `0x00011FC0` |

All eleven decoder members use `decltype` of the existing `BinkImports` fields. The factory table keeps `BinkSetSoundSystem` as a signed 32-bit stdcall function taking a sound-system-open callback and a 32-bit argument. `BinkOpenDirectSound` is that callback: it returns a sound-open function pointer. The sound state stays opaque and is never allocated or dereferenced here.

The callback type is corroborated by the [public Bink API declarations](https://raw.githubusercontent.com/ioquake/jedi-outcast/e4d728c502c1c50b01bcdaf1076f56d0fe7b982e/code/win32/bink.h#L41-L51) and sound-system declarations in that header. This older 1.0p header is a declaration reference for those two types, not the installed 2009 SDK and not a source for changing the eleven existing decoder signatures. The installed decorated names establish the relevant Win32 stack widths; no vendor body or dynamic ABI behavior was examined. Full declaration provenance/hash is in the report.

## Ownership and failure contract

The constructor rejects a non-absolute path, missing explicit extension or embedded NUL, then uses `LoadLibraryExW` with `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32`. The explicit module path has no default or bare-name fallback; dependency filesystem lookup is restricted to that directory and System32, under normal Windows loader module-reuse rules. A missing extension is rejected so LoadLibrary cannot append an implicit `.dll`. There is no process search-path mutation or retry with other flags/names. Unsupported loader flags produce the actual load error. The composing caller must select and freeze the audited installed artifact; this API does not hard-code a machine-specific install path or hash.

A completed RAII module member exists before export resolution begins. If any lookup or subsequent constructor operation throws, only that unescaped owned load reference is released. Missing-export errors include the selected path, exact export name and captured Win32 error. The public constructor returns only when all thirteen pointers are bound; no partial ready table or successful missing-import shim exists. This adapts the real XLive/FMOD module and checked-lookup patterns, with eager whole-table validation.

The public owner is noncopyable and nonmovable. Its unique heap `Impl` holds both tables in stable storage and owns the module reference. The two accessors return borrowed const references. Copied function pointers are also borrowed; a copy does not retain the module.

On **normal teardown**, the composing runtime must first close all real decoder handles, stop/finish native sound callback work and complete dependent shared/native-owner draining. It can then destroy the import owner, whose module is released last. This class owns no Bink handle and invents no automatic decoder or sound teardown.

On **interrupted native work or teardown**, the composing runtime must retain the entire dependent aggregate, including this library object, its stable table storage, callback context and all real decoder/renderer/native owners, until process exit. Its existing failed-native-path policy may release its owning `unique_ptr` without destroying that aggregate. Do not unwind this library, retain only an HMODULE while destroying table storage, or free the DLL before those consumers finish. This module-only API cannot count handles or certify that native consumers have drained.

## Validation and admission boundary

The new source compiled successfully as an MSVC Win32/x86 Release object with `/std:c++17 /permissive- /W4 /WX /fp:strict /EHsc /O2 /MD`. Object headers confirm machine 014Ch. Compilation used the source hashes recorded in the [receipt](../reports/cc16_bink_runtime_library.json). No module load, decoder call, game, probe, new test or test framework was used. This is object compilation, not CMake/link/full-suite/runtime acceptance.

Root still owns CMake admission, actual factory registration/publication/deletion and startup composition. The same retained library must reach the genuine factory and concrete `MovieDecoderHost` with proper normal and interrupted ownership. Existing locale and BDD600 filename APIs remain separate dormant provider steps; no alias or default provider is added. Actual renderer custody, the real Bink handle/realtime triple, widget/localisation/extratitles, Lua library-set and callback reachability are still required before subtitle activation.

`kMovieSubtitlesBound` stays OFF. This packet adds no native/vendor-body, original ABI, full decoder/clock, activation or playback credit. Review performed here is author self-review only.
