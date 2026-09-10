# Application initialize: locale tables (packet `app_init_locale`)

Addresses: 008d4870, 008d4890, 008d7bc0, 00aa09d0, 00aa06d0, 00aa0020, 00aa0d30, 00a9fc30,
00a9ec70, 00a9fad0, 00a9f4b0, 0073c240, 0073bae0, 004260b0, 00435c40

Every name in this document is a hypothesis, not a recovered symbol. The packet covers the
locale half of the map's `app_init_locale_gui`; the GUI singleton (`004c14c0`) and the GUI
object at `0073c960` are not in it.

## Scope note on 0073bae0

The packet brief calls `0073bae0` "the globals table". That is wrong, and the map
(`docs/APP_INITIALIZE_MAP.md` line 59) already had it right: `0073bae0` is the font
definition loader for `Fonts/Fonts.lua`, and the `globals` table name is pushed by
`00aa0d30` at `0073e103`. `0073bae0` is documented here only for its one locale input,
the language entry's `fontpath`; its font work belongs to the font packets.

## Call sequence in BSP_Application_Initialize

Disassembled at `0073e05e`-`0073e13c`:

| Address | Call | Arguments |
| --- | --- | --- |
| 0073e06a | `008d4870` | `ECX = 00f88980`, the settings object |
| 0073e08f | `00aa09d0` | `ECX = [00f8bc4c]`, one native string holding the returned name |
| 0073e103 | `00aa0d30` | `ECX = [00f8bc4c]`, the 7-byte literal `globals` (`00cf7774`) |
| 0073e135 | `00aa06d0` | `ECX = [00f8bc4c]`, `force = 0` |
| 0073e13c | `0073bae0` | `ECX = EBP` |

Because `00aa09d0` gates its file load on the registered-name vector being non-empty and
`globals` is pushed only afterwards, the load that actually reads the table on this path is
the `00aa06d0` call at `0073e135`, which runs because the map is still empty. `008d5030`
uses the same pair with `force = 1` (`008d5133`).

## Language selection

`008d7bc0` builds the language table once, gated on `00f88978`, and stores it behind the
pointer global `00f88974`. It seeds the strings `lockit` (`00d15e08`) and `.lng`
(`00d15e10`) and walks the descriptors; the six hardcoded names at `00d15d88`-`00d15df4`
are `lockit/englishauthentic.lng`, `lockit/spanish.lng`, `lockit/german.lng`,
`lockit/italian.lng`, `lockit/french.lng` and `lockit/english.lng`.

A descriptor is whitespace-separated `key value` text. The four keys are matched
case-insensitively with `00438e10` and stored as the four `{size, pointer}` native-string
pairs of a `0x20`-byte entry:

| Entry offset | Key | Literal | Accessor |
| --- | --- | --- | --- |
| +0x00/+0x04 | `lanfile` | 00d15d80 | `008d4870` |
| +0x08/+0x0c | `lockit_id` | 00d15d74 | not established |
| +0x10/+0x14 | `voice_dir` | 00d15d68 | not established |
| +0x18/+0x1c | `fontpath` | 00d15d5c | `008d4890` |

`008d4870` is `__thiscall`, `ECX` = the settings object, no stack argument, plain `RET`.
The assembly is five instructions: `[ECX+4] << 5`, add `[00f88974]`, load `+4`, and on
null return `0xf88a3c`. `00f88a3c` is zeroed in the image, so the fallback is the empty
string, not a language name. `008d4890` has the same shape for `+0x18`/`+0x1c` and falls
back to the empty string at `00ce3a0c`.

`docs/WINMAIN_STARTUP.md` covers how the language *name* is produced earlier:
`BSP_Startup_ResolveLanguage` (`008f7db0`) seeds `english`, reads the `Language` token from
`options.txt`, and falls back to the `HKLM\SOFTWARE\Eidos\Battlestations Pacific`
`language` REG_DWORD, mapping `0x407` german, `0x40a` spanish, `0x40c` french, `0x410`
italian. How that name reaches the integer index at settings `+4` was not established
here; that index, not the name, is what `008d4870` consumes.

### Installed descriptor

`I:/SteamLibrary/steamapps/common/Battlestations Pacific/lockit/englishauthentic.lng`, 80
bytes:

```
lanfile englishauthentic
lockit_id FE.opt_englishauthentic
voice_dir authentic
```

No `fontpath` key, so `008d4890` returns the empty string for the shipped English build and
`0073bae0` passes an empty language font path to `007371d0` and `00ac3910`.

## The two lockit loaders

Both build `lockit/` (`00d5bcd0`) + name + `.lan` (`00d5bcc8`) into one accumulator, hand
it to `00aa0020`, and then probe the same path with the decimal suffixes `0`..`98`
(`004260b0` formats with `sprintf("%d")`), stopping at the first name that
`BSP_VFS_ResolveExistingName` (`00bdf4c0`) rejects. So the numbered variants are
`lockit/<language>.lan0` .. `lockit/<language>.lan98`, digits after the extension. The
installed tree has none.

`00aa09d0` is `__thiscall`, `ECX` = the manager, one native-string pointer, `RET 4`
(`00aa0cbe`):

1. `00435c40` compares the argument with the stored name at `+0x4018`; equal means return.
2. Copy the argument into `+0x4018`.
3. `0073c240` on `manager+0x14` clears the map.
4. Load only when the vector at `+8`/`+0xc` is non-empty *and* the count at `+0x4014` is 0.
5. Refresh the GUI through `004c12b0` and `00aa4650`. Not reconstructed.

`00aa06d0` is `__thiscall`, `ECX` = the manager, one `char` stack argument, `RET 4`. It
loads when the count at `+0x4014` is zero or the flag is set, uses the name already stored
at `+0x4018`, and has neither the registered-name gate nor the GUI refresh.

### Manager layout (00f8bc4c)

| Offset | Field | Evidence |
| --- | --- | --- |
| +0x04..+0x10 | `vector<NativeString>` of registered table names, base +8, end +0xc, cap +0x10 | `00aa0d30` is `ADD ECX,4; JMP 00450540`; `00aa09d0` reads `+8`/`+0xc` with stride 8 |
| +0x14..+0x4013 | 0x1000 hash bucket heads | `0073c240` loops 0x1000 and memsets 0x4000 |
| +0x4014 | live entry count | `0073c240` zeroes `base+0x4000`; both loaders test it |
| +0x4018/+0x401c | current language name | `00aa06d0` at `00aa0718`/`00aa073e` |
| +0x4020..+0x402f | `vector<uint16>` from the `.lanx` sidecar | `00aa0020` phase two |
| +0x4030..+0x403f | second `vector<uint16>` from the sidecar | `00aa0020` phase two |

`globals` is a *category* prefix inside the table file, not a file name: the installed
English table has 509 records whose first token is `globals`. Nothing in either loader
reads the vector's contents, only whether it is empty.

## The .lan record format

`00aa0020` takes the manager in `ECX` plus an 8-byte native string by value and two ints on
the stack, collects every mount holding the name through `00bdef90`, and parses each
stream. The state machine at `local_4f80` gives the grammar:

```
record := category ' ' name ' ' utf16le-text 00 0A 00 0D
key    := category '.' name          ; '.' is 00ce3a70
```

The two tokens are read one byte at a time until a space, which is overwritten with NUL.
The text is read as 16-bit units and ends when the previous unit is `0x0A00` and the
current one is `0x0D00`, that is the four file bytes `00 0A 00 0D`. There is no BOM and no
header. Keys are ASCII; text is UTF-16LE.

Native buffer sizes, neither bounds-checked: the composed key goes into `acStack_4f5c`,
152 bytes, and the text into `auStack_4e2a`, 19994 bytes.

Values are stored with `00a9fc30`, which returns an existing node when the key already
matches, so a repeated key overwrites the earlier text: last record wins.

### The .lanx sidecar

When the second int argument is zero, `00aa0020` clears both `uint16` vectors, opens the
same name with `x` appended (`00ceb488`) and reads 16-bit pairs until end of stream,
appending the first of each pair to the vector at `+0x4020` and the second to `+0x4030`.
The installed `englishauthentic.lanx` is zero bytes, so the vectors stay empty and the
purpose of the pairs was not established.

## The map

`00a9fc30` (insert-or-get) and `00a9ec70` (find) share one hash:

```
h = 0
for each byte c of the key:
    if (unsigned char)(c + 0xbf) < 0x1a:  c += 0x20     # only 'A'..'Z'
    h = h * 0x83 + (signed char)c                        # bytes >= 0x80 subtract
bucket = h & 0xfff
```

Chains compare length first, then `__stricmp`. Nodes are `0x1c` bytes: key `{size,ptr}` at
`+0`/`+4`, wide text `{size,ptr}` at `+8`/`+0xc`, prev `+0x10`, next `+0x14`, pool index
`+0x18`. New nodes are pushed at the head. `0073c240` frees every chain through the pooled
allocator behind the critical section at `00e17698`, then memsets and zeroes the count.

Lookup entry points: `00a9fad0` splits the key on `|` with `_strcspn` and calls `00a9f4b0`
per piece; `00a9f4b0` strips a leading `^`, treats a leading `.` as a marker that suppresses
the map lookup, resolves the rest with `00a9ec70`, falls back to `00f8bc54`, then scans the
result for the `#` marker at `00cf828c` and substitutes in a loop. Only the split was
reconstructed; the substitution and the `^` meaning remain open.

## Installed-file check

Read-only against `I:/SteamLibrary/steamapps/common/Battlestations Pacific`. The game
installation was not modified.

`lockit/englishauthentic.lan`, 999674 bytes, parsed with the recovered grammar:

| Measure | Value |
| --- | --- |
| Records parsed, whole file consumed | 6864 |
| Occurrences of `00 0A 00 0D` in the file | 6864 |
| Occurrences of `0D 0A` in the file | 0 |
| Distinct keys after case-insensitive insert | 6856 |
| Occupied buckets of 4096 | 3356 |
| Longest chain | 10 |
| Longest text | 1448 units |
| Longest category / name token | 14 / 53 bytes |

Both the Python model and the C++ port produce those same numbers, and `find_00a9ec70`
resolves `FE.briefing` and `fe.BRIEFING` to the same 8 units, `Briefing`. The eight
duplicate keys resolve to the last record in file order, matching `00a9fc30`.

`lockit/future/japanese.lan` does **not** conform: it uses the five-byte separator
`00 0D 0A 00 0D` 5923 times and stores at least one value as 8-bit `?` placeholders. It
sits under `future/`, outside the path either loader builds, and the recovered parser
rejects it. Treat it as an unshipped artifact of a different writer, not as counter-evidence.

## What was reconstructed

`include/bsp/locale_tables.hpp` and `src/locale_tables.cpp` provide `bsp::LocaleTables`
with an injected `FileReader`; the module owns no file code, matching the repo rule that
the mounted-stream and physical-file layers already exist. The port keeps the native
bucket count, hash, chain order, overwrite-on-duplicate rule, numbered-probe order and
sidecar handling.

Two deliberate deviations, both because the native code is unguarded: records whose key
would exceed 152 bytes or whose text would exceed 19994 bytes are rejected with an error
instead of overflowing a stack frame.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 008d4870 | BSP_GameSettings_GetLanguageName | reconstructed, build-tested, installed-file-checked |
| 008d4890 | BSP_GameSettings_GetLanguageFontPath | reconstructed, build-tested |
| 008d7bc0 | BSP_GameSettings_BuildLanguageTable | analyzed; descriptor parse reconstructed and installed-file-checked; the enumeration and the five skip tests at 00553c80 are not reconstructed |
| 00aa09d0 | BSP_Localization_LoadTable | reconstructed, build-tested, installed-file-checked; GUI refresh tail excluded |
| 00aa06d0 | BSP_Localization_ReloadTables | reconstructed, build-tested, installed-file-checked |
| 00aa0020 | BSP_Localization_ParseTableFile | reconstructed, build-tested, installed-file-checked; mount collection left to the injected reader |
| 00aa0d30 | BSP_Localization_RegisterTableName | reconstructed, build-tested |
| 00a9fc30 | BSP_StringMap_InsertOrGet | reconstructed, build-tested, installed-file-checked |
| 00a9ec70 | BSP_StringMap_Find | reconstructed, build-tested, installed-file-checked |
| 0073c240 | BSP_StringMap_Clear | reconstructed, build-tested; pooled allocator not reproduced |
| 00a9fad0 | BSP_Localization_ResolveKeyList | analyzed; only the `|` split reconstructed |
| 00a9f4b0 | BSP_Localization_ResolveKey | analyzed, provisional |
| 004260b0 | BSP_NativeString_FromInt | analyzed; behavior reproduced inline |
| 00435c40 | BSP_NativeString_EqualsInsensitive | analyzed; behavior reproduced inline |
| 0073bae0 | BSP_FontSystem_LoadDefinitions | analyzed for its locale input only; exported |

## Open questions

1. How the language *name* from `008f7db0` becomes the integer index at settings `+4`.
2. What the `.lanx` 16-bit pairs mean. The shipped sidecar is empty.
3. What consumes the registered table names at manager `+8`. Only emptiness is read here.
4. Whether the numbered probes `lockit/<lang>.lan0`.. are a patch mechanism. None shipped.
5. The `#` substitution pass and the `^` prefix in `00a9f4b0`.
6. The `Content file name for %s` and `<SRCH><` strings the earlier map noted as reachable
   from `00aa09d0` were not encountered on any path walked here.
