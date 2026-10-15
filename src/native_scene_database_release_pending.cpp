#include "bsp/native_scene_database_release_pending.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_record_map_clear.hpp"
#include "bsp/native_scene_record_map_increment.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U read(U address, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(address + offset);
}
void write(U address, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(address + offset) = value;
}
void delete_object(U object) {
    if (object == 0) return;
    const U vtable = read(object);
    const U entry = read(vtable);
    using Delete = void (__thiscall*)(void*, U);
    reinterpret_cast<Delete>(entry)(reinterpret_cast<void*>(object), 1);
}
} // namespace

void release_native_scene_database_records_and_pending_0046bd10(
    void* actual_database, void* actual_private12, NativeStringRawPoolContext& strings) {
    const U database = reinterpret_cast<std::uintptr_t>(actual_database);
    const U initial_head = read(database, 0x1c);
    U node = read(initial_head);
    const U tree = database + 0x18u;
    const U backing = reinterpret_cast<std::uintptr_t>(actual_private12);
    const U iterator = backing + 4u;
    U owner = tree;
    write(backing, 0, database);
    write(backing, 8, node);
    write(backing, 4, owner);
    for (;;) {
        const U head = read(tree, 4);
        if (owner == 0 || owner != tree) invoke_native_invalid_parameter_00bf6713();
        if (node == head) break;
        if (owner == 0) invoke_native_invalid_parameter_00bf6713();
        if (node == read(owner, 4)) invoke_native_invalid_parameter_00bf6713();
        const U mapped = read(node, 0x14);
        delete_object(mapped);
        increment_native_scene_record_map_iterator_00467980(reinterpret_cast<void*>(iterator));
        node = read(backing, 8);
        owner = read(backing, 4);
    }

    const U saved_database = read(backing);
    U slot = read(saved_database, 0x28);
    const U vector = saved_database + 0x24u;
    if (slot > read(vector, 8)) invoke_native_invalid_parameter_00bf6713();
    for (;;) {
        const U end = read(vector, 8);
        if (read(vector, 4) > end) invoke_native_invalid_parameter_00bf6713();
        // 0046BD9D CMP ESI,ESI always skips the 0046BDA1 diagnostic.
        if (slot == end) break;
        if (slot >= read(vector, 8)) invoke_native_invalid_parameter_00bf6713();
        const U object = read(slot);
        delete_object(object);
        if (slot >= read(vector, 8)) invoke_native_invalid_parameter_00bf6713();
        slot += 4u;
    }

    clear_native_scene_record_map_0046b460(reinterpret_cast<void*>(tree), strings);
    const U erase_end = read(vector, 8);
    if (read(vector, 4) > erase_end) invoke_native_invalid_parameter_00bf6713();
    const U erase_begin = read(vector, 4);
    if (erase_begin > read(vector, 8)) invoke_native_invalid_parameter_00bf6713();
    if (erase_begin != erase_end) {
        const U delta = read(vector, 8) - erase_end;
        const U words = (delta >> 2u) | ((delta & 0x80000000u) ? 0xc0000000u : 0u);
        const U bytes = words * 4u;
        const U resulting_end = erase_begin + bytes;
        if (words != 0 && (words & 0x80000000u) == 0)
            (void)::memmove_s(reinterpret_cast<void*>(erase_begin), bytes,
                reinterpret_cast<const void*>(erase_end), bytes);
        write(vector, 8, resulting_end);
    }
}
} // namespace bsp
