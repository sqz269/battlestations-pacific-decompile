#pragma once

// Renderer debug draws, debug-record appends, cube-texture creation, the 44h-slot
// hardware-layout slab pool and two material/vertex-stream helpers from segments
// 84/85, reconstructed by cc13-render5. Names are hypotheses, not recovered
// symbols. Evidence, original ABIs and open questions are in
// docs/CC13_RENDER_SPRITES_AND_SHADOW_PASSES.md. None of these is a drop-in binary
// replacement: the native bodies are thiscall on the D3D9 renderer (vtable
// 00D5F0A8, singleton [00F8D394]) and reach the device through renderer+1A10h.
// Device, cached-state and camera calls stay host calls; the vertex data, state
// sequences, record layouts and flag decoding are reproduced here.
//
// The 2D sprite batcher (00B2B580), the 3D debug line/sphere consumers (00B2B270,
// 00B28D00, 00B2BB90) and the record-40 append (00B29330) were already
// reconstructed by cc12 (src/native_renderer_debug_records*.cpp,
// src/native_renderer_debug_lines.cpp); they are not repeated.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_render_sprites {

struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };

// D3D9 values the sequences use (d3d9types.h numbering; the cached setters pass
// them through unchanged).
enum : std::uint32_t {
    kRsZEnable = 7, kRsFillMode = 8, kRsZWriteEnable = 14, kRsAlphaTestEnable = 15,
    kRsCullMode = 22, kRsAlphaBlendEnable = 27, kRsFogEnable = 28,
    kRsSpecularEnable = 29, kRsStencilEnable = 52, kRsTextureFactor = 60,
    kRsLighting = 137, kRsColorWriteEnable = 168,
    kTssColorOp = 1, kTssColorArg1 = 2, kTssAlphaOp = 4, kTssAlphaArg1 = 5,
    kTssTexCoordIndex = 11,
    kSampAddressU = 1, kSampAddressV = 2,
    kTsPointerWorld = 0x100, kTsView = 2, kTsProjection = 3,
    kFvfXyz = 0x002, kFvfXyzRhwTex1 = 0x104,
    kPrimLineList = 2, kPrimTriangleStrip = 5,
    kFmtIndex16 = 0x65, kFmtIndex32 = 0x66,
};

// Renderer, device and camera services the debug draws call. Each method names
// the native callee or vtable slot.
class DebugDrawHost {
public:
    virtual ~DebugDrawHost() = default;
    virtual void set_render_state(std::uint32_t state, std::uint32_t value) = 0;   // 00B24460
    virtual void set_texture_stage_state(std::uint32_t stage, std::uint32_t type,
        std::uint32_t value) = 0;                                                   // 00B24510
    virtual void set_sampler_state(std::uint32_t sampler, std::uint32_t type,
        std::uint32_t value) = 0;                                                   // 00B24610
    virtual void bind_vertex_shader(void* shader) = 0;                             // 00B21D10
    virtual void bind_pixel_shader(void* shader) = 0;                              // 00B21C20
    // 00B26C60 clears the bound shaders inline instead of calling the binders:
    // under the optional guard, renderer+1770h := 0 and, when it was non-null,
    // device+170h SetVertexShader(NULL) and ++renderer+1BC0h; then the same for
    // renderer+176Ch / device+1ACh SetPixelShader(NULL) / renderer+1BBCh.
    virtual void clear_bound_shaders_inline() = 0;
    virtual void device_set_vertex_declaration(void* declaration) = 0;             // device+15Ch
    virtual void device_set_fvf(std::uint32_t fvf) = 0;                            // device+164h
    virtual void device_set_transform(std::uint32_t state, const float* matrix16) = 0; // device+B0h
    virtual const float* camera_view_matrix(void* camera) = 0;                     // 00B6FCB0
    virtual const float* camera_projection_matrix(void* camera) = 0;               // 00B6FCF0
    virtual void bind_texture(std::uint32_t stage, void* texture) = 0;             // renderer vt+130h
    virtual void bind_vertex_stream(void* stream, std::uint32_t index) = 0;        // renderer vt+134h
    virtual void bind_index_stream(void* stream, std::uint32_t index) = 0;         // renderer vt+138h
    // DWORD renderer+1D90h != 0 or byte renderer+1D8Ah != 0 skips only the draw.
    virtual bool draws_suppressed() = 0;
    virtual void device_draw_primitive_up(std::uint32_t type, std::uint32_t count,
        const void* vertices, std::uint32_t stride) = 0;                           // device+14Ch
    virtual void device_draw_indexed_primitive_up(std::uint32_t type,
        std::uint32_t min_vertex, std::uint32_t vertex_count, std::uint32_t primitive_count,
        const void* indices, std::uint32_t index_format, const void* vertices,
        std::uint32_t stride) = 0;                                                  // device+150h
    virtual void output_debug_string(const char* text) = 0;                        // [00CE2274] OutputDebugStringA
    // 00B33AD0 / 00B33B00, only when byte [0108D6DC] is set.
    virtual void enter_optional_guard() = 0;
    virtual void leave_optional_guard() = 0;
};

// 00CD7A80 static initializer of the eight-vertex box at 0108D4D0 (stride 0Ch):
// x, y in {-0.995, +0.995}, near z = 0 (zero-filled .bss, never written), far
// z = 0.995. With world = an inverse view-projection this is a frustum outline in
// D3D clip space (z in [0,1]); 00B26A10 draws it with the 24 indices at 00E13038.
void build_debug_frustum_box(Vec3 out[8]);
extern const std::uint16_t kDebugFrustumBoxLineIndices[24];

// Renderer vt+D0h 00B26A10, ECX renderer, stack (world matrix, camera), RET 8.
// Magenta (texture factor FFF00FFFh) line list of the box above.
void draw_debug_frustum_00b26a10(DebugDrawHost& host, const float* world16, void* camera);

// 00B26C60 vertex build: for each of six planes {nx, ny, nz, d} a line from d*n
// to (d + 400.0)*n (the 400 is the double at 00CE3D90, rounded to float before
// the multiply). Twelve vertices, line i at out[2i], out[2i+1].
void build_debug_plane_normal_lines(const float planes[6][4], Vec3 out[12]);

// Renderer vt+CCh 00B26C60, ECX renderer, stack (planes[6][4], camera), RET 8.
// Red (texture factor 00FF0000h) line list, identity world.
void draw_debug_plane_normals_00b26c60(DebugDrawHost& host, const float planes[6][4],
    void* camera);

// XYZRHW|TEX1 quad, stride 18h, triangle-strip order: (0,0) (s,0) (0,s) (s,s),
// z = rhw = 0.5, uv = the corner in [0,1].
struct ScreenQuadVertex { float x, y, z, rhw, u, v; };
void build_debug_texture_quad(float size, ScreenQuadVertex out[4]);

// Renderer vt+D8h 00B27440, ECX renderer, stack texture, RET 4. 320-pixel quad
// (float 320.0 at 00D5E614) bracketed by OutputDebugStringA("Texture db {") /
// ("}") and the optional renderer guard.
void draw_debug_texture_quad_00b27440(DebugDrawHost& host, void* texture);

// Renderer vt+DCh 00B27780, ECX renderer, stack (texture, unused), RET 8. Same
// quad at 512 pixels (00CE3C88), without the debug strings or the guard. Both
// quad draws unbind stage 0 twice: before and after the state tail.
void draw_debug_texture_quad_00b27780(DebugDrawHost& host, void* texture);

// The state tail every debug draw above ends with (shared inline sequence).
void restore_debug_draw_states(DebugDrawHost& host);

// Growable record array {data, count, capacity} as the renderer embeds them.
// reserve receives max(capacity*2, 1) when count == capacity, as in the native
// push paths (00B22940/00B229B0 family).
template <typename Record>
struct RecordArray {
    Record* data;
    std::int32_t count;
    std::int32_t capacity;
};

// 10h-byte world-space debug line endpoint (renderer+1CF4h array).
struct DebugLineRecord16 { float x, y, z; std::uint32_t color; };
// 14h-byte screen-space endpoint (renderer+1D00h array, consumed by 00B28D00).
struct DebugLineRecord20 { float x, y, z, w; std::uint32_t color; };

using Reserve16 = void (*)(RecordArray<DebugLineRecord16>&, std::int32_t);
using Reserve20 = void (*)(RecordArray<DebugLineRecord20>&, std::int32_t);

// Renderer vt+B4h 00B290D0, ECX renderer, stack (a*, b*, color_a, color_b),
// RET 10h. Two endpoint records under the renderer record guard (00B25BE0 lock).
// FoliageEntryNode_DrawBox (00AE0A50) calls it twelve times per box.
void append_debug_line_00b290d0(RecordArray<DebugLineRecord16>& lines, Reserve16 reserve,
    const Vec3& a, const Vec3& b, std::uint32_t color_a, std::uint32_t color_b);

// Coordinates 00B28AC0 writes for one 2D endpoint. aspect_scale is 0.75 when
// byte [[0109CF04]+0Dh] is set, else 1.0 (00D7A24C). pixel_space is byte
// renderer+1D88h; viewport_w/h come from renderer vt+A8h -> 00B1F740.
DebugLineRecord20 debug_line_2d_endpoint(const Vec2& p, std::uint32_t color, bool pixel_space,
    float aspect_scale, std::int32_t viewport_w, std::int32_t viewport_h);

// Renderer vt+C0h 00B28AC0, ECX renderer, stack (a*, b*, color_a, color_b),
// RET 10h. Appends both endpoints through 00B256F0 under the record guard.
void append_debug_line_2d_00b28ac0(RecordArray<DebugLineRecord20>& lines, Reserve20 reserve,
    const Vec2& a, const Vec2& b, std::uint32_t color_a, std::uint32_t color_b,
    bool pixel_space, float aspect_scale, std::int32_t viewport_w, std::int32_t viewport_h);

// Runtime texture creation flags (shared with 00B2A070): low nibble pool,
// bit 10h render target, F00h usage selector, F000h = 1000h dynamic,
// FF000000h = 01000000h autogen-mipmap.
struct TextureUsage {
    bool pool_set;            // false: the native pool slot keeps its stale stack value
    std::uint32_t pool;
    std::uint32_t usage;
};
TextureUsage decode_runtime_texture_flags(std::uint32_t flags);

// 00B2A380 retry predicate: pool DEFAULT (0), a failing HRESULT that is neither
// D3DERR_OUTOFVIDEOMEMORY (8876017Ch) nor E_OUTOFMEMORY (8007000Eh) recreates the
// device (00B29670) and calls CreateCubeTexture once more.
bool cube_texture_create_should_retry(std::uint32_t pool, std::int32_t hr);

// 44h-byte slot slab pool (pool object [0108FD70], used for hardware vertex
// layouts). Raw layouts by offset; see the doc. Slab: 32 slots of 44h with the
// slab index in each slot's last DWORD (+40h), free-index stack u16[32] at +880h,
// free count u16 at +8C0h; 8C4h bytes. Pool: CRITICAL_SECTION +0Ch, depth +24h,
// slab pointer vector {+28h data, +2Ch count, +30h capacity}, first slab with a
// free slot +34h (-1: none).
inline constexpr std::size_t kLayoutSlotSize = 0x44;
inline constexpr std::size_t kLayoutSlabSize = 0x8C4;

// 00B47750, ECX slab, stack slab index, RET 4.
void initialize_layout_slab_00b47750(std::uint8_t* slab, std::uint32_t slab_index);

class SlabPoolHost {
public:
    virtual ~SlabPoolHost() = default;
    virtual void enter(void* critical_section) = 0;
    virtual void leave(void* critical_section) = 0;
    virtual void* operator_new(std::size_t bytes) = 0;   // 00BF6A5F
    virtual void free(void* block) = 0;                  // _free
};

// 00B48780, ECX pool, EAX slot, plain RET. Its only reference is the unreferenced
// stub 00B488D0 (ECX = 0108FD70); live layouts come from 00B606F0 (pool 0108FE9C).
void* allocate_layout_slot_00b48780(std::uint8_t* pool, SlabPoolHost& host);
// 00B479C0, ECX pool, stack slot, RET 4.
void return_layout_slot_00b479c0(std::uint8_t* pool, void* slot, SlabPoolHost& host);

// 00B44C00, ECX material section, plain RET. When byte +80h is set, for each of
// the count +6Ch bindings at +5Ch[i]: resolve the binding's name (+8) through
// 00B1BC70, substitute "Default" when the result is empty, and pass it to
// (binding+4)->vslot 1Ch.
class GeneratorModeHost {
public:
    virtual ~GeneratorModeHost() = default;
    // 00B1BC70; returns the resolved text, empty for none.
    virtual const char* resolve_string_override(const void* name_string) = 0;
    virtual void select_mode(void* generator, const char* mode) = 0;   // generator vt+1Ch
};
void apply_instance_generator_modes_00b44c00(std::uint8_t* section, GeneratorModeHost& host);

} // namespace bsp::cc13_render_sprites
