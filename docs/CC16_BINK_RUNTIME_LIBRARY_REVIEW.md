# Independent Bink module owner review

No static source-admission blocker was found in candidate
`68ab882bbe7709822936c15b21f6e9804076ffb2`. All 53 header lines and 93 source
lines were inspected, together with existing BinkImports, movie factory,
FMOD and XLive loader patterns. This is a qualified import/module owner review,
not installed runtime ABI, constructor-load, decoder, sound or playback proof.

## Import names and type boundary

Fresh read-only metadata confirms the selected installed `binkw32.dll` is
178440 bytes, PE32/x86, SHA256
`7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724`.
All thirteen source lookup strings are exact named exports, and all thirteen
appear in the installed game's imports. None of these thirteen export rows
is a forwarder. The existing eleven BinkImports types are reused through
`decltype`; their header is unchanged. All arguments are one 32-bit Win32 word.

| Export | Source result / argument count | Stdcall bytes |
| --- | --- | --- |
| BinkOpen | BinkHandle pointer / 2 | 8 |
| BinkClose | void / 1 | 4 |
| BinkSetSoundTrack | void / 2 | 8 |
| BinkSetVolume | void / 3 | 12 |
| BinkDoFrame | int32 / 1 | 4 |
| BinkCopyToBuffer | int32 / 7 | 28 |
| BinkNextFrame | void / 1 | 4 |
| BinkWait | int32 / 1 | 4 |
| BinkGetRealtime | void / 3 | 12 |
| BinkGoto | void / 3 | 12 |
| BinkPause | int32 / 2 | 8 |
| BinkSetSoundSystem | int32 / 2 | 8 |
| BinkOpenDirectSound | **BinkSoundOpen function pointer** / 1 | 4 |

The report records full signatures, decorated names, ordinals and RVAs.
`BinkSoundSystemOpen` is a stdcall function pointer returning another function
pointer, `BinkSoundOpen`. The latter is a stdcall int32 callback taking the
opaque sound-state pointer, frequency, bits, channels, flags and Bink handle.
No vendor sound-state layout is allocated or projected.

The pinned [older vendor-header copy](https://github.com/ioquake/jedi-outcast/blob/e4d728c502c1c50b01bcdaf1076f56d0fe7b982e/code/win32/bink.h)
corroborates those sound callback declarations. Its same-revision
[rad.h](https://github.com/ioquake/jedi-outcast/blob/e4d728c502c1c50b01bcdaf1076f56d0fe7b982e/code/win32/rad.h)
maps Win32 linkage to stdcall and its signed/unsigned words to 32-bit long.
The copy identifies itself as Bink 1.0p, 2000-06-26. It is a declaration copy
in an older game-source repository, not a matching installed SDK or current
vendor certification. Indeed its old track/volume declarations differ in
arity from this DLL. It must not replace the eleven established decoder types.
The selected game install and J:/PROG filename search yielded no matching SDK
header; targeted RAD public declaration searches did not close that gap.

Original game factory 00737710 pushes argument zero and callback thunk
00A4D35C before calling SetSoundSystem thunk 00A4D356, then ignores the result.
Those thunks use IAT CE23D4 and CE23D0 respectively. The callback thunk is
undefined as a Ghidra function; its six live/disk bytes were inspected without
creating one. This supports caller identity and outer stack shape, not the
vendor's eventual callback return/use. Export decoration establishes argument
bytes, not signedness, pointee layout or return type. Installed runtime ABI
therefore remains unproved.

## Acquisition, failure and publication

The module member completes construction before the first export lookup.
Path/display allocation precedes acquisition; after successful acquisition,
every lookup/error-allocation failure unwinds that completed member and
releases its own unescaped load reference. Impl owns the module before both
tables; its successful constructor binds all thirteen pointers. No borrowed
table is exposed before the public constructor returns. No decoder, imported
Bink call, sound setup, fallback result or partial-ready table is supplied.

The owner rejects relative paths, embedded NUL and absent explicit extension.
It performs one LoadLibraryExW attempt with DLL_LOAD_DIR plus SYSTEM32 search
flags, without process search-path mutation or retry. The flags restrict
dependency filesystem search; they do not prove the mapped module's identity
under Windows loader reuse/redirection. Actual mapped identity/hash and loader
acceptance remain composition/gate obligations. See the
[Microsoft loader contract](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw).

On normal destruction, trivial tables are destroyed before the module member's
FreeLibrary call. The noncopyable/nonmovable owner and heap Impl keep borrowed
table addresses stable. A copied function pointer acquires no module ownership.

This is a **composition precondition**, not automatic native lifetime tracking:
all handles, callback work, pending calls and dependent native owners must drain
before destruction. Interrupted work requires retaining the whole aggregate,
including this object, both tables, callback context, decoder host/handles and
renderer/native owners, until process exit. Retaining only HMODULE while
destroying the borrowed tables is insufficient. The source states this rule
explicitly; no actual aggregate adoption/drain proof is supplied here.

## Verification and admission limits

Fresh MSVC Win32 strict object compilation passed C++17, `/permissive- /W4 /WX
/fp:strict /EHsc /O2 /MD`; object machine is x86. Metadata, source/interface
hashes, declaration-copy hashes and object receipt are retained in the report.
No target DLL was loaded; no constructor/imported function, test, probe or game
was executed. No source, CMake, shared metadata or Ghidra state was edited.

Root still owns registration and genuine factory/host/native-owner composition,
actual module/ABI validation and normal/interrupted lifetime gates. The subtitle
guard remains OFF. Body, vendor-body, ABI, activation and playback credit are
zero. The older callback declaration evidence is never promoted to installed
runtime acceptance.
