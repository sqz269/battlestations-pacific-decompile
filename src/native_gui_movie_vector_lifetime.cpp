#include "bsp/native_gui_movie_vector_lifetime.hpp"
#include "bsp/native_gui_movie_record_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Movie vector lifetime requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

namespace bsp {
namespace {
volatile std::uint32_t& word(void* header, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(header) + offset);
}
void* pointer(void* header, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(header, offset));
}
} // namespace

void tidy_native_gui_movie_subtitle_vector_00aaf720(void* actual_header,
    NativeStringRawPoolContext& actual_strings) {
    void* const first = pointer(actual_header, 4);
    if (first != nullptr) {
        const void* const end = pointer(actual_header, 8);
        destroy_native_gui_movie_subtitle_strings_00aaf0e0(first, end, actual_strings);
        singleton_lifetime_free(pointer(actual_header, 4));
    }
    word(actual_header, 4) = 0;
    word(actual_header, 8) = 0;
    word(actual_header, 0x0c) = 0;
}
void tidy_native_gui_movie_extratitle_vector_00aaf7a0(void* actual_header,
    NativeStringRawPoolContext& actual_strings) {
    void* const first = pointer(actual_header, 4);
    if (first != nullptr) {
        const void* const end = pointer(actual_header, 8);
        destroy_native_gui_movie_extratitle_strings_00aaf120(first, end, actual_strings);
        singleton_lifetime_free(pointer(actual_header, 4));
    }
    word(actual_header, 4) = 0;
    word(actual_header, 8) = 0;
    word(actual_header, 0x0c) = 0;
}
} // namespace bsp
