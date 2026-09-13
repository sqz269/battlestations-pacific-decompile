#include "bsp/native_mpak_lookup_storage.hpp"
#include <cstdint>
#include <cstring>
namespace bsp {
namespace {
using U = std::uint32_t;
U address(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
U word(const void* base, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<std::uintptr_t>(address(base) + offset));
}
// 005EFBA0: ECX actual {begin,count} member vector, stack key8h name.
std::int32_t find_native_mpak_member_005efba0(const void* vector, const void* key) {
    U current = word(vector);
    const U end = current + word(vector, 4) * 8u;
    if (current >= end) return -1;
    do {
        const U length = word(pointer(current));
        const U key_length = word(key);
        if (length == key_length &&
            (length == 0 ||
             _stricmp(static_cast<const char*>(pointer(word(pointer(current), 4))),
                      static_cast<const char*>(pointer(word(key, 4)))) == 0))
            return static_cast<std::int32_t>((current - word(vector)) / 8u);
        current += 8u;
    } while (current < end);
    return -1;
}
} // namespace
void* NativeMpakLookupStorage::file_at_00bb4140(void* vector, U index) {
    const U first_begin = word(vector, 4);
    if (first_begin == 0 || index >= static_cast<U>(
            static_cast<std::int32_t>(word(vector, 8) - first_begin) / 0x24)) {
        // 00BF6713 may return after repairing vector+4; then reload it.
        invalid_parameters_.invalid_parameter(invalid_parameters_.context);
    }
    return pointer(word(vector, 4) + index * 0x24u);
}
void* NativeMpakLookupStorage::find_member_directory_00bb4f40(void* first,
    void* captured_end, const void* temporary) {
    U current = address(first);
    const U end = address(captured_end);
    while (current != end) {
        if (find_native_mpak_member_005efba0(pointer(current + 8u), temporary) > -1)
            break;
        current += 0x14u;
    }
    return pointer(current);
}
} // namespace bsp
