#include "bsp/cc16_dyn_radix_constructor.hpp"

namespace bsp::cc16_dyn_radix_constructor {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered layout requires Win32 pointers.");

volatile U32& word(void* manager, U32 offset) noexcept {
    const U32 location = reinterpret_cast<U32>(manager) + offset;
    return *reinterpret_cast<volatile U32*>(location);
}
} // namespace

void* construct_manager_00c37210(void* manager,
    const VtableBinding& vtable) noexcept {
    // The image stores literal 00D7A118. Its actual table is a required
    // binding here; no fabricated table or image-address fallback exists.
    word(manager, 0) = reinterpret_cast<U32>(vtable.table_for_00d7a118);
    for (U32 offset = 4; offset != 0x2c; offset += 4) {
        word(manager, offset) = 0;
    }
    return manager;
}

} // namespace bsp::cc16_dyn_radix_constructor
