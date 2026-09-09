# Shader descriptor scalar options

`src/shader_lua.cpp` extracts the scalar/string portion of reader00b43b00
into `ShaderLuaOptions`. This is a stock Lua adapter, not the native descriptor
layout or a complete material loader. Project `bsp` and program
`/battlestationspacific.exe` were verified before the analysis batch.

Native reader ABI: ECX=descriptor; stack path-string object and unsigned
profile generation; RET8. Assembly00b43b8f..00b440df establishes this sequence:

| Lua key | Native offset | Default |
|---|---|---|
| PipeID | 04 | integer0 |
| Priority | 08 | integer0 |
| VertexFormat | 0C/10 string | simple.mvfm |
| ReceiveShadows | 15 | false |
| ShadowShader | 100/104 string | empty |
| derived has-shadow flag | 14 | nonempty ShadowShader |
| FinalLODFadeOut | 16 | false |
| FinalLODFadeOutRange | 18 | float0.01 |
| RTCount | 108 | integer1 |
| VisilityFade (native spelling) | 44 | true |
| AlphaToCoverage | 1C | false |
| NoBandingFix | 1D | false |
| DisableAlphaToCoverage | 1E | false |
| CompressedVertices | 1F | true |
| CompressedElemCount | 20 | integer999 |
| InstanceGenerator | 28/2C string | empty |
| PixelPositionRegister | 30 | false |
| OutputAlpha | 31 | true |
| LoResBlend | 32 | false |
| WriteDepth | 45 | false |

Boolean helper00b662f0 accepts only Lua BOOLEAN (type1); numeric truthiness
and strings do not override the default. Float helper00b66330 accepts only
NUMBER (type3), spills to float32 and reloads to ST0. Integer helper00b66380
likewise requires NUMBER, spills to float32, then calls00bf7420 for signed
truncation. All three take ECX=Lua reference and one stack default, RET4.
The host rejects nonfinite/out-of-range integer conversions; exceptional FP
and nondefault control modes remain unverified. Signed host integers retain
the native 32-bit result; later unsigned consumers need explicit conversion.

The float default is loaded at00b43dbd from00d7a238, bytes `0ad7233c`
(float32 bits3c23d70a), then stored at00b43dd6. Strings use the existing
00b685c0 exact-string gate and C-string copying, truncating embedded NUL.
The host preserves lookup order within this option group, but the complete
reader's interleaving with code/profile/table lookups is not reconstructed;
arbitrary metamethod side-effect parity is not claimed.

The existing installed-file probe checks debug priority23, OutputAlpha=false,
dummy priority0 and OutputAlpha=true, plus default compression/count,
visibility, RTCount and fade range. These are descriptor extraction checks.
The diagnostic draw still supplies its previous explicit source-generator
options. In particular, enabling decoded inputs requires compiled reflection
and runtime scale/offset binding: nominal registry slots87/95 are beyond the
explicit-register cutoff77 and cannot be assumed to be compiled slots.
No full material execution, ABI equivalence or gameplay validation is claimed.
