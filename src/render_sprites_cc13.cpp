// Segment 84/85 renderer debug draws and helpers, reconstructed by cc13-render5.
// Evidence and ABIs: docs/CC13_RENDER_SPRITES_AND_SHADOW_PASSES.md.

#include "bsp/render_sprites_cc13.hpp"

#include <cstring>

namespace bsp::cc13_render_sprites {

namespace {

constexpr float kBoxExtent = 0.995f;          // 00E13068 (3F7EB852h)
constexpr double kPlaneNormalLength = 400.0;  // 00CE3D90
constexpr float kQuadSize320 = 320.0f;        // 00D5E614
constexpr float kQuadSize512 = 512.0f;        // 00CE3C88

template <typename T>
T& field(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

void set_camera_transforms(DebugDrawHost& host, const float* world16, void* camera) {
    host.device_set_transform(kTsPointerWorld, world16);
    host.device_set_transform(kTsView, host.camera_view_matrix(camera));
    host.device_set_transform(kTsProjection, host.camera_projection_matrix(camera));
    host.bind_vertex_stream(nullptr, 0);
    host.bind_index_stream(nullptr, 0);
}

// 00B27440/00B27780 shared setup up to the draw: stage 0 = texture colour,
// stage 1 off, depth/stencil/fog off, solid fill, all channels written, clamped
// pre-transformed quad.
void draw_texture_quad(DebugDrawHost& host, void* texture, float size) {
    host.bind_pixel_shader(nullptr);
    host.bind_vertex_shader(nullptr);
    host.bind_texture(0, texture);
    host.set_texture_stage_state(0, kTssColorArg1, 2);   // D3DTA_TEXTURE
    host.set_texture_stage_state(0, kTssColorOp, 2);     // D3DTOP_SELECTARG1
    host.set_texture_stage_state(1, kTssColorOp, 1);     // D3DTOP_DISABLE
    host.set_texture_stage_state(1, kTssAlphaOp, 1);
    host.set_render_state(kRsFogEnable, 0);
    host.set_render_state(kRsStencilEnable, 0);
    host.set_render_state(kRsFillMode, 3);                // D3DFILL_SOLID
    host.set_render_state(kRsColorWriteEnable, 0xF);
    ScreenQuadVertex quad[4];
    build_debug_texture_quad(size, quad);
    host.device_set_vertex_declaration(nullptr);
    host.device_set_fvf(kFvfXyzRhwTex1);
    host.set_sampler_state(0, kSampAddressU, 3);          // D3DTADDRESS_CLAMP
    host.set_sampler_state(0, kSampAddressV, 3);
    host.set_texture_stage_state(0, kTssTexCoordIndex, 0);
    if (!host.draws_suppressed())
        host.device_draw_primitive_up(kPrimTriangleStrip, 2, quad, sizeof(ScreenQuadVertex));
    host.bind_texture(0, nullptr);
    restore_debug_draw_states(host);
    host.bind_texture(0, nullptr);
}

} // namespace

const std::uint16_t kDebugFrustumBoxLineIndices[24] = {
    0, 1, 1, 3, 3, 2, 2, 0,   // near face
    4, 5, 5, 7, 7, 6, 6, 4,   // far face
    0, 4, 1, 5, 3, 7, 2, 6,   // connecting edges
};

void build_debug_frustum_box(Vec3 out[8]) {
    // 00CD7A80: xmm1 = -0.0 (00D7A208) - 0.995, xmm0 = 0.995. The z of vertices
    // 0..3 is never stored and stays .bss zero.
    const float lo = -0.0f - kBoxExtent;
    const float hi = kBoxExtent;
    for (int i = 0; i < 8; ++i) {
        out[i].x = (i & 1) ? hi : lo;
        out[i].y = (i & 2) ? hi : lo;
        out[i].z = (i & 4) ? hi : 0.0f;
    }
}

void restore_debug_draw_states(DebugDrawHost& host) {
    // Literal order of the shared tail (00B26B6F.., 00B2730F.., 00B2764B..,
    // 00B27935..). FILLMODE is written as 2 (D3DFILL_WIREFRAME) twice.
    host.set_render_state(kRsFillMode, 2);
    host.set_render_state(kRsZEnable, 1);
    host.set_render_state(kRsAlphaBlendEnable, 0);
    host.set_render_state(kRsLighting, 0);
    host.set_render_state(kRsCullMode, 1);                // D3DCULL_NONE
    host.set_render_state(kRsAlphaBlendEnable, 0);
    host.set_render_state(kRsAlphaTestEnable, 0);
    host.set_render_state(kRsFogEnable, 0);
    host.set_render_state(kRsSpecularEnable, 0);
    host.set_render_state(kRsAlphaTestEnable, 0);
    host.set_render_state(kRsAlphaBlendEnable, 0);
    host.set_render_state(kRsFillMode, 2);
    host.set_texture_stage_state(0, kTssColorArg1, 3);   // D3DTA_TFACTOR
    host.set_texture_stage_state(0, kTssColorOp, 2);
    host.set_texture_stage_state(0, kTssAlphaArg1, 3);
    host.set_texture_stage_state(0, kTssAlphaOp, 2);
    host.set_texture_stage_state(1, kTssColorOp, 1);
    host.set_texture_stage_state(1, kTssAlphaOp, 1);
    host.set_render_state(kRsTextureFactor, 0xFFFFFFFFu);
}

void draw_debug_frustum_00b26a10(DebugDrawHost& host, const float* world16, void* camera) {
    host.set_render_state(kRsFogEnable, 0);
    host.set_render_state(kRsStencilEnable, 0);
    host.set_render_state(kRsFillMode, 3);
    host.set_render_state(kRsAlphaBlendEnable, 0);
    host.set_render_state(kRsZEnable, 1);
    host.set_render_state(kRsZWriteEnable, 1);
    host.set_render_state(kRsTextureFactor, 0xFFF00FFFu);
    host.bind_vertex_shader(nullptr);
    host.bind_pixel_shader(nullptr);
    host.device_set_vertex_declaration(nullptr);
    host.device_set_fvf(kFvfXyz);
    host.set_texture_stage_state(0, kTssColorArg1, 3);
    host.set_texture_stage_state(0, kTssColorOp, 2);
    set_camera_transforms(host, world16, camera);
    if (!host.draws_suppressed()) {
        // The native draws straight from the static 0108D4D0 box.
        Vec3 box[8];
        build_debug_frustum_box(box);
        host.device_draw_indexed_primitive_up(kPrimLineList, 0, 8, 12,
            kDebugFrustumBoxLineIndices, kFmtIndex16, box, sizeof(Vec3));
    }
    restore_debug_draw_states(host);
}

void build_debug_plane_normal_lines(const float planes[6][4], Vec3 out[12]) {
    // x87: d*n is a float*float product (exact in 53 bits, rounded once by FSTP);
    // d + 400.0 is FSTP'd to a float slot before the second products.
    for (int i = 0; i < 6; ++i) {
        const float* p = planes[i];
        const float d = p[3];
        const float far_d = static_cast<float>(static_cast<double>(d) + kPlaneNormalLength);
        out[2 * i] = {p[0] * d, p[1] * d, p[2] * d};
        out[2 * i + 1] = {p[0] * far_d, p[1] * far_d, p[2] * far_d};
    }
}

void draw_debug_plane_normals_00b26c60(DebugDrawHost& host, const float planes[6][4],
    void* camera) {
    Vec3 lines[12];
    build_debug_plane_normal_lines(planes, lines);   // computed before any state call
    host.set_render_state(kRsFogEnable, 0);
    host.set_render_state(kRsFillMode, 3);
    host.set_render_state(kRsAlphaBlendEnable, 0);
    host.set_render_state(kRsTextureFactor, 0x00FF0000u);
    host.clear_bound_shaders_inline();
    host.device_set_vertex_declaration(nullptr);
    host.device_set_fvf(kFvfXyz);
    host.set_texture_stage_state(0, kTssColorArg1, 3);
    host.set_texture_stage_state(0, kTssColorOp, 2);
    static const float kIdentity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    set_camera_transforms(host, kIdentity, camera);
    if (!host.draws_suppressed())
        host.device_draw_primitive_up(kPrimLineList, 6, lines, sizeof(Vec3));
    restore_debug_draw_states(host);
}

void build_debug_texture_quad(float size, ScreenQuadVertex out[4]) {
    for (int i = 0; i < 4; ++i) {
        const float u = (i & 1) ? 1.0f : 0.0f;
        const float v = (i & 2) ? 1.0f : 0.0f;
        out[i] = {u * size, v * size, 0.5f, 0.5f, u, v};
    }
}

void draw_debug_texture_quad_00b27440(DebugDrawHost& host, void* texture) {
    host.enter_optional_guard();
    host.output_debug_string("Texture db {");   // 00D5E618
    draw_texture_quad(host, texture, kQuadSize320);
    host.output_debug_string("}");              // 00CE4CD4
    host.leave_optional_guard();
}

void draw_debug_texture_quad_00b27780(DebugDrawHost& host, void* texture) {
    draw_texture_quad(host, texture, kQuadSize512);
}

void append_debug_line_00b290d0(RecordArray<DebugLineRecord16>& lines, Reserve16 reserve,
    const Vec3& a, const Vec3& b, std::uint32_t color_a, std::uint32_t color_b) {
    const DebugLineRecord16 records[2] = {{a.x, a.y, a.z, color_a}, {b.x, b.y, b.z, color_b}};
    for (const DebugLineRecord16& r : records) {
        if (lines.count == lines.capacity) {
            const std::int32_t grown = lines.capacity * 2;
            reserve(lines, grown < 2 ? 1 : grown);
        }
        DebugLineRecord16* slot = lines.data + lines.count;
        if (slot != nullptr)   // native placement-new null test
            *slot = r;
        ++lines.count;
    }
}

DebugLineRecord20 debug_line_2d_endpoint(const Vec2& p, std::uint32_t color, bool pixel_space,
    float aspect_scale, std::int32_t viewport_w, std::int32_t viewport_h) {
    const float sx = aspect_scale * p.x;   // FSTP float
    DebugLineRecord20 r{};
    if (pixel_space) {
        const double w = static_cast<float>(viewport_w);   // FILD, FSTP float
        const double h = static_cast<float>(viewport_h);
        r.x = static_cast<float>((static_cast<double>(sx) + 0.5) * w);
        r.y = static_cast<float>((static_cast<double>(p.y) + 0.5) * h);
    } else {
        r.x = sx;
        r.y = -0.0f - p.y;
    }
    r.z = 0.5f;   // 00CE3800
    r.w = 0.5f;
    r.color = color;
    return r;
}

void append_debug_line_2d_00b28ac0(RecordArray<DebugLineRecord20>& lines, Reserve20 reserve,
    const Vec2& a, const Vec2& b, std::uint32_t color_a, std::uint32_t color_b,
    bool pixel_space, float aspect_scale, std::int32_t viewport_w, std::int32_t viewport_h) {
    const DebugLineRecord20 records[2] = {
        debug_line_2d_endpoint(a, color_a, pixel_space, aspect_scale, viewport_w, viewport_h),
        debug_line_2d_endpoint(b, color_b, pixel_space, aspect_scale, viewport_w, viewport_h)};
    for (const DebugLineRecord20& r : records) {   // 00B256F0
        if (lines.count == lines.capacity) {
            const std::int32_t grown = lines.capacity * 2;
            reserve(lines, grown < 2 ? 1 : grown);
        }
        DebugLineRecord20* slot = lines.data + lines.count;
        if (slot != nullptr)
            *slot = r;
        ++lines.count;
    }
}

TextureUsage decode_runtime_texture_flags(std::uint32_t flags) {
    TextureUsage out{};
    const std::uint32_t pool = flags & 0xFu;
    out.pool_set = pool <= 3;
    out.pool = out.pool_set ? pool : 0;
    std::uint32_t usage = (flags & 0x10u) ? 1u : 0u;   // D3DUSAGE_RENDERTARGET
    switch (flags & 0xF00u) {
    case 0x100: usage |= 0x2; break;      // DEPTHSTENCIL
    case 0x200: usage |= 0x4000; break;   // DMAP
    case 0x300: usage |= 0x40; break;     // POINTS
    case 0x400: usage |= 0x100; break;    // NPATCHES
    case 0x500: usage |= 0x80; break;     // RTPATCHES
    default: break;
    }
    if ((flags & 0xF000u) == 0x1000u) usage |= 0x200;           // DYNAMIC
    if ((flags & 0xFF000000u) == 0x01000000u) usage |= 0x400;   // AUTOGENMIPMAP
    out.usage = usage;
    return out;
}

bool cube_texture_create_should_retry(std::uint32_t pool, std::int32_t hr) {
    return pool == 0 && hr != 0 && static_cast<std::uint32_t>(hr) != 0x8876017Cu &&
           static_cast<std::uint32_t>(hr) != 0x8007000Eu;
}

void initialize_layout_slab_00b47750(std::uint8_t* slab, std::uint32_t slab_index) {
    field<std::uint16_t>(slab, 0x8C0) = 0x20;
    for (std::uint32_t i = 0; i < 0x20; ++i) {
        field<std::uint16_t>(slab, 0x880 + 2 * i) = static_cast<std::uint16_t>(0x1F - i);
        field<std::uint32_t>(slab, i * kLayoutSlotSize + 0x40) = slab_index;
    }
}

void* allocate_layout_slot_00b48780(std::uint8_t* pool, SlabPoolHost& host) {
    void* const lock = pool + 0x0C;
    host.enter(lock);
    ++field<std::int32_t>(pool, 0x24);
    std::uint8_t**& slabs = field<std::uint8_t**>(pool, 0x28);
    std::uint32_t& count = field<std::uint32_t>(pool, 0x2C);
    std::uint32_t& capacity = field<std::uint32_t>(pool, 0x30);
    std::uint32_t& first_free = field<std::uint32_t>(pool, 0x34);
    if (first_free == 0xFFFFFFFFu) {
        first_free = count;
        auto* slab = static_cast<std::uint8_t*>(host.operator_new(kLayoutSlabSize));
        if (slab != nullptr)
            initialize_layout_slab_00b47750(slab, first_free);
        if (count == capacity) {
            capacity = capacity * 2 + 2;
            auto* grown = static_cast<std::uint8_t**>(host.operator_new(capacity * sizeof(void*)));
            for (std::uint32_t i = 0; i < count; ++i)
                if (grown + i != nullptr)
                    grown[i] = slabs[i];
            if (slabs != nullptr)
                host.free(slabs);
            slabs = grown;
        }
        if (slabs + count != nullptr)
            slabs[count] = slab;   // a failed slab allocation is stored as null
        ++count;
    }
    std::uint8_t* slab = slabs[first_free];
    std::uint16_t& free_count = field<std::uint16_t>(slab, 0x8C0);
    --free_count;
    void* const slot = slab + field<std::uint16_t>(slab, 0x880 + 2u * free_count) * kLayoutSlotSize;
    if (free_count == 0) {
        std::uint32_t next = first_free + 1;
        first_free = 0xFFFFFFFFu;
        while (next < count && field<std::uint16_t>(slabs[next], 0x8C0) == 0)
            ++next;
        if (next < count)
            first_free = next;
    }
    --field<std::int32_t>(pool, 0x24);
    host.leave(lock);
    return slot;
}

void return_layout_slot_00b479c0(std::uint8_t* pool, void* slot, SlabPoolHost& host) {
    void* const lock = pool + 0x0C;
    host.enter(lock);
    ++field<std::int32_t>(pool, 0x24);
    auto* const bytes = static_cast<std::uint8_t*>(slot);
    const std::uint32_t slab_index = field<std::uint32_t>(bytes, 0x40);
    std::uint8_t* const slab = field<std::uint8_t**>(pool, 0x28)[slab_index];
    // Signed division by 44h (magic 78787879h), truncated to 16 bits.
    const std::int32_t offset = static_cast<std::int32_t>(bytes - slab);
    std::uint16_t& free_count = field<std::uint16_t>(slab, 0x8C0);
    field<std::uint16_t>(slab, 0x880 + 2u * free_count) =
        static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(kLayoutSlotSize));
    ++free_count;
    std::uint32_t& first_free = field<std::uint32_t>(pool, 0x34);
    if (slab_index < first_free)   // unsigned: -1 always loses
        first_free = slab_index;
    --field<std::int32_t>(pool, 0x24);
    host.leave(lock);
}

void apply_instance_generator_modes_00b44c00(std::uint8_t* section, GeneratorModeHost& host) {
    if (field<std::uint8_t>(section, 0x80) == 0)
        return;
    const std::uint32_t count = field<std::uint32_t>(section, 0x6C);
    void** const bindings = reinterpret_cast<void**>(section + 0x5C);
    for (std::uint32_t i = 0; i < count; ++i) {
        auto* binding = static_cast<std::uint8_t*>(bindings[i]);
        void* generator = field<void*>(binding, 4);
        const char* mode = host.resolve_string_override(binding + 8);
        if (mode == nullptr || mode[0] == '\0')
            mode = "Default";   // 00CE3C5C, copied into the pooled temporary
        host.select_mode(generator, mode);
    }
}

} // namespace bsp::cc13_render_sprites

