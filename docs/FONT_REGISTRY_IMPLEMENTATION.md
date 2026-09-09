# Lua font descriptor registry

`font_registry.hpp/.cpp` provide a stock-Lua5.1.1 adapter for the descriptor
portion of00ac3910 and a typed name lookup for00ac3570. Project `bsp`, program
`/battlestationspacific.exe` was verified before analysis batches. Native source
routing and complete function hashes are recorded in FONT_GLYPH_SOURCE.

## Execution and descriptor fields

The adapter uses the already-linked Lua5.1.1 runtime, opens only its base library,
sets PC=true and supplied X360COMP/optional REGION, installs resolver-backed
DoFile, executes `Scripts\\fundamentals.lua`, then the requested descriptor.
Includes execute in the same state, preserving their globals. Script chunks use
protected calls and explicit host errors rather than native unprotected Lua
panic/SEH behavior. The host supplies requested paths; this is not a font-specific
replacement VFS or a claim about native overlay selection.

Global Fonts is traversed with lua_next, retaining that iteration order in the
registry vector. Native00b67080/00b67190 call the embedded next operation00a683a0;
the loader has no sorting or integer-key gate for Fonts. Each descriptor owns:

| Field | Native source / conversion |
| --- | --- |
| name | Outer key via00b662b0 Lua string conversion, then C-string copy |
| scale_ratio | Exact NUMBER converted to float32; otherwise1 |
| alpha_texture_scale | `alphatexturescale`, same exact NUMBER/default1 rule |
| uppercase_only | Exact BOOLEAN; otherwisefalse |
| data_file | datafiles[1].Data through Lua string conversion |
| gfx_file | datafiles[1].GFX through Lua string conversion |
| alpha_texture | datafiles[1].AlphaTexture exact STRING, otherwise `white.tga` |

Native00b67720 receives integer1 at00ac3bd3,00ac3c7c,00ac3d29. The implementation
repeats that index lookup for each field; later datafiles rows are ignored.
The key strings at00d5ca88/98/9C were read live and confirm AlphaTexture/GFX/Data.
Names, Data and GFX support Lua's numeric coercion; booleans/tables/nil fail the
host's required-value check instead of reaching a native null-string dereference.
Embedded NUL truncates these C-string-derived values. AlphaTexture keeps empty
strings when explicitly supplied; a numeric AlphaTexture uses its default.

The code converts a copy of the outer key so a numeric key's string conversion
cannot invalidate the next lua_next operation. Native saved-state/key-reference
handling is not reproduced. Stock Lua hash traversal can differ from the
embedded executable's runtime, so installed string-key table semantics are
supported without claiming identical ordering across arbitrary patched runtimes.

## Ordered registration and lookup

Successful loading appends descriptors to the supplied registry in traversal
order. It retains prior entries and records successful chunk paths in execution
completion order. Case variants and duplicate names are not consolidated.
The native registry list also appends font objects; lookup00ac3570 returns the
first name with equal length and a zero stricmp comparison.

`find_font_00ac3570` implements the same length gate followed by MSVC `_stricmp`;
empty/equal-length names match directly. This uses the process CRT locale just
as the original comparison does, rather than a new Unicode case-folding rule.
Pointers returned into the vector are borrowed and can be invalidated by later
registry mutations. Native manager/list/font allocation, refcounts and teardown
are not reconstructed by this ownership projection.

## Supported input and integration boundaries

The adapter rejects metatables on the global environment, Fonts table, font
entry, datafiles and datafiles[1]. It uses raw table lookups within this ordinary-
table subset. This avoids treating arbitrary metamethod behavior as established
native parity; scripts may otherwise execute ordinary Lua code before producing
the descriptors. Numeric fields retain native type gating rather than accepting
numeric strings. Nonfinite Lua numbers are not silently clamped.

Parsing builds a temporary copy of the registry. Reported script/shape/value
failure leaves the supplied registry unchanged. Native loading interleaves
registration and font resource construction and can leave earlier state behind;
transactional host failure is an explicit difference. General allocator/OOM,
resolver exceptions, native Lua wrapper lifetime and SEH parity are not claimed.

This adapter evaluates metadata only. It does not load DAT files, create glyph
payloads, apply uppercase text transformations, load texture resources or bind
font objects into GUI context+108h. The primary agent owns local path resolution
and MemoryStream/FontData integration. A descriptor's scale_ratio feeds the DAT
decoder; alpha_texture_scale and uppercase_only remain recovered metadata until
their consumers are implemented.

## Verification ownership

The existing D3D9 probe resolves installed fundamentals and Fonts/Fonts.lua
through PhysicalFile/MemoryStream. It evaluates six descriptors, resolves
`aRiAl16` through native case-insensitive name matching, checks texture defaults
and Viper's uppercase flag, then reads the selected arial18.dat and uses its
Lua scale8/9. All207 glyphs decode; height26 becomes23 and glyph A's12-unit
metrics become10. Unknown-name lookup returns null. Probe path resolution is
explicit host integration, not recovered native mount/alias selection.

The Win32 build, both existing CTests and full installed-asset D3D9 probe passed.
No new test target was added. See `reports/font_registry_texture_policy_probe.txt`
and `reports/font_registry_texture_policy_audit.json`. Parent integration named
and annotated the native loader/lookup, preserved prior values, saved Ghidra and
refreshed exports. These checks establish descriptor evaluation and DAT routing,
not rendered font parity, GUI context ownership, or a runnable game.
