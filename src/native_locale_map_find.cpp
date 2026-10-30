#include "bsp/native_locale_map_find.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale map lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
const void* pointer(Word value) noexcept {
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(value));
}
template<class T> T read(Word location) noexcept {
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(location), sizeof value);
    _ReadWriteBarrier();
    return value;
}
} // namespace

void* find_native_locale_map_node_00a9ec70(
    const void* actual_map4004, const void* actual_key8) {
    const Word map = address(actual_map4004);
    const Word key = address(actual_key8);
    const Word initial_length = read<Word>(key); // A9EC77, once for hash
    Word hash = 0u;
    if (initial_length != 0u) {
        const Word initial_data = read<Word>(key + 4u); // A9EC82, once
        for (Word index = 0u; index < initial_length; ++index) {
            Word unit = read<std::uint8_t>(initial_data + index);
            if (static_cast<std::uint8_t>(unit - 0x41u) <= 0x19u)
                unit += 0x20u;
            const Word signed_byte = (unit & 0x80u) != 0u
                ? unit | 0xffffff00u : unit;
            hash = hash * 0x83u + signed_byte;
        }
    }
    Word node = read<Word>(map + (hash & 0xfffu) * 4u);
    while (node != 0u) {
        const Word node_length = read<Word>(node); // left BEFORE right
        const Word key_length = read<Word>(key); // CURRENT for each candidate
        if (node_length == key_length) {
            // Equal zero lengths hit without reading either data pointer/CRT.
            if (node_length == 0u)
                return const_cast<void*>(pointer(node));
            const Word key_data = read<Word>(key + 4u); // right BEFORE left
            const Word node_data = read<Word>(node + 4u);
            const int comparison = _stricmp(
                static_cast<const char*>(pointer(node_data)),
                static_cast<const char*>(pointer(key_data)));
            if (comparison == 0)
                return const_cast<void*>(pointer(node));
        }
        // CURRENT next is sampled only after a mismatch/comparator return.
        node = read<Word>(node + 0x14u);
    }
    return nullptr;
}
} // namespace bsp
