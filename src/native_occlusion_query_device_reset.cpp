#include "bsp/native_occlusion_query_device_reset.hpp"
#include "bsp/native_shader_device_reset.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native occlusion query reset requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* address(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t result;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov result, eax }
    return result;
}
void* read_pointer(const void* location) noexcept {
    return reinterpret_cast<void*>(read_word(location));
}
__forceinline void clear_word(void* location) noexcept {
    __asm { mov eax, location }
    __asm { mov dword ptr [eax], 0 }
}

using Release = unsigned long (__stdcall*)(void*);
using CreateQuery = long (__stdcall*)(void*, std::uint32_t, void*);

} // namespace

void release_native_occlusion_query_for_reset_00b5fe20(void* actual_owner) {
    void* const captured_query = read_pointer(address(actual_owner, 0x10));
    if (captured_query) {
        const void* const table = read_pointer(captured_query);
        reinterpret_cast<Release>(read_word(address(table, 8)))(captured_query);
        clear_word(address(actual_owner, 0x10));
    }
}

void restore_native_occlusion_query_after_reset_00b5fe60(
    void* actual_owner, const void* actual_renderer_publication_00f8d394) {
    void* const device = get_native_renderer_device_00b1fef0(
        read_pointer(actual_renderer_publication_00f8d394));
    const void* const table = read_pointer(device);
    const auto create = reinterpret_cast<CreateQuery>(
        read_word(address(table, 0x1d8)));
    create(device, 9, address(actual_owner, 0x10));
}

} // namespace bsp
