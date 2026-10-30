#include "bsp/native_gui_movie_record_lifetime.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Movie record lifetime requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

namespace bsp {
namespace {
template<std::uint32_t Stride>
inline void destroy_record_strings(void* actual_first, const void* actual_end,
    NativeStringRawPoolContext& actual_strings) {
    std::uint32_t current = reinterpret_cast<std::uint32_t>(actual_first);
    const std::uint32_t captured_end = reinterpret_cast<std::uint32_t>(actual_end);
    while (current != captured_end) {
        destroy_native_string_header_0041dd20(reinterpret_cast<void*>(current), actual_strings);
        current += Stride;
    }
}
} // namespace

void destroy_native_gui_movie_subtitle_strings_00aaf0e0(void* actual_first,
    const void* actual_end, NativeStringRawPoolContext& actual_strings) {
    destroy_record_strings<0x14>(actual_first, actual_end, actual_strings);
}
void destroy_native_gui_movie_extratitle_strings_00aaf120(void* actual_first,
    const void* actual_end, NativeStringRawPoolContext& actual_strings) {
    destroy_record_strings<0x24>(actual_first, actual_end, actual_strings);
}
} // namespace bsp
