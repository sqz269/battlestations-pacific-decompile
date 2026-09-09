# Font DAT decoding and glyph membership

`include/bsp/font_data.hpp` and `src/font_data.cpp` implement the DAT population
fragment of00ad4c30 and semantic glyph membership/byte acceptance. The project
and program were reverified as `bsp` / `/battlestationspacific.exe` before this
implementation. Native byte hashes and assembly evidence are retained in
FONT_GLYPH_SOURCE, STREAM_SCALAR_READERS and TEXT_GLYPH_ACCEPTANCE.

## Data and behavior

`decode_font_data_00ad4c30_fragment` reads from the current MemoryStream position:
DWORD record count, word height, then24 bytes per record (word key, four floats,
three metric words). It calls the recovered null-count-pointer scalar reader
interfaces instead of decoding an independent byte buffer. Trailing bytes are
left unread, as the native count loop does not require EOF.

Header height is sign-extended from16 bits. Two scaled glyph metrics are
zero-extended; the first metric is copied unchanged. A private inline-assembly
helper preserves CVTSI2SS -> MULSS -> CVTTSS2SI and low16-bit storage. It neither
uses C++ signed overflow nor silently clamps scale/conversion results. The
caller's SSE environment remains in effect; exceptional/unmasked FP behavior
has not been independently validated.

All four float fields are read through00be4360's scalar projection and assigned
in order. No speculative names are attached: `fields_00_0c`, `field_10`,
`scaled_field_12`, `scaled_field_14` refer to original payload offsets. Native
padding+16h/+17h and texture/resource fields+18h/+1Ch are not exposed or invented.
This is not a32-byte ABI-compatible glyph structure.

Every record is read and scaled, even if its key duplicates an earlier one.
`std::map<uint16_t,FontGlyphData>::emplace` retains the first key's payload,
matching the original membership-check/insert-or-free branch. `source_record_count`
retains the header count separately from the number of unique keys. The map is
a typed ownership and ordering substitution, not the original checked-iterator
tree or allocator. It establishes equivalent valid-key membership without
claiming native node layouts, allocation counts or balancing behavior.

The decoder requires key0091h because the native post-record path dereferences
that glyph to form its fallback payload. It does not construct native special
space/newline/fallback render payloads, load textures, publish a font registry,
or own the original font object. Missing0091h returns an explicit host error
instead of executing the native invalid-iterator path.

`font_has_glyph_00ad4500` queries unsigned16 key membership. Acceptance wrapper
`font_accepts_text_byte_00ab6d00` always accepts space20h; otherwise it maps bytes
00..7F to0000..007F and80..FF toFF80..FFFF before lookup, preserving the native
signed-byte extension. It does not accept arbitrary high bytes using a host
font, change codepages, uppercase keys or apply the unreachable apparent
blacklist in00ab6d30. Remapping and actual text mutation remain separate.

## Explicit input and ownership boundaries

The host rejects missing/uninitialized backing, fewer than6 remaining header
bytes, or a record extent exceeding remaining initialized bytes. Native scalar
readers themselves permit short reads with null-argument-derived zero fill;
this decoder deliberately supports the complete-input fragment only. The
`uint64_t` extent calculation avoids wrapping count*24 into a smaller bound.
No arbitrary record-count cap is introduced beyond the available input extent.

Output is built in a temporary and remains unchanged on reported rejection.
Stream cursor is not rolled back: incomplete-header rejection occurs before
reads, extent rejection occurs after the header, and missing-fallback rejection
occurs after all records. Allocation exceptions propagate from the owning map;
native allocator failure and SEH behavior are not reproduced. The decoder does
not synchronize concurrent stream mutation.

## Integration and validation boundary

The existing D3D9 probe reads `Fonts/arial18.dat` through PhysicalFile and
MemoryStream, closes the file, then decodes all 4974 bytes. It checks 207 records
and unique keys, header height26 scaled to13 at explicit scale0.5, the four
binary-exact float fields for glyph A and its12-to6 metric scaling. Acceptance
checks space/A and the distinction between present0091 and absentFF91: byte91
is rejected through native signed-byte conversion. A scalar short-read/EOF
check follows at the end of the same stream. The fixed asset SHA256 is recorded
in FONT_GLYPH_SOURCE; no original font asset was added to the repository.

The Win32 build, both existing CTests and full installed-asset D3D9 probe pass;
see `reports/font_texture_reset_probe.txt`. No new test target was added.
Descriptive Ghidra names/comments were applied with previous values preserved,
the project saved and exports refreshed. This establishes DAT decoding and
membership against installed input, not rendered font metrics, the Lua font
registry, full owner lifetime, arbitrary duplicate/exceptional input or gameplay.
