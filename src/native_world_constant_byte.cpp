#include "bsp/native_world_constant_byte.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(bool) == 1);

bool __fastcall native_world_constant_byte_009035d0(
    void* /*unused_ecx_receiver*/, std::uint32_t /*unused_edx*/) noexcept {
    return true;
}

} // namespace bsp
