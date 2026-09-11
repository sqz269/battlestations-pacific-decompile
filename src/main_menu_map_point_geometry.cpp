#include "bsp/main_menu_map_point_geometry.hpp"
#include "bsp/main_menu_mission_detail.hpp"
#include <string>

namespace bsp {
namespace {
// Explicit native binary32 stores, retaining the caller's x87 control word.
float add(float a, float b) noexcept {
    float result;
    __asm {
        fld dword ptr [a]
        fadd dword ptr [b]
        fstp dword ptr [result]
    }
    return result;
}
float subtract(float a, float b) noexcept {
    float result;
    __asm {
        fld dword ptr [a]
        fsub dword ptr [b]
        fstp dword ptr [result]
    }
    return result;
}
float multiply(float a, float b) noexcept {
    float result;
    __asm {
        fld dword ptr [a]
        fmul dword ptr [b]
        fstp dword ptr [result]
    }
    return result;
}
float multiply_add(float a, float b, float c) noexcept {
    float result;
    __asm {
        fld dword ptr [a]
        fmul dword ptr [b]
        fadd dword ptr [c]
        fstp dword ptr [result]
    }
    return result;
}
std::string icon_name(std::string_view prefix, std::uint32_t index) {
    return std::string(prefix) + std::to_string(index + 1u) + std::string(kMissionMapIconSuffix);
}
}

MainMenuMapZoomStep main_menu_map_zoom_step_00588c8b(float zoom,
    float carried_delta, float input_delta, bool immediate_when_zooming) noexcept {
    float fraction = 0.2f; // 00CE54A0
    if (carried_delta != 0.0f || input_delta != 0.0f) {
        if (immediate_when_zooming || carried_delta != 0.0f) fraction = 1.0f;
        const float delta = carried_delta != 0.0f ? carried_delta : input_delta;
        // 00D7A3A0 is exactly double(float(0.1)), not the nearest double 0.1.
        zoom = multiply_add(delta, 0.1f, zoom);
        if (zoom < 0.5f) zoom = 0.5f;
        else if (zoom > 1.5f) zoom = 1.5f;
    }
    return {zoom, fraction};
}

std::array<float, 3> main_menu_map_lerp_005803e0(
    const std::array<float, 3>& old_position,
    const std::array<float, 3>& target, float fraction) noexcept {
    std::array<float, 3> differences{}, products{}, result{};
    for (std::size_t i = 0; i < result.size(); ++i)
        differences[i] = subtract(target[i], old_position[i]);
    for (std::size_t i = 0; i < result.size(); ++i)
        products[i] = multiply(differences[i], fraction);
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = add(old_position[i], products[i]);
    return result;
}

void update_main_menu_map_geometry_00588c70(MainMenuMapGeometryState& state,
    const MainMenuMapGeometryInput& input, MainMenuMapGeometryHost& host) {
    const auto step = main_menu_map_zoom_step_00588c8b(state.zoom,
        state.carried_zoom_delta, input.zoom_delta, input.immediate_when_zooming);
    state.zoom = step.zoom;
    if (!input.has_selected_point_widget) return;

    const auto selected = host.point_at(input.point_list_index, host.selected_mission_index());
    // 00588DB2..00588E32: each point product and offset sum is stored before
    // their addition. The point's third component is never read by geometry.
    const float scaled_x = multiply(selected[0], state.zoom);
    const float scaled_y = multiply(selected[1], state.zoom);
    const std::array<float, 3> target{
        add(add(input.additional_offset[0], state.base_offset[0]), scaled_x),
        add(add(state.base_offset[1], input.additional_offset[1]), scaled_y),
        add(add(state.base_offset[2], input.additional_offset[2]), 0.0f)};
    const auto anchor = main_menu_map_lerp_005803e0(state.anchor, target, step.anchor_fraction);
    state.anchor = anchor; // 00588E5B..00588E64, before widget callbacks

    for (std::uint32_t i = 0; i < host.current_group_mission_count(); ++i) {
        const auto point_widget = host.find_map_point(input.lookup_root,
            icon_name(kMissionMapPointPrefix, i), 1);
        const auto flag_widget = host.find_map_flag(input.lookup_root,
            icon_name(kMissionMapFlagPrefix, i), 1);
        if (point_widget == 0 || flag_widget == 0) continue;

        const auto point = host.point_at(input.point_list_index, i);
        const bool selected_marker = i == host.selected_mission_index();
        const std::array<float, 3> position{
            subtract(multiply_add(point[0], state.zoom, state.backdrop_position[0]), anchor[0]),
            subtract(multiply_add(state.zoom, point[1], state.backdrop_position[1]), anchor[1]),
            subtract(5.0f, anchor[2])}; // 00D7A370, unrelated to stored point z
        host.move_map_point(point_widget, {add(position[0], 0.0f),
            add(0.0f, position[1]), add(selected_marker ? -1.0f : 0.0f, position[2])});
        // Double constants at 00CEFB88/00CEFB80 are promoted float values.
        host.move_map_flag(flag_widget, {subtract(position[0], 0.0031250000465661287f),
            subtract(position[1], 0.0069444444961845875f), add(position[2], 0.0f)});
    }

    host.move_backdrop({subtract(state.backdrop_position[0], anchor[0]),
        subtract(state.backdrop_position[1], anchor[1]),
        subtract(state.backdrop_position[2], anchor[2])});
    host.resize_backdrop({multiply(state.backdrop_extent[0], state.zoom),
        multiply(state.zoom, state.backdrop_extent[1])});
    const auto selected_position = host.selected_point_position();
    host.move_selection_companion({add(0.0f, selected_position[0]),
        add(selected_position[1], 0.0f), subtract(selected_position[2], 1.0f)});
}
} // namespace bsp
