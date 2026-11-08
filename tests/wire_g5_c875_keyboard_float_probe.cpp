// Focused raw-tree/keyboard-provider compatibility fixture; no game launch.
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_input_keyboard_storage.hpp"
#include "bsp/native_input_scalar_map_full_cleanup.hpp"
#include "bsp/native_soldier_loop_lengths.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <stdexcept>

namespace {
struct Tree {
    std::uint32_t opaque;
    void* head;
    std::uint32_t count;
};
void check(bool condition) {
    if (!condition) throw std::runtime_error("keyboard raw-float map compatibility failed");
}
struct Map {
    bsp::NativeStringRawPoolContext& strings;
    Tree tree{0xaabbccddu, nullptr, 0};
    explicit Map(bsp::NativeStringRawPoolContext& context) : strings(context) {
        // Same 1Ch sentinel and fields as fresh_device's +24h map producer.
        tree.head = bsp::singleton_lifetime_allocate(
            {bsp::SingletonAllocationKind::object, 0x1c, 0x1c});
        for (unsigned offset : {0u, 4u, 8u})
            std::memcpy(static_cast<char*>(tree.head) + offset, &tree.head, 4);
        static_cast<unsigned char*>(tree.head)[0x18] = 1;
        static_cast<unsigned char*>(tree.head)[0x19] = 1;
    }
    ~Map() {
        if (tree.head) bsp::destroy_native_input_scalar_map_00445900(&tree, 0, strings);
    }
};
struct Key {
    bsp::NativeStringRawPoolContext& strings;
    bsp::NativeString value;
    Key(bsp::NativeStringRawPoolContext& context, const char* name) : strings(context) {
        value.assign_0041e870(strings, name);
    }
    ~Key() { bsp::destroy_native_string_header_0041dd20(&value, strings); }
};
} // namespace

int main() {
    try {
        auto& process = bsp::game::game_native_string_process();
        auto& strings = process.raw_context();
        bsp::NativeInputKeyboardStorage keyboard(process.strings());
        Map map(strings);
        float* alpha = nullptr;
        for (const char* name : {"Zulu", "alpha", "Echo", "bravo"}) {
            Key key(strings, name);
            float* const value = bsp::get_or_insert_native_soldier_loop_length_00444be0(
                &map.tree, &key.value, strings, {});
            check(*value == 0.0f);
            *value = 0.25f;
            if (std::strcmp(name, "alpha") == 0) alpha = value;
            check(keyboard.multiplier_00444be0(&map.tree, &key.value) == value);
        }
        {
            Key duplicate(strings, "ALPHA");
            check(bsp::get_or_insert_native_soldier_loop_length_00444be0(
                &map.tree, &duplicate.value, strings, {}) == alpha);
            check(*alpha == 0.25f && map.tree.count == 4);
        }
        {
            Key old_key(strings, "charlie");
            float* const old_value = keyboard.multiplier_00444be0(&map.tree, &old_key.value);
            *old_value = 0.5f;
            Key same(strings, "CHARLIE");
            check(bsp::get_or_insert_native_soldier_loop_length_00444be0(
                &map.tree, &same.value, strings, {}) == old_value);
            check(*old_value == 0.5f && map.tree.count == 5);
        }
        check(map.tree.opaque == 0xaabbccddu);
        bsp::destroy_native_input_scalar_map_00445900(&map.tree, 0, strings);
        check(!map.tree.head && !map.tree.count && map.tree.opaque == 0xaabbccddu);
        std::puts("c875 keyboard raw-float map: insert/casefold/value identity/legacy interop/cleanup PASS");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
