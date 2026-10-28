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

// Packet cc9_g51_recon_records (docs/RECON_SENSOR_PASS.md section "Records").
// True: recon_sensor_pass_rebuild_008073c0 replaces recon_sensor_pass_step_008073c0.
// Each of the three slots (004E0537 builds slots 0, 1 and 2 at load) runs its
// scan lists through recon_run_sensor_pass_00806840 against persistent per-(unit,
// party) records: the value carries across passes (the 00807490 reset walks only
// the class-18h squadron list), the own arm adds 1.0f each pass (008065B0), every
// party is covered, and the record notifications are kept for the `recon` channel.
// False: the per-target model from a zero value, sides covered only while they
// have a sensor-bearing observer.
inline constexpr bool kReconDetectionRecordsBound = true;  // ON: mechanism matches (RECON_SENSOR_PASS.md "Records", pairs)

// Packet cc9_s59_recon_growth (docs/RECON_SENSOR_PASS.md "Records survive a spawn").
// The image keeps one record per (unit, party) for the unit's life, so a new unit
// adds records and leaves every other record's value and level alone. True: when
// the host's unit count grows, the records are carried into the larger layout
// (unit indices are append-only) and only the new units start at zero. False: every
// record is zeroed on any count change, so the next pass re-notifies 0 -> level for
// every unit a side knows (each `recon` listener then fires on its first pass after
// any spawn), as before.
inline constexpr bool kReconRecordsSurviveSpawnBound = true;   // ON: RECON_SENSOR_PASS.md, pairs of 2026-10-18

// One delegate call, [+2Ch]->vtable[0](+30h, old, new), in pass order.
struct ReconLevelTransition {
    std::size_t unit{0};
    int party{0};
    ReconDetectionLevel old_level{ReconDetectionLevel::none};
    ReconDetectionLevel new_level{ReconDetectionLevel::none};
};

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

    // 0080749C..00807527 for one slot (kReconDetectionRecordsBound): the units the
    // scan files into the own, enemy and neutral arrays, each flattened in class-id
    // bucket order with the scan order inside a bucket, as 00804E10 (own, slot+DD8h)
    // and the 00807552 / 00807570 loops see them.
    virtual void scan_slot_lists_008065b0(int slot, std::vector<std::size_t>& own,
                                          std::vector<std::size_t>& enemy,
                                          std::vector<std::size_t>& neutral) = 0;
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

    // kReconDetectionRecordsBound census.
    unsigned long long record_rebuilds{0};
    unsigned long long record_targets{0};
    unsigned long long record_own_refreshes{0};
    unsigned long long record_carried_level{0};   // a target kept a level above what a zero start gives
    unsigned long long record_notifications{0};
    unsigned long long record_stale_gain_reads{0};
    unsigned long long record_growths{0};   // kReconRecordsSurviveSpawnBound carries
    unsigned long long record_resets{0};    // every record zeroed after the first layout

    // The delegate calls of the latest rebuild (kReconDetectionRecordsBound).
    const std::vector<ReconLevelTransition>& transitions() const noexcept { return transitions_; }

    // The persistent record, for 00805CF0's forced writes; null when out of range.
    ReconDetectionRecord* record(int party, std::size_t unit) noexcept;

private:
    friend void recon_sensor_pass_step_008073c0(ReconSensorPassState&, float,
                                                ReconSensorPassHost&);
    friend void recon_sensor_pass_rebuild_008073c0(ReconSensorPassState&, float,
                                                   ReconSensorPassHost&);
    std::size_t slot(int side, std::size_t target, bool create) noexcept;

    std::size_t unit_count_{0};
    std::vector<int> sides_{};
    std::vector<ReconDetectionLevel> levels_{};
    std::vector<float> values_{};
    std::size_t record_unit_count_{0};
    std::vector<ReconDetectionRecord> records_{};    // [party * record_unit_count_ + unit]
    std::vector<ReconLevelTransition> transitions_{};
};

// The level a caller must assume for a target the pass has not covered. It is
// `identified`, the permissive answer the tree used before rule (c) ran, so a
// side with no observers never becomes blind by accident.
inline constexpr ReconDetectionLevel kReconDetectionUnknownLevel =
    ReconDetectionLevel::identified;

// How often the native rebuild actually runs, and why a caller must not step
// this per frame.
//
// 008079B0 BSP_Recon_ServicePeriodicRefresh is the only periodic caller of
// 008073C0 (the other four are load, scene apply and two placement paths).
// It holds a countdown at [00F874B8]: 008079B6 subtracts the frame delta,
// 008079C8 returns while the result is still above zero, and 008079CE reloads
// it by adding the double at 00D7A2B0, which is 3.0, clamped at 0.0 by
// 008079EA. Only then does it walk the slot list at [00F874BC] and call
// 008073C0 on each.
//
// The pass's own dt is the measured gap, not the frame delta: 008073C1 loads
// the clock at [00F876A4], 008073CC subtracts the slot's last-pass stamp at
// slot+2Ch and 008073D6 stores the difference into slot+30h, which is the dt
// 00804B35 multiplies the gain by.
//
// This matters because the detection value does NOT carry across passes.
// 00807480..00807490 calls 00805BE0 BSP_Recon_ResetDetection on every record
// first, and 00805BF9 zeroes the value at record+0Ch while 00805BFE clears the
// level at +4h. So one pass has to reach a threshold on its own: the shipped
// `Gain` of 1.0 per second is halved to 0.5 by 00808626, and 0.5 * 3.0 = 1.5
// clears the 0.25 blip threshold and saturates the 1.0 cap, while 0.5 * 0.05
// (a 20 Hz frame) is 0.025 and can never clear anything. A host that steps
// this every frame publishes `none` for every target in the world.
inline constexpr float kReconSensorPassRefreshPeriod = 3.0f;

// 008073C0's loop over 00806840, once per refresh period.
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

// kReconDetectionRecordsBound: 008073C0 for slots 0, 1 and 2 over persistent
// records. Per slot: the own arm of 008065B0 (00805AF0(1.0f, 1, null) on each own
// record), then 00806840 over the enemy list (00807556) and the neutral list
// (00807574). levels()/values() then read the records' effective levels for every
// party. SUBSTITUTIONS, labelled: the class-18h squadron records the 00807490 loop
// resets are not kept here (the host folds squadrons from member levels when it
// publishes); one dt serves all three slots.
void recon_sensor_pass_rebuild_008073c0(ReconSensorPassState& state, float dt,
                                        ReconSensorPassHost& host);

// Packet cc9_forced_recon_level. 00805CF0 sets a (target, observing party) record's
// force byte (+10h = 1) and forced level (+8h); the pass then skips the sensor test
// for that record and publishes the forced level (det+10h, 00806883). The host's
// ReconSensorPassHost hooks are keyed by target alone, so the per-(side, target)
// force lives in this process-wide table, which the pass consults before them. It is
// empty unless SetForcedReconLevel (008AA8F0, kForcedReconLevelBound) writes it.
void set_forced_recon_level_00805cf0(std::size_t target, int side, int level);
bool forced_recon_level(int side, std::size_t target, ReconDetectionLevel& out);
// The record's +8h itself (the level argument, unclamped: scripts force 4), for
// readers of +8h such as GetProperty "reconlevel" (00779C18). Packet cc13-reconforce.
bool forced_recon_level_raw(int side, std::size_t target, int& out);
void clear_forced_recon_levels();
// Packet cc13-fix18 (docs/CC13_FIX18.md), ClearForcedReconLevel 008AAB90. 00803B50
// __fastcall(ECX = record), RET: old = +10h ? +8h : +4h (00803B52-00803B5E); +10h = 0
// (00803B66); when +4h differs from old, the record's listener [+2Ch] vt[0](+30h, old,
// +4h) (00803B6C-00803B7A). Here: the (side, target) entry leaves the forced table,
// and the next records rebuild clears the persistent record's force byte and queues
// that (old, +4h) transition ahead of the pass's own (SUBSTITUTION, labelled: the
// notify is delivered at the next pass, not inside the native). Answers whether the
// pair was forced. Nothing calls it unless kLuaClearForcedReconLevelBound is true.
bool clear_forced_recon_level_00803b50(std::size_t target, int side);

} // namespace bsp
