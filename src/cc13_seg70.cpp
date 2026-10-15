// Segment 70 reconstruction pass (packet cc13-seg70). See include/bsp/cc13_seg70.hpp and
// docs/CC13_SEG70.md. Every routine names its address, ABI and evidence; offsets are the image's.
// The _SECURE_SCL checked-iterator reports (CALL 00BF6713 on an invalid iterator) that the image
// carries in its list walks are not modelled: they fire only on a corrupted list.

#include "bsp/cc13_seg70.hpp"

#include <cstring>

namespace bsp::cc13_seg70 {
namespace {

// A std::list node: next +0h, prev +4h, value +8h.
void* node_next(void* node) { return Raw{static_cast<std::uint8_t*>(node)}.ptr(0x0); }
void* node_value(void* node) { return Raw{static_cast<std::uint8_t*>(node)}.ptr(0x8); }

// The world list entry: next +4h, value +8h.
void* world_list(Seg70Host& host, std::size_t offset) {
    Raw game{static_cast<std::uint8_t*>(host.global(kGameGlobal).ptr(0))};
    Raw lists{static_cast<std::uint8_t*>(game.ptr(0x19CC))};
    return lists.ptr(offset);
}

// The tail the image inlines after an insert: head->prev = node; node->prev->next = node.
void link_before_head(void* head, void* node) {
    Raw h{static_cast<std::uint8_t*>(head)};
    Raw n{static_cast<std::uint8_t*>(node)};
    h.ptr(0x4) = node;
    Raw prev{static_cast<std::uint8_t*>(n.ptr(0x4))};
    prev.ptr(0x0) = node;
}

// The release of a pooled string: 00419CC0 then 00BD1510(data, length + 1, 1), only when data is set.
void release_string(Seg70Host& host, NativeStringRaw& s) {
    if (s.data != nullptr) host.pool_return_00bd1510(s.data, s.length + 1);
}

// The literal key the image builds before each Lua store: {0, 0}, 0041DD40(len, 1), then
// memcpy(data, literal, length + 1) when the data pointer is set.
void build_key(Seg70Host& host, NativeStringRaw& s, const char* literal, std::uint32_t length) {
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(s, length, 1);
    if (s.data != nullptr) host.memcpy_00bf7680(s.data, literal, s.length + 1);
}

// The four-float colour the stripped debug passes initialise once (guard bit 0), each from the
// 1.0f at 00D7A24C.
void init_debug_colour(Seg70Host& host, std::uint32_t guard, std::uint32_t colour) {
    Raw g = host.global(guard);
    if ((g.at<std::uint32_t>(0) & 1u) != 0) return;
    g.at<std::uint32_t>(0) |= 1u;
    Raw c = host.global(colour);
    for (std::size_t i = 0; i < 4; ++i) c.at<float>(i * 4) = kOne;
}

// The inlined world-matrix refresh of 00A16CC0 / 00A25CB0 (entity +C8h clean byte): refresh the
// parent pose, multiply the local matrix by the parent's world matrix (or take the local matrix at
// the root), store the world matrix, then mark clean (+C8h = 1) and unchanged (+10Ch = 0).
// `inline_copy` is 00A25CB0's sixteen-dword store in place of 00A16CC0's 004134F0 call.
void refresh_world_matrix(Seg70Host& host, Raw entity, bool inline_copy) {
    if (entity.at<std::uint8_t>(kEntityPoseClean) != 0) return;
    void* parent = entity.ptr(kEntityParentPose);
    if (parent != nullptr) host.entity_pose_refresh_world_00414db0(parent);
    float product[16];
    const float* source;
    parent = entity.ptr(kEntityParentPose);
    if (parent == nullptr) {
        source = &entity.at<float>(kEntityLocalMatrix);
    } else {
        Raw p{static_cast<std::uint8_t*>(parent)};
        source = host.matrix_multiply_00413920(&entity.at<float>(kEntityLocalMatrix), product,
                                               &p.at<float>(0xCC));
    }
    if (inline_copy) {
        // 00A25E0A..00A25E93: sixteen FLD float / FSTP float pairs.
        for (std::size_t i = 0; i < 16; ++i) entity.at<float>(kEntityWorldMatrix + i * 4) = source[i];
    } else {
        host.matrix_copy_004134f0(&entity.at<float>(kEntityWorldMatrix), source);
    }
    entity.at<std::uint8_t>(kEntityPoseClean) = 1;
    entity.at<std::uint8_t>(kEntityPoseChanged) = 0;
}

// Delete every node of a local list and its head (the image's inline clear-and-free tail).
void destroy_local_list(Seg70Host& host, ListRaw& list) {
    Raw head{static_cast<std::uint8_t*>(list.head)};
    void* node = head.ptr(0x0);
    head.ptr(0x0) = list.head;
    head.ptr(0x4) = list.head;
    list.size = 0;
    while (node != list.head) {
        void* next = node_next(node);
        host.free_00bf65ac(node);
        node = next;
    }
    host.free_00bf65ac(list.head);
}

bool forced_rules_equal(const std::uint8_t* a, const std::uint8_t* b) {
    // 00A32500's identity test: [0], [1], [2] low byte, [4] low byte, +1Dh, +1Eh, and only when
    // [4] is set [5], [6] and [7] low byte. The weight +0Ch is not compared.
    auto d = [](const std::uint8_t* p, std::size_t off) {
        std::uint32_t v;
        std::memcpy(&v, p + off, 4);
        return v;
    };
    if (d(a, 0x00) != d(b, 0x00)) return false;
    if (d(a, 0x04) != d(b, 0x04)) return false;
    if (a[0x08] != b[0x08]) return false;
    if (a[0x10] != b[0x10]) return false;
    if (a[0x1D] != b[0x1D]) return false;
    if (a[0x1E] != b[0x1E]) return false;
    if (a[0x10] == 0) return true;
    return d(a, 0x14) == d(b, 0x14) && d(a, 0x18) == d(b, 0x18) && a[0x1C] == b[0x1C];
}

// One forced-rule table {data, count, capacity}: remove an identical rule, grow at capacity to
// 2n+2 (copying with the placement-new null test), append, then 00A038C0.
void forced_rule_table_insert(Seg70Host& host, Raw table, const std::uint8_t* rule) {
    auto* data = static_cast<std::uint8_t*>(table.ptr(0));
    std::uint32_t& count = table.at<std::uint32_t>(4);
    std::uint32_t& capacity = table.at<std::uint32_t>(8);
    std::uint8_t* const end = data + count * kForcedRuleSize;
    for (std::uint8_t* p = data; p != end; p += kForcedRuleSize) {
        if (!forced_rules_equal(p, rule)) continue;
        std::uint8_t* const last = data + count * kForcedRuleSize - kForcedRuleSize;
        while (p != last) {
            std::memcpy(p, p + kForcedRuleSize, kForcedRuleSize);
            p += kForcedRuleSize;
        }
        --count;
        break;
    }
    if (count == capacity) {
        capacity = capacity * 2 + 2;
        auto* grown = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * kForcedRuleSize));
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint8_t* slot = grown + i * kForcedRuleSize;
            if (slot != nullptr) std::memcpy(slot, data + i * kForcedRuleSize, kForcedRuleSize);
        }
        if (data != nullptr) host.free_00bf6989(data);
        data = grown;
    }
    table.ptr(0) = data;
    std::uint8_t* slot = data + count * kForcedRuleSize;
    if (slot != nullptr) std::memcpy(slot, rule, kForcedRuleSize);
    ++count;
    host.forced_target_weight_clear_00a038c0();
}

// The dword-array reallocation shared by 00A31B10 / 00A32200 / 00A32880: the byte count is n*4
// with MUL / SETO, so an overflowing request asks operator new for FFFFFFFFh.
void dword_array_reallocate(Seg70Host& host, ArrayRaw& array, std::uint32_t capacity) {
    array.capacity = capacity;
    const std::uint64_t bytes = static_cast<std::uint64_t>(capacity) * 4u;
    const std::uint32_t request = (bytes >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
    auto* grown = static_cast<std::uint32_t*>(host.operator_new_00bf55be(request));
    if (array.data != nullptr) {
        auto* old = static_cast<std::uint32_t*>(array.data);
        for (std::uint32_t i = 0; i < array.count; ++i) grown[i] = old[i];
        host.free_00bf6989(array.data);
    }
    array.data = grown;
}

// The 20h-element growth shared by 00A31A40 and 00A32120: capacity 2n+2 with no comparison,
// the copy skipped per element when operator new answered null.
void array32_reallocate(Seg70Host& host, ArrayRaw& array) {
    array.capacity = array.capacity * 2 + 2;
    auto* grown = static_cast<std::uint8_t*>(host.operator_new_00bf55be(array.capacity << 5));
    for (std::uint32_t i = 0; i < array.count; ++i) {
        std::uint8_t* slot = grown + i * 0x20u;
        if (slot != nullptr) std::memcpy(slot, static_cast<std::uint8_t*>(array.data) + i * 0x20u, 0x20);
    }
    if (array.data != nullptr) host.free_00bf6989(array.data);
    array.data = grown;
}

struct PlannerKindRow {
    std::uint32_t construct;
    std::uint32_t vtable;
    const char* name;       // the literal's text
    std::uint32_t name_at;  // the literal's address
    std::uint32_t length;   // the 0041DD40 length the constructor pushes
};
constexpr PlannerKindRow kPlannerKinds[] = {
    {0x00A1EFC0u, 0x00D22D44u, "Defend", 0x00D22D3Cu, 6},
    {0x00A1F0F0u, 0x00D22D94u, "Attack", 0x00D22D8Cu, 6},
    {0x00A1F220u, 0x00D22DE4u, "Sell", 0x00D22DDCu, 4},
    {0x00A1F390u, 0x00D22E34u, "Capture", 0x00D22E2Cu, 7},
    {0x00A1F500u, 0x00D22E7Cu, "Duel", 0x00CE73BCu, 4},
    {0x00A1F630u, 0x00D22EC4u, "Escort", 0x00CE73B4u, 6},
    {0x00A1F760u, 0x00D22F0Cu, "Siege", 0x00CE73ACu, 5},
    {0x00A1F890u, 0x00D22F54u, "Competitive", 0x00CE73A0u, 11},
};
const PlannerKindRow& row_of(PlannerConstructKind kind) {
    return kPlannerKinds[static_cast<std::size_t>(kind)];
}

}  // namespace

// ---------------------------------------------------------------------------
// Party brains.
// ---------------------------------------------------------------------------

void party_brain_destruct_body_00a16490(Seg70Host& host, Raw brain) {
    // Eight unrolled slots: a non-null planner gets vtable[0](1), then the slot is cleared.
    for (std::size_t i = 0; i < 8; ++i) {
        void* planner = brain.ptr(i * 4);
        if (planner != nullptr) {
            host.scalar_deleting_destructor_0(planner, 1);
            brain.ptr(i * 4) = nullptr;
        }
    }
}

void party_brain_delete_00a16990(Seg70Host& host, void** slot) {
    void* brain = *slot;
    if (brain == nullptr) return;
    party_brain_destruct_body_00a16490(host, Raw{static_cast<std::uint8_t*>(brain)});
    host.free_00bf65ac(brain);
    *slot = nullptr;
}

void destroy_party_brains_00a16b20(Seg70Host& host) {
    // EDI walks 00F8A89C..<00F8A8BC by 4.
    for (std::uint32_t at = kPartyBrainSlots; at < kPartyBrainSlotsEnd; at += 4) {
        Raw slot = host.global(at);
        void* brain = slot.ptr(0);
        if (brain != nullptr) {
            party_brain_destruct_body_00a16490(host, Raw{static_cast<std::uint8_t*>(brain)});
            host.free_00bf65ac(brain);
            slot.ptr(0) = nullptr;
        }
    }
}

float party_aggressive_ratio_00a184f0(Seg70Host& host, int party) {
    // LEA EAX,[ECX*8]; SUB EAX,ECX; FLD [EAX*4 + 00F8A8D0].
    const std::uint32_t at = kPartyAggressiveRatio + static_cast<std::uint32_t>(party * 7) * 4u;
    return host.global(at).at<float>(0);
}

// ---------------------------------------------------------------------------
// Planner objects.
// ---------------------------------------------------------------------------

void* planner_construct_base_00a1ee50(Seg70Host& host, Raw planner, Raw brain) {
    planner.at<std::uint32_t>(0x04) = 0;
    planner.at<std::uint32_t>(0x08) = 0;
    planner.at<std::uint32_t>(0x0C) = 0;
    planner.at<std::uint8_t>(0x10) = 0;
    planner.at<std::uint32_t>(0x00) = kPlannerBaseVtable;
    planner.at<std::uint32_t>(kPlannerName) = 0;
    planner.at<std::uint32_t>(kPlannerName + 4) = 0;
    planner.ptr(kPlannerGroupHead) = host.list_buy_head_node_00a168a0(&planner.at<std::uint8_t>(kPlannerGroups));
    planner.at<std::uint32_t>(kPlannerGroupCount) = 0;
    const int side = brain.at<int>(kBrainSide);
    planner.at<int>(kPlannerSide) = side;
    planner.ptr(kPlannerBrain) = brain.p;
    planner.at<std::uint8_t>(kPlannerChanged) = 0;
    planner.at<std::uint32_t>(kPlannerEnemySide) = side == 0 ? 1u : 0u;   // SETZ DL
    return planner.p;
}

std::uint32_t planner_construct_address(PlannerConstructKind kind) noexcept { return row_of(kind).construct; }
std::uint32_t planner_vtable(PlannerConstructKind kind) noexcept { return row_of(kind).vtable; }
const char* planner_literal_name(PlannerConstructKind kind) noexcept { return row_of(kind).name; }

void* planner_construct_00a1efc0_family(Seg70Host& host, PlannerConstructKind kind, Raw planner, Raw brain) {
    const PlannerKindRow& row = row_of(kind);
    planner_construct_base_00a1ee50(host, planner, brain);
    planner.at<std::uint32_t>(0) = row.vtable;
    // A local string takes the literal (0041DD40(len, 1), memcpy of length + 1)...
    NativeStringRaw local{0, nullptr};
    host.native_string_resize_0041dd40(local, row.length, 1);
    if (local.data != nullptr) host.memcpy_00bf7680(local.data, row.name, local.length + 1);
    // ...then planner+14h is resized to it and, for a non-empty one, given `length` bytes of it.
    auto& name = reinterpret_cast<NativeStringRaw&>(planner.at<std::uint32_t>(kPlannerName));
    if (&name != &local) {
        host.native_string_resize_0041dd40(name, local.length, 1);
        if (local.length != 0) host.memcpy_00bf7680(name.data, local.data, name.length);
    }
    if (local.data != nullptr) host.pool_return_00bd1510(local.data, local.length + 1);
    if (kind == PlannerConstructKind::Capture) {
        // 00A1F445..00A1F45E: XORPS / MOVSS.
        planner.at<float>(0x38) = 0.0f;
        planner.at<float>(0x3C) = 0.0f;
        planner.at<float>(0x40) = kMinusOne;
    }
    return planner.p;
}

void planner_sell_release_group_00a1f310(Seg70Host& host, Raw planner, void* group) {
    if (!host.planner_owns_group_00a1c8b0(planner.p, group)) return;
    host.observer_unregister_pair_006952a0(group, planner.p);   // ECX = group, EDX = planner
    void* const value = group;
    host.list_remove_00a1d1d0(&planner.at<std::uint8_t>(kPlannerGroups), &value);
    Raw{static_cast<std::uint8_t*>(group)}.at<std::uint32_t>(kGroupOwnerPlanner) = 0;
}

void planner_split_groups_by_class_00a22920(Seg70Host& host, Raw planner) {
    if (planner.at<std::uint32_t>(kPlannerGroupCount) == 0) return;
    // The new groups, a std::list<group*> on the stack (00A168A0 / 00A16AB0 / 00A172A0).
    ListRaw made_list{0, nullptr, 0};
    made_list.head = host.list_buy_head_node_00a168a0(&made_list);
    made_list.size = 0;
    void* const planner_head = planner.ptr(kPlannerGroupHead);
    for (void* node = node_next(planner_head); node != planner.ptr(kPlannerGroupHead); node = node_next(node)) {
        Raw group{static_cast<std::uint8_t*>(node_value(node))};
        int remaining = group.at<int>(kGroupPopulation);
        if (remaining < 2) continue;
        // Three 128-entry stack arrays (ESP+44h classes, +244h counts, +444h groups). The image
        // has no bound on any of them; neither does this.
        void* classes[128];
        int counts[128];
        void* made[128];
        int class_count = 0;    // ESP+40h
        int groups_made = 0;    // ESP+3Ch
        // The members put back into this group afterwards (004C1630 / 004C2220 / 004CEE30).
        ListRaw keep{0, nullptr, 0};
        keep.head = host.list_buy_head_node_004c1630(&keep);
        keep.size = 0;
        while (remaining > 0) {
            void* member = node_value(node_next(group.ptr(kGroupMemberHead)));
            void* ship = (member != nullptr && host.is_kind_of_5c(member, 5)) ? member : nullptr;
            --remaining;
            group_remove_entity_00a2d9d0(host, group, member, 0);
            bool keep_member = true;
            if (ship != nullptr) {
                void* cls = Raw{static_cast<std::uint8_t*>(ship)}.ptr(kShipClass);
                int index = 0;
                while (index < class_count && classes[index] != cls) ++index;
                if (index == class_count) {
                    // First member of a class: remembered, and it stays in the group.
                    classes[index] = cls;
                    counts[index] = 0;
                    ++class_count;
                } else {
                    keep_member = false;
                    const int seen = counts[index];
                    counts[index] = seen + 1;
                    if (seen != groups_made) {
                        // 00A22B78: added to the group made at index `seen`, whatever its class.
                        host.group_add_entity_00a2d8e0(made[seen], ship);
                    } else {
                        void* block = host.operator_new_00bf681b(kGroupSize);
                        void* fresh = block != nullptr ? host.group_construct_00a2dfa0(block, ship) : nullptr;
                        group_copy_command_from_00a2bd50(host, Raw{static_cast<std::uint8_t*>(fresh)}, group);
                        made[seen] = fresh;
                        void* const value = fresh;
                        void* made_head = made_list.head;
                        void* inserted = host.list_buy_node_00a16ab0(
                            &made_list, made_head, Raw{static_cast<std::uint8_t*>(made_head)}.ptr(4), &value);
                        host.list_incsize_00a172a0(&made_list, 1);
                        ++groups_made;
                        link_before_head(made_head, inserted);
                    }
                }
            }
            if (keep_member) {
                void* const value = member;
                void* keep_head = keep.head;
                void* inserted = host.list_buy_node_004c2220(
                    &keep, keep_head, Raw{static_cast<std::uint8_t*>(keep_head)}.ptr(4), &value);
                host.list_incsize_004cee30(&keep, 1);
                link_before_head(keep_head, inserted);
            }
        }
        for (void* k = node_next(keep.head); k != keep.head; k = node_next(k)) {
            host.group_add_entity_00a2d8e0(group.p, node_value(k));
        }
        destroy_local_list(host, keep);
    }
    if (made_list.size != 0) {
        for (void* node = node_next(made_list.head); node != made_list.head; node = node_next(node)) {
            void* fresh = node_value(node);
            (void)host.group_any_member_matches_00a2c600(fresh);   // 00A22C49: the answer is unused
            if (host.planner_owns_group_00a1c8b0(planner.p, fresh)) continue;
            void* const value = fresh;
            void* head = planner.ptr(kPlannerGroupHead);
            void* inserted = host.list_buy_node_00a16ab0(&planner.at<std::uint8_t>(kPlannerGroups), head,
                                                         Raw{static_cast<std::uint8_t*>(head)}.ptr(4), &value);
            host.list_incsize_00a172a0(&planner.at<std::uint8_t>(kPlannerGroups), 1);
            link_before_head(head, inserted);
            Raw{static_cast<std::uint8_t*>(fresh)}.ptr(kGroupOwnerPlanner) = planner.p;
            host.observer_register_pair_00694a60(fresh, planner.p);
            planner.at<std::uint8_t>(kPlannerChanged) = 1;
        }
    }
    destroy_local_list(host, made_list);
}

void planner_quick_spawn_00a25b90(Seg70Host& host, Raw planner, std::uint32_t name, std::uint8_t flag,
                                  std::uint32_t a, std::uint32_t b) {
    if (host.global(kSpawnDisabled).at<std::uint8_t>(0) != 0) return;
    Raw brain{static_cast<std::uint8_t*>(planner.ptr(kPlannerBrain))};
    void* manager = host.global(kSpawnRequestManager).ptr(0);
    const float queued = host.spawn_queued_requests_00946970(manager, brain.at<std::uint32_t>(kBrainSpawnKey));
    // FLDZ / FUCOMIP / TEST AH,44h / JP: only an exact 0.0 continues (a NaN returns too).
    if (!(queued == 0.0f)) return;
    VectorRaw candidates{0, nullptr, nullptr, nullptr};
    host.collect_spawn_candidates_00a08420(planner.at<int>(kPlannerEnemySide), flag, a, &candidates, 0, b);
    const std::uint32_t party = host.global(kCurrentPartyIndex).at<std::uint32_t>(0);
    const std::uint32_t record = party * static_cast<std::uint32_t>(kPartyRecordStride);
    const float* position = nullptr;
    if (host.global(kPartyHasSpawnPoint + record).at<std::uint8_t>(0) != 0) {
        position = &host.global(kPartySpawnPoint + record).at<float>(0);
    }
    host.planner_try_spawn_group_00a25a30(planner.p, position, kOne, &candidates, name, 1);   // FLD1
    if (candidates.first != nullptr) host.free_00bf65ac(candidates.first);
}

void debug_capture_scores_stripped_00a16cc0(Seg70Host& host) {
    const int team = host.global(kDebugTeam).at<int>(0);
    for (void* entry = world_list(host, 0x16C); entry != nullptr;
         entry = Raw{static_cast<std::uint8_t*>(entry)}.ptr(4)) {
        Raw entity{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(entry)}.ptr(8))};
        NativeStringRaw text{0, nullptr};
        // ECX = team, EDX = entity, (out = 0, explain = &text); the ST0 result is popped.
        if (entity.at<int>(kEntitySide) == team) {
            (void)host.planner_strategic_gain_00a243d0(team, entity.p, nullptr, &text);
        } else {
            (void)host.planner_capture_target_score_00a1e250(team, entity.p, nullptr, &text);
        }
        init_debug_colour(host, kCaptureDebugColourGuard, kCaptureDebugColour);
        refresh_world_matrix(host, entity, false);
        release_string(host, text);
    }
}

void planner_capture_debug_draw_stripped_00a1fc30(Seg70Host& host, Raw planner) {
    for (void* entry = world_list(host, 0x16C); entry != nullptr;
         entry = Raw{static_cast<std::uint8_t*>(entry)}.ptr(4)) {
        Raw entity{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(entry)}.ptr(8))};
        const int side = Raw{static_cast<std::uint8_t*>(planner.ptr(kPlannerBrain))}.at<int>(kBrainSide);
        if (entity.at<int>(kEntitySide) == side) continue;
        NativeStringRaw text{0, nullptr};
        (void)host.planner_capture_target_score_00a1e250(side, entity.p, nullptr, &text);
        init_debug_colour(host, kPlannerDebugColourGuard, kPlannerDebugColour);
        // 00A1FCDF: here the entity's own pose is refreshed through 00414DB0.
        if (entity.at<std::uint8_t>(kEntityPoseClean) == 0) host.entity_pose_refresh_world_00414db0(entity.p);
        release_string(host, text);
    }
}

void planner_defend_debug_draw_stripped_00a25cb0(Seg70Host& host, Raw planner) {
    const int team = Raw{static_cast<std::uint8_t*>(planner.ptr(kPlannerBrain))}.at<int>(kBrainSide);
    ListRaw items{0, nullptr, 0};
    items.head = host.list_buy_head_node_004c1630(&items);
    items.size = 0;
    for (void* node = node_next(planner.ptr(kPlannerGroupHead)); node != planner.ptr(kPlannerGroupHead);
         node = node_next(node)) {
        host.group_collect_members_in_world_set_00a2def0(node_value(node), team, &items);
    }
    for (void* node = node_next(items.head); node != items.head; node = node_next(node)) {
        Raw entity{static_cast<std::uint8_t*>(node_value(node))};
        NativeStringRaw text{0, nullptr};
        // ECX = planner+30h, EDX = entity, (0, &text).
        (void)host.planner_strategic_gain_00a243d0(planner.at<int>(kPlannerSide), entity.p, nullptr, &text);
        init_debug_colour(host, kPlannerDebugColourGuard, kPlannerDebugColour);
        refresh_world_matrix(host, entity, true);
        release_string(host, text);
    }
    destroy_local_list(host, items);
}

// ---------------------------------------------------------------------------
// AI group commands and membership.
// ---------------------------------------------------------------------------

void group_set_command_00a2bd00(Seg70Host& host, Raw group, void* command) {
    void* old = group.ptr(kGroupCommand);
    if (old != nullptr) host.scalar_deleting_destructor_0(old, 1);
    group.ptr(kGroupCommand) = command;
}

void group_copy_command_from_00a2bd50(Seg70Host& host, Raw group, Raw other) {
    // other+564Ch is called without a null test.
    void* clone = host.command_clone_for_group_20(other.ptr(kGroupCommand), group.p);
    void* old = group.ptr(kGroupCommand);
    if (old != nullptr) host.scalar_deleting_destructor_0(old, 1);
    group.ptr(kGroupCommand) = clone;
}

void group_forward_command_00a2bd90(Seg70Host& host, Raw group, std::uint32_t a, std::uint32_t b) {
    void* command = group.ptr(kGroupCommand);
    if (command != nullptr) host.command_forward_24(command, a, b);   // JMP [vtable+24h]
}

bool group_is_attacking_target_00a2bdb0(Seg70Host& host, Raw group, void* target_group) {
    if (!host.command_is_type_08(group.ptr(kGroupCommand), kCommandTypeAttack)) return false;
    return Raw{static_cast<std::uint8_t*>(group.ptr(kGroupCommand))}.ptr(0x1C) == target_group;
}

bool group_is_selling_00a2be10(Seg70Host& host, Raw group) {
    return host.command_is_type_08(group.ptr(kGroupCommand), kCommandTypeSelling);
}

void group_order_retreat_00a2be70(Seg70Host& host, Raw group) {
    if (host.command_is_type_08(group.ptr(kGroupCommand), kCommandTypeRetreat)) return;
    void* command = host.operator_new_00bf681b(8);
    if (command != nullptr) {
        Raw c{static_cast<std::uint8_t*>(command)};
        c.ptr(4) = group.p;
        c.at<std::uint32_t>(0) = kRetreatCommandVtable;
    }
    void* old = group.ptr(kGroupCommand);
    if (old != nullptr) host.scalar_deleting_destructor_0(old, 1);
    group.ptr(kGroupCommand) = command;
}

void group_order_selling_00a185e0(Seg70Host& host, Raw group) {
    if (group_is_selling_00a2be10(host, group)) return;
    void* command = host.operator_new_00bf681b(8);
    if (command != nullptr) {
        Raw c{static_cast<std::uint8_t*>(command)};
        c.ptr(4) = group.p;
        c.at<std::uint32_t>(0) = kSellingCommandVtable;
        group_set_command_00a2bd00(host, group, command);
        return;
    }
    group_set_command_00a2bd00(host, group, nullptr);
}

void group_order_regrouping_move_00a2c2a0(Seg70Host& host, Raw group, const float* point, std::uint32_t unused) {
    (void)unused;   // RET 8: the second argument is never read
    if (host.group_is_regrouping_to_00a2c1c0(group.p, point)) return;
    void* command = host.operator_new_00bf681b(0x14);
    if (command != nullptr) {
        Raw c{static_cast<std::uint8_t*>(command)};
        c.at<std::uint32_t>(0) = kMoveCommandBaseVtable;
        c.ptr(4) = group.p;
        c.at<float>(0x08) = point[0];
        c.at<float>(0x0C) = point[1];
        c.at<std::uint32_t>(0) = kRegroupingMoveVtable;
        c.at<float>(0x10) = point[2];
    }
    void* old = group.ptr(kGroupCommand);
    if (old != nullptr) host.scalar_deleting_destructor_0(old, 1);
    group.ptr(kGroupCommand) = command;
}

void group_remove_entity_00a2d9d0(Seg70Host& host, Raw group, void* entity, std::uint8_t queue_if_empty) {
    void* const value = entity;
    host.list_remove_0077bea0(&group.at<std::uint8_t>(kGroupMembers), &value);
    Raw e{static_cast<std::uint8_t*>(entity)};
    if (e.ptr(kEntityAiGroup) == group.p) e.ptr(kEntityAiGroup) = nullptr;
    host.observer_unregister_if_registered_006956a0(
        entity, group.p != nullptr ? &group.at<std::uint8_t>(kGroupObserver) : nullptr);
    if (group.at<std::uint32_t>(kGroupPopulation) != 0 || queue_if_empty == 0) return;
    // Append the emptied group to the list at 00F8AA78 (head [00F8AA7C]).
    Raw list = host.global(kEmptiedGroupList);
    void* head = list.ptr(4);
    void* const self = group.p;
    void* inserted = host.list_buy_node_00a16ab0(list.p, head, Raw{static_cast<std::uint8_t*>(head)}.ptr(4), &self);
    host.list_incsize_00a172a0(list.p, 1);
    link_before_head(head, inserted);
}

void group_fill_lua_info_00a2eee0(Seg70Host& host, Raw group, void* table) {
    if (group.at<std::uint32_t>(kGroupPopulation) == 0) return;
    NativeStringRaw key{0, nullptr};
    // "leader" (00CFD760): the first member's Lua self object.
    host.native_string_construct_0041e870(key, "leader");
    {
        void* leader = node_value(node_next(group.ptr(kGroupMemberHead)));
        LuaObjectRaw self;
        LuaObjectRaw* object = host.entity_lua_self_object_00927b40(leader, &self);
        host.lua_set_object_00b675d0(table, &key, object);
        host.lua_destruct_00b67700(&self);
    }
    release_string(host, key);
    // "members" (00D230CC): a new table, then one entry per member from index 1.
    host.native_string_construct_0041e870(key, "members");
    host.lua_set_new_table_00b67580(table, &key);
    release_string(host, key);
    LuaObjectRaw members;
    host.lua_get_by_name_00b67800(table, &members, "members");
    {
        int index = 1;
        for (void* node = node_next(group.ptr(kGroupMemberHead)); node != group.ptr(kGroupMemberHead);
             node = node_next(node), ++index) {
            LuaObjectRaw self;
            LuaObjectRaw* object = host.entity_lua_self_object_00927b40(node_value(node), &self);
            host.lua_set_object_at_index_00b666c0(&members, index, object);
            host.lua_destruct_00b67700(&self);
        }
    }
    // "autoGrouping" (00D230BC), the +5648h byte, MOVZX.
    build_key(host, key, "autoGrouping", 0x0C);
    host.lua_set_boolean_00b673a0(table, &key, group.at<std::uint8_t>(kGroupAutoGrouping));
    release_string(host, key);
    // "party" (00D14754) is the TEAM +5638h; "playerIndex" (00CF9D28) the party slot +5634h.
    build_key(host, key, "party", 5);
    host.lua_set_integer_00b67460(table, &key, group.at<std::int32_t>(kGroupTeam));
    release_string(host, key);
    build_key(host, key, "playerIndex", 0x0B);
    host.lua_set_integer_00b67460(table, &key, group.at<std::int32_t>(kGroupPartySlot));
    release_string(host, key);
    // "reconRatio" (00D230B0): 00A2C3D0's float through _ftol2 (00BF7420), so truncated to an int.
    build_key(host, key, "reconRatio", 0x0A);
    const float ratio = host.group_recon_ratio_00a2c3d0(group.p);
    host.lua_set_integer_00b67460(table, &key, static_cast<std::int32_t>(ratio));
    release_string(host, key);
    // "objectiveMembers" (00D2309C): the 00A2DEF0 members of the team's world set.
    build_key(host, key, "objectiveMembers", 0x10);
    host.lua_set_new_table_00b67580(table, &key);
    release_string(host, key);
    LuaObjectRaw objective;
    host.lua_get_by_name_00b67800(table, &objective, "objectiveMembers");
    ListRaw collected{0, nullptr, 0};
    collected.head = host.list_buy_head_node_004c1630(&collected);
    collected.size = 0;
    host.group_collect_members_in_world_set_00a2def0(group.p, group.at<int>(kGroupTeam), &collected);
    {
        int index = 1;
        for (void* node = node_next(collected.head); node != collected.head; node = node_next(node), ++index) {
            LuaObjectRaw self;
            LuaObjectRaw* object = host.entity_lua_self_object_00927b40(node_value(node), &self);
            host.lua_set_object_at_index_00b666c0(&objective, index, object);
            host.lua_destruct_00b67700(&self);
        }
    }
    // "unitTypes" (00D23090): name (00E0B590[key]) -> count over 00A2E1E0's map.
    build_key(host, key, "unitTypes", 9);
    host.lua_set_new_table_00b67580(table, &key);
    release_string(host, key);
    LuaObjectRaw types;
    host.lua_get_by_name_00b67800(table, &types, "unitTypes");
    TreeRaw counts{0, nullptr, 0};
    counts.head = host.tree_buy_head_node_005827f0(&counts);
    {
        Raw head{static_cast<std::uint8_t*>(counts.head)};
        head.at<std::uint8_t>(0x15) = 1;
        head.ptr(0x4) = counts.head;
        head.ptr(0x0) = counts.head;
        head.ptr(0x8) = counts.head;
    }
    counts.size = 0;
    host.group_count_unit_types_00a2e1e0(group.p, &counts);
    TreeIterRaw it{&counts, Raw{static_cast<std::uint8_t*>(counts.head)}.ptr(0)};
    while (it.node != counts.head) {
        Raw node{static_cast<std::uint8_t*>(it.node)};
        const std::uint32_t type = node.at<std::uint32_t>(0xC);
        const char* name = static_cast<const char*>(host.global(kUnitTypeNames + type * 4).ptr(0));
        key.length = 0;
        key.data = nullptr;
        host.native_string_resize_0041dd40(key, static_cast<std::uint32_t>(std::strlen(name)), 1);
        if (key.data != nullptr) host.memcpy_00bf7680(key.data, name, key.length + 1);
        host.lua_set_integer_00b67460(&types, &key, node.at<std::int32_t>(0x10));
        release_string(host, key);
        host.tree_iter_inc_0057f4a0(&it);
    }
    // "command" (00D186C4): a new table the command fills through its vtable+18h.
    build_key(host, key, "command", 7);
    host.lua_set_new_table_00b67580(table, &key);
    release_string(host, key);
    LuaObjectRaw command;
    host.lua_get_by_name_00b67800(table, &command, "command");
    host.command_fill_lua_18(group.ptr(kGroupCommand), &command);
    host.lua_destruct_00b67700(&command);
    // Teardown in the image's order: the map, the unit-type table, the collected list, the
    // objective table, the members table.
    TreeIterRaw erased{nullptr, nullptr};
    host.tree_erase_range_0058ad40(&counts, &erased, TreeIterRaw{&counts, Raw{static_cast<std::uint8_t*>(counts.head)}.ptr(0)},
                                   TreeIterRaw{&counts, counts.head});
    host.free_00bf65ac(counts.head);
    counts.head = nullptr;
    counts.size = 0;
    host.lua_destruct_00b67700(&types);
    destroy_local_list(host, collected);
    host.lua_destruct_00b67700(&objective);
    host.lua_destruct_00b67700(&members);
}

Raw debug_default_text_style_00a2c050(Seg70Host& host) {
    Raw guard = host.global(kDefaultTextStyleGuard);
    if ((guard.at<std::uint32_t>(0) & 1u) == 0) {
        guard.at<std::uint32_t>(0) |= 1u;
        Raw style = host.global(kDefaultTextStyle);
        style.at<float>(0x0) = 0.0f;
        style.at<float>(0x4) = kHalf;
        style.at<float>(0x8) = kOne;
        style.at<float>(0xC) = kOne;
    }
    return host.global(kDefaultTextStyle);
}

// ---------------------------------------------------------------------------
// The coordinator.
// ---------------------------------------------------------------------------

void* controller_tick_element_00a317c0(void* controller) {
    if (controller == nullptr) return nullptr;
    return static_cast<std::uint8_t*>(controller) + kControllerTickElement;
}

void coordinator_reset_singleton_00a31800(Seg70Host& host, Raw object) {
    host.global(kCoordinatorSingleton).ptr(0) = nullptr;
    object.at<std::uint32_t>(0) = kSingletonBaseVtable;
}

void* controller_message_construct_00a31c20(Seg70Host& host, Raw message, std::uint32_t value) {
    host.session_message_construct_base_0075b430(message.p, 0x4B);
    message.at<std::uint32_t>(0x04) = 1;
    message.at<std::uint8_t>(0x20) = 1;
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint8_t>(0x1C) = 0;
    message.at<std::uint32_t>(0x24) = 0;
    message.at<std::uint32_t>(0x2C) = 0;
    message.at<std::uint8_t>(0x30) = 0;
    message.at<std::uint32_t>(0x00) = 0x00D02C90;
    message.at<std::uint32_t>(0x28) = value;
    return message.p;
}

void controller_send_party_message_00a31c90(Seg70Host& host, Raw unit, std::uint32_t value) {
    if (host.is_kind_of_5c(unit.p, 0x1C)) {
        // An inline class-D3h message on the stack: vtable 00CFADBC, +4h = 1, +18h word 0, +1Ah 0,
        // +1Ch = value, +20h = 1.
        alignas(4) std::uint8_t storage[0x24] = {};
        Raw message{storage};
        host.session_message_construct_base_0075b430(message.p, 0xD3);
        message.at<std::uint16_t>(0x18) = 0;
        message.at<std::uint32_t>(0x04) = 1;
        message.at<std::uint8_t>(0x1A) = 0;
        message.at<std::uint32_t>(0x00) = 0x00CFADBC;
        message.at<std::uint32_t>(0x1C) = value;
        message.at<std::uint8_t>(0x20) = 1;
        host.unit_route_message_0077c2a0(unit.p, message.p, 7, 0);
    } else if (host.is_kind_of_5c(unit.p, 9)) {
        alignas(4) std::uint8_t storage[0x34] = {};
        void* message = controller_message_construct_00a31c20(host, Raw{storage}, value);
        host.unit_route_message_0077c2a0(unit.p, message, 7, 0);
    }
    unit.at<std::int32_t>(kUnitMessageSlot) = -1;
}

void controller_broadcast_party_state_00a329f0(Seg70Host& host) {
    Raw game{static_cast<std::uint8_t*>(host.global(kGameGlobal).ptr(0))};
    if (host.game_effective_game_mode_004bca50(game.p) > 3) return;
    // [ESP+10h] / [ESP+11h]: any AI party among slots 0..3 / 4..7; [ESP+1Ch + i]: player i present.
    std::uint8_t ai_bucket[2] = {0, 0};
    std::uint8_t present[8];
    for (int slot = 0; slot < 8; ++slot) {
        Raw player{static_cast<std::uint8_t*>(game.ptr(0x18CC + static_cast<std::size_t>(slot) * 4))};
        present[slot] = (player.at<std::uint8_t>(8) != 0 && player.at<std::uint8_t>(9) == 0) ? 1 : 0;
        const bool ai = host.party_ai_enabled_009ffe50(slot);
        if (slot < 4) ai_bucket[0] |= static_cast<std::uint8_t>(ai);
        else ai_bucket[1] |= static_cast<std::uint8_t>(ai);
    }
    const int ai_target = ai_bucket[0] != 0 ? 0 : 4;   // NEG / SBB / AND -4 / ADD 4
    ArrayRaw orphans{nullptr, 0, 0};
    for (void* entry = world_list(host, 0x58); entry != nullptr;
         entry = Raw{static_cast<std::uint8_t*>(entry)}.ptr(4)) {
        Raw unit{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(entry)}.ptr(8))};
        if (!host.is_kind_of_5c(unit.p, 9) && !host.is_kind_of_5c(unit.p, 0x1C)) continue;
        const int side = unit.at<int>(kEntitySide);
        if (side > 1) continue;                                 // CMP ECX,1 / JG (signed)
        const int slot = unit.at<int>(kEntityPlayerSlot);
        if (slot >= 8) continue;                                // CMP EAX,8 / JGE (signed)
        // LABELLED: a negative side or slot indexes the image's stack bytes below these arrays.
        // This model reads them as 0 (side) and as not present (slot). For slot -1..-3 that is the
        // image's value (the high bytes of the 0-or-4 target dword at [ESP+18h]); slot -4 reads
        // that dword's low byte (4 when slots 0..3 hold an AI party) and a negative side reads the
        // saved EBX, neither reproduced.
        const std::uint8_t bucket_ai = side >= 0 ? ai_bucket[side] : 0;
        if (bucket_ai != 0) {
            if (slot != ai_target) controller_send_party_message_00a31c90(host, unit, static_cast<std::uint32_t>(ai_target));
            continue;
        }
        const std::uint8_t is_present = slot >= 0 ? present[slot] : 0;
        if (is_present != 0) continue;
        // Inline push_back, growth 2n+2 only when larger (00A32B36..00A32BA9).
        if (orphans.count == orphans.capacity && orphans.capacity * 2 + 2 > orphans.capacity) {
            dword_array_reallocate(host, orphans, orphans.capacity * 2 + 2);
        }
        static_cast<void**>(orphans.data)[orphans.count] = unit.p;
        ++orphans.count;
    }
    if (orphans.count != 0) {
        // Round-robin over the present players of the human bucket (base 4 when slots 0..3 hold an
        // AI party, else 0), the cursor starting at 3. The image spins forever when no player of
        // that bucket is present; so does this.
        const int base = ai_bucket[0] != 0 ? 4 : 0;
        int cursor = 3;
        for (std::uint32_t i = 0; i < orphans.count; ++i) {
            do {
                ++cursor;
                if (cursor > 3) cursor = 0;
            } while (present[base + cursor] == 0);
            controller_send_party_message_00a31c90(
                host, Raw{static_cast<std::uint8_t*>(static_cast<void**>(orphans.data)[i])},
                static_cast<std::uint32_t>(base + cursor));
        }
    }
    if (orphans.data != nullptr) host.free_00bf6989(orphans.data);
}

void controller_destruct_body_00a32460(Seg70Host& host, Raw controller) {
    controller.at<std::uint32_t>(0x000) = 0x00D231A0;
    controller.at<std::uint32_t>(0x010) = 0x00D23188;
    controller.at<std::uint32_t>(0x024) = 0x00D23180;
    controller.at<std::uint32_t>(kControllerTickElement) = 0x00D23168;
    host.global(kCoordinatorLiveFlag).at<std::uint32_t>(0) = 0;
    destroy_party_brains_00a16b20(host);
    host.forced_target_weight_clear_00a038c0();
    Raw count = host.global(kGlobalForcedRuleCount);
    if (count.at<std::uint32_t>(0) != 0) count.at<std::uint32_t>(0) = 0;
    host.unit_tick_element_destruct_00875490(&controller.at<std::uint8_t>(kControllerTickElement));
    host.entity_destruct_base_00925780(controller.p);
}

void forced_target_weight_insert_00a32500(Seg70Host& host, const std::uint8_t* rule, bool global_table,
                                          int mode) {
    if (global_table) {
        forced_rule_table_insert(host, host.global(kGlobalForcedRules), rule);
        return;
    }
    if (mode < 0) mode = host.ai_effective_game_mode_index_009ffc80();
    forced_rule_table_insert(
        host, host.global(kModeForcedRules + static_cast<std::uint32_t>(mode) * kModeForcedRuleStride), rule);
}

void* active_mode_tuning_00a371a0(Seg70Host& host) {
    const int mode = host.ai_effective_game_mode_index_009ffc80();
    auto* coordinator = static_cast<std::uint8_t*>(host.coordinator_get_or_create_004c1c50());
    return coordinator + 4 + mode * 0x23C;
}

void array32_grow_00a31a40(Seg70Host& host, ArrayRaw& array) {
    array32_reallocate(host, array);
}

void array_dword_reserve_00a31b10(Seg70Host& host, ArrayRaw& array, std::uint32_t capacity) {
    if (capacity <= array.capacity) return;   // JBE
    dword_array_reallocate(host, array, capacity);
}

void array32_push_back_00a32120(Seg70Host& host, ArrayRaw& array, const void* element) {
    if (array.count == array.capacity) array32_reallocate(host, array);
    std::uint8_t* slot = static_cast<std::uint8_t*>(array.data) + (array.count << 5);
    if (slot != nullptr) std::memcpy(slot, element, 0x20);
    ++array.count;
}

void array_dword_grow_00a32200(Seg70Host& host, ArrayRaw& array) {
    const std::uint32_t grown = array.capacity * 2 + 2;
    if (grown <= array.capacity) return;      // JBE
    dword_array_reallocate(host, array, grown);
}

void array_dword_push_back_00a32880(Seg70Host& host, ArrayRaw& array, std::uint32_t value) {
    if (array.count == array.capacity) {
        const std::uint32_t grown = array.capacity * 2 + 2;
        if (grown > array.capacity) dword_array_reallocate(host, array, grown);
    }
    static_cast<std::uint32_t*>(array.data)[array.count] = value;
    ++array.count;
}

}  // namespace bsp::cc13_seg70
