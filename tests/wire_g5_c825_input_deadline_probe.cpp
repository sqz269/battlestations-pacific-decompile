// Focused source-lifetime check; run separately from the game, with /MANIFEST:EMBED.
#include "bsp/game_input_runtime.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
void* tree;
template<class T> T read(const void* p, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof(value));
    return value;
}
void require(bool condition) {
    if (!condition) std::abort();
}
void after_native_cleanup() {
    // Registered BEFORE CC9E30, hence called AFTER its actual CD9EC0 callback.
    require(tree != nullptr);
    require(read<std::uint32_t>(tree, 0) == 0);
    require(read<void*>(tree, 4) == nullptr);
    require(read<std::uint32_t>(tree, 8) == 0);
    std::puts("c825: registered native CRT cleanup cleared the retained header");
}
}

int main() {
    require(std::atexit(&after_native_cleanup) == 0);
    tree = bsp::game::GameInputRuntime::action_deadline_map();
    void* const head = read<void*>(tree, 4);
    require(head != nullptr);
    require(read<std::uint32_t>(tree, 0) == 0);
    require(read<std::uint32_t>(tree, 8) == 0);
    require(read<void*>(head, 0) == head && read<void*>(head, 4) == head
        && read<void*>(head, 8) == head);
    require(read<std::uint8_t>(head, 0x15) == 1);
    require(bsp::game::GameInputRuntime::action_deadline_map() == tree);
    require(read<void*>(tree, 4) == head); // No second allocation/rebinding.
    std::puts("c825: repeated access retained the single initialized native tree");
}
