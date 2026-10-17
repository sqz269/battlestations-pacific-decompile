#pragma once

#include <cstddef>

#include <bsp/recon_slot_lists.hpp>
#include <bsp/sensor_tables.hpp>

// Rule (c) of the recon contact list: the per-observer detection value, and the
// predicate that decides whether an enemy unit is published as a contact.
//
// docs/GUNNERY_RECON_DETECTION.md carries the evidence. Every name below is a
// hypothesis, not a recovered symbol.
//
// This header adds only the parts of the rule that had no pure form yet: the
// end-to-end 008048A0 observation (range scale, horizontal distance, row
// selection, bearing test) and the 00806840 fold over a slot's own triple that
// turns per-observer gains into the published level. The pieces that already
// have one are reused, not redefined:
//
//   bsp/recon_slot_lists.hpp  ReconSensorEntry (the 1Ch row entry),
//                             ReconDetectionLevel and its three thresholds,
//                             ReconRelation, recon_triple_filter_00807647,
//                             recon_detection_accumulate_00805af0,
//                             recon_detection_level_00805b3d,
//                             recon_effective_level_0080695c,
//                             recon_sensor_range_scale_00804947,
//                             recon_sensor_submerged_scale_008049a3,
//                             recon_sensor_entry_gain_00804ab0,
//                             recon_horizontal_distance_sq_00804a1e.
//   bsp/sensor_tables.hpp     SensorCategory, sensor_list_index_008085aa.
//   bsp/unit_rudder.hpp       wrapped_angle_subtract_00438b10 (used by the .cpp).
//
// No host, no globals: every native input is a documented field below.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants this rule adds
// ---------------------------------------------------------------------------

// [[unit+538h]+B8h], written by 009623D9 as the square of the vehicle class's
// `ReconModifier` Lua field (key at 00D1AAF4, default 1.0f from the FLD1 at
// 009623AE). 008048A0 reads it only for a target that is IsKindOf(05h); every
// other target contributes 00D7A24C, 1.0f.
inline constexpr float kGunneryReconModifierSqDefault = 1.0f;

// [[00E188A8]+1FE4h]. 0080686A/00806871 skips the whole sensor test when the
// word is 2, and the test is re-read inside 00806840's target loop.
// The same word gates message origination at 0071F373 (docs/DIRECTOR_UPDATE_ARMS.md)
// and 0077E3FF (docs/ENTITY_LIFECYCLE_TAILS.md), where 1 is the originating
// machine: 2 is the non-originating one, so on that machine detection is not
// computed locally at all.
inline constexpr int kGunneryReconNonOriginatingRole = 2;

// The gameplay-modifier category 008048A0 asks for on the observer, 0080491C.
inline constexpr int kGunneryReconEnvironmentModifierCategory = 0x0c;

// 00865206 reads the drained enemy chain straight off the slot. Same value as
// kReconEnemyChainHeadOffset; named here for the gunnery reader's sake.
inline constexpr std::size_t kGunneryReconContactChainOffset = kReconEnemyChainHeadOffset;

// No observer index. The fold reports this when nothing applied.
inline constexpr std::size_t kGunneryReconNoObserver = static_cast<std::size_t>(-1);

// ---------------------------------------------------------------------------
// One row of an observer's sensor table
// ---------------------------------------------------------------------------

// [[observer+538h]+B4h] + 8 + (observerCategory * 8 + targetCategory) * 0Ch,
// read as {begin, count} at 00804A86/00804A89. Entries are contiguous with a
// 1Ch stride (00804BA6).
// Note the units the writer establishes: entry+8h is the script's `Gain`
// HALVED (00808626, the double at 00D7A280), and entry+14h is a half-angle,
// so the admitted arc is twice `Angle` wide.
struct GunneryReconSensorRow {
    const ReconSensorEntry* entries{nullptr};
    std::size_t count{0};
};

// ---------------------------------------------------------------------------
// The observer
// ---------------------------------------------------------------------------

// One entry of the slot's own triple (+DD8h). 008068C2 admits it as an observer
// only when the unit is IsKindOf(05h); 008048A0 then requires a sensor table.
struct GunneryReconObserver {
    // The 56 addressable rows of [[observer+538h]+B4h], or null when the unit
    // has no table at all (the early `return false` at 008048B8).
    const GunneryReconSensorRow* rows{nullptr};
    std::size_t row_count{0};

    // Passing 008068C2's IsKindOf(05h) test.
    bool is_observer_class{true};

    // unit->[+1E4h]->vtable[1](), the row index. 00804A69..00804A76.
    SensorCategory category{SensorCategory::unclassified};

    // The world frame at +FCh and +104h. Height (+100h) is never read.
    float world_x{0.0f};
    float world_z{0.0f};

    // observer->vtable[50h](), radians. Only read by a bearing-limited entry.
    float heading{0.0f};

    // unit+54h, the party. 00804B2F indexes the target's detection record with
    // THIS value, while 00806840 writes the record indexed by the slot index.
    int party{0};

    // 008E6430(0Ch, observer) when [00E0C978] and [[00F88C30]+118h] are both
    // set, else 00D7A24C (1.0f). Resolved by the host; 008048A0 squares it.
    float environment_factor{1.0f};
};

// ---------------------------------------------------------------------------
// The target
// ---------------------------------------------------------------------------

struct GunneryReconTarget {
    // unit->[+1E4h]->vtable[1](), the column index. 00804A67.
    SensorCategory category{SensorCategory::unclassified};

    float world_x{0.0f};
    float world_z{0.0f};

    // [[target+538h]+B8h]: the SQUARE of the class's ReconModifier. Supply
    // kGunneryReconModifierSqDefault when `is_unit_base` is false; 008048E5
    // substitutes 1.0f in that case regardless.
    float recon_modifier_sq{kGunneryReconModifierSqDefault};

    // IsKindOf(05h) at 008048D1: gates reading recon_modifier_sq.
    bool is_unit_base{true};

    // IsKindOf(08h) at 0080494B (MSubmarine) and 00922DC0 at 00804987.
    // Both must hold (submarine AND not a surface target) for the sonar penalty.
    bool is_submarine{false};
    bool is_surface_target{true};

    // det+10h, read at 00806883 and 00805AF4. When set, the sensor pass is
    // skipped for this target and 0080695C publishes `forced_level` instead.
    bool detection_forced{false};
    ReconDetectionLevel forced_level{ReconDetectionLevel::none}; // det+8h
};

// ---------------------------------------------------------------------------
// The session-wide inputs
// ---------------------------------------------------------------------------

struct GunneryReconEnvironment {
    // [game+21C4h]+74h, the Lua `SimplifiedReconMultiplier`. 00444D20 writes
    // 1.0f as the default; the Lua binding rows at 00E0C248..00E0C264 pair the
    // setter 008B24A0 (storing at 008B243C) with the name at 00D0F8E4, so a
    // mission script can change it. 008048A0 squares it.
    float simplified_recon_multiplier{1.0f};

    // [game+21C4h]+78h, `SimplifiedSonarMultiplier`. Same story: 00444D20
    // writes 1.0f, 008B2770/008B270C is the Lua setter. 008048A0 squares it
    // too, but only for a submerged submarine.
    float simplified_sonar_multiplier{1.0f};

    // The raw-type mask. The rebuild passes 0FFh (008068E0), so all three of
    // RRT_VISION(0) / RRT_RADAR(1) / RRT_SONAR(2) always pass.
    int sensor_mask{kReconSensorMaskAll};

    // [00E188A8]+1FE4h. The pass runs only when this is not
    // kGunneryReconNonOriginatingRole.
    int network_role{1};

    // slot+28h, kReconSlotIndexOffset. 00806840 computes the detection record
    // at target+1E8h+slot_index*34h and hands it to 00805AF0 / 00805BE0.
    // The native dt is [slot+30h] (008068CD); it is a parameter here.
    int slot_index{0};
};

// ---------------------------------------------------------------------------
// One observer against one target: the whole of 008048A0
// ---------------------------------------------------------------------------

struct GunneryReconObservation {
    // *outGain at 00804BC3: the largest clamped gain over the row's entries.
    // Can be zero even when `applied` is true (every applying entry was already
    // at or past its cap), and is never negative for a fresh pass.
    float gain{0.0f};

    // The return value at 00804BC7: whether ANY entry applied. This, not the
    // gain, is what 00806840 uses to decide reset vs. accumulate.
    bool applied{false};

    // distSq / rangeScale at 00804A5F, the value compared against each entry's
    // +4h. Reported for evidence; not a native output.
    float normalized_distance_sq{0.0f};

    // env^2 * reconModifier^2 * m74^2, times m78^2 for a submerged submarine.
    float range_scale{0.0f};

    // The row 00804A82 selected, or kSensorListCount when the observer has no
    // table or the pair is out of range.
    std::size_t row_index{0};
};

// 00804947..00804977 and the submerged branch 008049A3..008049B5, in native
// order: each intermediate is rounded back to a float before the next multiply.
float gunnery_recon_range_scale_008048a0(const GunneryReconObserver& observer,
                                         const GunneryReconTarget& target,
                                         const GunneryReconEnvironment& env) noexcept;

// 00804A7B..00804A82: observerCategory * 8 + targetCategory, the same index the
// loader computes at 008085AA.
std::size_t gunnery_recon_row_index_00804a7b(SensorCategory observer,
                                             SensorCategory target) noexcept;

// The whole 008048A0. `current_value` is the target's accumulated detection
// value as 00804B45 reads it: from the record indexed by the OBSERVER's party,
// which is not necessarily the record 00806840 writes.
GunneryReconObservation gunnery_recon_observe_008048a0(const GunneryReconObserver& observer,
                                                       const GunneryReconTarget& target,
                                                       const GunneryReconEnvironment& env,
                                                       float current_value,
                                                       float dt) noexcept;

// ---------------------------------------------------------------------------
// One target against a whole own triple: 00806840, and what it publishes
// ---------------------------------------------------------------------------

struct GunneryReconContact {
    // det+0Ch after 00805AF0 clamped it to [0, 1] (00805B20..00805B48), or 0
    // after 00805BE0.
    float detection_value{0.0f};

    // det+4h: the threshold of 00805B3D applied to detection_value.
    ReconDetectionLevel level{ReconDetectionLevel::none};

    // The contact-list node's payload +0Ch, [[node+8h]+0Ch], written at
    // 00806981 -- NOT the detection record at unit+1E8h+slot*34h, which keeps
    // the raw value and level. Forced level when det+10h is set (00806979),
    // accumulated level otherwise (0080697E). This is what the drain reads at
    // 00807647 and 008077C5.
    ReconDetectionLevel published_level{ReconDetectionLevel::none};

    // 00807644..008076F3 (enemy) and 008077C2..00807871 (neutral), reused from
    // recon_triple_filter_00807647.
    ReconTripleFilterResult drain{};

    // The answer the packet exists for. True when the record survives the drain
    // in an enemy or neutral triple, which is exactly what 00865206 walks.
    bool admitted_as_contact{false};

    // Index into the observers array, or kGunneryReconNoObserver.
    std::size_t best_observer{kGunneryReconNoObserver};

    // 008068F4 OR BL,AL: whether any observer applied any entry.
    bool any_observer_applied{false};

    // True when 00806871 or 00806883 short-circuited the sensor test. The
    // record's level is then whatever the detection record already held.
    bool sensor_pass_skipped{false};

    // True when an applying observer's party differs from env.slot_index: the
    // cap is then clamped against a different record than the one written.
    // Inside the native rebuild the two always coincide.
    bool observer_party_mismatch{false};
};

// 00806840 over one target: pick the largest gain across the own triple, apply
// it through 00805AF0 (or reset through 00805BE0), publish the effective level
// at 0080695C, then drain. `relation` selects whether the drain runs at all:
// ReconRelation::own is never filtered.
//
// Natively the drain is not per target. 008073C0 runs all 97 class buckets
// through 00806840 first (00807556 enemy, 00807574 neutral), flattens them
// into the relation list (00807615), and drains that list once (00807634).
// Folding the drain in here is this reconstruction's composition for a
// caller's convenience, not the native control flow.
//
// `prior_value` is the target's detection value for this slot on entry. The
// OFF caller passes 0.0f. CORRECTION (packet cc9_g51_recon_records): the reset
// at 00807480..0080749A walks the class-18h squadron list only, so a unit's
// value carries across passes; recon_run_sensor_pass_00806840 below is the
// faithful form.
GunneryReconContact gunnery_recon_detect_00806840(const GunneryReconObserver* observers,
                                                  std::size_t observer_count,
                                                  const GunneryReconTarget& target,
                                                  const GunneryReconEnvironment& env,
                                                  ReconRelation relation,
                                                  float prior_value,
                                                  float dt) noexcept;

// The predicate on its own, for a caller that already has a level: 0080764A
// (`level >= 1 && level < 2` copies to "unknown") and 008076A5 (`level < 2`
// unlinks). Only an identified record stays in the enemy or neutral triple,
// and only those reach 00865206.
constexpr bool gunnery_recon_admits_contact_008076a5(ReconDetectionLevel level) noexcept
{
    return !recon_triple_filter_00807647(level).remove_from_relation;
}

// ---------------------------------------------------------------------------
// Packet cc9_g51_recon_records: the detection record itself, and 00806840 over it
// ---------------------------------------------------------------------------
//
// One record per (unit, observing party) at unit + 1E8h + party * 34h. Only the
// fields 00805AF0, 00805BE0, 00805CF0 and 00806840 touch are kept. +2Ch/+30h, the
// change delegate and its party argument, are the host's level_changed hook.
//
// The value is NOT zeroed before each pass. 008073C0's reset loop
// (00807462..0080749A) walks [[[00E188A8]+19CCh]+13Ch], the registry's class-18h
// list: 008074CA indexes the per-class heads as registry + id*0Ch + 1Ch, so +13Ch is
// id 18h, and only the plane squadron's world-list virtual 007F10B0 joins it (ADD
// ECX,138h at 007F10C8; the unit's 006FE620 joins ids 2, 4, 5, 6, 7). A unit's
// record therefore carries its value from pass to pass (docs/GAMEPLAY_LOOSE_ENDS_1.md
// section 2, re-verified 2026-10-17).
struct ReconDetectionRecord {
    ReconDetectionLevel level{ReconDetectionLevel::none};         // +4h
    ReconDetectionLevel forced_level{ReconDetectionLevel::none};  // +8h, 00805CF0
    float value{0.0f};                                            // +0Ch
    bool forced{false};                                           // +10h, 00805CF0
    // +28h: the winning observer (00694A60 registers the edge, 006952A0 drops it),
    // as a unit index. Its only reader in the image is the HUD recon warning
    // 009741E0 (entity+210h+side*34h), so it is presentation state here.
    std::size_t observer{kGunneryReconNoObserver};
};

// +8h when +10h is set, else +4h: the level every reader and the delegate use.
constexpr ReconDetectionLevel recon_record_effective_level(const ReconDetectionRecord& record) noexcept
{
    return record.forced ? record.forced_level : record.level;
}

// What a record call reports to the delegate: [+2Ch]->vtable[0](+30h, old, new),
// fired only when the effective level moved.
struct ReconRecordLevelChange {
    bool changed{false};
    ReconDetectionLevel old_level{ReconDetectionLevel::none};
    ReconDetectionLevel new_level{ReconDetectionLevel::none};
};

// 00805AF0 __thiscall(record, float value, bool accumulate, Unit* observer), RET 0Ch.
// Adds (or assigns) the value to +0Ch, clamps to [0, 1.0f], writes +4h from the 0.25f
// and 0.5f thresholds, swaps +28h, and reports the effective-level change.
ReconRecordLevelChange recon_record_accumulate_00805af0(ReconDetectionRecord& record,
                                                        float value,
                                                        bool accumulate,
                                                        std::size_t observer) noexcept;

// 00805BE0 __thiscall(record), RET. Zeroes +0Ch and +4h, drops +28h, and reports the
// effective-level change (none while +10h holds the level at +8h).
ReconRecordLevelChange recon_record_reset_00805be0(ReconDetectionRecord& record) noexcept;

// What 00806840 needs from its caller, one method per native read or write.
struct ReconRecordPassHost {
    virtual ~ReconRecordPassHost() = default;
    // [00E188A8]+1FE4h, read once per target at 0080686A.
    virtual int network_role() = 0;
    // unit + 1E8h + slot*34h.
    virtual ReconDetectionRecord& record(std::size_t unit, int slot) = 0;
    // The delegate [+2Ch]->vtable[0](+30h = slot, old, new), 0077B0C0 in the image.
    virtual void level_changed(std::size_t unit, int slot,
                               ReconDetectionLevel old_level, ReconDetectionLevel new_level) = 0;
    // [[node+8h]+0Ch] = effective level, 00806981.
    virtual void publish_level(std::size_t unit, ReconDetectionLevel level) = 0;
};

// One own-list entry as 00806840's inner loop sees it. `rows == nullptr` is the
// class with no sensor table, for which 008048A0 returns false at 008048C1 WITHOUT
// writing *outGain.
struct ReconPassObserver {
    GunneryReconObserver observer{};
    std::size_t unit{kGunneryReconNoObserver};
};

// One target-list record: the unit and what 008048A0 reads of it.
struct ReconPassTarget {
    std::size_t unit{kGunneryReconNoObserver};
    GunneryReconTarget subject{};
};

// The stack slot 008048A0 writes its gain into ([ESP+14h] at 008068D3). An
// observer with no sensor table leaves it unwritten, and 008068EC compares it
// anyway, so it carries the last written gain from an earlier observer, an
// earlier target or an earlier 00806840 call of the same rebuild (the caller's
// loop at 00807552 re-enters with the same ESP and calls nothing in between).
// SUBSTITUTION, labelled: its value before the first write of a rebuild is
// unknown stack contents; the host starts each rebuild at 0.0f.
struct ReconPassScratch {
    float out_gain{0.0f};
    unsigned long long stale_gain_reads{0};   // census: a no-table observer was compared
};

// 00806840 __thiscall(slot, List* targets, List* ownTriple), RET 8, whole. For each
// target: unless [00E188A8]+1FE4h == 2 (00806871) or the record's +10h is set
// (00806883), run every IsKindOf(5) own entry through 008048A0(0FFh, slot+30h) with
// the record's CURRENT value as the cap base, keep the strictly larger gain and its
// observer, then 00805AF0(gain, 1, observer) when any entry applied, else 00805BE0.
// Always ends by publishing the effective level into the payload's +0Ch (0080695C).
void recon_run_sensor_pass_00806840(const ReconPassTarget* targets, std::size_t target_count,
                                    const ReconPassObserver* own, std::size_t own_count,
                                    const GunneryReconEnvironment& env, int slot, float dt,
                                    ReconRecordPassHost& host, ReconPassScratch& scratch);

} // namespace bsp
