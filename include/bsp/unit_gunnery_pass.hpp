// The unit-side gunnery pass: the tick sub-node at unit+6DCh that turns the
// weapon director's stance and targets, plus the side's recon contacts, into a
// fire target for every gun the unit owns.
//
// Evidence: docs/UNIT_GUNNERY_PASS.md. Every routine here is a projection of one
// native body and the coverage of each is in that document's routine table.
// Names are hypotheses, not recovered symbols, except the weapon-group names
// (AA_Flak, Artillery, Torpedo, DC) and the skill-level names, which are string
// literals in the image.
//
// Reused rather than redeclared: gun_bot_ticks.hpp for the gun-bot base layout
// this class shares and for the gun's six bot slots; weapon_director.hpp for the
// director fields; unit_weapons.hpp and bot_fire_target.hpp for the gun side.
#ifndef BSP_UNIT_GUNNERY_PASS_HPP
#define BSP_UNIT_GUNNERY_PASS_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kUnitGunneryPassTickAddress = 0x00864FE0u;
inline constexpr std::uint32_t kUnitGunneryPassConstructAddress = 0x00864580u;
inline constexpr std::uint32_t kUnitGunneryPassDestructAddress = 0x00864380u;
inline constexpr std::uint32_t kUnitGunneryPassAttachAddress = 0x00864BD0u;
inline constexpr std::uint32_t kUnitGunneryInstallPoliciesAddress = 0x008636A0u;
inline constexpr std::uint32_t kUnitGunneryCreateDirectorBridgeAddress = 0x00863A80u;
inline constexpr std::uint32_t kUnitGunneryDirectorBridgeApplyAddress = 0x008624C0u;
inline constexpr std::uint32_t kUnitGunneryScoreCandidateAddress = 0x00863990u;
inline constexpr std::uint32_t kUnitGunneryCategoryAcceptsAddress = 0x008633D0u;
inline constexpr std::uint32_t kUnitGunneryTargetClassAllowedAddress = 0x00862820u;
inline constexpr std::uint32_t kUnitGunneryVisibilityTestAddress = 0x00864D90u;
inline constexpr std::uint32_t kUnitGunneryVisibilityAgeAddress = 0x00862C30u;
inline constexpr std::uint32_t kUnitGunneryAddGunToRecordAddress = 0x00864CA0u;
inline constexpr std::uint32_t kUnitGunneryRankTableBuildAddress = 0x00727BD0u;
inline constexpr std::uint32_t kUnitAiSuppressesGunneryAddress = 0x00862440u;
inline constexpr std::uint32_t kGunBotSlotAcceptsTargetAddress = 0x00729BC0u;
inline constexpr std::uint32_t kGunMinimumAirRangeAddress = 0x00729B90u;
inline constexpr std::uint32_t kGunIsTorpedoClassLauncherAddress = 0x005459E0u;
inline constexpr std::uint32_t kWeaponDirectorNewestCommandTargetAddress = 0x0071EBF0u;
inline constexpr std::uint32_t kUnitGunneryDirectorFireTargetAddress = 0x00863640u;
inline constexpr std::uint32_t kUnitGunnerySubmarineCategoryGateAddress = 0x008627A0u;
inline constexpr std::uint32_t kEntityAboveDepthChargeDepthAddress = 0x00852820u;
inline constexpr std::uint32_t kPartySlotIsAiHeldAddress = 0x00927F10u;

// The primary vtable, and the observer base at this+1Ch. Section 1.
inline constexpr std::uint32_t kUnitGunneryPassVtable = 0x00D0D360u;
inline constexpr std::uint32_t kUnitGunneryPassObserverVtable = 0x00D0D348u;

// The two policy objects installed by 008636A0. Section 6.
inline constexpr std::uint32_t kUnitGunneryFireTargetPolicyNullVtable = 0x00D0D314u;
inline constexpr std::uint32_t kUnitGunneryFireTargetPolicyDirectorVtable = 0x00D0D324u;
inline constexpr std::uint32_t kUnitGunneryCategoryGateAllVtable = 0x00D0D31Cu;
inline constexpr std::uint32_t kUnitGunneryCategoryGateClass8Vtable = 0x00D0D32Cu;

// The authored preference lists and the rank table 00727BD0 derives from them.
inline constexpr std::uint32_t kUnitGunneryPreferenceListsAddress = 0x00E092C8u;
inline constexpr std::uint32_t kUnitGunneryTargetRankTableAddress = 0x00E19BF8u;

// The four category group lists 008624C0's setters walk. Section 3.
inline constexpr std::uint32_t kUnitGunneryArtilleryGroupAddress = 0x00E0A4F8u;
inline constexpr std::uint32_t kUnitGunneryAaGroupAddress = 0x00E0A510u;
inline constexpr std::uint32_t kUnitGunneryTorpedoGroupAddress = 0x00E0A520u;
inline constexpr std::uint32_t kUnitGunneryDepthChargeGroupAddress = 0x00E0A528u;

// ---------------------------------------------------------------------------
// Shape constants
// ---------------------------------------------------------------------------

// The twelve weapon categories the pass loops over: 00865888 adds one and
// 0086588B compares against 0Ch.
inline constexpr int kUnitGunneryCategoryCount = 12;

// 61h, the size of every per-category row. docs/ENTITY_CLASS_IDS.md proves the
// class-id space is 0..60h, so a row is one entry per class id.
inline constexpr int kUnitGunneryClassIdCount = 0x61;

// The stack candidate array: 0086513C passes size 8, count 50h to the vector
// constructor iterator. The order array carries one extra slot.
inline constexpr int kUnitGunneryCandidateCapacity = 0x50;

// 008624D3 reloads the bridge's counter with 0Ah, so the full re-enable in
// step 3 runs at most once every ten calls.
inline constexpr int kUnitGunneryBridgeForcePeriod = 0x0A;

// The distance penalty 00863A6B adds to a plane that has no follow target,
// read from the double at 00D7A220.
inline constexpr float kUnitGunneryLoiteringPlanePenalty = 100.0f;

// The category the torpedo group owns, singled out three times in the tick
// (008651B4, 008651F5, 00865809).
inline constexpr int kUnitGunneryTorpedoCategory = 7;

// Entity class ids the pass tests through vtable[5Ch]. From
// docs/ENTITY_CLASS_IDS.md: 06h is the ship base, 0Fh the plane base.
inline constexpr int kUnitGunneryKindShipBase = 0x06;
inline constexpr int kUnitGunneryKindPlaneBase = 0x0F;

// ---------------------------------------------------------------------------
// Instance offsets, from 00864580 and every read site in 00864FE0. Section 2.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kUnitGunneryOffVtable = 0x00;
inline constexpr std::size_t kUnitGunneryOffObserverVtable = 0x1C;
inline constexpr std::size_t kUnitGunneryOffUnit = 0x50;
inline constexpr std::size_t kUnitGunneryOffEnabled = 0x58;
inline constexpr std::size_t kUnitGunneryOffSweepSuppressed = 0x59;
inline constexpr std::size_t kUnitGunneryOffFireTargetPolicy = 0x5C;
inline constexpr std::size_t kUnitGunneryOffCategoryGate = 0x60;
inline constexpr std::size_t kUnitGunneryOffDirectorBridge = 0x64;
inline constexpr std::size_t kUnitGunneryOffVisibilityCache = 0x68;
inline constexpr std::size_t kUnitGunneryOffThrottle = 0x6C;
inline constexpr std::size_t kUnitGunneryOffCategoryEnabled = 0x70;   // 12 bytes
inline constexpr std::size_t kUnitGunneryOffTorpedoEnabled = 0x7C;
inline constexpr std::size_t kUnitGunneryOffTorpedoMayTakeFireTarget = 0x7D;
inline constexpr std::size_t kUnitGunneryOffCategoryMask = 0x80;      // 12 dwords
inline constexpr std::size_t kUnitGunneryOffTargetRecordList = 0xB0;
inline constexpr std::size_t kUnitGunneryOffSubEntityScratch = 0xBC;
inline constexpr std::size_t kUnitGunneryOffClassAllowTable = 0xCC;   // 12 * 61h bytes
inline constexpr std::size_t kUnitGunneryInstanceSize = 0x558;

// The unit fields the pass reads. 0086516D, 00865777, 00863A34, 00810DD0.
inline constexpr std::size_t kUnitOffGunneryPass = 0x6DC;
inline constexpr std::size_t kUnitOffTickElement = 0x310;
inline constexpr std::size_t kUnitOffCategoryRecords = 0x394;   // 12 * 0Ch
inline constexpr std::size_t kUnitGunneryCategoryRecordStride = 0x0C;
inline constexpr std::size_t kUnitGunneryCategoryRecordOffGate = 0x00;
inline constexpr std::size_t kUnitGunneryCategoryRecordOffGunList = 0x04;
inline constexpr std::size_t kUnitOffCategoryRanges = 0x430;     // 12 floats
inline constexpr std::size_t kUnitOffScriptedFireInhibit = 0x634;
inline constexpr std::size_t kUnitOffSkillIndex = 0x390;

// The linked-list node shape shared by the recon contact list and the per
// category gun lists: next at +4h, payload at +8h.
inline constexpr std::size_t kUnitGunneryListNodeOffNext = 0x04;
inline constexpr std::size_t kUnitGunneryListNodeOffPayload = 0x08;

// The director bridge adapter 00863A80 allocates. 14h bytes.
inline constexpr std::size_t kUnitGunneryBridgeOffOwner = 0x00;
inline constexpr std::size_t kUnitGunneryBridgeOffDirector = 0x04;
inline constexpr std::size_t kUnitGunneryBridgeOffCountdown = 0x08;
inline constexpr std::size_t kUnitGunneryBridgeOffAllowFireCache = 0x0C;
inline constexpr std::size_t kUnitGunneryBridgeOffTorpedoCache = 0x0D;
inline constexpr std::size_t kUnitGunneryBridgeOffArtilleryCache = 0x0E;
inline constexpr std::size_t kUnitGunneryBridgeOffAaCache = 0x0F;
inline constexpr std::size_t kUnitGunneryBridgeOffDepthChargeCache = 0x10;
inline constexpr std::size_t kUnitGunneryBridgeSize = 0x14;

// The visibility cache 00864BD0 allocates. 24h bytes, 0Ch-byte entries.
inline constexpr std::size_t kUnitGunneryVisibilityOffUnit = 0x00;
inline constexpr std::size_t kUnitGunneryVisibilityOffEntries = 0x04;
inline constexpr std::size_t kUnitGunneryVisibilityOffSize = 0x08;
inline constexpr std::size_t kUnitGunneryVisibilityOffCapacity = 0x0C;
inline constexpr std::size_t kUnitGunneryVisibilityOffCountdown = 0x10;
inline constexpr std::size_t kUnitGunneryVisibilityEntryStride = 0x0C;
inline constexpr std::size_t kUnitGunneryVisibilitySize = 0x24;

// ---------------------------------------------------------------------------
// The four weapon groups the director's sub-kind flags drive. The names are the
// string literals of the parameter descriptor at 00CE5214,
// "All|AA_Flak|Artillery|Torpedo|DC", and the bit numbers are the unit+634h bits
// the mission-script actions at 00465290 and 00465350 edit. Section 3.
// ---------------------------------------------------------------------------
enum class WeaponGroup {
    kAaFlak = 0,      // director+221h, unit+634h bit 0, category mask bit 0
    kArtillery = 1,   // director+220h, unit+634h bit 1, category mask bit 1
    kTorpedo = 2,     // director+222h, unit+634h bit 2
    kDepthCharge = 3, // director+223h, unit+634h bit 3
};

// The category membership lists, read from 00E0A4F8, 00E0A510, 00E0A520 and
// 00E0A528. Each native list is terminated by the first entry >= 0Ch.
inline constexpr std::array<int, 5> kArtilleryCategories{1, 2, 3, 4, 6};
inline constexpr std::array<int, 3> kAaFlakCategories{1, 5, 6};
inline constexpr std::array<int, 1> kTorpedoCategories{7};
inline constexpr std::array<int, 2> kDepthChargeCategories{8, 9};

// The two bits of unit_gunnery mask word 80h+cat*4 that 008633D0 tests.
inline constexpr std::uint32_t kCategoryMaskMayEngagePlane = 0x1;
inline constexpr std::uint32_t kCategoryMaskMayEngageOther = 0x2;

// The six skill levels of every robot descriptor, from the key literals at
// 00D18398..00D183CC. Shared by every gun bot; the index is unit+390h.
enum class RobotSkillLevel {
    kStun = 0,
    kSinglePlayerNormal = 1,
    kSinglePlayerVeteran = 2,
    kMultiplayerNormal = 3,
    kMultiplayerVeteran = 4,
    kElite = 5,
};
inline constexpr int kRobotSkillLevelCount = 6;

// Every robot descriptor is a 0Ch-byte header followed by six skill records:
// vtable at +0h, NoTargetTimeUntilRest at +4h, the class name pointer at +8h.
inline constexpr std::size_t kRobotDescriptorHeaderSize = 0x0C;
inline constexpr std::size_t kRobotDescriptorOffRestTime = 0x04;
inline constexpr std::size_t kRobotDescriptorOffName = 0x08;

// ---------------------------------------------------------------------------
// Rules
// ---------------------------------------------------------------------------

// 00727BD0. Inverts one authored preference list into the rank row the pass
// indexes: the one-based position of a class id, zero when it is absent.
// `out` must have kUnitGunneryClassIdCount entries and is cleared first.
void build_target_rank_row_00727bd0(const int* preference_list,
                                    int preference_count,
                                    int* out) noexcept;

// 008624C0 step 3a and 3b, plus the four setters at 00861CD0, 00861D20,
// 00861D70 and 00861DC0. The stance a director pushes into the pass.
struct DirectorGunneryStance {
    bool allow_fire = true;        // director+3Ch
    bool artillery = true;         // director+220h
    bool anti_air = true;          // director+221h
    bool torpedo = true;           // director+222h
    bool depth_charge = true;      // director+223h
    bool torpedo_category_enable = true; // director+3Dh -> this+7Ch
};

// The twelve-category enable and mask state the bridge maintains.
struct UnitGunneryCategoryState {
    std::array<bool, kUnitGunneryCategoryCount> enabled{};
    std::array<std::uint32_t, kUnitGunneryCategoryCount> mask{};
    bool torpedo_enable = false;       // this+7Ch
    bool torpedo_group_flag = false;   // this+77h, written by 00861D70
    bool depth_charge_group_flag = false; // this+78h, written by 00861DC0
};

// The constructor's starting state: every category enabled, every mask 3.
UnitGunneryCategoryState unit_gunnery_initial_category_state_00864580() noexcept;

// 008624C0. `owner_is_plane` is [this+50h]->vtable[5Ch](0Fh), used only by
// step 3b. `countdown` is adapter+8h and `allow_fire_cache` is adapter+0Ch; both
// are read and written as the native body does. Returns the state after the push.
UnitGunneryCategoryState apply_director_stance_008624c0(
    const UnitGunneryCategoryState& previous,
    const DirectorGunneryStance& stance,
    bool owner_is_plane,
    bool force,
    int& countdown,
    bool& allow_fire_cache) noexcept;

// 008633D0 steps 2 and 3: the mask bit that must be set for a target of this
// class. A plane needs bit 0, anything else needs bit 1.
bool category_mask_admits_target_008633d0(std::uint32_t mask,
                                          bool target_is_plane) noexcept;

// 00862820 step 1: the four liveness bytes a candidate must pass.
struct GunneryTargetLiveness {
    bool registered = false;  // target+5Ch != 0
    bool dead = false;        // target+5Dh != 0
    bool flag60 = false;      // target+60h != 0
    bool flag5e = false;      // target+5Eh != 0
};
bool target_is_engageable_00862820(const GunneryTargetLiveness& liveness) noexcept;

// One entry of the stack candidate array: 0086513C builds eighty of these.
struct GunneryCandidate {
    void* entity = nullptr;
    float distance = 0.0f;
    int rank = 0;   // DAT_00E19BF8[classId + category*61h]
};

// 00865284..0086542B. The ordered insert the recon sweep uses. `order` holds
// indices into `candidates`; the scan stops at the first position where the new
// candidate outranks the one already there, ties broken on distance. Returns the
// insertion index.
int insert_ranked_candidate_00865284(const GunneryCandidate* candidates,
                                     int* order,
                                     int count,
                                     int new_index) noexcept;

// 008657A3. The walk order step 8.8 uses: the order array from the last entry
// back to index 0, which is best rank first and nearest first inside a rank.
// Writes `count` indices into `out` and returns `count`.
int candidate_walk_order_008657a3(const int* order, int count, int* out) noexcept;

// 008657C0..00865820. The per-gun decision for one candidate.
enum class GunneryCandidateVerdict {
    kSkip,       // try the next candidate
    kAssign,     // 00727F10(gun)(entity, is_fire_target)
};

struct GunneryGunInputs {
    bool is_torpedo_class_launcher = false; // 005459E0
    float minimum_air_range = 0.0f;         // 00729B90
    bool slot_accepts_target = false;       // 00729BC0
    int weapon_sub_type = 0;                // [[gun+3F4h]+80h]
};

GunneryCandidateVerdict evaluate_candidate_for_gun_008657c0(
    const GunneryGunInputs& gun,
    const GunneryCandidate& candidate,
    bool candidate_is_plane,
    bool candidate_is_fire_target,
    int category,
    bool torpedo_may_take_fire_target) noexcept;

// 00865838. The weapon sub-types that also get an engagement record.
bool weapon_sub_type_wants_target_record_00865838(int weapon_sub_type) noexcept;

// 00729BC0's dispatch on the projectile kind at [proj+8h]. Returns the gun
// offset of the bot slot, or 0 when the kind maps to none.
std::size_t bot_slot_for_projectile_kind_00729bc0(int projectile_kind,
                                                  int weapon_sub_type) noexcept;

// 00863A34. The per-category range gate, and 00863A4F's loitering-plane penalty.
struct GunneryScoreInputs {
    float distance = 0.0f;
    float category_range = 0.0f;   // unit+430h + category*4
    bool target_is_plane = false;
    bool target_lacks_follow_target = false; // 007B8AD0
};
struct GunneryScoreResult {
    bool accepted = false;
    float distance = 0.0f;
};
GunneryScoreResult score_candidate_00863990(const GunneryScoreInputs& in) noexcept;

// 00862C30. Ages one visibility cache entry; returns true when it survives.
bool visibility_entry_survives_00862c30(float& ttl, float dt) noexcept;

// ---------------------------------------------------------------------------
// The host. One virtual per native call site 00864FE0 makes. Nothing here
// allocates or owns anything; the sequence routine below drives it.
// ---------------------------------------------------------------------------
struct UnitGunneryPassHost {
    virtual ~UnitGunneryPassHost() = default;

    // Step 0 and step 1, 00864FF0..00865035.
    virtual bool unit_present() = 0;                    // this+50h
    virtual bool unit_is_dead() = 0;                    // [unit+5Dh]
    virtual float throttle_threshold_00432650() = 0;    // [GlobalConfig+88h]

    // Step 2, 0086503B.
    virtual bool unit_allows_sweep() = 0;               // [unit+61h]

    // Steps 3 to 6.
    virtual void age_visibility_cache_00862c30(float elapsed) = 0;
    virtual bool has_director_bridge() = 0;             // this+64h
    virtual void apply_director_stance_008624c0() = 0;
    virtual void update_target_records_00862cd0() = 0;  // the this+B0h walk

    // Step 8.1 to 8.4.
    virtual bool category_record_present(int category) = 0;      // unit+394h
    virtual bool category_enabled(int category) = 0;             // this+70h, this+80h
    virtual bool torpedo_category_enabled() = 0;                 // this+7Ch
    virtual bool torpedo_may_take_fire_target() = 0;             // this+7Dh
    virtual bool category_gate_slot4(int category) = 0;          // [this+60h]->vtable[4]

    // Step 8.5.
    virtual bool sweep_suppressed() = 0;                         // this+59h
    virtual int recon_contact_count_008053c0() = 0;              // [recon+DE8h]
    virtual void* recon_contact(int index) = 0;
    virtual bool score_candidate_00863990(int category, void* target, float& distance) = 0;
    virtual bool unit_ai_suppresses_00862440(void* target) = 0;
    virtual bool visible_00864d90(void* target) = 0;
    virtual int target_rank(int category, void* target) = 0;     // DAT_00E19BF8

    // Step 8.6.
    virtual void* director_fire_target_slot4() = 0;              // [this+5Ch]->vtable[4]
    virtual void* director_command_target_0071ebf0() = 0;        // 0071EBF0 then 00521EA0

    // Step 8.7.
    virtual std::array<float, 3> unit_world_position_00427eb0() = 0;
    virtual int sub_entities_slot0fc(void* target) = 0;          // target->vtable[0FCh]
    virtual void* sub_entity(int index) = 0;
    virtual std::array<float, 3> entity_world_position(void* entity) = 0;
    virtual float vector_length_0042b2f0(const std::array<float, 3>& v) = 0;

    // Step 8.8.
    virtual int category_gun_count(int category) = 0;            // unit+398h list
    virtual void* category_gun(int category, int index) = 0;
    virtual GunneryGunInputs gun_inputs(void* gun, void* target) = 0;
    virtual bool target_is_plane(void* target) = 0;              // vtable[5Ch](0Fh)
    virtual void set_bot_fire_target_00727f10(void* gun, void* target,
                                              bool is_fire_target) = 0;
    virtual void add_gun_to_target_record_00864ca0(void* target, void* gun) = 0;
    virtual void clear_bot_fire_target_00728000(void* gun) = 0;
};

// 00864FE0 as a sequence over the host. `throttle` is this+6Ch and `enabled` is
// this+58h; both are read and written exactly as the native body does.
void unit_gunnery_pass_tick_00864fe0(UnitGunneryPassHost& host,
                                     float dt,
                                     float& throttle,
                                     bool& enabled) noexcept;

}  // namespace bsp

#endif  // BSP_UNIT_GUNNERY_PASS_HPP
