# Segment 97: file store, debug profiles, RAM image and error reporter (cc13-files1)

Partition segment 97 covers `00BE4460..00C30570`. Names here are descriptive hypotheses,
not recovered symbols. "Unreferenced" means the disk image has no rel32 CALL/JMP and no
absolute dword pointing at the entry (scan of every section of the installed
`battlestationspacific.exe`, not Ghidra's xref list). Status vocabulary: *named* (ledger
and Ghidra), *reconstructed* (C++ in `src/file_store_cc13.cpp`), *build-tested*. Nothing
here is fixture-tested, ABI-compatible or game-validated.

## Triage

At `fa36636cd` + `origin/main` the segment held 1,666 functions in the index: 1,471 covered
(ledger name, reconstruction record or library tag), 6 EH funclets and 195 raw `FUN_`
entries, all between `00BE44C0` and `00BF5520`. Everything above `00BF55BE` is CRT/STL
and already carries library tags (`operator_new`, `_Lockit`, `LIBCRT_unmatched_*`, ...).

| Cluster | Owner / state | Action |
| --- | --- | --- |
| cFileStore core (`00BE5320..00BE80B0`: open, request, add, remove-by-name, clear, trees) | orch4/cc12 packets `orch4_native_filestore_*` (closed), `docs/NATIVE_FILESTORE_REMOVE_BK.md` | skipped; only the dead dump/remove-by-stream bodies and STL leftovers were raw |
| Application singleton `00BEA8B0`, `00BEA950`, `00BEA990`, `00BEA9A0` | cc12 readiness docs (`CC12_APPLICATION_*`) | skipped |
| PC storage `00BEB0C0`, `00BEB630` | cc13-options1 (landed) | skipped; the remaining PcStorage slots were raw and are named here |
| Physical directory / stream / provider pool | cc12/orch `vfs_*`, `native_physical_*` (closed) | only unreferenced leftovers named |
| Debug feature owner storage (`00BE94F0`, `00BE8210`, `00BE8DF0`, ...) | `docs/NATIVE_DEBUG_FEATURE_OWNER.md` (closed) | its methods, lock, and the profile load/save were raw: named and reconstructed here |
| `00BF0DF0` | referenced by cc9's `src/game_hosts_gunnery.cpp` | not touched |
| CRT/STL from `00BF55BE` | library inventory tags | none needed |

No lease covered any raw address. 97 of the 195 raw entries are unreferenced COMDAT bodies
(mostly checked-iterator and container instantiations); they are named with the evidence
prefix "Unreferenced" and not reconstructed.

Result: 191 segment-97 functions plus six accessors at `00A776B0..00A77720` named (197
ledger records). Seven bodies Ghidra had never defined were created with
`tools/ghidra_define_function.py` (record `reports/cc13_files1_defined_functions.json`):
the six accessors and `00BF0390`. Names were applied with `tools/ghidra_annotate.py
--apply`; prior values are in that run's `local/ghidra-annotations-*.json`.

## Accessors `00A776B0..00A77720`

None of the six has a reference in the image (the two absolute hits for `00A77700` at
`00D27957`/`00D2E157` are unaligned data). Classes come from field use elsewhere:

| Address | Name | Body | Owning class evidence |
| --- | --- | --- | --- |
| 00A776B0 | BSP_DialogChannelRecord_SetSpeakerLayout | `+10h = i; +08h = [00E12EF0 + 4i]`, RET 4 | `BSP_DialogChannelRecord_Copy` (00A77EF0) does the same table read; `00E12EF0` = {0,1,2,6} channel counts |
| 00A776D0 | BSP_SoundStream_IsActive | state `+20h` not 0 and not 3 | stream states `00E12F00`: 0 STOP, 1 STARTING, 2 PLAY, 3 OVER (`docs/CC13_SOUND_STREAMING.md`); same test inline at 00A77740 on `logical+04` |
| 00A776F0 | BSP_SoundStream_IsOver | state `+20h == 3` | as above |
| 00A77700 | BSP_StreamedDialogLogical_SetByte4E | byte `+4Eh = arg`, RET 4 | 5Ch logical channel, bytes 4C/4D/4E (`docs/SOUND_ALTERNATE_OWNER.md`); `BSP_StreamedDialogManager_Update` tests `+4Eh` at 00A78A78 |
| 00A77710 | BSP_StreamedDialogLogical_GetByte4E | returns byte `+4Eh` | as above |
| 00A77720 | BSP_StreamedDialogLogical_GetCategory | returns `+50h` | category written by 00A78150/00A781C0, tested at 00A78A63 |

The meaning of byte `+4Eh` is open. `00A77730` (referenced from 00702788/0070288C) is
outside this packet and still unnamed: it returns true when `[ECX+0Ch]` (a logical channel)
has byte `+4Ch` set or an active stream.

## RAM image and TGA codec (live: terrain)

Vtable `00D68D88`, 1Ch bytes: refcounted base (`+04` count), width `+08`, height `+0C`,
depth `+10`, D3DFORMAT `+14`, pixels `+18`.

| Address | Name | Original ABI | Status |
| --- | --- | --- | --- |
| 00BEFBB0 | BSP_RamImage_Construct | ECX, (w, h, depth, format), RET 10h | reconstructed |
| 00BEFB40 | BSP_RamImage_GetByteSize | ECX; bits(format)*h*w>>3, no depth | reconstructed |
| 00BEFB60..00BEFBA0 | GetData/Width/Height/Depth/Format | ECX, RET | named |
| 00BEFC40/00BEFC70 | Destroy / ScalarDelete (slot0) | ECX | named |
| 00BEFCB0 | BSP_RamImage_LoadFromFile | stdcall(NativeString by value), RET 8, EAX | path rule reconstructed |
| 00BEFFA0 | BSP_RamImage_LoadTgaFile | ECX path*, EAX | named (VFS open mode 2, decode, release) |
| 00BEFDC0 | BSP_RamImage_DecodeTga | ECX stream, EAX | reconstructed |
| 00BF0050 | BSP_RamImage_SaveTgaFile | ECX path*, EDX image, RET | reconstructed (after the open) |

Evidence: `00BEFDC0` reads 18 bytes through stream slot +24h, switches on header byte
16 (`MOVZX [ESP+34h]`; jump table at 00BEFF68/00BEFF7C): 8 with image type 3 or 11 ->
50 (L8), 16 -> 24 (X1R5G5B5), 24 -> 20 (R8G8B8), 32 -> 21 (A8R8G8B8), else 0. It
constructs `(width, height, 1, format)`, reads `ByteSize` bytes directly after the header,
then swaps rows `i` and `h-1-i` for `i < h/2` with row `(bpp>>3)*width` (00BEFF00..00BEFF3E).
The ID field, colour map, RLE and the origin bit are ignored, so RLE type 11 is read raw.
An unknown format gives a 0-byte buffer that the flip then overruns; the C++ skips the flip.

`00BF0050` opens mode 5; a null stream gives `MessageBoxA(0, path, "File is write
protected", 0)`. The header byte at `+2` (image type) receives 20h/18h/10h/3 by format
(00BF0138..00BF014D): only L8 produces a valid type. Bytes 16 and 17 both receive
`depth*8` (00BF0172..00BF018B), so a loaded image (depth 1) is always written as 8 bpp.
Rows are written last to first with row `(bpp>>3)*width`. The routine allocates and frees
an unused row buffer. Only the L8 terrain map round-trips correctly.

`00BEFCB0` upper-cases the path in place (`004FB1F0`, unnamed, outside this packet) and
loads only when `.TGA` occurs anywhere (00467CF0 reverse find), else returns 0.

## Debug feature profiles (dead code except three methods)

Owner layout from `docs/NATIVE_DEBUG_FEATURE_OWNER.md`. All methods take the lock
singleton `00BE8360` (8 bytes, vtable `00D68B68`, critical section `+4`, publication
`0109DB74`). Records are 88h: NativeString name + 80h flag bytes.

Referenced: `00BE8200` (profile count, 0051FCB1), `00BE8CA0` (copy selected profile name,
0051FCDD), `00BE85C0` refresh, `00BE8680` set, and the helpers the loader calls. The loader
`00BE9100` and saver `00BE8900` have no references.

Saver format (00BE8900, stream slot +5Ch = `BSP_NativeStream_WriteString`, whose second
argument is a bytes-written pointer, passed 0):

```
Profile <name>\n
\tFlag <feature>true\n        (or false; no separator before the value)
End\n
```

Strings: `00D68B7C` "Profile ", `00D68B74` "\tFlag ", `00CE4378`/`00CE4370`
"true"/"false", `00CE4390` "\n", `00D68B6C` "End\n".

Loader (00BE9100): VFS slot +8 existence test, then TextScanner over the file. While a
token remains: `ExpectKeyword("Profile")` (result ignored), ReadString name, append an
empty record (00BE8FD0) and assign the name; while `AcceptKeyword("Flag")`: ReadString
feature, `ReadBool`, `FindOrAddFeature`, store the byte at `record + 8 + index` with no
bound; `ExpectKeyword("End")`. Afterwards select record 0 and refresh, then select the
saved name `+10` when it is non-empty (00BE8820).

`ReadBool` (00BEF0B0 over 00BEEC40) parses `%d`: `ok` is set for "0" or any value 1;
`"true"`/`"false"` fail and are not consumed. Combined with the missing separator, a saved
file reloads with feature names like `ShowAItrue`, every flag 0, and the remaining
tokens treated as further profile names. This matters only if the dead paths are revived.

`00BE85C0` loops `for (p = selected+8; count != 0; ++p)`: the bound re-tests the count,
never an index, so with every flag zero it scans past the record until it finds a nonzero
byte. The C++ stops at the 80h flags (documented deviation).

## Error reporter (live: fatal errors)

Singleton `0109DBDC` built by `00BD2180` (outside this packet) through
`BSP_ErrorReporter_Construct` (00BF2C40): vtable `00D68E60`, `+4` critical section,
`+8` message-box byte (1), `+0C` map of thread id -> NativeString vector (node 20h, key
`+0C`, isnil `+1D`).

`BSP_ErrorReporter_Fail` (00BF1F50; callers 00BD2C67 and the unreferenced
`FailUnknownAssertion` 00BF1FB0) composes the text with `00BF1B50` and shows
`MessageBoxA(0, text, "Failure", MB_ICONERROR)` when `+8` is set. Composition, for the
current thread's contexts in order: first character upper-cased for the first entry and
lower-cased for later ones, one trailing `.` or `!` removed, `" because:\n"` appended; then
the message, or `"Empty error message"`. An empty context string dereferences a null
buffer natively; the C++ skips it. `PushContext` (00BF2560) and `PopContext` (00BF1A80) are
unreferenced, so in this build the context list is always empty and the box shows only
the message.

## Chunked stream writer (live: terrain/landscape writers)

`00BF08A0` BeginChunk writes the name length (slot +54h), the name characters (slot +5Ch)
and a zero dword, then pushes `{name, position}` (0Ch records, push_back 00BF0810, copy
00BF0580). `00BF0780` EndChunk takes the top position `o`, `pos = slot+20h`, seeks to
`o-4`, writes `pos-o`, seeks back to `pos`, pops. The budget wrappers 00BF0340 (dword),
00BF0360 (float), 00BF0390 (string), 00BF03B0 (bytes) subtract the bytes the stream reports
from a caller counter, mirroring the existing `BSP_StreamReader_*AndDebit` readers.

## cFileStore listing and removal (dead)

`00BE5DF0` logs `"cFileStore:"` then `"  %s"` for each resident key (tree head `store+18h`)
through the RET-only logger `004254B0`. `00BE7210` walks the resident tree and erases every
node whose value `+14h` equals the stream argument, logging
`"cFileStore::RemoveFile: %s removed!"` and continuing to the end. Neither is referenced.
The C++ models the resident tree as `std::map<std::string, const void*>`; the native key
order (normalized names) is not reproduced.

## Other named groups

- PC storage (vtable `00D68C10`): `00BEAB30` root path, `00BEB190` slot +14h (true when
  free space and no "valid" marker), `00BEB810` slot +18h (walks `+53Ch` calling slot +20h
  with the result discarded and no `FindClose`, then `GetDiskFreeSpaceExA(+534h)` >= 1 MiB),
  `00BEB220`/`00BEB610` destroy and scalar delete.
- Win32 platform vtable `00D68CC4` (`docs/PLATFORM_LOOP.md`): `00BEC140` (+10h) and
  `00BEC190` (+1Ch) no-op, `00BEC150` (+14h) true, `00BEC180` (+18h) false, `00BECA30`
  (+2Ch) returns `WS_CAPTION|WS_THICKFRAME` (slot purposes open).
- Structured reader handles: `00BE9A60` ReadFloat (25 call sites), `00BE9A10` SkipU32.
- Text scanner: `00BEED60` AcceptKeyword, `00BEEF60` ExpectKeyword (same body),
  `00BEEC40` PeekBool, `00BEF0B0` ReadBool.
- `00BEE140` Vector4 scale (ECX out, EDX scalar*, stack vector*), ten callers at 0077A400.
- `00BF0AC0`/`00BF0C60` grow-to-index for string vectors and vectors of them (callers 00B812xx).
- STL instantiations of the file-store trees, the error-context map, the text queue list
  and pointer arrays; EH-only release helpers (`*_ForUnwind`).

## Reconstruction records

`src/file_store_cc13.cpp`, `include/bsp/file_store_cc13.hpp` (namespace
`bsp::cc13_files`): 00BE4770, 00BEF8F0, 00BEFBB0, 00BEFB40, 00BEFDC0, 00BEFCB0 (path rule),
00BF0050 (encode), 00BF08A0, 00BF0780, 00BE9060, 00BE85C0, 00BE8680, 00BE8620, 00BE8820,
00BE8900 (text), 00BE9100 (text), 00BEEC40, 00BF1B50, 00BE5DF0, 00BE7210. These are
behavior models over host data with new C++ interfaces; build-tested only.

## Open questions

- Byte `+4Eh` of the logical dialog channel; owner class of `00A77730`.
- Purpose of Win32 platform slots +10h..+1Ch and +2Ch.
- `00BF0A30` (returns 1, caller 00B8174E) owner.
- Undefined tiny bodies remain between `00BE5280` and `00BE5320` (allocator thunks that
  call 00BE4F30/00BE4FA0); not defined in Ghidra.

## Handoff (2026-10-12)

The segment is exhausted for game code. After this packet the index shows six raw entries
in the band: the four cc12 application functions (`00BEA8B0`, `00BEA950`, `00BEA990`,
`00BEA9A0`), `00BF0DF0` (cc9 gunnery reference) and `00C03B44` (newly defined CRT body
with no references). The undefined allocator thunks at `00BE5280..00BE5320` and
`00BEC480`/`00BEC490` have no references either; defining them would only add names.

Next steps if anyone continues: name `004FB1F0` (in-place ASCII upper-case of a
NativeString, used by `00BEFCB0`), `00BD2180` (error reporter singleton getter) and
`00BD2B30` (fatal error caller of `BSP_ErrorReporter_Fail`), and `00A77730`. Wiring the
TGA codec into a host would let the terrain path use `decode_tga_00befdc0` directly.
