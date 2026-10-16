#include "bsp/native_crt_os_error_map.hpp"

namespace bsp {

std::uint32_t map_native_crt_os_error_00bffb50(
    std::uint32_t error,
    const volatile std::uint32_t* actual_pairs90) noexcept {
    for (std::uint32_t index = 0; index < 45; ++index) {
        if (error == actual_pairs90[index * 2]) {
            return actual_pairs90[index * 2 + 1];
        }
    }

    if (error - 0x13u <= 0x11u) {
        return 13;
    }

    return error - 0xbcu <= 0x0eu ? 8u : 22u;
}

} // namespace bsp
