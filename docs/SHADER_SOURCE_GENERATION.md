# Shader source generation boundary

Read-only investigation of `00b36800` and `00b39880` in verified project `bsp`,
program `/battlestationspacific.exe`. Six relevant routine bodies match the
original executable byte-for-byte. Hashes, ABI notes and installed descriptor
identities are in `reports/shader_source_generation_evidence.json`. Raw assembly
and pseudocode remain in ignored `exports/shader_source_generation/`.

## Generated pixel source

`00b39880` is a pixel-source generator, thiscall with builder in ECX and two
stack arguments, RET8. The first argument supplies the field-list input for
`sPixelIn`; the second is forwarded to `00b37000` for interpolator code.
In one builder path both arguments are builder+1Ch. Output is the builder's
string at+4Ch (length) and+50h (data). This string is reset before generation.

The builder has a base descriptor at+70h and a combined/effect descriptor at+74h.
Do not confuse these builder offsets with similarly placed runtime constant
metadata pointers in a finished pass. The generator performs this sequence:

1. Select an RM define from effect descriptor+10Ch: 0 NORMAL, 1 REFLECTION,
   3 UNDERWATER, 4 REFRACTION, 6 DRAW_SHADOW, 7 MAP. Other values emit none at
   this particular branch. Then invoke header generation `00b38ff0(1)`.
2. Append both descriptor strings at+E8h (data pointers+ECh), separated by
   newlines. Generate additional declarations through `00b36e30`.
3. Emit `sPixelIn` using the first argument, starting at field 1, without
   semantic annotations; emit `sSysValues` from builder+34h starting at field 0.
   Additional system helpers include ambient/fog and sRGB sampling code.
4. Wrap base descriptor string+F8h in
   `void ShaderCode(sPixelIn IN, inout sSysValues SYS)`.
5. Wrap effect descriptor string+F8h in
   `void EffectCode(sPixelIn IN, inout sSysValues SYS, out float4 FinalColor[N])`,
   adding an output Depth argument when builder+A8h is nonzero.
6. Emit interpolator conversion through `00b37000`, then `main`. Builder+A4h
   controls one through four COLOR outputs; +A8h controls DEPTH. Main unpacks
   interpolators into IN, initializes SYS through `00b357d0`, calls ShaderCode
   followed by EffectCode, and routes FinalColors to outputs. Mode, fog,
   visibility, alpha and premultiplication branches can modify the result.

This establishes the entrypoint name `main` and why literal descriptor PS
strings cannot be compiled alone. It does not establish a complete generated
debug shader, compiled bytecode, or native shader cache identity.

## Installed debug descriptor relation

The current 806-byte `shaderfx/common/debugshader.shfx` includes `dx9_lua.inc`.
It declares Position and Color as FLOAT4 inputs, Color as a FLOAT4 interpolator,
VS assignments through SYS/IN/OUT, and a PS assignment to SYS.DiffuseColor.
Its four combiner entries name `dummy.shfx` for normal, underwater, reflection,
and refraction modes. The installed file found by that basename is
`shaderfx/lights/dummy.shfx`; its PS assigns SYS.DiffuseColor to FinalColor[0].
These two PS fragments match the distinct ShaderCode and EffectCode wrappers.
The exact filename resolution/descriptor merge that chooses that file has not
been proven in this pass.

`dx9_lua.inc` confirms FLOAT=0, INT=1 and the declaration semantic numbers used
by the formatter below. `cViewProjMat` in the VS still requires generated
constant declarations and register assignment; neither the empty Constants
section nor the short descriptor proves those dependencies are absent.

## Bounded next implementation: field and struct formatting

`00b385b0` is a complete field formatter: ECX points to a 1Ch field record,
stack arguments are an output string object and a semantic-enable flag, RET8,
and EAX returns the output string pointer. The field record is:

| Offset | Meaning |
|---|---|
| +0,+4 | Name string length/data |
| +8 | Base type: 0 float, 1 int |
| +C | Component count |
| +10 | Component mask, not consumed by this formatter |
| +14 | Semantic kind |
| +18 | Semantic index |

It emits the type, a decimal component suffix when unsigned count >1, two tabs,
the name, then optionally two tabs plus ` : `, the semantic name and decimal
index. Semantic kinds 0..10 map to POSITION, COLOR, TEXCOORD, NORMAL, BINORMAL,
TANGENT, BLENDINDICES, BLENDWEIGHT, FOG, INDEX, VPOS. Finally it appends `;`.
For example a valid float4 Color record with semantics enabled produces
`float4\t\tColor\t\t : COLOR0;`.

The trailing semicolon is especially important: pseudocode incorrectly removes
the blocks at `00b389d2`, `00b389f0`, and `00b38a18`. Assembly copies the literal
at `00ce5698` (`;`) and appends it before returning. Use the assembly-backed
contract, not the incomplete decompiler tail. Unsupported type/semantic values
do not have useful validated HLSL behavior; an initial typed interface can
explicitly accept the known descriptor domain instead of inventing tokens.

`00b38b50` is a 261-byte struct emitter, ECX builder, five stack arguments:
start field index, name string, field-list pointer, semantic flag, and allow-vPos
flag; RET14h. It appends `\nstruct <name>\n{\n`, formats fields from start through
the live list count, and prefixes each field with a tab and ends it with a newline.
If allow-vPos is true and either descriptor byte+30h is nonzero, it appends
`\tfloat2 vPos;\n`. Its variadic formatting helper `00b35110` appends that
extra newline after formatted text, despite decompiler-elided copy blocks. It closes
with `};\n` via `00b34f20`, which additionally appends a newline.

These two formatters are a bounded next port using new C++ strings and field
records. They generate real native header syntax and can consume the known
debug declaration projection without first reconstructing all Lua parsing.
Proposed descriptive names are `BSP_ShaderField_FormatDeclaration` for
`00b385b0` and `BSP_ShaderBuilder_AppendStructDeclaration` for `00b38b50`;
neither name has been applied to saved analysis.
They must not be presented as a complete shader generator or as a replacement
for header/register assignment, system initialization, interpolator packing,
descriptor merge and compile/cache handling.

## Interpolator selection is separate

`00b36800` is thiscall RETCh: two optional component-usage inputs and an output
pointer vector. It first adds a float4 ScreenSpacePos/POSITION0 record, then
walks descriptor interpolator lists at+ DCh/count+E0h in base/effect order.
With both usage inputs null it copies full component counts and masks. Filtered
paths select TEXCOORD/COLOR components, alter counts/masks and maintain separate
running offsets. Their precise usage-buffer layout remains a dependency.

`00b34aa0`, thiscall RET4, consumes that list beginning at index1. It creates
two-byte field-index/component-index records for selected TEXCOORD and COLOR
components (testing only component bits0..3), and records a FOG field index.
It does not emit HLSL. This separation explains why selecting descriptors and
packing interpolators must precede structure and main generation.

No C++, analysis metadata, naming ledger, or saved Ghidra project changed.
No generated shader compilation, native differential test or gameplay validation
was performed.

Integration follow-up: proposed function names and evidence comments were saved
in Ghidra, and affected exports refreshed. These investigated routines remain
unported; the separate GUI texture resolver/lookup now passes its installed
atlas integration probe.
