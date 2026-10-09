#pragma once

#include <cstdint>

namespace bsp {

// Byte result only; no C++ promise for the other EAX bits or flags.
std::uint8_t return_native_one_byte_00876180();

} // namespace bsp
