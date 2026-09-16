#include "bsp/native_renderer_debug_lines.hpp"
#include "bsp/native_renderer_cached_states.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/native_renderer_shader_binding.hpp"
#include "bsp/native_renderer_texture_stage_state.hpp"

#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
std::uint8_t byte(const void* base, Word byte_offset) noexcept {
    std::uint8_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov al, byte ptr [eax + edx]
        mov result, al
    }
    return result;
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
void* at(void* base, Word offset) noexcept {
    return pointer(reinterpret_cast<Word>(base) + offset);
}
void put(void* base, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(base, offset)) = value;
}
void renderer_slot(void* renderer, Word offset, Word expected,
    const NativeRendererDebugLinesContext& context) {
    if (word(renderer) != 0x00d5f0a8u ||
        word(context.actual_renderer_profile_00d5f0a8, offset) != expected) {
        throw std::invalid_argument("unsupported actual debug-line renderer profile");
    }
}
using SetFvf = std::int32_t (__stdcall*)(void*, Word);
using SetTransform = std::int32_t (__stdcall*)(void*, Word, const volatile Word*);
using DrawPrimitiveUp = std::int32_t (__stdcall*)(void*, Word, Word, const void*, Word);

void transform(void* renderer, const volatile float& actual_one,
    volatile Word (&matrix)[16], Word state) {
    // MOVSS reads the original cell once per transform, before device capture.
    // Each diagonal receives those bits; all other stores are positive zero.
    const Word one = word(&actual_one);
    void* const device = pointer(word(renderer, 0x1a10));
    matrix[0] = one; matrix[1] = 0; matrix[2] = 0; matrix[3] = 0;
    matrix[4] = 0; matrix[5] = one; matrix[6] = 0; matrix[7] = 0;
    matrix[8] = 0; matrix[9] = 0; matrix[10] = one; matrix[11] = 0;
    matrix[12] = 0; matrix[13] = 0; matrix[14] = 0; matrix[15] = one;
    const void* const table = pointer(word(device));
    const auto call = reinterpret_cast<SetTransform>(word(table, 0xb0));
    (void)call(device, state, matrix);
}
} // namespace

void __fastcall draw_native_renderer_debug_lines_00b28d00(void* renderer,
    const NativeRendererDebugLinesContext* context) {
    if (word(renderer, 0x1d04) == 0) return; // B28D06; context-free empty path.
    auto& globals = context->actual_synchronization_0108d6dc;
    set_native_renderer_render_state_00b24460(renderer, 0x1c, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x34, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 8, 3, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1b, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x0f, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 7, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x0e, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0xa8, 0x0f, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x88, 0, globals);
    bind_native_renderer_vertex_shader_00b21d10(renderer, &globals, nullptr);
    bind_native_renderer_pixel_shader_00b21c20(renderer, &globals, nullptr);
    {
        void* const device = pointer(word(renderer, 0x1a10));
        const void* const table = pointer(word(device));
        const auto call = reinterpret_cast<SetFvf>(word(table, 0x164));
        (void)call(device, 0x4042);
    }
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 2, 0, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 1, 2, globals);
    volatile Word matrix[16];
    transform(renderer, context->actual_one_00d7a24c, matrix, 0x100);
    transform(renderer, context->actual_one_00d7a24c, matrix, 2);
    transform(renderer, context->actual_one_00d7a24c, matrix, 3);
    renderer_slot(renderer, 0x134, 0x00b24840u, *context);
    bind_native_renderer_vertex_stream_00b24840(renderer, 0, nullptr,
        context->actual_vertex);
    renderer_slot(renderer, 0x138, 0x00b24b00u, *context);
    bind_native_renderer_index_stream_00b24b00(renderer, nullptr, 0,
        context->actual_index);
    if (word(renderer, 0x1d90) == 0 && byte(renderer, 0x1d8a) == 0) {
        const void* const data = pointer(word(renderer, 0x1d00));
        const Word count = word(renderer, 0x1d04);
        void* const device = pointer(word(renderer, 0x1a10));
        const void* const table = pointer(word(device));
        // CDQ/SUB/SAR implements signed division by two toward zero.
        const Word primitives = static_cast<Word>(static_cast<std::int32_t>(count) / 2);
        const auto call = reinterpret_cast<DrawPrimitiveUp>(word(table, 0x14c));
        (void)call(device, 2, primitives, data, 0x14);
    }
    set_native_renderer_render_state_00b24460(renderer, 8, 2, globals);
    set_native_renderer_render_state_00b24460(renderer, 7, 1, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1b, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x89, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x16, 1, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1b, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x0f, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1c, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1d, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x0f, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x1b, 0, globals);
    set_native_renderer_render_state_00b24460(renderer, 8, 2, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 2, 3, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 1, 2, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 5, 3, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 0, 4, 2, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 1, 1, 1, globals);
    set_native_renderer_texture_stage_state_00b24510(renderer, 1, 4, 1, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x88, 1, globals);
    set_native_renderer_render_state_00b24460(renderer, 0x3c, 0xffffffffu, globals);
    void* const header = at(renderer, 0x1d00);
    if (static_cast<std::int32_t>(word(header, 8)) < 0) {
        reserve_native_renderer_records20_00b22940(header, 0, 0);
    }
    while (static_cast<std::int32_t>(word(header, 4)) > 0) {
        put(header, 4, word(header, 4) - 1u);
    }
    put(header, 4, 0);
}
} // namespace bsp
