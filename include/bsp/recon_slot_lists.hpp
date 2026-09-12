#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

// The per-party recon slots: how a unit gets into a party's recon list, and how
// 008073C0 rebuilds the five published lists every three seconds.
//
// docs/RECON_SLOT_LISTS.md carries the evidence. Every name below is a
// hypothesis, not a recovered symbol.
//
// bsp/fixed_step_countdown.hpp already declares the countdown, the three-slot
// pointer table (kReconSlotTableAddress, kReconSlotCount), the slot size
// (kReconSlotSize), the dirty byte (kReconSlotDirtyOffset), the index
// (kReconSlotIndexOffset) and the three relation arrays
// (kReconSlotArrayOffsets / Length / Stride). None of those is redefined here.
// bsp/recon_values.hpp declares recon_category_names_00e0b590. This header adds
// the fourth array, the five triples, the membership rule and the sensor model.

namespace bsp {

// ---------------------------------------------------------------------------
// Slot layout beyond bsp/fixed_step_countdown.hpp
// ---------------------------------------------------------------------------

// Set by 00806818 when a unit enters the slot for the first time; cleared at the
// top of the rebuild (008073E6) and at its end (0080799C). Distinct from the
// +25h publish-dirty byte 00803BAB sets.
inline constexpr std::size_t kReconSlotContentsChangedOffset = 0x24;

// The prologue at 008073C1..008073E1: +30h = now - +2Ch, then +2Ch = now, with
// `now` the float at 00F876A4. +30h is the dt 00806840 hands to the sensor
// evaluation at 008068CD.
inline constexpr std::size_t kReconSlotLastRefreshTimeOffset = 0x2c;
inline constexpr std::size_t kReconSlotElapsedSecondsOffset = 0x30;
inline constexpr std::uint32_t kReconGlobalTimeAddress = 0x00f876a4;

// The five {count, head, tail} triples at +DD8h..+E13h. The first four are the
// four Lua relation keys 00806B10 publishes; the fifth is native-only.
inline constexpr std::size_t kReconTripleOwnOffset = 0x0dd8;     // "own"
inline constexpr std::size_t kReconTripleEnemyOffset = 0x0de4;   // "enemy"
inline constexpr std::size_t kReconTripleNeutralOffset = 0x0df0; // "neutral"
inline constexpr std::size_t kReconTripleUnknownOffset = 0x0dfc; // "unknown"
inline constexpr std::size_t kReconTripleAllOffset = 0x0e08;     // union, not published
inline constexpr std::array<std::size_t, 5> kReconTripleOffsets{{
    kReconTripleOwnOffset, kReconTripleEnemyOffset, kReconTripleNeutralOffset,
    kReconTripleUnknownOffset, kReconTripleAllOffset
}};

// The head of the "enemy" triple, the intrusive chain 009F5D30 and the flak
// fuse walk. 00807634 reads the same dword.
inline constexpr std::size_t kReconEnemyChainHeadOffset = 0x0de8;

// The fourth 97-entry array of 0Ch-byte lists, at +E14h..+129Fh: the carry-over
// lists. 00805240 destroys it with the same array helper and the same
// {0Ch, 61h, 00804E00} triplet as the other three (00805300..0080530C), so it is
// a constructed member, not padding. 0xE14 + 0x61 * 0x0C == 0x12A0 exactly.
inline constexpr std::size_t kReconSlotCarryOverArrayOffset = 0x0e14;

// ---------------------------------------------------------------------------
// The relation rule, 008065FF..0080672B (and again at 00806636..00806662)
// ---------------------------------------------------------------------------

// The three per-class arrays are one per relation, in this order: the array at
// +34h is `own`, the one at +4C0h is `enemy`, the one at +94Ch is `neutral`.
// 008067C1 / 00806783 / 00806742 select them.
enum class ReconRelation : int { own = 0, enemy = 1, neutral = 2 };

inline constexpr int kReconNeutralParty = 2; // the CMP against 2 at 00806602

// Which relation array a unit lands in, given the slot's own index at +28h and
// the unit's party at +54h. The slot index 2 arm is not the general rule: for
// that slot every non-party-2 unit is `neutral`, never `enemy`.
constexpr ReconRelation recon_relation_for_008065ff(int slot_index, int unit_party) noexcept
{
    if (slot_index == kReconNeutralParty) {
        // 00806721: SUB/NEG/SBB/AND 2 -> 0 when the party is 2, else 2.
        return unit_party == kReconNeutralParty ? ReconRelation::own : ReconRelation::neutral;
    }
    if (slot_index == unit_party) {
        return ReconRelation::own; // 0080660E/00806616
    }
    // 00806712: SETZ on (party == 2), then +1 -> 2 for party 2, 1 otherwise.
    return unit_party == kReconNeutralParty ? ReconRelation::neutral : ReconRelation::enemy;
}

// ---------------------------------------------------------------------------
// The per-unit, per-party detection record: unit + 1E8h + party * 34h
// ---------------------------------------------------------------------------

// 008066EE, 0080680E, 00806936 and 00804B45 all form the same address:
// unit + 1E8h + party * 34h. 00779B28 walks the array with three iterations.
inline constexpr std::size_t kReconUnitDetectionArrayOffset = 0x1e8;
inline constexpr std::size_t kReconUnitDetectionStride = 0x34;      // IMUL ..,34h
inline constexpr std::size_t kReconUnitDetectionCount = 3;          // 00779B4E CMP EDI,3

// Field offsets inside one 34h record, all from 00805AF0 / 00805BE0 / 008069A0.
inline constexpr std::size_t kReconDetectionLevelOffset = 0x04;     // 00805B73
inline constexpr std::size_t kReconDetectionForcedLevelOffset = 0x08;
inline constexpr std::size_t kReconDetectionValueOffset = 0x0c;     // 00805B10, float
inline constexpr std::size_t kReconDetectionForceByteOffset = 0x10; // 00805AF4
inline constexpr std::size_t kReconDetectionObserverOffset = 0x28;  // 00805B76

// The three image floats the level threshold reads.
inline constexpr float kReconDetectionValueMax = 1.0f;          // 00D7A24C
inline constexpr float kReconDetectionBlipThreshold = 0.25f;    // 00CE3868
inline constexpr float kReconDetectionIdentifyThreshold = 0.5f; // 00CE3800

// 0 = not detected, 1 = a blip (published under "unknown"), 2 = identified
// (published under its own relation). Native stores it as a plain int.
enum class ReconDetectionLevel : int { none = 0, blip = 1, identified = 2 };

// 00805B02..00805B4D. `accumulate` is the native second argument: the sensor
// pass and the own-unit call both pass 1, so the add arm is the live one; the
// assign arm has no reached call site in this packet.
float recon_detection_accumulate_00805af0(float current, float delta, bool accumulate) noexcept;

// 00805B3D..00805B65, on the already-clamped value.
ReconDetectionLevel recon_detection_level_00805b3d(float clamped_value) noexcept;

// The value a record's +0Ch carries into the triple filter: 0080695C picks the
// forced level when the force byte is set, the accumulated level otherwise.
constexpr ReconDetectionLevel recon_effective_level_0080695c(bool force,
                                                             ReconDetectionLevel level,
                                                             ReconDetectionLevel forced) noexcept
{
    return force ? forced : level;
}

// ---------------------------------------------------------------------------
// The unit gate, 008074D5..008074FA
// ---------------------------------------------------------------------------

// The four bytes the scan tests before it will consider a unit at all. The
// names are bsp/unit_instance_layout's: +5Ch is the live gate, +5Dh the
// simulate byte, +5Eh the dead byte, +60h a further gate byte.
struct ReconUnitGateBytes {
    bool live_5c{false};
    bool simulate_5d{false};
    bool dead_5e{false};
    bool gate_60{false};
};

// Live must be set and the other three clear. The order in the listing is
// 5Ch, 5Dh, 60h, 5Eh; it is a conjunction, so the order does not change the
// result, only which compare short-circuits.
constexpr bool recon_unit_gate_passes_008074d5(const ReconUnitGateBytes& gate) noexcept
{
    return gate.live_5c && !gate.simulate_5d && !gate.gate_60 && !gate.dead_5e;
}

// The class the gate then demands, through vtable slot 5Ch: IsKindOf(2).
inline constexpr int kReconScanRequiredClassId = 0x02; // 008074F2 PUSH 2

// ---------------------------------------------------------------------------
// Which classes are scanned, and the two aggregate classes
// ---------------------------------------------------------------------------

// The 22 ids 00805F60 push_backs into the singleton's first vector, in native
// push order. 00806480 hands that vector to the rebuild; the scan visits the
// world registry's per-class list for each.
inline constexpr std::array<int, 22> kReconScannedClassIds{{
    0x35, 0x07, 0x0a, 0x0b, 0x0c, 0x0d, 0x08, 0x09, 0x0e, 0x13,
    0x10, 0x12, 0x11, 0x15, 0x16, 0x17, 0x19, 0x1b, 0x45, 0x46,
    0x2b, 0x34
}};

// The singleton's second vector (00806387..008063DF): the two classes that are
// never scanned because the grouping passes create their entries. 008073C0
// retires them with two literal 00805430 calls instead of reading this vector.
inline constexpr std::array<int, 2> kReconAggregateClassIds{{0x18, 0x1a}};

// The seven plane leaf classes 00805490 folds into PlaneSquadronGen, in the
// native call order at 00807581..008075DC.
inline constexpr std::array<int, 7> kReconPlaneMemberClassIds{{
    0x10, 0x13, 0x12, 0x11, 0x16, 0x15, 0x17
}};
inline constexpr int kReconSquadronClassId = 0x18;    // PlaneSquadronGen
inline constexpr int kReconLandVehicleClassId = 0x19; // MLandVehicle
inline constexpr int kReconConvoyClassId = 0x1a;      // LandConvoy

// The back pointer each member reads to find its group: a plane's squadron at
// +9D4h (00805490), a land vehicle's convoy at +738h (00805680).
inline constexpr std::size_t kReconPlaneSquadronBackPointerOffset = 0x9d4;
inline constexpr std::size_t kReconLandVehicleConvoyBackPointerOffset = 0x738;

// A member is folded into its group only once it is at least a blip.
constexpr bool recon_member_groups_008054d7(ReconDetectionLevel member_level,
                                            bool has_group) noexcept
{
    return has_group && static_cast<int>(member_level) >= 1;
}

// ---------------------------------------------------------------------------
// The triple filter, 00807644..008076F3 and again 008077C2..00807871
// ---------------------------------------------------------------------------

struct ReconTripleFilterResult {
    bool copy_to_unknown{false};   // record level == 1
    bool remove_from_relation{false}; // record level < 2
};

// The enemy and neutral triples are drained by level: a blip is copied into the
// "unknown" triple and removed from its own, an undetected entry is removed
// without a copy, and only an identified entry stays. The own triple is never
// filtered.
constexpr ReconTripleFilterResult recon_triple_filter_00807647(ReconDetectionLevel level) noexcept
{
    const int value = static_cast<int>(level);
    return ReconTripleFilterResult{value == 1, value < 2};
}

// ---------------------------------------------------------------------------
// The sensor evaluation, 008048A0
// ---------------------------------------------------------------------------

// One row of the observer's sensor table, 1Ch bytes. The table lives at
// [[observer+538h]+B4h]+8, indexed observerCategory * 8 + targetCategory with a
// 0Ch stride per pair; each pair holds {const Entry* begin, int count, ...}.
struct ReconSensorEntry {
    float max_normalized_distance_sq{0.0f}; // +4h, compared at 00804AB7
    float gain_per_second{0.0f};            // +8h, multiplied by dt at 00804B35
    float cap{0.0f};                        // +0Ch, the clamp at 00804B69
    float max_bearing_error{0.0f};          // +14h, compared at 00804B29
    int mask_bit{0};                        // +10h, 1 << it tested at 00804ACF
    bool bearing_limited{false};            // +18h, gates the bearing test
};

inline constexpr std::size_t kReconSensorBlockOffset = 0x538;      // observer+538h
inline constexpr std::size_t kReconSensorTablePointerOffset = 0xb4; // block+B4h
inline constexpr std::size_t kReconSensorSignatureOffset = 0xb8;    // block+B8h
inline constexpr std::size_t kReconSensorTableArrayOffset = 0x08;
inline constexpr std::size_t kReconSensorTableStride = 0x0c;
inline constexpr std::size_t kReconSensorCategoriesPerObserver = 8; // EAX*8
inline constexpr std::size_t kReconSensorEntryStride = 0x1c;

// The default the image uses when the target has no sensor block of its own and
// when the gameplay-modifier query is skipped: 00D7A24C, the same 1.0f the
// detection value clamps to.
inline constexpr float kReconSensorDefaultFactor = 1.0f;

// The mask the rebuild's sensor pass passes: 008068E0 PUSH 0FFh.
inline constexpr int kReconSensorMaskAll = 0xff;

// The two class tests inside 008048A0.
inline constexpr int kReconSensorSignatureClassId = 0x05; // 008048D1, the unit base
inline constexpr int kReconSubmarineClassId = 0x08;       // 0080494B, MSubmarine

// The squared-range normaliser at 00804947..00804977, in native evaluation
// order: env is squared, multiplied by the signature, then by the square of the
// tuning value at [game+21C4h]+74h. A submerged submarine multiplies it again by
// the square of [game+21C4h]+78h (008049A3..008049B5).
float recon_sensor_range_scale_00804947(float environment_factor,
                                        float target_signature,
                                        float tuning_74) noexcept;
float recon_sensor_submerged_scale_008049a3(float range_scale, float tuning_78) noexcept;

// One entry's contribution: the gain, clamped so the accumulated value does not
// pass the entry's cap (00804B45..00804B7F). Returns 0 when the entry does not
// apply, with `applies` false.
struct ReconSensorEntryResult {
    float gain{0.0f};
    bool applies{false};
};

ReconSensorEntryResult recon_sensor_entry_gain_00804ab0(const ReconSensorEntry& entry,
                                                        float normalized_distance_sq,
                                                        int mask,
                                                        float bearing_error,
                                                        float current_value,
                                                        float dt) noexcept;

// The whole 008048A0 arithmetic once the table row is known: the best gain over
// the row's entries, and whether any of them applied. `bearing_error` is
// supplied per entry because the native code only computes it for entries whose
// +18h byte is set.
struct ReconSensorEvaluation {
    float best_gain{0.0f};
    bool detected{false};
};

ReconSensorEvaluation recon_evaluate_sensor_row_008048a0(const ReconSensorEntry* entries,
                                                         std::size_t count,
                                                         const float* bearing_errors,
                                                         float normalized_distance_sq,
                                                         int mask,
                                                         float current_value,
                                                         float dt) noexcept;

// The horizontal distance the range test uses: the x and z of the world frames
// only (00804A1E..00804A57 read +FCh and +104h), never the height.
float recon_horizontal_distance_sq_00804a1e(float observer_x, float observer_z,
                                            float target_x, float target_z) noexcept;

// ---------------------------------------------------------------------------
// What the publish excludes
// ---------------------------------------------------------------------------

// 00805DE6..00805E1C: ten class ids never reach a Lua category table, so only
// the squadron and convoy aggregates stand for their members in the script's
// view. The list is the plane base 0Fh, its seven leaves, the two recon-plane
// leaves, and MLandVehicle.
inline constexpr std::array<int, 10> kReconPublishExcludedClassIds{{
    0x0f, 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x17, 0x13, 0x19
}};

bool recon_publish_excludes_class_00805de6(int class_id) noexcept;

// The category index comes from the unit's fourth vptr at +170h, slot 0; 13h
// means "no category" and drops the unit. 19 category tables, named by
// bsp/recon_values.hpp's recon_category_names_00e0b590.
inline constexpr int kReconPublishNoCategory = 0x13; // 00805E33
inline constexpr std::size_t kReconPublishCategoryCount = 19;
inline constexpr std::size_t kReconUnitCategoryVptrOffset = 0x170;
inline constexpr std::size_t kReconUnitLuaIdOffset = 0x174; // u16, the table key

// ---------------------------------------------------------------------------
// The rebuild sequence
// ---------------------------------------------------------------------------

// One method per native call site of 008073C0. The host owns the lists; this
// header models the order and the decisions, not the storage.
struct ReconSlotListsHost {
    virtual ~ReconSlotListsHost() = default;

    // 008073C1 / 008073D9: the float at 00F876A4.
    virtual float global_time_00f876a4() = 0;

    // 008042B0, the pop-front-and-free loop, called once per triple.
    virtual void clear_triple_008042b0(std::size_t triple_offset) = 0;

    // 00804150, the splice-all at 00807430..00807460: the carry-over list for
    // one class absorbs that class's three relation lists.
    virtual void move_relation_list_into_carry_over_00804150(int class_index,
                                                             ReconRelation relation) = 0;

    // 00805BE0 at 00807490: reset one unit's detection record for this slot.
    // The units come from the world list at [[game+19CCh]+13Ch].
    virtual void reset_detection_for_world_list_00805be0() = 0;

    // 00806480: the lazily built singleton's first vector.
    virtual const std::vector<int>& scanned_class_ids_00806480() = 0;

    // The world registry's per-class unit list, [game+19CCh] + 18h + id * 0Ch.
    // Returns the units that pass no filter yet; the gate is applied by the
    // caller through unit_gate_bytes / unit_is_kind_of.
    virtual std::vector<void*> world_units_of_class_008074ca(int class_id) = 0;

    virtual ReconUnitGateBytes unit_gate_bytes_008074d5(void* unit) = 0;
    virtual bool unit_is_kind_of_008074f6(void* unit, int query_class_id) = 0;
    virtual int unit_party_00806605(void* unit) = 0;

    // 008065B0: move the unit's carry-over record into `relation`'s list for
    // `class_id`, or create one. Returns true when it was created, which is what
    // sets the slot's +24h byte at 00806818.
    virtual bool add_or_refresh_unit_008065b0(int class_id, void* unit, ReconRelation relation) = 0;

    // 008067FA / 008066E1: the own-relation arm forces the unit's detection
    // record for this slot to the maximum with 00805AF0(1.0f, 1, nullptr).
    virtual void force_own_unit_detected_00805af0(void* unit) = 0;

    // 00805430: retire whatever is still in the carry-over list for one class.
    virtual void retire_carry_over_00805430(int class_id) = 0;

    // 00804E10, the copy-append: a triple takes a copy of every node of a list.
    virtual void append_class_list_to_triple_00804e10(std::size_t triple_offset,
                                                      ReconRelation relation,
                                                      int class_index) = 0;
    virtual void append_triple_to_triple_00804e10(std::size_t destination_offset,
                                                  std::size_t source_offset) = 0;

    // 00806840: the sensor pass. Runs one relation's class list against the own
    // triple and writes each record's level.
    virtual void run_sensor_pass_00806840(ReconRelation relation, int class_index) = 0;

    // 00805490 and 00805680: fold members into their group.
    virtual void group_members_00805490(ReconRelation relation, int member_class_id,
                                        int group_class_id) = 0;
    virtual void group_members_00805680(ReconRelation relation, int member_class_id,
                                        int group_class_id) = 0;

    // 008069A0 and 00806A60: publish the group's level onto the group unit's own
    // detection record. 008069A0 recomputes the maximum over the member list;
    // 00806A60 takes the group record's stored value.
    virtual void publish_group_level_from_members_008069a0(ReconRelation relation,
                                                           int group_class_id) = 0;
    virtual void publish_group_level_stored_00806a60(ReconRelation relation,
                                                     int group_class_id) = 0;

    // 00807634 and 008077B4: drain one relation triple by level into "unknown".
    virtual void drain_triple_to_unknown_00807634(std::size_t relation_triple_offset) = 0;

    // 00807995: clear the cached-view byte at game+193Ch when this slot is the
    // active local player's. The index test is 0080797D..00807993.
    virtual bool slot_is_active_local_player_00807986(int slot_index) = 0;
    virtual void invalidate_local_view_00807995() = 0;
};

// One slot's state as the rebuild reads and writes it.
struct ReconSlotState {
    int index{0};                // +28h
    float last_refresh_time{0.0f}; // +2Ch
    float elapsed_seconds{0.0f};   // +30h
    bool contents_changed{false};  // +24h
};

// 008073C0 in full, as the order of host calls. Returns the number of units the
// scan handed to 008065B0, which the native code does not keep; it exists so a
// caller can assert the pass ran.
std::size_t rebuild_recon_slot_lists_008073c0(ReconSlotListsHost& host,
                                              ReconSlotState& slot) noexcept;

} // namespace bsp
