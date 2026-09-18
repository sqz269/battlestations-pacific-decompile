#pragma once
// bsp_game.exe: the AI coordinator's fixed-step tick as a process binding.
//
// Addresses: 00A32350 BSP_AiController_Create (the coordinator, created at
// 004E1838 from BSP_Game_LoadMissionScene and at 00A373AF from the Lua binding
// BSP_LuaBinding_AICreate), 00A31730 (its constructor, which installs vtable
// 00D23168 on the tick-element node at coordinator+170h and registers it into
// fixed-step group 0 through 00875890 at 00A31766), 00A32D50 (slot +8h of that
// vtable: the twenty-byte routine Ghidra has no function for, which gates on
// the byte at 00E0E34C and calls the two passes), 00A2E720 (the composition
// pass, every fixed step), 00A182C0 -> 00A181A0 (the party think, per party
// every 3 to 5 s), 009FFE50 BSP_Ai_IsPartyAiEnabled (the party gate),
// 004BCA50 BSP_Game_GetEffectiveGameMode, 00A371A0 (the tuning block loaded by
// 00A335D0), 00A2CBD0 (the attack order) and 0077D600 (the order path the
// issued commands take in this process).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::AiGroupThinkHost or bsp::AiPlannerHost, both already on
// main in bsp/ai_group_think.hpp and bsp/ai_planners.hpp, satisfied either by
// the units the mission host already created or by the explicit unimplemented
// policy in GameHostLog.
//
// What this process does not hold, and what stands in for it:
//
//   * There is no AI group, brain or planner object, and no world entity
//     collection block at world+19CCh. The records below are this process's
//     own; every native pointer the two host interfaces pass is a pointer into
//     them, and no native layout is imitated.
//   * The five seed collections of phase 3 are stood in for by one collection
//     holding every created unit; collections 1 to 4 are empty. 00A2E835 is
//     recorded, and the census says how many units the one collection yielded.
//   * A group's AI command object (00A10890 MOVETOATTACK, 00A109B0
//     CAUTIOUSATTACK) is not built. 00A2CBD0's choice between them is taken and
//     recorded, and the consequence is issued to each member as a scene command
//     through the same 0046AAB0 -> 0077D600 path a scripted order takes, which
//     is what makes the plane task machinery and the ship order ring treat it
//     as a native order. The native per-member dispatch 00A2C790's
//     member->vtable[+114h] is unread, so this is a labelled substitution for
//     it and not a reconstruction of it.
//
// Evidence: docs/AI_COORDINATOR_TICK.md, docs/AI_GROUP_THINK.md,
// docs/AI_PLANNERS.md, docs/ENTITY_LUA_ORDER_PATH.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {

// game+21A4h..+21C0h: the eight per-player-slot objective sets 00A2C450 walks.
// The world builds them at 004DF917 and only the mission Lua fills them, through
// 008CD440 Objectives_Add, 008CDD60 Objectives_AddUnit and 008CE510
// Objectives_RemoveUnit. Those three live in the Lua host and the reader lives in
// the AI coordinator, so the table is a process-wide object the way the native's
// is a field of the world singleton. docs/MISSION_OBJECTIVES.md.
struct GameObjectiveSets {
    static constexpr std::size_t kSlotCount = 8;   // 008CE44B CMP EBP,0x21C4

    // One objective as the set holds it: its name and the units of its own
    // +20h list. The native record carries more (text, kind at +18h, state at
    // +1Ch); only what 008DDF90's membership walk reads is kept here.
    struct Objective {
        std::string name;
        std::vector<std::size_t> units;
    };
    std::vector<Objective> slots[kSlotCount];

    unsigned long long adds{0};          // 008E1F80 reached
    unsigned long long unit_adds{0};     // 008DF2B0 pushed a unit
    unsigned long long unit_removes{0};  // 008DFC00 dropped one
    unsigned long long rejected{0};      // a slot or a target the native drops

    void reset() noexcept;
    // 008E1F80: create the objective in this slot, or return the existing one.
    Objective* add_objective(int slot, const std::string& name);
    // 008DF2B0's push, after the liveness test its caller already made.
    bool add_unit(int slot, const std::string& name, std::size_t unit);
    // 008DFC00's removal.
    bool remove_unit(int slot, const std::string& name, std::size_t unit);
    // Every unit of every objective of this slot: what 00A2C450 walks.
    std::vector<std::size_t> units_in_slot(int slot) const;
    std::size_t total_units() const noexcept;
};

// The one instance, cleared when a mission scene is built.
GameObjectiveSets& game_objective_sets() noexcept;

class GameHostLog;
class GameUnitsHost;

// One side's share of what the coordinator did, for the run log.
struct GameAiPartyRow {
    int party{0};
    bool record_enabled{false};
    bool ai_enabled{false};        // 009FFE50's answer for this slot
    bool brain_created{false};
    unsigned long long thinks{0};
    unsigned long long groups_claimed{0};
    unsigned long long planner_ticks{0};
    unsigned long long attack_orders{0};
    unsigned long long commands_issued{0};
    unsigned long long commands_refused{0};
};

struct GameAiSummary {
    int game_mode{0};
    // 00A335D0's record, as 009FFC80 selected it. docs/AI_TUNING_GLOBALS.md.
    int tuning_mode{0};
    float tuning_merge_dist{0.0f};   // record +208h, AutoMerge_MergeDist
    float tuning_near_dist{0.0f};    // record +1D0h, FreeAttack_NearDist
    float tuning_far_dist{0.0f};     // record +1D4h, FreeAttack_FarDist
    float tuning_sticky{0.0f};       // record +1D8h, FreeAttack_ExistingTargetMul
    unsigned long long compose_passes{0};
    unsigned long long seed_candidates{0};
    unsigned long long groups_created{0};
    unsigned long long groups_destroyed{0};
    unsigned long long members_added{0};
    unsigned long long members_evicted{0};
    unsigned long long splits{0};
    unsigned long long splits_taken{0};   // 00A2E260 actually moved a subset
    unsigned long long auto_merges{0};
    unsigned long long proximity_merges{0};
    unsigned long long member_passes{0};
    // 00A2C790's per-member chain and the group's own command schedule.
    unsigned long long member_reports{0};        // command vt+24h, 00A0FC90
    unsigned long long member_descriptors{0};    // 0071EB60 answered
    unsigned long long member_scene_commands{0}; // 0071BE40 answered non-zero
    unsigned long long command_ticks{0};         // command vt+0Ch, on the 2-4 s draw
    unsigned long long commands_replaced{0};     // 00A2BD00 deleted an outgoing one
    unsigned long long commands_retargeted{0};   // 00A2DB80 phase A rebound one
    unsigned long long tick_orders{0};           // 00A02020 reached 0077D600
    unsigned long long tick_formation_requests{0};  // 0077C8D0
    unsigned long long tick_followers{0};        // 00A10DC0 walked one
    unsigned long long command_promotions{0};    // MOVETOATTACK became CLOSEATTACK
    unsigned long long close_members_served{0};      // 00A13B60 served a member
    unsigned long long close_attack_move_orders{0};  // 00E08F78 attackmove
    unsigned long long close_set_target_orders{0};   // 00E08EF8 settarget
    unsigned long long close_fallback_movetos{0};    // the no-candidate arm
    unsigned long long close_candidates_scored{0};
    unsigned long long party_think_calls{0};
    unsigned long long parties_thought{0};
    unsigned long long planner_ticks{0};
    unsigned long long planner_claims{0};
    unsigned long long planner_spawn_arms{0};   // no owned group: the tag spawn
    unsigned long long attack_orders{0};        // 00A2CBD0 reached
    unsigned long long attack_cautious{0};      // the 00A109B0 arm
    unsigned long long attack_movetoattack{0};  // the 00A10890 arm
    unsigned long long commands_issued{0};
    unsigned long long commands_refused{0};
    unsigned long long units_with_task{0};      // distinct units that got one
    // The plane squadron layer. docs/PLANE_SQUADRON_ENTITY.md.
    unsigned long long squadrons_built{0};        // 004F0AD0 + 007F4580 mode 1
    unsigned long long squadron_members{0};       // planes in a +3D0h array
    unsigned long long squadron_group_members{0}; // squadrons a group holds
    unsigned long long squadron_excluded{0};      // 007EDA90 answered true
    unsigned long long squadron_commands{0};      // a command landed on one
    unsigned long long squadron_member_orders{0}; // 007ECF80 reached a plane
    unsigned long long orders_suppressed{0};      // a duplicate re-issue
    // 00A2C450, the per-player-slot objective-set test. docs/AI_WORLD_SETS.md.
    unsigned long long world_set_queries{0};
    unsigned long long world_set_hits{0};
    unsigned long long objective_set_units{0};   // total across the eight sets
    // 00A0F810, the candidate target weight. docs/AI_TARGET_WEIGHT.md.
    unsigned long long weight_queries{0};
    unsigned long long weight_objective_hits{0};    // 008DDF90 answered true
    unsigned long long weight_fort_targets{0};      // the 009FE0B0 trio
    unsigned long long weight_non_command_targets{0}; // trio, not 1Ch
    float first_command_seconds{-1.0f};
};

// The coordinator for one mission run, owned for the whole run.
class GameAiCoordinatorHost {
public:
    GameAiCoordinatorHost(GameHostLog& log, GameUnitsHost& units);
    ~GameAiCoordinatorHost();
    GameAiCoordinatorHost(const GameAiCoordinatorHost&) = delete;
    GameAiCoordinatorHost& operator=(const GameAiCoordinatorHost&) = delete;

    // 00A32350 BSP_AiController_Create, whose 00A31730 constructor registers the
    // tick element. Runs once, after create_units, because phase 3 seeds from
    // the world entity collections and there is nothing to seed before then.
    void create_00a32350();

    // 00A32D50, slot +8h of vtable 00D23168. The float the wave passes is
    // discarded by both callees (RET 4 with no pushes), so the AI is time-blind
    // and reads the fixed-step clock; `step_seconds` only advances that clock.
    void fixed_step(float step_seconds);

    const GameAiSummary& summary() const noexcept;
    const std::vector<GameAiPartyRow>& party_rows() const noexcept;
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
