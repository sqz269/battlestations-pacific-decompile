#pragma once
// How an AI-controlled unit picks what its guns shoot at.
//
// Two independent mechanisms meet here, and docs/BOT_FIRE_TARGET.md records the
// evidence for both:
//
//  * The weapon director's automatic target selector, the 40h-byte subobject at
//    director+38h that BSP_WeaponDirector_Construct (008366D0) builds with
//    009F6A20 at 0083676A. Its one virtual, 009F5DA0 (vtable 00D21B48 slot 1,
//    the derived half of the pair 00D21B40 / 00D21B48 in .rdata), runs about
//    once a second, scans the owner's party list, scores every candidate and
//    hands the winner to BSP_WeaponDirector_SetFireTarget (00835860).
//
//  * The gun-side bot ticks in segments 33, 59 and 62 that turn a chosen target
//    into a pair of gun angles and offer them to BSP_TurningGun_SetTargetAngles
//    (0085ABA0), which accepts a pair only inside the platform firing window.
//
// Every descriptive name in this header is a hypothesis, not a recovered symbol.
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/gun_aiming.hpp" // NativeHandle and the gun field offsets

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kAutoTargetTickAddress = 0x009F5DA0u;
inline constexpr std::uint32_t kAutoTargetConstructAddress = 0x009F6A20u;
inline constexpr std::uint32_t kAutoTargetSearchInitAddress = 0x009F69C0u;
inline constexpr std::uint32_t kAutoTargetPriorityBuildAddress = 0x009F65E0u;
inline constexpr std::uint32_t kAutoTargetScanAddress = 0x009F5D30u;
inline constexpr std::uint32_t kAutoTargetScoreAddress = 0x009F5B70u;
inline constexpr std::uint32_t kAutoTargetSwitchTestAddress = 0x009F52F0u;
inline constexpr std::uint32_t kAutoTargetEngageableAddress = 0x009F59F0u;
inline constexpr std::uint32_t kAutoTargetDestructorAddress = 0x009F6170u;

// The vtable pair in .rdata. 00D21B40 is the base half (00D21B40 = 009F5290,
// 00D21B44 = __purecall 00BF698E); 00D21B48 is the derived half the constructor
// installs (00D21B48 = the deleting destructor 009F6170, 00D21B4C = the tick
// 009F5DA0). Read from the image, not from Ghidra xrefs: nothing in .text holds
// a 4-aligned pointer to 00D21B4C, and both halves are written as immediates.
inline constexpr std::uint32_t kAutoTargetBaseVtable = 0x00D21B40u;
inline constexpr std::uint32_t kAutoTargetDerivedVtable = 0x00D21B48u;
inline constexpr std::size_t kAutoTargetVtableSlotTick = 0x04; // 00D21B4C

// The gun-side ticks that reach 0085ABA0.
inline constexpr std::uint32_t kGunBotTurretTickAddress = 0x008FFA20u;
inline constexpr std::uint32_t kGunBotHeadingTickAddress = 0x008FFF20u;
inline constexpr std::uint32_t kGunBotLeadTickAddress = 0x00902920u;
inline constexpr std::uint32_t kGunBotBallisticTickAddress = 0x009030C0u;
inline constexpr std::uint32_t kGunBotMuzzleTickAddress = 0x006DF520u;
inline constexpr std::uint32_t kUnitGunAimMessageAddress = 0x00959C20u;
inline constexpr std::uint32_t kGunBotAnglesFromDirectionAddress = 0x008FDAF0u;
inline constexpr std::uint32_t kGunBotTriggerAddress = 0x008FEF40u;

// The three other BSP_WeaponDirector_SetFireTarget call sites.
inline constexpr std::uint32_t kFireTargetOrderApplyAddressA = 0x008438B0u;
inline constexpr std::uint32_t kFireTargetOrderApplyAddressB = 0x00744A90u;
inline constexpr std::uint32_t kFireTargetFromEntityAddress = 0x004643A0u;

// ---------------------------------------------------------------------------
// Layout of the selector, director+38h, 40h bytes
// ---------------------------------------------------------------------------
inline constexpr std::size_t kAutoTargetSize = 0x40;
inline constexpr std::size_t kAutoTargetOffVtable = 0x00;       // 009F6A45
inline constexpr std::size_t kAutoTargetOffOwnerDirector = 0x04; // 009F6A42
inline constexpr std::size_t kAutoTargetOffUnit = 0x08;         // 009F6A51, director+24Ch
inline constexpr std::size_t kAutoTargetOffDirector = 0x0c;     // 009F6A9x, the same director
inline constexpr std::size_t kAutoTargetOffSearchState = 0x10;  // ECX of 009F5D30 at 009F5E7C
inline constexpr std::size_t kAutoTargetOffListBegin = 0x14;    // freed by 009F6170
inline constexpr std::size_t kAutoTargetOffListEnd = 0x18;      // 009F6170 clears it
inline constexpr std::size_t kAutoTargetOffListCapacity = 0x1c; // 009F6170 clears it
inline constexpr std::size_t kAutoTargetOffMaxRange = 0x20;     // 009F660x writes searchState+10h
inline constexpr std::size_t kAutoTargetOffSelf = 0x24;         // 009F69F5 writes searchState+14h
inline constexpr std::size_t kAutoTargetOffBestCandidate = 0x28; // 009F5CF1
inline constexpr std::size_t kAutoTargetOffBestScore = 0x2c;    // 009F5CF4
inline constexpr std::size_t kAutoTargetOffIssueMoveFlag = 0x30; // 009F5D02, read at 009F5ED9
inline constexpr std::size_t kAutoTargetOffThinkInterval = 0x34; // 009F5DB9
inline constexpr std::size_t kAutoTargetOffThinkCountdown = 0x38; // 009F5DA6
inline constexpr std::size_t kAutoTargetOffRetainedScore = 0x3c; // 009F5E77

// Director fields the tick reads. The names for +3Dh, +54h and +23Ch come from
// docs/WEAPON_DIRECTOR.md; kDirectorOffAllowMove and kDirectorOffCommandSlots
// are already declared in include/bsp/weapon_director.hpp and are not repeated.
inline constexpr std::size_t kDirectorOffCommandState = 0x30;   // 009F5E69, compared with 2
inline constexpr std::size_t kDirectorOffCommandTarget = 0x18c; // 009F5EBE, 00521EA0's argument
inline constexpr std::size_t kDirectorOffFireTargetLocked = 0x23c; // 009F5E37, 009F5F12
inline constexpr std::size_t kDirectorVtableSlotCurrentTarget = 0x2c; // 009F5E27, 009F5F06

// The command singletons the tick compares against, named in
// docs/SCENE_COMMAND_TYPES.md by their index in the 00E08EF8 + 8*(k-1) table.
inline constexpr std::uint32_t kTargetSelectorFollowCommandObject = 0x00E08F60u;     // index 14
inline constexpr std::uint32_t kCommandObjectAttackMoveAddr = 0x00E08F78u; // index 17
inline constexpr std::uint32_t kCommandObjectSetTargetAddr = 0x00E08EF8u;  // index 1
inline constexpr std::uint32_t kCommandObjectClearTargetAddr = 0x00E08F00u; // index 2
inline constexpr std::uint32_t kCommandObjectClearOrdersAddr = 0x00E08F08u; // index 3

// ---------------------------------------------------------------------------
// Selection constants
// ---------------------------------------------------------------------------
// 009F6A20 seeds the think interval with 1.0f (00D7A24C) and the countdown with
// the negation of a random float in [0, 1) (00BD2F10 at 009F6A67), so every
// director's once-a-second think starts on a different frame.
inline constexpr float kAutoTargetThinkInterval = 1.0f;
// 009F6A20 seeds the retained score with FLT_MAX (00D7A248); 009F5E6F restores
// it whenever the director's command state is not 2.
inline constexpr float kAutoTargetRetainedScoreReset = 3.4028234663852886e+38f;
// 009F5CBE. The priority tier is worth this many metres of distance.
inline constexpr float kAutoTargetTierWeight = 10000.0f;
// 009F52F0 multiplies the retained score by the 00CE3D40 double before the
// comparison. gun_aiming.hpp already declares that constant as
// kGunAimSnapThreshold; this is the same 0.8 and is deliberately not redeclared.

// The three max ranges 009F65E0 writes, chosen by the owner's kind.
inline constexpr float kAutoTargetRangeKindE = 2500.0f; // 00D20278
inline constexpr float kAutoTargetRangeKind7 = 3500.0f; // 00D04698, also kind 0Dh
inline constexpr float kAutoTargetRangeKindA = 3000.0f; // 00CFA424, also kind 8

// Entity kind ids as the IsKindOf predicate (entity vtable[5Ch], written n(k)
// in the other docs) takes them. The 7..0Eh ids are the ones this packet sees;
// what each one names is NOT established here.
inline constexpr int kEntityKindCandidateGate = 6;  // 009F5B81, every candidate
inline constexpr int kEntityKindNeedsSpecialWeapon = 8; // 009F59FB
inline constexpr int kEntityKindWeaponDevice = 0x20; // 009F5A1x, matches n(20h)

// 009F59F0 accepts a kind-8 candidate only when the owner carries a weapon
// device whose class at [device+3F4h]+80h is one of these two.
inline constexpr int kOwnerWeaponClassA = 8;  // 009F5A2x
inline constexpr int kOwnerWeaponClassB = 9;  // 009F5A3x

// ---------------------------------------------------------------------------
// Gun-side constants
// ---------------------------------------------------------------------------
// 008FFA20's aim error, a random draw in [0, errorDegrees) scaled by pi/180.
inline constexpr float kGunBotDegreesToRadiansNumerator = 3.1415927410125732f; // 00CE3D28
inline constexpr float kGunBotDegreesToRadiansDenominator = 180.0f;           // 00CE3D20
// 008FFA20's fire gate, both halves of one hysteresis.
inline constexpr float kGunBotOpenFireRangeMargin = 40.0f;   // 00D7A378
inline constexpr float kGunBotCeaseFireRangeMargin = 20.0f;  // 00CE3D88
inline constexpr float kGunBotOpenFireAngleSum = 0.10471975803375244f;  // 00D18390, six degrees
inline constexpr float kGunBotCeaseFireAngleSum = 0.15707963705062866f; // 00D18388, nine degrees
// 008FEF40's debounce.
inline constexpr float kGunBotTriggerPressDelay = 0.1f;   // 00D17D3C
inline constexpr float kGunBotTriggerReleaseDelay = 0.3f; // 00CE69C8
// 008FDAF0 negates the horizontal angle through 0.0f - h (00D7A208 is -0.0f),
// the same convention 0085B980 uses.
inline constexpr float kGunBotHorzAngleNegateBase = -0.0f; // 00D7A208

// Bot fields 008FFA20 and its siblings share. The stride is the bot object, not
// the gun; the gun is the pointer at +68h.
inline constexpr std::size_t kGunBotOffFireStateCommitted = 0x58; // 008FEF40 param_1[0x16]
inline constexpr std::size_t kGunBotOffFireStateRequested = 0x59; // 008FEF40
inline constexpr std::size_t kGunBotOffTriggerTimer = 0x5c;       // 008FEF40 param_1[0x17]
inline constexpr std::size_t kGunBotOffGun = 0x68;                // param_1[0x1a]
inline constexpr std::size_t kGunBotOffThinkCountdown = 0x6c;     // param_1[0x1b]
inline constexpr std::size_t kGunBotOffAimHorz = 0x70;            // param_1[0x1c]
inline constexpr std::size_t kGunBotOffAimVert = 0x74;            // param_1[0x1d]
inline constexpr std::size_t kGunBotOffLeadPoint = 0x78;          // param_1[0x1e..0x20]
inline constexpr std::size_t kGunBotOffAimErrorDegrees = 0x84;    // param_1[0x21]
inline constexpr std::size_t kGunBotOffThinkMin = 0x88;           // param_1[0x22]
inline constexpr std::size_t kGunBotOffThinkMax = 0x8c;           // param_1[0x23]
inline constexpr std::size_t kGunBotOffMaxRange = 0x90;           // param_1[0x24]
inline constexpr std::size_t kGunBotVtableSlotClearTarget = 0x38; // 008FFA6C
inline constexpr std::size_t kGunBotVtableSlotTargetEntity = 0x44; // 008FFAE8

// ---------------------------------------------------------------------------
// The selection rule as pure functions over an injected candidate view
// ---------------------------------------------------------------------------

// One entry of the priority list 009F65E0 builds. The scorer reads only the
// kind; the flag at +0h is written by every push and read by nothing this
// packet found.
struct AutoTargetPriorityEntry {
    bool flag = false; // element +0h
    int entity_kind = 0; // element +4h, the argument of IsKindOf at 009F5C16
};

// The whole per-owner search configuration, searchState = selector+10h.
struct AutoTargetSearchConfig {
    std::vector<AutoTargetPriorityEntry> priority; // +4h begin, +8h end
    float max_range = 0.0f;                        // +10h
};

// 009F65E0. `owner_kind_test` answers the owner's IsKindOf questions in the
// order the routine asks them; the first hit wins.
struct AutoTargetOwnerKinds {
    bool kind_0e = false;
    bool kind_07 = false;
    bool kind_0a = false;
    bool kind_0d = false;
    bool kind_08 = false;
    bool kind_0b = false;
    bool kind_09 = false;
    bool kind_0c = false;
};
AutoTargetSearchConfig auto_target_build_priority_009f65e0(
    const AutoTargetOwnerKinds& kinds) noexcept;

// 009F5B70's score for a candidate that matched priority entry `index` of a
// list of `entry_count` entries, at `distance` metres. Higher is better.
float auto_target_score_009f5b70(int entry_count, int index, float distance) noexcept;

// 009F5C9B. A candidate further than the configured range is dropped before it
// is scored.
bool auto_target_distance_in_range_009f5b70(float distance, float max_range) noexcept;

// 009F5CD1. The scan keeps the maximum, and the running best starts at 0.0f
// (009F5D49), so a negative score never wins: the last priority tier can only
// be selected when the list has more than one entry above it.
bool auto_target_score_beats_best_009f5b70(float score, float best) noexcept;

// 009F52F0, __stdcall(float candidate, float retained) -> bool, RET 8.
bool auto_target_switch_allowed_009f52f0(float candidate_score,
                                         float retained_score) noexcept;

// 009F59F0. A candidate of kind 8 needs an owner weapon device of class 8 or 9.
struct AutoTargetOwnerWeapon {
    bool is_weapon_device = false; // IsKindOf(20h) at 009F5A1x
    int weapon_class = 0;          // [device+3F4h]+80h
};
bool auto_target_owner_can_engage_009f59f0(
    bool candidate_is_kind_8,
    bool candidate_blocked, // 00852820's answer, true suppresses the search
    const std::vector<AutoTargetOwnerWeapon>& owner_devices) noexcept;

// One candidate as the scan sees it.
struct AutoTargetCandidate {
    void* entity = nullptr;
    bool passes_candidate_gate = false; // IsKindOf(6) at 009F5B81
    bool skip_all_entries = false;      // the +5Dh byte at 009F5BFA
    bool zone_blocked = false;          // 0071C4F0 at 009F5BB4
    bool state_blocked = false;         // 00811F50 at 009F5BC1
    int entity_kind_matches = -1;       // the first priority index IsKindOf accepts
    float distance = 0.0f;
    bool owner_can_engage = true;       // 009F59F0's answer
    bool issue_move_flag = false;       // the byte 009F5860 returns
};

struct AutoTargetScanResult {
    void* best = nullptr;
    float best_score = 0.0f;
    bool issue_move_flag = false;
};

// 009F5D30 plus 009F5B70: the whole scan over one party list.
AutoTargetScanResult auto_target_scan_009f5d30(
    const AutoTargetSearchConfig& config,
    const std::vector<AutoTargetCandidate>& candidates) noexcept;

// ---------------------------------------------------------------------------
// The per-gun aim-point rule
// ---------------------------------------------------------------------------

struct GunAimAngles {
    float horz = 0.0f;
    float vert = 0.0f;
};

// 008FDAF0: the world-space direction is transformed by the gun's derived
// affine inverse, 00521370 turns the local direction into a pair, and the
// horizontal half is negated.
GunAimAngles gun_bot_angles_from_local_008fdaf0(float local_horz,
                                                float local_vert) noexcept;

// 008FFA20's aim error: one draw per angle, each in [0, degrees) converted to
// radians. `draw` is the value 00BD2F10 returned.
float gun_bot_apply_aim_error_008ffa20(float angle, float draw) noexcept;

// 008FFA20's fire gate, the pair of thresholds at 008FFEB0..008FFF03. `firing`
// is the bot's current committed state; the result is the request handed to
// 008FEF40.
bool gun_bot_wants_fire_008ffa20(bool firing, float distance, float max_range,
                                 float horz_error, float vert_error) noexcept;

// 008FEF40's debounce, applied for one step of `dt` seconds.
struct GunBotTriggerState {
    bool committed = false;  // +58h
    bool requested = false;  // +59h
    float timer = 0.0f;      // +5Ch
};
void gun_bot_trigger_step_008fef40(GunBotTriggerState& state, bool request,
                                   float dt) noexcept;

// ---------------------------------------------------------------------------
// The routines as sequences over an injected host
// ---------------------------------------------------------------------------

// One virtual per native call site of 009F5DA0. Nothing here allocates; the
// session message the director sends is a contract, not a port.
struct BotFireTargetHost {
    virtual ~BotFireTargetHost() = default;

    // -- 009F5DA0, the tick -------------------------------------------------
    virtual bool controller_belongs_to_another(void* unit) = 0;   // 007788B0 at 009F5DC4
    virtual void* director_command_slot() = 0;                    // [director+54h] at 009F5DD0
    virtual void release_controller(void* unit, int mode) = 0;    // 0077C980 at 009F5DEB
    virtual bool unit_suppresses_targeting(void* unit) = 0;       // [unit+184h] at 009F5E06
    virtual bool selection_enabled() = 0;                         // 009F5610 at 009F5E15
    virtual void send_command_state(void* director, int state) = 0; // 0071D9E0 at 009F5F41
    virtual void* director_current_target() = 0;                  // vtable[2Ch] at 009F5E2C
    virtual bool director_target_locked() = 0;                    // [director+23Ch] at 009F5E37
    virtual int director_command_state() = 0;                     // [director+30h] at 009F5E69
    virtual void* build_command_target(void* entity, float value) = 0; // 00465080 at 009F5E4B
    virtual bool command_accepts_target(std::uint32_t command,
                                        void* command_target) = 0; // 0071D6D0 at 009F5E59
    virtual AutoTargetScanResult scan_party_list() = 0;           // 009F5D30 at 009F5E7F
    virtual void* resolve_command_target_object() = 0;            // 00521EA0 at 009F5EC4
    virtual bool director_accepts_new_target() = 0;               // 0071DF70 at 009F5ED0
    virtual void issue_command(std::uint32_t command,
                               void* command_target) = 0;         // 0071D980 at 009F5EF8
    virtual void set_fire_target(void* entity, bool force) = 0;   // 00835860 at 009F5F21

    // -- 009F5610, the enable gate ------------------------------------------
    virtual bool director_allow_move() = 0;                       // [director+3Dh] at 009F5614
    virtual int command_slot_kind(void* slot) = 0;                // vtable[0Ch] at 009F562x
};

// The selector's mutable state, the 40h-byte subobject.
struct AutoTargetState {
    float think_interval = kAutoTargetThinkInterval; // +34h
    float think_countdown = 0.0f;                    // +38h
    float retained_score = kAutoTargetRetainedScoreReset; // +3Ch
    bool issue_move_flag = false;                    // +30h
};

// 009F5DA0, __thiscall(this, float dt), RET 4, body 009F5DA0..009F5F45.
void auto_target_tick_009f5da0(BotFireTargetHost& host, AutoTargetState& state,
                               void* unit, float dt);

// 009F5610, __fastcall(this) -> bool, the gate the tick runs before it scans.
bool auto_target_selection_enabled_009f5610(bool allow_move, bool has_command_slot,
                                            int command_slot_kind) noexcept;

// One row of the order-driven setter table, 008438B0 and 00744A90.
struct FireTargetOrder {
    std::uint32_t command = 0; // the singleton BSP_EntityOrderMessage_CommandFromOrdinal returned
    bool applies = false;      // whether SetFireTarget is called at all
    bool clears = false;       // whether the target argument is null
};
FireTargetOrder fire_target_order_008438b0(std::uint32_t command) noexcept;

} // namespace bsp
