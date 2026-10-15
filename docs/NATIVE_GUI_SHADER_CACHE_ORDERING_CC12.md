# GuiFont shader-cache ordering: bounded read-only evidence (CC12)

Packet `cc12_gui_shader_cache_ordering`, Source base `b8395fc39`. This packet
changes no reconstructed Source, lookup/mode policy, records or game process.
Evidence family `72deb3bc1` and its artifacts remain untouched.

The installed physical `shaderfx/shaders.bin` is 2,250,700 bytes, SHA256
`dcc65686e26e0b7b4f4179b45b153aaa353696b0b1b08c32251fbc7c97c0ecf8`.
The format already used by `B35340`/`BE4620` is a DWORD record count, then each
DWORD name length, exact name bytes without NUL, DWORD bytecode size and blob.
Read-only parsing consumes exactly EOF for all 1,628 records, with no duplicate
case-insensitive ASCII names. This is physical-file evidence, not execution of
the native VFS/record loader or proof of its actual selected stream.

| Zero-based row | Name | Bytecode bytes | Version word |
| --- | --- | ---: | --- |
| 700 | guifont0F3.vso | 784 | fffe0300 |
| 701 | guifont0F3.pso | 424 | ffff0300 |
| 702 | guifont0T3.vso | 784 | fffe0300 |
| 703 | guifont0T3.pso | 424 | ffff0300 |
| 704 | guifontbilinear0F3.vso | 740 | fffe0300 |
| 705 | guifontbilinear0F3.pso | 728 | ffff0300 |
| 706 | guifontbilinear0T3.vso | 740 | fffe0300 |
| 707 | guifontbilinear0T3.pso | 728 | ffff0300 |

The current established query producer strips the last extension from the
resolved descriptor name, appends the decimal mode index, policy `T`/`F`, and
fixed generation `3`. The compiler then strips through the last forward slash
and appends `.vso`, followed by `.pso`. The root and mode descriptors remain
real identities; this analysis does not create another descriptor/glyph map.

The direct installed `gui/guifont.shfx` and `gui/guifontbilinear.shfx` each
contain one combiner `{RM_NORMAL,"dummy.shfx"}`. Their actual `dx9_lua.inc`
defines `RM_NORMAL=0`, and the direct dummy file supplies the simple pixel
finish. These files omit `ShadowShader`; the current reader's empty default
leaves the extra `sha` program branch disabled for this direct-file case.
`B45E00` constructs/copies the secondary primary-mode pass and retains its
shader owners; its body does not invoke a new `cached_shader` lookup. These
statements do not exclude a different actual VFS-selected/override descriptor,
provider side effect, failed creator or fallback path in the live probe.

With variants zero, `B46950` performs one program load using the current actual
shadow policy. Variants one instead generates the F/T variants, so its write
route is a different case, not a cached-mode comparison here. No mode change
is proposed or performed.

`B35340` initializes the real owner cursor to zero while loading binary rows.
`B34890` starts its separate loop index at the current owner cursor, compares
current lengths and `_stricmp`, and advances the owner cursor for every skipped
or matching row. A match returns the just-consumed row; a miss reaches count
and returns null. There is no rewind or wrap. Existing `cached_shader` still
dereferences a missing row; this packet does not add a default or soften it.

For the candidate primary sequence GuiFont VS/PS then GuiFontBilinear VS/PS,
all four policy pairs FF, FT, TF and TT succeed in this physical table's
forward order. FF finishes at cursor 706, FT at 708, TF at 706, TT at 708.
That does not require selecting a different policy or consuming the unused
variant. Reversing the two fonts would miss the earlier GuiFont records after
the cursor advanced through Bilinear. This counterexample illustrates the
existing rule; it is not a proposed ordering change or an executed game case.
The installed preload text also places the two names adjacently on lines
80/81, but textual adjacency alone is not proof of Lua `next` iteration order.

The same file has `error0F3.vso/pso` at rows 192/193 and `error0T3.vso/pso`
at 194/195. A cold error fallback after a GuiFont lookup has advanced past
those rows cannot recover them by the existing forward search. An already
resident effect-cache error hit would not perform that binary lookup. This is
a conditional diagnostic limit, not a claim that GUI05 entered fallback.

This static result provides a useful next-failure discriminator: a primary
query with these exact names from the expected cursor range has a real record.
A miss needs its reached query bytes, current owner cursor/count, actual
descriptor resolution/policy and any extra/fallback path, rather than a private
cache or reordered search. The joined query temporary is released before
cached reflection; a freed query buffer in a dump is not current live input.
The retained parent local name remains live until the compiler's finish path,
so it may establish the stem if that memory is present in the dump. No such
runtime value is claimed by this packet.

Six installed inputs and nine current Source files are frozen/hash-pinned under
`local/cc12_gui_shader_cache_ordering/inputs`; detailed rows, candidate cursor
steps and qualifications are in `reports/native_gui_shader_cache_ordering_cc12.json`.
No C++ build or new test is needed for these read-only evidence files. Root's
immutable GUI05 result remains pending. Native cache loading, actual query
identity, shader creation, sampler-guard reach, GuiFont acquisition and draw
remain separate runtime claims.
