// cc15_seg48: the live, unbuilt game bodies of segment 48 (packet cc15-r48a, docs/CC15_SEG48.md).
// Names are hypotheses; see the header for the hooks and the x87 model. Each body was transcribed
// from its export listing (bsp.py show --asm) or, where Ghidra's listing is short, from the disk
// bytes (bsp.py disasm-raw). Reconstructed and build-tested only; nothing is wired.

#include "bsp/cc15_seg48.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg48_at<T>(base, offset);
}

template <class T>
T rd(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof(T));
    return value;
}

void* ptr(const void* base, std::size_t offset) {
    return rd<void*>(base, offset);
}

std::uint8_t* bytes(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

// The squadron's member array: count at +3CCh, pointers from +3D0h.
std::int32_t member_count(const void* squadron) {
    return rd<std::int32_t>(squadron, 0x3CC);
}

void* member(const void* squadron, std::int32_t index) {
    return ptr(squadron, 0x3D0 + static_cast<std::size_t>(index) * 4);
}

void* game_pointer(Cc15Seg48Host& host) {
    return *reinterpret_cast<void**>(host.image_global(0x00E188A8));
}

// The zero vector the image reads at 00F87574 (three floats).
const float* zero_vector(Cc15Seg48Host& host) {
    return reinterpret_cast<const float*>(host.image_global(0x00F87574));
}

// FLD float / FSTP float: exact for every value except a signalling NaN, which the
// x87 load quiets (sets mantissa bit 22).
float x87_float_round_trip(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0) bits |= 0x00400000u;
    std::memcpy(&value, &bits, sizeof(bits));
    return value;
}

std::uint32_t float_bits(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

std::uint32_t pointer_value(const void* p) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}

// The native {size, data} string 0041E870 assigns, and its release: the block goes
// back to the sized pool (00419CC0, 00BD1510(data, size + 1, 1)) when data is non-null.
struct NativeString {
    std::uint32_t size;
    char* data;
};

void release_native_string(NativeString& string, Cc15Seg48Host& host) {
    if (string.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), string.data, string.size + 1, 1);
    }
}

// The home-base observer link both 007F1C00 (007F1C08-007F1C2F) and 007F1CA0
// (007F1CC0-007F1CEA) inline: the link is the member at squadron+3F0h, whose +14h is the
// observed base (so squadron+404h). An unchanged base does nothing; otherwise the old base
// drops the pair (006952A0, ECX = old, EDX = link), the field takes the new base, and a
// non-null new base registers the pair (00694A60, ECX = base, EDX = link).
void set_home_link(void* squadron, void* base, Cc15Seg48Host& host) {
    void* const link = bytes(squadron, 0x3F0);
    void* const old = ptr(squadron, 0x404);
    if (old == base) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, link);
    at<void*>(link, 0x14) = base;
    if (base != nullptr) host.observer_register_pair_00694a60(base, link);
}

// The child walk the heading and matrix setters share (007F2F51-007F2F70, 007F2FA1-007F2FC2):
// bytes +C8h and +10Ch cleared, then 0042ED50 on every node of the list at +48h linked by +44h.
void invalidate_children(void* squadron, Cc15Seg48Host& host) {
    void* child = ptr(squadron, 0x48);
    at<std::uint8_t>(squadron, 0xC8) = 0;
    at<std::uint8_t>(squadron, 0x10C) = 0;
    for (; child != nullptr; child = ptr(child, 0x44)) {
        host.scene_child_invalidate_0042ed50(child);
    }
}

// Head task of a member's task stack (member+DF4h: data +58h, size +5Ch), as 007EE6B0 and
// 007EE720 read it: null when there is no stack, the size is 0 or the first entry is null.
void* head_task(const void* member_unit) {
    const void* stack = ptr(member_unit, 0xDF4);
    if (stack == nullptr || rd<std::uint32_t>(stack, 0x5C) == 0) return nullptr;
    return ptr(ptr(stack, 0x58), 0);
}

}  // namespace

// ---------------------------------------------------------------------------
// 007DF360 BSP_PlaneNeighbours_LineOfSightClear, __thiscall(ECX = neighbours (unit+C50h);
// target entity, const float[3] point), RET 8, AL. Evidence: listing 007DF360-007DF4E6.
// The owner is [[neighbours+4]+28h]; none gives 0. The direction is (owner - point) in x/z
// with y = 0 (007DF3A8-007DF3C4), its length from 0042B2F0. Every neighbour in the pointer
// array {+30h first, +34h count} that is non-null, not the target and answers slot +5Ch(6)
// is tested: with d = (neighbour - point) in x/z, dot = (dx*vx + 0) + dz*vz (the y term is
// FLDZ FMUL ST0 = 0), skipped unless dot > 0 (FCOMI, JBE); proj = dot / length; with r =
// [[neighbour+538h]+A0h], the neighbour blocks (AL = 1) when r*r * 0.22000000476837158
// (double 00D06BA0) > (0 + dx*dx + dz*dz) - proj*proj (FCOMIP, JBE). dot, proj, the
// squared distance and the difference are FSTP float spills; at the main thread's 24-bit
// precision control every product and sum also rounds to float. Uncertainty: the double
// constant product is modelled as float(double(r*r) * k), a double rounding where the x87
// rounds once to 24 bits; the two differ only on exact float halfway cases. The pointer
// array's end is re-read each iteration, as the listing does. Name is a hypothesis: the AL
// = 1 answer means "a neighbour is in the way", the inverse of the name.
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_neighbours_line_of_sight_clear_007df360(void* neighbours, const void* target,
                                                              const float* point, Cc15Seg48Host& host) {
    void* const owner = ptr(ptr(neighbours, 4), 0x28);
    if (owner == nullptr) return false;
    if (rd<std::uint8_t>(owner, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(owner);
    const float own_x = x87_float_round_trip(rd<float>(owner, 0xFC));
    const float own_z = x87_float_round_trip(rd<float>(owner, 0x104));
    float direction[3] = {own_x - point[0], 0.0f, own_z - point[2]};
    const float length = host.vector3_length_float_threshold_0042b2f0(direction);
    constexpr double kRadiusScale = 0.22000000476837158;  // 00D06BA0
    const auto array_end = [&] {
        return static_cast<std::uint8_t*>(ptr(neighbours, 0x30)) +
               static_cast<std::size_t>(rd<std::int32_t>(neighbours, 0x34)) * 4;
    };
    for (std::uint8_t* cursor = static_cast<std::uint8_t*>(ptr(neighbours, 0x30)); cursor != array_end();
         cursor += 4) {
        void* const other = ptr(cursor, 0);
        if (other == nullptr || other == target) continue;
        if (!host.vcall_5c_has_ordnance_kind(other, 6)) continue;
        if (rd<std::uint8_t>(other, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(other);
        const float dx = rd<float>(other, 0xFC) - point[0];
        const float dz = rd<float>(other, 0x104) - point[2];
        const float zero_square = 0.0f * 0.0f;
        const float dot = (dx * direction[0] + zero_square) + dz * direction[2];
        if (!(dot > 0.0f)) continue;
        const float projection = dot / length;
        const float radius = rd<float>(ptr(other, 0x538), 0xA0);
        const float distance_square = (zero_square + dx * dx) + dz * dz;
        const float perpendicular_square = distance_square - projection * projection;
        const float reach = static_cast<float>(static_cast<double>(radius * radius) * kRadiusScale);
        if (reach > perpendicular_square) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// 007EE150 BSP_PlaneSquadron_MinMember908, __fastcall(ECX = squadron), ST0. Evidence:
// listing 007EE150-007EE2D7 (the loop unrolled by four, then the remainder). It is a
// MAXIMUM from 0.0, not a minimum: each member's +908h goes through a float slot (FLD/FSTP,
// quieting a signalling NaN) and replaces the running value unless running > value (FCOMIP,
// JA), so a NaN member value is taken. The old name is kept until the Ghidra rename.
// ---------------------------------------------------------------------------
float cc15_seg48_plane_squadron_max_member_908_007ee150(const void* squadron) {
    float best = 0.0f;
    const std::int32_t count = member_count(squadron);
    for (std::int32_t i = 0; i < count; ++i) {
        const float value = x87_float_round_trip(rd<float>(member(squadron, i), 0x908));
        if (!(best > value)) best = value;
    }
    return best;
}

// ---------------------------------------------------------------------------
// 007EE2E0 BSP_PlaneSquadron_IsLeavingOnPurpose, __thiscall(ECX = squadron, byte
// check_leader), RET 4, AL. Evidence: listing 007EE2E0-007EE403. cc9's host reads only the
// retreat arm (game_hosts_units.cpp:933); this is the whole body. With the flag, a leader
// ([+3D0h]) in local-player role 1 (00927F30) or with byte +184h set answers 1. Then the
// current command class (slot +174h): retreat 00E08F90 answers 1; moveto 00E08F78 or
// 00E08F68 with a target present (slot +178h, byte +1h) asks 0071C4F0 of the target point
// (slot +178h again; +8h, or the zero vector 00F87574 when absent); 00E08F80 asks 0071C4F0
// of the director's ([+348h]) path cursor's follow-mode last point (0071BFF0(0), 007B1350)
// when its valid byte (+0Ch) is set (the NEG/SBB/AND 00F872D4 select only tests it).
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_squadron_is_leaving_on_purpose_007ee2e0(void* squadron, std::uint8_t check_leader,
                                                              Cc15Seg48Host& host) {
    if (check_leader != 0) {
        if (host.unit_instance_is_local_player_role_00927f30(member(squadron, 0), 1)) return true;
        if (rd<std::uint8_t>(member(squadron, 0), 0x184) != 0) return true;
    }
    const std::uint32_t command_class = host.squadron_vcall_174_command_class(squadron);
    if (command_class == 0x00E08F90u) return true;
    if (command_class == 0x00E08F78u || command_class == 0x00E08F68u) {
        alignas(4) std::uint8_t first[0x18] = {};
        alignas(4) std::uint8_t second[0x18] = {};
        const void* record = host.squadron_vcall_178_command_target(squadron, first);
        if (rd<std::uint8_t>(record, 1) != 0) {
            record = host.squadron_vcall_178_command_target(squadron, second);
            const float* source = rd<std::uint8_t>(record, 1) != 0
                                      ? reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(record) + 8)
                                      : zero_vector(host);
            float target_point[3];
            std::memcpy(target_point, source, sizeof(target_point));  // MOVSS copies
            if (host.game_point_outside_map_bounds_0071c4f0(game_pointer(host), target_point)) return true;
        }
    }
    if (command_class == 0x00E08F80u) {
        alignas(4) std::uint8_t last[0x10] = {};
        void* const cursor = host.weapon_director_command_path_cursor_0071bff0(ptr(squadron, 0x348), 0);
        host.path_cursor_follow_mode_last_point_007b1350(cursor, last);
        if (last[0x0C] != 0 &&
            host.game_point_outside_map_bounds_0071c4f0(game_pointer(host), reinterpret_cast<const float*>(last))) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// 007EE4E0 BSP_PlaneSquadron_DetachPilot, __thiscall(ECX = squadron), RET. Evidence:
// listing 007EE4E0-007EE5B0. Byte +361h = 1 (the retreat exit latch); then, members from
// the last to the first, a UNITEXITED message (type 50h) built in place exactly as 007EDF50
// does (inlined here: 0075B430(50h), word +18h, bytes +1Ah/+1Ch = 0, +4h = 1, vtable
// 00D02CE0, +20h = squadron+54h re-read per member) is routed to the member through
// 0077C2A0(message, channel, 0) with channel 2 when the game ([00E188A8], non-null) is in
// mode 2 (+1FE4h), else 7. The member count is read once. The message is never destroyed
// beyond the EH state reset (it has no destructor call).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_detach_pilot_007ee4e0(void* squadron, Cc15Seg48Host& host) {
    at<std::uint8_t>(squadron, 0x361) = 1;
    void* const game = game_pointer(host);
    const bool mode_two = game != nullptr && rd<std::int32_t>(game, 0x1FE4) == 2;
    const std::int32_t channel = mode_two ? 2 : 7;
    for (std::int32_t i = member_count(squadron) - 1; i >= 0; --i) {
        alignas(4) std::uint8_t message[0x24] = {};
        cc13_seg48_session_message_unitexited_construct_007edf50(message, rd<std::uint32_t>(squadron, 0x54), host);
        host.session_route_message_0077c2a0(member(squadron, i), message, channel, 0);
    }
}

// ---------------------------------------------------------------------------
// 007EE720 BSP_PlaneSquadron_FirstMemberDF4Entry, __thiscall(ECX = squadron), AL.
// Evidence: listing 007EE720-007EE781. 1 when every member's head task (member+DF4h stack)
// answers its slot +20h; a member without a head task is skipped. cc9's host takes this as
// true (game_hosts_units.cpp:17299, labelled). Count re-read each iteration.
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_squadron_first_member_df4_entry_007ee720(void* squadron, Cc15Seg48Host& host) {
    for (std::int32_t i = 0; i < member_count(squadron); ++i) {
        void* const task = head_task(member(squadron, i));
        if (task == nullptr) continue;
        if (!host.task_vcall_20(task)) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// 007EE790 BSP_PlaneSquadron_RecountActiveWingmen, __thiscall(ECX = squadron), RET.
// Evidence: listing 007EE790-007EE7E0. +3BCh = 0, then +1 for every member 1..count-1 whose
// float at +9C4h + 8 * [word 00F876B8] (the buffer index, re-read per member) is >= 0
// (COMISS, JC: a NaN does not count, -0 does). cc9's host recounts at the call instead
// (game_hosts_units.cpp:8858, labelled substitution).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_recount_active_wingmen_007ee790(void* squadron, Cc15Seg48Host& host) {
    at<std::int32_t>(squadron, 0x3BC) = 0;
    for (std::int32_t i = 1; i < member_count(squadron); ++i) {
        const std::uint16_t index = *reinterpret_cast<const std::uint16_t*>(host.image_global(0x00F876B8));
        const float value = rd<float>(member(squadron, i), 0x9C4 + static_cast<std::size_t>(index) * 8);
        if (value >= 0.0f) ++at<std::int32_t>(squadron, 0x3BC);
    }
}

// ---------------------------------------------------------------------------
// 007EE8F0 BSP_Unit_AttackCommandApplies, __thiscall(ECX = squadron; command class,
// target, byte allow, loadout), RET 10h, AL (EAX = 1 or 0 on some arms). Evidence: listing
// 007EE8F0-007EEBFB. Null target or no leader: 0. Side gate: a target answering +5Ch(1Ch)
// fails on the same side (+54h); any other target fails on the same side or side 2. Then
// by class: attackmove 00E08F78 1; levelbomb 00E08F28: with level-bomb ordnance (007ED830)
// allow && leader +5Ch(10h) && target +5Ch(1Ch), else allow && leader +5Ch(10h) && general
// bomb ordnance (007ED7E0) && !target +5Ch(0Eh); divebomb 00E08F20: allow && !leader
// +5Ch(10h) && 007ED7E0 && !target +5Ch(0Eh); rocket 00E08F48: allow && 007ED970 && !target
// +5Ch(0Eh); torpedo 00E08F18: 007ED8D0, then the leader's first slot descriptor (007B9230)
// of kind 10 with +F0h > 0.001 or !(0.001 >= +F4h) (COMISS 00D7A23C) lifts the allow
// requirement, then !00828EC0(target class +538h); depthcharge 00E08F38: target +5Ch(8) &&
// 007EDA10; dogfight 00E08F58: 00922B10(target) && leader byte +C24h && !leader +5Ch(16h)
// && !0047B850(leader); strafe 00E08F50: allow && leader +5Ch(17h) && (!target +5Ch(6) ||
// !00827F70(target class) || 00604A50(leader)); kamikaze 00E08F40: (allow || target
// +5Ch(41h)) && leader byte +C24h && !0047B850(leader); dropkamikaze 00E08F30: allow &&
// 007ED880; anything else 0. The 007ED7E0 family is cc13_seg48's.
// ---------------------------------------------------------------------------
bool cc15_seg48_unit_attack_command_applies_007ee8f0(void* squadron, std::uint32_t command_class, void* target,
                                                     std::uint8_t allow, std::uint32_t loadout,
                                                     Cc15Seg48Host& host) {
    if (target == nullptr || member(squadron, 0) == nullptr) return false;
    const bool target_1c = host.vcall_5c_has_ordnance_kind(target, 0x1C);
    const std::int32_t target_side = rd<std::int32_t>(target, 0x54);
    if (target_1c) {
        if (target_side == rd<std::int32_t>(squadron, 0x54)) return false;
    } else if (rd<std::int32_t>(squadron, 0x54) == target_side || target_side == 2) {
        return false;
    }
    switch (command_class) {
    case 0x00E08F78u:  // attackmove
        return true;
    case 0x00E08F28u: {  // levelbomb
        const bool level_bomber = host.vcall_5c_has_ordnance_kind(member(squadron, 0), 0x10);
        if (cc13_seg48_unit_has_level_bomb_ordnance_007ed830(squadron, loadout, host)) {
            return allow != 0 && level_bomber && host.vcall_5c_has_ordnance_kind(target, 0x1C);
        }
        return allow != 0 && level_bomber && cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(squadron, loadout, host) &&
               !host.vcall_5c_has_ordnance_kind(target, 0x0E);
    }
    case 0x00E08F20u:  // divebomb
        if (allow == 0) return false;
        if (host.vcall_5c_has_ordnance_kind(member(squadron, 0), 0x10)) return false;
        if (!cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(squadron, loadout, host)) return false;
        return !host.vcall_5c_has_ordnance_kind(target, 0x0E);
    case 0x00E08F48u:  // rocket
        if (allow == 0) return false;
        if (!cc13_seg48_unit_has_rocket_ordnance_007ed970(squadron, loadout, host)) return false;
        return !host.vcall_5c_has_ordnance_kind(target, 0x0E);
    case 0x00E08F18u: {  // torpedo
        if (!cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(squadron, loadout, host)) return false;
        bool needs_allow = true;
        const void* slot = host.weapon_controller_first_slot_descriptor_007b9230(member(squadron, 0));
        if (slot != nullptr && rd<std::int32_t>(slot, 8) == 10) {
            constexpr float kThreshold = 0.001f;  // 00D7A23C
            if (rd<float>(slot, 0xF0) > kThreshold || !(kThreshold >= rd<float>(slot, 0xF4))) needs_allow = false;
        }
        if (allow == 0 && needs_allow) return false;
        return !host.vehicle_class_is_torpedo_proof_small_craft_00828ec0(ptr(target, 0x538));
    }
    case 0x00E08F38u:  // depthcharge
        if (!host.vcall_5c_has_ordnance_kind(target, 8)) return false;
        return cc13_seg48_unit_has_depth_charge_ordnance_007eda10(squadron, loadout, host);
    case 0x00E08F58u:  // dogfight
        if (!host.entity_is_airborne_target_00922b10(target)) return false;
        if (rd<std::uint8_t>(member(squadron, 0), 0xC24) == 0) return false;
        if (host.vcall_5c_has_ordnance_kind(member(squadron, 0), 0x16)) return false;
        return !host.fun_0047b850(member(squadron, 0));
    case 0x00E08F50u:  // strafe
        if (allow == 0) return false;
        if (!host.vcall_5c_has_ordnance_kind(member(squadron, 0), 0x17)) return false;
        if (!host.vcall_5c_has_ordnance_kind(target, 6)) return true;
        if (!host.vehicle_class_is_torpedo_boat_or_small_landing_ship_00827f70(ptr(target, 0x538))) return true;
        return host.fun_00604a50(member(squadron, 0));
    case 0x00E08F40u:  // kamikaze
        if (allow == 0 && !host.vcall_5c_has_ordnance_kind(target, 0x41)) return false;
        if (rd<std::uint8_t>(member(squadron, 0), 0xC24) == 0) return false;
        return !host.fun_0047b850(member(squadron, 0));
    case 0x00E08F30u:  // dropkamikaze
        return allow != 0 && cc13_seg48_unit_has_drop_kamikaze_ordnance_007ed880(squadron, loadout, host);
    default:
        return false;
    }
}

// ---------------------------------------------------------------------------
// 007EEE20 BSP_PlaneSquadron_TargetIsHome, __fastcall(ECX = squadron), EAX 0/1. Evidence:
// listing 007EEE20-007EEE4B. 1 when the director [+348h] and the home base [+404h] are
// both non-null and 00521EA0(director+58h) resolves to the home base.
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_squadron_target_is_home_007eee20(void* squadron, Cc15Seg48Host& host) {
    void* const director = ptr(squadron, 0x348);
    if (director == nullptr) return false;
    void* const home = ptr(squadron, 0x404);
    if (home == nullptr) return false;
    return host.command_target_resolve_object_00521ea0(bytes(director, 0x58)) == home;
}

// ---------------------------------------------------------------------------
// 007EF490 BSP_SessionMessage_PLANESQUADRON_CREATE_Construct, __thiscall(ECX = message;
// class word, squadron, home base, byte inside), RET 10h, EAX = message. EH frame 00C8F303.
// Evidence: listing 007EF490-007EF57F. 00766FC0(BBh, class word, squadron, 1); vtable
// 00D08740; +110h = 0095B9C0([squadron+35Ch]); word +114h = home base+174h (0 without
// one); byte +116h = inside; a 40h block from 00BF681B, when non-null built by
// 00721280(squadron slot +114h()) and given vtable 00D086F4, at +118h; float +11Ch =
// [squadron+350h]+14h (FLD/FSTP); +120h = squadron+364h; byte +124h = squadron+369h. The
// read-side constructor and the destructor are cc13_seg48's 007EDB80 / 007EDC10.
// ---------------------------------------------------------------------------
void* cc15_seg48_session_message_planesquadron_create_construct_007ef490(void* message, std::uint32_t class_word,
                                                                         void* squadron, const void* home_base,
                                                                         std::uint8_t inside_home_base,
                                                                         Cc15Seg48Host& host) {
    host.net_entity_create_message_construct_00766fc0(message, 0xBB, class_word, squadron, 1);
    at<std::uint32_t>(message, 0) = 0x00D08740u;
    at<std::uint32_t>(message, 0x110) = host.vehicle_class_get_registry_id_0095b9c0(ptr(squadron, 0x35C));
    at<std::uint16_t>(message, 0x114) = home_base != nullptr ? rd<std::uint16_t>(home_base, 0x174) : 0;
    at<std::uint8_t>(message, 0x116) = inside_home_base;
    void* pair = host.operator_new_00bf681b(0x40);
    if (pair != nullptr) {
        host.command_controller_state_build_00721280(pair, host.squadron_vcall_114_controller(squadron));
        at<std::uint32_t>(pair, 0) = 0x00D086F4u;
    }
    at<void*>(message, 0x118) = pair;
    at<float>(message, 0x11C) = x87_float_round_trip(rd<float>(ptr(squadron, 0x350), 0x14));
    at<std::uint32_t>(message, 0x120) = rd<std::uint32_t>(squadron, 0x364);
    at<std::uint8_t>(message, 0x124) = rd<std::uint8_t>(squadron, 0x369);
    return message;
}

// ---------------------------------------------------------------------------
// 007EF590 BSP_SessionMessage_PLANESQUADRON_CREATE_Write, __thiscall(ECX = message,
// cursor), RET 4. Evidence: listing 007EF590-007EF60D. 00765000; +110h unsigned 10 bits;
// word +114h 12 bits (00429120); the +118h record's writer 007213B0(cursor); float +11Ch
// through 004295C0(value, 0, 0, 10.0 (00CE38B8), 16); +120h signed 4 bits; byte +124h.
// ---------------------------------------------------------------------------
void cc15_seg48_session_message_planesquadron_create_write_007ef590(void* message, void* cursor,
                                                                   Cc15Seg48Host& host) {
    host.net_entity_create_message_write_00765000(message, cursor);
    host.bit_cursor_write_unsigned_dword_bits_00429070(cursor, rd<std::uint32_t>(message, 0x110), 10);
    host.bit_cursor_write_word_bits_00429120(cursor, rd<std::uint16_t>(message, 0x114), 12);
    host.command_controller_state_write_007213b0(ptr(message, 0x118), cursor);
    host.bit_cursor_write_numeric_float_004295c0(cursor, x87_float_round_trip(rd<float>(message, 0x11C)), 0, 0,
                                                 10.0f, 16);
    host.bit_cursor_write_signed_dword_bits_00429090(cursor, rd<std::uint32_t>(message, 0x120), 4);
    host.bit_cursor_write_boolean_004290b0(cursor, rd<std::uint8_t>(message, 0x124));
}

// ---------------------------------------------------------------------------
// 007EF610 BSP_SessionMessage_PLANESQUADRON_CREATE_Read, __thiscall(ECX = message,
// stream), RET 4. Evidence: listing 007EF610-007EF68D. 00767E30(stream); the cursor is
// stream+4: +110h 10 bits (00428D10), word +114h 12 bits (00428E30), the +118h record's
// reader 007213C0(stream itself), float +11Ch (004293F0(&, 0, 0, 10.0, 16)), +120h signed 4
// bits (00428D30), byte +124h (00428D70).
// ---------------------------------------------------------------------------
void cc15_seg48_session_message_planesquadron_create_read_007ef610(void* message, void* stream,
                                                                  Cc15Seg48Host& host) {
    host.net_entity_create_message_read_00767e30(message, stream);
    void* const cursor = bytes(stream, 4);
    host.bit_cursor_read_unsigned_dword_00428d10(cursor, &at<std::uint32_t>(message, 0x110), 10);
    host.bit_cursor_read_word_bits_00428e30(cursor, &at<std::uint16_t>(message, 0x114), 12);
    host.command_controller_state_read_007213c0(ptr(message, 0x118), stream);
    host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x11C), 0, 0, 10.0f, 16);
    host.bit_cursor_read_signed_dword_00428d30(cursor, &at<std::uint32_t>(message, 0x120), 4);
    host.bit_cursor_read_boolean_00428d70(cursor, &at<std::uint8_t>(message, 0x124));
}

// ---------------------------------------------------------------------------
// 007EF690 / 007EF6C0 BSP_SessionMessage_PLANESQUADRON_ATTACKRUN_Write / _Read,
// __thiscall(ECX = message, cursor or stream), RET 4. Evidence: listings 007EF690-007EF6B8
// and 007EF6C0-007EF6E9. The extended header (0075B480 / 0075B4C0), then bytes +1Ch and +20h
// as booleans (the reader's cursor is stream+4).
// ---------------------------------------------------------------------------
void cc15_seg48_session_message_planesquadron_attackrun_write_007ef690(void* message, void* cursor,
                                                                      Cc15Seg48Host& host) {
    host.session_message_write_extended_header_0075b480(message, cursor);
    host.bit_cursor_write_boolean_004290b0(cursor, rd<std::uint8_t>(message, 0x1C));
    host.bit_cursor_write_boolean_004290b0(cursor, rd<std::uint8_t>(message, 0x20));
}

void cc15_seg48_session_message_planesquadron_attackrun_read_007ef6c0(void* message, void* stream,
                                                                     Cc15Seg48Host& host) {
    host.session_message_read_extended_header_0075b4c0(message, stream);
    void* const cursor = bytes(stream, 4);
    host.bit_cursor_read_boolean_00428d70(cursor, &at<std::uint8_t>(message, 0x1C));
    host.bit_cursor_read_boolean_00428d70(cursor, &at<std::uint8_t>(message, 0x20));
}

// ---------------------------------------------------------------------------
// 007F0B30 BSP_PlaneSquadron_SendCreateMessage, __thiscall(ECX = squadron, peer), RET 4.
// EH frame 00C8F37B. Evidence: listing 007F0B30-007F0BE2. Nothing without a leader. A
// 128h-byte CREATE message on the stack, 007EF490([[leader+538h]+70h], squadron, +404h,
// byte +408h); 00779FC0(squadron, peer, message); then the destructor 007EDC10 inlined
// (vtable 00D08740, 00BF65AC of +118h, 007668E0).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_send_create_message_007f0b30(void* squadron, std::uint32_t peer,
                                                           Cc15Seg48Host& host) {
    void* const leader = member(squadron, 0);
    if (leader == nullptr) return;
    alignas(4) std::uint8_t message[0x128] = {};
    cc15_seg48_session_message_planesquadron_create_construct_007ef490(
        message, rd<std::uint32_t>(ptr(leader, 0x538), 0x70), squadron, ptr(squadron, 0x404),
        rd<std::uint8_t>(squadron, 0x408), host);
    host.entity_send_message_to_peer_00779fc0(squadron, peer, message);
    cc13_seg48_session_message_planesquadron_create_destruct_007edc10(message, host);
}

// ---------------------------------------------------------------------------
// 007EF8B0 BSP_CommandRecord_InitWithTarget, __thiscall(ECX = record; command class, const
// target descriptor*), RET 8, EAX = record. Evidence: listing 007EF8B0-007EF8E9. +0h =
// class; words +4h/+6h, dword +8h and floats +Ch..+18h copied from the descriptor's
// +0h/+2h/+4h/+8h..+14h (the floats through FLD/FSTP).
// ---------------------------------------------------------------------------
void* cc15_seg48_command_record_init_with_target_007ef8b0(void* record, std::uint32_t command_class,
                                                         const void* target) {
    at<std::uint32_t>(record, 0) = command_class;
    at<std::uint16_t>(record, 4) = rd<std::uint16_t>(target, 0);
    at<std::uint16_t>(record, 6) = rd<std::uint16_t>(target, 2);
    at<std::uint32_t>(record, 8) = rd<std::uint32_t>(target, 4);
    for (std::size_t i = 0; i < 4; ++i) {
        at<float>(record, 0x0C + i * 4) = x87_float_round_trip(rd<float>(target, 8 + i * 4));
    }
    return record;
}

// ---------------------------------------------------------------------------
// 007F1000 BSP_CommandRecord_InitWithPoint, __thiscall(ECX = record; command class,
// entity or null), RET 8, EAX = record. Evidence: listing 007F1000-007F1069. +0h = class;
// +Ch..+14h = the zero vector 00F87574 (MOVSS); word +4h = 0; +8h = entity; without an
// entity byte +4h = 0 and word +6h = 0, with one byte +4h = 1 and word +6h = entity+174h;
// float +18h = 0 either way. It records an entity target, despite the name.
// ---------------------------------------------------------------------------
void* cc15_seg48_command_record_init_with_entity_007f1000(void* record, std::uint32_t command_class,
                                                         const void* entity, Cc15Seg48Host& host) {
    at<std::uint32_t>(record, 0) = command_class;
    std::memcpy(bytes(record, 0x0C), zero_vector(host), 12);
    at<std::uint16_t>(record, 4) = 0;
    at<std::uint32_t>(record, 8) = pointer_value(entity);
    if (entity == nullptr) {
        at<std::uint8_t>(record, 4) = 0;
        at<std::uint16_t>(record, 6) = 0;
    } else {
        at<std::uint8_t>(record, 4) = 1;
        at<std::uint16_t>(record, 6) = rd<std::uint16_t>(entity, 0x174);
    }
    at<float>(record, 0x18) = 0.0f;
    return record;
}

// ---------------------------------------------------------------------------
// 007EF9D0 BSP_PlaneSquadron_Destruct, __thiscall(ECX = squadron), RET. EH frame 00C8F341.
// Evidence: listing 007EF9D0-007EFA62. The six vtables (00D087C0 +0h, 00D087A8 +10h,
// 00D087A0 +24h, 00D0879C +170h, 00D08794 +1E4h, 00D0877C +310h), then 006E0860(+3F0h),
// 00875490(+310h) and 0077E380(squadron). Reached from the scalar deleting destructor
// 007F1140 (a CG_ body, not part of this packet).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_destruct_007ef9d0(void* squadron, Cc15Seg48Host& host) {
    at<std::uint32_t>(squadron, 0) = 0x00D087C0u;
    at<std::uint32_t>(squadron, 0x10) = 0x00D087A8u;
    at<std::uint32_t>(squadron, 0x24) = 0x00D087A0u;
    at<std::uint32_t>(squadron, 0x170) = 0x00D0879Cu;
    at<std::uint32_t>(squadron, 0x1E4) = 0x00D08794u;
    at<std::uint32_t>(squadron, 0x310) = 0x00D0877Cu;
    host.observer_member_destruct_006e0860(bytes(squadron, 0x3F0));
    host.unit_tick_element_destruct_00875490(bytes(squadron, 0x310));
    host.entity_base_destruct_0077e380(squadron);
}

// 007EFA80 BSP_PlaneSquadron_GetSubobject310, __fastcall(ECX = squadron), EAX: squadron+310h
// (the tick element), null for a null squadron. Disk bytes 007EFA80-007EFA8D.
void* cc15_seg48_plane_squadron_get_subobject_310_007efa80(void* squadron) {
    return squadron != nullptr ? bytes(squadron, 0x310) : nullptr;
}

// 007EFA90 BSP_PlaneSquadron_GetField1E4, __thiscall(ECX = object), EAX = [ECX+1E4h]. Disk
// bytes 007EFA90-007EFA96 (8B 81 E4 01 00 00 C3). On the full squadron +1E4h holds the
// vtable 00D08794 (007EFA14), so the slot is called on a secondary base (this-adjusted);
// include/bsp/recon_slot_lists.hpp reads it as squadron+354h through the +170h base.
std::uint32_t cc15_seg48_plane_squadron_get_field_1e4_007efa90(const void* object) {
    return rd<std::uint32_t>(object, 0x1E4);
}

// 007EFAC0 BSP_PlaneSquadron_Slot18C_Noop, __thiscall(ECX = squadron, dword), RET 4. Disk
// bytes C2 04 00.
void cc15_seg48_plane_squadron_slot18c_noop_007efac0(void*, std::uint32_t) {}

// ---------------------------------------------------------------------------
// 007EFB30 BSP_PilotControl_RaiseForceReleaseFlag, __thiscall(ECX = squadron), RET. Disk
// bytes 007EFB30-007EFB54. 007EE7F0(squadron, 0), then byte +378h = 1 when +374h >= +390h
// (FCOMIP, JB skips on less or unordered).
// ---------------------------------------------------------------------------
void cc15_seg48_pilot_control_raise_force_release_flag_007efb30(void* squadron, Cc15Seg48Host& host) {
    host.pilot_control_refresh_armed_fraction_007ee7f0(squadron, 0);
    if (rd<float>(squadron, 0x374) >= rd<float>(squadron, 0x390)) at<std::uint8_t>(squadron, 0x378) = 1;
}

// ---------------------------------------------------------------------------
// 007EFEB0 BSP_PlaneSquadron_Slot168_CommandTargetCheck, __thiscall(ECX = squadron, const
// char* command), RET 4, AL. Evidence: listing 007EFEB0-007EFFA7 and the strings. Byte
// +3B0h set: 0. Then 00438E10(command, s) == 0 against "clearorders" (00CF8AF4) and
// "returntobase" (00CFB5F0): 1; "cleartarget" (00CF8B00): 00521EA0(0071EB60(director
// [+348h])) != 0; "settarget", "moveto", "strafe", "dogfight": 1; "bomb" (00D08950):
// 007ED7E0(squadron, 0); "torpedo": 007ED8D0(squadron, 0); otherwise command == "land".
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_squadron_slot168_command_target_check_007efeb0(void* squadron, const char* command,
                                                                     Cc15Seg48Host& host) {
    if (rd<std::uint8_t>(squadron, 0x3B0) != 0) return false;
    const auto is = [&](const char* name) { return host.cstring_compare_insensitive_00438e10(command, name) == 0; };
    if (is("clearorders")) return true;
    if (is("returntobase")) return true;
    if (is("cleartarget")) {
        void* const descriptor = host.entity_command_active_target_descriptor_0071eb60(ptr(squadron, 0x348));
        return host.command_target_resolve_object_00521ea0(descriptor) != nullptr;
    }
    if (is("settarget") || is("moveto") || is("strafe") || is("dogfight")) return true;
    if (is("bomb")) return cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(squadron, 0, host);
    if (is("torpedo")) return cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(squadron, 0, host);
    return is("land");
}

// ---------------------------------------------------------------------------
// 007EFFB0 BSP_PlaneSquadron_PromoteArmedLeader, __thiscall(ECX = squadron), AL. Evidence:
// listing 007EFFB0-007F002B. Only when some member can drop ordnance (007ED790(0)) and the
// leader cannot (007B9140(0)): the first member 1..count-1 with byte +C25h clear, +C20h <=
// 0 and 007B9140(0) true is handed to 007EEE50 and the answer is 1.
// ---------------------------------------------------------------------------
bool cc15_seg48_plane_squadron_promote_armed_leader_007effb0(void* squadron, Cc15Seg48Host& host) {
    if (!cc13_seg48_plane_squadron_any_can_drop_ordnance_007ed790(squadron, 0, host)) return false;
    if (host.unit_can_drop_ordnance_007b9140(member(squadron, 0), 0)) return false;
    for (std::int32_t i = 1; i < member_count(squadron); ++i) {
        void* const candidate = member(squadron, i);
        if (rd<std::uint8_t>(candidate, 0xC25) != 0) continue;
        if (rd<std::int32_t>(candidate, 0xC20) > 0) continue;
        if (host.unit_can_drop_ordnance_007b9140(candidate, 0)) {
            host.squadron_request_leader_promotion_007eee50(squadron, member(squadron, i));
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// 007F0030 BSP_PilotControl_HandleMessage, __thiscall(ECX = squadron, message), RET 4, AL.
// Evidence: listing 007F0030-007F01E3, the index table 007F0204 (byte per id - 4Bh, 74h
// entries) and the jump table 007F01E8. 4Bh: 00780090(member, message) for every member,
// then 007EFFB0 when byte +20h and +2Ch are both 0; 4Ch: member slot +148h(+20h, +24h);
// 7Fh: slot +2Ch(+1Ch, squadron+58h, &zeroed int) on the squadron then on every member;
// 80h: the same with (squadron+54h, +1Ch); BCh: squadron+370h = byte +20h ? 2 : 0; BEh:
// 007ED610(+1Ch). All handled ids answer 1, every other id 0. cc9's plane_squadron_host
// models only the BEh arm (src/plane_squadron_host.cpp:559).
// ---------------------------------------------------------------------------
bool cc15_seg48_pilot_control_handle_message_007f0030(void* squadron, void* message, Cc15Seg48Host& host) {
    switch (rd<std::uint8_t>(message, 0x10)) {
    case 0x4B:
        for (std::int32_t i = 0; i < member_count(squadron); ++i) {
            host.session_dispatch_entity_message_to_member_00780090(member(squadron, i), message);
        }
        if (rd<std::uint8_t>(message, 0x20) == 0 && rd<std::uint32_t>(message, 0x2C) == 0) {
            cc15_seg48_plane_squadron_promote_armed_leader_007effb0(squadron, host);
        }
        return true;
    case 0x4C:
        for (std::int32_t i = 0; i < member_count(squadron); ++i) {
            host.vcall_148(member(squadron, i), rd<std::uint32_t>(message, 0x20), rd<std::uint32_t>(message, 0x24));
        }
        return true;
    case 0x7F: {
        std::int32_t out = 0;
        host.entity_vcall_2c(squadron, rd<std::uint32_t>(message, 0x1C), rd<std::uint32_t>(squadron, 0x58), &out);
        for (std::int32_t i = 0; i < member_count(squadron); ++i) {
            std::int32_t member_out = 0;
            host.entity_vcall_2c(member(squadron, i), rd<std::uint32_t>(message, 0x1C),
                                 rd<std::uint32_t>(squadron, 0x58), &member_out);
        }
        return true;
    }
    case 0x80: {
        std::int32_t out = 0;
        host.entity_vcall_2c(squadron, rd<std::uint32_t>(squadron, 0x54), rd<std::uint32_t>(message, 0x1C), &out);
        for (std::int32_t i = 0; i < member_count(squadron); ++i) {
            std::int32_t member_out = 0;
            host.entity_vcall_2c(member(squadron, i), rd<std::uint32_t>(squadron, 0x54),
                                 rd<std::uint32_t>(message, 0x1C), &member_out);
        }
        return true;
    }
    case 0xBC:
        at<std::int32_t>(squadron, 0x370) = rd<std::uint8_t>(message, 0x20) != 0 ? 2 : 0;
        return true;
    case 0xBE:
        host.squadron_promote_flight_leader_007ed610(squadron, rd<std::uint32_t>(message, 0x1C));
        return true;
    default:
        return false;
    }
}

// ---------------------------------------------------------------------------
// 007F10E0 BSP_PlaneSquadron_UnregisterFromWorldLists, __thiscall(ECX = squadron), RET.
// Disk bytes 007F10E0-007F1135. 00928570(squadron), then for the lists at [squadron+30h]+30h
// and +138h (head node at list+4h, node {+4h next, +8h value}) the first node holding the
// squadron is erased with 004837D0(list, node). The registering twin 007F10B0 is cc9's
// (game_hosts_units.cpp:4025).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_unregister_from_world_lists_007f10e0(void* squadron, Cc15Seg48Host& host) {
    host.game_entity_unregister_base_00928570(squadron);
    for (const std::size_t offset : {std::size_t{0x30}, std::size_t{0x138}}) {
        void* const list = bytes(ptr(squadron, 0x30), offset);
        void* node = ptr(list, 4);
        while (node != nullptr && ptr(node, 8) != squadron) node = ptr(node, 4);
        if (node != nullptr) host.unit_list_erase_node_004837d0(list, node);
    }
}

// ---------------------------------------------------------------------------
// 007F0BF0 BSP_PilotBotSettings_SaveToArchive, __thiscall(ECX = settings (squadron+37Ch),
// writer*), RET 4. Evidence: listing 007F0BF0-007F0DBA. Slot +4h(0, "PilotBotSettings"),
// then slot +Ch puts: floats (type 2, MOVSS) "maxattackalt" +1Ch, "maxtravelalt" +18h,
// "maxreleasealt" +20h, "travelspeed" +24h; bytes (type 3; the upper 24 bits of the value
// dword are stale stack in the image, zero here) "newtravelalt" +2Dh, "newattackalt" +2Eh,
// "newreleasealt" +2Fh, "newtravelspd" +30h, "enablealtrnd" +31h; slot +8h.
// ---------------------------------------------------------------------------
void cc15_seg48_pilot_bot_settings_save_007f0bf0(void* settings, void* writer, Cc15Seg48Host& host) {
    host.archive_vcall_04(writer, 0, "PilotBotSettings");
    host.archive_vcall_0c_put(writer, 0, "maxattackalt", 2, rd<std::uint32_t>(settings, 0x1C));
    host.archive_vcall_0c_put(writer, 0, "maxtravelalt", 2, rd<std::uint32_t>(settings, 0x18));
    host.archive_vcall_0c_put(writer, 0, "maxreleasealt", 2, rd<std::uint32_t>(settings, 0x20));
    host.archive_vcall_0c_put(writer, 0, "travelspeed", 2, rd<std::uint32_t>(settings, 0x24));
    host.archive_vcall_0c_put(writer, 0, "newtravelalt", 3, rd<std::uint8_t>(settings, 0x2D));
    host.archive_vcall_0c_put(writer, 0, "newattackalt", 3, rd<std::uint8_t>(settings, 0x2E));
    host.archive_vcall_0c_put(writer, 0, "newreleasealt", 3, rd<std::uint8_t>(settings, 0x2F));
    host.archive_vcall_0c_put(writer, 0, "newtravelspd", 3, rd<std::uint8_t>(settings, 0x30));
    host.archive_vcall_0c_put(writer, 0, "enablealtrnd", 3, rd<std::uint8_t>(settings, 0x31));
    host.archive_vcall_08(writer);
}

// ---------------------------------------------------------------------------
// 007F0DC0 BSP_PilotBotSettings_LoadFromArchive, __thiscall(ECX = settings, reader*),
// RET 4. Evidence: the export (slot +10h gets in this order; note newattackalt is read
// before newtravelalt, the reverse of the writer).
// ---------------------------------------------------------------------------
void cc15_seg48_pilot_bot_settings_load_007f0dc0(void* settings, void* reader, Cc15Seg48Host& host) {
    host.archive_vcall_04(reader, 0, "PilotBotSettings");
    host.archive_vcall_10_get(reader, 0, "maxattackalt", 2, bytes(settings, 0x1C));
    host.archive_vcall_10_get(reader, 0, "maxtravelalt", 2, bytes(settings, 0x18));
    host.archive_vcall_10_get(reader, 0, "maxreleasealt", 2, bytes(settings, 0x20));
    host.archive_vcall_10_get(reader, 0, "travelspeed", 2, bytes(settings, 0x24));
    host.archive_vcall_10_get(reader, 0, "newattackalt", 3, bytes(settings, 0x2E));
    host.archive_vcall_10_get(reader, 0, "newtravelalt", 3, bytes(settings, 0x2D));
    host.archive_vcall_10_get(reader, 0, "newreleasealt", 3, bytes(settings, 0x2F));
    host.archive_vcall_10_get(reader, 0, "newtravelspd", 3, bytes(settings, 0x30));
    host.archive_vcall_10_get(reader, 0, "enablealtrnd", 3, bytes(settings, 0x31));
    host.archive_vcall_08(reader);
}

// ---------------------------------------------------------------------------
// 007F1160 BSP_PlaneSquadron_SaveToArchive, __thiscall(ECX = squadron, writer*), RET 4.
// Evidence: listing 007F1160-007F14D0 and the export. 0077C160(writer); slot +4h
// "_planeSquadron" (00D08AD4) and "planes" (00CF8880); every member as an indexed key
// (name word 0 = 1, key word 1 = the index), type 4; slot +8h. Then "attackRun" +370h,
// "numFollowers" +3BCh, "initialSize" +3C8h, "psFormation" +3E4h (type 1), "homeBase"
// +404h (type 4), "collectedDamage" +40Ch, "pCollectedDamage" +410h (type 2),
// "retreatInProgress" byte +361h, "avoidGridTanClimbAngle" [+34Ch]+14h; "gameUnit" section
// holding the director's 0071BDD0(writer); "isInsideHomeBase" byte +408h, "classIndex"
// 0095B9C0([+35Ch]) (type 1), "climbAngleTangent" [+350h]+14h, the PilotBotSettings at
// +37Ch (007F0BF0), "EQper" +374h, "EnableLAutDrop" byte +378h; slot +8h closes
// "_planeSquadron". Float values are MOVSS copies; the type-3 upper bytes are zero here.
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_save_to_archive_007f1160(void* squadron, void* writer, Cc15Seg48Host& host) {
    host.net_entity_write_replication_state_0077c160(squadron, writer);
    host.archive_vcall_04(writer, 0, "_planeSquadron");
    host.archive_vcall_04(writer, 0, "planes");
    for (std::int32_t i = 0; i < member_count(squadron); ++i) {
        const char* const index_key = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(i));
        host.archive_vcall_0c_put(writer, 1, index_key, 4, pointer_value(member(squadron, i)));
    }
    host.archive_vcall_08(writer);
    host.archive_vcall_0c_put(writer, 0, "attackRun", 1, rd<std::uint32_t>(squadron, 0x370));
    host.archive_vcall_0c_put(writer, 0, "numFollowers", 1, rd<std::uint32_t>(squadron, 0x3BC));
    host.archive_vcall_0c_put(writer, 0, "initialSize", 1, rd<std::uint32_t>(squadron, 0x3C8));
    host.archive_vcall_0c_put(writer, 0, "psFormation", 1, rd<std::uint32_t>(squadron, 0x3E4));
    host.archive_vcall_0c_put(writer, 0, "homeBase", 4, rd<std::uint32_t>(squadron, 0x404));
    host.archive_vcall_0c_put(writer, 0, "collectedDamage", 2, rd<std::uint32_t>(squadron, 0x40C));
    host.archive_vcall_0c_put(writer, 0, "pCollectedDamage", 2, rd<std::uint32_t>(squadron, 0x410));
    host.archive_vcall_0c_put(writer, 0, "retreatInProgress", 3, rd<std::uint8_t>(squadron, 0x361));
    host.archive_vcall_0c_put(writer, 0, "avoidGridTanClimbAngle", 2, rd<std::uint32_t>(ptr(squadron, 0x34C), 0x14));
    void* const director = ptr(squadron, 0x348);
    host.archive_vcall_04(writer, 0, "gameUnit");
    host.director_write_archive_0071bdd0(director, writer);
    host.archive_vcall_08(writer);
    host.archive_vcall_0c_put(writer, 0, "isInsideHomeBase", 3, rd<std::uint8_t>(squadron, 0x408));
    host.archive_vcall_0c_put(writer, 0, "classIndex", 1,
                              host.vehicle_class_get_registry_id_0095b9c0(ptr(squadron, 0x35C)));
    host.archive_vcall_0c_put(writer, 0, "climbAngleTangent", 2, rd<std::uint32_t>(ptr(squadron, 0x350), 0x14));
    cc15_seg48_pilot_bot_settings_save_007f0bf0(bytes(squadron, 0x37C), writer, host);
    host.archive_vcall_0c_put(writer, 0, "EQper", 2, rd<std::uint32_t>(squadron, 0x374));
    host.archive_vcall_0c_put(writer, 0, "EnableLAutDrop", 3, rd<std::uint8_t>(squadron, 0x378));
    host.archive_vcall_08(writer);
}

// ---------------------------------------------------------------------------
// 007F2BD0 BSP_PlaneSquadron_InitTimerBlock, __thiscall(ECX = block), EAX = block. Disk
// bytes 007F2BD0-007F2C55. The block is the PilotBotSettings at squadron+37Ch (the offsets
// match 007F0BF0's keys), so this is its default initialiser, not a timer block: the three
// altitudes +18h/+1Ch/+20h = tuning+53Ch (0042E740, MOVSS), travelspeed +24h = [00D08B28]
// (bits 427471C8h, 61.1111f), bytes +2Dh..+30h = 0, dword +28h = FFh, byte +31h
// (enablealtrnd) = 1, floats +0h..+Ch = -1.0 ([00D7A260]), bytes +10h..+13h = 0, float
// +14h = tuning+5F4h (the tuning getter called again; FLD/FSTP), bytes +32h and +2Ch = 0.
// ---------------------------------------------------------------------------
void* cc15_seg48_pilot_bot_settings_init_007f2bd0(void* settings, Cc15Seg48Host& host) {
    const std::uint32_t altitude = rd<std::uint32_t>(host.game_tuning_get_singleton_0042e740(), 0x53C);
    at<std::uint32_t>(settings, 0x20) = altitude;
    at<std::uint32_t>(settings, 0x18) = altitude;
    at<std::uint32_t>(settings, 0x1C) = altitude;
    at<std::uint32_t>(settings, 0x24) = 0x427471C8u;
    at<std::uint8_t>(settings, 0x2F) = 0;
    at<std::uint8_t>(settings, 0x30) = 0;
    at<std::uint8_t>(settings, 0x2D) = 0;
    at<std::uint8_t>(settings, 0x2E) = 0;
    at<std::uint32_t>(settings, 0x28) = 0xFF;
    at<std::uint8_t>(settings, 0x31) = 1;
    for (const std::size_t offset : {std::size_t{4}, std::size_t{0}, std::size_t{8}, std::size_t{0x0C}}) {
        at<std::uint32_t>(settings, offset) = float_bits(-1.0f);
    }
    at<std::uint8_t>(settings, 0x11) = 0;
    at<std::uint8_t>(settings, 0x10) = 0;
    at<std::uint8_t>(settings, 0x13) = 0;
    at<std::uint8_t>(settings, 0x12) = 0;
    at<float>(settings, 0x14) = x87_float_round_trip(rd<float>(host.game_tuning_get_singleton_0042e740(), 0x5F4));
    at<std::uint8_t>(settings, 0x32) = 0;
    at<std::uint8_t>(settings, 0x2C) = 0;
    return settings;
}

// ---------------------------------------------------------------------------
// 007F1C00 BSP_PlaneSquadron_SetHomeAirBase, __thiscall(ECX = squadron; base, byte
// inside), RET 8. Disk bytes 007F1C00-007F1C93. The observer link (set_home_link); then
// with a base: block = 006BCD20(base, 0); in game mode +1FE4h != 0, 006CC760(block,
// squadron) and 007ED6E0(squadron); in mode 0, 006CC7B0(block, squadron) and, only when
// inside is set, byte +408h = inside and 007ED6E0. Without a base, byte +408h = 0. The
// game pointer is not null-checked (as in the image).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_set_home_air_base_007f1c00(void* squadron, void* base, std::uint8_t inside,
                                                         Cc15Seg48Host& host) {
    set_home_link(squadron, base, host);
    void* const home = ptr(squadron, 0x404);
    if (home == nullptr) {
        at<std::uint8_t>(squadron, 0x408) = 0;
        return;
    }
    void* const block = host.air_ops_get_block_006bcd20(home, 0);
    if (rd<std::int32_t>(game_pointer(host), 0x1FE4) != 0) {
        host.air_ops_push_ready_plane_006cc760(block, squadron);
        cc13_seg48_plane_squadron_stow_members_inside_007ed6e0(squadron, host);
        return;
    }
    host.air_ops_push_ready_plane_alternate_006cc7b0(block, squadron);
    if (inside != 0) {
        at<std::uint8_t>(squadron, 0x408) = inside;
        cc13_seg48_plane_squadron_stow_members_inside_007ed6e0(squadron, host);
    }
}

// ---------------------------------------------------------------------------
// 007F1CA0 BSP_PlaneSquadron_ReturnPlaneToStock, __thiscall(ECX = squadron; base, plane),
// RET 8. Disk bytes 007F1CA0-007F1D66. Byte +3B8h = (plane is the leader); slot +10h
// (result unused); the observer link to base; block = 006BCD20(base, 0);
// 006CA770(block, plane+538h class, 1); with one member left: 00922F80(squadron, 0), game
// byte +193Ch = 0, 00696350(ECX = squadron, EDX = 1; +404h), and outside game mode 2
// 006C5950(block, squadron); then 00926D90(plane, 5) and byte +3ECh = 1. cc9's host carries
// only the stock add, the slot return and the kill (game_hosts_units.cpp:15741).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_return_plane_to_stock_007f1ca0(void* squadron, void* base, void* plane,
                                                             Cc15Seg48Host& host) {
    at<std::uint8_t>(squadron, 0x3B8) = plane == member(squadron, 0) ? 1 : 0;
    host.object_vcall_10_name(squadron);
    set_home_link(squadron, base, host);
    void* const block = host.air_ops_get_block_006bcd20(base, 0);
    host.air_ops_add_stock_006ca770(block, ptr(plane, 0x538), 1);
    if (member_count(squadron) == 1) {
        host.fun_00922f80(squadron, 0);
        at<std::uint8_t>(game_pointer(host), 0x193C) = 0;
        host.fun_00696350(squadron, 1, ptr(squadron, 0x404));
        if (rd<std::int32_t>(game_pointer(host), 0x1FE4) != 2) host.air_ops_return_slot_006c5950(block, squadron);
    }
    host.unit_kill_00926d90(plane, 5);
    at<std::uint8_t>(squadron, 0x3EC) = 1;
}

// ---------------------------------------------------------------------------
// 007F1D90 BSP_PlaneSquadron_SelectAvoidZoneLayers, __thiscall(ECX = squadron; float
// slope, int flag), RET 8. Disk bytes 007F1D90-007F1DD1. +350h = 0041DF40(004C17D0();
// slope (FLD/FSTP), flag); +34Ch = 0041DF40(004C17D0(); 1.5 ([00CE380C]), 1).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_select_avoid_zone_layers_007f1d90(void* squadron, float slope, std::int32_t flag,
                                                                Cc15Seg48Host& host) {
    at<std::uint32_t>(squadron, 0x350) =
        host.avoid_zone_layer_select_0041df40(host.avoid_zone_layer_manager_004c17d0(), x87_float_round_trip(slope), flag);
    at<std::uint32_t>(squadron, 0x34C) =
        host.avoid_zone_layer_select_0041df40(host.avoid_zone_layer_manager_004c17d0(), 1.5f, 1);
}

// ---------------------------------------------------------------------------
// 007F1FE0 BSP_PlaneSquadron_BeginOrderSpeed, __fastcall(ECX = squadron), RET (vtable
// +A0h, the creation pass B). EH frame 00C8F3FB. Evidence: listing 007F1FE0-007F2280.
// 0077F0E0; +370h = 1, +388h = 0.5 ([00CE3800]), +3A0h = [+35Ch]+190h, byte +3ACh = 1,
// byte +38Fh = 0. Without a creation record (+C0h) of kind 3, a 22Ch command block from
// 00BF681B built by 0084D810(squadron) (null when the allocation fails) goes to +348h. The
// value handed to slot +128h at the end defaults to 1. Kind 1 (scene): 007F1D90([+35Ch]+1ECh,
// 1); the value = 00927A80(); every member takes +58h, +54h and the nine dwords at +188h;
// 009292B0([[leader+538h]+70h], slot +10h name); a state record built by
// 00721280([+348h]) with vtable 00D086F4; from the property bag (record+8h, 008F2260):
// "FireStance" (00CFC850) with +Ch >= 0 sets state bytes +4h/+5h from the controller's
// slots +24h/+28h(value != 0); "ReloadEnabled" of type 3 sets byte +369h;
// "PlayerCommandsEnabled" absent or non-zero gives state +8h = 1FFh, else 0; then the
// controller's slot +3Ch(&state). Kind 2 (network): the CREATE message at record+8h:
// +58h = byte +C4h; a non-zero word +114h resolved by 00521E30 sets the home base
// (007F1C00(base, byte +116h)); the value = +B8h; the controller's slot +3Ch(+118h); byte
// +369h = +124h. Name is a hypothesis (the order-speed field is one of many it seeds).
// Uncertainty: the state record bytes 00721280 does not write are stack garbage in the
// image and zero here. cc9 models only the ReloadEnabled and skill reads
// (game_hosts_units.cpp:13374, game_hosts_lua.cpp:4740).
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_begin_from_creation_record_007f1fe0(void* squadron, Cc15Seg48Host& host) {
    host.unit_owner_entity_activate_and_register_0077f0e0(squadron);
    at<std::int32_t>(squadron, 0x370) = 1;
    at<std::uint32_t>(squadron, 0x388) = float_bits(0.5f);
    at<std::uint32_t>(squadron, 0x3A0) = rd<std::uint32_t>(ptr(squadron, 0x35C), 0x190);
    at<std::uint8_t>(squadron, 0x3AC) = 1;
    at<std::uint8_t>(squadron, 0x38F) = 0;
    const void* record = ptr(squadron, 0xC0);
    if (record == nullptr || rd<std::int32_t>(record, 4) != 3) {
        void* const block = host.operator_new_00bf681b(0x22C);
        at<void*>(squadron, 0x348) =
            block != nullptr ? host.plane_squadron_command_block_construct_0084d810(block, squadron) : nullptr;
    }
    record = ptr(squadron, 0xC0);
    std::uint32_t value = 1;
    const std::int32_t kind = rd<std::int32_t>(record, 4);
    if (kind == 1) {
        cc15_seg48_plane_squadron_select_avoid_zone_layers_007f1d90(
            squadron, x87_float_round_trip(rd<float>(ptr(squadron, 0x35C), 0x1EC)), 1, host);
        value = host.property_bag_get_skill_00927a80(squadron);
        for (std::int32_t i = 0; i < member_count(squadron); ++i) {
            void* const m = member(squadron, i);
            at<std::uint32_t>(m, 0x58) = rd<std::uint32_t>(squadron, 0x58);
            at<std::uint32_t>(m, 0x54) = rd<std::uint32_t>(squadron, 0x54);
            std::memcpy(bytes(m, 0x188), bytes(squadron, 0x188), 9 * 4);
        }
        void* const leader = member(squadron, 0);
        const char* const name = host.object_vcall_10_name(squadron);
        host.unit_bind_lua_class_009292b0(squadron, rd<std::uint32_t>(ptr(leader, 0x538), 0x70), name);
        alignas(4) std::uint8_t state[0x40] = {};
        host.command_controller_state_build_00721280(state, ptr(squadron, 0x348));
        at<std::uint32_t>(state, 0) = 0x00D086F4u;
        const void* field = host.scene_property_bag_find_008f2260(ptr(ptr(squadron, 0xC0), 8), "FireStance");
        if (field != nullptr && rd<std::int32_t>(field, 0x0C) >= 0) {
            const std::uint8_t stance = rd<std::int32_t>(field, 0x0C) != 0 ? 1 : 0;
            state[4] = host.controller_vcall_24(ptr(squadron, 0x348), stance);
            state[5] = host.controller_vcall_28(ptr(squadron, 0x348), stance);
        }
        field = host.scene_property_bag_find_008f2260(ptr(ptr(squadron, 0xC0), 8), "ReloadEnabled");
        if (field != nullptr && rd<std::int32_t>(field, 4) == 3) {
            at<std::uint8_t>(squadron, 0x369) = rd<std::uint8_t>(field, 0x0C);
        }
        field = host.scene_property_bag_find_008f2260(ptr(ptr(squadron, 0xC0), 8), "PlayerCommandsEnabled");
        const bool commands_enabled = field == nullptr || rd<std::uint8_t>(field, 0x0C) != 0;
        at<std::uint32_t>(state, 8) = commands_enabled ? 0x1FFu : 0u;
        host.controller_vcall_3c(ptr(squadron, 0x348), state);
    } else if (kind == 2) {
        const void* const message = ptr(record, 8);
        at<std::uint32_t>(squadron, 0x58) = rd<std::uint8_t>(message, 0xC4);
        const std::uint16_t base_id = rd<std::uint16_t>(message, 0x114);
        if (base_id != 0) {
            void* const base = host.entity_handle_table_resolve_00521e30(base_id);
            if (base != nullptr) {
                cc15_seg48_plane_squadron_set_home_air_base_007f1c00(squadron, base, rd<std::uint8_t>(message, 0x116),
                                                                    host);
            }
        }
        value = rd<std::uint32_t>(message, 0xB8);
        host.controller_vcall_3c(ptr(squadron, 0x348), ptr(message, 0x118));
        at<std::uint8_t>(squadron, 0x369) = rd<std::uint8_t>(message, 0x124);
    }
    host.squadron_vcall_128(squadron, value);
}

// ---------------------------------------------------------------------------
// 007F2E20 BSP_PlaneSquadron_SetWorldPosition, __thiscall(ECX = squadron, const float[3]),
// RET 4. EH frame 00C8F438. Disk bytes 007F2E20-007F2EA4. In game mode 1 with byte
// +218Ch clear, an ENTITY_RELOCATE position message (007EDF00(position, 0)) is routed
// through 0077C7B0(message, 0) and destroyed (vtable 00CE4974); then 00489760(position)
// and 007F2920.
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_set_world_position_007f2e20(void* squadron, const float* position,
                                                          Cc15Seg48Host& host) {
    void* const game = game_pointer(host);
    if (rd<std::int32_t>(game, 0x1FE4) == 1 && rd<std::uint8_t>(game, 0x218C) == 0) {
        alignas(4) std::uint8_t message[0x38] = {};
        void* const built =
            cc13_seg48_session_message_entity_relocate_construct_position_007edf00(message, position, 0, host);
        host.entity_route_message_0077c7b0(squadron, built, 0);
        at<std::uint32_t>(message, 0) = 0x00CE4974u;
    }
    host.entity_set_world_position_00489760(squadron, position);
    host.squadron_place_members_007f2920(squadron);
}

// ---------------------------------------------------------------------------
// 007F2EB0 BSP_PlaneSquadron_SetHeading, __thiscall(ECX = squadron, float heading), RET 4.
// EH frame 00C8F458. Disk bytes 007F2EB0-007F2F8A. In game mode 1 with byte +218Ch clear,
// an ENTITY_RELOCATE heading message built in place exactly as 007EDEB0(heading, 0) does
// (inlined; MOVSS heading) is routed through 0077C7B0(message, 0); then 0047F200(heading,
// FLD/FSTP), the child walk and 007F2920.
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_set_heading_007f2eb0(void* squadron, float heading, Cc15Seg48Host& host) {
    void* const game = game_pointer(host);
    if (rd<std::int32_t>(game, 0x1FE4) == 1 && rd<std::uint8_t>(game, 0x218C) == 0) {
        alignas(4) std::uint8_t message[0x38] = {};
        cc13_seg48_session_message_entity_relocate_construct_heading_007edeb0(message, heading, 0, host);
        host.entity_route_message_0077c7b0(squadron, message, 0);
    }
    host.entity_set_heading_0047f200(squadron, x87_float_round_trip(heading));
    invalidate_children(squadron, host);
    host.squadron_place_members_007f2920(squadron);
}

// ---------------------------------------------------------------------------
// 007F2F90 BSP_PlaneSquadron_SetWorldMatrix, __thiscall(ECX = squadron, const float[16]),
// RET 4. Disk bytes 007F2F90-007F2FCD. 004134F0(squadron+74h, matrix), the child walk and
// 007F2920.
// ---------------------------------------------------------------------------
void cc15_seg48_plane_squadron_set_world_matrix_007f2f90(void* squadron, const float* matrix,
                                                        Cc15Seg48Host& host) {
    host.matrix_copy4x4_004134f0(bytes(squadron, 0x74), matrix);
    invalidate_children(squadron, host);
    host.squadron_place_members_007f2920(squadron);
}

// ---------------------------------------------------------------------------
// 007F2FD0 BSP_PlaneSquadron_RebuildLuaMemberTable, __thiscall(ECX = entity; int reason,
// int count), RET 8. EH frame 00C8F4A8. Evidence: listing 007F2FD0-007F3196. It adds count
// to the entity's Lua self "PlaneKillReasons" (00D08B2C) table entry named 00E0CF04[reason]
// (none, harm, soft, sell, exitzone, landed, editor), so the old name is wrong. Under the
// mission-entity lock (004C1570; its CRITICAL_SECTION at +4h is entered and the counter at
// +18h raised, both undone at the end; the scoped-lock local's vtable 00CE37FC is not
// modelled): self = 00927B40; the table = self["PlaneKillReasons"]; when nil, a new table
// is set under a native-string key and re-fetched into it (00B67580, 00B67800, 00B67690);
// the current value (00B66290, 0 when nil) plus count is stored through 00B67460 under a
// native-string key; the three Lua objects are destroyed in reverse order.
// ---------------------------------------------------------------------------
void cc15_seg48_entity_add_plane_kill_reason_007f2fd0(void* entity, std::int32_t reason, std::int32_t count,
                                                      Cc15Seg48Host& host) {
    static constexpr char kTable[] = "PlaneKillReasons";
    void* const section = ptr(host.mission_entity_lock_get_004c1570(), 4);
    if (section != nullptr) {
        host.win32_enter_critical_section(section);
        ++at<std::int32_t>(section, 0x18);
    }
    alignas(4) std::uint8_t self[0x14] = {};
    alignas(4) std::uint8_t table[0x14] = {};
    host.mission_entity_lua_self_object_00927b40(entity, self);
    host.lua_object_get_by_name_00b67800(self, table, kTable);
    if (host.lua_object_is_nil_00b65fb0(table)) {
        NativeString key{0, nullptr};
        host.native_string_assign_0041e870(&key, kTable);
        host.lua_object_set_new_table_00b67580(self, &key);
        release_native_string(key, host);
        alignas(4) std::uint8_t fresh[0x14] = {};
        host.lua_object_assign_00b67690(table, host.lua_object_get_by_name_00b67800(self, fresh, kTable));
        host.lua_object_destruct_00b67700(fresh);
    }
    const auto reason_name = [&] {
        return reinterpret_cast<const char*>(
            static_cast<std::uintptr_t>(*host.image_global(0x00E0CF04u + static_cast<std::uint32_t>(reason) * 4u)));
    };
    std::int32_t current = 0;
    alignas(4) std::uint8_t value[0x14] = {};
    host.lua_object_get_by_name_00b67800(table, value, reason_name());
    if (!host.lua_object_is_nil_00b65fb0(value)) current = host.lua_object_get_integer_00b66290(value);
    NativeString key{0, nullptr};
    host.native_string_assign_0041e870(&key, reason_name());
    host.lua_object_set_native_string_integer_00b67460(table, &key, current + count);
    release_native_string(key, host);
    host.lua_object_destruct_00b67700(value);
    host.lua_object_destruct_00b67700(table);
    host.lua_object_destruct_00b67700(self);
    if (section != nullptr) {
        --at<std::int32_t>(section, 0x18);
        host.win32_leave_critical_section(section);
    }
}

}  // namespace bsp
