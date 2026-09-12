#include "bsp/native_physical_stream_conversion.hpp"

#include <Windows.h>

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical stream conversion requires MSVC Win32.
#endif

namespace bsp {

bool query_native_physical_stream_type_00bf4ff0(std::uint32_t token,
    const volatile std::uint32_t* ids_0109dc30) noexcept {
    for (std::uint32_t index = 0; index != 3; ++index) {
        if (ids_0109dc30[index] == token) return true;
    }
    return false;
}

std::int32_t seek_native_physical_stream_00bf4f20(void* actual_stream,
    std::uint32_t distance_low, std::uint32_t distance_high,
    std::uint32_t origin) noexcept {
    LARGE_INTEGER distance;
    distance.LowPart = distance_low;
    std::memcpy(&distance.HighPart, &distance_high, sizeof(distance_high));
    auto* storage = static_cast<unsigned char*>(actual_stream);
    HANDLE handle;
    std::memcpy(&handle, storage + 8, sizeof(handle));
    return SetFilePointerEx(handle, distance,
        reinterpret_cast<PLARGE_INTEGER>(storage + 0x10), origin);
}

} // namespace bsp
