#include "bsp/frontend_screen_sets.hpp"

namespace bsp {
namespace {

// 006873EC, read back as 1Dh dword targets and resolved to the constant each
// target loads into EAX. Index 0 is interface id 3. The default arm 006873E7 is
// `xor eax,eax; ret 4`, so every id outside 3..1Fh and the six holes inside it
// map to 0.
constexpr int kMultiMenuScreenIds[kMultiMenuInterfaceIdLast - kMultiMenuInterfaceIdFirst + 1] = {
    0x03, // 03 INTF_BRIEFING          006873BF
    0x04, // 04 INTF_REWARDS           006873C7
    0x00, // 05 INTF_UNITLIB           006873E7 default
    0x00, // 06 INTF_VIDEOLIB          006873E7 default
    0x00, // 07 INTF_MEDALS            006873E7 default
    0x00, // 08 INTF_CREDITS           006873E7 default
    0x00, // 09 INTF_DOWNLOADEDCONTENT 006873E7 default
    0x00, // 0A INTF_LEADERBOARDS      006873E7 default
    0x0B, // 0B INTF_ACHIEVEMENTS      006873DF
    0x00, // 0C INTF_OPTIONS           006873E7 default
    0x00, // 0D INTF_CONTROLLAYOUT     006873E7 default
    0x00, // 0E INTF_KEYBOARDSETUP     006873E7 default
    0x10, // 0F INTF_MULTIMAINMENU     00687347
    0x11, // 10 INTF_MULTIMODESELECTOR 0068734F
    0x12, // 11 INTF_MULTILIVERANKEDSELECTOR 00687357
    0x13, // 12 INTF_MULTILAN          0068735F
    0x14, // 13 INTF_MULTICREATELANSERVER 00687367
    0x16, // 14 INTF_MULTIGAMELOBBY    0068736F
    0x17, // 15 INTF_MULTISELECTPLAYER 00687377
    0x18, // 16 INTF_MULTIPLAYERS      0068737F
    0x19, // 17 INTF_MULTIINGAME       00687387
    0x1A, // 18 INTF_MULTIFRIENDS      006873AF
    0x1B, // 19 INTF_MULTISIGNIN       0068738F
    0x1C, // 1A INTF_MULTISESSIONBROWSER 0068739F
    0x1D, // 1B INTF_MULTIERROR        006873B7
    0x1E, // 1C INTF_MULTICREATENEWACCOUNTPC 00687397
    0x1F, // 1D INTF_MULTISESSIONFILTER 006873A7
    0x54, // 1E INTF_MULTITACTSHIT     006873CF
    0x5E, // 1F INTF_MULTIPOINTSSHIT   006873D7
};

// Copies a zero-terminated varargs list. The native loop stops at the first
// zero and never stores it, so a caller that passes an interior zero publishes a
// shorter list than it wrote.
void copy_terminated_list(std::vector<int>& out, const int* ids, std::size_t count) {
    out.clear();
    for (std::size_t i = 0; i < count; ++i) {
        if (ids[i] == 0) {
            break;
        }
        out.push_back(ids[i]);
    }
}

} // namespace

int multi_menu_screen_id_00687330(int interface_id) noexcept {
    if (interface_id < kMultiMenuInterfaceIdFirst || interface_id > kMultiMenuInterfaceIdLast) {
        return kFrontEndScreenIdNone;
    }
    return kMultiMenuScreenIds[interface_id - kMultiMenuInterfaceIdFirst];
}

int main_menu_screen_id_00685820(int interface_id) noexcept {
    // 00685844 indexes the eleven-entry table at 00685948 with id - 1; every arm
    // pushes the id itself, so the map is the identity over 1..0Bh. The default
    // arm at 00685938 publishes the empty set.
    if (interface_id < kMainMenuInterfaceIdFirst || interface_id > kMainMenuInterfaceIdLast) {
        return kFrontEndScreenIdNone;
    }
    return interface_id;
}

int options_menu_screen_id_00689820(int interface_id) noexcept {
    // 00689838 is a three-way compare chain, not a table: 0Ch pushes 0Dh, 0Dh
    // pushes 0Eh, 0Eh pushes 0Fh, anything else falls to 0068984B with the bare
    // terminator already pushed.
    if (interface_id < kOptionsMenuInterfaceIdFirst || interface_id > kOptionsMenuInterfaceIdLast) {
        return kFrontEndScreenIdNone;
    }
    return interface_id + 1;
}

void recompute_front_end_screen_requests_004f7620(FrontEndScreenSetStack& stack,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings) {
    // 004F7627: the 5Fh-byte scratch bitmap, one byte per registry slot.
    bool requested[kFrontEndScreenSlotCount] = {};

    stack.occlusion_level = kFrontEndOcclusionLevelNone; // 004F7638
    // 004F7642: ESI starts at 00E18D38 (level 5) and walks down by 10h.
    for (int level = kFrontEndScreenSetHighestLevel; level >= kFrontEndScreenSetLowestLevel;
         --level) {
        const std::vector<int>& ids = stack.levels[level - 1];
        for (std::size_t i = 0; i < ids.size(); ++i) {
            const int slot = ids[i];
            if (slot < 0 || slot >= kFrontEndScreenSlotCount) {
                continue; // the native indexes 00E18B60 unchecked; see the doc
            }
            if (table.slots[slot] == nullptr) {
                continue; // 004F7685
            }
            // 004F7689: the predicate is only asked while this level can still
            // raise the floor, so the first opaque screen in a level wins.
            if (stack.occlusion_level < level && bindings.screen_occludes_lower_levels(slot)) {
                stack.occlusion_level = level;
            }
            // 004F76A2: SETGE on level >= floor, OR-ed into the bitmap.
            requested[slot] = requested[slot] || (stack.occlusion_level <= level);
        }
    }

    // 004F76D0, the clear pass. It runs over all 95 slots before the set pass
    // starts, which is how the native emitted it.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        FrontEndScreen* screen = table.slots[slot];
        if (screen == nullptr || bindings.screen_manages_own_visibility(slot)) {
            continue;
        }
        if (!requested[slot]) {
            screen->wanted = false; // 004F76EE
        }
    }

    // 004F7700, the set pass.
    for (int slot = 0; slot < kFrontEndScreenSlotCount; ++slot) {
        FrontEndScreen* screen = table.slots[slot];
        if (screen == nullptr || bindings.screen_manages_own_visibility(slot)) {
            continue;
        }
        if (requested[slot]) {
            screen->wanted = true; // 004F771E
        }
    }
}

void set_front_end_screen_set_level(FrontEndScreenSetStack& stack, FrontEndScreenTable& table,
    FrontEndScreenSetHostBindings& bindings, int level, const int* ids, std::size_t count) {
    if (level < kFrontEndScreenSetLowestLevel || level > kFrontEndScreenSetHighestLevel) {
        return;
    }
    std::vector<int> pending;
    copy_terminated_list(pending, ids, count);
    // 004F877E: vector::operator= into the level's global. An empty source runs
    // the erase(begin, end) arm at 004F7E80, so the level is cleared.
    stack.levels[level - 1] = pending;
    stack.dirty = true; // 004F8777, the byte at 00E18CDC
    recompute_front_end_screen_requests_004f7620(stack, table, bindings); // 004F8783
}

void set_front_end_screen_set_004f8710(FrontEndScreenSetStack& stack, FrontEndScreenTable& table,
    FrontEndScreenSetHostBindings& bindings, const int* ids, std::size_t count) {
    set_front_end_screen_set_level(stack, table, bindings, 4, ids, count);
}

void apply_input_context_levels_004c4300(GameInputContextSetStack& stack, int level) {
    if (level < 1 || level > kGameInputContextSetLevels) {
        return;
    }
    for (int current = level; current >= 1; --current) {
        // 004C4310: contexts 1..19h, not 0..19h.
        for (int id = 1; id < kGameInputContextCount; ++id) {
            if (stack.context_level[id] == current) {
                stack.context_level[id] = 0;
            }
        }
        const std::vector<int>& ids = stack.levels[current - 1];
        for (std::size_t i = 0; i < ids.size(); ++i) {
            const int id = ids[i];
            if (id < 0 || id >= kGameInputContextCount) {
                continue; // the native indexes [manager+10h] unchecked
            }
            if (stack.context_level[id] < current) { // 004C4360
                stack.context_level[id] = current;
            }
        }
    }
}

void set_game_input_context_set_004d8c00(GameInputContextSetStack& stack,
    const int* ids, std::size_t count) {
    std::vector<int> pending;
    copy_terminated_list(pending, ids, count);
    stack.levels[kGameInputContextSetLevels - 1] = pending; // 004D8C6B, game+5A0h
    apply_input_context_levels_004c4300(stack, kGameInputContextSetLevels); // 004D8C74
}

void commit_front_end_screen_visibility_004f83b0(const FrontEndScreen& screen, int slot,
    FrontEndScreenCommitHost& host) {
    std::vector<void*> children;
    host.collect_screen_children(slot, children); // virtual +24h
    for (std::size_t i = 0; i < children.size(); ++i) {
        if (children[i] != nullptr) { // 004F83F5
            host.set_child_visible(children[i], screen.active); // child virtual +34h
        }
    }
}

void publish_front_end_manager_screen_set(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts, int screen_id) {
    if (screen_id == kFrontEndScreenIdNone) {
        // 00685938 and 0068984B: the terminator alone, and no 004D8C00 call.
        set_front_end_screen_set_004f8710(screens, table, bindings, nullptr, 0);
        return;
    }
    const int screen_ids[1] = {screen_id};
    set_front_end_screen_set_004f8710(screens, table, bindings, screen_ids, 1);
    const int context_ids[1] = {kFrontEndManagerInputContext};
    set_game_input_context_set_004d8c00(contexts, context_ids, 1);
}

void clear_front_end_manager_screen_set(FrontEndScreenSetStack& screens,
    FrontEndScreenTable& table, FrontEndScreenSetHostBindings& bindings,
    GameInputContextSetStack& contexts) {
    set_front_end_screen_set_004f8710(screens, table, bindings, nullptr, 0); // 00683AA6
    set_game_input_context_set_004d8c00(contexts, nullptr, 0); // 00683AB3
}

} // namespace bsp
