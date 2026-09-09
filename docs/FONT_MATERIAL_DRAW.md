# Installed bilinear font draw

The existing D3D9 diagnostic now draws glyph A from the installed Arial16
descriptor, DAT and atlas using the installed bilinear font shader source.
The full probe passes: **74 nonblack pixels**, all inside the glyph bounds;
observed extent `64,70..73,82`, with no nonblack pixel outside the expected
quad area `63,63..75,88`. Target/depth/state restoration calls succeed.
This is a real-device source-driven draw, not a complete native text context,
original-game render comparison or gameplay validation.

## Connected path

The stock Lua adapter evaluates actual Fonts.lua and finds Arial16, scale
8/9. `FontResources` loads GFX then alpha then DAT, owns the two retained
textures once at font level, and supplies scalar glyphs. Atlas recreation
from retained bytes remains checked before rendering. The selected font
shader name follows the recovered selector, ASCII lowercase, and `.mshd`
substring replacement, yielding `guifontbilinear.shfx`.

The probe evaluates that installed source and its `RM_NORMAL=0` combiner
`dummy.shfx`, then generates shader source with the existing reconstructed
builder. It compiles PS first, parses its disassembly for live interpolators,
generates/compiles VS and reflects actual register spans. The live pixel
sampler mask is 1: both descriptors/material texture slots exist, but the
bilinear shader's active pixel code samples only atlas slot zero.

The new nine-slot material owner feeds the existing sampler binder. Glyph
selection and the x87 quad writer produce four vertices and six indices.
Reconstructed vertex/index allocation, declaration, stream locks, bindings,
constant upload and indexed draw submit them to the device. This integrates
existing pieces without replacing the installed shader body with a synthetic
shader. Logical COM projections remain borrowed; font image owners are kept
alive throughout the draw and no texture recreation occurs while bound.

## Vertex output versus packed interpolators

The first attempt exposed a probe integration error: replacing the full
vertex output list with the live pixel list removed UV1, yet the installed
VS still writes `OUT.UV1`. Compilation correctly failed with invalid subscript.
The core generator already models these lists separately; the fixture now
filters only packed interpolators and keeps all vertex output fields. The same
latent assignment error was corrected in the older alpha fixture.

Fresh native caller assembly corroborates this separation. `00b3b882` takes
builder `+28` for the filtered call at `00b3b910`; mapping is rebuilt at
`00b3ba26`. It then takes builder `+1C` at `00b3ba30` and repopulates it with
null filter arguments at `00b3bab2` before generating VS at `00b3bab9`.
The generator declares and zeroes `sVertexOut` from `+1C`, while packing uses
`+28`. Dropping the original UV assignment or keeping an unused pixel sampler
would not reproduce this behavior.

## Explicit diagnostic inputs

The host provides a 256x256 nonmultisampled target and matrix mapping normalized
GUI x/960,y/720 to target pixels. Glyph origin is 64,64, width scale and vertical
scale are 1, and height comes from the decoded font. Color is white, visibility
1, overbright 0, alpha scale 1 and clipping disabled. Vertex decode scale/offset
are identity. These are controlled inputs, not a recovered GUI camera or full
system-constant gatherer. Unknown reflected constants cause failure.

The resolver now uses the recovered startup texture/shader lists and ordered
VFS candidate logic with the first two physical startup mounts and a supplied
installation root (`PROVIDER_FACTORY_STARTUP.md`). This resolves
`Fonts/white.tga` to `effects/white.dds` and the two shader basenames under
`shaderfx/gui` and `shaderfx/lights` without per-filename mappings. Native
provider teardown, mounted FileStore population and archive loading remain unported. The successful
draw does not prove the original game's current resource selection.
See `VFS_MOUNT_LOOKUP.md` and `reports/vfs_font_draw_probe.txt`.

Bounds checking proves nonempty bounded output, not pixel-for-pixel glyph
shape/antialiasing equivalence with the original game. The nonbilinear branch,
alpha texture multiplication, clipping, wrapping/alignment, optional child UI,
native batching/cache lifetime and font reload are not validated here.

MSVC Win32 build and both existing CTests pass. This adds one font draw to the
existing D3D9 probe, with no new test target or suite. Full output is in
`reports/font_material_draw_probe.txt`; code/asset identity evidence is in
`reports/font_material_resource_audit.json`.
