#include "bsp/native_material_pass_execution.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_camera_cache_getters.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_general_inverse.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/material_effect_plane.hpp"
#include "bsp/native_material_geometry_queries.hpp"
#include "bsp/native_material_diagnostics_draw.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_renderer_clip_planes.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_stream_frequency.hpp"
#include "bsp/native_renderer_shader_binding.hpp"
#include "bsp/native_renderer_draw_primitive.hpp"
#include "bsp/native_shader_constants.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material pass execution requires MSVC Win32 x87/SSE.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(bits(p) + offset);
}
__declspec(noinline) Word word(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
__declspec(noinline) Word profile_word(const volatile Word* p, Word offset) noexcept {
    return p[offset / 4];
}
__declspec(noinline) std::uint8_t byte(const void* p, Word offset) noexcept {
    return *static_cast<volatile std::uint8_t*>(at(p, offset));
}
void* pointer(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::int32_t signed_word(const void* p, Word offset = 0) noexcept {
    return static_cast<std::int32_t>(word(p, offset));
}
void require(bool c, const char* why) {
    if (!c) throw std::logic_error(why);
}
void renderer_slot(void* renderer, Word slot, Word target,
    const NativeMaterialPassExecutionContext& c) {
    const auto token = word(renderer);
    require(token == 0x00d5f0a8u &&
        profile_word(c.actual_renderer_profile_00d5f0a8, slot) == target,
        "material pass requires its current concrete D5F0A8 renderer slot");
}
void vertex_slot(void* vertex, const NativeMaterialPassExecutionContext& c) {
    require(word(vertex) == 0x00d61d6cu &&
        profile_word(c.actual_vertex_profile_00d61d6c, 0x28) == 0x00b48d10u,
        "material pass requires its current D61D6C logical vertex offset getter");
}

// Exact COMISS/JBE gate, including unordered comparison and exception effects.
__declspec(naked) bool __fastcall positive_distance(const void*, const volatile float*) {
    __asm {
        movss xmm0, dword ptr [ecx]
        comiss xmm0, dword ptr [edx]
        seta al
        ret
    }
}

// B448FC..B44959. Native effect distance spill, x87 products and simultaneous
// SSE negative-zero subtraction. Direction has already been written in order.
void scale_plane(const void* distance, const EffectPlaneVector& direction,
    EffectPlaneVector& scaled, EffectPlaneVector& normal,
    const volatile float* negative_zero) {
    const float* src = direction.data();
    float* dst = scaled.data();
    float* normals = normal.data();
    float spill;
    __asm {
        mov eax, distance
        mov edx, src
        mov ecx, negative_zero
        fld dword ptr [eax]
        movss xmm0, dword ptr [ecx]
        fstp spill
        movaps xmm1, xmm0
        fld dword ptr [edx]
        subss xmm1, dword ptr [edx]
        fld spill
        movaps xmm2, xmm0
        subss xmm2, dword ptr [edx + 4]
        fld st(0)
        subss xmm0, dword ptr [edx + 8]
        fmulp st(2), st(0)
        fxch st(1)
        mov ecx, normals
        movss dword ptr [ecx], xmm1
        movss dword ptr [ecx + 4], xmm2
        mov eax, dst
        fstp dword ptr [eax]
        movss dword ptr [ecx + 8], xmm0
        fld dword ptr [edx + 4]
        fmul st(0), st(1)
        fstp dword ptr [eax + 4]
        fmul dword ptr [edx + 8]
        fstp dword ptr [eax + 8]
    }
}

// B44972..B449F4. Preserve position FLD/FSTP snapshots, point stores, dot
// product order Y then X then Z, the final float spill and FCHS reload.
void form_plane(const void* actual_position, const EffectPlaneVector& scaled,
    const EffectPlaneVector& normal, CameraPlane& plane) {
    const float* scale = scaled.data();
    const float* n = normal.data();
    float* output = plane.data();
    float position[3], point[3], dot;
    __asm {
        mov eax, actual_position
        fld dword ptr [eax]
        fstp dword ptr [position]
        fld dword ptr [eax + 4]
        fstp dword ptr [position + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [position + 8]
        mov ecx, n
        mov edx, output
        movss xmm1, dword ptr [ecx]
        movss xmm2, dword ptr [ecx + 4]
        movss xmm0, dword ptr [ecx + 8]
        movss dword ptr [edx], xmm1
        movss dword ptr [edx + 4], xmm2
        mov eax, scale
        fld dword ptr [eax]
        movss dword ptr [edx + 8], xmm0
        fadd dword ptr [position]
        fstp dword ptr [point]
        fld dword ptr [position + 4]
        fadd dword ptr [eax + 4]
        fstp dword ptr [point + 4]
        fld dword ptr [position + 8]
        fadd dword ptr [eax + 8]
        fstp dword ptr [point + 8]
        fld dword ptr [point + 4]
        fmul dword ptr [ecx + 4]
        fld dword ptr [point]
        fmul dword ptr [ecx]
        faddp st(1), st(0)
        fld dword ptr [point + 8]
        fmul dword ptr [ecx + 8]
        faddp st(1), st(0)
        fstp dot
        fld dot
        fchs
        fstp dword ptr [edx + 0xc]
    }
}

void append_effect_plane(void* captured_renderer, void* effect,
    void* captured_camera, NativeMaterialPassExecutionContext& c) {
    auto* queue = get_native_render_command_queue_004c11f0(c.queue);
    void* current_context = get_native_material_plane_context_00b1bfa0(queue);
    void* const scene = pointer(current_context, 0x0c);
    if (!(byte(scene, 0x5c) & 2)) refresh_native_camera_world_00b6db70(scene);
    EffectPlaneVector direction, basis, scaled, normal;
    basis[0] = 0.0f;
    basis[1] = 0.0f;
    // MOVSS reads the actual shared +1 float, without x87 quieting.
    const volatile float* one = &c.actual_one_00d7a24c;
    float* basis_words = basis.data();
    __asm {
        mov eax, one
        mov edx, basis_words
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx + 8], xmm0
    }
    transform_effect_direction_0042d0d0_no_normalize(direction, basis,
        *static_cast<const CameraMatrix*>(at(scene, 0xf0)));
    // TEST at B448F2 precedes the live mode/distance read and all scale math.
    const bool refresh = !(byte(scene, 0x5c) & 2);
    const Word mode = word(captured_camera, 0x198);
    scale_plane(at(effect, 0x140u + mode * 4u), direction, scaled, normal,
        &c.actual_negative_zero_00d7a208);
    if (refresh) refresh_native_camera_world_00b6db70(scene);
    CameraPlane plane, transformed;
    form_plane(at(scene, 0x120), scaled, normal, plane);
    CameraMatrix inverse_view, inverse_projection, product, transposed;
    void* view = get_native_camera_view_00b6fcb0(captured_camera);
    invert_native_camera_scaled_affine_00b63b30(inverse_view.data(), view);
    void* projection = get_native_camera_projection_00b6fcf0(captured_camera);
    invert_native_camera_matrix_00b632d0(inverse_projection.data(), projection);
    multiply_native_camera_matrices_00413920(inverse_projection.data(), nullptr,
        product.data(), inverse_view.data());
    copy_transpose_effect_matrix_00b23360(transposed, product);
    transform_native_plane_00b65ba0(plane.data(), transformed.data(), transposed.data());
    append_native_renderer_clip_plane_00b25040(captured_renderer,
        transformed.data(), c.synchronization);
}

// Actual material callback is a code pointer, not a numeric class token.
// B435B1 has no pushed arguments. Seed every known general-register input;
// native callback uses normal preservation of EBX/EBP/ESI/EDI on return.
__declspec(naked) void __cdecl call_material_callback(
    void*, void*, Word, void*, void*, Word, Word) {
    __asm {
        push ebx
        push ebp
        push esi
        push edi
        mov ecx, dword ptr [esp + 20]
        mov edx, dword ptr [esp + 24]
        mov eax, dword ptr [esp + 28]
        mov esi, dword ptr [esp + 32]
        mov edi, dword ptr [esp + 36]
        mov ebx, dword ptr [esp + 40]
        mov ebp, dword ptr [esp + 44]
        test eax, eax
        call eax
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}
} // namespace

void apply_native_material_pass_00b43410(void* pass, void* entry, void* override_owner,
    Word indexed, void* index_owner, Word index_base, Word vertex_base,
    NativeMaterialPassExecutionContext& c, NativeMaterialPassExecutionFrame& f) {
    static_cast<void>(index_owner); // original fourth stack word has no body read
    void* const captured_renderer = c.actual_renderer_00f8d394; // B43414
    void* const section = pointer(entry, 4); // B43426
    f.stage = 0x00b4342c;
    bind_native_renderer_material_render_states_00b27a80(captured_renderer,
        &c.material_states, static_cast<NativeMaterialStateOwnerStorage*>(pointer(pass, 0x18)));
    bind_native_renderer_material_sampler_states_00b27b90(captured_renderer,
        &c.material_states, static_cast<NativeMaterialStateOwnerStorage*>(pointer(pass, 0x20)));
    const Word alpha = get_native_material_word_104_00b17320(
        static_cast<NativeMaterialStorage*>(pointer(pointer(entry, 4), 0x20)));
    if (static_cast<std::int32_t>(alpha) > -1)
        set_native_renderer_render_state_00b24460(c.actual_renderer_00f8d394,
            0x39, alpha, c.synchronization);
    bind_native_renderer_vertex_shader_00b21d10(captured_renderer,
        pointer(pass, 0x54), c.synchronization);
    bind_native_renderer_pixel_shader_00b21c20(captured_renderer,
        pointer(pass, 0x58), c.synchronization);
    const Word pair_count = word(pass, 0x28);
    const Word enabled_mask = word(pointer(pass, 0x74), 0x84);
    Word pixel_slot = 0, vertex_slot_index = 16;
    for (Word i = 0; i < pair_count; ++i) {
        if (!(enabled_mask & (1u << (i & 31u))) &&
            !byte(pointer(pass, 0x24), i * 8u + 4u)) {
            ++pixel_slot;
            continue;
        }
        void* pair = at(pointer(pass, 0x24), i * 8u);
        const bool negative = signed_word(pair) < 0;
        void* effect = pointer(pass, 0x14); // read also on material branch
        void* texture;
        if (negative) {
            const Word selector = 0xffffffffu - word(pair);
            texture = select_native_material_effect_texture_00b17d90(
                static_cast<NativeMaterialEffectBaseStorage*>(effect),
                c.actual_increment_00ce221c, static_cast<std::int32_t>(selector));
        } else {
            const auto selector = signed_word(pair);
            void* material = pointer(section, 0x20);
            const auto count = static_cast<std::int16_t>(
                *static_cast<volatile std::uint16_t*>(at(material, 0x34)));
            texture = selector < 0 || selector >= count ? nullptr :
                pointer(material, 0x10u + static_cast<Word>(selector) * 4u);
        }
        const auto special = byte(pointer(pass, 0x24), i * 8u + 4u);
        void* renderer = c.actual_renderer_00f8d394;
        renderer_slot(renderer, 0x130, 0x00b24710, c);
        bind_native_renderer_texture_00b24710(renderer,
            special ? vertex_slot_index : pixel_slot, texture, c.textures);
        if (special) ++vertex_slot_index;
        else ++pixel_slot;
    }
    f.stage = 0x00b4353c;
    build_native_material_constants_00b42350(pass, entry, override_owner, c.constants, f.constants);
    const Word vertex_end = byte(pointer(pass, 0x70), 0x74);
    void* const pixel_stage = pointer(pass, 0x74);
    Word start = c.actual_system_register_count_00e13078;
    const Word pixel_end = byte(pixel_stage, 0x74);
    const Word vertex_count = vertex_end - start;
    const Word pixel_count = pixel_end - start;
    if (static_cast<std::int32_t>(vertex_count) > 0) {
        set_native_vertex_shader_constants_f_00b21820(c.actual_renderer_00f8d394,
            &c.synchronization, start,
            static_cast<const float*>(at(c.actual_vertex_constants_0108ebf4, start << 4)),
            vertex_count);
        start = c.actual_system_register_count_00e13078; // B43575 only after VS call
    }
    // Gate uses CURRENT PS end/start; pushed count remains captured EBP.
    if (static_cast<std::int32_t>(Word(byte(pointer(pass, 0x74), 0x74)) - start) > 0)
        set_native_pixel_shader_constants_f_00b218c0(c.actual_renderer_00f8d394,
            &c.synchronization, start,
            static_cast<const float*>(at(c.actual_pixel_constants_0108dbec, start << 4)),
            pixel_count);
    void* current_section = pointer(entry, 4);
    const Word callback = word(pointer(current_section, 0x20), 8);
    if (callback) call_material_callback(entry, current_section, callback,
        pass, section, vertex_count, pixel_count);
    f.stage = 0x00b435b3;
    void* renderer = c.actual_renderer_00f8d394;
    Word primitives, vertices;
    if (static_cast<std::uint8_t>(indexed)) {
        renderer_slot(renderer, 0x13c, 0x00b24010, c);
        const Word count = word(section, 0x18);
        const Word first_index = word(section, 0x14) + index_base;
        const Word vertex_total = word(section, 0x10);
        const Word minimum = word(section, 0x0c);
        const Word primitive = word(section, 8);
        draw_native_renderer_indexed_00b24010(renderer, primitive, minimum,
            vertex_total, first_index, count, c.indexed_draw);
        vertices = word(section, 0x10);
        primitives = word(section, 0x18);
    } else {
        renderer_slot(renderer, 0x140, 0x00b21b40, c);
        const Word count = word(section, 0x18);
        const Word start_vertex = word(section, 0x0c) + vertex_base;
        const Word primitive = word(section, 8);
        draw_native_renderer_primitive_00b21b40(renderer, primitive,
            start_vertex, count, c.synchronization);
        primitives = word(section, 0x18);
        switch (word(section, 8)) {
        case 1: vertices = primitives; break;
        case 2: vertices = primitives * 2; break;
        case 3: vertices = primitives + 1; break;
        case 4: vertices = primitives * 3; break;
        case 5: case 6: vertices = primitives + 2; break;
        default: vertices = 0; break;
        }
    }
    const Word mode = word(pointer(entry, 0x10), 0x198);
    void* diagnostics = c.actual_diagnostics_00f8d39c; // before pass effect getter
    void* effect = get_native_material_pass_effect_00b172c0(
        *static_cast<NativeMaterialPassRootStorage*>(pass));
    f.stage = 0x00b43659;
    record_native_material_diagnostics_draw_00b16f80(diagnostics,
        c.diagnostic_strings, effect, mode, primitives, vertices, vertex_count, pixel_count);
}

void bind_native_material_pass_geometry_00b44750(void* pass, void* entry,
    void* override_owner, NativeMaterialPassExecutionContext& c,
    NativeMaterialPassExecutionFrame& f) {
    void* const section = pointer(entry, 4);
    if (!word(section, 0x4c)) return;
    void* const first_vertex = pointer(section, 0x3c);
    vertex_slot(first_vertex, c);
    if (native_logical_vertex_stream_get_offset_00b48d10(first_vertex) == 0xffffffffu) return;
    const Word instance_count = get_native_draw_section_instance_count_00b855a0(section);
    for (Word stream = 0; static_cast<std::int32_t>(stream) < signed_word(section, 0x4c); ++stream) {
        void* renderer = c.actual_renderer_00f8d394;
        void* current_vertex = pointer(section, 0x3cu + stream * 4u);
        renderer_slot(renderer, 0x134, 0x00b24840, c);
        f.stage = instance_count ? 0x00b447b4 : 0x00b44822;
        bind_native_renderer_vertex_stream_00b24840(renderer, stream, current_vertex, c.vertices);
        if (instance_count) {
            current_vertex = pointer(section, 0x3cu + stream * 4u);
            if (word(current_vertex, 0x54) == 0x80000000u) {
                const Word frequency = word(current_vertex, 0x54) | 1u;
                set_native_renderer_stream_frequency_00b24a40(c.actual_renderer_00f8d394,
                    stream, frequency, c.synchronization);
            } else {
                const Word tag = word(current_vertex, 0x54);
                void* captured = c.actual_renderer_00f8d394; // B447DD before getter
                const Word count = get_native_draw_section_instance_count_00b855a0(section);
                set_native_renderer_stream_frequency_00b24a40(captured, stream,
                    tag | count, c.synchronization);
            }
        }
    }
    if (!instance_count) {
        set_native_renderer_stream_frequency_00b24a40(c.actual_renderer_00f8d394,
            0, 1, c.synchronization);
        set_native_renderer_stream_frequency_00b24a40(c.actual_renderer_00f8d394,
            1, 1, c.synchronization);
    }
    void* const effect = pointer(pass, 0x14);
    const bool changed = effect != c.actual_cached_effect_0108fbf4;
    void* captured_renderer = c.actual_renderer_00f8d394; // B4485A even if unchanged
    if (changed) {
        c.actual_cached_effect_0108fbf4 = effect;
        const auto enabled = byte(effect, 0x13c);
        void* const camera = pointer(entry, 0x10); // captured even if byte disabled
        if (enabled && positive_distance(at(effect, 0x140u + word(camera, 0x198) * 4u),
            &c.actual_zero_00d7a218)) {
            f.stage = 0x00b448a3;
            append_effect_plane(captured_renderer, effect, camera, c);
        } else {
            restore_native_renderer_pending_clip_planes_00b25080(captured_renderer, c.synchronization);
        }
    }
    void* renderer = c.actual_renderer_00f8d394;
    void* layout = pointer(pointer(entry, 4), 0x50);
    renderer_slot(renderer, 0xe0, 0x00b23f20, c);
    bind_native_renderer_vertex_layout_00b23f20(renderer, layout, c.layouts);
    void* const index = pointer(pointer(entry, 8), 0x60);
    Word index_base = 0;
    if (index) index_base = get_native_logical_index_base_00b48de0(index);
    renderer = c.actual_renderer_00f8d394;
    void* const captured_vertex = pointer(section, 0x3c);
    const Word renderer_token = word(renderer); // B44A9F capture before getter
    const Word vertex_base = get_native_logical_vertex_base_00b48d50(captured_vertex);
    require(renderer_token == 0x00d5f0a8u &&
        profile_word(c.actual_renderer_profile_00d5f0a8, 0x138) == 0x00b24b00u,
        "material pass requires its captured renderer index binding slot");
    bind_native_renderer_index_stream_00b24b00(renderer, index, vertex_base, c.indices);
    const auto indexed = index && get_native_draw_section_indexed_byte_00b855f0(section);
    // BL only is overwritten; the higher EBX bytes retain captured renderer.
    const Word indexed_word = (bits(renderer) & 0xffffff00u) | (indexed ? 1u : 0u);
    const Word current_base = get_native_logical_vertex_base_00b48d50(captured_vertex);
    f.stage = 0x00b44af2;
    apply_native_material_pass_00b43410(pass, entry, override_owner, indexed_word,
        index, index_base, current_base, c, f);
}

void execute_native_material_pass_00b454d0(void* pass, void* entry,
    NativeMaterialPassExecutionContext& c, NativeMaterialPassExecutionFrame& f) {
    bind_native_material_pass_geometry_00b44750(pass, entry, nullptr, c, f);
}

void execute_native_material_pass_current08(void* pass, void* entry,
    NativeMaterialPassExecutionContext& c, NativeMaterialPassExecutionFrame& f) {
    require(f.phase == NativeMaterialPassExecutionFrame::Phase::fresh ||
        f.phase == NativeMaterialPassExecutionFrame::Phase::complete,
        "material pass frame is already running or retains a failed execution");
    f.phase = NativeMaterialPassExecutionFrame::Phase::running;
    f.stage = 0;
    try {
        const Word token = word(pass);
        if (token == 0x00d62a80u) {
            require(profile_word(c.actual_base_pass_profile_00d62a80, 8) == 0x00b5e5e0u,
                "material base pass requires its current B5E5E0 slot");
            execute_native_material_pass_base_00b5e5e0(pass, nullptr, entry);
        } else {
            require(token == 0x00d61be8u &&
                profile_word(c.actual_derived_pass_profile_00d61be8, 8) == 0x00b454d0u,
                "material derived pass requires its current B454D0 slot");
            execute_native_material_pass_00b454d0(pass, entry, c, f);
        }
        f.phase = NativeMaterialPassExecutionFrame::Phase::complete;
    } catch (...) {
        f.phase = NativeMaterialPassExecutionFrame::Phase::failed;
        throw;
    }
}
} // namespace bsp
