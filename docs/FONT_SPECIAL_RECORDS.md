# Font special scalar records and selection

The FontData decoder now populates the known scalar fields of native embedded
glyph records and provides `select_font_glyph_00ad4480(const FontData&, uint16_t)`.
This advances the earlier FONT_DATA_IMPLEMENTATION boundary, which excluded
special records. Resource pointers, padding and original payload ABI remain
excluded; this is not the complete native font loader or renderer.

## Recovered behavior

After all DAT records and the required0091h check, the decoder produces:

* `space_lf_glyph` (native font+4Ch): zero float/metric fields except horizontal
  advance+12h = signed16(scaled height)/4, truncating toward zero, retaining the
  low16 result bits. This uses the already-scaled stored height, not raw height
  or an independently recomputed float. Height26 gives advance6; negative height
  follows the native sign-bias/SAR behavior rather than arithmetic-shift floor.
* `missing_glyph` (font+6Ch): a snapshot of key0091h's known scalar fields. Its
  four float fields are copied with memcpy because the native REP MOVSD copy
  does not perform floating-point conversion. Metrics retain their raw words.
* `carriage_return_glyph` (font+8Ch): zero known scalar fields.

The selector returns a const reference in this order: space0020h or LF000Ah ->
space/LF record; CR000Dh -> carriage-return record; exact unsigned16 map entry ->
that glyph; otherwise -> missing record. Special keys take precedence even if
the DAT contains entries for them. There is no uppercase or acceptance filter
inside selection. Acceptance and rendering fallback are distinct native routes.

The missing record is copied once, matching native post-load initialization.
Mutating the public glyph map later does not implicitly refresh that snapshot.
A default or manually edited FontData is not evidence of a native initialized
font; normal callers should obtain it through the successful decoder.

## Layout evidence and limits

Field comments now record immediate consumer evidence without changing existing
field names or pretending to reproduce the native struct: floats+0..+Ch supply
UV edges, raw word+10h is interpreted as a signed horizontal offset by quad
writer00ab98f0, unsigned+12h advances text layout, and unsigned+14h controls quad
width. Detailed consumer and uppercase evidence is in FONT_GLYPH_FALLBACK.

The native special records also include two resource values at+18h/+1Ch, and
the0091h copy includes native padding+16h/+17h. This implementation supplies
neither invented resources nor a zeroed stand-in for unknown native padding.
Texture bindings, uppercase conversion with override tables, glyph quad writing,
native checked iterators and resource lifetime remain independent dependencies.

## Verification ownership

Before editing, the agent verified project `bsp`, program
`/battlestationspacific.exe`, and re-read the two relevant complete/fragment byte
ranges. Their SHA256 values match the earlier disk/live evidence:

| Range | SHA256 |
| --- | --- |
| 00ad4480..00ad44fd | fe786a41f924b7a9ef37af1db008c658c9d494a19f10a90d2adf400579efd1ce |
| 00ad508f..00ad514d | 5c9c1d581bb0e8ea33ef4125dcb3812433750cf705c2ba02be77ad34dc7b62c8 |

The installed-font probe now checks shared space/LF selection and advance5 from
Arial16's scaled height23, CR's zero advance, exact A payload identity, and a
distinct copied missing-glyph record matching0091's known fields. The Win32
build, both CTests and full D3D9 probe pass; no new test target was added. See
`reports/query_draw_font_fallback_probe.txt` and fresh hashes in
`reports/query_use_font_fallback_audit.json`. Parent integration named/commented
the selector and extended the loader evidence, preserved previous annotations,
saved the project and refreshed exports. Scalar selection checks do not establish
rendered text placement, full resource ownership or gameplay parity.
