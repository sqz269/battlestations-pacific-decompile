# Shader interpolator packing and source generation

The typed shader builder now implements three assembly-grounded operations:
`00b34aa0` appends component mappings, `00b36e30` emits `sInterpolators`, and
`00b37000` emits `UnpackInterpolators`. Original executable and saved Ghidra
bytes agree for all three complete bodies. See `reports/shader_interpolators.json`
for assembly, hashes and the source literals used by the emitters.

The mapping routine is thiscall RET4. It starts at field index 1, appends rather
than clears, and walks component bits 0 through 3 for TEXCOORD and COLOR fields.
It ignores component count and semantic index. Each mapping stores the low byte
of the field index and one component byte. Other semantics are ignored except
FOG, whose last field replaces the two-byte fog mapping. The low byte FF is the
absent-fog sentinel even when produced by index truncation. The new interface
preserves these decisions using vectors and a two-byte value projection; it
does not reproduce the native allocator or failure behavior.

The structure emitter is thiscall RET0Ch with include-position, include-fog and
allow-vPos arguments. Mapping counts, rather than source field widths, determine
the declaration widths: groups of four followed by a final float1/2/3/4 group.
Builder metadata at +7Ch/+80h and +84h/+88h records the register count and final
width for TEXCOORD and COLOR respectively. Empty lists set count zero and final
width four. Position is optional; Fog additionally requires a non-FF mapping;
vPos requires allow-vPos and either descriptor's byte +30h. The typed interface
rejects counts whose native signed count+3 would overflow, leaving outputs
unchanged, rather than attempting a malformed huge declaration.

The unpack emitter is thiscall RET4. It emits assignments in TEXCOORD order then
COLOR order. Mapping i reads register i/4 and channel i%4, assigning to the
stored field's name and component. Names follow native `%s` termination. Fog is
copied from INT.Fog unless builder byte +98h requests zero; vPos is copied when
either descriptor requests it. These conditions deliberately do not consult
the structure emitter's include flags: callers must choose a consistent pair.
Unmapped fields/components remain uninitialized, matching the generated native
text. Out-of-range mapping references are explicit new-interface errors with
unchanged source output; native undefined accesses are not reproduced.

String helpers `00b35030`, `00b34f20` and the previously verified `00b35110`
append a newline. The implementation retains native tabs, spaces, the extra
newline after `sPixelIn PixelIn;`, and the double newline after `};`.

The existing shader probe uses a sparse x/z TEXCOORD field, another four-channel
TEXCOORD field and a four-channel COLOR field. It checks the six-component
mapping's register crossing and compiles the generated structure/unpack source
with a diagnostic main as ps_2_0, then checks actual shader binding. Win32 Release
build, both existing CTests and the full D3D9 probe passed. No new test target or
framework was added. This is compilation and binding evidence, not a native
differential execution or pixel-output comparison for the generated shader.

Descriptor filtering (`00b36800`), vertex-side packing, system/register
declarations, system initialization, effect wrappers and complete main generation
remain dependencies of the game's material pipeline. The atlas draw still uses
its diagnostic fixed-function material; the full game is not rebuilt.
