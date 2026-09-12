#include "bsp/hud_markers_runtime.hpp"

#include <cmath>

// Packet cc_hud_minimap, docs/HUD_MARKERS_RUNTIME.md.
namespace bsp {
namespace {

// 0043A7BB and 0043A7FC read the extent object 00AA1FE0 returns; only +4h, the
// height, is used by modes 1 and 3.
constexpr float kHalf = 0.5f;   // 00D7A280, a double
constexpr float kOne = 1.0f;    // 00D7A24C
constexpr float kMinusOne = -1.0f; // 00D7A250, a double

} // namespace

HudMarkerScreenProjection camera_project_world_to_screen_0043a660(
    float clip_x, float clip_y, float clip_z, float clip_w, HudMarkerProjectMode mode,
    bool clamp_depth, float gui_extent_height, float renderer_aspect) noexcept {
    HudMarkerScreenProjection out{};
    // 0043A6D5..0043A72B: w_inv = 1/clip.w, then ndc = clip.xyz * w_inv. The
    // reciprocal is formed once and multiplied three times, not three divides.
    const float w_inv = kOne / clip_w;
    out.ndc_x = w_inv * clip_x;
    out.ndc_y = clip_y * w_inv;
    out.ndc_z = clip_z * w_inv;

    // 0043A72E: the whole mask is skipped unless w_inv > 0 (00D7A218).
    if (w_inv > 0.0f) {
        if (out.ndc_x >= kMinusOne && out.ndc_x < kOne) {
            out.clip_mask |= kHudMarkerProjectClipX;
        }
        if (out.ndc_y >= kMinusOne && out.ndc_y < kOne) {
            out.clip_mask |= kHudMarkerProjectClipY;
        }
        // 0043A762..0043A77A. The clamp only adds the upper bound.
        if (out.ndc_z >= 0.0f && (!clamp_depth || out.ndc_z < kOne)) {
            out.clip_mask |= kHudMarkerProjectClipZ;
        }
    }

    float x = out.ndc_x;
    float y = out.ndc_y;
    switch (mode) {
    case HudMarkerProjectMode::Viewport:
        // 0043A787..0043A79E.
        out.screen_x = (x + kOne) * kHalf;
        out.screen_y = (kOne - y) * kHalf;
        break;
    case HudMarkerProjectMode::ViewportAspect:
        // 0043A7BB: out.y is first scaled by the extent height, then flipped.
        y = gui_extent_height * y;
        out.screen_x = (x + kOne) * kHalf;
        out.screen_y = (kOne - y) * kHalf;
        break;
    case HudMarkerProjectMode::Pixels:
        // 0043A7xx, the mode-2 branch: pixel space with its own two factors.
        out.screen_x = (x + kOne) * kHalf * kHudMarkerProjectPixelWidth;
        out.screen_y = (kOne - y) * kHalf * kHudMarkerProjectPixelHeight;
        break;
    case HudMarkerProjectMode::ViewportWide:
        // 0043A7FC: extent height on y, renderer aspect on x.
        y = gui_extent_height * y;
        x = renderer_aspect * x;
        out.screen_x = (x + kOne) * kHalf;
        out.screen_y = (kOne - y) * kHalf;
        break;
    }
    return out;
}

HudMarkerClipResult hud_marker_project_and_clip_00638e50(
    float screen_x, float screen_y, unsigned clip_mask, bool wide_aspect_active,
    const HudMarkerClipRect& clip) noexcept {
    HudMarkerClipResult out{};
    out.screen_y = screen_y;

    const bool fully = (clip_mask & kHudMarkerProjectClipAll) == kHudMarkerProjectClipAll;

    if (wide_aspect_active) {
        // 00638E74..00638E90: x is rescaled about 0.5 before any test.
        out.screen_x = kHalf + (screen_x - kHalf) * kHudMarkerWideAspect;
        if (!fully) {
            // 00638E94: the partial case additionally needs both lateral bits.
            if ((clip_mask & (kHudMarkerProjectClipX | kHudMarkerProjectClipY)) !=
                (kHudMarkerProjectClipX | kHudMarkerProjectClipY)) {
                return out;
            }
            if (!(clip.left < out.screen_x && out.screen_x < clip.right)) {
                return out;
            }
        }
    } else {
        out.screen_x = screen_x;
        // 00638EC6: without the rescale the point must be fully inside NDC.
        if (!fully) {
            return out;
        }
        if (!(clip.left < out.screen_x && out.screen_x < clip.right)) {
            return out;
        }
    }

    // 00638F01..00638F29, common to both paths.
    out.visible = clip.top < out.screen_y && out.screen_y < clip.bottom;
    return out;
}

HudMarkerFlags hud_marker_flags_006430c0(const void* unit, const void* self_unit,
                                         const void* target_unit, int unit_team_id,
                                         int local_team_record_id, bool in_objective_container,
                                         bool force_objective_colour) noexcept {
    HudMarkerFlags flags{};
    flags.is_self = (unit == self_unit);                       // 00643136
    flags.is_friendly = (unit_team_id == local_team_record_id); // 0064315B
    flags.is_target = (unit == target_unit);                    // 0064316A
    // 0064318E..006431A1: an objective unit loses the friendly colour unless
    // the caller forced it.
    if (flags.is_friendly && in_objective_container && !force_objective_colour) {
        flags.is_friendly = false;
    }
    return flags;
}

HudMarkerDispatch hud_marker_dispatch_006430c0(const void* unit, const HudMarkerKindProbe& probe,
                                               const HudMarkerFlags& flags,
                                               bool force_objective_colour,
                                               bool delegate_is_45_or_46, int caller_kind_a,
                                               int caller_kind_b, int unit_team_id,
                                               int local_team_record_id) noexcept {
    HudMarkerDispatch out{};
    out.kind_a = caller_kind_a;
    out.kind_b = caller_kind_b;

    if (probe.is_kind_of(unit, kHudMarkerKindGroupOwner)) {
        out.builder = HudMarkerBuilder::GroupOwner; // 006431D9
        out.kind_a = 2;
        out.kind_b = 0;
    } else if (probe.is_kind_of(unit, kHudMarkerKindSubUnits)) {
        out.builder = HudMarkerBuilder::SubUnits; // 00643206
        out.kind_a = 2;
        out.kind_b = 0;
    } else if (probe.is_kind_of(unit, kHudMarkerKindDelegating) && delegate_is_45_or_46) {
        out.builder = HudMarkerBuilder::Delegating; // 0064326B
        out.kind_a = 1;
        out.kind_b = 2;
    } else if (probe.is_kind_of(unit, kHudMarkerKindClose)) {
        out.builder = HudMarkerBuilder::Close; // 006432A9
        // 00643284..0064329B: the fourth argument is isTarget OR the caller's
        // force flag, not isTarget alone.
        out.close_highlight = flags.is_target || force_objective_colour;
    } else if (probe.is_kind_of(unit, kHudMarkerKindShip)) {
        out.builder = HudMarkerBuilder::Ship; // 006432DB, caller kinds pass through
    } else if (probe.is_kind_of(unit, kHudMarkerKindTracked)) {
        out.builder = HudMarkerBuilder::TrackedSet; // 00643311
    }

    // 00643316..00643351: runs after every branch, including None.
    if (probe.is_kind_of(unit, kHudMarkerKindLockA) ||
        probe.is_kind_of(unit, kHudMarkerKindLockB)) {
        out.run_lock_tail = (local_team_record_id != unit_team_id);
    }
    return out;
}

void hud_marker_accumulate_corner_0063a6c0(HudMarkerScreenBounds& bounds,
                                           const HudMarkerProjectedCorner& corner) noexcept {
    // 0063A9DB..0063AA8B. The visibility result is discarded here; only the
    // projected coordinates take part in the min/max.
    if (corner.x < bounds.min_x) {
        bounds.min_x = corner.x;
    }
    if (corner.y < bounds.min_y) {
        bounds.min_y = corner.y;
    }
    if (corner.x > bounds.max_x) {
        bounds.max_x = corner.x;
    }
    if (corner.y > bounds.max_y) {
        bounds.max_y = corner.y;
    }
}

bool hud_marker_apply_minimum_size_0063a6c0(HudMarkerScreenBounds& bounds,
                                            float screen_field_88) noexcept {
    // 0063AABD..0063AACA: the margin is this[88h] / 1.5.
    const float margin_x = screen_field_88 / kHudMarkerMinSizeDivisor;
    bool widened_x = false;
    // 0063AAE4..0063AB1D: only a box narrower than the margin is widened, and
    // the half-step literal is negative, so the min moves left and the max right.
    const float width = bounds.max_x - bounds.min_x;
    if (margin_x > width) {
        const float half = kHudMarkerWidenHalfStep * (width - margin_x);
        bounds.min_x -= half;
        bounds.max_x += half;
        widened_x = true;
    }

    // 0063AB25: the y margin is the same value scaled by 1.33.
    const float margin_y = margin_x * kHudMarkerMinSizeYFactor;
    bool widened_y = false;
    const float height = bounds.max_y - bounds.min_y;
    if (margin_y > height) {
        const float half = kHudMarkerWidenHalfStep * (height - margin_y);
        bounds.min_y -= half;
        bounds.max_y += half;
        widened_y = true;
    }

    // 0063AB55/0063AB73: the collapse only happens when both axes widened.
    return widened_x && widened_y;
}

HudMarkerCornerSigns hud_marker_corner_signs_0063a6c0(int index) noexcept {
    // 0063A860, 0063A870, 0063A890: three nested loops, each running -1 then +1
    // (ADD reg,2 / CMP reg,1 / JLE). The outermost is the right axis, then up,
    // then forward, so the forward sign alternates fastest.
    HudMarkerCornerSigns signs{};
    signs.right = (index & 4) != 0 ? 1.0f : -1.0f;
    signs.up = (index & 2) != 0 ? 1.0f : -1.0f;
    signs.forward = (index & 1) != 0 ? 1.0f : -1.0f;
    return signs;
}

HudMarkerBuilder hud_markers_add_unit_marker_006430c0(
    void* unit, const HudMarkerFlags& flags, bool force_objective_colour, int caller_kind_a,
    int caller_kind_b, bool delegate_is_45_or_46, int unit_team_id, int local_team_record_id,
    const HudMarkerClipRect& clip, bool wide_aspect_active, float screen_field_88,
    HudMarkersRuntimeHost& host, HudMarkerScreenBounds& bounds) noexcept {
    if (unit == nullptr) {
        return HudMarkerBuilder::None; // 006430CD
    }
    // 006430D8..006430ED: a ship leaf whose parts-object byte is 1 is dropped.
    if (host.is_kind_of(unit, kHudMarkerKindShipLeaf) && host.parts_object_flag_is_one(unit)) {
        return HudMarkerBuilder::None;
    }
    if (host.is_kind_of(unit, kHudMarkerKindExcludedA)) {
        return HudMarkerBuilder::None; // 00643100
    }
    if (host.is_kind_of(unit, kHudMarkerKindExcludedB)) {
        return HudMarkerBuilder::None; // 00643113
    }
    if (host.already_marked(unit)) {
        return HudMarkerBuilder::None; // 00643123
    }
    host.mark_unit(unit); // 00643131

    if (!host.is_alive_and_visible(unit)) {
        return HudMarkerBuilder::None; // 006431B0
    }

    struct Probe final : HudMarkerKindProbe {
        HudMarkersRuntimeHost* h;
        bool is_kind_of(const void* u, int class_id) const override {
            return h->is_kind_of(const_cast<void*>(u), class_id);
        }
    } probe{};
    probe.h = &host;

    const HudMarkerDispatch dispatch =
        hud_marker_dispatch_006430c0(unit, probe, flags, force_objective_colour,
                                     delegate_is_45_or_46, caller_kind_a, caller_kind_b,
                                     unit_team_id, local_team_record_id);

    if (dispatch.builder != HudMarkerBuilder::Ship &&
        dispatch.builder != HudMarkerBuilder::Delegating) {
        // Only 00642040 reaches 0063A6C0 with a screen rectangle in this
        // reconstruction; 00642960, 00642B80, 00641910 and 0063F1E0 are not
        // covered. docs/HUD_MARKERS_RUNTIME.md lists them as follow-up work.
        return dispatch.builder;
    }

    // 006420EE and 00642211: pose refresh, then the oriented bounding box.
    host.refresh_pose(unit);
    float right[3] = {0.0f, 0.0f, 0.0f};
    float up[3] = {0.0f, 0.0f, 0.0f};
    float forward[3] = {0.0f, 0.0f, 0.0f};
    float anchor[3] = {0.0f, 0.0f, 0.0f};
    host.world_matrix_rows(unit, right, up, forward, anchor);

    float extent_forward = 0.0f;
    float extent_right = 0.0f;
    float extent_up = 0.0f;
    host.class_record_extents(unit, extent_forward, extent_right, extent_up);

    // 0063A7C3..0063A847: a ship leaf that is not kind 8 rides its own up
    // extent and then doubles the up row.
    float up_row[3] = {up[0], up[1], up[2]};
    if (host.is_kind_of(unit, kHudMarkerKindShipLeaf) && !host.is_kind_of(unit, 0x08)) {
        for (int i = 0; i < 3; ++i) {
            anchor[i] += up_row[i] * extent_up;
            up_row[i] *= kHudMarkerUpAxisScale;
        }
    }

    bounds = HudMarkerScreenBounds{};
    for (int corner = 0; corner < kHudMarkerCornerCount; ++corner) {
        const HudMarkerCornerSigns signs = hud_marker_corner_signs_0063a6c0(corner);
        float world[3];
        for (int i = 0; i < 3; ++i) {
            world[i] = anchor[i] + 0.5f * signs.right * extent_right * right[i] +
                       0.5f * signs.up * extent_up * up_row[i] +
                       0.5f * signs.forward * extent_forward * forward[i];
        }
        float clip_point[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        host.project_clip_space(world, clip_point);

        float extent_w = 0.0f;
        float extent_h = 0.0f;
        host.gui_extent(extent_w, extent_h);
        const HudMarkerScreenProjection projected = camera_project_world_to_screen_0043a660(
            clip_point[0], clip_point[1], clip_point[2], clip_point[3],
            HudMarkerProjectMode::ViewportAspect, false, extent_h, 1.0f);
        const HudMarkerClipResult clipped = hud_marker_project_and_clip_00638e50(
            projected.screen_x, projected.screen_y, projected.clip_mask, wide_aspect_active, clip);

        HudMarkerProjectedCorner accumulated{};
        accumulated.x = clipped.screen_x;
        accumulated.y = clipped.screen_y;
        accumulated.visible = clipped.visible;
        hud_marker_accumulate_corner_0063a6c0(bounds, accumulated);
    }

    bounds.collapsed = hud_marker_apply_minimum_size_0063a6c0(bounds, screen_field_88);
    return dispatch.builder;
}

} // namespace bsp
