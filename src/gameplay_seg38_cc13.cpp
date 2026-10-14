// Segment 38 reference models. Packet cc13-gpr38; docs/CC13_GPR_38.md.
// Semantic models read from the disk bytes and the Ghidra pseudocode; see the
// header for the ABI of each routine and the adoption note. Build-tested only.

#include "bsp/gameplay_seg38_cc13.hpp"

#include <cmath>
#include <cstring>
#include <limits>

namespace bsp {
namespace gameplay_seg38_cc13 {

namespace {

// 00D7A248: FLT_MAX, the descriptor floats' scale and 0071DB90's seed.
constexpr float kFltMax_00d7a248 = std::numeric_limits<float>::max();
// 00D7A264: pi as a float (DB 0F 49 40).
constexpr float kPi_00d7a264 = 3.14159274f;
// 00CE4ADC: -1e10 as a float (F9 02 15 D0).
constexpr float kMinusTenBillion_00ce4adc = -1.0e10f;
// 00D7A3A0 / 00CE3928: +-0.1f promoted to double (A0 99 99 B9 3F / BF).
constexpr double kPushEpsilon = static_cast<double>(0.1f);

bool category_attacks(int category) noexcept { return category == 1 || category == 2; }

}  // namespace

int first_empty_slot(const DirectorQueue& q) noexcept {
    int i = 0;
    while (i < kCommandSlots && q.slots[static_cast<std::size_t>(i)].command_class != 0) ++i;
    return i;
}

int filled_slot_count_0071be60(const DirectorQueue& q) noexcept { return first_empty_slot(q); }

int command_path_cursor_slot_0071bff0(const DirectorQueue& q, unsigned index) noexcept {
    if (index > 9 || !q.paths[index].present) return 0;
    return static_cast<int>(index);
}

NewestTarget newest_command_target_0071ebf0(const DirectorQueue& q, std::int32_t mode_30) noexcept {
    NewestTarget t;
    if (mode_30 == 1) {
        for (int i = first_empty_slot(q) - 1; i >= 0; --i) {
            const CommandSlot& s = q.slots[static_cast<std::size_t>(i)];
            if (s.command_class != 0 && category_attacks(s.category)) {
                t.source = NewestTargetSource::kSlot;
                t.slot = i;
                return t;
            }
        }
    } else if (mode_30 == 2) {
        t.source = NewestTargetSource::kOverride;
    }
    return t;
}

std::vector<std::uint32_t> collect_children_by_prefix_0071ba20(
    const std::vector<std::array<std::uint32_t, 2>>& entries,
    const std::function<bool(const std::array<std::uint32_t, 2>&)>& accepts) {
    std::vector<std::uint32_t> out;
    for (const auto& e : entries) {
        if (accepts(e)) out.push_back(e[1]);
    }
    return out;
}

int slot_moveonpath_path_0071bde0(const DirectorQueue& q, unsigned index) noexcept {
    if (index >= static_cast<unsigned>(kCommandSlots)) return -1;
    if (q.slots[index].command_class != kCmdMoveOnPath) return -1;
    const SlotPath& p = q.paths[index];
    if (!p.present || !p.vt4_valid) return -1;
    return static_cast<int>(index);
}

bool has_free_command_slot_0071c0c0(const DirectorQueue& q, int want) noexcept {
    if (want == 0) return false;
    return first_empty_slot(q) < kCommandSlots;
}

int path_follow_mode_slot_0071c1b0(const DirectorQueue& q) noexcept {
    // 0071C1C9: MOV ECX,[ESI+EAX*4+1A0h] with EAX = first empty index.
    return first_empty_slot(q) - 1;
}

int command_count_0071d780(const DirectorQueue& q) noexcept {
    int i = 0;
    do {
        const CommandSlot& s = q.slots[static_cast<std::size_t>(i)];
        if (s.command_class == 0) return i;
        int step = 1;
        const SlotPath& p = q.paths[static_cast<std::size_t>(i)];
        if (s.command_class == kCmdMoveOnPath && p.present && p.vt4_valid && p.has_point_list &&
            p.point_count > 0) {
            step = p.point_count;
        }
        i += step;
    } while (i < kCommandSlots);
    return i;
}

bool slot_has_active_order_0071dee0(const DirectorQueue& q, unsigned index) noexcept {
    if (index >= static_cast<unsigned>(kCommandSlots)) return false;
    const std::uint32_t c = q.slots[index].command_class;
    if (c == 0 || c == kCmdStop || c == kCmdCruise || c == kCmdLeave || c == kCmdDisband) return false;
    if (c != kCmdMoveOnPath) return true;
    const SlotPath& p = q.paths[index];
    return p.present && p.has_point_list && p.point_count > 0;
}

bool accepts_new_target_0071df70(const DirectorQueue& q, float override_delay_40) noexcept {
    if (0.0f < override_delay_40) return false;   // 00D7A218 < +40h
    const int n = first_empty_slot(q);
    for (int i = 0; i < n; ++i) {
        if (category_attacks(q.slots[static_cast<std::size_t>(i)].category)) return false;
    }
    return true;
}

bool last_command_needs_new_path_0071dc80(const DirectorQueue& q,
                                         std::optional<int> resolved_kind) noexcept {
    int i = kCommandSlots - 1;
    while (q.slots[static_cast<std::size_t>(i)].command_class == 0) {
        if (--i < 0) return true;
    }
    const CommandSlot& s = q.slots[static_cast<std::size_t>(i)];
    if (s.command_class != kCmdMoveOnPath) return true;
    if (!s.target.has_entity_00) return false;
    if (!resolved_kind) return false;            // the handle table answered null
    return *resolved_kind == 0x12;
}

bool command_accepts_target_0071d6d0(const CommandSlot& command, const CommandTarget& target,
                                     const AcceptsTargetFacts& f) noexcept {
    if (command.requires_target &&
        (!target.has_point_01 || category_attacks(command.category))) {
        if (!f.resolves) return false;
        if (f.target_marked_5d) return false;
    }
    if (command.command_class == kCmdTorpedo && !f.torpedo_target_ok) return false;
    if (command.command_class == kCmdMoveOnPath && target.has_entity_00 && !f.has_path_interface) {
        return false;
    }
    // attackmove resolves its target here and discards the answer.
    return true;
}

bool slot_target_matches_0071e200(const CommandTarget& a, const CommandTarget& b,
                                  bool same_resolved_object) noexcept {
    if (!same_resolved_object) return false;
    const Vec3f zero{};
    const Vec3f& pb = b.has_point_01 ? b.point_08 : zero;
    const Vec3f& pa = a.has_point_01 ? a.point_08 : zero;
    const float dz = pa.z - pb.z;
    const float dx = pa.x - pb.x;
    const float dy = pa.y - pb.y;
    return dz * dz + dx * dx + dy * dy <= 1.0f;
}

void store_flag_3c_0071d5d0(DirectorFlags& d, bool v) noexcept { d.flag_3c = v; }
void set_flags_1cc_0071c0a0(DirectorFlags& d, std::uint32_t v) noexcept { d.flags_1cc = v; }
void set_enables_220_00720de0(DirectorFlags& d, bool a, bool b, bool c, bool e) noexcept {
    d.enables_220 = {a, b, c, e};
}
void set_avoidance_240_00720e50(DirectorFlags& d, bool a, bool b, bool c) noexcept {
    d.avoidance_240 = {a, b, c};
}
bool land_collision_avoidance_00720e40(const DirectorFlags& d) noexcept { return d.avoidance_240[2]; }

bool director_flags_allow_0071c060(bool has_director, std::uint32_t flags_1cc,
                                   std::uint32_t mask) noexcept {
    if (!has_director) return true;
    return (flags_1cc & mask) != 0;
}

DirectorMessageResult apply_command_message_0071c1e0(DirectorFlags& d, std::int32_t sub_kind,
                                                     std::int32_t value) noexcept {
    DirectorMessageResult r;
    switch (sub_kind) {
        case 0: r.action = DirectorMessageAction::kVirtual64; r.virtual_argument = value != 0; break;
        case 1: r.action = DirectorMessageAction::kVirtual68; r.virtual_argument = value != 0; break;
        case 2: d.flags_1cc = static_cast<std::uint32_t>(value); r.action = DirectorMessageAction::kStored; break;
        case 3: case 4: case 5: case 6:
            d.enables_220[static_cast<std::size_t>(sub_kind - 3)] = value != 0;
            r.action = DirectorMessageAction::kStored;
            break;
        default: break;
    }
    return r;
}

RoutedDirectorMessage director_sub_kind_message(std::uint32_t sub_kind, std::uint32_t value) noexcept {
    return RoutedDirectorMessage{0x5Au, sub_kind, value};
}
RoutedDirectorMessage clear_command_slot_message_0071d900(std::uint32_t slot) noexcept {
    return RoutedDirectorMessage{0x5Du, 1u, slot};
}
RoutedDirectorMessage clearcmd_construct_0071c730(std::uint8_t field_20, std::uint32_t field_24) noexcept {
    return RoutedDirectorMessage{0x5Du, field_20, field_24};
}
RoutedDirectorMessage set_cur_path_index_message_0071cdd0(std::uint32_t index) noexcept {
    return RoutedDirectorMessage{0x60u, index, 0u};
}
RoutedDirectorMessage clear_user_path_point_message_0071d3e0() noexcept {
    return RoutedDirectorMessage{0x5Fu, 0u, 0u};
}

RoutedDirectorMessage clear_commands_message_0071d880() noexcept {
    return RoutedDirectorMessage{0x5Du, 1u, 0xFFFFFFFFu};
}

UserPathPointMessage add_user_path_point_message_0071d340(const Vec3f& point) noexcept {
    UserPathPointMessage m;
    m.point = point;
    m.add_2c = 1;
    return m;
}

namespace {
std::optional<RoutedDirectorMessage> raise_stage(std::int32_t& stage_field, std::int32_t stage,
                                                 std::int32_t session_mode, std::uint8_t primary) noexcept {
    if (stage_field >= stage) return std::nullopt;
    stage_field = stage;
    if (stage != 2 || session_mode == 2) return std::nullopt;
    return clearcmd_construct_0071c730(primary, 0u);
}
}  // namespace

std::optional<RoutedDirectorMessage> raise_command_stage_0071d810(std::int32_t& stage_48, std::int32_t stage,
                                                                  std::int32_t session_mode) noexcept {
    return raise_stage(stage_48, stage, session_mode, 1);
}

std::optional<RoutedDirectorMessage> raise_override_stage_0071d9e0(std::int32_t& stage_50, std::int32_t stage,
                                                                   std::int32_t session_mode) noexcept {
    return raise_stage(stage_50, stage, session_mode, 0);
}

int path_follower_remaining_or_minus_one_0071d2a0(bool attached, int vt14_answer) noexcept {
    return attached ? vt14_answer : -1;
}

bool send_override_command_0071d980(float override_delay_40) noexcept {
    // 0071D98x: FLD [ECX+40h]; FCOMP 0.0 -> sent only for a strictly negative delay.
    return override_delay_40 < 0.0f;
}

bool echo_clear_command_0071e2e0(std::int32_t& override_stage_50, std::int32_t session_mode) noexcept {
    if (override_stage_50 >= 2) return false;
    override_stage_50 = 2;
    return session_mode != 2;
}

EntityRemovedActions on_entity_removed_0071ddb0(const DirectorQueue& q,
                                                const EntityRemovedInputs& in) noexcept {
    EntityRemovedActions a;
    if (!in.removed_present || in.game_state_5d4 <= 0xB) return a;
    if (in.override_targets_removed && in.override_vt70 && in.session_mode != 2) {
        a.raise_override_stage_2 = true;
    }
    for (int i = 0; i < kCommandSlots; ++i) {
        const auto u = static_cast<std::size_t>(i);
        if (q.slots[u].command_class == 0 || !in.slot_targets_removed[u]) continue;
        if (!in.slot_vt70[u] || in.session_mode == 2) continue;
        if (i == 0) a.raise_command_stage_2 = true;
        else if (in.slot_vt74[u]) a.clear_slots.push_back(i);
    }
    return a;
}

RetargetResult retarget_to_member_or_point_0071edd0(CommandTarget& target,
                                                    const RetargetInputs& in) noexcept {
    RetargetResult r;
    if (!target.has_entity_00) return r;   // answer true, target unchanged
    const RetargetMember* chosen = nullptr;
    if (in.resolved_present && in.resolved_is_kind_0f && in.has_squadron_block) {
        for (std::size_t i = 0; i < in.members.size(); ++i) {
            if (i >= 5) { r.image_faults = true; break; }   // null member, reads +5Dh at 5Dh
            const RetargetMember& m = in.members[i];
            if (m.marked_5d) continue;
            const float dx = m.position.x - in.director_unit_position.x;
            const float dy = m.position.y - in.director_unit_position.y;
            const float dz = m.position.z - in.director_unit_position.z;
            // 00D7A278 is FLT_MAX as a double: every finite distance passes,
            // so the LAST live member wins.
            if (static_cast<double>(dz * dz + dx * dx + dy * dy) <
                static_cast<double>(std::numeric_limits<float>::max())) {
                chosen = &m;
            }
        }
    }
    if (chosen != nullptr) {
        target = CommandTarget{};
        target.has_entity_00 = true;
        target.entity_04 = chosen->entity;
        target.entity_index_02 = chosen->entity_index_174;
        r.answer = false;
        r.member_chosen = true;
        return r;
    }
    if (!in.resolved_present) r.image_faults = true;   // reads null+C8h
    target = CommandTarget{};
    target.has_point_01 = true;
    target.point_08 = in.resolved_position;
    r.answer = true;
    return r;
}

ControllerUpdateActions controller_update_0071f290(ControllerUpdateState& s,
                                                   const ControllerUpdateInputs& in) noexcept {
    ControllerUpdateActions a;
    if (!in.unit_present || !in.unit_alive_5c || in.unit_marked_5d || in.unit_flag_60 ||
        in.unit_flag_5e) {
        return a;
    }
    a.ran = true;
    a.zero_slot0_path_offset = in.slot0_path_present;
    if (0.0f <= s.override_delay_40) s.override_delay_40 -= in.dt;
    if (s.mode_30 == 0 && in.slot0_filled) s.mode_30 = 1;
    if (!in.override_acceptance_4c && in.override_filled && !in.vt78_override) {
        a.raise_override_stage_2 = true;
    }
    if (!in.acceptance_44 && in.slot0_filled && !in.vt78_primary) a.raise_command_stage_2 = true;
    if (in.session_mode != 2) {
        a.observer_tick = in.observer_present;
        a.step = true;
    }
    return a;
}

void copy_state_from_00720450(DirectorCopyState& dst, const DirectorCopyState& src, bool owner_is_kind_6,
                              const std::function<NearestUnitAnswer(const Vec3f&)>& nearest_to,
                              const std::function<Vec3f(const CommandTarget&)>& target_position) {
    dst.byte_3c = src.byte_3c;
    dst.byte_3d = src.byte_3d;
    dst.field_16c = src.field_16c;
    dst.target_170 = src.target_170;
    dst.mode_30 = src.mode_30;
    for (int i = 0; i < kCommandSlots; ++i) {
        const auto u = static_cast<std::size_t>(i);
        CommandSlot s = src.queue.slots[u];
        if (owner_is_kind_6 && s.command_class == kCmdLand) {     // 00720600..
            const Vec3f at = target_position(s.target);           // BSP_TargetDescriptor_ResolvePosition
            const NearestUnitAnswer n = nearest_to(at);
            s.command_class = kCmdAttackMove;
            s.target = CommandTarget{};
            s.target.entity_04 = n.entity;
            if (n.entity != 0) {
                s.target.has_entity_00 = true;
                s.target.entity_index_02 = n.entity_index_174;
            }
        }
        dst.queue.slots[u] = s;
        dst.queue.paths[u] = src.queue.paths[u];                 // 0071FD10
    }
    dst.override_188 = CommandSlot{};
    dst.override_acceptance_4c = false;
    dst.acceptance_44 = false;
    dst.override_stage_50 = 0;
    dst.stage_48 = 0;
}

CurrentPathPointSource current_path_point_0071e4c0(const DirectorQueue& q) noexcept {
    const CommandSlot& head = q.slots[0];
    if (head.command_class == kCmdMoveOnPath) {
        return q.paths[0].present ? CurrentPathPointSource::kPathCursor
                                  : CurrentPathPointSource::kNullPathCursor;
    }
    if (!head.target.has_point_01 && !head.target.has_entity_00) return CurrentPathPointSource::kInvalid;
    return CurrentPathPointSource::kTargetPosition;
}

BeginCommandResult begin_command_base_0071f600(const BeginCommandInputs& in,
                                               std::uint32_t head_class_after_rewrite) noexcept {
    BeginCommandResult r;
    if (!in.use_queue_head) {
        if (!in.override_present) return r;   // +4Ch = 0, but AL keeps its initial 1
        r.accepted = in.override_accepts;     // +4Ch
        r.stored_acceptance = r.accepted;
        if (r.accepted) {
            r.report_start = true;               // not gated on [00F8A0C4]
            r.report_target = in.override_target_resolves;
        }
        return r;
    }
    if (!in.head_present) {
        // +44h = 0 but AL keeps its initial 1 (0071F63x).
        r.accepted = true;
        return r;
    }
    if (in.head_target_resolves && in.head_target_kind_1c &&
        category_attacks(in.head_category) && in.head_target_same_endpoint) {
        r.rewrote_head_to_moveto = true;
    }
    r.accepted = in.head_accepts_after_rewrite;
    r.stored_acceptance = r.accepted;
    if (!r.accepted) return r;
    if (head_class_after_rewrite == kCmdMoveOnPath) {
        r.path_build = in.head_target_has_entity_58 ? MoveOnPathBuild::kFromEntityPath
                                                    : MoveOnPathBuild::kFromSlotPath;
    }
    if (in.events_enabled_00f8a0c4) {
        r.report_start = true;
        r.report_target = in.head_target_resolves;
    }
    return r;
}

void gun_bot_retire_0071c4a0(GunBotRetireBytes& b) noexcept {
    b.b10 = true;
    b.b11 = false;
}

bool stance_is_zero_0071d5a0(std::int32_t stance) noexcept { return stance == 0; }
bool arg_is_zero_0071c480(std::int32_t v) noexcept { return v == 0; }

float max_of_twenty_floats_0071d600(const std::array<float, 20>& v) noexcept {
    float m = kMinusTenBillion_00ce4adc;
    for (const float f : v) {
        if (m <= f) m = f;
    }
    return m;
}

Vec3f path_listener_point_0071ce60(const Vec3f& cursor_point, bool offset_on_18,
                                   const Vec3f& offset_20) noexcept {
    if (!offset_on_18) return cursor_point;
    return Vec3f{offset_20.x + cursor_point.x, offset_20.y + cursor_point.y,
                 offset_20.z + cursor_point.z};
}

bool oriented_rect_contains_0071cea0(const OrientedRect& r, float x, float z) noexcept {
    const float rx = x - r.centre[0];
    const float rz = z - r.centre[1];
    return std::fabs(rz * r.axis_a[1] + rx * r.axis_a[0]) < r.half_a_20 &&
           std::fabs(rz * r.axis_b[1] + r.axis_b[0] * rx) < r.half_b_1c;
}

std::array<float, 2> oriented_rect_clamp_0071cf30(const OrientedRect& r, float x, float z,
                                                  float dx, float dz) noexcept {
    const float rx = x - r.centre[0];
    const float rz = z - r.centre[1];
    float u = rz * r.axis_b[1] + rx * r.axis_b[0];   // along axis b, [ESP+18h]
    float v = rz * r.axis_a[1] + r.axis_a[0] * rx;   // along axis a, [ESP+1Ch]
    const float hb = r.half_b_1c;
    const float ha = r.half_a_20;
    // 0071CF93-0071CFD2. The fourth test is u <= -ha (FCOMPI ST(3) holds u).
    if (hb <= u || u <= -hb || ha <= v || u <= -ha) return {x, z};

    const float pb = dx * r.axis_b[0] + dz * r.axis_b[1];
    const float pa = dx * r.axis_a[0] + dz * r.axis_a[1];
    float t = 0.0f;
    bool settled = false;
    auto along_a = [&](float num, float den) {
        // 0071D01A: walk to the a-boundary, then fix the b overshoot.
        const float k = num / den;
        u = pb * k + u;
        v = v + pa * k;
        if (u <= hb) {
            if (-hb <= u) { settled = true; return; }
            t = u + hb;
        } else {
            t = u - hb;
        }
        t = t / pb;
    };
    if (static_cast<double>(pa) <= kPushEpsilon) {
        if (static_cast<double>(pa) < -kPushEpsilon) {
            along_a(v + ha, -pa);
        } else {
            // |pa| <= 0.1: walk to the b-boundary first.
            bool over;
            if (pb <= 0.0f) {
                const float k = (u + hb) / -pb;
                u = k * pb + u;
                v = v + k * pa;
                over = !(v <= ha);
            } else {
                const float k = (hb - u) / pb;
                u = k * pb + u;
                v = v + k * pa;
                over = ha < v;
            }
            if (over) {
                t = (v - ha) / pa;
            } else if (-ha <= v) {
                settled = true;
            } else {
                t = (v + ha) / pa;
            }
        }
    } else {
        along_a(ha - v, pa);
    }
    if (!settled) {
        u = u - t * pb;
        v = v - t * pa;
    }
    // 0071D1B9.
    return {r.centre[0] + v * r.axis_a[0] + u * r.axis_b[0] + dx,
            r.centre[1] + v * r.axis_a[1] + u * r.axis_b[1] + dz};
}

bool path_follower_attached_0071d270(bool source_present, bool source_is_own_list) noexcept {
    return source_present && source_is_own_list;
}

int path_follower_remaining_0071d2e0(bool attached, int cursor_18, int own_point_count) noexcept {
    if (attached) return cursor_18;
    if (own_point_count > 0) return 0;
    return -1;
}

DirectorPathObject director_path_object_construct_0071fb90(std::uint32_t owner) noexcept {
    DirectorPathObject p;
    p.owner_04 = owner;
    p.owner_24 = owner;
    return p;
}

void path_drop_last_point_0071f570(DirectorPathObject& p) noexcept {
    if (p.points_40.size() > 1) p.points_40.pop_back();
}

int clear_user_path_points_slot_0071f5d0(const DirectorQueue& q) noexcept {
    return first_empty_slot(q) - 1;   // 0071F5E5: MOV ECX,[ECX+EAX*4+1A0h]
}

bool entity_as_kind_0f_0071ea00(bool present, bool is_kind_0f) noexcept {
    return present && is_kind_0f;
}

bool active_target_object_resolves_0071eda0(std::int32_t mode_30) noexcept { return mode_30 != 0; }

std::array<NamedValue, 2> path_follow_mode_write_0071eb00(std::int32_t follow_mode,
                                                          std::int32_t start_mode) noexcept {
    return {NamedValue{"followMode", follow_mode}, NamedValue{"startMode", start_mode}};
}

AddUserPathPlan apply_add_user_path_point_007207c0(const DirectorQueue& q, bool needs_new_path,
                                                   bool can_push_moveonpath) noexcept {
    AddUserPathPlan p;
    if (!needs_new_path) {
        p.add = true;
        p.slot = first_empty_slot(q) - 1;   // the last filled slot's path
        return p;
    }
    if (!can_push_moveonpath) return p;
    // vtable[34h] queued the moveonpath; the index is counted after it.
    p.add = true;
    p.replace = true;
    p.slot = first_empty_slot(q);
    return p;
}

AddPointResult director_path_add_point_0071fde0(
    std::vector<Vec3f>& points, const Vec3f& point, const AddPointInputs& in,
    const std::function<void(std::vector<Vec3f>&)>& clip_points,
    const std::function<bool(const Vec3f&)>& outside_map,
    const std::function<std::array<Vec3f, 2>(const Vec3f& from, const Vec3f& to)>& clip_segment,
    const std::function<void(std::vector<Vec3f>&, int)>& prune) {
    AddPointResult r;
    if (!in.replace) {
        if (static_cast<int>(points.size()) - in.cursor_18 > 7) {   // 0071FE5x
            r.rejected = true;
            return r;
        }
    } else {
        points.clear();                                             // 004C82B0 erase(begin, end)
    }
    const Vec3f first = points.empty() ? Vec3f{} : points.front(); // 0059CAE0(0), else 00F87574
    clip_points(points);                                            // 007B1EC0
    const Vec3f* push = &point;
    std::array<Vec3f, 2> clipped{};
    if (outside_map(point) && points.size() < 7) {                 // 0071C4F0
        Vec3f from;
        if (!in.replace && !points.empty()) from = points.back();
        else if (in.owner_position) from = *in.owner_position;
        else from = first;
        clipped = clip_segment(from, point);                        // 004BBDD0
        points.push_back(clipped[0]);
        push = &clipped[1];
    }
    points.push_back(*push);                                        // 004215D0
    prune(points, in.cursor_18);
    if (in.replace) r.issued_moveonpath = true;
    if (in.session_mode != 2) r.sent_message_57 = true;
    return r;
}

int nearest_unit_index_0071db90(const Vec3f& pos, const std::vector<Vec3f>& units) noexcept {
    int best = -1;
    float best_d = kFltMax_00d7a248;
    for (std::size_t i = 0; i < units.size(); ++i) {
        const float dx = pos.x - units[i].x;
        const float dz = pos.z - units[i].z;
        const float d = dz * dz + dx * dx;
        if (best < 0 || d < best_d) {
            best = static_cast<int>(i);
            best_d = d;
        }
    }
    return best;
}

// 00720FA0..00720FD6: MOVZX the word, CMP EAX,[00F89A10] / JGE (signed), then
// SUB the table's base id, SHL 4 and MOV EAX,[entry + table + 0Ch].
void* entity_from_message_00720fa0(std::uint16_t index_20, const ObjectHandleTables& tables) noexcept {
    const std::uint32_t id = index_20;
    std::uint32_t index;
    const void* entries;
    if (static_cast<std::int32_t>(id) < tables.first_end_00f89a10) {
        index = id - static_cast<std::uint32_t>(tables.first_begin_00f89a0c);
        entries = tables.first_entries_00f89a54;
    } else {
        index = id - static_cast<std::uint32_t>(tables.second_begin_00f89a60);
        entries = tables.second_entries_00f89aa8;
    }
    const std::uint32_t address = reinterpret_cast<std::uint32_t>(entries) + (index << 4) + 0xc;
    void* entity;
    std::memcpy(&entity, reinterpret_cast<const void*>(address), sizeof(entity));
    return entity;
}

// 00720FE0..00721021: TEST AX,AX on the word / JZ -> XOR EAX,EAX; otherwise
// the same two-table read as 00720FA0 (inlined in the image, not a call).
void* entity_from_message_nonzero_00720fe0(std::uint16_t index_20, const ObjectHandleTables& tables) noexcept {
    if (index_20 == 0) return nullptr;
    return entity_from_message_00720fa0(index_20, tables);
}

CommandTargetDescriptor descriptor_init_00721070(std::uint32_t ordinal_00e08f8c) noexcept {
    CommandTargetDescriptor d;
    d.class_ordinal_00 = ordinal_00e08f8c;
    return d;
}

CommandTargetDescriptor descriptor_build_00721090(std::optional<std::uint32_t> class_ordinal,
                                                  const CommandTarget& target,
                                                  std::uint16_t resolved_index_174) noexcept {
    CommandTargetDescriptor d;
    d.class_ordinal_00 = class_ordinal.value_or(0u);
    d.has_entity_04 = target.has_entity_00;
    d.entity_index_06 = target.has_entity_00 ? resolved_index_174 : 0;
    d.has_point_08 = target.has_point_01;
    d.point_0c = target.has_point_01 ? target.point_08 : Vec3f{};   // else 00F87574
    return d;
}

void descriptor_write_00721100(const CommandTargetDescriptor& d, BitSink& out) {
    out.write_u32_bits_00429070(d.class_ordinal_00, 5);
    out.write_bool_004290b0(d.has_entity_04);
    if (d.has_entity_04) out.write_word_bits_00429120(d.entity_index_06, 0xD);
    out.write_bool_004290b0(d.has_point_08);
    if (d.has_point_08) {
        out.write_numeric_float_004295c0(d.point_0c.x, 0, 1, kFltMax_00d7a248, 0x20);
        out.write_numeric_float_004295c0(d.point_0c.y, 0, 1, kFltMax_00d7a248, 0x20);
        out.write_numeric_float_004295c0(d.point_0c.z, 0, 1, kFltMax_00d7a248, 0x20);
    }
}

void descriptor_read_007211b0(CommandTargetDescriptor& d, BitSource& in) {
    d.class_ordinal_00 = in.read_u32_bits_00428d10(5);
    d.has_entity_04 = in.read_bool_00428d70();
    if (d.has_entity_04) d.entity_index_06 = in.read_word_bits_00428e30(0xD);
    d.has_point_08 = in.read_bool_00428d70();
    if (d.has_point_08) {
        d.point_0c.x = in.read_numeric_float_004293f0(0, 1, kFltMax_00d7a248, 0x20);
        d.point_0c.y = in.read_numeric_float_004293f0(0, 1, kFltMax_00d7a248, 0x20);
        d.point_0c.z = in.read_numeric_float_004293f0(0, 1, kFltMax_00d7a248, 0x20);
    }
}

ControllerStateMessage state_message_build_00721280(bool byte_3c, bool byte_3d,
                                                    std::uint32_t flags_1cc, std::uint32_t mode_30,
                                                    const CommandTargetDescriptor& primary,
                                                    const CommandTargetDescriptor& override_) noexcept {
    ControllerStateMessage m;
    m.byte_3c = byte_3c;
    m.byte_3d = byte_3d;
    m.flags_1cc = flags_1cc;
    m.mode_30 = mode_30;
    m.primary = primary;
    m.override_ = override_;
    return m;
}

ControllerSnapshotMessage snapshot_build_007214c0(const ControllerStateMessage& state,
                                                  const DirectorFlags& d, bool getter_008356c0,
                                                  float getter_008356b0, float getter_008356f0) noexcept {
    ControllerSnapshotMessage m;
    m.state = state;
    m.enables_40 = d.enables_220;
    m.avoidance_44 = d.avoidance_240;
    m.flag_47 = getter_008356c0;
    m.value_48 = getter_008356b0;
    m.value_4c = getter_008356f0;
    return m;
}

void state_body_write_007212e0(const ControllerStateMessage& m, BitSink& out) {
    out.write_bool_004290b0(m.byte_3c);
    out.write_bool_004290b0(m.byte_3d);
    out.write_u32_bits_00429070(m.flags_1cc, 9);
    out.write_u32_bits_00429070(m.mode_30, 2);
    if (m.mode_30 != 0) {
        descriptor_write_00721100(m.primary, out);
        if (m.mode_30 != 1) descriptor_write_00721100(m.override_, out);
    }
}

void state_body_read_00721340(ControllerStateMessage& m, BitSource& in) {
    m.byte_3c = in.read_bool_00428d70();
    m.byte_3d = in.read_bool_00428d70();
    m.flags_1cc = in.read_u32_bits_00428d10(9);
    m.mode_30 = in.read_u32_bits_00428d10(2);
    if (m.mode_30 != 0) {
        descriptor_read_007211b0(m.primary, in);
        if (m.mode_30 != 1) descriptor_read_007211b0(m.override_, in);
    }
}

void snapshot_write_00721550(const ControllerSnapshotMessage& m, BitSink& out) {
    state_body_write_007212e0(m.state, out);
    for (const bool b : m.enables_40) out.write_bool_004290b0(b);
    for (const bool b : m.avoidance_44) out.write_bool_004290b0(b);
    out.write_bool_004290b0(m.flag_47);
    out.write_numeric_float_004295c0(m.value_48, 1, 1, m.flag_47 ? kPi_00d7a264 : 1.0f, 10);
    out.write_numeric_float_004295c0(m.value_4c, 1, 1, 1.0f, 10);
}

void snapshot_read_00721610(ControllerSnapshotMessage& m, BitSource& in) {
    state_body_read_00721340(m.state, in);
    for (bool& b : m.enables_40) b = in.read_bool_00428d70();
    for (bool& b : m.avoidance_44) b = in.read_bool_00428d70();
    m.flag_47 = in.read_bool_00428d70();
    m.value_48 = in.read_numeric_float_004293f0(1, 1, m.flag_47 ? kPi_00d7a264 : 1.0f, 10);
    m.value_4c = in.read_numeric_float_004293f0(1, 1, 1.0f, 10);
}

void snapshot_apply_extension_007219c0(const ControllerSnapshotMessage& m, DirectorFlags& d) noexcept {
    d.enables_220 = m.enables_40;
    d.avoidance_240 = m.avoidance_44;
    d.snapshot_243 = m.flag_47;
    d.snapshot_244 = m.value_48;
    d.snapshot_248 = m.value_4c;
}

void snapshot_apply_enables_00721980(const ControllerSnapshotMessage& m, DirectorFlags& d) noexcept {
    d.enables_220 = m.enables_40;
}

StateApplyPlan state_apply_00721890(const ControllerStateMessage& m) noexcept {
    StateApplyPlan p;
    if (m.mode_30 != 0) {
        p.issue_primary = true;
        p.primary_ordinal = m.primary.class_ordinal_00;
        if (m.mode_30 == 2) {
            p.set_override = true;
            p.override_ordinal = m.override_.class_ordinal_00;
        }
    }
    p.byte_3c = m.byte_3c;
    p.byte_3d = m.byte_3d;
    p.flags_1cc = m.flags_1cc;
    return p;
}

std::optional<std::size_t> command_from_ordinal_007216d0(std::uint8_t ordinal,
                                                         const std::vector<std::uint32_t>& class_ordinals) noexcept {
    if (ordinal == 0xFF) return std::nullopt;
    for (std::size_t i = 0; i < class_ordinals.size(); ++i) {
        if (class_ordinals[i] == ordinal) return i;
    }
    return std::nullopt;
}

CommandTarget command_record_build_00721760(std::uint32_t entity, std::uint16_t entity_index_174,
                                            const Vec3f& point, std::uint32_t param) noexcept {
    CommandTarget t;
    t.has_point_01 = true;
    t.point_08 = point;
    t.entity_04 = entity;
    t.param_14 = param;
    if (entity != 0) {
        t.has_entity_00 = true;
        t.entity_index_02 = entity_index_174;
    }
    return t;
}

CommandTarget descriptor_to_target_007217c0(
    const CommandTargetDescriptor& d, std::uint32_t entity, std::uint16_t entity_index_174,
    const std::function<CommandTarget(std::uint32_t)>& from_entity_00465080) {
    if (!d.has_entity_04) {
        CommandTarget t;
        if (d.has_point_08) {
            t.has_point_01 = true;
            t.point_08 = d.point_0c;
        }
        return t;
    }
    if (!d.has_point_08) return from_entity_00465080(entity);
    return command_record_build_00721760(entity, entity_index_174, d.point_0c, 0);
}

int vector_size6_00721ed0(std::uint32_t begin, std::uint32_t end) noexcept {
    if (begin == 0) return 0;
    return static_cast<int>(end - begin) / 6;
}

std::optional<Aabb> segment_walk_box_00722ca0(const std::vector<Vec3f>& a_20,
                                              const std::vector<Vec3f>& b_24,
                                              const std::array<float, 12>& m,
                                              const std::function<Vec3f(const Vec3f&)>& transform_point) noexcept {
    const std::size_t n = a_20.size() < b_24.size() ? a_20.size() : b_24.size();
    std::size_t i = 0;
    while (i < n && b_24[i].x < a_20[i].x) ++i;
    if (i >= n) return std::nullopt;
    const Vec3f& lo = a_20[i];
    const Vec3f& hi = b_24[i];
    const float half = 0.5f;   // 00D7A280, double 0.5
    const Vec3f centre{(hi.x + lo.x) * half, (hi.y + lo.y) * half, (hi.z + lo.z) * half};
    const float hx = (hi.x - lo.x) * half;
    const float hy = (hi.y - lo.y) * half;
    const float hz = half * (hi.z - lo.z);
    const float ex = hx * std::fabs(m[0]) + 0.0f + hy * std::fabs(m[4]) + hz * std::fabs(m[8]);
    const float ey = std::fabs(m[9]) * hz + std::fabs(m[5]) * hy + std::fabs(m[1]) * hx + 0.0f;
    const float ez = hz * std::fabs(m[10]) + hy * std::fabs(m[6]) + hx * std::fabs(m[2]) + 0.0f;
    const Vec3f c = transform_point(centre);
    Aabb box;
    box.lo = Vec3f{c.x - ex, c.y - ey, c.z - ez};
    box.hi = Vec3f{c.x + ex, ey + c.y, c.z + ez};
    return box;
}

}  // namespace gameplay_seg38_cc13
}  // namespace bsp
