#include "bsp/native_cube_texture_base.hpp"
#include "bsp/native_logical_texture_named_base.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cube texture base requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const void* storage, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm { mov eax, storage }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* storage, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm { mov eax, storage }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
} // namespace

void* construct_native_logical_texture_unnamed_base_00b34020(
    void* owner, void* borrowed_com, std::uint32_t flags,
    std::uint32_t& serial) noexcept {
    put(owner, 0, 0x00ceb130);
    put(owner, 0, 0x00d5f1f4);
    put(owner, 4, 1);
    put(owner, 8, 0);
    put(owner, 0x0c, 0);
    put(owner, 0x14, 0);
    put(owner, 0x10, reinterpret_cast<std::uintptr_t>(borrowed_com));
    put(owner, 0x1c, flags);
    put(owner, 0x20, word(&serial));
    put(&serial, 0, word(&serial) + 1u);
    put(owner, 0, 0x00d5f280);
    return owner;
}

void unwind_native_logical_texture_unnamed_base_00b34090(
    void* owner, NativeStringStorage& strings) {
    destroy_native_logical_texture_named_base_00b33f50(owner, strings);
}
} // namespace bsp
