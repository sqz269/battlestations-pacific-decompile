#include "bsp/oct18b_nB_high.hpp"

namespace bsp::oct18b_nb_high {
void* identity_00800810(void* self) noexcept { return self; }
void* identity_00800820(void* self, std::uint32_t) noexcept { return self; }
void* identity_0088ab20(void* self, std::uint32_t, std::uint32_t) noexcept { return self; }
std::uint32_t owner_word4_008721f0(const void* owner) noexcept {
    return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(owner) + 4);
}
}
