# WinMain and application construction

Addresses: 008f81f0, 008f7db0, 00436430, 004c5e60, 00425850, 00737970, 00bea970.

Every descriptive name below is a hypothesis, not a recovered symbol. Control flow was taken
from the disassembly; the Ghidra pseudocode for `008f81f0` is wrong in three places that are
called out under "Pseudocode corrections".

## Entry point

`BSP_WinMain` at `008f81f0` is `__stdcall(HINSTANCE, HINSTANCE, LPSTR, int)`. Its only caller
is the CRT at `00bfd0dd`. Both exits are `XOR EAX,EAX` followed by `RET 10h` at `008f83f9`
and `008f8496`, so the process exit code is always zero, whichever path runs.

None of the four arguments is read. The frame is `SUB ESP,0xc48` plus `PUSH ESI`/`PUSH EDI`,
and no instruction in the body addresses the frame above the return address, so the command
line is not consulted here at all. Command-line handling, if the game has any, is elsewhere.

## Recovered sequence

| Step | Address | Behavior |
| --- | --- | --- |
| COM apartment | `008f81f8` | `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` |
| COM security | `008f820a` | `CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL)` |
| Game Explorer | `008f8245` | `CoCreateInstance(CLSID_GameExplorer, NULL, CLSCTX_ALL, IID_IGameExplorer, &p)` |
| GDF path | `008f824f` | `GetCurrentDirectoryA(0xbfe, buf)`, append `\battlestationspacific.exe`, widen |
| Access check | `008f82aa` | `IGameExplorer::VerifyAccess(path, &has_access)`, vtable slot `+18h` |
| Denied | `008f82ee` | CRT `exit(0)` at `00bfbdbb` |
| Release | `008f82c4` | `IUnknown::Release`, vtable slot `+08h`, only when the pointer is non-null |
| COM teardown | `008f82cc` | `CoUninitialize` |
| Random threads | `008f82d2` | `BSP_RandomThreads_Initialize`, one byte slot, `BSP_RandomThreads_RegisterCurrent` |
| Single instance | `008f8301` | `CreateMutexA(NULL, TRUE, "MidwayThreadMutex")` and `GetLastError` |
| Affinity | `008f83fc` | `SetThreadAffinityMask(GetCurrentThread(), 1)` |
| Resource factory | `008f840b` | `BSP_GameResourceFactory_GetSingleton`, result stored to `00f8d31c` |
| Application | `008f8419` | Construct, `Initialize(0, "cachedload")`, run loop, Shutdown, Destruct |
| Teardown | `008f8449` | Singleton manager at `01090aa0`, `CloseHandle`, random-thread shutdown |

The two GUIDs are the documented Windows Game Explorer pair: `CLSID_GameExplorer`
`{9a5ea990-3034-4d6f-9128-01f3c61022bc}` at `00d16a94` and `IGameExplorer`
`{e7b2fb72-d728-49b3-a5f2-18ebf5f1349e}` at `00d16844`, byte-verified in the image. Slot
`+18h` of that interface is `VerifyAccess`, which reports whether parental controls allow the
titled binary to run. A `FALSE` result ends the process through the CRT exit at `008f82ee`,
which runs before `Release` and before `CoUninitialize`.

Both COM initialization failure branches jump to `008f82cc`, so `CoUninitialize` is called
even when `CoInitializeEx` failed. The `Release` at `008f82c4` is guarded by the interface
pointer rather than by the `CoCreateInstance` result.

### Single instance

`CreateMutexA` requests initial ownership. The already-running path at `008f8322` is taken
only when the handle is non-null **and** `GetLastError()` is `ERROR_ALREADY_EXISTS` (`0xb7`);
a failed `CreateMutexA` therefore lets a second copy start.

That path resolves the language, shows a localized `MessageBoxW(NULL, text, caption,
MB_ICONHAND)`, frees the language buffer inline at `008f83cb` and returns zero. It closes
nothing: the mutex handle stays open, the per-thread random slot is not released, and
`BSP_RandomThreads_Shutdown` never runs. The process is exiting, so this is harmless, but it
is asymmetric with the normal path.

Message pairs, chosen by `00425850` comparisons in the order english, french, italian,
german, spanish:

| Language | Text | Caption |
| --- | --- | --- |
| english / unmatched | `00d16a10` | `00d16a00` `Error` |
| french | `00d16990` | `00d1697c` `Erreur` |
| italian | `00d16920` | `00d16910` `Errore` |
| german | `00d168c0` | `00d168b0` `Fehler` |
| spanish | `00d16858` | `00d16a00` `Error` |

Two quirks are in the image, not transcription errors: the italian text begins with a space,
and the french text uses U+2019 as its apostrophe (`64 00 19 20` at `00d169e4`). Spanish sets
only the text register and falls into the english caption load at `008f83ba`.

### Normal path

`BSP_Application_Construct`, `BSP_Application_Initialize`, `BSP_Platform_RunLoopDispatch`,
`BSP_Application_Shutdown` and `BSP_Application_Destruct` all receive the same `this`, a
stack object at frame `+0x1c`. `BSP_Application_Initialize` is `__thiscall` with `RET 8` and
is called as `Initialize(this, 0, "cachedload")`; the literal zero is pushed at `008f8423`
and `"cachedload"` lives at `00ce8168`.

`BSP_Platform_RunLoopDispatch` (`00bea800`) is given the application object in `ECX` but
ignores it: it loads the platform singleton at `0109cf04` and tail-jumps to virtual slot
`+24h`, which `docs/PLATFORM_LOOP.md` identifies as the message loop `00bec1a0`.

Teardown order is singleton manager, then mutex, then random threads. When `01090aa0` is set,
`BSP_SingletonLifetime_Destroy` runs, the pointer is freed, and the global is cleared to zero
at `008f8463`. `BSP_RandomThreads_Shutdown` at `008f8486` is unconditional; only the
unregister and free of the per-thread slot are gated on the slot being non-null.

## Object layout

`BSP_Application_Construct` (`00737970`) chains to `BSP_ApplicationLoop_Construct`
(`00bea970`), which chains to `BSP_ApplicationSingleton_Construct` (`00bea810`).

| Offset | Set by | Value |
| --- | --- | --- |
| `+00` | all three, last wins | vtable `00d68bc4`, then `00d68bc8`, then `00cfeab0` |
| `+04` | `00bea970` | byte, zeroed |
| `+08`..`+14` | `00737970` | four dwords, zeroed |
| `+18` | `00737970` | byte, set to 1 |
| `+19`, `+1a` | `00737970` | bytes, zeroed |

Size is `0x1c`: WinMain puts the object at frame `+0x1c` and the next local, the current
directory buffer, at frame `+0x38`. The `00cfeab0` scalar deleting destructor is `00737e00`.

`BSP_ApplicationSingleton_Construct` publishes `this` into the global at `00e1ae90` and
registers it with `BSP_SingletonLifetime_Register`, holding the manager's critical section
(from `BSP_SingletonLifetime_GetManager` `+0x10`) and bumping its recursion counter. It has a
SEH frame at `00cc7240`.

## String classes

Two distinct classes share the `{length at +0, data at +4}` shape.

The **wide** string is built by `BSP_WideString_ConstructFromCString` (`004c5e60`,
`__thiscall`, `RET 4`, returns `this`) and released by `BSP_WideString_Destruct` (`00436430`,
`__fastcall`). The destructor frees `length * 2 + 2` bytes, which is what proves the element
is 16 bits. Widening at `008f8288` zero-extends each byte; it is not
`MultiByteToWideChar`, so a current directory outside ASCII reaches `VerifyAccess` as raw
ANSI bytes. `BSP_WideString_Resize` (`004c53e0`) returns immediately when the length is
unchanged, so resizing a fresh `{0,0}` object to zero leaves the data pointer null; that is
why `008f828d` substitutes the shared empty wide string at `00f89988`, which is all zeroes
and referenced only from WinMain.

The **narrow** string is the existing `BSP_NativeString` family. `00425850` compares one
against a C string: a null data pointer matches only a null or empty candidate, otherwise the
result is `_stricmp`. The inline destructor at `008f83d7` frees `length + 1` bytes, which is
how the language object is distinguished from the wide path object even though the
pseudocode gives them the same local.

## Language resolution

`BSP_Startup_ResolveLanguage` (`008f7db0`) is `__thiscall` with `ECX` pointing at an out
native string (`LEA ECX,[ESP+0xc]` at `008f8322`); it returns `this` in `EAX` and has an SEH
frame at `00ca4de7`.

1. Seed the string with `"english"` (`00d15b14`) through `0041e870`.
2. `SHGetSpecialFolderPathA(NULL, buf, CSIDL_PERSONAL, fCreate=TRUE)`, concatenate
   `\Battlestations-Pacific` (`00d15af0`) with `BSP_NativeString_Concat`, then append
   `\options.txt` (`00d15ae0`).
3. `fopen(path, "rt")`.
4. **File opened**: size the file with `fseek`/`ftell`, allocate a `0x10` byte header through
   `operator new` and construct a memory backing (`BSP_MemoryBacking_Construct`), read the
   whole file, and wrap it with `BSP_MemoryStream_CreateFromBacking`. Tokens are pulled in a
   loop at `008f8010`; an empty token ends the scan. Each token is compared against
   `"Language"` (`00d15f1c`) with the null-safe `BSP_CString_CompareInsensitive`; the first
   match advances once and assigns the following token with
   `BSP_NativeString_AssignCString`, then breaks. The backing and the stream are released
   through `InterlockedDecrement` on their refcount at `+4`.
5. **File not opened**: `RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\Eidos\Battlestations
   Pacific", 0, KEY_READ)` and `RegQueryValueExA(..., "language", ...)`. The value is used
   only when the returned type is `REG_DWORD`. The switch at `008f7f9c` maps `0x407` german,
   `0x40a` spanish, `0x40c` french, `0x410` italian, anything else english.

The registry is a fallback, not an override: it is read only when the options file cannot be
opened.

## Pseudocode corrections

1. The decompiler reuses one stack slot for the Game Explorer out pointer and for the
   language string object, both at frame `+0x8`. `local_c48` is the interface pointer early
   and the string's **length** late; `iStack_c44` is the string's **data** pointer. The tail
   destructor reads them in that role.
2. The `DAT_01090aa0` branch is shown as returning. It does not: `008f845b` falls through to
   `008f8460`, which clears the global and continues into `CloseHandle`.
3. `BSP_RandomThreads_Shutdown` is shown inside the "no thread slot" branch. Both branches
   converge on `008f8486`, so it always runs on the normal path.

## Reconstruction state

| Address | Name | State |
| --- | --- | --- |
| `008f81f0` | `BSP_WinMain` | reconstructed, build-tested |
| `008f7db0` | `BSP_Startup_ResolveLanguage` | analyzed; decision logic reconstructed, I/O left to the host |
| `004c5e60` | `BSP_WideString_ConstructFromCString` | reconstructed as `startup_widen_path`, build-tested |
| `00436430` | `BSP_WideString_Destruct` | analyzed; the C++ port uses `std::wstring` lifetime instead |
| `00425850` | unchanged, existing ledger record | analyzed; semantics reproduced in `language_equals_00425850` |
| `00737970` | `BSP_Application_Construct` | analyzed, layout recovered; injected as a host step |
| `00bea970` | `BSP_ApplicationLoop_Construct` | analyzed |
| `00bea810` | `BSP_ApplicationSingleton_Construct` | analyzed |
| `004261a0` | `BSP_NativeString_Concat` | analyzed |
| `0041e350` | `BSP_NativeString_AssignCString` | analyzed |
| `00438e10` | `BSP_CString_CompareInsensitive` | reconstructed, build-tested |
| `004c53e0` | `BSP_WideString_Resize` | analyzed |

Nothing here is fixture-tested, ABI-compatible or game-validated. `bsp::run_win_main` takes
explicit arguments and a `StartupHost` interface; it is not a drop-in replacement and the
`StartupHost` methods are an integration contract, not stubs of the game.

## Uncertainties and what remains

- The options tokenizer (`008d8a70`, `008d8960`, `008d99f0`, `008d9f20`) is not
  reconstructed. `startup_language_from_options_tokens` therefore works over an
  already-tokenized sequence, and what the native reader yields at end of file after a
  trailing `Language` key is not established; the port assigns nothing in that case.
- The delimiter set `" \t\r\n,"` sits at `00d15f2c`, adjacent to the `"Language"` literal,
  but no reference to it was proven from this routine.
- The native GDF path buffer has `0xc10` bytes for a `0xbfe` byte directory plus a 26
  character suffix, an eight byte shortfall at the maximum. Whether `GetCurrentDirectoryA`
  can return a path long enough to reach it was not investigated; the port uses a sized
  string.
- `0041e870`, the construct-from-C-string counterpart of `0041e350`, is leased to another
  packet and was not renamed. It copies `length + 1` bytes with `preserve=1`, where
  `0041e350` copies `length` bytes with `preserve=0`.
- `BSP_Application_Initialize`, `RunFrame`, `Shutdown` and `Destruct` are owned elsewhere and
  were treated as external calls. The meaning of the first `Initialize` argument (literal
  zero) and of the application flags at `+18`..`+1a` is unresolved.
- The per-thread random slot is a one byte `operator new` whose value is never read. Neither
  `BSP_RandomThreads_RegisterCurrent` nor `UnregisterCurrent` takes an argument, so the
  allocation only acts as a success flag for the register/unregister pair.

## Correction from docs/STARTUP_COM_BINDING.md

Live assembly and SDK constants correct the COM labels in the earlier table:
`008F81F8` pushes `8`, which is `COINIT_SPEED_OVER_MEMORY` with the default
multithreaded model, not `COINIT_APARTMENTTHREADED`. The security call passes
authentication level `0` (`RPC_C_AUTHN_LEVEL_DEFAULT`), not `NONE` (`1`).
The process host now calls real Game Explorer creation/access/release and
preserves the pointer release guard independently of creation HRESULT. The
access BOOL is not preinitialized and its HRESULT is ignored. `BFBDBB`
calls the full-cleanup CRT path with two zero flags, including the reverse
on-exit table; host exit now uses `std::exit`. The isolated current-process
COM/access query and on-exit callback checks passed. See the named correction
document and report for exact scope and remaining ownership boundaries.
