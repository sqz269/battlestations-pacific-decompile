#include "bsp/native_string_vector_lookup.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual native string-vector lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(NativeMeshWeightNameStorage) == 8);
static_assert(sizeof(NativeStringVectorStorage) == 0xc);

U word(const void* owner, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
const void* pointer(U address) noexcept {
    return reinterpret_cast<const void*>(address);
}
} // namespace

std::int32_t find_native_string_vector_005efba0(
    const NativeStringVectorStorage& vector, const void* key) {
    const U count = word(&vector, 4);
    U cursor = word(&vector);
    const U captured_end = cursor + count * 8u;
    while (cursor < captured_end) {
        const U row_length = word(pointer(cursor));
        const U key_length = word(key);
        if (row_length == key_length) {
            bool equal = row_length == 0;
            if (!equal) {
                const auto* key_data = static_cast<const char*>(pointer(word(key, 4)));
                const auto* row_data = static_cast<const char*>(pointer(word(pointer(cursor), 4)));
                equal = _stricmp(row_data, key_data) == 0;
            }
            if (equal) {
                // 5EFC07 reloads the vector's data after the comparison.
                return static_cast<std::int32_t>(cursor - word(&vector)) >> 3;
            }
        }
        cursor += 8u;
    }
    return -1;
}

} // namespace bsp
