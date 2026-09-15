#include "bsp/native_game_resource_named_groups.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    if (callbacks.invalid_parameter) callbacks.invalid_parameter(callbacks.context);
    else _invalid_parameter_noinfo();
}

template<bool Count>
Word scan(const void* resource, const NativeLegacySboStringStorage& name,
          Word index, const SingletonLifetimeCallbacks& callbacks) {
    Word cursor = read(address(resource), 0x68);
    const Word owner = address(resource) + 0x64;
    Word result = 0;
    if (cursor > read(owner, 8)) invalid(callbacks);
    for (;;) {
        const Word end = read(owner, 8);
        if (read(owner, 4) > end) invalid(callbacks);
        // The native CMP owner,owner guard can never dispatch validation.
        if (cursor == end) return result;
        if (cursor >= read(owner, 8)) invalid(callbacks);
        const Word key_capacity = read(address(&name), 0x18);
        const Word item = read(cursor);
        const Word key_length = read(address(&name), 0x14);
        const Word key_data = key_capacity < 16 ? address(&name) + 4 : read(address(&name), 4);
        const Word item_length = read(item, 0x1c);
        const Word compared = item_length < key_length ? item_length : key_length;
        const Word item_data = read(item, 0x20) < 16 ? item + 0x0c : read(item, 0x0c);
        // 004B3FC0's unsigned comparison is a library contract. Only zero is
        // observed here, so use the CRT without copying its implementation.
        if ((!compared || std::memcmp(pointer(item_data), pointer(key_data), compared) == 0)
            && item_length == key_length && read(item, 0x24) == index) {
            if constexpr (Count) ++result;
            else result = item;
        }
        if (cursor >= read(owner, 8)) invalid(callbacks);
        cursor += 4;
    }
}

std::int32_t point_distance(Word end, Word begin) noexcept {
    return static_cast<std::int32_t>(end - begin) / 12;
}
} // namespace

std::uint8_t contains_native_game_group_00717f20(
    const void* resource, const NativeLegacySboStringStorage& name,
    Word index, const SingletonLifetimeCallbacks& callbacks) {
    return scan<true>(resource, name, index, callbacks) != 0 ? 1 : 0;
}
void* find_native_game_group_00718000(
    const void* resource, const NativeLegacySboStringStorage& name,
    Word index, const SingletonLifetimeCallbacks& callbacks) {
    return pointer(scan<false>(resource, name, index, callbacks));
}
void* lookup_native_game_group_00718870(
    const void* resource, const NativeLegacySboStringStorage& name,
    Word index, const SingletonLifetimeCallbacks& callbacks) {
    if (contains_native_game_group_00717f20(resource, name, index, callbacks) == 0) return nullptr;
    return find_native_game_group_00718000(resource, name, index, callbacks);
}
std::int32_t count_native_named_group_points_004fba10(const void* item) noexcept {
    const Word begin = read(address(item), 0x48);
    return begin ? point_distance(read(address(item), 0x4c), begin) : 0;
}
void* copy_native_named_group_point_00484270(
    const void* item, void* destination, Word index,
    const SingletonLifetimeCallbacks& callbacks) {
    const Word begin = read(address(item), 0x48);
    if (!begin || index >= static_cast<Word>(point_distance(read(address(item), 0x4c), begin)))
        invalid(callbacks);
    const Word source = read(address(item), 0x48) + index * 12u;
    __asm {
        mov edx, source
        mov eax, destination
        fld dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [edx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [edx+8]
        fstp dword ptr [eax+8]
    }
    return destination;
}

} // namespace bsp
