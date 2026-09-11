#include "bsp/native_hardware_layout_device_release.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <unknwn.h>

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hardware layout device release requires MSVC Win32.
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
void clear(void* owner) noexcept {
    __asm { mov eax, owner }
    __asm { mov dword ptr [eax + 40h], 0 }
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
using ComReferenceCall = ULONG (STDMETHODCALLTYPE*)(void*);
ComReferenceCall current_com_slot(void* object, std::uint32_t offset) noexcept {
    return reinterpret_cast<ComReferenceCall>(word(pointer(word(object)), offset));
}
} // namespace

void release_native_hardware_layout_device_resource_00b600b0(void* owner) {
    auto* const captured = pointer(word(owner, 0x40));
    if (captured) {
        (void)current_com_slot(captured, 4)(captured);
        (void)current_com_slot(captured, 8)(captured);
    }
    auto* const current = pointer(word(owner, 0x40));
    if (current) {
        (void)current_com_slot(current, 8)(current);
        clear(owner);
    }
}
} // namespace bsp
