#include "bsp/game_ai_forced_rules.hpp"
#include <cstring>
#include <cstdio>

int main() {
    bsp::game::detail::ai_forced_rule_tables().load();
    auto& tables = bsp::game::detail::ai_forced_rule_tables();
    const unsigned expected_count[7] = {18, 16, 16, 0, 14, 0, 0};
    if (tables.global_rules().size() != 0) return 1;
    unsigned total = 0;
    for (int mode = 0; mode != 7; ++mode) {
        const auto actual = tables.mode_rules(mode);
        const auto& source = bsp::ai_shipped_forced_rules(mode);
        if (actual.size() != expected_count[mode] || actual.size() != source.size()) return 2;
        for (std::size_t i = 0; i != actual.size(); ++i) {
            if (std::memcmp(&actual[i], &source[i], sizeof(source[i])) != 0) return 3;
            ++total;
        }
        bsp::game::detail::ai_forced_rule_tables().load();
        const auto again = tables.mode_rules(mode);
        if (actual.data != again.data || actual.size() != again.size()) return 4;
    }
    struct Subject final : bsp::AiForcedRuleSubject {
        int type;
        explicit Subject(int value) : type(value) {}
        int class_id() const override { return -1; }
        bool is_type(int query) const override { return type == query; }
    } attacker(0x0c), target(0x1c);
    float weight = 0;
    int index = -1;
    if (!bsp::ai_forced_rule_scan_00a31db0(tables.mode_rules(0), attacker, target,
            false, weight, index) || weight != 0.18f || index != 6) return 5;
    if (!bsp::ai_forced_rule_scan_00a31db0(tables.mode_rules(0), attacker, target,
            true, weight, index) || weight != 0.18f || index != 7) return 6;
    if (!bsp::ai_forced_rule_scan_00a31db0(tables.mode_rules(1), attacker, target,
            false, weight, index) || weight != 0.12f || index != 6) return 7;
    std::printf("PASS: %u native rows, all seven mode tables, idempotent startup, native-table scans\n", total);
}
