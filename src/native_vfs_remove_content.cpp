#include "bsp/native_vfs_remove_content.hpp"

#include "bsp/native_string_vector_erase_unordered.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);

U word(const void* base, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}

void* pointer(U address) noexcept {
    return reinterpret_cast<void*>(address);
}
} // namespace

void remove_native_vfs_content_package_00bdd020(void* manager,
    const void* key, NativeStringRawPoolContext& strings) {
    // Preserve initial count-before-begin capture and DWORD end arithmetic.
    const U initial_count = word(manager, 0x4c);
    U cursor = word(manager, 0x48);
    auto* const vector = pointer(static_cast<U>(
        reinterpret_cast<std::uintptr_t>(manager)) + 0x48u);
    void* iterator = pointer(cursor);
    if (cursor == cursor + initial_count * 8u)
        return;

    do {
        const U row_length = word(pointer(cursor));
        const U key_length = word(key);
        bool equal = false;
        if (row_length == key_length) {
            equal = row_length == 0;
            if (!equal) {
                // Native reads key data before row data, then pushes both.
                const auto* key_data = static_cast<const char*>(pointer(word(key, 4)));
                const auto* row_data = static_cast<const char*>(
                    pointer(word(pointer(cursor), 4)));
                equal = _stricmp(row_data, key_data) == 0;
            }
        }
        if (equal) {
            erase_native_string_vector_unordered_0043ea00(vector, strings, &iterator);
            // Native ESI is retained; the iterator cell is not read back.
        } else {
            cursor += 8u;
            iterator = pointer(cursor);
        }
        const U current_count = word(vector, 4);
        const U current_begin = word(vector);
        if (cursor == current_begin + current_count * 8u)
            return;
    } while (true);
}
} // namespace bsp
