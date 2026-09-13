#include "bsp/native_pending_entity_drain.hpp"

#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>

namespace bsp {
namespace {
using List = NativePendingEntityListStorage;
using Node = NativePendingEntityNode;

volatile Node& node(Node* value) noexcept { return *value; }

// Actual source CRT validation; a configured returning handler resumes the
// native iterator schedule. No silent-success validation callback is added.
__declspec(noinline) void invalid_iterator_00bf6713() {
    _invalid_parameter_noinfo();
}

struct CompletedScratch final {
    List& list;
    bool complete = false;
    ~CompletedScratch() {
        if (complete) destroy_native_effect_deletion_list_004c5940(list);
    }
};

// Exact inlined global-clear sequence, NOT whole-list destruction. Keep
// compare-before-count-store and the head reload following each actual free.
void clear_pending_nodes(List& input) noexcept {
    volatile auto& list = input;
    auto* const initial_head = list.head_04;
    auto* cursor = node(initial_head).next_00;
    node(initial_head).next_00 = initial_head;
    auto* const current_head = list.head_04;
    node(current_head).previous_04 = current_head;
    bool more = cursor != list.head_04;
    list.count_08 = 0;
    while (more) {
        auto* const next = node(cursor).next_00;
        singleton_lifetime_free(cursor);
        more = next != list.head_04;
        cursor = next;
    }
}

void destroy_scratch_pair(List& destroy_input, List& kill_input,
    Node* const initial_kill_head) noexcept {
    volatile auto& destroy = destroy_input;
    volatile auto& kill = kill_input;
    auto* cursor = node(initial_kill_head).next_00;
    node(initial_kill_head).next_00 = initial_kill_head;
    auto* const current_kill_head = kill.head_04;
    node(current_kill_head).previous_04 = current_kill_head;
    auto* comparison_head = kill.head_04;
    const bool kill_nonempty = cursor != comparison_head;
    kill.count_08 = 0;
    if (kill_nonempty) {
        do {
            auto* const next = node(cursor).next_00;
            singleton_lifetime_free(cursor);
            comparison_head = kill.head_04;
            cursor = next;
        } while (cursor != comparison_head);
    }
    singleton_lifetime_free(comparison_head);
    // 92756E captures this pointer BEFORE 927572 clears the killed-list head.
    auto* const initial_destroy_head = destroy.head_04;
    kill.head_04 = nullptr;
    cursor = node(initial_destroy_head).next_00;
    node(initial_destroy_head).next_00 = initial_destroy_head;
    auto* const current_destroy_head = destroy.head_04;
    node(current_destroy_head).previous_04 = current_destroy_head;
    comparison_head = destroy.head_04;
    const bool destroy_nonempty = cursor != comparison_head;
    destroy.count_08 = 0;
    if (destroy_nonempty) {
        do {
            auto* const next = node(cursor).next_00;
            singleton_lifetime_free(cursor);
            comparison_head = destroy.head_04;
            cursor = next;
        } while (cursor != comparison_head);
    }
    singleton_lifetime_free(comparison_head);
    destroy.head_04 = nullptr;
}
} // namespace

void drain_native_pending_entities_009273a0(NativePendingEntityDrainContext& context) {
    volatile auto& owners = context.owners;
    while (owners.destroy_00f899a8.count_08 != 0 || owners.kill_00f899b4.count_08 != 0) {
        List destroy_scratch; // native allocator-word preimage remains untouched
        List kill_scratch;
        CompletedScratch destroy_cleanup{destroy_scratch};
        CompletedScratch kill_cleanup{kill_scratch};
        context.access.copy_list_00926fa0(destroy_scratch, context.owners.destroy_00f899a8);
        destroy_cleanup.complete = true;
        context.access.copy_list_00926fa0(kill_scratch, context.owners.kill_00f899b4);
        kill_cleanup.complete = true;

        clear_pending_nodes(context.owners.destroy_00f899a8);
        clear_pending_nodes(context.owners.kill_00f899b4);
        volatile auto& destroy = destroy_scratch;
        volatile auto& kill = kill_scratch;
        auto* cursor = node(destroy.head_04).next_00;
        while (cursor != destroy.head_04) {
            auto view = context.access.resolve_unit(node(cursor).payload_08);
            const std::uint32_t profile = view.alias.prefixes.observed_00.native_vtable_00;
            context.access.call_virtual_74(view.alias, profile);
            if (cursor == destroy.head_04) invalid_iterator_00bf6713();
            cursor = node(cursor).next_00;
        }

        auto* comparison_head = kill.head_04;
        cursor = node(comparison_head).next_00;
        while (cursor != comparison_head) {
            auto view = context.access.resolve_unit(node(cursor).payload_08);
            // Full native control/capture comparison is recorded in the report.
            remove_native_scene_immediate_009263c0(view, context.lifecycle);
            comparison_head = kill.head_04;
            if (cursor == comparison_head) {
                invalid_iterator_00bf6713();
                comparison_head = kill.head_04;
            }
            cursor = node(cursor).next_00;
        }
        destroy_scratch_pair(destroy_scratch, kill_scratch, comparison_head);
        kill_cleanup.complete = false;
        destroy_cleanup.complete = false;
    }
}
} // namespace bsp
