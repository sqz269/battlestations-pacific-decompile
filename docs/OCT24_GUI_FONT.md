# Native GUI Font storage, 2026-10-24

`GameNativeFontApplication::enable_native_font_images()` selects an optional
image-shaped Font before descriptor loading. It defaults to the established
typed arm. The integrating GUI switch selects this capability before font
startup; this packet adds no switch or replacement publication.

The selected arm owns one actual `0xAC` Font allocation, real native strings,
one native glyph tree and its separately allocated glyph payloads. DAT records
populate that tree directly. There is no `FontData` allocation or second live
glyph/metric map in this arm. `data()` rejects an image Font. The existing
stable registry descriptor remains host name-lookup metadata and resolves to
the same `NativeFontResources` owner, not another Font body. The registry list
itself is still the existing C++ startup composition, not a recovered native
registry image.

## Evidence and storage

Analysis used configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; live `ghidra proto 00ad55c0` and `ghidra ensure
--status` confirmed the program. Existing exports, raw providers and ledgers
were inspected before this change. No Ghidra name/prototype was replaced.

| Original body | Native interface and relevant evidence | Source composition |
| --- | --- | --- |
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

## Consumer and ownership contract

`native_image()` exposes the owned Font BASE, suitable for native Text `+7C`.
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

The operation stores its incomplete Source owner before raw constructor effects
and retains a pending payload if insertion throws. Failed operations follow
the application's established process-retention rule. There is no partial-font
destructor replay, language reload or recovery API.

## Validation and limits

Build validation is pending the packet's `scripts/build.ps1` invocation.
No new tests or game runs were added by this packet. The integrating lane owns
the OFF/ON USN04 and visual Text evidence.

This is an owned source composition with genuine image layout and storage.
It does not claim the original calling convention, private CRT heap, register
or FH3/SEH/fault ABI, identical partial-failure timing, pool allocator binary
identity, native FontRegistry image, reload `00AD51D0`, or game/visual proof.
Raw Font `+00` and padding begin with an admitted zero preimage; DAT extent and
required-fallback checks are explicit Source input-domain guards. GFX/alpha
production still occurs through the established application sequence; this is
not an assertion of whole-loader instruction/callback scheduling equivalence.
