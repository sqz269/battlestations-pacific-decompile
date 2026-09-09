# Descriptor flags entering shader generation

This is read-only routing evidence, not a reconstructed material loader. The
`bsp` project and `/battlestationspacific.exe` program were verified by the
export tool before fresh exports of00b45ee0,00b3c3a0,00b3b3c0,00b354d0,
00b39110 and00b39880. Assembly, rather than inferred decompiler parameters,
establishes the register arguments below. Existing subordinate exports provide
additional direct-consumer evidence. Descriptive roles are hypotheses, not
recovered C++ symbols.

## Normal combiner route

Loader00b45ee0 takes ECX=owner, then stack path-string pointer, projected-shadow
selection byte, and an override selector; RET0Ch. At00b45f03..00b45f20 it uses
that byte if the selector is nonzero; otherwise it reads byte `[00f8bbf0]+0E`.
The pointed-to object's complete type and configuration ownership are outside
this analysis. This is not a descriptor flag.

The base descriptor pointer is stored at owner+C4 (00b45f58). Its PipeID and Priority
are copied into owner+AC and+B0 (00b45f63..00b45f75). Nonempty combiner slots
are iterated for mode0..13, loading a separate effect descriptor, whose+10C
is assigned that mode. At00b462f8..00b4631e the compile wrapper receives:

| Location on wrapper entry | Value |
|---|---|
| ECX | owner |
| EDX | base descriptor pointer loaded from owner+C4 |
| stack+04 | effect descriptor |
| stack+08 | effect.WriteDepth, byte+45, zero extended |
| stack+0C | effect.RTCount, DWORD+108 |
| stack+10 | base.VisilityFade, byte+44, zero extended |
| stack+14 | constructed cache-name string pointer |
| stack+18 | selected external projected-shadow byte |
| stack+1C | unsigned profile generation3 |

Wrapper00b3c3a0 uses ECX and EDX as independent inputs and returns with RET1Ch.
It constructs a local builder, then calls00b3b3c0 with ECX=builder and stack
owner/base/effect, RET0Ch. The latter stores these at builder+78/+70/+74
respectively (00b3b3f6,00b3b407,00b3b40a).

The wrapper's stack-relative stores at00b3c41a..00b3c474, accounting for the
intervening PUSH instructions and local builder base `[initial ESP+0C]`, set:

| Builder location | Source | Existing typed projection |
|---|---|---|
| +9C/+A0 | copied cache-name string | not a source option |
| +A4 DWORD | wrapper stack+0C | pixel.color_outputs |
| +A8 byte | wrapper stack+08 | pixel.depth_output |
| +A9 byte | wrapper stack+10 | pixel.visibility_alpha |
| +AA byte | wrapper stack+18 | pixel.projected_shadow |
| +AC DWORD | wrapper stack+1C | profile generation |
| +98 byte | unsigned generation <3 (`CMP`, `SETC`) | pixel.zero_fog |

Consequently normal generation3 uses zero_fog=false. Neither RTCount nor
WriteDepth is merged with the base descriptor; visibility comes only from
the base. The source emitter reads count+A4 and depth+A8 throughout
00b39be3..00b3a228 and count-dependent copies00b3a53f..00b3a579.

## Direct descriptor consumers

These consumers retain both descriptors and do not make a universal combined
options object:

| Option | Source used by generator | Assembly evidence |
|---|---|---|
| ReceiveShadows | effect+15 in both stages |00b3935a..00b3936e;00b39a6c..00b39a81 |
| OutputAlpha | effect+31 |00b3a4af..00b3a4e0;00b3a4f6..00b3a526 |
| NoBandingFix | base+1D |00b3a46d..00b3a47c |
| LoResBlend | base+32 |00b3a50a..00b3a53a |
| PixelPositionRegister | base+30 OR effect+30, additionally gated by caller allow-vPos |00b36fb0..00b36fc9;00b38c0d..00b38c28 |
| CompressedVertices | base+1F |00b3583b..00b35842 |
| CompressedElemCount | base+20, bounded by total input count |00b35848..00b35859 |

NoBandingFix suppresses the time-dependent color transform only for effect
render modes0,12,8,10. ReceiveShadows controls emitted helper text; it does not
by itself prove a complete shadow-resource binding path. OutputAlpha chooses
between the existing source literals for alpha assignment, with base visibility
and fog branches also contributing. LoResBlend's premultiplication is emitted
only in the no-fog/zero-fog path of the current source routine; it is not an
unconditional final multiplication. Preserve the already reconstructed emitter
branching instead of flattening these booleans into generic postprocessing.

The positive vPos tests above are a logical OR for those source declarations,
not evidence that every descriptor field should merge by OR. The wrapper's
external projected-shadow byte is distinct from ReceiveShadows.

## Separate shadow pass

The explicit shadow branch of00b45ee0 changes the descriptor pairing. It loads
`shadow_passtrough.shfx` as effect and the original base's ShadowShader as a
new base, assigns both render mode2, and copies the new base's compression
count+20 into the original descriptor's+24. At00b46775..00b46795 it calls the
same wrapper with ECX=owner, EDX=new shadow base, stack effect, forced depth1,
effect RTCount, forced visibility0, cache name, external projected byte,
generation3. It does not reuse the normal pass's depth/visibility selection.
The spelling `shadow_passtrough.shfx` is the installed/native string.

## Implementation boundary

For the normal mode0 debug/dummy pair, the next source-assembly adapter can
route effect RTCount/WriteDepth/ReceiveShadows/OutputAlpha, base visibility/
NoBandingFix/LoResBlend/compression, and each descriptor's own vPos flag exactly
as above. Supply generation and projected-shadow selection explicitly; do not
invent defaults from Lua flags. In particular, default compression=true still
requires actual compiled constant-register reflection and scale/offset uploads.
Nominal registry positions87/95 are outside the explicit register cutoff77.
This routing evidence does not establish material constant values, the native
file-manager/overlay behavior, complete compiled-resource ownership, shadow
execution, binary ABI compatibility of typed projections, or gameplay parity.

