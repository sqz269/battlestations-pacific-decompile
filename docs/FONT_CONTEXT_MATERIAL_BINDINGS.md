# Font context shader cache and material parameter bindings

Addresses: `00ab8ce0`, `00ab8c30`, `00ab8530`, `00aba8d0`, `00ab9650`, `00abb2c0`, `00abb630`, `00ab7b60`, `00ab72d0`, `00aa9390`, `00aa9f10`, `00b19210`, `00b17e10`, `00b18aa0`, `00b18ac0`, `00b18b00`, `00b18b20`, `00b44d60`, `00b18790`, `00b185a0`, `00b17af0`, `00b18900`, `00b17840`, `00b179f0`, `00b407a0`, `00b18d60`, `00b5b820`, `00b5b870`, `00b5b880`, `00b5bc60`, `00b3a750`, `00b42350`

The font context registers **borrowed live pointers**, not copied constant
values. Its effect binder resolves these names against the compiled, non-system
VS/PS metadata for fourteen pass selectors. The existing material packing
fragment subsequently copies the current source words into the selected
registers. This resolves the missing bridge between font parameter registration
and real compiled-register locations, including the repeated shadow blend name.

The resulting generic table is implemented in
[material_parameter_bindings.hpp](../include/bsp/material_parameter_bindings.hpp)
and [material_parameter_bindings.cpp](../src/material_parameter_bindings.cpp).
It supplies named registration and live-source packing to the current font
draw. Native shader compilation/cache ownership, scene traversal and a full
font-material state owner remain separate boundaries.

## Target, evidence and original ABIs

Each fresh export/read batch used `Client` from `tools/ghidra_export.py` with
`config/target.json` and verified project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base `00400000`. The primary authorized
the exact renderer/base helpers outside the font lane. Thirty-two complete
function bodies were exported, compared against the original PE and decoded
through their complete recorded spans. Raw evidence is in ignored
`exports/bsp/parallel_font_context_material/`; exact spans, hashes, ABI notes
and annotation proposals are in
[font_context_material_bindings_audit.json](../reports/font_context_material_bindings_audit.json).
No Ghidra or shared metadata mutation occurred in this packet.

| Routine | Original interface |
|---|---|
| `00ab8ce0` | ECX text context; no stack arguments; AL reports a selection attempt, `RET`. |
| `00ab8c30` | ECX text context; font-name wrapper stack; `RET4`. |
| `00aba8d0` | ECX text context; transformed UTF-16 wrapper stack; `RET4`. |
| `00aa9390` | ECX base context; type DWORD stack; EAX context, `RET4`. Text construction passes 3. |
| `00aa9f10` | ECX context; material stack; `RET4`. Pseudocode incorrectly omits that material argument. |
| `00b19210` | ECX material; effect pointer stack; `RET4`. Retain/release effect, mark nonnull effect `+B4`, then clear parameters even when the pointer is unchanged. |
| `00b17e10` | ECX material; name wrapper, source pointer, word count, matrix byte on stack; EAX parameter record or null, `RET10h`. |
| `00b18aa0` / `00b18b00` / `00b18b20` | ECX material; name wrapper/source pointer; `RET8`. Forward word counts 4/2/1 and matrix byte zero. |
| `00b18ac0` | ECX material; name wrapper/source pointer/vector count; `RET0Ch`. Forward `4*count` words and matrix byte zero. |
| `00b44d60` | ECX effect; material, existing record, name wrapper, source pointer, word count, matrix byte; EAX resulting record, `RET18h`. Concrete effect vtable `00d61a00+10h`. |
| `00b5b820` / `00b5b870` / `00b5b880` | ECX metadata record; return `record+14h` / DWORD `+0` / DWORD `+4` in EAX; `RET`. |
| `00b5bc60` | ECX metadata record; register, register count, name, semantic, rows, columns, elements; EAX record, `RET1Ch`. |
| `00b3a750` | ECX stage metadata; register/count/name/rows/columns/elements; `RET18h`; supplies material semantic 55 to `00b5bc60`. |
| `00b42350` | ECX selected pass; entry/override stack; `RET8`. Existing packing fragment consumes registered sources using entry selector. |

The full `00b44d60..00b44f9d` span is 574 bytes, SHA-256
`76f359bd844ec3ef2bdd4025d485540e9d89b0185e3e072c5e6cd808730e93aa`.
Inherited binding `00aa9f10..00aaa0e8` is 473 bytes, SHA-256
`4c909a4ebe35613f70e6b82c1199a036c15f88eb178363cdb6d9fc1862e50b71`.
All original code spans have disk backing. Data `00f8be54` is instead PE
zero-fill: its four saved-image zero bytes were compared to that mapped-image
default, not falsely claimed as four bytes read from the file. Mutable data
values below are saved-image defaults, not observations of a running game.

## Font shader selection, invalidation and defaults

`00ab8530` ensures main/shadow sections and assigns `guidefault.mshd` material
when creating a section. It runs before text comparison. The main drawable
must already exist. It does not establish the final font effect or initialize
the missing generic scene graph; ownership details remain in
[FONT_CONTEXT_OWNERSHIP.md](FONT_CONTEXT_OWNERSHIP.md).

`00ab8ce0` returns AL zero without loading when context `+1EC` is nonnull.
Otherwise a nonempty `+1C0` shader override wins. With an empty override,
ordered comparisons of `CVTSI2SS(global0109cf04->DWORD+28)` against 720 and
font `+18` against 1 select `GuiFont.mshd`; every other result selects
`GuiFontBilinear.mshd`. Renderer virtual `+48` returns the retained effect
reference stored directly at `+1EC`. AL becomes one even if that result is
null. The existing name selector and `.mshd` to `.shfx` rewrite cover these
selection/name portions; see [FONT_SHADER_RESOLUTION.md](FONT_SHADER_RESOLUTION.md).

On an unequal font name, `00ab8c30` stores the name, resolves the font,
stores its pointer at `+108`, and copies font `+1C` to context alpha scale
`+1D4` (or 1 for a null lookup). It then releases/clears cached effect `+1EC`.
It does not rebuild text. The same-text shortcut in `00aba8d0` precedes
`00ab8ce0`, so invalidating the shader and submitting equal text does not
force reselection. Explicit rebuild or changed nonempty text reaches it.

Within the inspected property loader `00abb630`, the `ShaderName` property
writes the string at `+1C0` directly, after the font-name setter call. No
additional cached-shader clear accompanies that property assignment. This is
not proof about all possible property mutation paths; an implementation must
not invent universal automatic invalidation on every scalar/name change.
The copy constructor `00abb2c0` copies the override/font name but explicitly
starts `+1EC` at null before rebuilding. The ordinary text constructor
`00ab9650` also starts both strings and the cached pointer empty/null.

Fresh base construction proves these context defaults:

| Context value | Constructor value |
|---|---|
| `+94` overbright | 0 |
| `+A4..+B0` low color | `(0,0,0,1)` |
| `+B4..+C0` high color | `(1,1,1,1)` |
| `+C4` blend | 0 |

Neither inspected base/text constructor initializes the later alpha-scale
`+1D4` or pair `+1DC/+1E0`; font resolution and new shader selection establish
those values. No unconditional initial pair `(0,1)` is inferred from the
constructors. Text shadow color is copied from mutable `00e12fd8..00e12fe4`,
whose saved-image default is `(0,0,0,0.75)`.

## Exact registered values and order

For changed nonempty text, newly attempted selection causes the main material
shader assignment and this registration sequence:

| Name | Count in DWORDs | Borrowed source |
|---|---:|---|
| `cOverbrightAlphatex` | 2 | context `+1DC/+1E0`, written by separate x87 `FLD/FSTP` copies from `+94/+1D4` at `00abaa03..00abaa24` |
| `cLowColor` | 4 | context `+A4` |
| `cHighColor` | 4 | context `+B4` |
| `cBlendFactor` | 1 | context `+C4` |
| inherited clipping | varies | `00aa9f10(context, mainMaterial)` |

The pair is copied in this selection block; the audit does not turn it into
an unconditional per-draw recomputation from `+94/+1D4`. Subsequent packing
reads the registered pair itself. Low/high/blend pointers address their
context fields directly. All source owners must outlive their registrations.

For shadow on the same selection-attempt flag, the sequence is shader
assignment, inherited clipping, `cBlendFactor` from `00f8be54`, then pair,
low color, high color and **another** `cBlendFactor` from context `+C4`.
The binder resolves the duplicate name to the existing record and replaces
its source pointer, so the final context pointer wins when the name is present
in a compiled pass. No extra blend multiplication or duplicate record is implied.

Inherited `00aa9f10` walks the supplied context then its `+70` parent chain,
calling virtual `+5C` until it returns type 16. It writes context `+E8` to
1 when found and 0 otherwise, then always registers `cClip` from `+E8`.
Only the active case registers `cClipCenter` (two words from found ancestor
`+EC`), `cClipBorder` (four from ancestor `+F4`) and `cAspectRatio` (one from
mutable global `00e12fc0`, saved-image value about 1.33333337).
When inactive, it does not register these latter names. Their destination
registers are therefore not automatically zeroed or initialized to one.
Resolving an actual clipping ancestor remains a scene-owner responsibility;
a future font adapter can accept explicit resolved sources without fake nodes.

Main diffuse material color has another route. `00b18900` initializes its
embedded block at material `+38` through `00b17840`, whose first four words
are `(1,1,1,1)`. `00b179f0` returns material `+38` regardless of the consumed
stack index. Every changed nonempty font update copies context `+168..+174`
there for the shadow material. `00b42350` then packs this color to system
semantic 47 (`cMatDiffColor`, metadata byte `+37`) independently for PS at
`00b4305d..00b43093` and VS at `00b43096..00b430cc`. This is a system constant,
not one of the named non-system parameter records.

## Name lookup, records and compiled selectors

`00b17e10` returns null immediately for a null material shader. Otherwise it
searches the material's inline pointer table at `+80`, count `+100`. First
equal stored string length followed by CRT `__stricmp` equality wins. Existing
record or null is passed to effect virtual `+10`; the table has room for 32
pointers before its count field. Native does not check that capacity.

`00b44d60` scans fourteen effect pass pointers `+C8..+FC` in order. Null passes
are skipped. For each nonnull pass, VS metadata `+70` and PS metadata `+74`
each contain a non-system vector at `+78/+7C`, record stride `20h`. It searches
each stage in order by the same length/CRT comparison and stops at its first
match, even when that register value is negative. Missing matches yield -1.
Only a selector with at least one signed nonnegative match changes a record.

On the first qualifying selector, a missing record is allocated and appended;
all fourteen VS/PS register words start at -1. For an existing record, those
arrays are **not cleared**. Both new and reused records copy the supplied name
spelling and store the supplied source pointer, count and matrix byte. Assembly
`00b44ebd` jumps to the shared name-copy block `00b44f19..00b44f3a` for an
existing record. Thus case-only re-registration can change stored spelling.
The exact wrapper-address equality check only avoids self-copying its name.

Each qualifying selector writes both returned stage register values, including
-1 for an absent stage. Null/no-match selectors preserve their old entries.
If no selector qualifies at all, the original record is returned unchanged;
no new record is added and source/name/count/matrix fields are not refreshed.
This stale-index behavior matters after compiled pass metadata changes without
a shader assignment. `00b19210` instead clears all records even when assigning
the same shader pointer, so that path starts clean.

| Native parameter record offset | Meaning |
|---|---|
| `+0/+4` | Owned name length/data |
| `+8` | Borrowed source address |
| `+C` | Source DWORD count |
| `+10` | Matrix flag byte |
| `+14..+48` | Fourteen signed VS register indices |
| `+4C..+80` | Fourteen signed PS register indices |

Allocation trampoline `00b18790` selects pool `00f8d3e4` and tail-jumps to
`00b185a0`; it is not a static destructor. The pool uses 88h-byte slots with
its allocator metadata at record `+84`. `00b19210` destroys each owned name
then returns the record to the pool through `00b17af0`. It does not free the
borrowed source words. The raw `00b185a0` body continues after `_free` at
`00b18631` through actual returns `00b186c7/00b186db`; misleading no-return
pseudocode was not used as the cleanup boundary. Correct CRT names and global
no-return annotations remain unchanged.

This classification follows the data flow, not merely the trampoline syntax:
`00b1865b` decrements the free-slot count at chunk `+4500`, `00b1866e` reads
the free-index table at `+4400`, and `00b18676..7c` computes the selected
chunk-plus-index-times-88h slot. Both returns put that slot in EAX. The binder
calls the trampoline at `00b44ec4`, initializes the returned record and its
fourteen-element register arrays, then appends it to material `+80/+100`.
The old generated name `CG_static_dtor_stub_00b18790` is therefore a false
classification; excluding this reviewed address from that tag generator is
supported without changing classifications of other pattern matches.

## Packing and the implemented host table

`00b42350` reads selector `[entry+10]+198`, traverses the material records in
order and uses their signed stage indices. Nonmatrices copy exactly
`word_count*4` bytes; a two-word pair does not pad a float4. Negative indices
skip writes, and unrelated words retain their prior contents. This feeds
shared VS `0108ebf4` and PS `0108dbec` before existing uploads.

Matrix VS shape lookup has a naming trap: native metadata getter `00b5b880`
returns record `+4`, which `00b5bc60` receives as reflected **RegisterCount**.
It does not return reflected `Rows` at `+8`. The existing host
`VertexConstantShape.row_count` therefore receives `register_count`.
Packing still uses the last matching register's count and writes only VS
counts 2/3/4, while PS writes four rows. The existing helper's finite-matrix
and exceptional-x87 limits remain as documented in [MATERIAL_CONSTANTS.md](MATERIAL_CONSTANTS.md).

`MaterialParameterBindings` owns copied selector metadata, a retained shader
identity token, and stable heap records. Its source primitive explicitly
borrows a pointer with `available_words`, native `word_count` and matrix flag.
Changing those live words between serialized pack calls affects the next
pack. Sources must stay at stable readable addresses until replacement,
shader clearing or destruction; the component does not pretend they are owned.
The retained identity must own the actual effect/compiled-shader dependencies.

- `set_shader_00b19210_fragment(identity, selectors, error)` copies metadata
  and always clears registrations, including equal identity. Null identity
  disables registration. Native effect `+B4` signaling is external.
- `replace_selectors(selectors, error)` is an explicit host metadata-refresh
  operation, retaining records and stale indices until registration updates them.
- `register_words_00b17e10_00b44d60` returns `bound`, `no_match` or host
  `unsupported`, plus a stable borrowed record view where applicable. Matching,
  name replacement, partial selector updates and no-match behavior follow the
  verified binder. Record addresses survive table growth and moves.
- `pack_00b423c5` snapshots current selected source words and calls the existing
  pack helper. Source/output overlap is outside this snapshot interface and
  returns `shared_output_buffer`; selector, source and destination guards use
  the other existing pack statuses. Output buffers are not resized or cleared.

Names must be coherent, no embedded NUL and at most `INT32_MAX` bytes. Metadata
lists must fit the native signed `count*32` span. A new bound record must fit
32 slots. Source byte counts must not overflow DWORD arithmetic; nonmatrices
need their requested word count and matrices need sixteen readable words.
Source guards apply only when a compiled selector match would bind it.
Guards and allocation failures leave existing registrations/output buffers
unchanged; these are host restrictions, not native failure emulation.
Allocation exceptions propagate. All access is caller-serialized, and CRT
case mapping uses the host process locale.

## Remaining font integration boundary

The installed bilinear descriptor contains the pair and clipping names, and
uses system material diffuse color. Low/high/blend are absent from its active
compiled constants; no-match registration is valid and must not fabricate
registers. The nonbilinear descriptor declares separate `cOverbrightFactor`
and `cAlphaTextureScale` names. Exact name matching proves that a registered
`cOverbrightAlphatex` pair does not bind those names by alias or shape. That
source/version mismatch remains explicit.

The new table can replace the probe's name-specific material constant writes
using its real compiled metadata and stable fixture parameter sources. It
does not supply world/view matrices, time, visibility, stream decode state,
clipping ancestors or shader compilation. These belong to existing explicit
renderer/scene inputs or the next retained shader-cache implementation. A
future font material owner can use the proved base defaults and explicit
resolved font alpha scale; no such class or scene/callback stubs were added.

The primary integrated actual reflected VS/PS metadata and a retained compiled
shader COM owner into the existing font draw. The coordinated MSVC Win32
`/W4 /WX` build, both existing CTests and the full installed D3D9 probe passed.
For selector 0, the installed bilinear font reported zero VS and five PS
material constants and five registered records. Assigning the same shader
cleared all records; re-registration and a borrowed alpha change to 0.5 reached
its actual compiled register without re-registration. The probe restored alpha
to 1 before drawing. Existing single-glyph and wrapped draws retained 74 and
900 lit pixels respectively, zero pixels outside their expected bounds, and
successful device-state restoration.
The retained COM owner is established by the source's AddRef/Release ownership;
the probe's printed owner label is not an independent lifetime assertion.

The draw still supplies explicit fixed inputs: overbright/alpha `(0,1)`, low
color `(0,0,0,1)`, high color `(1,1,1,1)`, blend and clip zero, clip center
`(0,0)`, clip border `(1,1,1,1)` and aspect ratio 1. Inactive clip values beyond
`cClip` are probe inputs, not newly inferred native defaults. Runtime coverage
uses only selector 0 and actual PS material bindings (no VS material binding),
with nonmatrix source registrations; it does not prove other selector
transitions, dynamic clipping/scene values, matrix registration
behavior, or the complete native shader cache. This packet added no test target
or independent fixture. Native differential behavior, binary ABI compatibility
and original game execution are not claimed.
