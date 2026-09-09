# Installed alpha material integration

The existing D3D9 probe now assembles an additional material from the evaluated
installed `shaderfx/common/alphablend.shfx`. Its `Combiners[7]` selects
`dummy.shfx`; installed `dx9_lua.inc` defines RM_MAP as7. The descriptor code
is passed through the reconstructed source generators without replacing its
texture sample, alpha multiplication or clip expression.

`assemble_shader_programs` in shader_lua.cpp shares the recovered normal
base/effect assembly route between debug and alpha materials. It initializes
input/system/interpolator lists, system registry, samplers and the distinct
base/effect flags documented in SHADER_DESCRIPTOR_FLAG_ROUTE.md. Mode,
generation and projected-shadow selection are explicit caller inputs. It is
a projection of00b3c3a0/00b3b3c0, not the cache/material ownership machinery.
Source assembly returns typed programs; compilation and native disassembly
usage filtering remain separate operations.

The alpha PS is compiled first. Its real disassembly drives the recovered
TEXCOORD/COLOR usage parser and selected-interpolator builder, then the final
VS is generated and compiled. The existing sampler state/texture readback
check now uses the actual alpha PS usage mask, replacing the earlier minimal
sampler-use shader. The compiler is still the host D3DCompiler47 adapter;
neither original D3DX bytecode nor native cache compatibility is established.

The isolated draw uses a host triangle and1x1 red texture, explicit identity
world/view-projection transforms, identity vertex decoding, visibility1 and
cAlpha1. These are diagnostic input values, not recovered scene defaults or
game asset loading. Full owner construction, authored mesh decoding, material
parameter loading, scene execution and gameplay remain outstanding.

## Verified outcome

MSVC Win32 build and both existing CTests pass. The complete installed-asset
D3D9 probe reports alpha center ffff0000, outside ff000000 and successful
state restoration. The debug material retains ff407fbf/ff000000. The alpha
helper uses the recovered logical vertex buffer/declaration/layout/draw path,
not DrawPrimitiveUP. It reflects live constant spans, verifies native semantic
mapping and packs cAlpha through the existing material parameter routine.
Unknown live uniforms fail explicitly instead of receiving invented values.

Render and sampler records from both evaluated descriptors are supplied in
order. The fixture explicitly controls raster/depth/fog/filter state and uses
a fully opaque red texel; this does not establish intermediate sRGB accuracy,
alpha blending across arbitrary alpha values, UV sampling across an authored
texture, or compressed mesh decoding. Target0/depth and captured device state
are restored, with the existing probe's single-render-target setup assumed.
No new test executable/framework was introduced.
