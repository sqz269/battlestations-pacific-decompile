# Draw submission recovery

Concrete renderer vtable `00d5f0a8` contains logical vertex-stream binding at
`+134h` (`00b24840`), index binding at `+138h` (`00b24b00`), indexed draw at `+13ch`
(`00b24010`) and non-indexed draw at `+140h` (`00b21b40`). The last three were
missing functions and have been recovered, named and exported. Another missing
target `00b23c50` copies a render target into a surface and forces opaque alpha;
its complete output behavior remains unreviewed.

`draw_primitive_00b21b40` is now reconstructed in `D3D9StateCache`. It takes primitive
type, start vertex and primitive count (thiscall RET Ch), skips when renderer
`+1d90h` is nonzero or device-lost byte `+1d8ah` is set, then skips zero count.
The semantic projection leaves the exact meaning of `+1d90h` unresolved. On the
active path it uses the existing optional guard and calls DrawPrimitive. The native
pre-draw helper `00b1f740` is just LEA EAX,[ECX+10h]; RET, with an unused return.
Its call is omitted because it has no observable effect. The new interface exposes
HRESULT and S_FALSE for skipped calls; native code ignores HRESULT.

`set_stream_frequency_00b24a40` is also reconstructed with four cached frequencies,
initially zero. It changes SetStreamSourceFreq only when a cached value differs,
using the optional guard. New interface invalidation clears those frequencies;
the full native binding-cache reset is still separate work.

## Pixel evidence

The existing D3D9 probe adds a 64x64 offscreen render target and a system-memory
readback surface. It uploads three pretransformed green vertices through the
recovered buffer creation/lock/unlock path. Diagnostic host code supplies FVF,
stream binding, fixed-function texture-stage setup, scene boundaries and readback.
The draw itself goes through `draw_primitive_00b21b40`; cached state setters supply
defaults and diagnostic depth/culling overrides.

GetRenderTargetData followed by LockRect observes `ff00ff00` at (16,16) inside the
triangle and `ff000000` at (60,60) outside. This establishes a bounded upload/draw/
readback path on the real device. It does not establish the game's materials,
logical stream binding, indexed draw gates, scene rendering, visual parity or
gameplay. No additional CTest cases were introduced; build and existing tests pass.
Retained original assembly and byte hashes are under `reports/d3d9_draw_*`.

## Remaining logical stream work

Vertex binding holds intrusive references to logical stream objects. It compares
their COM buffer, stride and offset before SetStreamSource, and can replace a
logical reference without changing API state when those values match. A repeated
identical logical object returns early. Stride comes from a declaration at `+cch`;
those declarations and wrapper methods must be recovered before porting.

Index binding always updates stored base vertex `+17bch`, even when its logical
index object is unchanged. It only calls SetIndices on object changes. Indexed
draw uses that base vertex, skips zero vertex/primitive counts, and has an
additional stream-zero count/tag condition (`40000001h`) that remains unported.
The diagnostic host's direct SetStreamSource is not a replacement for these methods.
