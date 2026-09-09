#include "bsp/stream_scalars.hpp"

namespace bsp {
std::uint32_t stream_read_u32_00be4300(MemoryStream& stream) {
    std::uint32_t value{};
    stream.read_00bef590(&value, 4);
    return value;
}

std::uint16_t stream_read_word_00be4340(MemoryStream& stream) {
    std::uint16_t value{};
    stream.read_00bef590(&value, 2);
    return value;
}

std::uint16_t stream_read_word_00be4320(MemoryStream& stream) {
    std::uint16_t value{};
    stream.read_00bef590(&value, 2);
    return value;
}

float stream_read_float_00be4360(MemoryStream& stream) {
    std::uint32_t bits{};
    stream.read_00bef590(&bits, 4);
    float value;
    __asm {
        fld dword ptr bits
        fstp dword ptr value
    }
    return value;
}
}
