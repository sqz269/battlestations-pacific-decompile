# Font image resources and material bindings

This bounded audit connects the existing scalar font/quad reconstruction to
native texture slots and shader selection. It does not reconstruct a text draw,
font resource destruction, compiled material loading, or shader execution.

## Evidence and ABI

The live `bsp` project, `/battlestationspacific.exe`, x86 little-endian image
base `00400000`, were verified before each analysis batch. Ignored exports are
under `exports/bsp/owner_textures/font/`. Every byte span below was read from
Ghidra and compared equal to the installed executable. End addresses are
exclusive; spans are evidence windows, not necessarily whole function bodies.

| Start | End | Bytes | SHA-256 |
|---|---|---:|---|
| `00b189f0` | `00b18a40` | 80 | `ddcd3b96b4cce2f22b205119a4e9cf9a648c23a228bf1bb608373d76cd2b9116` |
| `00ab8ce0` | `00ab8e78` | 408 | `9f7ca21a2f624aa63185e247dd530c871cc4d6c27d72ed4f85cee38d3ca37b7d` |
| `00ab8c30` | `00ab8cd5` | 165 | `d434b39aa4b1049e73f05217bd1fec64ed4e1e017db853441ea7b8e006f93e06` |
| `00aba270` | `00aba8c7` | 1623 | `4c435d42b8c60f1eeab6987c2530b1bd54216c74c9d1b325f0f491f8d114d54b` |
| `00ad4c30` | `00ad4cd6` | 166 | `cc6f67915be3aa03d945e0df16c351e4dbde51faae759cb9c9fabe0f68da7539` |
| `00ad4e01` | `00ad4e98` | 151 | `f30e21170457fab293a7223a522ac805b0aa6424f1f172fa189aed9df965f379` |
| `00ad4ff3` | `00ad500a` | 23 | `cc5701724542269ad546a5288e26b34bb6852d215ec6b6d08b05f1c52fc3073d` |
| `00abaa03` | `00abaa40` | 61 | `2ffdd1ca560cc12074913dbe991248f366c17bad2c463d3b86422e73a382c51f` |

`00ad4c30` is ECX font and five stack arguments (`RET 14h`): prefix,
Data name, GFX name, AlphaTexture name, extra path component. The call at
`00ac3d99` establishes Data/GFX/AlphaTexture argument order. The startup extra
path component still has the unresolved origin described in
`FONT_GLYPH_SOURCE.md`; do not replace it with an invented directory.

`00aba270` is ECX text context, stack UTF-16 string and draw section, `RET 8`
at `00aba8c4`. Its decompiler omits the second argument. `00b189f0` is ECX
material, stack unsigned slot and resource pointer, `RET 8` at `00b18a3d`.
`00ab8ce0` is ECX context, no stack arguments, AL indicates a newly performed
selection (including a null loader result); it is not a success-only predicate.

## Image loading and glyph aliases

The first texture name is `(prefix + extra component) + GFX name`. Assembly
`00ad4c9e..00ad4cbc` performs the two native string concatenations; renderer
virtual `+64` receives that name and flags zero at `00ad4cd0`. Its result is
retained locally at stack `+64` in the established post-prologue frame.

AlphaTexture has a distinct path rule. At `00ad4e01`, its native string length
selects these branches:

- Nonempty: concatenate **prefix + AlphaTexture**, without the extra component,
  then call renderer virtual `+64` with flags zero at `00ad4e29`.
- Empty: construct literal `white.tga` (`00ce77a4`) and load that name alone,
  again flags zero, at `00ad4e6e`.

Both save the second result at stack `+6C`. The registry's missing/non-string
AlphaTexture default is the nonempty string `white.tga`; therefore a missing
field takes the prefix branch, while an explicitly empty string takes the
unprefixed branch. Do not collapse these into one fallback path. Installed
`fonts/fonts.lua` omits AlphaTexture for all six entries.

`00ad4ff3..00ad5007` copies these two pointers into each ordinary glyph:
`glyph+18 = GFX`, `glyph+1C = AlphaTexture`. These are direct stores without
per-glyph retains in this sequence. No null-result rejection appears between
the texture calls and these stores. This establishes aliases, not independent
glyph ownership. Native font destruction and the ownership of the initial
loader references remain unresolved here; do not release every glyph pointer
as if each store acquired a reference. Special space/LF/fallback record copying
also remains governed by the separate scalar-record audit.

## Material slots and geometry

In `00aba270`, `00aba688..00aba6a7` binds selected glyph `+18` to material slot 0
and `+1C` to slot 1. The material comes from the second argument's `+20` field.
The byte flag at local stack `+13` is cleared at `00aba6b2`: the initial
resource-binding block runs once, on the first glyph reaching that block,
rather than switching texture pages on every character. A multipage batcher
cannot be inferred from these two calls.

Material slots are pointers at `material+10+4*index`; the high-water count is
the signed 16-bit field `+34`, sign-extended and compared as unsigned to index.
`00b189f0` first raises that count to low16(index+1) if necessary. If the pointer
differs, it stores the new pointer, increments its intrusive `+4` count when
nonnull, then decrements the old pointer's count and invokes old virtual zero
on zero. Identical pointers skip the retain/release, after the count update.
There is no bounds check. These are material bindings, separate from the
shader-owner source-3 texture array.

The existing `FONT_GEOMETRY.md` establishes the sole supplied UV pair as glyph
atlas coordinates and the normalized object positions (960/720 divisors).
There is no extra alpha UV array to invent in the CPU quad. Shader source
generates its second UV from object position, described below.

## Shader selection and alpha scale

`00ab8ce0` skips selection when `context+1EC` is nonnull. Otherwise the native
case-insensitive inequality helper `00449af0` compares the name at `+1C0` with
an empty string (`00ce3a0c`). A nonempty explicit name is loaded through renderer
virtual `+48` and cached. With an empty name:

- `GuiFont.mshd` is selected only when `CVTSI2SS(global0109cf04->DWORD+28)`
  compares ordered-equal to float 720 (`00d5c56c`, bytes `00 00 34 44`) **and**
  font float `+18` compares ordered-equal to 1.
- Otherwise `GuiFontBilinear.mshd` is selected. The `UCOMISS; LAHF; TEST AH,44h;
  JP` sequences at `00ab8d82..00ab8da5` also route unordered values here.

This audit names the global field by address; identifying it as a particular
window dimension requires its writers, which are outside this bounded trace.
There is no universal point-filtered or bilinear font default.

On a changed font name, `00ab8c8e..00ab8c9d` copies font `+1C` into context
`+1D4` (1 if resolution returned null), then releases and clears the cached
shader at `+1EC`. `00aba8d0`, after a newly performed selection, copies context
`+94` and `+1D4` via x87 into adjacent floats `+1DC/+1E0` at
`00abaa03..00abaa24`; it registers their address under `cOverbrightAlphatex`
through `00b18b00`. The same named pair is registered for the subsequent
section path at `00abac71`. This establishes alpha-scale provenance without
assuming the underlying shader constant upload implementation.

## Installed shader source distinction

Read-only installed asset evidence:

| Asset | Bytes | SHA-256 |
|---|---:|---|
| `shaderfx/gui/guifont.shfx` | 1947 | `0a9c7265a757c377275b3a04955de00ee59f6531dcb2ce080f464e81ef68470c` |
| `shaderfx/gui/guifontbilinear.shfx` | 3161 | `b587561dc1b3ed48bdd0e18e2c8d17b1785b678a988551d376cd248274bd91cd` |
| `fonts/fonts.lua` | 1115 | `26ebc6c0905f3aff36e95f40195c0fe8693d9f17474d2fcd3e51bcc7091fdd93` |

Both sources declare `MyTexture0` index 0 and `MyTexture1` index 1, 2D samplers.
Atlas sampling clamps U/V and disables mips; nonbilinear uses point min/mag,
bilinear uses linear min/mag. The second sampler wraps U/V and disables mips;
its min/mag are not explicitly supplied by these source tables.

Nonbilinear `guifont.shfx` computes secondary UV as
`IN.Position.xy * cAlphaTextureScale`, and multiplies both sampled **RGBA**
values with interpolated vertex/material color. AlphaTexture is not merely an
alpha-channel lookup. RGB is blended toward white by `cOverbrightFactor`, and
the final float4 is saturated.

However, bilinear `guifontbilinear.shfx` computes secondary UV using
`cOverbrightAlphatex.y` but **comments out the second texture sample**. Its
active pixel shader multiplies only atlas RGBA and color, brightens RGB using
the pair's x component, and optionally applies clipping to alpha using world
position, aspect ratio and clip constants. The final result is not wrapped in
the nonbilinear shader's final saturate. Both sources request alpha test
GREATER 0, SRCALPHA/INVSRCALPHA blending, and disable Z test/write and fog.

These are installed source facts, not proof of the loaded `.mshd` bytecode.
The nonbilinear source's separate constant names disagree with the native
`cOverbrightAlphatex` registration. This audit did not locate/read compiled
`.mshd` content or establish its mapping to these source versions. Preserve
that boundary before claiming native shader parity.

## Next integration boundary

A retained TGA texture loader can provide the real atlas resources with the
observed flags zero. It must preserve the general 2D loader's actual COM
description versus requested/source metadata distinction; this font audit
found no font-specific replacement of that behavior. Next native integration
needs font-level image ownership, the two material slots, and actual compiled
material/shader resolution. White-image fallback, two-sample alpha behavior,
and point/bilinear selection must remain distinct until those routes are
resolved. Existing glyph geometry plus an arbitrary alpha shader is not a
reconstructed text renderer. No code, tests, build, Ghidra annotation or
installed asset mutation was performed by this audit.

Parent integration subsequently applied the reviewed material-slot, shader
selector and glyph-geometry names/comments, preserved prior annotations, saved
the project and refreshed affected exports. Implemented glyph and texture
checks are recorded separately in `reports/font_geometry_texture_probe.txt`.
