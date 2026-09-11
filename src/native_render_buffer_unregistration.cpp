#include "bsp/native_render_buffer_unregistration.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t bits(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* address(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
void* offset(void* actual, std::uint32_t value) noexcept {
    return address(bits(actual) + value);
}
// Isolated native DWORD MOVs retain aliased actual-storage reads and writes
// without creating a C++ lifetime or overlay for an enclosing owner.
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void write_word(void* location, std::uint32_t value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
std::uint32_t remove_swap_last(void* actual_array,
    const void* actual_value_word) noexcept {
    const std::uint32_t data = read_word(actual_array);
    const std::uint32_t count = read_word(offset(actual_array, 4));
    const std::uint32_t end = data + count * 4u;
    std::uint32_t cursor = data;
    if (cursor >= end) return 0; // Does not dereference value or data.
    const std::uint32_t sought = read_word(actual_value_word);
    while (read_word(address(cursor)) != sought) {
        cursor += 4u;
        if (cursor >= end) return 0;
    }
    const std::uint32_t delta = cursor - data;
    // Native SAR DWORD,2, including the all-ones sentinel comparison.
    const std::uint32_t index = (delta >> 2) |
        ((delta & 0x80000000u) != 0 ? 0xc0000000u : 0u);
    if (index == 0xffffffffu) return 0;
    if (index != count - 1u) {
        const std::uint32_t last = read_word(address(data + count * 4u - 4u));
        write_word(address(data + index * 4u), last);
    }
    // This read follows the replacement store, even if data aliases count.
    void* const count_word = offset(actual_array, 4);
    write_word(count_word, read_word(count_word) - 1u);
    return 1;
}
void unregister_physical(void* actual_physical, std::uint32_t raw_stream,
    const void* volatile& actual_renderer_global,
    NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    const bool entry_enabled = globals.mode_00 != 0;
    if (entry_enabled) {
        guard.renderer_04 = actual_renderer_global;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            guard.renderer_04, globals);
    }
    (void)remove_native_physical_logical_pointer_00b4b2e0(
        offset(actual_physical, 8), &raw_stream);
    if (globals.mode_00 != 0) {
        // This is the documented valid-input condition, not a new runtime
        // branch or initialized fallback for native indeterminate state.
        __assume(entry_enabled);
        // The isolated MOV reads the native saved DWORD, including padding;
        // B33B00 ignores all 32 bits. No C++ indeterminate scalar read occurs.
        const std::uint32_t ignored = read_word(&guard);
        const void* const captured_renderer = address(read_word(offset(&guard, 4)));
        leave_native_renderer_optional_guard_00b33b00(
            captured_renderer, ignored, globals);
    }
}
} // namespace

std::uint32_t remove_native_renderer_vertex_pointer_00b25300(
    void* actual_array, const void* actual_value_word) noexcept {
    return remove_swap_last(actual_array, actual_value_word);
}
std::uint32_t remove_native_renderer_index_pointer_00b25370(
    void* actual_array, const void* actual_value_word) noexcept {
    return remove_swap_last(actual_array, actual_value_word);
}
std::uint32_t remove_native_renderer_texture_pointer_00b25580(
    void* actual_array, const void* actual_value_word) noexcept {
    return remove_swap_last(actual_array, actual_value_word);
}
std::uint32_t remove_native_physical_logical_pointer_00b4b2e0(
    void* actual_array, const void* actual_value_word) noexcept {
    return remove_swap_last(actual_array, actual_value_word);
}
std::uint32_t unregister_native_renderer_vertex_stream_00b268e0(
    void* actual_renderer, std::uint32_t raw_stream) noexcept {
    return remove_native_renderer_vertex_pointer_00b25300(
        offset(actual_renderer, 0x1aac), &raw_stream);
}
std::uint32_t unregister_native_renderer_index_stream_00b26900(
    void* actual_renderer, std::uint32_t raw_stream) noexcept {
    return remove_native_renderer_index_pointer_00b25370(
        offset(actual_renderer, 0x1ab8), &raw_stream);
}
std::uint32_t unregister_native_renderer_texture_00b27d40(
    void* actual_renderer, std::uint32_t raw_texture) noexcept {
    return remove_native_renderer_texture_pointer_00b25580(
        offset(actual_renderer, 0x1b00), &raw_texture);
}
void unregister_native_physical_index_stream_00b4b390(void* actual_physical,
    std::uint32_t raw_stream, const void* volatile& actual_renderer_global,
    NativeRendererSynchronizationGlobals& globals) {
    unregister_physical(actual_physical, raw_stream, actual_renderer_global, globals);
}
void unregister_native_physical_vertex_stream_00b4b3f0(void* actual_physical,
    std::uint32_t raw_stream, const void* volatile& actual_renderer_global,
    NativeRendererSynchronizationGlobals& globals) {
    unregister_physical(actual_physical, raw_stream, actual_renderer_global, globals);
}
} // namespace bsp
