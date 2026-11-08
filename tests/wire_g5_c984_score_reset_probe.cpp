// One regression case for 00915760's base-record boundary and shared runtime
// award ownership. Compile as Win32 without NDEBUG and link bsp_core.lib.
#include "bsp/cc13_seg59b.hpp"
#include "bsp/mission_result.hpp"

#include <cassert>
#include <cstdio>

int main() {
    bsp::MissionScoringManagerState manager;
    manager.commit_slot = 3;
    manager.ignored_kill_entities_1488.insert(42);
    manager.slots[2].ranking_04 = 9;
    auto& score = manager.slots[3];
    score.mission_completed_00 = 1;
    score.difficulty_08 = 2;
    score.used_slot_250 = 3;
    score.score_maps[0]["mission"] = 71;
    score.player_damages_9c[1][2][3] = 4;
    score.objectives_204[0]["objective"] = {"display", 5};
    score.checkpoint_pending_24c = true;
    // These are outside the native clear's write set, despite sharing the
    // persistence model. A blanket `score = {}` would lose them.
    score.completed_by_difficulty_26c = {1, 2, 3};
    score.scores_by_difficulty_278 = {10, 20, 30};
    score.count_284 = 6;
    manager.runtime_awards[3]["GA_AE"] = 7;
    auto& runtime = manager.runtime_slots[3];
    runtime.unit_ids[0].push_back(8);
    runtime.kill_records_1bc[9] = {};
    runtime.messages[0] = "condition";

    bsp::cc13_seg59b::mission_score_record_clear_00915760(manager, 3);

    assert(score.mission_completed_00 == 0 && score.difficulty_08 == 0);
    assert(score.used_slot_250 == 0 && !score.checkpoint_pending_24c);
    assert(score.score_maps[0].empty() && score.player_damages_9c.empty());
    assert(score.objectives_204[0].empty());
    assert(manager.runtime_awards[3].empty());
    assert(runtime.unit_ids[0].empty() && runtime.kill_records_1bc.empty());
    assert(runtime.messages[0].empty());
    assert((score.completed_by_difficulty_26c == std::array<int, 3>{1, 2, 3}));
    assert((score.scores_by_difficulty_278 == std::array<int, 3>{10, 20, 30}));
    assert(score.count_284 == 6 && manager.slots[2].ranking_04 == 9);
    assert(manager.commit_slot == 3 && manager.ignored_kill_entities_1488.count(42) == 1);
    std::puts("c984 score reset: shared state cleared; extended fields and neighbors preserved");
}
