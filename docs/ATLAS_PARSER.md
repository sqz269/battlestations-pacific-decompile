# Native ATS parser evidence

Read-only follow-up to `ASSET_ENTRY.md`, 2026-09-09 UTC. The exporter Client
verified project `bsp`, `/battlestationspacific.exe`, x86 LE32 and base 00400000
before each batch. Existing functions sufficed; no analysis creation, annotations,
naming ledger edits, save, source implementation or test additions were made.
Raw pseudocode and assembly are in ignored `exports/bsp/functions/<address>/`.

## Native route and checked ABI

All descriptive names below are proposals, not recovered symbols. Ghidra
pseudocode loses stack arguments in several functions; the ABI column follows
assembly, including caller register setup and callee returns.

| Address | Proposed name | Original interface |
|---|---|---|
| 00aeeaf0 | BSP_TextureAtlas_ParseBuffer | ECX manager; stack buffer pointer; RET 4; AL status |
| 00aee620 | BSP_TextureAtlasItem_ParseParams | ECX item; stack buffer pointer; RET 4; AL status |
| 00aee4d0 | BSP_TextureAtlasItem_Ctor | ECX item; stack item-name string, texture pointer, descriptor-filename string; RET 0c; EAX item |
| 00af5740 | BSP_TextBuffer_ReadNormalizedLine | ECX buffer; stack output string object; RET 4; AL status |
| 00af55f0 | BSP_TextBuffer_Rewind | ECX buffer; no stack args; clears +4 and +18 |
| 00aee3c0 | BSP_AtlasLine_GetToken | ECX line string object; stack output token object and zero-based index; RET 8; EAX output |
| 00aee340 | BSP_AtlasToken_AssignPrintablePrefix | ECX token object; stack char pointer; RET 4; EAX copied count |

`00aeeaf0` rewinds the buffer, reads its first line and compares token zero with
`TextureAtlas` case-insensitively. On success it extracts token one, constructs a
path using the descriptor's directory, and calls renderer `DAT_00f8d394` vtable
+0x64 with (path-string, 0). The result is appended to manager's texture array
(base+0x10, count+0x14, capacity+0x18) **before parsing the body**. Full helper
path-normalization rules remain outside this bounded trace.

It scans lines for an exact `{`, then loops until exact `}` or EOF. On a
`TextureItem` first token, it allocates 0x30 bytes, constructs an item from token
one and the texture/descriptor, calls `00aee620`, and appends to the manager item
array (base+4, count+8, capacity+0x0c). Unknown outer lines are ignored. The item
parser scans for its own exact `{`, reads until exact `}` or EOF, and accepts
`Param` plus `U1`, `V1`, `U2`, or `V2` case-insensitively, converting token two
with CRT `atof` and storing the result to float32. Unknown parameter names and
other lines are ignored. Repeated known fields overwrite earlier values.

## Line and token rules

`00af5740` returns false only when the buffer data pointer is null or cursor
has reached extent. It scans bytes until LF or extent, skips leading bytes
whose **signed byte value** is <= 0x20, then copies only bytes with signed value
>= 0x20 into global scratch `00f8c2c8`. It NUL-terminates the scratch result,
increments the cursor past the line boundary (also on the final unterminated
line), and copies into the output string. Thus indentation tabs/CR disappear,
embedded tabs disappear rather than becoming separators, spaces after content
survive, and high-bit bytes are discarded. No scratch length bound appears in
the loop; its actual capacity was not determined. This is not reentrant.

`00aee3c0` selects space-separated tokens; multiple spaces are skipped together.
`00aee340` copies the selected run of printable nonspace ASCII (0x21..0x7e).
There is no quotation/escape interpretation in this chain. Missing-token handling
can produce a null token object; later stricmp/atof callers do not establish a
safe malformed-input contract. Braces are compared against the entire normalized
line, so a trailing space or same-line body does not count as a brace delimiter.
The first descriptor line must contain TextureAtlas: an initial blank/comment
line is not skipped. Unknown lines inside bodies may be ignored, but that is
not a general comment grammar.

The selected `menu_dxt1_2.ats` uses the supported shape: declaration line,
separate opening brace, seven TextureItem declaration/block pairs, four Param
lines in each item, and a final closing brace. Its printable paths and numeric
fields avoid token ambiguity.

## Native item layout and packed UVs

| Offset | Observed field |
|---|---|
| +00,+04 | Descriptor-filename string length and pointer |
| +08 | Atlas texture pointer supplied to constructor |
| +0c,+10 | Item-name string length and pointer |
| +14,+18,+1c,+20 | Float32 U1,V1,U2,V2 |
| +24,+26,+28,+2a | Low 16 bits of packed U1,V1,U2,V2 |
| +2c,+2e | Low 16 bits of packed V extent, U extent |

Constructor `00aee4d0` copies the two strings, searches item name for `.`
(constant 00ce3a70), and truncates it at the returned position if present.
Follow-up export and assembly of helper `00467cf0` confirm reverse search
from min(length, limit) minus needle length, stopping before offset zero. Thus
the final dot suffix is removed, except a dot at offset zero. For every selected
item there is one dot and the result is the extensionless path. The constructor sets the texture pointer but **does not initialize the UV
floats or packed fields**. Missing Params cannot be assigned invented defaults
and called native-compatible behavior.

At item completion `00aee620` multiplies each coordinate by 65535, converts to
32-bit integer with x87 rounding mode explicitly set to **truncate toward zero**,
and stores the low 16 bits. It packs `(V2 - V1)` at +2c and `(U2 - U1)` at +2e.
The scale is a double at 00ce5f90, verified live bytes
`00000000e0ffef40` = 65535.0. Assembly 00aee94a loads the double;
00aee95d ORs control word with 0xc00; 00aee96a performs FISTP; 00aee973
stores the low word, then the original control word is restored. The same
pattern repeats for all six outputs. Pseudocode `ROUND` is misleading.

The extent subtraction/multiplication runs on x87 values without an intervening
float32 store, so retain that precision boundary in a later port. For this
fixture all UV inputs and differences are exactly representable binary fractions.
Its first item `(0,0,0.25,0.25)` packs to
`[0,0,16383,16383,16383,16383]`, not 16384. No clamps or range checks are present
in this conversion sequence. Nonfinite/out-of-range FISTP behavior and ambient
x87 exception settings must be scoped explicitly if later exposed to untrusted
inputs. The seven items and predicted packed words are recorded in
`reports/atlas_parser_evidence.json`; these are computed expectations, not native
execution results.

## Failure behavior and implementation boundary

The outer parser returns false for no first line or a non-TextureAtlas first
token. After a recognized header it reaches true at EOF, including premature
EOF while searching for/opening/inside a body. The item parser also returns true
at EOF and packs its fields. There is no mandatory-field validation in the
inspected chain, and the outer parser does not test its return value. Texture
lookup has no visible null-result rejection before array insertion. Allocation
failure can reach item parsing with a null item. Missing numeric tokens and
uninitialized UVs mean arbitrary malformed input has unsafe/undefined behavior;
this is not a native strict-validation parser. Successful return does not prove
well-formed input, complete items or successful texture acquisition. There is no
observed rollback of already appended texture/items.

A bounded next implementation can expose a new C++ interface for normalized line
reading, token extraction and complete four-field item parsing, using a supplied
texture callback and ordinary ownership. Label deliberate input validation and
scratch bounds as adapter safety policy rather than recovered behavior. Before
claiming a native reconstruction, preserve source-function addresses and original
ABIs in comments and resolve the corresponding ownership/path interfaces. A
single installed seven-item descriptor comparison is enough initial coverage;
no broad test framework is justified by this handoff. Native differential
execution, build validation, texture loading and visual comparison are pending.

Integration follow-up: the seven proposed names and evidence comments were saved
in Ghidra after review. The bounded C++ implementation is now present as described below.


## C++ integration

`include/bsp/texture_atlas.hpp` and `src/texture_atlas.cpp` implement
`parse_texture_atlas_00aeeaf0(text, descriptor_path, lookup)`. The supplied
`TextureAtlasLookup` receives the joined texture path and native flags zero,
and returns a borrowed `void*` handle. The result exposes a status, explanatory
failure detail, texture path/handle and owning item records. Each item exposes
extensionless name, descriptor path, borrowed texture, four float32 UVs and six
packed words. These are new records, not original binary layouts or ownership.

The implementation carries semantic portions of seven investigated functions:
outer parsing, item parsing/construction, normalized line reading/rewind and
token selection/copy. It does not reconstruct the engine allocator, string ABI,
manager arrays, renderer texture service or file service. It retains native
lookup-before-body ordering, null texture acceptance, earlier completed items on
failure, unknown-line/field ignoring, repeated-field overwrites and success at
EOF after complete fields. First-line mismatch has a distinct native rejection
status. Missing names/numeric tokens/UV fields and unsupported numeric conversion
ranges return `unsupported_malformed`; these are explicit adapter safety policy,
not recovered native validation or guessed defaults. Dynamic scratch storage
replaces the original global scratch. Numeric prefixes are accepted as by atof,
but tokens with no numeric conversion are rejected by adapter policy.

Relative texture path joining and slash normalization are an adapter; full
native path helper semantics remain unverified. Numeric parsing uses the host
CRT locale, as native atof does; arbitrary locale/x87 mode parity is not claimed.
The double arithmetic retains the absence of float32 rounding for extent
subtraction and is exact for this fixture's binary fractions. Texture ownership
stays with the caller, including failure results.

Parent integration owns CMake, installed-asset probe, Ghidra comments and the
address ledger. This implementation handoff ran no tests or build; status must
be updated from the integration checks before claiming build/fixture coverage.
