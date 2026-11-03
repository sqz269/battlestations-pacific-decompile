#include "bsp/frontend_screen_sets.hpp"
#include "bsp/seg10_cc13.hpp"
#include "bsp/sound_cc13.hpp"
#include "bsp/wire_goal4_chain256_switches.hpp"

#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {

// 004D6410: assigns game+(level+56h)*10h, applies that level, scans the
// first/last pairs at +5A4h/+5A8h downwards, then writes context 1Dh.
// 004D8B70: builds an int vector and calls that body with level 3.
// Evidence: exports/bsp/functions/{004d6410,004d8b70}/decompiled.c.
// This is a borrowed field view, not a replacement GGame or input singleton.
// The only durable data is the menu's existing GameInputContextSetStack.
struct InputContextVectorFields {
    void* proxy{}; // neither body reads the checked-iterator proxy
    int* first{};
    int* last{};
    int* end{};
};
struct InputContextGameFields {
    std::uint8_t unused_000[0x570];
    InputContextVectorFields levels[4];
};
static_assert(sizeof(InputContextVectorFields) == 0x10);
static_assert(offsetof(InputContextGameFields, levels) == 0x570);
static_assert(sizeof(InputContextGameFields) == 0x5B0);

class MenuInputContextBinding final : public cc13_seg10::InputContextHost {
public:
    using Object = cc13_seg10::Object;
    explicit MenuInputContextBinding(GameInputContextSetStack& stack) : stack_(stack) {
        refresh();
    }
    Object game_fields() { return reinterpret_cast<Object>(&fields_); }

    void vector_push_back_004d6840(Object vector, const std::int32_t* value) override {
        // The body's temporary owns its allocation through this adapter. Its
        // header is refreshed on each push, including after reallocation.
        temporary_.push_back(*value);
        publish_vector(vector, temporary_);
    }
    void crt_free_00bf65ac(void* memory) override {
        if (memory != temporary_.data())
            throw std::logic_error("input-context temporary allocation mismatch");
        std::vector<int>().swap(temporary_);
    }
    void pointer_vector_assign_004d3740(Object vector, Object source) override {
        int level = 0;
        while (level < 4 && vector != reinterpret_cast<Object>(&fields_.levels[level])) ++level;
        if (level == 4) throw std::logic_error("input-context destination outside borrowed view");
        int* const first = cc13_seg10::native_at<int*>(source, 4);
        int* const last = cc13_seg10::native_at<int*>(source, 8);
        if (first == last) stack_.levels[level].clear();
        else stack_.levels[level].assign(first, last);
        refresh(); // the native backwards scan must see the just-written list
    }
    void input_contexts_apply_levels_004c4300(Object game, std::int32_t level) override {
        if (game != game_fields()) throw std::logic_error("unbound input-context game fields");
        apply_input_context_levels_004c4300(stack_, level);
    }
    Object input_manager_004bec00() override {
        return reinterpret_cast<Object>(&stack_);
    }
    void input_manager_set_context_level_00a933f0(Object manager, std::int32_t index,
        std::int32_t level) override {
        if (manager != reinterpret_cast<Object>(&stack_) || index != 0x1D)
            throw std::logic_error("unbound input-context manager field");
        stack_.context_level[index] = level;
    }

private:
    static void publish_vector(Object target, std::vector<int>& values) {
        int* const first = values.data();
        cc13_seg10::native_at<int*>(target, 4) = first;
        cc13_seg10::native_at<int*>(target, 8) = first ? first + values.size() : nullptr;
        cc13_seg10::native_at<int*>(target, 12) = first ? first + values.capacity() : nullptr;
    }
    void refresh() {
        for (int level = 0; level < 4; ++level)
            publish_vector(reinterpret_cast<Object>(&fields_.levels[level]), stack_.levels[level]);
    }
    GameInputContextSetStack& stack_;
    InputContextGameFields fields_;
    std::vector<int> temporary_;
};

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
    if constexpr (kWireGoal4Chain256Bound) {
        // 004C4323 and 004C4371 call 00A92290 for the context id. Borrow
        // this host's persistent manager-level array: its real writers below
        // publish/clear levels from GameMenuHost's authored screen context sets
        // (004D8A50/004D8AE0/004D8C00). Stores stay visible to later reads in
        // this very walk. No raw manager/vtable is fabricated for the view.
        const sound_cc13::InputManagerContextLevelsView manager{stack.context_level};
        for (int current = level; current >= 1; --current) {
            for (int id = 1; id < kGameInputContextCount; ++id) {
                if (sound_cc13::input_manager_context_level_00a92290(manager, id)
                    == static_cast<std::uint32_t>(current)) {
                    stack.context_level[id] = 0;
                }
            }
            const std::vector<int>& ids = stack.levels[current - 1];
            for (std::size_t i = 0; i < ids.size(); ++i) {
                const int id = ids[i];
                if (id < 0 || id >= kGameInputContextCount) {
                    continue; // retain the host's existing valid-index boundary
                }
                if (static_cast<std::int32_t>(
                        sound_cc13::input_manager_context_level_00a92290(manager, id))
                    < current) { // 004C437A..7C compares signed DWORD levels
                    stack.context_level[id] = current;
                }
            }
        }
    } else {
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
}

void set_game_input_context_set_004d8c00(GameInputContextSetStack& stack,
    const int* ids, std::size_t count) {
    std::vector<int> pending;
    copy_terminated_list(pending, ids, count);
    stack.levels[kGameInputContextSetLevels - 1] = pending; // 004D8C6B, game+5A0h
    apply_input_context_levels_004c4300(stack, kGameInputContextSetLevels); // 004D8C74
}

void publish_native_input_context_set(GameInputContextSetStack& stack, int level,
    const int* ids, std::size_t count) {
    if (level != 2 && level != 3) throw std::invalid_argument("input-context binding supports levels 2/3");
    // The public host API supplies a bounded list; the original cdecl body
    // consumes the same ids followed by its binary zero sentinel.
    std::vector<int> terminated;
    copy_terminated_list(terminated, ids, ids ? count : 0);
    terminated.push_back(0);
    MenuInputContextBinding binding(stack);
    if (level == 2)
        cc13_seg10::input_contexts_push_level2_004d8ae0(binding, binding.game_fields(), terminated.data());
    else
        cc13_seg10::input_contexts_push_level3_004d8b70(binding, binding.game_fields(), terminated.data());
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
