// One focused activation/cooldown sequence through the production adapters.
#include "../src/game_hosts_mission_result.cpp"
#include "../src/game_hosts_powerups.cpp"
#include <cstdio>

namespace {
namespace cp = bsp::cc13_powerup;
namespace gp = bsp::game_powerups;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
struct Host final : cp::PowerupManagerHost {
    bsp::MissionScoringManagerState scoring;
    cp::PowerupRuntime* runtime{};
    float now{20.0f};
    unsigned awards{}, rebuilds{};
    std::vector<bsp::PowerUpSlot> local_records;
    [[noreturn]] static void unexpected() { throw std::logic_error("unexpected fixture provider"); }
    float clock_00f876a4() override { return now; }
    std::int32_t local_player_slot_18ec() override { return 0; }
    std::int32_t session_word_1fe4() override { return 0; }
    bool powerups_enabled_00e0c978() override { return true; }
    void award_active_powerup_use(std::int32_t slot) override {
        // A still-default cooldown proves the award runs before publication.
        if (awards == 0)
            require(runtime->other_records[slot][0].source == nullptr,
                "cooldown published before the scoring provider");
        bsp::game::NonlocalPowerupAwardBinding binding(scoring, 0, 0, 13);
        binding.grant(slot);
        ++awards;
    }
    bool powerup_screen_shown_00e198c4() override { return true; }
    void powerup_screen_rebuild_lists_0061b0f0() override { ++rebuilds; }
    std::vector<bsp::PowerUpSlot>& active_slot_records() override { return local_records; }
    void report_powerup_009789a0(std::int32_t, const cp::PowerupItem&, const std::string&) override { unexpected(); }
    bool hint_has_been_shown_0068e600(const std::string&) override { unexpected(); }
    void trigger_hint_00690fd0(const std::string&, std::int32_t) override { unexpected(); }
    void resolve_air_support_classes_008e34f0(bsp::PowerupClassConfig&) override { unexpected(); }
    void send_powerup_message67(std::int32_t, const cp::PowerupItem&, std::int32_t) override { unexpected(); }
    void send_powerup_message68(const gp::PowerupModifier&, std::int32_t) override { unexpected(); }
    void set_active_slot_notify_gate(bool) override { unexpected(); }
};
}

int main() {
    try {
        // Explicit fixture inputs; production takes these from the loaded
        // descriptor, actual AI inventory slot and mission frame clock.
        bsp::PowerupConfigOwner manager(bsp::PowerupOwnerAllocationWords{});
        bsp::PowerupClassConfig cls{};
        cls.group_08 = 2;
        cls.duration_7c = 5.0f;
        cls.cooldown_80 = 10.0f;
        cls.target_type_84 = 4;
        cls.multipliers_8c.fill(1.0f);
        cls.multipliers_8c[0] = 0.75f;
        Host host;
        cp::PowerupRuntime runtime;
        runtime.manager = &manager;
        runtime.host = &host;
        host.runtime = &runtime;
        runtime.inventories[1].emplace_back();
        auto& item = runtime.inventories[1].back();
        item.cls = &cls;
        item.use_limit = 3;
        gp::use_item_008eada0(runtime, item, 1, nullptr, nullptr);
        const auto& cooldown = cp::slot_record_00617030(runtime, 1, &cls);
        require(host.scoring.runtime_awards[1].at("GA_AE") == 1
            && host.scoring.runtime_awards[0].empty(), "GA_AE wrote the wrong live scoring tree");
        require(cooldown.source == &cls && cooldown.absolute_start == 25.0f
            && cooldown.absolute_expiry == 30.0f && !cooldown.notified
            && cooldown.subject == nullptr && !cooldown.relative_deadline
            && cooldown.source_offset == 10.0f, "cooldown record was not published from the body");
        require(item.use_limit == 2 && item.word_00 == 1
            && runtime.category_lists[0].size() == 1 && host.rebuilds == 1,
            "activation tail repeated or skipped inventory/HUD state");
        host.now = 25.0f;
        gp::use_item_008eada0(runtime, item, 1, nullptr, nullptr);
        require(host.awards == 1 && item.use_limit == 2 && host.rebuilds == 1,
            "cooldown did not block a second activation");

        // Expire the first modifier before the actual deadline permits reuse.
        gp::expire_modifier_008e8c30(runtime, &runtime.category_lists[0].front());
        host.scoring.runtime_awards[1]["GA_AE"] = 9;
        host.now = 30.0f;
        gp::use_item_008eada0(runtime, item, 1, nullptr, nullptr);
        require(host.scoring.runtime_awards[1].at("ga_ae") == 9
            && host.awards == 2 && item.use_limit == 1 && host.rebuilds == 2
            && cooldown.absolute_start == 35.0f && cooldown.absolute_expiry == 40.0f,
            "deadline reuse changed the once-only award or skipped the next cooldown");
        bool rejected = false;
        try {
            bsp::game::NonlocalPowerupAwardBinding local(host.scoring, 0, 0, 13);
            local.grant(0);
        } catch (const std::logic_error&) { rejected = true; }
        require(rejected && host.scoring.runtime_awards[0].empty(),
            "unbound local award fabricated success");
        std::puts("PASS: nonlocal active use, award-before-cooldown, retained slot, use limit, blocked repeat, deadline reuse, once-only map, local boundary");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
