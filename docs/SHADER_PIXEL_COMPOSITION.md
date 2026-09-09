# Pixel sampler and output composition

`00b37ef0` now has a typed source emitter. It walks base/effect sampler lists in
that order and selects records whose byte +Ch is zero. Comparing the complete
body with `00b38080` resolves that byte as shader-stage selection: nonzero means
vertex, zero means pixel. A record skipped by the vertex emitter is not globally
disabled. ShaderSamplerDeclaration now calls the field `vertex_stage`.

Both emitters share formatting logic with independently initialized counters.
Selected unknown dimensions still advance their stage's counter without emitting
source; records for the other stage do neither. Pixel declarations preserve the
same signed register formatting, name termination and dimension map as vertex
declarations. The native ABI is ECX builder, RET; typed storage is not ABI-compatible.

The existing shader probe checks pixel s0, a skipped vertex entry, an unknown
pixel entry consuming s1, and an effect volume sampler at s2. The generated
declarations compile in ps_2_0 and the shader binds. They remain unused by its
diagnostic main, so texture sampling execution is not established. Win32 build,
two existing CTests and the full D3D9 probe pass without new test targets.

## Full pixel generator boundary

Complete disk/saved bytes for `00b39880` agree; assembly and hash are recorded
alongside the sampler body in `reports/shader_pixel_composition.json`. The full
pixel generator remains unported. Its output policy requires preserving several
distinct branches, rather than sharing the diagnostic main:

- Builder +A4h selects 1..4 COLOR outputs and +A8h adds DEPTH. Each supported
  signature initializes its outputs. The effect wrapper receives a FinalColor
  array sized by +A4h and an optional depth output; main allocates FinalColors,
  zeroes SYS, then calls ShaderCode followed by EffectCode.
- Effect modes 3, 9 and 11 unconditionally emit the power/min color transform.
  Modes 0, 12, 8 and 10 emit its cElapsedTime[1] conditional version only when
  base descriptor byte +1Dh is zero. The interpretation of that byte is unresolved.
- A non-FF fog mapping and builder +98h zero select fog blending. Otherwise
  Color0 is copied directly and base byte +32h may premultiply RGB by alpha.
  The fog branch does not apply that premultiplication.
- Effect byte +31h selects a DiffuseColor alpha override. Builder +A9h additionally
  multiplies it by saturate(cVisibility); otherwise alpha comes directly from SYS.
  Remaining COLOR outputs are copied from FinalColors[1..3] according to count.

Upstream dependencies include effect +15h conditional shadow helpers `00b38230`
and `00b382b0`, unconditional ambient/fog and sRGB sampling helper literals,
base/effect PS strings at +F8h, separate declaration/unpack field lists and
system-value list +34h. These must be composed with the recovered sampler,
constant, struct and unpack emitters before a matched game shader pair can run.

## Full pixel composition implementation

`generate_pixel_source_00b39880` now implements the full sequence through a
ShaderPixelProgram projection. Separate input and unpack lists preserve the
native two-argument distinction. Constants, descriptor flags and system-value
fields remain explicit inputs. The implementation composes all recovered
helpers, length-preserved PS bodies, formatted headers, output signatures,
SYS initialization and the output policies above. Source and interpolator
metadata are replaced only on success. Counts outside1..4 produce a typed
error with unchanged outputs; native malformed source for those counts and
native allocator/exception ABI are not reproduced.

Pixel-specific ambient/fog and sRGB helper literals are retained separately in
`src/shader_pixel_literals.inc`, with original/saved byte agreement. They are
not substituted with the similar vertex literals. The generator preserves the
native distinction between alpha premultiplication in the no-fog path and its
absence in the fog path, plus the two possible effect signatures for depth.

The existing shader probe now compiles full generated debug/dummy vertex and
pixel source and creates/binds that pair. Pixel statements were compared with
the installed descriptors ignoring whitespace; hashes and validation scope
are in `reports/shader_pixel_generator.json`. The fixture uses normal mode,
one color output, no depth or fog, and explicit DiffuseColor system state.
Its registry adds a scalar cElapsedTime[2] declaration to support the emitted
conditional color transform. The supplied register limits256/32 and ordering
are diagnostic projections, not recovered native register identity.

Win32 build, both existing CTests and the full D3D9 probe pass. Other output,
depth, vPos and fog/alpha branch combinations have assembly-grounded source
implementations but were not compiled or executed in this fixture. The pair
has not been used for draw/readback. Descriptor loading, native system registry,
material execution and the runnable game remain incomplete.
