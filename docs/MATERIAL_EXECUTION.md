# Material execution to native drawing

`00b44750` receives a selected pass in ECX, an entry and optional override on
the stack, and returns RET8. The old pseudocode omits the second argument.
It skips sections with zero stream count (entry+4 -> section+4Ch) or a first
stream whose virtual+28h returns -1. Streams occupy section+3Ch onward.

Stream binding uses renderer virtual+134h (`00b24840`). Section+1Ch is the
instancing count/tag input, returned verbatim by `00b855a0`. If zero, streams
are bound and frequency slots 0 and 1 are reset to one. Otherwise each stream's
+54h value is ORed with that count, except exactly 80000000h becomes 80000001h.
Frequencies go through the reconstructed `00b24a40` cache setter.

The helper compares pass+14h against global 0108fbf4 and only updates its
plane-related renderer state when that owner changes. The optional calculation
uses owner+13Ch, owner+140h indexed by entry+10h -> +198h, transforms and camera
data; its precise plane interpretation remains unported. This cache must not
be mistaken for a complete pass cache.

The concrete renderer vtable is 00d5f0a8. Original-file/saved-image bytes confirm:

| Slot | Target | Established behavior |
|---|---|---|
| +E0h | 00b23f20 | Vertex declaration binding, device virtual+15Ch |
| +130h | 00b24710 | Texture binding, device virtual+104h |
| +134h | 00b24840 | Existing logical vertex-stream binding |
| +138h | 00b24b00 | Existing logical index binding and base vertex |
| +13Ch | 00b24010 | Existing indexed draw |
| +140h | 00b21b40 | Existing non-indexed draw |

Declaration input comes from section+50h. Index input comes from entry+8h ->
+60h. Getter `00b48de0` reads index+20h (base index), while `00b48d50` reads
first stream+70h (base vertex). Indexed dispatch requires both a non-null index
object and section+58h nonzero (`00b855f0` returns that byte).

The final call is `00b43410(pass, entry, override, indexed, index_object,
base_index, base_vertex)` with pass in ECX and six stack arguments (RET18h).
Its index-object argument is carried by this caller but not read by this body.
It performs the following work before dispatching the existing draw methods:

1. Bind pass+18h render-state block and pass+20h sampler-state block through
   00b27a80 and 00b27b90. Material+104h, obtained by 00b17320, overrides render
   state 39h only when signed greater than -1.
2. Bind pass+54h vertex shader (00b21d10) and pass+58h pixel shader (00b21c20).
3. Resolve 8-byte texture descriptors at pass+24h/count+28h. Negative indices
   use owner lookup 00b17d90 with -1-index; nonnegative indices select material
   inline slots +10h onward under signed 16-bit count+34h. The descriptor's
   byte+4 selects a vertex slot beginning at 16 versus a pixel slot beginning
   at zero. Skipped pixel descriptors still increment the pixel slot. Mask
   testing uses x86 SHL with index modulo 32, not an unbounded C++ shift.
4. Call 00b42350(entry, override), which populates shared shader constants and
   performs additional texture/callback work. This substantial helper remains
   unported; its pseudocode has incomplete arguments and overlapping globals.
5. Upload positive constant-register tails through 00b21820 and 00b218c0, from
   0108ebf4 and 0108dbec respectively, starting at global 00e13078. Tail limits
   are bytes at pass+70h/+74h -> +74h.
6. Invoke a non-null material+8h callback, then draw and update statistics.

Indexed drawing passes section+8h primitive type, +Ch minimum vertex, +10h
vertex count, +14h plus base index as start index, and +18h primitive count.
Non-indexed drawing passes primitive type, section+Ch plus base vertex and
primitive count. The already reconstructed index binder supplies base vertex
to the indexed renderer draw. Thus this is an actual material-to-draw route,
not merely a registry or shader-loader guess.

# Bounded dependencies ready for reconstruction

The two state-block setters are small: pointer identity skips all work;
replacement retains the new intrusive reference before releasing the old;
null removes the retained block without resetting individual API states.
Render blocks contain 8-byte state/value pairs at +8h with signed count+Ch;
sampler blocks contain 12-byte sampler/state/value triples. Each forwards to
the existing cached setters and increments its block-change counter once.

The shortest remaining shader upload wrappers are 00b21820 (vertex) and
00b218c0 (pixel): thiscall RETCh, arguments start register/data/vector count.
Zero count skips the optional guard and API call. Otherwise they call device
virtual+178h / +1B4h (SetVertexShaderConstantF / SetPixelShaderConstantF),
increment call count and add count*16 to byte statistics. Native ignores HRESULT.

Shader bind wrappers 00b21d10 (vertex) and 00b21c20 (pixel) are thiscall RET4.
They enter the optional guard before comparison. Cached logical shader pointers
are renderer+1770h / +176Ch; COM shader is logical shader+8h. If both objects
are non-null and their COM pointers equal, they skip binding and do not replace
the cached logical pointer. Null-to-null also skips binding. Other transitions
store the logical pointer, call device+170h / +1ACh and increment the shader
counter. There are no retain/release calls in these bodies: a typed port must
make lifetime requirements explicit instead of inventing native ownership.

Two indirect targets were missing Ghidra functions. They were inspected using
Capstone on saved bytes without changing analysis: 00b23f20 and 00b24710.
Declaration binding caches logical identity, retains/releases it, and calls
SetVertexDeclaration only for non-null replacements; null does not unbind the
device. Texture binding retains/releases logical identity, remaps logical
slots >=16 by adding F1h, obtains the COM texture via virtual+1Ch, and calls
SetTexture including null unbinding. Both should be annotated before a port.

Evidence is in `reports/material_execution_evidence.json` with assembly exports
under ignored `exports/material_execution/`. No Ghidra functions were created,
renamed or saved during this investigation. No C++ changed and no runtime,
native ABI compatibility or gameplay validation is claimed. The full constant
builder, plane state, callback contracts and resource ownership still prevent
a faithful complete material executor.

Integration follow-up: the two float constant upload wrappers are now ported in
D3D9StateCache and checked by real-device register readback, zero-count skipping
and balanced optional locking. The material executor itself remains unported.
Confirmed pass helpers and upload wrappers were named/commented and saved in
Ghidra; affected exports were refreshed.
