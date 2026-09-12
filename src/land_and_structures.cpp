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

    const float scale = land_convoy_direction_scale(formation.reverse);
    out.live_arc = live_arc + t * formation.speed * scale;

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
    out.odometer = odometer + t * std::fabs(formation.speed);
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
    const float scale = land_convoy_direction_scale(formation.reverse);
    return committed_arc + dt * formation.speed * scale;
}

// ---------------------------------------------------------------------------
// 00742400's formation arithmetic
// ---------------------------------------------------------------------------
float land_convoy_group_arc_00742400(const LandConvoyFormation& formation,
                                     float live_arc,
                                     int group) noexcept
{
    const float scale = land_convoy_direction_scale(formation.reverse);
    float arc = static_cast<float>(group) * scale * formation.row_gap + live_arc;

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

}  // namespace bsp
