# Material texture slots

`MaterialTextureSlots` and `set_material_texture_00b189f0` provide the bounded
texture-owner portion of a material. Its `textures()` vector feeds the existing
`bind_material_textures_00b43470` source-0 binding interface directly. This does
not reconstruct the remaining material fields, shaders, or material lifecycle.

## Capacity and ABI evidence

Before the live analysis batch, project `bsp` and program
`/battlestationspacific.exe`, Win32 x86 little-endian, base `00400000`, were
verified. Full raw byte bodies were compared equal to the installed executable:

| Function | Range (inclusive) | Bytes | SHA-256 |
|---|---|---:|---|
| Constructor | `00b18900..00b189e4` | 229 | `6768407d2713726def1fc414fc244ec076fa5369c1289686bed59a2821b722b0` |
| Texture setter | `00b189f0..00b18a3f` | 80 | `ddcd3b96b4cce2f22b205119a4e9cf9a648c23a228bf1bb608373d76cd2b9116` |

Constructor `00b18900` takes ECX material and one stack argument, returns this
in EAX and uses `RET4`. It writes zero to signed16 count `+34` at `00b1893c`,
then explicitly zeros nine DWORDs at `+10,+14,+18,+1C,+20,+24,+28,+2C,+30`.
The last store is at `00b1895f`; a separate subobject begins at `+38` through
`00b17840`. Nine slots are established by these constructor writes and the
following count field, not inferred from D3D sampler limits. The constructor
also initializes other material state and retains its argument at `+7C`; none
of those fields are projected by this narrow host type.

Setter `00b189f0` takes ECX material, unsigned slot at stack `+4`, resource
pointer at `+8`, and returns with `RET8` at `00b18a3d`. No useful return value
is established; the host bool reports its added bounds check.

The setter's complete bytes are:

```text
0fbf41348b5424043bd072078d4201668941348b442408568b7491103bf0742c85c089449110740a83c00450ff151c22ce0085f674168d4e0451ff152022ce0085c075088b168b028bceffd05ec20800
```

## Mutation and ownership behavior

Native `MOVSX` reads count `+34`, then compares the unsigned slot against that
sign-extended value using `CMP; JC`. A slot at or above the count raises it to
low16(slot+1), before pointer identity is checked at `00b18a0c`. Thus setting
a previously unused null slot still advances the count; clearing a populated
slot never lowers it. Native has no bounds check, so malformed indexes can
overwrite unrelated fields. The host rejects indexes 9 and above unchanged,
and represents only valid initialized counts 0..9 by vector length. Corrupted
negative native counts and integer-overflow writes are outside this interface.

When different, native stores the new pointer at `+10+4*slot`, increments its
intrusive reference at `+4` through `00ce221c`, then decrements the old reference
through `00ce2220`. An old count of zero invokes virtual function zero. Equal
pointers skip those operations after the high-water update.

The host uses existing `shared_ptr<LogicalTexture>` ownership instead of
inventing an intrusive layout. It snapshots the source before growing the
vector because callers may pass a reference to another slot in that vector.
This adds a temporary shared-owner retain even on the identity path, and it
occurs before the count update; it is an explicit host ownership difference.
After growth, raw pointer identity is compared with `.get()`. On a change,
swap publishes the retained new owner before the old owner is released by the
temporary's destructor. Consequently an old deleter observes the new slot and
updated count. LogicalTexture objects must have consistent shared ownership;
independent control blocks for the same raw pointer are not supported.

The pointer inside `LogicalTexture` is still a borrowed COM texture pointer,
as required by the existing D3D9 interfaces. Retaining its shared owner does
not call COM AddRef or establish font atlas lifetime by itself. Font-level
resource ownership must continue to keep the actual COM object alive.

The vector exposes only the active high-water prefix, including null holes,
and is read-only to callers. Native starts with all nine physical slots zero;
the host creates these null entries lazily as the count increases. Allocation
failure may throw before the requested update, unlike the fixed native array.

## Integration boundary

The font caller `00aba270` sets glyph resource aliases `+18` and `+1C` into
material slots 0 and 1 on the first qualifying glyph. See
`FONT_MATERIAL_BINDINGS.md` for the native alias/ownership boundary. This type
allows those retained logical texture owners to enter the existing sampler
binding path without asserting that `.mshd` loading or font draw submission is
reconstructed.

Source and assembly inspection completed. No tests, builds, Ghidra mutations,
or shared integration edits were performed in this bounded change.

Parent integration now compiles this type into bsp_core and uses slots0/1 in
the existing installed-font draw. Both CTests and the full D3D9 probe pass;
the compiled bilinear source samples slot0, while both logical slots remain
retained. Range/alias/identity edge cases remain source/assembly-backed rather
than a new test suite. See `FONT_MATERIAL_DRAW.md` for validation limits.
