# Vertex source composition boundary

`00b38080` was previously described as a system helper. It actually emits vertex
sampler declarations. The reconstructed typed emitter walks base then effect
descriptor lists (+C4h pointer, +C8h count), using each record's name string
at +4h/+8h, enabled byte +Ch, and dimension +10h. Dimensions 1/2/3/4 emit
sampler1D/sampler2D/samplerCUBE/sampler3D. Disabled records consume no register;
enabled unknown dimensions consume a register but emit nothing. The shared
counter starts at zero, continues across descriptors, and does not deduplicate
names. Native register text is signed decimal and names use `%s` termination.
Native ABI: ECX builder, RET. New vectors/strings do not reproduce object ABI.

The existing shader probe checks one disabled record, one enabled unknown,
a 2D sampler at s1 and an effect cube sampler at s2. These declarations compile
in its diagnostic vs_2_0 source but are unused. Actual vertex texture sampling,
device limits and native bytecode identity are not verified by this fixture.
Win32 build, both existing CTests and the full D3D9 probe pass. Full-body byte
comparison and assembly are in `reports/shader_vertex_samplers.json`.

## Full generator assembly order

The inspected `00b39110` generator is still unported. Its dependencies now
have typed projections, but assembling an arbitrary diagnostic main is not
equivalent to its native main. The native sequence is:

1. Clear source and append a render-mode define from effect descriptor +10Ch.
2. Emit system constant header with register annotation enabled; append both
   descriptor header strings at +E8h using formatted string termination.
3. Emit sVertexIn from builder+4h with semantics, sSysValues from +10h without
   semantics, and sVertexOut from +1Ch without semantics. Emit sInterpolators
   with position and fog enabled, vPos disabled.
4. Emit vertex samplers. Emit the literal GetShadowFadeOut helper when effect
   byte +15h is set, then the unconditional literal GetAmbient/GetFog helpers.
   Those literals reference ambient/fog constants even for otherwise simple
   descriptors. GetShadowFadeOut's early zero return is present in the native
   literal and must not be replaced by an inferred lighting implementation.
5. Wrap base VS text (+F0h) in ShaderCode(IN,SYS,OUT); wrap effect VS text in
   EffectCode(SYS,OUT). These are length-based string appends, not `%s` inserts.
6. Emit PackInterpolators. Its name lookup uses builder+28h, which is distinct
   from the +1Ch list used to declare sVertexOut; preserve this distinction.
7. Main declares SYS and OUT, emits zero assignments from lists +10h and +1Ch,
   decodes input fields, calls ShaderCode then EffectCode, and unconditionally
   copies SYS.ScreenSpacePos into OUT.ScreenSpacePos before packing/returning.

Next integration must provide the actual system constant registry, both relevant
output-field lists and helper literals, then compile a complete generated source
using descriptor VS fragments. The host diagnostic main remains separate from
that implementation. Pixel main/effect generation, material selection and game
runtime still remain incomplete.
