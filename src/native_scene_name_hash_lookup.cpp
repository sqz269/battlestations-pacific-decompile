#include "bsp/native_scene_name_hash_lookup.hpp"
#include "bsp/native_scene_name_hash_iterator.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + offset);
}
void write(Word base, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(base) = value;
}
} // namespace

void* find_native_scene_name_hash_value_00467ee0(
    void* actual_database, const void* actual_parameter4, void* actual_iterator12) {
    const Word iterator = address(actual_iterator12);
    write(iterator, address(actual_database) + 0x34u);
    begin_native_scene_name_hash_00466bb0(pointer(iterator));
    Word node = read(iterator, 4);
    if (node == 0) return nullptr;
    const Word key = read(address(actual_parameter4));
    for (;;) {
        const Word value = read(node, 8);
        if (read(value) == key) return pointer(value);
        (void)next_native_scene_name_hash_00466c00(pointer(iterator));
        node = read(iterator, 4);
        if (node == 0) return nullptr;
    }
}

void* find_native_scene_database_class_name_00468660(
    void* actual_database, const void* actual_parameter4, void* actual_iterator12,
    const void* actual_empty_00e18560) {
    const Word iterator = address(actual_iterator12);
    write(iterator, address(actual_database) + 0x34u);
    begin_native_scene_name_hash_00466bb0(pointer(iterator));
    Word node = read(iterator, 4);
    if (node == 0) return nullptr;
    const Word key = read(address(actual_parameter4));
    for (;;) {
        const Word value = read(node, 8);
        if (read(value) == key) {
            const Word name = read(node, 4);
            return pointer(name != 0 ? name : address(actual_empty_00e18560));
        }
        (void)next_native_scene_name_hash_00466c00(pointer(iterator));
        node = read(iterator, 4);
        if (node == 0) return nullptr;
    }
}

} // namespace bsp
