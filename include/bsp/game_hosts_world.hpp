#pragma once
// bsp_game.exe milestone 2i: the world's per-frame entity pass, as process
// bindings.
//
// Addresses: 009037f0 (BSP_World_AllocateEntityChains, the two 0Ch chain
// headers 004de69c stores at world+4h and world+8h), 00904bf0
// (BSP_World_UpdateEntities, call 3 of the in-mission subsystem tick at
// 004c40ce) with the gate at 00904c00 and the sibling link at 00904c1a,
// 00904600 (BSP_World_UpdateMatrixInterpolators, the pass 00904c2b closes the
// walk with) and 00905080 (the `AddMatrixInterpolator` binding that fills its
// list), and 004c3cb0 (the eight local-player unit lists at game+1964h..+19B8h)
// with 004bfdf0, 00484540 and 004c2be0.
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::WorldEntityUpdateHost, bsp::MatrixInterpolatorHost or
// bsp::LocalPlayerUnitListsHost, satisfied either by a reconstruction already
// on main or by the explicit unimplemented policy in GameHostLog.
//
// One thing this file supplies is a stand-in and is recorded as one: the source
// of walk 0 of 004c3cb0. That walk reads the local player's unit registry at
// [[game+18CCh + slot*4]+30h], which nothing in this process fills, and
// docs/LOCAL_PLAYER_UNIT_LISTS.md does not settle what each of the registry's
// five lists holds. The executable hands walk 0 the units the instantiate pass
// created so the recovered classify chain runs over real class ids, and records
// the two other walk heads.
//
// Evidence: docs/WORLD_ENTITY_UPDATE.md, docs/WORLD_TIMED_ATTACHMENTS.md,
// docs/LOCAL_PLAYER_UNIT_LISTS.md, docs/IN_MISSION_SUBSYSTEM_TICK.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {

class GameHostLog;
class GameUnitsHost;

struct GameWorldSummary {
    bool chains_built{false};
    std::size_t chain_entities{0};       // [[world+4]] and its +38h links
    unsigned long long walks{0};         // 00904bf0 calls
    unsigned long long entities_walked{0};
    unsigned long long entities_updated{0};  // the +5Ch gate was set
    unsigned long long interpolator_passes{0};
    std::size_t interpolator_records{0};
    bool lists_built{false};             // 004c3cb0 ran its body once
    unsigned long long list_guard_calls{0};
    std::size_t list_counts[8]{};
    std::size_t list_walk0_units{0};
};

// The world object's entity half, owned for the whole run because 009037f0
// allocates the chain headers once, at load time.
class GameWorldHost {
public:
    GameWorldHost(GameHostLog& log, GameUnitsHost& units);
    ~GameWorldHost();
    GameWorldHost(const GameWorldHost&) = delete;
    GameWorldHost& operator=(const GameWorldHost&) = delete;

    // 009037f0 at 004de69c, over the units the instantiate pass created. The
    // world object itself is not built: construct_world 004de610 is a load
    // record, and what this owns is the chain the walk reads.
    void build_entity_chains_009037f0();

    // 00904bf0 at 004c40ce: the walk, then 00904600 at 00904c2b.
    void run_world_entity_update_00904bf0(float scaled_delta);

    // 004c3cb0 at 004c40b4: the guard, the clear, the three walks and the merge.
    void build_local_player_unit_lists_004c3cb0();

    // One per-frame line: entities walked, units updated, list counts.
    void log_frame(unsigned long long mission_frame);
    void report();

    const GameWorldSummary& summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
