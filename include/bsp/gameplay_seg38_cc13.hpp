#pragma once

// Segment 38 (0071BA20-00722FB0) reference models: the weapon director's
// ten-slot command queue, its path objects and user-path messages, the
// command-target descriptor and the controller state/snapshot messages.
// Packet cc13-gpr38; docs/CC13_GPR_38.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw)
// and the Ghidra pseudocode. They are not native layouts and not
// ABI-compatible replacements. Field names are hypotheses; the "_NNN" suffix is
// the native byte offset. Every engine service the image calls (virtual
// queries, target resolution, the bit cursor, the session router) is an input,
// an output record or a host interface, so nothing here invents a global or
// stubs a call. Build-tested only; not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is docs/CC13_GPR_38.md section 2):
// - Replaces: nothing the hosts run today as a separate routine. The command
//   hosts (src/game_hosts_commands.cpp) fold the queue into per-unit rows. The
//   models that a host could adopt directly are:
//   * 0071DDB0 on_entity_removed: the host's release_observed_target_0071ddb0
//     (src/game_hosts_commands.cpp) answers one slot; the image walks all ten
//     and sends a slot clear (0071D900) for every later slot whose
//     vtable[74h] allows it;
//   * 0071EDD0 retarget_to_member_or_point: a squadron target is replaced by
//     its LAST live member (the distance gate is FLT_MAX, so it never rejects
//     a finite distance), not the nearest; a non-squadron target becomes a
//     fixed point at the entity's pose;
//   * 0071F290 controller_update: the +40h override-delay countdown and the
//     two vtable[78h] validity tests that end a command (stage 2).
// - Inputs: slot classes as the image's class addresses (00E08F68 moveto and
//   so on), the class category (vtable[0Ch]) and requires-target (vtable[8])
//   answers, resolved-target facts and entity live bytes, passed as values.
// - RNG: none. No model in this segment draws a random number.
// - OFF-switch prediction: OFF changes nothing. ON, only the three models above
//   could move a run: a squadron target with two or more live members picks
//   the last member instead of the host's choice; a dead target clears later
//   queued slots that the host keeps.

#include "bsp/object_handle_resolvers.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace bsp {
namespace gameplay_seg38_cc13 {

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Command classes by their image addresses (docs/COMMAND_CLASSES.md). These
// are identities the models compare against, not globals they read.
inline constexpr std::uint32_t kCmdTorpedo = 0x00E08F18u;
inline constexpr std::uint32_t kCmdFollow = 0x00E08F60u;
inline constexpr std::uint32_t kCmdMoveTo = 0x00E08F68u;
inline constexpr std::uint32_t kCmdCruise = 0x00E08F70u;
inline constexpr std::uint32_t kCmdAttackMove = 0x00E08F78u;
inline constexpr std::uint32_t kCmdMoveOnPath = 0x00E08F80u;
inline constexpr std::uint32_t kCmdStop = 0x00E08F88u;
inline constexpr std::uint32_t kCmdLand = 0x00E08FA0u;
inline constexpr std::uint32_t kCmdLeave = 0x00E08FB0u;
inline constexpr std::uint32_t kCmdDisband = 0x00E08FB8u;

inline constexpr int kCommandSlots = 10;

// The 18h-byte target record that follows each slot's class pointer
// (director+58h+i*1Ch; 0071DB50 copies it field by field) and that 00465080 /
// 00721760 build.
struct CommandTarget {
    bool has_entity_00 = false;
    bool has_point_01 = false;
    std::uint16_t entity_index_02 = 0;  // the entity's +174h network index
    std::uint32_t entity_04 = 0;        // cached entity pointer, 0 = unresolved
    Vec3f point_08{};
    std::uint32_t param_14 = 0;
};

// One queue slot: the class pointer at director+54h+i*1Ch and its target.
// `category` and `requires_target` are the class's vtable[0Ch] and vtable[8]
// answers, supplied by the caller.
struct CommandSlot {
    std::uint32_t command_class = 0;  // 0 = empty
    int category = 0;
    bool requires_target = false;
    CommandTarget target{};
};

// The slot's path object (director+1A4h+i*4) as the queue routines see it.
struct SlotPath {
    bool present = false;
    bool vt4_valid = false;      // path->vtable[4]()
    bool has_point_list = false; // path->vtable[8]() != 0
    int point_count = 0;         // its 0Ch-stride Vec3 vector size
};

struct DirectorQueue {
    std::array<CommandSlot, kCommandSlots> slots{};
    std::array<SlotPath, kCommandSlots> paths{};
};

// The loop every queue routine inlines: index of the first empty slot (10 when
// all are full). Not a separate image function.
int first_empty_slot(const DirectorQueue& q) noexcept;

// 0071BE60 __fastcall(director): the filled slot count, i.e. first_empty_slot.
int filled_slot_count_0071be60(const DirectorQueue& q) noexcept;

// 0071BFF0 __thiscall(director, uint i), RET 4: path(i)+10h, falling back to
// slot 0's path when i > 9 or path(i) is null (slot 0's own path is not
// checked: a null slot-0 path yields address 10h). Returns the slot used.
int command_path_cursor_slot_0071bff0(const DirectorQueue& q, unsigned index) noexcept;

// 0071EBF0 __fastcall(director): the target record a director is acting on.
// Mode 1: the newest filled slot (scanning down from the last) whose category
// is 1 or 2; mode 2: the override target (+18Ch); otherwise, or when no slot
// qualifies, the neutral static record 00E19BB4 (a zero target, lazily
// initialised under bit 0 of 00E19BCC).
enum class NewestTargetSource { kSlot, kOverride, kNeutral };
struct NewestTarget {
    NewestTargetSource source = NewestTargetSource::kNeutral;
    int slot = -1;
};
NewestTarget newest_command_target_0071ebf0(const DirectorQueue& q, std::int32_t mode_30) noexcept;

// 0071BA20 __thiscall(node, vector* out, const char* prefix), RET 8: out is
// cleared, then every 8-byte child entry of the node's +80h..+84h vector that
// BSP_ModelNoteNameEquals_Predicate accepts contributes its +4h dword, in
// order. The predicate is supplied.
std::vector<std::uint32_t> collect_children_by_prefix_0071ba20(
    const std::vector<std::array<std::uint32_t, 2>>& entries,
    const std::function<bool(const std::array<std::uint32_t, 2>&)>& accepts);

// 0071BDE0 __thiscall(director, uint i), RET 4: slot i's path object when the
// slot is moveonpath and the path's vtable[4]() holds; else null. Returns the
// slot index whose path is answered, or -1.
int slot_moveonpath_path_0071bde0(const DirectorQueue& q, unsigned index) noexcept;

// 0071C0C0 __thiscall(director, int want), RET 4: false (AL = 0, upper EAX
// bytes left) when want == 0; else first_empty_slot < 10.
bool has_free_command_slot_0071c0c0(const DirectorQueue& q, int want) noexcept;

// 0071C1B0 __thiscall(director, const int pair[2]): writes followMode/startMode
// (+8h/+0Ch) of the path object at director+1A0h+first_empty*4, i.e. the LAST
// filled slot's path. With an empty queue the index is -1 and the image reads
// director+1A0h, which is not a path pointer. Returns the slot index written
// (-1 for that case).
int path_follow_mode_slot_0071c1b0(const DirectorQueue& q) noexcept;

// 0071D780 __fastcall(director): commands counted, a valid moveonpath path
// counting as its point count (at least 1), stopping at the first empty slot
// or at 10.
int command_count_0071d780(const DirectorQueue& q) noexcept;

// 0071DEE0 __thiscall(director, uint i), RET 4: slot i holds an order that
// is neither stop, cruise, Leave nor disband; a moveonpath counts only while
// its path has a non-empty point list.
bool slot_has_active_order_0071dee0(const DirectorQueue& q, unsigned index) noexcept;

// 0071DF70 __fastcall(director): no new target while the override delay +40h
// is above 0.0f (00D7A218), or while any filled slot is category 1 or 2.
bool accepts_new_target_0071df70(const DirectorQueue& q, float override_delay_40) noexcept;

// 0071DC80 __fastcall(director): does adding a user-path point need a new
// moveonpath command? True for an empty queue or when the last filled slot is
// not moveonpath; for a moveonpath with an entity target, true only when the
// resolved entity's kind (its +170h object's first virtual) is 12h, and false
// when the entity cannot be resolved. `resolved_kind` is nullopt for an
// unresolvable entity.
bool last_command_needs_new_path_0071dc80(const DirectorQueue& q,
                                         std::optional<int> resolved_kind) noexcept;

// 0071D6D0 cdecl(class, const CommandTarget*): may this command take this
// target? Inputs are the facts the image resolves.
struct AcceptsTargetFacts {
    bool resolves = false;            // 00521EA0 non-null
    bool target_marked_5d = false;    // resolved +5Dh
    bool torpedo_target_ok = false;   // 009229F0 on the resolved target
    bool has_path_interface = false;  // BSP_Entity_PathInterfaceForKind
};
bool command_accepts_target_0071d6d0(const CommandSlot& command, const CommandTarget& target,
                                     const AcceptsTargetFacts& f) noexcept;

// 0071E200 __thiscall(target a, target b), RET 4: same resolved object and the
// two points (zero when has_point is clear) within 1.0 (squared sum z, x, y).
bool slot_target_matches_0071e200(const CommandTarget& a, const CommandTarget& b,
                                  bool same_resolved_object) noexcept;

// Director byte/dword fields written by the small setters.
struct DirectorFlags {
    bool flag_3c = false;                         // 0071D5D0
    std::uint32_t flags_1cc = 0;                  // 0071C0A0, message sub-kind 2
    std::array<bool, 4> enables_220{};            // 00720DE0, sub-kinds 3..6
    std::array<bool, 3> avoidance_240{};          // 00720E50; +242h read by 00720E40
    bool snapshot_243 = false;                    // 007219C0
    float snapshot_244 = 0.0f;
    float snapshot_248 = 0.0f;
};
void store_flag_3c_0071d5d0(DirectorFlags& d, bool v) noexcept;
void set_flags_1cc_0071c0a0(DirectorFlags& d, std::uint32_t v) noexcept;
void set_enables_220_00720de0(DirectorFlags& d, bool a, bool b, bool c, bool e) noexcept;
void set_avoidance_240_00720e50(DirectorFlags& d, bool a, bool b, bool c) noexcept;
bool land_collision_avoidance_00720e40(const DirectorFlags& d) noexcept;

// 0071C060 __fastcall(uint mask, Unit* unit): unit->vtable[114h]() is the
// director; no director answers true, else flags_1cc & mask.
bool director_flags_allow_0071c060(bool has_director, std::uint32_t flags_1cc,
                                   std::uint32_t mask) noexcept;

// 0071C1E0 __thiscall(director, msg*), RET 4: msg+20h sub-kind, msg+24h value.
// 0 -> vtable[64h](value != 0), 1 -> vtable[68h](value != 0), 2 -> +1CCh = value,
// 3..6 -> +220h..+223h = value != 0. Other sub-kinds do nothing.
enum class DirectorMessageAction { kNone, kVirtual64, kVirtual68, kStored };
struct DirectorMessageResult {
    DirectorMessageAction action = DirectorMessageAction::kNone;
    bool virtual_argument = false;
};
DirectorMessageResult apply_command_message_0071c1e0(DirectorFlags& d, std::int32_t sub_kind,
                                                     std::int32_t value) noexcept;

// The records the senders route through 004F1AB0-style BSP_Session_RouteMessage
// (route 7, flags 0). Only the payload fields the receivers read are modelled.
struct RoutedDirectorMessage {
    std::uint8_t type = 0;       // 5Ah, 5Dh, 5Fh, 60h
    std::uint32_t field_20 = 0;  // sub-kind / terminal byte / path index
    std::uint32_t field_24 = 0;  // value / slot
};
// 0071DAD0/0071DD30/0071E050/0071E150: type 5Ah, sub-kind 1/2/4/6 with a value
// (a byte for 1, 4 and 6; a dword for 2).
RoutedDirectorMessage director_sub_kind_message(std::uint32_t sub_kind, std::uint32_t value) noexcept;
// 0071D900: MT_GAMEUNIT_CLEARCMD (5Dh) for slot `slot`, +20h = 1.
RoutedDirectorMessage clear_command_slot_message_0071d900(std::uint32_t slot) noexcept;
// 0071C730: the CLEARCMD constructor with its two arguments.
RoutedDirectorMessage clearcmd_construct_0071c730(std::uint8_t field_20, std::uint32_t field_24) noexcept;
// 0071CDD0: message 60h (session message 96) carrying a path index.
RoutedDirectorMessage set_cur_path_index_message_0071cdd0(std::uint32_t index) noexcept;
// 0071D3E0: message 5Fh (session message 95) with a zero point and +2Ch = 0.
RoutedDirectorMessage clear_user_path_point_message_0071d3e0() noexcept;
// 0071D880: CLEARCMD with +20h = 1 and slot +24h = FFFFFFFFh (all slots).
RoutedDirectorMessage clear_commands_message_0071d880() noexcept;
// 0071D340 cdecl(const Vec3*): message 5Fh carrying the point at +20h..+28h
// and +2Ch = 1 (0071D3E0 sends the same type with a zero point and +2Ch = 0).
struct UserPathPointMessage {
    std::uint8_t type = 0x5F;
    Vec3f point{};
    std::uint32_t add_2c = 0;
};
UserPathPointMessage add_user_path_point_message_0071d340(const Vec3f& point) noexcept;

// 0071D810 / 0071D9E0 __thiscall(director, int stage), RET 4: raise the
// primary (+48h) or override (+50h) stage; never lowers. Reaching 2 routes a
// CLEARCMD (0071C730) with +20h = 1 (primary) or 0 (override), +24h = 0,
// unless the session mode [GGame+1FE4h] is 2. Returns the message when sent.
std::optional<RoutedDirectorMessage> raise_command_stage_0071d810(std::int32_t& stage_48, std::int32_t stage,
                                                                  std::int32_t session_mode) noexcept;
std::optional<RoutedDirectorMessage> raise_override_stage_0071d9e0(std::int32_t& stage_50, std::int32_t stage,
                                                                   std::int32_t session_mode) noexcept;

// 0071D2A0 __fastcall(follower): attached (0071D270's test) -> vtable[14h](),
// else -1. The virtual's answer is supplied.
int path_follower_remaining_or_minus_one_0071d2a0(bool attached, int vt14_answer) noexcept;

// 0071D980 __thiscall(director, cls, target), RET 8: a SetCommand message is
// built and routed only while the override delay +40h is below zero.
bool send_override_command_0071d980(float override_delay_40) noexcept;

// 0071E2E0 __fastcall(director): when the override stage +50h is below 2 it is
// set to 2, and unless the session mode ([GGame+1FE4h]) is 2 a CLEARCMD (0, 0)
// is routed. Returns whether the message is sent.
bool echo_clear_command_0071e2e0(std::int32_t& override_stage_50, std::int32_t session_mode) noexcept;

// 0071DDB0 __thiscall(director+1Ch, Entity* removed), RET 4: what the
// director does when an entity it targets is removed.
struct EntityRemovedInputs {
    bool removed_present = false;
    std::int32_t game_state_5d4 = 0;   // [GGame+5D4h], must exceed 0Bh
    std::int32_t session_mode = 0;     // [GGame+1FE4h]
    bool override_targets_removed = false;   // the +188h target resolves to it
    bool override_vt70 = false;              // vtable[70h](+18Ch target, 0)
    std::array<bool, kCommandSlots> slot_targets_removed{};
    std::array<bool, kCommandSlots> slot_vt70{};   // vtable[70h](slot target, 1)
    std::array<bool, kCommandSlots> slot_vt74{};   // vtable[74h](class, i)
};
struct EntityRemovedActions {
    bool raise_override_stage_2 = false;   // 0071D9E0(2)
    bool raise_command_stage_2 = false;    // 0071D810(2), slot 0
    std::vector<int> clear_slots;          // 0071D900(i), i > 0
};
EntityRemovedActions on_entity_removed_0071ddb0(const DirectorQueue& q,
                                                const EntityRemovedInputs& in) noexcept;

// 0071EDD0 __thiscall(director, CommandTarget*), RET 4. A squadron (kind 0Fh)
// target becomes its last live member; anything else becomes a fixed point at
// the resolved entity's pose. Returns the image's answer (false after a member
// substitution).
struct RetargetMember {
    bool marked_5d = false;
    Vec3f position{};
    std::uint32_t entity = 0;
    std::uint16_t entity_index_174 = 0;
};
struct RetargetInputs {
    bool resolved_present = false;
    bool resolved_is_kind_0f = false;
    bool has_squadron_block = false;           // resolved+9D4h != 0
    std::vector<RetargetMember> members;       // block +3D0h.., count +3CCh
    Vec3f director_unit_position{};            // director+34h unit pose
    Vec3f resolved_position{};                 // resolved+FCh..104h
};
struct RetargetResult {
    bool answer = true;
    bool image_faults = false;   // a null resolved target or a member index >= 5
    bool member_chosen = false;
};
RetargetResult retarget_to_member_or_point_0071edd0(CommandTarget& target,
                                                    const RetargetInputs& in) noexcept;

// 0071F290 __thiscall(controller, float dt), RET 4: the per-frame update.
struct ControllerUpdateInputs {
    bool unit_present = false;
    bool unit_alive_5c = false, unit_marked_5d = false, unit_flag_60 = false, unit_flag_5e = false;
    bool slot0_path_present = false;
    float dt = 0.0f;
    bool slot0_filled = false;          // +54h
    bool override_filled = false;       // +188h
    bool acceptance_44 = false;         // +44h byte
    bool override_acceptance_4c = false;// +4Ch byte
    bool vt78_override = true;          // vtable[78h](0)
    bool vt78_primary = true;           // vtable[78h](1)
    std::int32_t session_mode = 0;      // [GGame+1FE4h]
    bool observer_present = false;      // +38h
};
struct ControllerUpdateState {
    float override_delay_40 = 0.0f;
    std::int32_t mode_30 = 0;
};
struct ControllerUpdateActions {
    bool ran = false;
    bool zero_slot0_path_offset = false;   // path+30h..38h = (0,0,0)
    bool raise_override_stage_2 = false;
    bool raise_command_stage_2 = false;
    bool observer_tick = false;            // (+38h)->vtable[4](dt)
    bool step = false;                     // vtable[7Ch]()
};
ControllerUpdateActions controller_update_0071f290(ControllerUpdateState& s,
                                                   const ControllerUpdateInputs& in) noexcept;

// 0071F600 __thiscall(director, char use_queue_head), RET 4, base director
// vtable 00D09EC0 slot 78h: begin the override command (arg 0) or the queue
// head (arg != 0).
struct BeginCommandInputs {
    bool use_queue_head = false;
    // override arm
    bool override_present = false;          // +188h
    bool override_accepts = false;          // 0071D6D0(+188h, +18Ch)
    bool override_target_resolves = false;
    // queue-head arm
    bool head_present = false;              // +54h
    bool head_target_resolves = false;
    bool head_target_kind_1c = false;       // resolved vtable[5Ch](1Ch)
    int head_category = 0;                  // head vtable[0Ch]
    bool head_target_same_endpoint = false; // resolved+54h == unit+54h
    bool head_accepts_after_rewrite = false;// 0071D6D0 on the (possibly rewritten) head
    bool head_target_has_entity_58 = false;
    bool events_enabled_00f8a0c4 = false;
};
enum class MoveOnPathBuild { kNone, kFromSlotPath, kFromEntityPath };
struct BeginCommandResult {
    bool accepted = true;                   // AL (stays 1 when the command is absent)
    bool stored_acceptance = false;         // the byte written to +44h / +4Ch
    bool rewrote_head_to_moveto = false;    // 0071F67B: +54h = 00E08F68
    MoveOnPathBuild path_build = MoveOnPathBuild::kNone;
    // kFromSlotPath starts the cursor with the pair {followMode 1, startMode 5}.
    bool report_start = false;              // 00984300 "start"
    bool report_target = false;             // 00984800
};
BeginCommandResult begin_command_base_0071f600(const BeginCommandInputs& in,
                                               std::uint32_t head_class_after_rewrite) noexcept;

// The controller state that 00720450 copies.
struct DirectorCopyState {
    DirectorQueue queue{};
    bool byte_3c = false;
    bool byte_3d = false;
    std::int32_t mode_30 = 0;
    std::uint32_t field_16c = 0;      // the 1Ch block at +16Ch..+187h
    CommandTarget target_170{};
    CommandSlot override_188{};       // +188h class, +18Ch target
    bool acceptance_44 = false;
    bool override_acceptance_4c = false;
    std::int32_t stage_48 = 0;
    std::int32_t override_stage_50 = 0;
};
// 00720450 __thiscall(dst, const src*) RET 4: copy +3Ch/+3Dh, the +16Ch block
// and mode +30h; clear the override (0071E610); copy the ten slots and their
// paths (0071FD10). For an owner passing vtable[5Ch](6), a copied `land`
// slot becomes `attackmove` on the unit nearest the land target's position
// (0071DB90 over [GGame+19CCh]+16Ch; supplied as `nearest_to`), with the point
// and +14h cleared and has_entity set only when a unit is found. Then the
// override, both acceptance bytes and both stages are zeroed. The observer
// pair bookkeeping (unregister/register) is not modelled.
struct NearestUnitAnswer {
    std::uint32_t entity = 0;
    std::uint16_t entity_index_174 = 0;
};
void copy_state_from_00720450(DirectorCopyState& dst, const DirectorCopyState& src, bool owner_is_kind_6,
                              const std::function<NearestUnitAnswer(const Vec3f&)>& nearest_to,
                              const std::function<Vec3f(const CommandTarget&)>& target_position);

// 0071E4C0 __thiscall(director, out{Vec3, bool valid}) RET 4: the queue head's
// destination. A moveonpath head asks 007B1350 on (+1A4h path)+10h, the
// follow-mode last point; with a null path the image passes ECX = 10h
// (0071E4F2 MOV ECX,10h) - kNullPathCursor. Otherwise a head target with
// neither entity (+58h) nor point (+59h) clears only the valid byte (+0Ch of
// out, the point is left as it was); else BSP_TargetDescriptor_ResolvePosition
// fills the point and valid = 1.
enum class CurrentPathPointSource { kPathCursor, kNullPathCursor, kInvalid, kTargetPosition };
CurrentPathPointSource current_path_point_0071e4c0(const DirectorQueue& q) noexcept;

// 0071C4A0 __fastcall(obj): +10h = 1, +11h = 0.
struct GunBotRetireBytes {
    bool b10 = false;
    bool b11 = true;
};
void gun_bot_retire_0071c4a0(GunBotRetireBytes& b) noexcept;

// 0071D5A0 / 0071C480: x == 0.
bool stance_is_zero_0071d5a0(std::int32_t stance) noexcept;
bool arg_is_zero_0071c480(std::int32_t v) noexcept;

// 0071D600 __fastcall(director): the largest of the twenty floats at
// +1D0h..+21Ch, seeded with -1e10 (00CE4ADC); `<=` keeps the later of ties.
float max_of_twenty_floats_0071d600(const std::array<float, 20>& v) noexcept;

// 0071CE60 __thiscall(listener, Vec3* out, int index), RET 8: the cursor point
// plus the listener's offset (+20h..+28h) when +18h is set.
Vec3f path_listener_point_0071ce60(const Vec3f& cursor_point, bool offset_on_18,
                                   const Vec3f& offset_20) noexcept;

// The oriented rectangle used by the user-path clamp: two axes (+0h,+4h) and
// (+8h,+0Ch), centre (+10h,+14h), half extents +1Ch (second axis) and +20h
// (first axis), all in the x/z plane.
struct OrientedRect {
    float axis_a[2] = {1.0f, 0.0f};   // +0h, +4h
    float axis_b[2] = {0.0f, 1.0f};   // +8h, +0Ch
    float centre[2] = {0.0f, 0.0f};   // +10h, +14h
    float half_b_1c = 0.0f;
    float half_a_20 = 0.0f;
};
// 0071CEA0 __thiscall(rect, x, z), RET 8.
bool oriented_rect_contains_0071cea0(const OrientedRect& r, float x, float z) noexcept;
// 0071CF30 __thiscall(rect, float out[2], x, z, dx, dz), RET 14h. A point
// outside the rectangle is returned unchanged. Inside, it is pushed out along
// (dx, dz) to the boundary and the push is added. The fourth inside test
// compares the b coordinate against -half_a, not -half_b: that is the image's
// test (0071CFD0 FCOMPI ST(3)), kept as is.
std::array<float, 2> oriented_rect_clamp_0071cf30(const OrientedRect& r, float x, float z,
                                                  float dx, float dz) noexcept;

// The path follower (+14h source, +18h cursor, +40h own Vec3 list).
// 0071D270: attached when the source exists and its vtable[24h]() is the
// follower's own list. 0071D2E0: the cursor when attached; else 0 when the own
// list is non-empty; else -1.
bool path_follower_attached_0071d270(bool source_present, bool source_is_own_list) noexcept;
int path_follower_remaining_0071d2e0(bool attached, int cursor_18, int own_point_count) noexcept;

// 0071FB90 __thiscall(path, owner): the director path object's defaults.
struct DirectorPathObject {
    std::uint32_t owner_04 = 0;
    std::int32_t follow_mode_08 = 1;
    std::int32_t start_mode_0c = 5;
    std::uint32_t source_14 = 0;
    std::int32_t cursor_18 = 0;
    std::int32_t field_1c = 1;
    bool flag_20 = true;
    std::uint32_t owner_24 = 0;
    bool flag_28 = false;
    std::int32_t field_2c = -1;
    float field_3c = -1.0f;   // 00D7A260
    std::vector<Vec3f> points_40;
};
DirectorPathObject director_path_object_construct_0071fb90(std::uint32_t owner) noexcept;
// 0071F570 __fastcall(path): with two or more points, erase the LAST one
// (0071F3E0(count - 1)). Fewer points are left alone.
void path_drop_last_point_0071f570(DirectorPathObject& p) noexcept;
// 0071F5D0 __fastcall(director): the same last-filled-slot index as 0071C1B0
// (director+1A0h+first_empty*4), then a tail JMP to 0071F570 on that path.
// Returns the slot whose path loses its last point (-1: the image uses +1A0h).
int clear_user_path_points_slot_0071f5d0(const DirectorQueue& q) noexcept;
// 0071EA00 __fastcall(entity): the entity when non-null and vtable[5Ch](0Fh)
// holds, else null.
bool entity_as_kind_0f_0071ea00(bool present, bool is_kind_0f) noexcept;
// 0071EDA0 __fastcall(director): with mode +30h != 0 the active descriptor is
// resolved (0071EB60, 00521EA0) and passed through the recon filter
// BSP_Recon_FilterUndetectedUnit; mode 0 answers null without resolving.
bool active_target_object_resolves_0071eda0(std::int32_t mode_30) noexcept;
// 0071EB00 __thiscall(pair*, writer*): writes "followMode" then "startMode".
struct NamedValue {
    const char* name;
    std::int32_t value;
};
std::array<NamedValue, 2> path_follow_mode_write_0071eb00(std::int32_t follow_mode,
                                                          std::int32_t start_mode) noexcept;

// 007207C0 __thiscall(director, point): which AddPoint call (0071FDE0) a
// user-path point makes. needs_new_path is 0071DC80; can_push_moveonpath is
// vtable[34h](00E08F80). On the replace arm `q` is the queue after that push:
// the image counts the first empty slot again after vtable[34h] returns.
struct AddUserPathPlan {
    bool add = false;
    bool replace = false;
    int slot = -1;
};
AddUserPathPlan apply_add_user_path_point_007207c0(const DirectorQueue& q, bool needs_new_path,
                                                   bool can_push_moveonpath) noexcept;

// 0071FDE0 __thiscall(path, const Vec3* point, char replace, uint slot) RET 0Ch:
// add a user-path point. The world-bounds helpers are supplied:
// - clip_points: 007B1EC0 BSP_Point3Vector_ClipToWorldBounds on the list;
// - outside_map: 0071C4F0 BSP_Game_PointOutsideMapBounds on the point;
// - clip_segment: 004BBDD0 BSP_Game_ClipSegmentToWorldBounds(from, to, out),
//   answered as the two points the image then pushes (its `to` copy, then
//   `out`); the argument roles of 004BBDD0 are not settled here;
// - prune: BSP_Point3Vector_PruneFromIndex(cursor) on the list.
struct AddPointInputs {
    bool replace = false;
    int cursor_18 = 0;                 // +18h of path(slot), or of path(0) when slot > 9 / null
    std::optional<Vec3f> owner_position; // (+4h)->+34h unit pose, when there is one
    std::int32_t session_mode = 0;     // [GGame+1FE4h]
};
struct AddPointResult {
    bool rejected = false;             // more than 7 points past the cursor
    bool issued_moveonpath = false;    // replace: owner vtable[60h](00E08F80, empty target)
    bool sent_message_57 = false;      // unit vtable[13Ch](2,0), 0077C470(2,0), message 57h
};
AddPointResult director_path_add_point_0071fde0(
    std::vector<Vec3f>& points, const Vec3f& point, const AddPointInputs& in,
    const std::function<void(std::vector<Vec3f>&)>& clip_points,
    const std::function<bool(const Vec3f&)>& outside_map,
    const std::function<std::array<Vec3f, 2>(const Vec3f& from, const Vec3f& to)>& clip_segment,
    const std::function<void(std::vector<Vec3f>&, int)>& prune);

// 0071DB90 __fastcall(float pos[3]): the nearest unit of [GGame+19CCh]+16Ch's
// list in x/z; the first unit is always taken, later ones only when strictly
// nearer. Returns the index into `units`, or -1 for an empty list.
int nearest_unit_index_0071db90(const Vec3f& pos, const std::vector<Vec3f>& units) noexcept;

// The entity handle tables at 00F89A0C..00F89AA8: index < [00F89A10] reads the
// first table at (index - [00F89A0C]), else the second at (index - [00F89A60]),
// each 10h-byte entry's +0Ch dword, which both functions return in EAX.
// 00720FA0 does this for the word at msg+20h; 00720FE0 answers null for a zero
// word first. Native __fastcall(ECX=msg), plain RET; `index_20` is word
// [msg+20h], and `tables` aliases the live globals (signed split compare,
// unchecked entry read, as in the image).
void* entity_from_message_00720fa0(std::uint16_t index_20, const ObjectHandleTables& tables) noexcept;
void* entity_from_message_nonzero_00720fe0(std::uint16_t index_20, const ObjectHandleTables& tables) noexcept;

// The wire descriptor of one command (0071Exxx messages): the class ordinal
// (class+4h), the entity's network index and an optional point.
struct CommandTargetDescriptor {
    std::uint32_t class_ordinal_00 = 0;
    bool has_entity_04 = false;
    std::uint16_t entity_index_06 = 0;
    bool has_point_08 = false;
    Vec3f point_0c{};
};
// 00721070: ordinal [00E08F8C] (zero in the image), both flags clear.
CommandTargetDescriptor descriptor_init_00721070(std::uint32_t ordinal_00e08f8c) noexcept;
// 00721090 __thiscall(desc, class, target), RET 8. `class_ordinal` is class+4h
// (0 for a null class); `resolved_index_174` is the resolved entity's +174h.
CommandTargetDescriptor descriptor_build_00721090(std::optional<std::uint32_t> class_ordinal,
                                                  const CommandTarget& target,
                                                  std::uint16_t resolved_index_174) noexcept;

// The bit cursor operations the descriptor and snapshot messages use, named
// by the native helpers. A host binds them to its cursor.
struct BitSink {
    virtual ~BitSink() = default;
    virtual void write_u32_bits_00429070(std::uint32_t value, std::uint32_t bits) = 0;
    virtual void write_bool_004290b0(bool value) = 0;
    virtual void write_word_bits_00429120(std::uint16_t value, std::uint32_t bits) = 0;
    virtual void write_numeric_float_004295c0(float value, std::uint32_t zero_flag,
                                              std::uint32_t signed_flag, float scale,
                                              std::uint32_t bits) = 0;
};
struct BitSource {
    virtual ~BitSource() = default;
    virtual std::uint32_t read_u32_bits_00428d10(std::uint32_t bits) = 0;
    virtual bool read_bool_00428d70() = 0;
    virtual std::uint16_t read_word_bits_00428e30(std::uint32_t bits) = 0;
    virtual float read_numeric_float_004293f0(std::uint32_t zero_flag, std::uint32_t signed_flag,
                                              float scale, std::uint32_t bits) = 0;
};
// 00721100 / 007211B0: ordinal 5 bits; has_entity; index 13 bits; has_point;
// three raw floats (zero 0, signed 1, scale FLT_MAX 00D7A248, 32 bits).
void descriptor_write_00721100(const CommandTargetDescriptor& d, BitSink& out);
void descriptor_read_007211b0(CommandTargetDescriptor& d, BitSource& in);

// The controller state message (00721280): +4h/+5h the +3Ch/+3Dh bytes, +8h
// flags_1cc, +0Ch mode (+30h), +10h the primary descriptor (+54h slot), +28h
// the override descriptor (+188h).
struct ControllerStateMessage {
    bool byte_3c = false;
    bool byte_3d = false;
    std::uint32_t flags_1cc = 0;
    std::uint32_t mode_30 = 0;
    CommandTargetDescriptor primary{};
    CommandTargetDescriptor override_{};
};
// 007212E0 (007213B0 is a JMP thunk to it) / 00721340 (thunk 007213C0): flags 9 bits,
// mode 2 bits, then the primary descriptor when mode != 0 and the override
// descriptor too when mode != 1.
// 00721280 __thiscall(msg, director) RET 4: vtable 00CFDB90; the two
// descriptors are 00721090 on (+54h class, +58h target) and (+188h, +18Ch),
// built by the caller and passed in.
ControllerStateMessage state_message_build_00721280(bool byte_3c, bool byte_3d,
                                                    std::uint32_t flags_1cc, std::uint32_t mode_30,
                                                    const CommandTargetDescriptor& primary,
                                                    const CommandTargetDescriptor& override_) noexcept;
void state_body_write_007212e0(const ControllerStateMessage& m, BitSink& out);
void state_body_read_00721340(ControllerStateMessage& m, BitSource& in);

// The snapshot extension (007214C0 builds it; vtable 00CFDBA8): +40h..+43h the
// +220h enables, +44h..+46h the +240h avoidance bytes, +47h 008356C0(),
// +48h 008356B0(), +4Ch 008356F0().
struct ControllerSnapshotMessage {
    ControllerStateMessage state{};
    std::array<bool, 4> enables_40{};
    std::array<bool, 3> avoidance_44{};
    bool flag_47 = false;
    float value_48 = 0.0f;
    float value_4c = 0.0f;
};
// 007214C0 __thiscall(msg, director) RET 4: 00721280, then the extension from
// the director flags and the three getters' answers (008356C0 byte,
// 008356B0 float, 008356F0 float; the x87 returns are stored as floats).
ControllerSnapshotMessage snapshot_build_007214c0(const ControllerStateMessage& state,
                                                  const DirectorFlags& d, bool getter_008356c0,
                                                  float getter_008356b0, float getter_008356f0) noexcept;
// 00721550 / 00721610: the body, eight bools, then +48h as numeric (zero 1,
// signed 1, scale pi (00D7A264) when +47h else 1.0, 10 bits) and +4Ch (zero 1,
// signed 1, scale 1.0, 10 bits).
void snapshot_write_00721550(const ControllerSnapshotMessage& m, BitSink& out);
void snapshot_read_00721610(ControllerSnapshotMessage& m, BitSource& in);
// 007219C0: copy the snapshot's extension back into the director (after the
// state apply); 00721980 copies only the four enables.
void snapshot_apply_extension_007219c0(const ControllerSnapshotMessage& m, DirectorFlags& d) noexcept;
void snapshot_apply_enables_00721980(const ControllerSnapshotMessage& m, DirectorFlags& d) noexcept;

// 00721890 __thiscall(msg, director): the order of what the state apply does.
struct StateApplyPlan {
    bool clear_all_slots = true;                 // 00720CA0, always
    bool issue_primary = false;                  // vtable[60h](class, target)
    std::optional<std::uint32_t> primary_ordinal;
    bool set_override = false;                   // 0071E7F0(class, target)
    std::optional<std::uint32_t> override_ordinal;
    bool byte_3c = false;                        // vtable[64h]
    bool byte_3d = false;                        // vtable[68h]
    std::uint32_t flags_1cc = 0;                 // 0071C0A0
};
StateApplyPlan state_apply_00721890(const ControllerStateMessage& m) noexcept;

// 007216D0 / the ordinal lookups in 00721890: the class whose +4h ordinal
// matches, walking [00E19A70]'s list; none, or ordinal FFh at 007216D0, is null.
std::optional<std::size_t> command_from_ordinal_007216d0(std::uint8_t ordinal,
                                                         const std::vector<std::uint32_t>& class_ordinals) noexcept;

// 00721760 __thiscall(out, entity, const Vec3*, param), RET 0Ch: a point
// target that also names an entity (index from entity+174h).
CommandTarget command_record_build_00721760(std::uint32_t entity, std::uint16_t entity_index_174,
                                            const Vec3f& point, std::uint32_t param) noexcept;
// 007217C0 __thiscall(desc, out), RET 4. `entity` is the handle table's answer
// for the descriptor's index; an entity without a point goes through 00465080
// (entity, 0.0), which the caller supplies.
CommandTarget descriptor_to_target_007217c0(
    const CommandTargetDescriptor& d, std::uint32_t entity, std::uint16_t entity_index_174,
    const std::function<CommandTarget(std::uint32_t)>& from_entity_00465080);

// 00721ED0 __fastcall(vec): size of a 6-byte-element vector, 0 when unallocated.
int vector_size6_00721ed0(std::uint32_t begin, std::uint32_t end) noexcept;

// 00722CA0 __thiscall(parts, const float m[12]), RET 4: a debug draw. It skips
// boxes whose max x (+24h list) is below their min x (+20h list), takes the
// first other one and draws it through the affine matrix as an AABB: the
// centre goes through BSP_Vector3f_TransformAffinePoint (supplied), the half
// extents through |m|: ext.x = hx|m0| + hy|m4| + hz|m8|, ext.y with m1/m5/m9,
// ext.z with m2/m6/m10. Count is +28h.
struct Aabb {
    Vec3f lo{};
    Vec3f hi{};
};
std::optional<Aabb> segment_walk_box_00722ca0(const std::vector<Vec3f>& a_20,
                                              const std::vector<Vec3f>& b_24,
                                              const std::array<float, 12>& m,
                                              const std::function<Vec3f(const Vec3f&)>& transform_point) noexcept;

}  // namespace gameplay_seg38_cc13
}  // namespace bsp
