#include "bsp/recon_sensor_pass.hpp"

#include <algorithm>
#include <map>
#include <utility>

namespace bsp {

void ReconSensorPassState::reset(std::size_t unit_count) noexcept {
    unit_count_ = unit_count;
    sides_.clear();
    levels_.clear();
    values_.clear();
}

std::size_t ReconSensorPassState::slot(int side, std::size_t target, bool create) noexcept {
    if (target >= unit_count_) return static_cast<std::size_t>(-1);
    const auto it = std::find(sides_.begin(), sides_.end(), side);
    std::size_t row = 0;
    if (it == sides_.end()) {
        if (!create) return static_cast<std::size_t>(-1);
        row = sides_.size();
        sides_.push_back(side);
        levels_.resize((row + 1) * unit_count_, ReconDetectionLevel::none);
        values_.resize((row + 1) * unit_count_, 0.0f);
    } else {
        row = static_cast<std::size_t>(it - sides_.begin());
    }
    return row * unit_count_ + target;
}

bool ReconSensorPassState::side_covered(int side) const noexcept {
    return std::find(sides_.begin(), sides_.end(), side) != sides_.end();
}

ReconDetectionLevel ReconSensorPassState::level(int side, std::size_t target) const noexcept {
    if (target >= unit_count_) return kReconDetectionUnknownLevel;
    const auto it = std::find(sides_.begin(), sides_.end(), side);
    if (it == sides_.end()) return kReconDetectionUnknownLevel;
    const std::size_t row = static_cast<std::size_t>(it - sides_.begin());
    return levels_[row * unit_count_ + target];
}

float ReconSensorPassState::value(int side, std::size_t target) const noexcept {
    if (target >= unit_count_) return 0.0f;
    const auto it = std::find(sides_.begin(), sides_.end(), side);
    if (it == sides_.end()) return 0.0f;
    const std::size_t row = static_cast<std::size_t>(it - sides_.begin());
    return values_[row * unit_count_ + target];
}

namespace {
// Packet cc13-fix18: the (side, target) pairs 00803B50 cleared since the last rebuild.
std::vector<std::pair<int, std::size_t>>& forced_recon_clears();
} // namespace

void recon_sensor_pass_step_008073c0(ReconSensorPassState& state, float dt,
                                     ReconSensorPassHost& host) {
    const std::size_t count = host.unit_count();
    state.reset(count);
    state.suppressed_by_network_role = false;
    // This pass keeps no persistent records; it reads the forced table each time.
    forced_recon_clears().clear();

    // 00806871 / 0080686A: on a non-originating machine the whole sensor test
    // is skipped and every detection record keeps what it held.
    const int role = host.network_role();
    if (role == kGunneryReconNonOriginatingRole) {
        state.suppressed_by_network_role = true;
        return;
    }

    // Collect the sides that have at least one present observer-class unit.
    std::vector<int> sides;
    for (std::size_t i = 0; i < count; ++i) {
        if (!host.unit_present(i)) continue;
        if (!host.unit_is_observer_class(i)) continue;
        const int side = host.unit_side(i);
        if (std::find(sides.begin(), sides.end(), side) == sides.end())
            sides.push_back(side);
    }

    GunneryReconEnvironment env{};
    env.simplified_recon_multiplier = host.simplified_recon_multiplier();
    env.simplified_sonar_multiplier = host.simplified_sonar_multiplier();
    env.sensor_mask = kReconSensorMaskAll;  // 008068E0 passes 0FFh
    env.network_role = role;

    std::vector<GunneryReconObserver> observers;
    for (const int side : sides) {
        // 008068C2: only IsKindOf(05h) units of this side observe.
        observers.clear();
        for (std::size_t i = 0; i < count; ++i) {
            if (!host.unit_present(i)) continue;
            if (host.unit_side(i) != side) continue;
            if (!host.unit_is_observer_class(i)) continue;
            GunneryReconObserver observer{};
            observer.rows = host.unit_sensor_rows(i, observer.row_count);
            if (observer.rows == nullptr || observer.row_count == 0) {
                // 008048C1: [[observer+538h]+B4h] null, the early false.
                ++state.no_sensor_table;
                continue;
            }
            observer.is_observer_class = true;
            observer.category = host.unit_sensor_category(i);
            host.unit_world_xz(i, observer.world_x, observer.world_z);
            observer.heading = host.unit_heading(i);
            observer.party = side;
            observer.environment_factor = host.unit_environment_factor(i);
            observers.push_back(observer);
        }
        if (observers.empty()) continue;
        state.observers_admitted += observers.size();
        ++state.passes;

        // slot+28h. The rebuild runs one pass per slot and the slot index is
        // the observing party, which is what 00804B2F indexes the target's
        // detection record with.
        env.slot_index = side;

        for (std::size_t target = 0; target < count; ++target) {
            if (!host.unit_present(target)) continue;
            if (host.unit_side(target) == side) continue;  // relation own

            GunneryReconTarget subject{};
            subject.category = host.unit_sensor_category(target);
            host.unit_world_xz(target, subject.world_x, subject.world_z);
            subject.is_unit_base = host.unit_is_unit_base(target);
            subject.recon_modifier_sq = subject.is_unit_base
                ? host.unit_recon_modifier_sq(target)
                : kGunneryReconModifierSqDefault;
            subject.is_submarine = host.unit_is_submarine(target);
            subject.is_surface_target = host.unit_is_surface_target(target);
            subject.detection_forced = host.unit_detection_forced(target);
            subject.forced_level = host.unit_forced_level(target);
            ReconDetectionLevel forced = ReconDetectionLevel::none;
            if (forced_recon_level(side, target, forced)) {   // packet cc9_forced_recon_level
                subject.detection_forced = true;
                subject.forced_level = forced;
            }

            const GunneryReconContact contact = gunnery_recon_detect_00806840(
                observers.data(), observers.size(), subject, env,
                ReconRelation::enemy, 0.0f, dt);

            ++state.targets_tested;
            if (contact.sensor_pass_skipped) ++state.targets_skipped_forced;
            switch (contact.published_level) {
            case ReconDetectionLevel::identified: ++state.detected_identified; break;
            case ReconDetectionLevel::blip:       ++state.detected_blip; break;
            default:                              ++state.detected_none; break;
            }

            const std::size_t at = state.slot(side, target, true);
            if (at != static_cast<std::size_t>(-1)) {
                state.levels_[at] = contact.published_level;
                state.values_[at] = contact.detection_value;
            }
        }
    }
}

ReconDetectionRecord* ReconSensorPassState::record(int party, std::size_t unit) noexcept {
    if (party < 0 || party >= 3 || unit >= record_unit_count_) return nullptr;
    return &records_[static_cast<std::size_t>(party) * record_unit_count_ + unit];
}

namespace {

// 00806840's view of the state: the records, the delegate and the payload level.
class RecordPassAdapter final : public ReconRecordPassHost {
public:
    RecordPassAdapter(ReconSensorPassState& state, ReconSensorPassHost& host,
                      std::vector<ReconLevelTransition>& transitions)
        : state_(state), host_(host), transitions_(transitions) {}
    int network_role() override { return host_.network_role(); }
    ReconDetectionRecord& record(std::size_t unit, int slot) override {
        ReconDetectionRecord* r = state_.record(slot, unit);
        return r != nullptr ? *r : scratch_;
    }
    void level_changed(std::size_t unit, int slot, ReconDetectionLevel old_level,
                       ReconDetectionLevel new_level) override {
        transitions_.push_back({unit, slot, old_level, new_level});
    }
    void publish_level(std::size_t, ReconDetectionLevel) override {}

private:
    ReconSensorPassState& state_;
    ReconSensorPassHost& host_;
    std::vector<ReconLevelTransition>& transitions_;
    ReconDetectionRecord scratch_{};
};

} // namespace

void recon_sensor_pass_rebuild_008073c0(ReconSensorPassState& state, float dt,
                                        ReconSensorPassHost& host) {
    const std::size_t count = host.unit_count();
    if (state.record_unit_count_ != count) {
        const std::size_t old_count = state.record_unit_count_;
        if (kReconRecordsSurviveSpawnBound && old_count != 0 && count > old_count) {
            // A spawn: the existing units keep their records (packet cc9_s59_recon_growth).
            std::vector<ReconDetectionRecord> grown(3 * count, ReconDetectionRecord{});
            for (std::size_t party = 0; party < 3; ++party) {
                for (std::size_t unit = 0; unit < old_count; ++unit) {
                    grown[party * count + unit] = state.records_[party * old_count + unit];
                }
            }
            state.records_.swap(grown);
            ++state.record_growths;
        } else {
            state.records_.assign(3 * count, ReconDetectionRecord{});
            if (old_count != 0) ++state.record_resets;
        }
        state.record_unit_count_ = count;
    }
    state.reset(count);
    state.suppressed_by_network_role = false;
    state.transitions_.clear();
    ++state.record_rebuilds;

    // Packet cc13-fix18: 00803B50 from ClearForcedReconLevel (008AAB90), the only
    // writer that clears +10h. old = the effective level, +10h = 0, then the
    // listener with (old, +4h) when they differ, ahead of this pass's own calls.
    // Empty unless kLuaClearForcedReconLevelBound routes the native.
    for (const std::pair<int, std::size_t>& c : forced_recon_clears()) {
        ReconDetectionRecord* r = state.record(c.first, c.second);
        if (r == nullptr || !r->forced) continue;
        const ReconDetectionLevel old = recon_record_effective_level(*r);   // 00803B52
        r->forced = false;                                                   // 00803B66
        if (r->level != old) {                                               // 00803B64
            ReconLevelTransition t;
            t.unit = c.second;
            t.party = c.first;
            t.old_level = old;
            t.new_level = r->level;
            state.transitions_.push_back(t);
        }
    }
    forced_recon_clears().clear();

    // 00805CF0's writes, kept in the forced table by SetForcedReconLevel: +10h = 1
    // and +8h = level. 00803B50 (above) is the one writer that clears +10h.
    for (int party = 0; party < 3; ++party) {
        for (std::size_t u = 0; u < count; ++u) {
            ReconDetectionLevel level = ReconDetectionLevel::none;
            if (!forced_recon_level(party, u, level)) continue;
            ReconDetectionRecord& r = *state.record(party, u);
            r.forced = true;
            r.forced_level = level;
        }
    }

    GunneryReconEnvironment env{};
    env.simplified_recon_multiplier = host.simplified_recon_multiplier();
    env.simplified_sonar_multiplier = host.simplified_sonar_multiplier();
    env.sensor_mask = kReconSensorMaskAll;  // 008068E0 PUSH 0FFh
    env.network_role = host.network_role();

    RecordPassAdapter adapter(state, host, state.transitions_);
    ReconPassScratch scratch{};
    std::vector<std::size_t> own, enemy, neutral;
    std::vector<ReconPassObserver> observers;
    std::vector<ReconPassTarget> targets;
    const auto build_targets = [&](const std::vector<std::size_t>& list) {
        targets.clear();
        for (const std::size_t u : list) {
            ReconPassTarget t{};
            t.unit = u;
            t.subject.category = host.unit_sensor_category(u);
            host.unit_world_xz(u, t.subject.world_x, t.subject.world_z);
            t.subject.is_unit_base = host.unit_is_unit_base(u);
            t.subject.recon_modifier_sq = t.subject.is_unit_base
                ? host.unit_recon_modifier_sq(u) : kGunneryReconModifierSqDefault;
            t.subject.is_submarine = host.unit_is_submarine(u);
            t.subject.is_surface_target = host.unit_is_surface_target(u);
            const ReconDetectionRecord* r = state.record(env.slot_index, u);
            if (r != nullptr && !r->forced && r->value > 0.0f) ++state.record_carried_level;
            targets.push_back(t);
        }
        state.record_targets += targets.size();
    };

    for (int slot = 0; slot < 3; ++slot) {
        own.clear(); enemy.clear(); neutral.clear();
        host.scan_slot_lists_008065b0(slot, own, enemy, neutral);
        env.slot_index = slot;   // slot+28h

        // 008065B0's own arm: 00805AF0(1.0f, 1, null) on the unit's record for this slot.
        for (const std::size_t u : own) {
            ReconDetectionRecord* r = state.record(slot, u);
            if (r == nullptr) continue;
            const ReconRecordLevelChange change =
                recon_record_accumulate_00805af0(*r, 1.0f, true, kGunneryReconNoObserver);
            ++state.record_own_refreshes;
            if (change.changed) {
                state.transitions_.push_back({u, slot, change.old_level, change.new_level});
            }
        }

        // The own triple's flat list (slot+DD8h, 00804E10 at 0080753A).
        observers.clear();
        for (const std::size_t u : own) {
            ReconPassObserver entry{};
            entry.unit = u;
            entry.observer.is_observer_class = host.unit_is_observer_class(u);
            entry.observer.rows = host.unit_sensor_rows(u, entry.observer.row_count);
            if (entry.observer.rows == nullptr) entry.observer.row_count = 0;
            entry.observer.category = host.unit_sensor_category(u);
            host.unit_world_xz(u, entry.observer.world_x, entry.observer.world_z);
            entry.observer.heading = host.unit_heading(u);
            entry.observer.party = host.unit_side(u);
            entry.observer.environment_factor = host.unit_environment_factor(u);
            if (entry.observer.is_observer_class && entry.observer.rows == nullptr) {
                ++state.no_sensor_table;
            }
            observers.push_back(entry);
        }
        state.observers_admitted += observers.size();
        ++state.passes;

        // 00807552 (enemy buckets), then 00807570 (neutral buckets).
        build_targets(enemy);
        recon_run_sensor_pass_00806840(targets.data(), targets.size(), observers.data(),
                                       observers.size(), env, slot, dt, adapter, scratch);
        build_targets(neutral);
        recon_run_sensor_pass_00806840(targets.data(), targets.size(), observers.data(),
                                       observers.size(), env, slot, dt, adapter, scratch);
    }
    state.record_stale_gain_reads += scratch.stale_gain_reads;
    state.record_notifications += state.transitions_.size();

    // Every slot is covered; each (party, unit) answers its record's effective level.
    for (int party = 0; party < 3; ++party) {
        for (std::size_t u = 0; u < count; ++u) {
            const std::size_t at = state.slot(party, u, true);
            const ReconDetectionRecord& r = *state.record(party, u);
            const ReconDetectionLevel level = recon_record_effective_level(r);
            state.levels_[at] = level;
            state.values_[at] = r.value;
            ++state.targets_tested;
            switch (level) {
            case ReconDetectionLevel::identified: ++state.detected_identified; break;
            case ReconDetectionLevel::blip:       ++state.detected_blip; break;
            default:                              ++state.detected_none; break;
            }
        }
    }
}

namespace {
std::map<std::pair<int, std::size_t>, ReconDetectionLevel>& forced_recon_table() {
    static std::map<std::pair<int, std::size_t>, ReconDetectionLevel> table;
    return table;
}
std::vector<std::pair<int, std::size_t>>& forced_recon_clears() {
    static std::vector<std::pair<int, std::size_t>> clears;
    return clears;
}
} // namespace

// 00805CF0 also unregisters the record's observer (+14h, when its target +28h is set:
// 006952A0, then +28h = 0) and, when the effective level changes, calls the change
// listener [+2Ch] slot 0 with (+30h, old, new). BLOCKED (packet D4): the host keeps no
// per-target recon record, observer or listener; this table holds the forced level only.
void set_forced_recon_level_00805cf0(std::size_t target, int side, int level) {
    // The record's int level: 0 none, 1 blip, 2 identified (bsp::ReconDetectionLevel).
    const ReconDetectionLevel value = level <= 0 ? ReconDetectionLevel::none
        : level == 1 ? ReconDetectionLevel::blip : ReconDetectionLevel::identified;
    forced_recon_table()[std::make_pair(side, target)] = value;
}

bool forced_recon_level(int side, std::size_t target, ReconDetectionLevel& out) {
    const auto& table = forced_recon_table();
    const auto it = table.find(std::make_pair(side, target));
    if (it == table.end()) return false;
    out = it->second;
    return true;
}

// Packet cc13-reconclear: the world teardown, under kForcedReconMissionClearCc13Bound.
void clear_forced_recon_levels() {
    forced_recon_table().clear();
    forced_recon_clears().clear();
}

bool clear_forced_recon_level_00803b50(std::size_t target, int side) {
    if (forced_recon_table().erase(std::make_pair(side, target)) == 0) return false;
    forced_recon_clears().emplace_back(side, target);
    return true;
}

} // namespace bsp
