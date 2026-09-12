#include "bsp/hud_minimap.hpp"

#include <cmath>

// Packet cc_hud_minimap, docs/HUD_MINIMAP.md.
namespace bsp {

HudMinimapTerrainUv hud_minimap_terrain_uv(float world_x, float world_z) noexcept {
    // minimap_terrain.shfx: eyeOffs = float2(cam.x, -cam.z) / 30000 and
    // limits = (border + 15000) / 30000, so the map itself is
    // (world + 15000) / 30000 with the z axis inverted.
    HudMinimapTerrainUv uv{};
    uv.u = (world_x + kHudMinimapTerrainWorldHalf) / kHudMinimapTerrainWorldSize;
    uv.v = (-world_z + kHudMinimapTerrainWorldHalf) / kHudMinimapTerrainWorldSize;
    return uv;
}

bool hud_minimap_terrain_inside_mask(float quad_u, float quad_v) noexcept {
    // if (dot(UV - 0.5, UV - 0.5) > CenterBorderDistPow2) alpha = 0.
    const float du = quad_u - 0.5f;
    const float dv = quad_v - 0.5f;
    return (du * du + dv * dv) <= kHudMinimapTerrainMaskRadiusSq;
}

HudMinimapHeadings hud_minimap_headings_005c1788(float heading_raw, float range) noexcept {
    HudMinimapHeadings out{};
    // 005C1788..005C179F: 80.0 divided by the range, computed before the atan2.
    out.scale = range != 0.0f ? kHudMinimapRadiusUnits / range : 0.0f;
    // 005C17F2 negates the atan2 for the icons; 005C1812 negates it again for
    // the compass, island map and direction wedge.
    out.icon = -heading_raw;
    out.map = heading_raw;
    return out;
}

bool hud_minimap_within_visibility(const HudMinimapWorldPoint& point,
                                   const HudMinimapWorldPoint& camera,
                                   float visibility_range) noexcept {
    // 005C168C..005C16DB: the deltas are camera minus unit and the comparison
    // is against the squared range, which 005C15A0 squares once per frame.
    const float dx = camera.x - point.x;
    const float dy = camera.y - point.y;
    const float dz = camera.z - point.z;
    const float d2 = dx * dx + dy * dy + dz * dz;
    return d2 <= visibility_range * visibility_range;
}

HudMinimapWorldPoint hud_minimap_clamp_to_rim_005c1a3c(const HudMinimapWorldPoint& point,
                                                       const HudMinimapWorldPoint& camera,
                                                       float range) noexcept {
    // 005C1988..005C1A48: the squared distance uses camera minus point, in the
    // order x*x + y*y then + z*z.
    const float ax = camera.x - point.x;
    const float ay = camera.y - point.y;
    const float az = camera.z - point.z;
    const float d2 = ax * ax + ay * ay + az * az;
    if (d2 <= range * range) {
        return point;
    }

    // 005C1A52..005C1B39: one sqrt, then point minus camera scaled to `range`
    // and added back to the camera. The delta here is point minus camera, the
    // opposite order from the test above, which is what the two FSUBP forms at
    // 005C1A6D and 005C1A6F encode.
    const float d = std::sqrt(d2);
    HudMinimapWorldPoint out{};
    if (d == 0.0f) {
        return camera;
    }
    out.x = camera.x + (point.x - camera.x) * range / d;
    out.y = camera.y + (point.y - camera.y) * range / d;
    out.z = camera.z + (point.z - camera.z) * range / d;
    return out;
}

HudGuiPoint hud_minimap_icon_position_005c1b62(const HudMinimapWorldPoint& point,
                                               const HudMinimapWorldPoint& camera, float range,
                                               float icon_heading, float depth) noexcept {
    const HudMinimapWorldPoint clamped = hud_minimap_clamp_to_rim_005c1a3c(point, camera, range);
    const float scale = range != 0.0f ? kHudMinimapRadiusUnits / range : 0.0f;

    // 005C1B62..005C1B8D: x from the x axis, y from the z axis, both scaled.
    float mx = (clamped.x - camera.x) * scale;
    float my = (clamped.z - camera.z) * scale;

    // 005C1B6C tests the heading against zero and 005C1B91 skips the rotation
    // when it is exactly zero.
    if (icon_heading != 0.0f) {
        const float c = std::cos(icon_heading);
        const float s = std::sin(icon_heading);
        const float rx = mx * c + my * s; // 005C1BBB..005C1BCD
        const float ry = my * c - mx * s; // 005C1BED..005C1BFF
        mx = rx;
        my = ry;
    }

    // 005C1C61..005C1C92: the two divisors differ, so the space is 4:3.
    HudGuiPoint out{};
    out.x = mx / kHudMinimapXDivisor;
    out.y = -my / kHudMinimapYDivisor;
    out.z = depth;
    return out;
}

float hud_minimap_icon_rotation_005c1ca1(float unit_heading, float icon_heading) noexcept {
    // 005C1CB1 adds the icon heading to the unit's own, then 005C1CBC subtracts
    // the sum from pi/2 (FSUBR against 00CE3830).
    return kHudMinimapHeadingBias - (unit_heading + icon_heading);
}

int hud_minimap_place_unit_icons_005c154e(int team_index, HudMinimapHost& host,
                                          float depth) noexcept {
    // 005C1561..005C1578: the team index must be 0..7 and a camera unit must
    // exist, otherwise the whole pass is skipped.
    if (team_index < 0 || team_index > 7) {
        return 0;
    }
    void* camera = host.camera_unit();
    if (camera == nullptr) {
        return 0;
    }

    const float range = host.minimap_range();
    const float visibility = host.visibility_range();

    host.refresh_pose(camera); // 005C15BB
    const HudMinimapWorldPoint camera_pos = host.world_position(camera);
    void* self_unit = host.displayed_self_unit(nullptr); // 005C15F2

    // 005C1788..005C1812: one heading for the whole frame.
    float basis_y = 0.0f;
    float basis_x = 0.0f;
    host.renderer_basis(basis_y, basis_x);
    const HudMinimapHeadings headings =
        hud_minimap_headings_005c1788(std::atan2(basis_y, basis_x), range);

    // 005C1825, 005C184F and 005C1872: the three map layers turn with the map
    // heading, not the icon heading.
    if (void* compass = host.map_layer(kHudMinimapCompassSlot)) {
        host.set_map_layer_rotation(compass, headings.map);
    }
    if (void* island = host.map_layer(kHudMinimapIslandMapSlot)) {
        host.set_map_layer_rotation(island, headings.map);
    }

    int placed = 0;
    for (void* node = host.team_unit_list_head(team_index); node != nullptr;
         node = host.team_unit_list_next(node)) {
        void* unit = host.team_unit_list_unit(node);
        if (unit == nullptr) {
            continue;
        }
        if (!host.unit_is_alive_and_visible(unit)) {
            continue; // 005C1628..005C164A
        }
        if (unit == camera) {
            continue; // 005C1650
        }
        if (!host.is_kind_of(unit, 5)) {
            continue; // 005C165D
        }
        if (!host.unit_shows_on_minimap(unit)) {
            continue; // 005C1675
        }

        const HudMinimapWorldPoint position = host.world_position(unit);
        // 005C167F: the displayed self unit skips the distance cull.
        if (unit != self_unit &&
            !hud_minimap_within_visibility(position, camera_pos, visibility)) {
            continue; // 005C16DB
        }

        void* icon = host.find_icon_entry(unit); // 005C170A
        if (icon == nullptr) {
            icon = host.create_icon_entry(unit); // 005C171C..005C1767
        }
        if (icon == nullptr) {
            continue;
        }

        // 005C19B1: the second cull, after the icon exists.
        if (!hud_minimap_within_visibility(position, camera_pos, visibility)) {
            continue;
        }

        host.set_icon_position(
            icon, hud_minimap_icon_position_005c1b62(position, camera_pos, range, headings.icon,
                                                     depth));
        ++placed;
    }
    return placed;
}

} // namespace bsp
