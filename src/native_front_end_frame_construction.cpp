#include "bsp/native_front_end_frame_construction.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// Keep the real base call visible for the bounded provider/object audit.
__declspec(noinline) void* construct_native_front_end_screen_004f7180(
    void* actual_receiver) {
    void* const captured = actual_receiver;
    auto* const bytes = static_cast<volatile unsigned char*>(captured);
    *static_cast<volatile std::uint32_t*>(captured) = 0x00ceae54u;
    bytes[4] = 0;
    bytes[5] = 0;
    return captured;
}

__declspec(noinline) void* construct_native_front_end_frame_00517d10(
    void* actual_receiver) {
    void* const captured = actual_receiver;
    construct_native_front_end_screen_004f7180(captured);
    auto* const words = static_cast<volatile std::uint32_t*>(captured);
    words[2] = 0x00cec388u;
    words[0] = 0x00cec39cu;
    words[2] = 0x00cec398u;
    return captured;
}

} // namespace bsp
