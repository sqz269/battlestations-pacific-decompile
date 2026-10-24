# Native GUI Font storage, 2026-10-24

`GameNativeFontApplication::enable_native_font_images()` selects an optional
image-shaped Font before descriptor loading. It defaults to the established
typed arm. The integrating GUI switch selects this capability before font
startup; this packet adds no switch or replacement publication.

The selected arm owns one actual `0xAC` Font allocation, real native strings,
one native glyph tree and its separately allocated glyph payloads. DAT records
populate that tree directly. There is no `FontData` allocation or second live
glyph/metric map in this arm. `data()` rejects an image Font. The existing
stable descriptor remains custody metadata for the existing Text companion;
its name is not consulted by the selected native Font lookup. ON additionally
owns an actual `0x10` registry and `0x0C` circular list nodes. Their values are
the same Font BASE pointers, and `00AC3570` selection reads each Font's current
native name at `+0C/+10`. OFF retains the existing typed registry path.

## Evidence and storage

Analysis used configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; live `ghidra proto 00ad55c0` and `ghidra ensure
--status` confirmed the program. Existing exports, raw providers and ledgers
were inspected before this change. No Ghidra name/prototype was replaced.
`ghidra_annotate.py --apply --addresses 00ad4c30` appended the bounded Source
evidence under the write lock, preserved prior comments and saved the project.
Prior annotation values are recorded in worker
`local/ghidra-annotations-20261024T085758Z.json`; the shared `00AD4C30` export
was refreshed afterward. A narrow reconstruction fragment records this wiring
without adding game-body or stock STL reconstruction credit.

| Original body | Native interface and relevant evidence | Source composition |
| --- | --- | --- |
| `00AC3690` / `00AC3400` | ECX registry, EAX same registry, plain RET. `D5CA84` at `+00`, sentinel at `+08`, count at `+0C`. Sentinel is `0Ch`, self-linked next/previous, payload untouched. | Optional actual registry/list storage, constructed before Font production; its `+04` has an admitted zero preimage. |
| `00AC3910` / `00AC3490` | Construct AC-byte Font, allocate node next/previous/Font, perform `00AC37F0` count guard, then link node before `00AD4C30`. | Same raw Font enters list before GFX. Existing evaluated-Lua descriptor records retain custody only. `AC3BB5/AC3BCC` link ordering checked in assembly. |
| `00AC3570` | ECX registry; name header stack, `RET4`; EAX borrowed Font. Traverse actual list; equal current lengths then current CRT case-insensitive comparison; first match. | Active image profile never selects through descriptor names. Native Font identity resolves to its sole companion after native-name selection. |
| `00AD55C0` | ECX Font; name, scale bits, alpha bits, uppercase word on stack; EAX same Font; `RET10h`. Tree head/count `+04/+08`, name `+0C`, height `+14`, scale/alpha `+18/+1C`, five strings `+20..+40`, uppercase byte `+48`. No Font vtable. | Reuses `construct_native_font_storage_00ad55c0` and the same `GameNativeStringProcess`. |
| `00AD4C30` | ECX Font; root, DAT, GFX, alpha, language string headers; `RET14h`. DAT count/height followed by 24-byte records. | Same existing GFX/alpha renderer-cache producer and VFS DAT open; direct scalar DAT reads into native storage. |
| `00AD4540` / `00AD4AA0` | Font/tree BASE; unsigned WORD lookup and unique insertion. Head `+04`; links `+00/+04/+08`, key `+0C`, payload `+10`, color/nil bytes `+14/+15`; node size `18h`. | Reuses the existing raw lookup, unique driver and native rotations/allocation. No replacement tree algorithm. |
| `00AD4480` / `00AD4500` | Select glyph / exact unsigned-WORD tree membership. `20h`/LF select embedded `+4C`, CR `+8C`, absent ordinary key selects copied `0091h` at `+6C`. | `glyph()` returns a reference to the same embedded/native payload; membership uses the real tree. |
| `00AD53A0` | ECX Font, plain RET. Free payloads, release/clear GFX `+64`, release/clear alpha `+68`, destroy strings `+40,+38,+30,+28,+20,+0C`, erase nodes and sentinel. | Same order over the owned image, using existing raw string/tree/actual-resource lifetime providers, then frees the enclosing Source allocation. |

The five persistent strings are DAT `+20`, alpha `+28`, GFX `+30`, root `+38`
and language `+40`. Embedded payload offsets are statically asserted at
`+4C/+6C/+8C`; payload size is `20h`, with borrowed actual image slots
`+18/+1C`. Only the space record's `font+64/+68` pair owns the two transferred
image references. Aliasing images still require two references. There are no
new texture wrappers or per-glyph retains.

Assembly `00AD4FCD..00AD5017` confirms unsigned metric conversion with
`CVTSI2SS`, `MULSS`, `CVTTSS2SI`; the image decoder uses those same operations.
Height uses a signed input WORD; native quad height remains the current
unsigned low WORD. `0091h` is copied as all eight DWORDs, not float arithmetic.
The actual DAT stream is released before completing embedded glyphs and
transferring the image pair, matching `00AD5079` before `00AD508F`.
Native saved path strings are written after GFX and before alpha. The list
nodes and `0x10` header use the same existing
`singleton_lifetime_allocate/free` Source CRT boundary as the Font allocations,
qualified bindings for Original `00BF681B/00BF65AC`; no new pool is introduced.
The list capacity guard retains an allocated pending node on failure and uses
Source `std::length_error`, not Original exception/RTTI/heap ABI.

## Consumer and ownership contract

`native_image()` exposes the owned Font BASE. The current Text companion still
stores a custody descriptor pointer; raw Text `+7C` is not populated or claimed.
`NativeFontResourceOwners::resolve_image()` validates its canonical association.
`glyph()`, `has_glyph()`, `accepts_text_byte()`, `scaled_height_14()`,
`signed_height_14()`, `scale_ratio_18()`, `alpha_texture_scale_1c()` and
`uppercase_only_48()` read the same image fields without a metric cache.
The scalar single-line/wrapped layout and ellipsis overloads share their old
arithmetic core and borrow these records directly, allowing the sprite bridge
to render other pages while this font arm is selected.

The content environment's optional `fonts` pointer selects the same owner for
the live uppercase byte. Existing standalone typed callers default it to null.
Font-name assignment reads alpha scale from the resolved owner. Single-line
geometry reads the owner's low height WORD. The shader service's optional
`fonts` pointer also reads the image's live `+18` scale for point filtering.
Texts and retained callbacks must
retire before fonts; materials retain their existing independent resource refs.

The ON binding is
`GuiTextFontRegistryAccess::borrowed_native_images(font_resources)`, using the
same `fonts.native_fonts()` collection already borrowed for layout/resources.
It reads only the actual registry list and native Font names to choose a Font.
`find_native_font_00ac3570(name_header8)` exposes that same selection for other
ON callers; descriptor pointers may locate custody by identity afterward.
The existing sprite bridge keeps calling
`FontRegistryStartup::find_font_00ac3570`. After complete image startup, that
method delegates to this same raw-list lookup through a read-only eight-byte
input header, then locates its custody row only by descriptor pointer identity.
The OFF name-vector loop remains unchanged. Retirement clears the borrowed
lookup association and keeps the image arm selected, so a later lookup cannot
fall back to descriptor-name authority.
`native_registry_image()` identifies the completed lookup authority for factory
admission. The borrowed profile does not create a substitute `F8BF44` cell or
claim the `007371D0` lazy getter, singleton-manager registration or native locks.

ON retirement walks the native list and retires each same Font exactly once,
including its existing combined contents/outer-free Source operation, then
frees the nodes and sentinel, stores base `CE3818` identity and frees the Source
registry storage. Companion/vector destruction performs no second Font/image
release. The header/list remains retained on an entered failed startup; it is
not published to Text until every Font load completes.

The operation stores its incomplete Source owner before raw constructor effects
and retains a pending payload if insertion throws. Failed operations follow
the application's established process-retention rule. There is no partial-font
destructor replay, language reload or recovery API.

## Validation and limits

`scripts/build.ps1` passed with MSVC Win32 Release, including `bsp_core.lib`,
the image-layout assertions and `bsp_game.exe`. Both existing CTest checks
passed: `reconstructed_math` (0.35 s) and `tool_tests` (13.76 s), total 14.15 s.
The durable worker log is `local/font-image-build.log` (ignored). The initial
full build exposed a string-adapter pointer-type mismatch; the final build
compares the underlying three process cells, as verified in VFS/singleton
source, and passed. No new tests or game runs were added by this packet.
The integrating lane owns the OFF/ON USN04 and visual Text evidence.

The active native registry extension also passed `./scripts/build.ps1` in the
worker checkout, including `bsp_game.exe` and both existing CTest checks
(`reconstructed_math` 0.30 s, `tool_tests` 10.85 s, total 11.19 s). The captured
private log is `local/font-registry-build.log`. No additional tests or game
runs were added for the extension.
`00AC3570/00AC3910` evidence comments were saved to the verified BSP Ghidra
project and their exports refreshed; prior annotations are recorded in
`local/ghidra-annotations-20261024T110224Z.json`.

This is an owned source composition with genuine image layout and storage.
It does not claim the original calling convention, private CRT heap, register
or FH3/SEH/fault ABI, identical partial-failure timing, pool allocator binary
identity, native registry singleton/getter ABI, reload `00AD51D0`, or game/visual proof.
Raw Font `+00`, `+16..17` and `+49..4B` begin with an admitted zero preimage; DAT extent and
required-fallback checks are explicit Source input-domain guards. GFX/alpha
production still occurs through the established application sequence; this is
not an assertion of whole-loader instruction/callback scheduling equivalence.
