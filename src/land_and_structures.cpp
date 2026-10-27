// Land units and structures. docs/LAND_AND_STRUCTURES.md.
//
// The rules here are the convoy's motion and formation arithmetic, read from
// 00743060, 007410C0, 007410B0, 00742400 and 00742C10, plus the class layout
// table and the shipyard-launch sentinel. Every entity is an opaque handle
// behind LandConvoyPlacementHost.
#include "bsp/land_and_structures.hpp"

#include <cmath>
#include <vector>

namespace bsp {
namespace {

template <class T>
T& land_scene_at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

std::uint8_t* land_scene_pointer(std::uint32_t address) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(address));
}

// 007B03C0's tangent goes into matrix row 2 and its position into row 3;
// 00742400 negates the tangent when "Reverse" is set (007425BE, the double -1.0
// at 00D7A250).
void store_vec3(float* row, const LandVec3& v) noexcept
{
    row[0] = v.x;
    row[1] = v.y;
    row[2] = v.z;
}

LandVec3 load_vec3(const float* row) noexcept
{
    return LandVec3{row[0], row[1], row[2]};
}

}  // namespace

const LandClassLayout* land_class_layout(std::uint32_t class_id) noexcept
{
    for (const LandClassLayout& layout : kLandClassLayouts)
    {
        if (layout.class_id == class_id)
        {
            return &layout;
        }
    }
    return nullptr;
}

bool land_class_offset_exists(std::uint32_t class_id, std::size_t offset) noexcept
{
    const LandClassLayout* layout = land_class_layout(class_id);
    return layout != nullptr && offset < layout->instance_size;
}

// ---------------------------------------------------------------------------
// 00743060, element slot +4h
// ---------------------------------------------------------------------------
LandConvoyArcStep land_convoy_advance_arc_00743060(const LandConvoyFormation& formation,
                                                   float live_arc,
                                                   float odometer,
                                                   float t) noexcept
{
    LandConvoyArcStep out{live_arc, odometer};

    // 00743063: the whole advance sits behind the +3A8h gate. On the stopped
    // path the body falls straight through to the 00742400 call, so the arc and
    // the odometer keep their values and the members are still re-placed.
    if (formation.stopped)
    {
        return out;
    }

    // 0074308B..007430B3: (speed * scale) * t + live in one x87 chain, stored
    // through binary32 once (packet cc9_land_convoy_movement: was three
    // binary32 roundings).
    const double scale = land_convoy_direction_scale(formation.reverse);
    out.live_arc = static_cast<float>(static_cast<double>(live_arc)
        + static_cast<double>(formation.speed) * scale * static_cast<double>(t));

    // 007430D8-007430F0 and 007430FE-00743114: two loops, not two conditionals.
    // The length comes from [[convoy+344h]+28h], which is zero until 007420B0
    // has resolved the "Path" property; guard the loops so an unresolved path
    // cannot hang.
    const float length = formation.path_length;
    if (length > 0.0f)
    {
        while (out.live_arc < 0.0f)
        {
            out.live_arc += length;
        }
        while (out.live_arc >= length)
        {
            out.live_arc -= length;
        }
    }

    // 00743126-0074313E: the odometer takes the absolute speed, so it grows on
    // a reversed convoy too, and it is never wrapped.
    out.odometer = static_cast<float>(static_cast<double>(odometer)
        + static_cast<double>(t) * std::fabs(static_cast<double>(formation.speed)));
    return out;
}

// ---------------------------------------------------------------------------
// 007410C0, element slot +8h
// ---------------------------------------------------------------------------
float land_convoy_restore_arc_007410c0(const LandConvoyFormation& formation,
                                       float committed_arc,
                                       float dt) noexcept
{
    // 007410C1: the same +3A8h gate. No wrap here: the value is re-derived from
    // the committed arc, which 007410B0 only ever copies from an already
    // wrapped live arc.
    if (formation.stopped)
    {
        return committed_arc;
    }
    // 007410E5..007410FA: speed * scale * dt + committed, one binary32 store.
    const double scale = land_convoy_direction_scale(formation.reverse);
    return static_cast<float>(static_cast<double>(formation.speed) * scale
        * static_cast<double>(dt) + static_cast<double>(committed_arc));
}

// ---------------------------------------------------------------------------
// 00742400's formation arithmetic
// ---------------------------------------------------------------------------
float land_convoy_group_arc_00742400(const LandConvoyFormation& formation,
                                     float live_arc,
                                     int group) noexcept
{
    // 00742552..00742577: FILD group, * scale, * RowGap, + live, one binary32
    // store.
    const double scale = land_convoy_direction_scale(formation.reverse);
    float arc = static_cast<float>(static_cast<double>(group) * scale
        * static_cast<double>(formation.row_gap) + static_cast<double>(live_arc));

    // 00742581-007425A9: one conditional each way, unlike 00743060's loops. A
    // row gap wider than the path therefore leaves the arc outside [0, length),
    // and the path sampler receives it as authored.
    const float length = formation.path_length;
    if (arc >= 0.0f)
    {
        if (length <= arc)
        {
            arc -= length;
        }
    }
    else
    {
        arc += length;
    }
    return arc;
}

float land_convoy_lateral_offset_00742400(const LandConvoyFormation& formation,
                                          int lane) noexcept
{
    // 00742622-00742644, in x87 double precision: the file index is centred on
    // (columns - 1) * 0.5 before scaling by the column gap.
    const double centred = static_cast<double>(lane)
                           - (static_cast<double>(formation.columns) - 1.0) * 0.5;
    return static_cast<float>(centred) * formation.column_gap;
}

LandConvoySlot land_convoy_slot_00742400(const LandConvoyFormation& formation, int index) noexcept
{
    LandConvoySlot slot;
    if (formation.columns <= 0)
    {
        return slot;
    }
    slot.group = index / formation.columns;
    slot.lane = index % formation.columns;
    return slot;
}

LandPoseMatrix land_pose_identity() noexcept
{
    LandPoseMatrix out;
    out.m[0] = 1.0f;
    out.m[5] = 1.0f;
    out.m[10] = 1.0f;
    out.m[15] = 1.0f;
    return out;
}

// ---------------------------------------------------------------------------
// 00742400 BSP_LandConvoy_PlaceMembers
// ---------------------------------------------------------------------------
LandConvoyPlacementResult land_convoy_place_members_00742400(
    const LandConvoyFormation& formation,
    float live_arc,
    bool world_pose_law,
    LandConvoyPlacementHost& host)
{
    LandConvoyPlacementResult result;

    const int slots = land_convoy_slot_count(formation);
    if (slots <= 0 || formation.columns <= 0)
    {
        return result;
    }

    // 00742409-00742447: one matrix per row, allocated up front and zeroed.
    // The native cache is freed at 00742900-00742925; the ten bytes Ghidra drops
    // after the free at 00742908 (0074290D ADD ESP,4; 00742910 MOV [EDI+ESI*4],0)
    // are what make that loop free every entry instead of returning after the
    // first, so there is no leak to reproduce.
    std::vector<bool> group_ready(static_cast<std::size_t>(formation.rows), false);
    std::vector<LandPoseMatrix> group_pose(static_cast<std::size_t>(formation.rows));

    const std::uintptr_t parent = host.convoy_parent();
    const int available = host.member_count();

    for (int index = 0; index < slots; ++index)
    {
        ++result.slots_visited;

        // 00742470-007424A0: the native code indexes the member vector and traps
        // through LIBCRT_unmatched_00BF6713 when it is shorter than rows *
        // columns. Projected as "skip the slot": the trap arm carries no
        // placement behaviour.
        if (index >= available)
        {
            continue;
        }
        const std::uintptr_t member = host.member_at(index);
        if (member == 0)
        {
            // 007422A0 nulls a dead member's slot in place; 00742497 skips it.
            // The game host nulls it under kLandConvoyBodiesBound (packet
            // cc13-convoy: 007422A0 and 00742210 share the motion state's slots).
            continue;
        }

        const LandConvoySlot slot = land_convoy_slot_00742400(formation, index);
        const std::size_t group = static_cast<std::size_t>(slot.group);
        if (group >= group_ready.size())
        {
            continue;
        }

        if (!group_ready[group])
        {
            // 007424B0-0074260E: build this row's frame once.
            LandPoseMatrix pose = land_pose_identity();
            const float arc = land_convoy_group_arc_00742400(formation, live_arc, slot.group);

            LandVec3 position;
            LandVec3 tangent;
            host.sample_path_007b03c0(arc, &position, &tangent);

            if (formation.reverse)
            {
                // 007425BE-007425E0: the forward axis is negated, the position
                // is not.
                tangent.x = -tangent.x;
                tangent.y = -tangent.y;
                tangent.z = -tangent.z;
            }

            store_vec3(pose.row(2), tangent);
            store_vec3(pose.row(3), position);
            store_vec3(pose.row(1), host.world_up_00f8758c());
            host.orthonormalize_frame_0085dc80(&pose);

            group_pose[group] = pose;
            group_ready[group] = true;
            ++result.groups_sampled;
        }

        // 00742628-0074264A: the row frame is copied, then offset laterally
        // along row 0.
        LandPoseMatrix pose = group_pose[group];
        const float lateral = land_convoy_lateral_offset_00742400(formation, slot.lane);
        const LandVec3 right = load_vec3(pose.row(0));
        float* translation = pose.row(3);
        translation[0] += lateral * right.x;
        translation[1] += lateral * right.y;
        translation[2] += lateral * right.z;

        if (world_pose_law)
        {
            // 00742662-007426DF, taken when +3C8h is set. Row 1 becomes the
            // literal world up written at 00742666-0074268B: the global triple
            // 00F8758C is used for the path frame, not for this rebuild.
            host.orthonormalize_from_normal_0085dad0(&pose, LandVec3{0.0f, 1.0f, 0.0f});
            host.set_member_world_pose_00741e90(member, pose);
            host.notify_member_scene_node(member);
            ++result.world_pose_law_placements;
        }
        else
        {
            // 007426E4-007428CE, taken when +3C8h is clear: snap onto the
            // parent's height field. 00742792 makes the height parent-relative.
            host.refresh_world_pose_00414db0(parent);
            const float x = translation[0];
            const float z = translation[2];
            translation[1] = host.terrain_height_0087fa20(parent, x, z)
                             - host.parent_height_origin_00742792(parent);
            const LandVec3 normal = host.terrain_normal_0087fb90(parent, x, z);
            host.orthonormalize_from_normal_0085dad0(&pose, normal);
            host.write_member_local_matrix(member, pose);
            host.invalidate_member_subtree_0042ed50(member);
            host.notify_member_scene_node(member);
            ++result.terrain_law_placements;
        }

        ++result.members_placed;
    }

    return result;
}

// 007482B0..00748986, native ECX fort / RET. Complete ordinary body from all
// 438 listing instructions, including the checked-back stores and final
// 00BF65AC which Ghidra's unreachable-block pseudocode omits. FH3 unwind
// actions are excluded; normal frees retain their actual targets and order.
// Reserve(30) is preserved: no pointer repair is invented when a later vector
// growth relocates previously published shape descriptors.
void land_fort_init_scene_and_create_static_body_007482b0(
    std::uint8_t* fort, std::uint8_t* const& game_00e188a8, LandFortSceneHost& host) {
    host.call_00955420(fort);                                          // 007482D5
    std::uint8_t* holder = land_scene_at<std::uint8_t*>(fort, 0xC0);
    const auto kind = land_scene_at<std::uint32_t>(holder, 4);
    if (kind == 1) {
        std::uint8_t* bag = land_scene_at<std::uint8_t*>(holder, 8);
        std::int32_t level = 0;
        if (host.call_0048e9f0(bag, "DamageLevel")) {                   // 00748312
            std::uint8_t* property = host.call_008f2260(bag, "DamageLevel"); // 00748326
            level = land_scene_at<std::int32_t>(property, 0xC);
            if (level > 0) host.vcall_70(fort, 1);                     // 0074833C
        }
        if (land_scene_at<void*>(fort, 0x360) != nullptr)
            host.call_00710bb0(land_scene_at<void*>(fort, 0x360), static_cast<std::uint32_t>(level)); // 00748349
        for (std::uint8_t* child = land_scene_at<std::uint8_t*>(fort, 0x48); child != nullptr;
             child = land_scene_at<std::uint8_t*>(child, 0x44)) {
            if (host.vcall_5c(child, 0x20)) host.vcall_1b4(child, level); // 0074835E/0074836F
        }
        auto* skill_slot = land_scene_at<std::uint8_t*>(fort, 0) + 0x128; // captured 00748378
        const std::uint32_t skill = host.call_00927a80(fort);           // 00748383
        host.vcall_128(fort, skill, land_scene_at<std::uint32_t>(skill_slot, 0)); // 0074838D
        if (land_scene_at<void*>(fort, 0x50) != nullptr
            && land_scene_at<std::uint32_t>(game_00e188a8, 0x1FE4) != 2) {
            void* storage = host.call_00bf681b(0x558);                 // 007483A7
            void* gunnery = storage == nullptr ? nullptr : host.call_00864580(storage); // 007483C4
            land_scene_at<void*>(fort, 0x6DC) = gunnery;
            // The native code dereferences gunnery even after a null result.
            host.vcall_04(gunnery, fort + 0x310);                      // 007483F3
        }
    } else if (kind == 2) {
        land_scene_at<std::uint32_t>(fort, 0x58) =
            land_scene_at<std::uint8_t>(land_scene_at<void*>(holder, 8), 0xC4);
    }
    if (land_scene_at<std::uint8_t>(land_scene_at<void*>(fort, 0x30), 0x4A4) != 0) {
        std::uint8_t* parent = land_scene_at<std::uint8_t*>(fort, 0x3C); // captured before slot38
        host.vcall_38(land_scene_at<void*>(fort, 0x4A4), reinterpret_cast<const float*>(fort + 0x74)); // 00748413
        if (parent != nullptr) {
            void* parent_node = host.vcall_40(land_scene_at<void*>(parent, 0x3D0)); // 00748426
            host.call_00b6e680(land_scene_at<void*>(fort, 0x4A4), parent_node); // 0074842F
        }
    }
    alignas(4) std::uint8_t shapes[0x10];
    land_scene_at<std::uint32_t>(shapes, 4) = 0;
    land_scene_at<std::uint32_t>(shapes, 8) = 0;
    land_scene_at<std::uint32_t>(shapes, 0xC) = 0;
    host.call_00747cc0(shapes, 0x1E);                                  // 00748455
    alignas(4) std::uint8_t body[0x84];
    body[0] = 0;
    for (std::size_t offset = 4; offset <= 0x80; offset += 4)
        land_scene_at<std::uint32_t>(body, offset) = 0;
    for (std::size_t offset : {4u, 8u, 0xCu, 0x10u, 0x14u, 0x24u, 0x34u})
        land_scene_at<float>(body, offset) = 1.0f;
    land_scene_at<float>(body, 0x68) = 1000.0f;                         // 00CE3804
    land_scene_at<float>(body, 0x6C) = 1000.0f;

    std::uint8_t* scene = land_scene_at<void*>(fort, 0x360) == nullptr ? nullptr
        : land_scene_at<std::uint8_t*>(land_scene_at<void*>(fort, 0x360), 0x160);
    std::uint8_t* owner = scene + 0x4C;
    std::uint32_t cursor = land_scene_at<std::uint32_t>(scene, 0x50);
    if (cursor > land_scene_at<std::uint32_t>(scene, 0x54)) host.invalid_iterator_00bf6713(); // 00748576
    const auto checked_back = [&]() -> std::uint8_t* {
        std::uint32_t current_end = land_scene_at<std::uint32_t>(shapes, 8);
        const std::uint32_t saved_end = current_end; // ESI survives the handler
        if (land_scene_at<std::uint32_t>(shapes, 4) > current_end) {
            host.invalid_iterator_00bf6713();
            current_end = land_scene_at<std::uint32_t>(shapes, 8); // EAX reload only
        }
        const std::uint32_t back = saved_end - 0x48u;
        if (back > current_end || back < land_scene_at<std::uint32_t>(shapes, 4))
            host.invalid_iterator_00bf6713();
        if (back >= land_scene_at<std::uint32_t>(shapes, 8)) host.invalid_iterator_00bf6713();
        return land_scene_pointer(back);
    };
    for (;;) {
        std::uint8_t* current = host.call_00476b90(fort);               // 00748587
        const std::uint32_t end = land_scene_at<std::uint32_t>(current, 0x54);
        if (land_scene_at<std::uint32_t>(current, 0x50) > end) host.invalid_iterator_00bf6713(); // 00748599
        if (owner == nullptr || owner != current + 0x4C) host.invalid_iterator_00bf6713(); // 007485A9
        if (cursor == end) break;
        if (owner == nullptr) host.invalid_iterator_00bf6713();         // 007485BF
        if (cursor >= land_scene_at<std::uint32_t>(owner, 8)) host.invalid_iterator_00bf6713(); // 007485D0
        std::uint8_t* group = land_scene_at<std::uint8_t*>(land_scene_pointer(cursor), 0);
        alignas(4) std::uint8_t shape[0x48];
        for (std::size_t offset = 0; offset != 0x48; offset += 4)
            land_scene_at<std::uint32_t>(shape, offset) = 0;
        land_scene_at<std::uint32_t>(shape, 0x10) = 4;
        for (std::size_t offset : {0x18u, 0x28u, 0x38u}) land_scene_at<float>(shape, offset) = 1.0f;
        host.call_00748150(shapes, shape);                             // 00748692
        land_scene_at<float>(checked_back(), 4) = 1.0f;                // 007486DD
        land_scene_at<void*>(checked_back(), 0x14) = group + 0xC;       // 00748723
        land_scene_at<std::uint32_t>(checked_back(), 8) = 1;            // 00748764
        land_scene_at<std::uint32_t>(checked_back(), 0xC) = 0xD;        // 007487A9
        std::uint8_t* back = checked_back();                           // 007487EE
        const std::uint32_t count = land_scene_at<std::uint32_t>(body, 0x7C);
        if (count == land_scene_at<std::uint32_t>(body, 0x80)) {
            const std::uint32_t capacity = land_scene_at<std::uint32_t>(body, 0x80) * 2u + 2u;
            land_scene_at<std::uint32_t>(body, 0x80) = capacity;
            void* grown = host.call_00bf55be(capacity * 4u);            // 0074880E
            for (std::uint32_t i = 0; i < land_scene_at<std::uint32_t>(body, 0x7C); ++i) {
                auto* slot = reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uintptr_t>(grown) + i * 4u);
                if (slot != nullptr) *slot = land_scene_at<std::uint32_t*>(body, 0x78)[i];
            }
            if (land_scene_at<void*>(body, 0x78) != nullptr)
                host.call_00bf6989(land_scene_at<void*>(body, 0x78));   // 00748850
            land_scene_at<void*>(body, 0x78) = grown;
        }
        auto* slot = reinterpret_cast<void**>(land_scene_at<std::uintptr_t>(body, 0x78) + count * 4u);
        if (slot != nullptr) *slot = back;
        ++land_scene_at<std::uint32_t>(body, 0x7C);
        if (cursor >= land_scene_at<std::uint32_t>(owner, 8)) host.invalid_iterator_00bf6713(); // 0074888B
        cursor += 8;
    }
    if (land_scene_at<std::uint32_t>(body, 0x7C) != 0) {
        if (land_scene_at<std::uint8_t>(fort, 0xC8) == 0) {
            if (land_scene_at<void*>(fort, 0x3C) != nullptr) host.call_00414db0(land_scene_at<void*>(fort, 0x3C)); // 007488C8
            float scratch[16];
            const float* matrix = land_scene_at<void*>(fort, 0x3C) == nullptr
                ? reinterpret_cast<const float*>(fort + 0x74)
                : host.call_00413920(reinterpret_cast<const float*>(fort + 0x74), scratch,
                    reinterpret_cast<const float*>(land_scene_at<std::uint8_t*>(fort, 0x3C) + 0xCC)); // 007488E5
            host.call_004134f0(reinterpret_cast<float*>(fort + 0xCC), matrix); // 007488F8
            land_scene_at<std::uint8_t>(fort, 0xC8) = 1;
            land_scene_at<std::uint8_t>(fort, 0x10C) = 0;
        }
        host.call_00c336c0(reinterpret_cast<float*>(body + 0x14), reinterpret_cast<const float*>(fort + 0xCC)); // 00748910
        land_scene_at<std::uint32_t>(body, 0x5C) |= 1;
        land_scene_at<void*>(body, 0x74) = fort;
        land_scene_at<void*>(fort, 0x754) = host.call_00c5d580(land_scene_at<void*>(game_00e188a8, 0x18), body); // 00748932
    } else land_scene_at<void*>(fort, 0x754) = nullptr;
    if (land_scene_at<void*>(body, 0x78) != nullptr) host.call_00bf6989(land_scene_at<void*>(body, 0x78)); // 00748959
    if (land_scene_at<void*>(shapes, 4) != nullptr) host.call_00bf65ac(land_scene_at<void*>(shapes, 4)); // 0074896A
}

}  // namespace bsp
