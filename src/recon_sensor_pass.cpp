#include "bsp/recon_sensor_pass.hpp"

#include <algorithm>

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

void recon_sensor_pass_step_008073c0(ReconSensorPassState& state, float dt,
                                     ReconSensorPassHost& host) {
    const std::size_t count = host.unit_count();
    state.reset(count);
    state.suppressed_by_network_role = false;

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

} // namespace bsp
