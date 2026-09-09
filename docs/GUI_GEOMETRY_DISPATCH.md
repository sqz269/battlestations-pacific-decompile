# Stateful GUI geometry dispatch

The stateful texture route now has a concrete class-level dispatch:
`00ab44b0 -> 00ab2690 -> vtable+80h -> 00ab3cb0 -> vtable+7Ch -> 00ab1860`.
This establishes one GUI class and inherited implementations, not every GUI
object that can call the state setter.

Constructor `00ab5c60` calls base constructor 00aa9390 with type argument 6,
then stores vtable 00d5c4c0 at 00ab5c91. It initializes current signed16 state
at +ECh to -1, vector +F4h/+F8h/+FCh to null, HasTexture +100h to one,
DelayedTextureLoad +101h to zero, PartialDisplayType +110h to zero and
PartialDisplayRatio +114h to one. The names come from adjacent serializer
00ab2b70's field strings. Copy constructor 00ab5d30 installs the same table,
copies state and invokes +80h. Table +80h is 00ab3cb0; +7Ch is 00ab1860.
Two additional tables, 00d5bce0 and 00d5cc18, share those two targets, but their
complete derived-class behavior is outside this investigation.

`00ab3cb0` is ECX=this plus one state argument, RET4. It stores the supplied
low 16 bits into current state, obtains geometry through 00b74640(0), and
creates a logical stream when absent using `SimpleColor.mvfm` and renderer
virtual+5Ch with arguments 4,1,declaration. It requests a four-vertex lock
through stream virtual+10h. Section creation/reuse selects GUI material names
`guifade.mshd`, `guidefault.mshd`, `guidefault_point.mshd`, or an override.
The section receives primitive type 5, first/minimum vertex zero, four vertices,
start index zero and two primitives (stores 00ab4126..00ab413c). Type 5 is the
triangle-strip value already consumed by the reconstructed renderer.

When textured, it assigns state texture to material slot zero, reads state
UVs +0Ch..18h, size +34h/+38h and pivot +2Ch/+30h. Otherwise its UVs are
0,0,1,1 and it uses object size. Unless object+74h suppresses the writes,
it invokes +7Ch with stream, UV float4, four crop floats from object+118h..124h,
and size float2. It subsequently writes white color to all four vertices,
using either a packed DWORD or declaration-selected float color components.
It unlocks, binds material via 00b864c0, connects the section to geometry via
00b865a0 and conditionally appends it through 00b73c60. This prepares retained
geometry; the function itself does not issue a D3D draw.

# Smallest geometry writer

`00ab1860` was a missing function reached by the confirmed +7Ch slot. It is
now created as FUN_00ab1860, with no descriptive rename or project save in this
investigation. Its ABI is ECX=this and seven stack arguments (RET1Ch): logical
stream, UV float4, crop left/top/right/bottom by value, and size float2.
Its assembly uses stream+8h mapped bytes, +Ch vertex stride, +10h position
offset and +28h UV offset. It writes float3 positions and float2 UVs. There
is no packed 16-bit or half-float UV conversion in this writer, and no fixed
vertex stride should be inferred from the material name.

Define `uL=u0+cropL*(u1-u0)`, `uR=u0+cropR*(u1-u0)`, and equivalently `vT/vB`.
For PartialDisplayType zero the output array is:

| Vertex | Position | UV |
|---|---|---|
| 0 | (0, 0, 0) | (uL, vT) |
| 1 | (width, 0, 0) | (uR, vT) |
| 2 | (0, yScale*height, 0) | (uL, vB) |
| 3 | (width, yScale*height, 0) | (uR, vB) |

Writes execute in order 0,1,3,2; the final array order above forms the strip.
`yScale` is the float at 00e12fd0, verified as 0.75 in both saved-image and
disk bytes. No writer xrefs were returned, but its runtime mutability and
meaning are not established; do not silently turn it into a viewport rule.
The writer contains no index-buffer writes or six-index triangle-list builder.

PartialDisplayRatio is clamped with COMISS/JBE to [0,1] for ordered values,
stored back to object+114h, and used by modes 1..4 to crop geometry and UVs
from opposite vertical or horizontal sides. Unordered comparisons leave NaN
unchanged. An unsigned mode above four takes the default return without
position/UV writes, after the ratio update. Exact x87 evaluation/spill order
must be preserved or bounded explicitly in a numerical port; mode-zero's
0,1,3,2 writes were checked against assembly, not only pseudocode.

This writer is the nearest bounded reconstruction target. It can accept typed
crop/size/UV fields and a four-vertex output projection while leaving stream
allocation, declaration selection, section lifetime and full GUI state updates
unported. A full 00ab3cb0 port still needs those dependencies and its material
parameters. No universal GUI renderer, native ABI replacement, pixel result or
game validation is claimed.

`reports/gui_geometry_dispatch_evidence.json` records original/saved-byte parity,
the one permitted function creation, and proposed evidence-based names for
parent integration. Raw exports remain in ignored `exports/gui_geometry/`.

# Typed reconstruction

`gui_write_cropped_quad_00ab1860` now reconstructs all five modes in
`include/bsp/gui_geometry.hpp` and `src/gui_geometry.cpp`. It takes explicit
UV bounds, crop bounds, dimensions, Y scale, mode and mutable ratio; it emits
four typed position/UV vertices in triangle-strip order. Mode zero emits the
full rectangle. Modes 1/2 retain the top/bottom fraction; modes 3/4 retain the
left/right fraction, moving the corresponding geometry and UV edge together.
Ratio clamps preserve NaN and signed zero. Invalid unsigned modes return false
after updating ratio and leave the supplied vertex array unchanged.

This is a bounded semantic numerical port. It uses float32 intermediate
expressions under the project's strict floating-point compilation. Native
x87 extended intermediates and distinct spill sequences can produce different
last bits, and exceptional inputs can expose different arithmetic/status
behavior. No bitwise, floating-point-exception or native differential parity
is claimed. Invalid-mode arithmetic is skipped in the typed interface after
clamping, whereas native code evaluates some UV arithmetic before returning.
The default Y scale records the saved/disk 0.75 value and remains an explicit
caller-controlled parameter. Stream packing, allocation, color fields,
materials and the stateful object's lifetime remain outside this function.

Integration follow-up: proposed function names and evidence comments were saved
in Ghidra, and affected exports refreshed. The geometry writer now has a typed float32 projection; construction and
full state rebuild remain unported.

Validation: the existing Win32 probe now consumes mode0 typed vertices, projects
logical coordinates with an explicit host scale and renders the installed atlas.
Its BMP hash is unchanged from the previous diagnostic corners. All five modes
are represented in source, but only mode0 is draw-validated. The planned isolated
native comparison was not completed when parallel agents stopped on a usage
limit. Numerical native equivalence and modes1..4 validation remain next work;
the ledger counts this as a fragment. Build and2existingCTest checks pass.
