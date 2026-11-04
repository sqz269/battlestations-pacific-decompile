#include "bsp/frontend_seg16_24_28_cc13.hpp"

#include <cassert>
#include <cstdio>

int main() {
    bsp::MainMenuManager menu;
    bsp::FrontEndManagerRegistry registry;
    bsp::FrontEndManagerBase other;
    registry.managers.push_back(&other);
    int payload = 0;
    menu.base.applied = {7, &payload};
    menu.base.pending = {9, &payload};
    menu.base.applied.observer_words.fill(42);
    menu.base.pending.observer_words.fill(43);
    menu.base.active = true;
    menu.base.native_vtable = 0xffffffffu;
    menu.music_paths = {"old0", "old1"};
    menu.music_handles.fill(&payload);
    menu.screens.fill(&payload);
    menu.flag_74 = true;

    // One poisoned-owner case checks that Init receives the constructor's
    // actual members, with its registry insertion already visible.
    auto* result = bsp::cc13_fe5::main_menu_construct_00686170(menu, registry);
    assert(result == &menu);
    assert(menu.base.native_vtable == 0x00cf774cu);
    assert(!menu.base.active);
    for (const auto* record : {&menu.base.applied, &menu.base.pending}) {
        assert(record->interface_id == 0);
        assert(record->payload == nullptr);
        assert(record->observer_vtable == 0x00ce3cf0u);
        assert(record->observer_enabled);
        for (auto word : record->observer_words) assert(word == 0);
    }
    for (auto path : menu.music_paths) assert(path.empty() && path.data() == nullptr);
    for (auto handle : menu.music_handles) assert(handle == nullptr);
    for (auto screen : menu.screens) assert(screen == nullptr);
    assert(!menu.flag_74);
    assert(registry.managers.size() == 2);
    assert(registry.managers[0] == &other && registry.managers[1] == &menu.base);
    // The typed host reuses its persistent allocation on return to the menu;
    // native insert-unique must not append another copy of the same base.
    assert(bsp::cc13_fe5::main_menu_construct_00686170(menu, registry) == &menu);
    assert(registry.managers.size() == 2);
    std::puts("chain503: canonical constructor fields and unique registry publication passed");
}
