# Shader owner texture registration and lookup

Read-only recovery from `bsp.gpr`, `/battlestationspacific.exe`, on 2026-09-09.
The project/program were verified before each export/read batch. Assembly and
pseudocode are in ignored `exports/bsp/owner_textures/`; all eight bodies in the
evidence table below were compared byte-for-byte with the installed executable.
This documents the source-3 route; it does not implement the texture loader or
establish game validation. Descriptive owner names are hypotheses.

## Registration and consumption

`00b3b280` handles a descriptor sampler with source kind 3 by calling `00b18fb0`
with ECX = builder owner at `builder+78h`, stack index = sampler `+20h`, and stack
string reference = sampler `+18h`. It then appends the signed selector
`-1-index` through `00b5f100`, preserving the stage byte at sampler `+0Ch`.
Registration does not load or replace a texture.

`00b18fb0` is ECX owner, stack unsigned index, stack pointer to native eight-byte
string `{length, data}`, `RET 8`. It sign-extends the short at owner `+94h` and
compares it to the index **unsigned**. If index is at least that value, it stores
the low 16 bits of `index+1`. It assigns the name at `owner+3Ch+index*8` unless
the source reference is exactly that destination. Assignment uses `0041dd40`
to resize with preserve flag 1, then copies `destination.length` bytes for a
nonempty source. The resize helper maintains a trailing zero; embedded zero
bytes are copied as ordinary bytes. There is no index/capacity check.

`00b43410` reads the owner from pass `+14h`. A negative selector is decoded
using `-1-selector`, then passed to `00b17d90`. The resulting pointer is passed
to renderer virtual `+130h` for either pixel or vertex stage. The pass itself
does not release a returned pointer after this call.

`00b17d90` is ECX owner, stack **signed** index, EAX result, `RET 4`:

1. Compare signed index with sign-extended owner short `+38h`.
2. If index is below the count, read `owner+0Ch+index*4` and return a non-null
   pointer immediately, **without incrementing its reference count**.
3. Otherwise, or for a null slot, call `InterlockedIncrement` on
   `owner.error_texture+4`, then return `owner.error_texture` from `+98h`.

There is no negative-index guard, and fallback does not guard a null error
texture. Valid source-3 selectors with original indices 0 through 10 decode
to valid nonnegative lookup indices. Do not normalize this asymmetric reference
behavior into an always-retained getter: it is present in the assembly. Its
overall lifetime correctness across renderer binding is not established here.

## Owner storage and lifetime

Constructor `00b18d60` has ECX destination, no stack arguments, EAX destination,
plain `RET`; destructor body `00b18eb0` has ECX owner, no stack arguments, plain
`RET`. The owner vtable is `00d5e534`.

| Offset | Established contents |
| --- | --- |
| `+04h` | Interlocked reference count, initialized to 1 |
| `+0Ch..+37h` | Eleven texture pointers, zeroed by constructor |
| `+38h` | Signed-short texture high-water count, initialized to zero |
| `+3Ch..+93h` | Eleven eight-byte names, constructed then zeroed |
| `+94h` | Signed-short name high-water count, initialized to zero |
| `+98h` | Error texture returned by renderer virtual `+64h` with `"error.tga"`, flag 0 |
| `+9Ch` | Start of retained secondary object pointer storage |
| `+A8h` | Secondary object count, initialized to zero |
| `+B8h/+BCh` | Separate name length/data, initialized to zero |
| `+C0h` | Serial copied from and incrementing global `00f8d3a8` |

The fixed capacity eleven follows the constructor's 44-byte texture clear and
eleven-element, eight-byte name constructor. It is not an inferred allocation
capacity from unrelated sampler limits. Neither registration nor lookup checks
against eleven.

Destructor first releases the error texture and nulls `+98h`, calls `00b187a0`,
frees the separate `+B8h/+BCh` string, destroys all eleven name strings, then
calls the base destructor. `00b187a0` releases and nulls each texture up to the
current texture count and resets `+38h`; it then releases/nulls secondary object
slots up to `+A8h` and resets that count. Each release uses InterlockedDecrement
and calls object virtual slot zero only on transition to zero. It leaves the
registered names/name count and error texture intact when called independently.

## Deferred loading boundary

`00b19000` has ECX owner, no stack arguments, plain `RET` and SEH/string cleanup.
It iterates registered names in ascending index order. For each name it calls
`00b1bc70` on global manager `00f8d434`; a nonempty returned name takes priority,
otherwise it uses the registered name. If the selected name is empty, it skips
the load and leaves an existing texture at that slot unchanged.

For a nonempty name, it calls renderer global `00f8d394`, virtual `+64h`, with
the native string reference and flag 0. It grows the texture high-water short
when needed, assigns the returned pointer to `owner+0Ch+index*4`, retains the
new pointer before releasing the old one if their identities differ, and then
releases the temporary returned reference. A null load result therefore clears
an old slot; an empty selected name does not. There is no rollback or sorting.

The only direct caller reported for `00b19000` was `00b24dd0`: ECX renderer,
no stack arguments, plain `RET`. It walks 2Ch-byte records at renderer `+1A9Ch`
with count `+1AA0h`, passing each record's `+28h` pointer to the owner loader.
The broader renderer lifecycle invoking this loop remains outside this batch.

`00b1bc70` has ECX override manager, stack output-string storage, stack name
reference, EAX output storage, `RET 8`. It creates a Lua globals reference from
manager `+4`, looks up the name (C string), and returns either exact-string
conversion with empty fallback or a copy of global string `00f8d438` for nil.
`00f8d438` is zero-initialized in the saved image; its runtime writes were not
traced, so it must not be assumed always empty. Name override initialization,
the renderer virtual `+64h` texture loader, its path rules/cache/error behavior,
and device/reset lifetime are unresolved dependencies. Stop here rather than
substituting the existing diagnostic DDS loader for that native lifecycle.

## Next implementable unit

The narrowest complete routine is `00b17d90` with an explicit owner model holding
eleven logical texture pointers, the signed-short active count, and an existing
non-null fallback texture. Preserve borrowed success versus retained fallback.
Document nonnegative in-capacity host preconditions instead of claiming native
bounds safety. `00b18fb0` can independently provide full name registration under
the same valid-index precondition, with its exact self-assignment/count ordering.
Neither routine requires pretending that native texture loading is implemented.

Candidate Ghidra names, for coordinated primary-agent review: `00b18fb0`
`BSP_ShaderOwner_RegisterTextureName`, `00b17d90`
`BSP_ShaderOwner_GetTextureOrRetainedError`, `00b19000`
`BSP_ShaderOwner_LoadRegisteredTextures`. No Ghidra mutations were made by this
analysis batch.

## Verified body evidence

End addresses are exclusive; byte counts include final RET operands.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00b18fb0` | `00b19000` | 80 | `58fd67f221744c024c933d6617afec76a3657468f6c7dea9a9d11ca938a62b5f` |
| `00b17d90` | `00b17dc1` | 49 | `c54d6769cf94eeab47078b3a800bc340475c1dd8ca0c6373fafd0c49847f913e` |
| `00b18d60` | `00b18e90` | 304 | `b7aa3f1e6a7a8f18575d4901439ba841e54aa873fb6a2b236dcc8ee7335ebebc` |
| `00b18eb0` | `00b18f6c` | 188 | `7d32856743d67267dd100d2bf744b5f226ace335660d11e8cac9badd0df58666` |
| `00b19000` | `00b191ce` | 462 | `efac483a85ca420e0c670bd25547e6678b8ce87779d1490f34ab2211f4899c56` |
| `00b187a0` | `00b18849` | 169 | `2328b8d60a6d6236d79ebb8904a7776b3e3043ee9ab6c785dd1e2f4010a1181b` |
| `00b1bc70` | `00b1bd1d` | 173 | `777f8f5afa7e2cc968c3bd63a00703134d7a6842e69693e90c62db6730385ef0` |
| `00b24dd0` | `00b24e11` | 65 | `546c5e92b87cea6a70b1ecf786b9a6f721a51ae8e4e043eef7ee854cb622152b` |
