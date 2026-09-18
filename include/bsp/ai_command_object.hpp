#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/ai_planners.hpp"

// The AI command object a group carries at group+564Ch: how it is installed,
// what replaces it, how it answers the auto-merge predicate and what the
// group's per-member pass 00A2C790 does with it. Reconstructed from the
// read-only analysis of packet cc8_ai_command_object.
// docs/AI_COMMAND_OBJECT.md carries the address evidence. Every descriptive
// name here is a hypothesis, not a recovered symbol.
//
// The class table, the id space, the IsType families, the merge-override rule
// and the Lua factory recipe already live in bsp/ai_planners.hpp and
// bsp/ai_planner_tails.hpp; this header adds only what packet
// cc8_ai_command_object read for the first time and does not restate them.
//
// Coverage. Complete as rules: the group constructor's initial class
// (00A2E0F1-00A2E140 as docs/AI_GROUP_THINK.md transcribes it), the install
// 00A2BD00, the shared member-pass interval 00A0FC50, the notification census
// over all sixteen vtables, the IDLE secondary predicate 00A2C600 and the
// member pass 00A2C790 read to its RET. Partial: the per-class tick bodies at
// vt+0Ch below the three shared helpers 00A10EC0 / 00A10DC0 / 00A11070, which
// were not read.

namespace bsp {

// ---------------------------------------------------------------------------
// Where the object lives
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kAiGroupCommandSlot = 0x564Cu;        // group+564Ch
inline constexpr std::uint32_t kAiGroupMemberPassDueSlot = 0x5650u;  // group+5650h
inline constexpr std::uint32_t kAiGroupPopulationSlot = 0x5644u;     // group+5644h

// The MOVE family's destination, written by the factory's MOVETO arm at
// 00A134C5-00A134DC as three floats.
inline constexpr std::uint32_t kAiCommandMoveTargetPosition = 0x08u; // +8h,+0Ch,+10h

// 00A2C790 reaches a member's weapon director through this entity slot, not
// through the command. The packet brief called it a command slot; it is a slot
// of the member entity's own vtable.
inline constexpr std::uint32_t kAiEntityVtableWeaponDirector = 0x114u;

// 00A0FC50, the vt+10h body every one of the sixteen classes shares. It writes
// the two constants below and returns; the group's next member pass is drawn
// uniformly between them.
inline constexpr float kAiCommandMemberPassIntervalLow = 2.0f;   // 00CE3958
inline constexpr float kAiCommandMemberPassIntervalHigh = 4.0f;  // 00CE3D34

// ---------------------------------------------------------------------------
// The object
// ---------------------------------------------------------------------------

// The fields a reconstruction needs. The native instance also carries the
// observer sub-object at +8h that the ATTACK and MOVE families install; this
// process holds no observers, so it is not modelled.
struct AiCommandObject {
    AiCommandType type{AiCommandType::NonControl};
    void* owner_group{nullptr};   // +4h, written by every arm of the factory
    void* target_group{nullptr};  // +1Ch, the ATTACK family only
    float target_position[3]{0.0f, 0.0f, 0.0f};  // +8h..+10h, the MOVE family only
};

// The group constructor's choice at 00A2E0F1-00A2E140: an 8-byte instance with
// the NONCONTROL vtable 00D22990 when the group's AI party slot (+5634h) is
// negative, the IDLE vtable 00D229E0 when 009FFE50 admits that slot, and
// NONCONTROL otherwise. Both classes override the merge predicate, so a group
// can auto-merge from the moment it is built and stops being able to as soon
// as an order lands.
AiCommandType ai_command_initial_type(int ai_party_slot, bool party_slot_admitted) noexcept;

// 00A2BD00 BSP_AiGroup_SetCommand, __thiscall(group)(command), RET 4: when
// group+564Ch is non-null its vtable[+0h] runs with flag 1, then the new
// pointer is stored. True when the caller must destroy the outgoing object.
bool ai_command_install_deletes_previous(bool group_has_command) noexcept;

// vt+24h is 00A0FC90 (`RET 8`) in every one of the sixteen vtables between
// 00D22968 and 00D22C68, so no class acts on a member notification. This
// closes docs/AI_GROUP_THINK.md's "whether any class in the block overrides
// the slot was not settled".
bool ai_command_notification_is_discarded(AiCommandType type) noexcept;

// ---------------------------------------------------------------------------
// 00A2C600, the extra equality the IDLE merge override adds
// ---------------------------------------------------------------------------

// 009FE0B0, __thiscall(entity), RET 0: entity->vtable[+5Ch] is asked for class
// ids 0x1B, 0x45 and 0x46 in that order and the first hit wins. Which three
// entity classes those ids name was not resolved.
bool ai_entity_class_matches_009fe0b0(bool is_class_1b, bool is_class_45,
                                      bool is_class_46) noexcept;

// 00A2C600, __thiscall(group), RET 0: walks the +563Ch member list and returns
// true at the first member 009FE0B0 accepts. The IDLE override 00A12450
// requires the two groups to answer it alike; NONCONTROL's 00A11F80 does not
// ask at all.
bool ai_group_any_member_matches_009fe0b0(const bool* member_matches,
                                          std::size_t member_count) noexcept;

// ---------------------------------------------------------------------------
// The auto-merge predicate, assembled
// ---------------------------------------------------------------------------

// What 00A2C8D0 needs from both groups to reach a verdict through
// from+564Ch's vt+14h. The family gate (a ship group and an air group never
// merge) stays in ai_group_families_may_merge; this is the delegated half.
struct AiCommandMergeFacts {
    AiCommandType type{AiCommandType::NonControl};  // the absorbed group's command
    bool other_group_present{false};       // the argument is non-null
    std::uint32_t other_population{0};     // other+5644h
    std::uint32_t own_population{0};       // ownerGroup+5644h
    bool own_has_groupable_combatant{false};    // 00A2C5A0(owner)
    bool other_has_groupable_combatant{false};  // 00A2C5A0(other)
    bool own_matches_009fe0b0{false};      // 00A2C600(owner), IDLE only
    bool other_matches_009fe0b0{false};    // 00A2C600(other), IDLE only
    float own_leader_weight{0.0f};         // 009FFD70(owner's first member)
    float other_leader_weight{0.0f};       // 009FFD70(other's first member)
    float own_leader_position[3]{0.0f, 0.0f, 0.0f};
    float other_leader_position[3]{0.0f, 0.0f, 0.0f};
    float auto_merge_dist{0.0f};           // tuning +208h, AutoMerge_MergeDist
};

// Composes 00A11F80 / 00A12450 with 00A10D50 and 00A10C60. Any class other
// than NONCONTROL and IDLE answers false through the default 00A0FC80.
bool ai_command_can_merge_with(const AiCommandMergeFacts& facts) noexcept;

// ---------------------------------------------------------------------------
// 00A2C790, the group's per-member pass
// ---------------------------------------------------------------------------

// One method per native call the pass makes.
struct AiCommandMemberPassHost {
    virtual ~AiCommandMemberPassHost() = default;

    virtual std::uint32_t group_population(void* group) = 0;            // +5644h
    virtual std::size_t group_member_count(void* group) = 0;            // the +563Ch list
    virtual void* group_member_at(void* group, std::size_t index) = 0;  // node+8h
    virtual void* group_command(void* group) = 0;                       // +564Ch

    // member->vtable[+114h], called once per member and again for each of the
    // two readers below. Null means the member has nothing to report.
    virtual void* member_weapon_director(void* member) = 0;
    virtual void* director_target_descriptor(void* director) = 0;       // 0071EB60
    virtual void* director_current_command(void* director) = 0;         // 0071BE40

    // command->vtable[+24h](currentCommand, targetDescriptor). 00A0FC90 in
    // every class, so a faithful host may do nothing but count the call.
    virtual void command_notify_member(void* command, void* current_command,
                                       void* target_descriptor) = 0;

    // command->vtable[+0Ch], the per-class tick, and 00A0FC50 at vt+10h which
    // hands back the interval bounds.
    virtual void command_tick(void* command) = 0;
    virtual void command_pass_interval(void* command, float* low, float* high) = 0;

    virtual float fixed_step_clock() = 0;                      // 00F876A4
    virtual float random_interval(float low, float high) = 0;  // 00BD2F10
    virtual float member_pass_due(void* group) = 0;            // +5650h
    virtual void store_member_pass_due(void* group, float when) = 0;
};

struct AiCommandMemberPassResult {
    std::uint32_t members_walked{0};
    std::uint32_t directors_found{0};
    std::uint32_t notifications_sent{0};
    bool ticked{false};
    float next_due{0.0f};
};

// 00A2C790, __thiscall(group), RET 0, body 00A2C790-00A2C8C6, read in full.
// Returns at once when the population is zero. Otherwise it walks the member
// list, and for every member whose weapon director is non-null it reports that
// director's current command and active target descriptor to the group's AI
// command. Then, when the group's due time has been reached, the command ticks
// and the next due time becomes the clock plus a uniform draw from the
// interval vt+10h supplies.
AiCommandMemberPassResult ai_command_member_pass_00a2c790(AiCommandMemberPassHost& host,
                                                          void* group);

}  // namespace bsp
