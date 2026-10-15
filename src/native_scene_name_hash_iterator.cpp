#include "bsp/native_scene_name_hash_iterator.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == sizeof(U));
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
U read(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
void write(U p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(p + offset) = value;
}
U head_offset(U index) noexcept { return 8u + index * 4u; }
} // namespace

void __fastcall begin_native_scene_name_hash_00466bb0(void* actual_iterator12) {
    const U iterator = address(actual_iterator12);
    const U table = read(iterator);
    if (read(table, 4) == 0) {
        write(iterator, 4, 0);
        return;
    }
    write(iterator, 8, 0);
    if (read(table, 8) == 0) {
        for (;;) {
            U index = read(iterator, 8);
            if (index >= 0x40u) break;
            ++index;
            write(iterator, 8, index);
            if (read(table, head_offset(index)) != 0) break;
        }
    }
    const U index = read(iterator, 8);
    const U node = read(table, head_offset(index));
    write(iterator, 4, node);
}

std::uint32_t __fastcall next_native_scene_name_hash_00466c00(void* actual_iterator12) {
    const U iterator = address(actual_iterator12);
    const U node = read(iterator, 4);
    if (node != 0) {
        U next = read(node, 12);
        if (next == 0) {
            write(iterator, 8, read(iterator, 8) + 1u);
            U index = read(iterator, 8);
            if (index < 0x40u) {
                const U scan_table = read(iterator);
                while (read(scan_table, head_offset(index)) == 0) {
                    ++index;
                    write(iterator, 8, index);
                    if (index >= 0x40u) break;
                }
            }
            index = read(iterator, 8);
            if (index >= 0x40u) {
                write(iterator, 4, 0);
                return read(iterator, 4) != 0 ? 1u : 0u;
            }
            const U current_table = read(iterator);
            next = read(current_table, head_offset(index));
        }
        write(iterator, 4, next);
    }
    return read(iterator, 4) != 0 ? 1u : 0u;
}

} // namespace bsp
