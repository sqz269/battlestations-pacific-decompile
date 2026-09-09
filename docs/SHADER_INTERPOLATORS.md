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

## Vertex packing follow-up

`00b35540` now emits the matching `PackInterpolators(sVertexOut OUT)` function.
Its native ABI uses ECX for the builder and no stack arguments (RET). Unlike
pixel unpacking's explicit list parameter, names come from the builder's field
list at +28h. It always assigns INT.Position from OUT.ScreenSpacePos, then
TEXCOORD and COLOR components in mapping order, followed by optional fog and
the return. It does not emit vPos or consult the pixel zero-fog flag. Output
field declarations and the interpolator structure must be prepared consistently
by the caller. The typed API preserves uninitialized unmapped components and
native formatted-name termination, with the same new bounds-error policy as
unpacking. Original disk/Ghidra body and literal comparisons are retained in
`reports/shader_vertex_pack.json`.

The existing compilation fixture now generates both sides: a vs_2_0 vertex
shader with this packer and a ps_2_0 pixel shader with the unpacker. Sparse x/z
and register-crossing assignment checks pass, as do creation/binding on the
real D3D9 device, the Win32 build and both existing CTests. Shader main bodies
still supply diagnostic values; this fixture does not execute a pixel readback
through the generated pair or establish native game shader equivalence.

Tracing the caller identified `00b39110` as the full vertex-source generator.
It clears builder source, selects the render-mode define, calls system constant
header generation `00b38ff0`, emits structures, adds system helpers through
`00b38080`, wraps base/effect descriptor VS strings at +F0h, emits this packer,
and builds main ending in PackInterpolators(OUT). Its remaining body is unported.
The inspected `00b372d0` builds pixel system-value field descriptors rather than
vertex packing code; its allocation-heavy body is also unported.
