#pragma once

// Rule (c) of the recon contact lists: the sensor pass, as a sequence a host
// can run every tick.
//
// Packet cc8_recon_sensor_pass_rule_c. docs/RECON_SENSOR_PASS.md.
//
// The rules themselves are NOT reconstructed here. 008048A0 and 00806840 were
// projected by packets cc2_recon_slot_lists and cc7_gunnery_recon_detection and
// live in bsp/gunnery_recon_detection.hpp; until this packet they had no caller
// anywhere in the tree, which is why docs/RECON_SLOT_LISTS.md rule (c) was
// labelled absent and every rule-(a)+(b) member counted as identified. This
// header is the missing driver: the 008073C0 loop that runs 00806840 for each
// side over its own observers, and a small state object that keeps the answer
// where both the gunnery contact sweep and the ship AI's union-list read can
// see it.
//
// Native control flow, from docs/GUNNERY_RECON_DETECTION.md: 008073C0 walks the
// 97 class buckets, runs each through 00806840 (00807556 enemy, 00807574
// neutral), flattens them at 00807615 and drains once at 00807634. This driver
// keeps the per-target composition that bsp/gunnery_recon_detection.hpp already
// chose, so the drain runs per target rather than once. That is a composition
// difference and it is recorded in the doc's uncertainties, not a rule change.

#include <cstddef>
#include <vector>

#include "bsp/gunnery_recon_detection.hpp"
#include "bsp/recon_slot_lists.hpp"
#include "bsp/sensor_tables.hpp"

namespace bsp {

// Everything 008048A0 and 00806840 read that this process must supply. One
// method per native read, none with a default.
struct ReconSensorPassHost {
    virtual ~ReconSensorPassHost() = default;

    virtual std::size_t unit_count() = 0;

    // Rule (b): +5Ch set and +5Dh / +5Eh / +60h clear, and the unit still in
    // the world registry. A unit that fails it is neither observer nor target.
    virtual bool unit_present(std::size_t index) = 0;

    // unit+54h, the party.
    virtual int unit_side(std::size_t index) = 0;

    // 008068C2 IsKindOf(05h): only these are admitted as observers.
    virtual bool unit_is_observer_class(std::size_t index) = 0;

    // 008048D1 IsKindOf(05h) on the TARGET: gates reading the class signature.
    virtual bool unit_is_unit_base(std::size_t index) = 0;

    // 0080494B IsKindOf(08h) and 00804987 00922DC0. Both must hold for the
    // sonar penalty.
    virtual bool unit_is_submarine(std::size_t index) = 0;
    virtual bool unit_is_surface_target(std::size_t index) = 0;

    // unit->[+1E4h]->vtable[1](). The five installed getters are constants:
    // 0074E190 XOR EAX,EAX (air), 006DFD20 MOV EAX,1 (surface), 00852B90 for
    // the three submarine states, 0085EA40 (underwater) and 004F1740 MOV EAX,6
    // (unclassified).
    virtual SensorCategory unit_sensor_category(std::size_t index) = 0;

    // unit+FCh and unit+104h. Height is never read by 008048A0.
    virtual void unit_world_xz(std::size_t index, float& x, float& z) = 0;

    // unit->vtable[50h](), radians. Only a bearing-limited entry reads it.
    virtual float unit_heading(std::size_t index) = 0;

    // [[unit+538h]+B8h], the SQUARE of the class's ReconModifier Lua field.
    // Supply kGunneryReconModifierSqDefault when the class row has none.
    virtual float unit_recon_modifier_sq(std::size_t index) = 0;

    // [[unit+538h]+B4h] + 8, the 56 addressable rows. Null when the unit's
    // class has no sensor table, which is 008048A0's early false at 008048C1.
    virtual const GunneryReconSensorRow* unit_sensor_rows(std::size_t index,
                                                          std::size_t& count) = 0;

    // 008E6430(0Ch, observer) when [00E0C978] and [[00F88C30]+118h] are both
    // set, else 1.0f.
    virtual float unit_environment_factor(std::size_t index) = 0;

    // det+10h at 00806883: the sensor test is skipped and forced_level is
    // published instead.
    virtual bool unit_detection_forced(std::size_t index) = 0;
    virtual ReconDetectionLevel unit_forced_level(std::size_t index) = 0;

    // [game+21C4h]+74h and +78h, both 1.0f from 00444D20 unless a mission
    // script's Lua setter changed them.
    virtual float simplified_recon_multiplier() = 0;
    virtual float simplified_sonar_multiplier() = 0;

    // [00E188A8]+1FE4h. The pass is skipped entirely when this is
    // kGunneryReconNonOriginatingRole.
    virtual int network_role() = 0;
};

// What one pass left, indexed by (side, target). Sides are kept dense in the
// order they were first seen, because a party id is not an array index.
class ReconSensorPassState {
public:
    void reset(std::size_t unit_count) noexcept;

    // kReconDetectionUnknownLevel until a pass has run for that side.
    ReconDetectionLevel level(int side, std::size_t target) const noexcept;
    float value(int side, std::size_t target) const noexcept;

    // True when a pass has run for this side at all. A caller that asks about a
    // side the pass never saw must not read `level` as a negative.
    bool side_covered(int side) const noexcept;

    std::size_t unit_count() const noexcept { return unit_count_; }

    // Census, for the run log.
    unsigned long long passes{0};
    unsigned long long observers_admitted{0};
    unsigned long long targets_tested{0};
    unsigned long long targets_skipped_forced{0};
    unsigned long long detected_blip{0};
    unsigned long long detected_identified{0};
    unsigned long long detected_none{0};
    unsigned long long no_sensor_table{0};

    // Set by the step when the whole pass was skipped for the network role.
    bool suppressed_by_network_role{false};

private:
    friend void recon_sensor_pass_step_008073c0(ReconSensorPassState&, float,
                                                ReconSensorPassHost&);
    std::size_t slot(int side, std::size_t target, bool create) noexcept;

    std::size_t unit_count_{0};
    std::vector<int> sides_{};
    std::vector<ReconDetectionLevel> levels_{};
    std::vector<float> values_{};
};

// The level a caller must assume for a target the pass has not covered. It is
// `identified`, the permissive answer the tree used before rule (c) ran, so a
// side with no observers never becomes blind by accident.
inline constexpr ReconDetectionLevel kReconDetectionUnknownLevel =
    ReconDetectionLevel::identified;

// 008073C0's loop over 00806840, once per tick.
//
// For each side that has at least one present observer-class unit, the step
// builds that side's observer array and runs every present unit of a different
// side through gunnery_recon_detect_00806840, storing the published level.
// `prior_value` is 0.0f for every target, which is what the native rebuild
// guarantees by zeroing the records at 00807480..0080749A before the pass.
//
// Own-side units are not tested: ReconRelation::own is never filtered, so a
// side always knows its own units and the union list admits them by rule (a).
void recon_sensor_pass_step_008073c0(ReconSensorPassState& state, float dt,
                                     ReconSensorPassHost& host);

} // namespace bsp
