// Focused source-level fixture for the private live-inventory adapter.
// Compile with tests/wire_g5_c799_modifier_release_probe.cmd after the official
// build; this does not run the game.
#include "../src/game_hosts_powerups.cpp"
#include <cstdio>

namespace {
namespace cp = bsp::cc13_powerup;
namespace gp = bsp::game_powerups;
struct Host final : cp::PowerupManagerHost {
    [[noreturn]] static void unexpected() { throw std::logic_error("unexpected fixture provider"); }
    float clock_00f876a4() override { unexpected(); }
    std::int32_t local_player_slot_18ec() override { unexpected(); }
    std::int32_t session_word_1fe4() override { return 0; }
    bool powerups_enabled_00e0c978() override { unexpected(); }
    void award_active_powerup_use(std::int32_t) override { unexpected(); }
    void report_powerup_009789a0(std::int32_t, const cp::PowerupItem&, const std::string&) override { unexpected(); }
    bool powerup_screen_shown_00e198c4() override { unexpected(); }
    void powerup_screen_rebuild_lists_0061b0f0() override { unexpected(); }
    bool hint_has_been_shown_0068e600(const std::string&) override { unexpected(); }
    void trigger_hint_00690fd0(const std::string&, std::int32_t) override { unexpected(); }
    void resolve_air_support_classes_008e34f0(bsp::PowerupClassConfig&) override { unexpected(); }
    void send_powerup_message67(std::int32_t, const cp::PowerupItem&, std::int32_t) override { unexpected(); }
    void send_powerup_message68(const gp::PowerupModifier&, std::int32_t) override { unexpected(); }
    std::vector<bsp::PowerUpSlot>& active_slot_records() override { unexpected(); }
    void set_active_slot_notify_gate(bool) override { unexpected(); }
};
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
}

int main() {
    try {
        // Synthetic fixture owners; the production adapter borrows its actual
        // published runtime manager, class and modifier records.
        bsp::PowerupConfigOwner manager(bsp::PowerupOwnerAllocationWords{});
        bsp::PowerupClassConfig cls{};
        cls.group_08 = 3;
        Host host;
        cp::PowerupRuntime runtime;
        runtime.manager = &manager;
        runtime.host = &host;
        cp::PowerupItem item;
        item.cls = &cls;
        item.word_00 = 17;
        auto& records = runtime.category_lists[0];
        records.emplace_back();
        gp::PowerupModifier* const unrelated = &records.back();
        for (int i = 0; i < 2; ++i) {
            records.emplace_back();
            auto& record = records.back();
            record.category = 0;
            record.item = &item;
            item.modifiers.push_back(&record);
        }
        gp::release_item_modifiers_008e6230(runtime, item);
        require(item.modifiers.empty(), "passive release skipped a real list node");
        require(records.size() == 1 && &records.front() == unrelated,
            "passive release erased an unrelated category record");
        require(item.word_00 == 17, "passive release changed active-use count");

        // The non-passive arm only clears the real record's owner pointer;
        // it leaves both lists and the item's active-use count in place.
        cls.group_08 = 2;
        records.emplace_back();
        auto& active = records.back();
        active.category = 0;
        active.item = &item;
        item.modifiers.push_back(&active);
        gp::release_item_modifiers_008e6230(runtime, item);
        require(active.item == nullptr && records.size() == 2
            && item.modifiers.size() == 1 && item.word_00 == 17,
            "non-passive release did more than clear the owner");

        // With no manager the body must not inspect even a missing class.
        runtime.manager = nullptr;
        item.cls = nullptr;
        active.item = &item;
        gp::release_item_modifiers_008e6230(runtime, item);
        require(active.item == &item && item.modifiers.size() == 1,
            "manager-absent release changed retained state");
        runtime.manager = &manager;
        cp::PowerupItem empty;
        gp::release_item_modifiers_008e6230(runtime, empty);
        std::puts("PASS: live modifier release: passive erase-next, unrelated record, non-passive detach, absent manager, empty list");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
