# Generated debug shader draw

Registry follow-up: the draw now uses the recovered52-record registry with
annotation cutoff77, identity matrix at c15 and float4 cElapsedTime at c34.
The pixel profile is ps_3_0; the original diagnostic assignments described below
are historical. Pixel results remain unchanged. See `SHADER_SYSTEM_REGISTRY.md`.

The existing D3D9 shader probe now draws through the full reconstructed vertex
and pixel generators using the installed debug/dummy statements. This closes
the earlier compilation/binding-only boundary for one diagnostic fixture.

The fixture creates a private 64x64 A8R8G8B8 render target, disables depth,
blending, alpha test, fog, culling, scissor and sRGB output, and uploads an
identity view/projection plus zeroed remaining constant registers through
`00b21820`/`00b218c0`. The projected pixel registry places cElapsedTime at c15/c16;
zero values disable the normal-mode conditional color transform. These slots
are consequences of the fixture's explicit registry, not recovered native slots.

A three-vertex triangle uses FLOAT4/POSITION and FLOAT4/COLOR. Its uniform input
color is (0.25,0.5,0.75,1). Logical stream recreation, lock/upload/unlock, binding,
hardware declaration creation/binding and `00b21b40` draw feed the generated
pair. An identity transform keeps the triangle in clip space. Readback observed:

| Sample | Observed ARGB | Required |
|---|---|---|
| Center (32,32) | FF407FBF | RGB within one quantization step, alpha255 |
| Outside (2,2) | FF000000 | Unchanged clear pixel |

This checks the final SYS-to-OUT position assignment, COLOR pack/unpack, debug
DiffuseColor assignment, dummy FinalColor assignment and output routing together
with the recovered renderer paths. It is not a comparison with the original
game's rendering or native shader bytecode.

The probe captures a D3DSBT_ALL state block and the target/depth surfaces before
the draw and reapplies them afterward; all restoration calls succeeded. Subsequent
shader unbinding, material and installed-atlas probes also passed. Win32 Release
build and both existing CTests pass. No new test target or framework was added.

Descriptor parsing, system constant registry reconstruction, material selection,
other shader branches and the runnable game remain incomplete. This fixture
uses explicit typed descriptor/field/constant projections and no game scene.
