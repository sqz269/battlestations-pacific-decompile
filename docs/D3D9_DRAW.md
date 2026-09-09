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

## Logical streams and indexed drawing

Vertex binding holds intrusive references to logical stream objects. It compares
their COM buffer, stride and offset before SetStreamSource, and can replace a
logical reference without changing API state when those values match. A repeated
identical logical object returns early. Stride comes from a declaration at `+cch`;
the declaration itself remains a stride-only projection in the new interface.

Index binding always updates stored base vertex `+17bch`, even when its logical
index object is unchanged. It only calls SetIndices on object changes. Indexed
draw uses that base vertex, skips zero vertex/primitive counts, and has an
additional stream-zero count/tag condition (`40000001h`). These three routines
are now reconstructed with typed logical stream projections and shared ownership.
The probe uses the recovered binding methods instead of direct SetStreamSource.

Factory `00b287c0` calls vertex constructor `00b4bc00`, installing vtable `00d61d6c`.
Recovered getters read vertex count at `+64h`, declaration at `+68h`, offset at
`+5ch`, and obtain the COM buffer through physical wrapper `+58h` virtual `+1ch`.
The physical getter returns its `+28h` COM pointer. Index constructor `00b4bf30`
installs vtable `00d61de0`; its getter delegates through physical wrapper `+8h`.
Full constructors, shared-buffer allocation, declaration formats and resource
registry are still unported. Getter evidence supports the projections, not complete
native object layout or ABI equivalence.

The typed cache retains logical streams with shared_ptr rather than copying native
intrusive objects. Logical streams retain physical bindings; the new physical
binding destructor releases any remaining COM reference. Explicit reset release
still preserves metadata. This is new interface ownership, not the complete native
wrapper teardown. Invalidation releases logical references but requires the caller
to coordinate actual device unbinding/reset. Same-object binding deliberately returns
early even if its fields have changed; equivalent distinct objects transfer ownership
without API rebinding. The implementation preserves both native behaviors.

Indexed drawing checks inhibit/lost flags, then enters the optional guard before
checking counts. An oversized vertex count is rejected only when stream zero is
present and has tag `40000001h`. It calls DrawIndexedPrimitive with the base vertex
stored by index binding. As with non-indexed drawing, the new interface exposes
HRESULT or S_FALSE; native code ignores API errors.

The probe preserves the non-indexed pixel check and adds indexed readback. Three
equivalent vertex bindings generate only one SetStreamSource; rebinding the same
index object with base vertex 1 generates only one SetIndices while selecting the
correct triangle after a dummy vertex. A request exceeding the tagged stream count
returns S_FALSE. The valid indexed draw again yields green inside and black outside.
The limits of this check are recorded separately in `reports/d3d9_indexed_validation.json`.
No game-material, full stream-constructor, registry, concurrent or gameplay claim follows.
