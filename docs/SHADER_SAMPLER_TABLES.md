# Native shader sampler tables

Read-only recovery on 2026-09-09. Client.verify() confirmed project `bsp`,
program `/battlestationspacific.exe` before each analysis/export batch.
Ignored evidence: `exports/bsp/sampler_analysis/`. No C++, Ghidra state,
shared metadata or game files changed. Names below describe inferred roles.

## Entry points and ordering

`00b41830`: ECX = shader descriptor; stack argument = Shader Lua reference;
RET4 at00b419a6. It checks `Shader.Samplers` for exact TABLE, then fetches
it again and iterates with00b67080/00b67190 (Lua next, not array indexing).
Missing/non-table leaves the existing descriptor list unchanged.
At00b418f0 it checks the **key** using00b66a60. Only exact Lua NUMBER keys
whose float32 conversion equals the converted signed integer qualify;
negative and zero keys are not excluded. Numeric strings do not qualify.
The actual key does not choose a slot: accepted values are appended in
iteration order to descriptor+C4h (pointer vector, count+C8h, capacity+CCh).
There is no sort, deduplication or clear. A selected value has no table-type
check before being handed to the entry reader; rejecting malformed values
would be an explicit host-adapter behavior.

`00b57b50`: ECX = state-definition manager (global0108fe90 in caller);
stack argument = entry Lua reference; returns a new sampler pointer in EAX;
RET4 at00b57fca. Do not use the decompiler's apparent one-register signature.
The native allocation is2Ch bytes. Fields are fetched by name in this order:

| Field | Native offset | Conversion and default |
|---|---|---|
| Name | +4h length, +8h data | Unguarded Lua string coercion, then C-string copy |
| Type | +10h | Unguarded numeric coercion, float32 then signed truncation |
| TextureSource | +14h | Initially0; first lookup must pass integer-number predicate, then fresh coercing numeric lookup |
| TextureSourceName | +18h length, +1Ch data | Initially empty; read with string coercion only when converted source is1 or3 |
| Index | +20h | Initially0; integer-number predicate then fresh coercing numeric lookup |
| VertexSampler | +0Ch byte | Initiallyfalse; exact BOOLEAN predicate then fresh Boolean getter(defaultfalse) |
| SamplerStates | +24h pointer | Always allocated empty12-byte list; table values read with manager+10h registry |
| TextureStageStates | +28h pointer | Always allocated empty12-byte list; table values read with manager+1Ch registry |

Object+0 is vtable00d621f4. +10h is not initialized in the allocation block,
but is always assigned from Type at00b57c62. Padding is not semantic data.

`Name` and `TextureSourceName` are **not** exact-string/default-empty getters.
00b662b0 calls00a67810 (Lua tostring with null length argument); existing
strings pass, numbers are converted by00a6d1e0 using `%.14g`, and other
values return null.00b57bc7 and00b57d0a pass that pointer to0041e870;
instruction **0041e890** dereferences without a
null check while finding the terminating NUL. A safe adapter should report
missing/nonconvertible required names, not invent native empty defaults.
Embedded NUL truncates the native name. Stock Lua number formatting still
needs the same locale/library assumptions for byte-for-byte parity.

00b66290 calls Lua numeric coercion00a67770, spills the result to float32,
reloads it and jumps to00bf7420. Numeric strings are accepted, nonconvertible
values become0. Thus missing Type becomes0; this routine does not validate
supported dimensions. Out-of-range/NaN conversion and allocation failures
must remain explicit unsupported adapter cases.00b66a60's assembly uses
FSTP float, CVTTSS2SI and x87 comparison, not an integer-key range1..N check.

The installed dx9_lua.inc names texture sources0 material,1 texture animator,
2 shadow buffer,3 file. This describes input constants; actual texture lookup,
loading, animator ownership and shadow selection are beyond this parser.
Existing recovered declaration emitters00b38080/00b37ef0 select vertex stage
for nonzero+0Ch, pixel stage for zero. Type1/2/3/4 maps1D/2D/CUBE/3D;
unsupported selected types still consume declaration slots. Preserve order
and Type values instead of sorting by Index or silently normalizing dimensions.
Index is separate metadata and is not this parser's append position.

## Ordered state registries

Both state tables use00b579b0, the existing render-state reader, with different
registry arguments. It walks registry order, performs exact-NUMBER first
lookup followed by a fresh coercing numeric lookup, and adds state/value pairs
with first-state-ID-wins behavior. Tag0 means float32-to-signed-int bits;
tag1 means float32 bits. No table means an empty list, not D3D defaults.
See `SHADER_RENDER_STATE_TABLE.md` for that shared helper's detailed contract.

Constructor00b585a0 switches to manager+10h at00b5925e and manager+1Ch at
00b5976d; the decompiler omits these ECX destination changes. The tables below
list exact case-sensitive keys, native state IDs, numeric tags and call sites.
There are13 sampler and18 texture-stage definitions, after31 render definitions.

### SamplerStates

| Key | ID | Tag | Registration call |
|---|---:|---:|---|
| ADDRU | 1 | 0 | 00b59269 |
| ADDRV | 2 | 0 | 00b592cc |
| ADDRW | 3 | 0 | 00b5932f |
| BORDERCOLOR | 4 | 0 | 00b59392 |
| MAGFILTER | 5 | 0 | 00b593f5 |
| MINFILTER | 6 | 0 | 00b59458 |
| MIPFILTER | 7 | 0 | 00b594bb |
| MIPMAPLODBIAS | 8 | 1 | 00b5951f |
| MAXMIPMAPLEVEL | 9 | 0 | 00b59582 |
| MAXANISOTROPY | 10 | 0 | 00b595e5 |
| SRGBTEXTURE | 11 | 0 | 00b59648 |
| ELEMENTINDEX | 12 | 0 | 00b596ab |
| DMAPOFFSET | 13 | 0 | 00b5970e |

### TextureStageStates

| Key | ID | Tag | Registration call |
|---|---:|---:|---|
| COLOROP | 1 | 0 | 00b59778 |
| COLORARG1 | 2 | 0 | 00b597db |
| COLORARG2 | 3 | 0 | 00b5983e |
| ALPHAOP | 4 | 0 | 00b598a1 |
| ALPHAARG1 | 5 | 0 | 00b59904 |
| ALPHAARG2 | 6 | 0 | 00b59967 |
| BUMPENVMAT00 | 7 | 1 | 00b599cb |
| BUMPENVMAT01 | 8 | 1 | 00b59a2f |
| BUMPENVMAT10 | 9 | 1 | 00b59a93 |
| BUMPENVMAT11 | 10 | 1 | 00b59af7 |
| TEXCOORDINDEX | 11 | 0 | 00b59b5a |
| BUMPENVLSCALE | 22 | 1 | 00b59bbe |
| BUMPENVLOFFSET | 23 | 1 | 00b59c22 |
| TEXTURETRANSFORMFLAGS | 24 | 0 | 00b59c85 |
| D3DTSS_COLORARG0 | 26 | 0 | 00b59ce8 |
| D3DTSS_ALPHAARG0 | 27 | 0 | 00b59d4b |
| D3DTSS_RESULTARG | 28 | 0 | 00b59dae |
| D3DTSS_CONSTANT | 32 | 0 | 00b59e11 |

## Next implementation and evidence limits

The smallest useful parser is a typed00b41830/00b57b50 projection in the
existing Lua adapter: preserve accepted outer lua_next order; retain name,
stage flag, Type, source metadata and both state lists; reuse the established
00b579b0 conversion with these two literal registries. Feed only the already
recovered name/stage/Type projection into source generation. Full texture
binding or material sampler creation is a separate integration step.
An existing installed sampler-bearing descriptor can exercise extraction;
the current debug/dummy files have no Samplers and cannot verify this route.
No new tests, runtime execution or reconstruction-count changes were made here.

Live Ghidra bytes matched the original disk PE for these complete bodies:

| Routine | Bytes | SHA-256 |
|---|---:|---|
|00b41830..00b419a8|377|1e22f771cc9ae446482ae0df189a67024766c619ce2e0d86954019125e61055f|
|00b57b50..00b57fcc|1149|2a72cdf3d77507ffc20c4d678a25be378fc9c4b2a03e172a05275f19f405ae4d|
|00b585a0..00b59e48|6313|54922e72d2050e41c8013de250fbab2357c6bdc2ee3b6f5d0b974ce89b6a7f60|

## Implemented adapter

`ShaderLuaSampler` and `read_samplers` in src/shader_lua.cpp now retain the
name/stage/dimension, source/index/name and both ordered state lists. The
existing state conversion is shared across all three registries. Malformed
selected entries and nonconvertible required names report host errors; no
native crash, allocator lifetime or arbitrary metamethod parity is claimed.
The second Samplers lookup is checked for TABLE explicitly; state-list
lookups likewise tolerate a second non-table as empty host output.

The existing D3D9 probe evaluates installed alphablend.shfx and verifies one
MyTexture pixel sampler, dimension2, source/index0, U/V wrap states in that
order and its generated s0 declaration. Win32 build, both existing CTests
and the complete installed-asset D3D9 probe pass. Debug/dummy have no samplers;
their parsed declaration vectors are now passed into both stage generators.
This check does not exercise a textured alphablend draw, texture-source
resolution, nonempty texture-stage states or arbitrary sampler records.
Those remain separate work; no new test target or test framework was added.

## Correction from docs/NATIVE_SHADER_SAMPLER_READER.md

B57B50 and B41830 are now fully reconstructed over the actual sampler, descriptor, definition manager and tracked Lua storage. Native/rebuilt fixtures cover metatable freshness, iteration, append growth, live manager reload and installed shader inputs. The host outer reader explicitly installs shared callable sampler dispatch for descriptor cleanup. Original exception ABI, full descriptor loading, rendering and gameplay remain unvalidated. See the new document and reports/native_shader_sampler_reader.json for evidence.
