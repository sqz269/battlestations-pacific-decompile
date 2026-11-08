// Focused chain543 ownership fixture. Link against the built Win32 bsp_core.lib
// (and bsp_lua511/bsp_zlib121), /MD /link /MANIFEST:EMBED. Synthetic unit indices
// exercise list ownership/order only; this is not a game-runtime probe.
#include "bsp/game_recon_relation_lists.hpp"

#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <utility>
#include <vector>

namespace {
struct Sensors final : bsp::cc13_seg49::ReconEntrySensorHost {
    int shift{};
    int recon_unit_sensor_class_index(std::size_t unit) override {
        return static_cast<int>(unit) + shift; // fixture getter, never game input
    }
};

void check(const bsp::cc13_seg49::ReconList& list,
    std::initializer_list<std::pair<std::size_t, int>> expected) {
    assert(list.count == static_cast<int>(expected.size()));
    const bsp::cc13_seg49::ReconNode* previous = nullptr;
    auto* node = list.head;
    for (const auto& value : expected) {
        assert(node != nullptr && node->prev == previous);
        const auto& entry = *static_cast<const bsp::GameReconRelationLists::Entry*>(node->payload);
        assert(entry.unit == value.first && entry.level == value.second);
        assert(entry.native_vtable == bsp::cc13_seg49::kReconEntryVtable);
        assert(entry.members.count == 0 && entry.members.head == nullptr && entry.members.tail == nullptr);
        assert(entry.node == node && entry.owner == &list);
        previous = node;
        node = node->next;
    }
    assert(node == nullptr && list.tail == previous);
}
} // namespace

int main() {
    Sensors sensors;
    bsp::GameReconRelationLists state;
    state.begin_pass();
    state.refresh(0x10, 0, 10, 2, sensors);
    state.refresh(0x10, 0, 30, 2, sensors);
    state.refresh(0x10, 0, 20, 2, sensors);
    state.refresh(0x10, 1, 40, 1, sensors);
    state.refresh(0x10, 2, 50, 0, sensors);
    state.refresh(0x19, 0, 60, 2, sensors);
    state.retire_unscanned(0x10);
    state.retire_unscanned(0x19);
    check(state.list(0x10, 0), {{10, 2}, {30, 2}, {20, 2}});
    auto* ten = state.list(0x10, 0).head;
    auto* thirty = ten->next;
    auto* twenty = state.list(0x10, 0).tail;
    auto* forty = state.list(0x10, 1).head;
    assert(static_cast<const bsp::GameReconRelationLists::Entry*>(thirty->payload)->sensor_class_index == 30);

    // Reuse nodes while changing relation, level and scan order. Unit50 and
    // the entire class19 bucket disappear, so neither may survive projection.
    state.begin_pass();
    sensors.shift = 100;
    for (std::size_t relation = 0; relation < 3; ++relation)
        check(state.list(0x10, relation), {});
    state.refresh(0x10, 1, 20, 1, sensors);
    state.refresh(0x10, 2, 10, 2, sensors);
    state.refresh(0x10, 0, 40, 2, sensors);
    state.refresh(0x10, 0, 30, 2, sensors);
    state.retire_unscanned(0x10);
    state.retire_unscanned(0x19);
    check(state.list(0x10, 0), {{40, 2}, {30, 2}});
    check(state.list(0x10, 1), {{20, 1}});
    check(state.list(0x10, 2), {{10, 2}});
    check(state.list(0x19, 0), {});
    assert(state.list(0x10, 0).head == forty && state.list(0x10, 0).tail == thirty);
    assert(static_cast<const bsp::GameReconRelationLists::Entry*>(thirty->payload)->sensor_class_index == 130);
    assert(state.list(0x10, 1).head == twenty && state.list(0x10, 2).head == ten);
    struct Rec { std::size_t unit; int level; };
    std::map<int, std::vector<Rec>> projection;
    state.project(0, projection);
    assert(projection.size() == 1 && projection.at(0x10).size() == 2);
    assert(projection.at(0x10)[0].unit == 40 && projection.at(0x10)[1].unit == 30);

    // A same-relation refresh still follows current scan order. The last pass
    // retires everything before one final insertion exercises destruction.
    state.begin_pass();
    state.refresh(0x10, 0, 30, 2, sensors);
    state.refresh(0x10, 0, 40, 2, sensors);
    state.refresh(0x10, 1, 20, 0, sensors);
    state.retire_unscanned(0x10);
    check(state.list(0x10, 0), {{30, 2}, {40, 2}});
    check(state.list(0x10, 1), {{20, 0}});
    check(state.list(0x10, 2), {});
    assert(state.list(0x10, 0).head == thirty && state.list(0x10, 0).tail == forty);
    state.begin_pass();
    state.retire_unscanned(0x10);
    state.project(0, projection);
    assert(projection.empty());
    // Leave one real node for destruction, after the all-empty transition.
    state.refresh(0x10, 0, 70, 2, sensors);
    check(state.list(0x10, 0), {{70, 2}});
    std::puts("chain543 recon ownership/order fixture passed");
}
