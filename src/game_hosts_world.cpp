// bsp_game.exe milestone 2i: the world's per-frame entity pass.
//
// Milestone 2f recorded call 3 of the in-mission subsystem tick because nothing
// walked any entity; milestone 2h created 32 units and still recorded it,
// because the chain the walk reads hangs off a world object construct_world
// 004de610 does not build here. This file owns that chain instead, so the
// recovered walk 00904bf0 runs over the created units, and the pass 00904600
// that closes it and the eight lists 004c3cb0 rebuilds run beside it.

#include "bsp/game_hosts_world.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"

#include "bsp/in_mission_subsystem_tick.hpp"
#include "bsp/local_player_unit_lists.hpp"
#include "bsp/unit_instance.hpp"
#include "bsp/world_entity_update.hpp"

#include <cstdio>
#include <stdexcept>
#include <vector>

namespace bsp::game {

struct GameWorldHost::Impl {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in) : log(log_in), units(units_in) {}

    GameHostLog& log;
    GameUnitsHost& units;
    GameWorldSummary summary{};

    // The chain 009037f0's first header holds. An entry is the index of a unit
    // in the units host, which is the creation order the instantiate pass used.
    std::vector<std::size_t> chain;
    // The second header, world+8h. 009037f0 allocates it and nothing this
    // packet read ever touches it again (docs/WORLD_ENTITY_UPDATE.md).
    std::vector<std::size_t> second_chain;

    // world+4B0h, the matrix-interpolator list.
    std::vector<bsp::MatrixInterpolatorRecord> interpolators;
    float clock{0.0f};  // 00f876a4, the mission clock the pass works off

    bsp::LocalPlayerUnitLists lists{};
    bsp::UnitListsGate gate{};
    bool logged_empty_interpolators{false};
    bool logged_walk_sources{false};

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }
};

namespace {

// ---------------------------------------------------------------------------
// bsp::MatrixInterpolatorHost, the four call sites inside 00904600
// ---------------------------------------------------------------------------

class MatrixInterpolatorBinding final : public bsp::MatrixInterpolatorHost {
public:
    explicit MatrixInterpolatorBinding(GameWorldHost::Impl& owner) : owner_(owner) {}
    bool entity_active(std::uint32_t) override { return true; }  // entity+5Ch
    void entity_set_local_matrix(std::uint32_t, const float[16]) override {
        // The entity vtable slot 88h. docs/WORLD_ENTITY_UPDATE.md records that
        // slot 88h's body was not read at all; packet cc_entity_matrix owns it.
        owner_.record("MatrixInterpolator::entity_set_local_matrix", 0x00904ade);
    }
    void entity_invalidate_subtree_pose(std::uint32_t) override {
        owner_.record("MatrixInterpolator::invalidate_subtree_pose", 0x0042ed50);
    }
    void entity_refresh(std::uint32_t) override {
        owner_.record("MatrixInterpolator::entity_refresh", 0x00955970);
    }

private:
    GameWorldHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::WorldEntityUpdateHost, the two call sites inside 00904bf0
// ---------------------------------------------------------------------------

class WorldEntityUpdateBinding final : public bsp::WorldEntityUpdateHost {
public:
    explicit WorldEntityUpdateBinding(GameWorldHost::Impl& owner) : owner_(owner) {}

    void update_entity(std::size_t index, float scaled_delta) override {
        // 00904c18: entity->vtable[0DCh](scaledDelta), which for a unit is
        // 008255b0. The chain position is the units host's index.
        if (index >= owner_.chain.size()) return;
        owner_.units.update_entity_008255b0(owner_.chain[index], scaled_delta);
        ++owner_.summary.entities_updated;
    }

    void update_timed_attachments_00904600(float scaled_delta) override {
        // 00904c2b. The float the walk forwards is dead in the body: 00904600
        // works entirely off the mission clock at 00f876a4.
        static_cast<void>(scaled_delta);
        MatrixInterpolatorBinding host(owner_);
        const bsp::MatrixInterpolatorPassResult result
            = bsp::run_matrix_interpolator_pass_00904600(host, owner_.interpolators,
                owner_.clock);
        static_cast<void>(result);
        ++owner_.summary.interpolator_passes;
        owner_.summary.interpolator_records = owner_.interpolators.size();
        if (!owner_.logged_empty_interpolators) {
            owner_.logged_empty_interpolators = true;
            owner_.log.notef("world+4B0h holds %zu matrix interpolator record(s): the only "
                "producer is 00905080, the `AddMatrixInterpolator` binding, and no script of "
                "this mission called it", owner_.interpolators.size());
        }
        owner_.done("World::update_matrix_interpolators", 0x00904600u);
    }

private:
    GameWorldHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::LocalPlayerUnitListsHost, the call sites of 004c3cb0
// ---------------------------------------------------------------------------

class LocalPlayerUnitListsBinding final : public bsp::LocalPlayerUnitListsHost {
public:
    explicit LocalPlayerUnitListsBinding(GameWorldHost::Impl& owner) : owner_(owner) {
        // Walk 0's source: the units the instantiate pass created. See the
        // header comment; this is the one stand-in of this file.
        for (std::size_t index = 0; index < owner_.units.count(); ++index) {
            walk0_.push_back(static_cast<bsp::UnitRef>(index + 1));
        }
    }

    void clear_all_lists_004bfdf0() override {
        bsp::clear_all_unit_lists_004bfdf0(owner_.lists);
        owner_.done("UnitLists::clear_all", 0x004bfdf0u);
    }

    const std::vector<bsp::UnitRef>& registry_walk_units(bsp::UnitRegistryWalk walk) override {
        if (walk == bsp::UnitRegistryWalk::kWalk0) {
            if (!owner_.logged_walk_sources) {
                owner_.logged_walk_sources = true;
                owner_.log.notef("unit list stand-in: the local player's unit registry at "
                    "[[game+18CCh + slot*4]+30h] is not filled in this process and "
                    "docs/LOCAL_PLAYER_UNIT_LISTS.md does not settle what each of its five "
                    "triples holds, so walk 0 is handed the %zu created unit(s) and the "
                    "heads of walks 1 and 2 are recorded", owner_.units.count());
            }
            owner_.done("UnitLists::registry_walk0", 0x004c3cf8u);
            return walk0_;
        }
        if (walk == bsp::UnitRegistryWalk::kWalk1) {
            owner_.record("UnitLists::registry_walk1", 0x004c3d61u);
        } else {
            owner_.record("UnitLists::registry_walk2", 0x004c3eb4u);
        }
        return empty_;
    }

    bsp::UnitListFilterFlags unit_filter_flags(bsp::UnitRef unit) override {
        const std::size_t index = static_cast<std::size_t>(unit) - 1;
        bsp::SceneNodeFlags flags;
        bool pending_destroy;
        if (!owner_.units.unit_scene_node_flags(index, flags) ||
            !owner_.units.unit_pending_destroy_0060(index, pending_destroy)) {
            throw std::logic_error("registered unit has no canonical scene flags");
        }
        return {flags.active, flags.torn_down, pending_destroy, flags.destroyed};
    }

    bool unit_is_kind_of(bsp::UnitRef unit, int class_id) override {
        const std::size_t index = static_cast<std::size_t>(unit) - 1;
        return owner_.units.unit_is_kind_of(index, class_id);
    }

    bool unit_in_local_objective_set_008ddf90(bsp::UnitRef) override {
        // 008ddf90 with ECX = the local player's SzurkeNyil set at
        // game+21A4h + team*4. The eight sets are the world's.
        owner_.record("UnitLists::objective_set_contains", 0x008ddf90u);
        return false;
    }

    void append_00484540(bsp::LocalPlayerUnitList list, bsp::UnitRef unit) override {
        bsp::unit_list_push_back_00484540(owner_.lists[static_cast<std::size_t>(list)], unit);
    }

    void append_all_004c2be0(bsp::LocalPlayerUnitList dst,
        bsp::LocalPlayerUnitList src) override {
        bsp::unit_list_append_all_004c2be0(owner_.lists[static_cast<std::size_t>(dst)],
            owner_.lists[static_cast<std::size_t>(src)]);
    }

private:
    GameWorldHost::Impl& owner_;
    std::vector<bsp::UnitRef> walk0_;
    std::vector<bsp::UnitRef> empty_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameWorldHost
// ---------------------------------------------------------------------------

GameWorldHost::GameWorldHost(GameHostLog& log, GameUnitsHost& units)
    : impl_(std::make_unique<Impl>(log, units)) {}

GameWorldHost::~GameWorldHost() = default;

void GameWorldHost::build_entity_chains_009037f0() {
    Impl& host = *impl_;
    host.chain.clear();
    for (std::size_t index = 0; index < host.units.count(); ++index) {
        host.chain.push_back(index);
    }
    host.second_chain.clear();
    host.summary.chains_built = true;
    host.summary.chain_entities = host.chain.size();
    host.done("World::allocate_entity_chains", 0x009037f0u);
    host.log.notef("world entity chain: [[world+4]] holds %zu entity(ies), linked by +38h; "
        "the second header at world+8h is allocated and has no reader in anything "
        "docs/WORLD_ENTITY_UPDATE.md read", host.chain.size());
}

void GameWorldHost::run_world_entity_update_00904bf0(float scaled_delta) {
    Impl& host = *impl_;
    host.clock += scaled_delta;
    std::vector<bool> active;
    active.reserve(host.chain.size());
    for (std::size_t entity : host.chain) active.push_back(host.units.unit_active(entity));
    WorldEntityUpdateBinding binding(host);
    const std::size_t visited
        = bsp::update_world_entities_00904bf0(active, binding, scaled_delta);
    ++host.summary.walks;
    host.summary.entities_walked += visited;
    host.done("InMissionTick::update_world_entities", 0x00904bf0u);
}

void GameWorldHost::build_local_player_unit_lists_004c3cb0() {
    Impl& host = *impl_;
    ++host.summary.list_guard_calls;
    host.gate.slot_index = 0;
    LocalPlayerUnitListsBinding binding(host);
    const bool ran = bsp::build_local_player_unit_lists_004c3cb0(host.gate, binding);
    host.done("InMissionTick::unit_lists", 0x004c3cb0u);
    if (!ran) return;
    host.summary.lists_built = true;
    host.summary.list_walk0_units = host.units.count();
    for (std::size_t index = 0; index < bsp::kLocalPlayerUnitListCount; ++index) {
        host.summary.list_counts[index] = host.lists[index].size();
    }
    host.log.notef("local-player unit lists rebuilt: walk0_ships=%zu walk0_rest=%zu ships=%zu "
        "squadrons=%zu airfields=%zu shipyards=%zu land_forts=%zu merged=%zu",
        host.summary.list_counts[0], host.summary.list_counts[1], host.summary.list_counts[2],
        host.summary.list_counts[3], host.summary.list_counts[4], host.summary.list_counts[5],
        host.summary.list_counts[6], host.summary.list_counts[7]);
}

void GameWorldHost::log_frame(unsigned long long mission_frame) {
    Impl& host = *impl_;
    host.log.notef("  world frame %-4llu entities=%zu walked=%llu updated=%llu "
        "interpolators=%zu lists{ships=%zu rest=%zu merged=%zu}", mission_frame,
        host.chain.size(), host.summary.entities_walked, host.summary.entities_updated,
        host.summary.interpolator_records, host.summary.list_counts[0],
        host.summary.list_counts[1], host.summary.list_counts[7]);
}

void GameWorldHost::report() {
    Impl& host = *impl_;
    host.log.notef("summary world walk entities=%zu walks=%llu walked=%llu updated=%llu "
        "interpolator_passes=%llu records=%zu", host.chain.size(), host.summary.walks,
        host.summary.entities_walked, host.summary.entities_updated,
        host.summary.interpolator_passes, host.summary.interpolator_records);
    host.log.notef("summary world unit lists built=%d guard_calls=%llu counts="
        "%zu/%zu/%zu/%zu/%zu/%zu/%zu/%zu", host.summary.lists_built ? 1 : 0,
        host.summary.list_guard_calls, host.summary.list_counts[0],
        host.summary.list_counts[1], host.summary.list_counts[2], host.summary.list_counts[3],
        host.summary.list_counts[4], host.summary.list_counts[5], host.summary.list_counts[6],
        host.summary.list_counts[7]);
}

const GameWorldSummary& GameWorldHost::summary() const noexcept { return impl_->summary; }

}  // namespace bsp::game
