#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace bsp {
// Projected fields of the screen, not a native object overlay.
struct MainMenuMapGeometryState {
    float zoom{1.0f};                         // +130h
    std::array<float, 3> anchor{};            // +184h..18Ch
    std::array<float, 3> base_offset{};       // +190h..198h
    float carried_zoom_delta{};              // +19Ch, read but not consumed/cleared
    std::array<float, 3> backdrop_position{}; // +124h..12Ch
    std::array<float, 2> backdrop_extent{};   // +11Ch..120h
};

struct MainMenuMapGeometryInput {
    std::uintptr_t lookup_root{};            // Native second stack dword
    float zoom_delta{};                      // Third stack dword
    std::size_t point_list_index{};           // Fourth; +134h + index*10h
    std::array<float, 3> additional_offset{}; // Fifth points here, caller uses +1A4h
    bool immediate_when_zooming{};           // Low byte of sixth stack dword
    bool has_selected_point_widget{};        // screen+330h != 0
};

struct MainMenuMapZoomStep {
    float zoom;
    float anchor_fraction;
};

// 00588C8B..00588D52. Unlike the older apply_map_zoom_00588c70 delta-only
// projection, carried_delta overrides input_delta. No active delta means no
// clamp and fraction 0.2 even when immediate_when_zooming is true.
MainMenuMapZoomStep main_menu_map_zoom_step_00588c8b(float zoom,
    float carried_delta, float input_delta, bool immediate_when_zooming) noexcept;

// 005803E0, native ECX=result XYZ, EDX=old XYZ, stack=(target XYZ*, fraction*),
// RET8, EAX=result pointer. Separate binary32 difference/product/add stores.
// This value-returning interface does not reproduce the original ABI.
std::array<float, 3> main_menu_map_lerp_005803e0(
    const std::array<float, 3>& old_position,
    const std::array<float, 3>& target, float fraction) noexcept;

// Native library vector 004215D0 stores 12-byte entries; the already existing
// std::array<float,3> convention in MissionDetailHost is reused here. STL
// allocation, iterator checking and string temporaries remain host/library work.
struct MainMenuMapGeometryHost {
    virtual ~MainMenuMapGeometryHost() = default;
    // Data reads, not additional game calls. point_at must enforce the native
    // non-null/range precondition; it must not invent missing points. List index
    // and published group index are distinct selectors in the original routine.
    virtual std::uint32_t selected_mission_index() = 0; // 00E194DC, reloaded for markers
    virtual std::size_t current_group_mission_count() = 0; // 00E198AC+5Ch, group 00E194D8
    virtual std::array<float, 3> point_at(std::size_t list, std::uint32_t index) = 0;

    // One method per game call site; both lookups run even if the first misses.
    // The final literal 1 is ignored by 00AA7E00: it searches direct children.
    virtual std::uintptr_t find_map_point(std::uintptr_t root,
        std::string_view name, int unused_argument) = 0; // 00588F9C -> 00AA7E00
    virtual std::uintptr_t find_map_flag(std::uintptr_t root,
        std::string_view name, int unused_argument) = 0; // 005890FE -> 00AA7E00
    virtual void move_map_point(std::uintptr_t widget,
        const std::array<float, 3>& position) = 0;  // 005892F0 -> 00AA8240
    virtual void move_map_flag(std::uintptr_t widget,
        const std::array<float, 3>& position) = 0;  // 00589326 -> 00AA8240
    virtual void move_backdrop(const std::array<float, 3>& position) = 0; // 00589378, +328h
    virtual void resize_backdrop(const std::array<float, 2>& extent) = 0; // 005893B5, virtual+58h
    virtual std::array<float, 3> selected_point_position() = 0; // 005893C5 -> 00AA6750, +330h
    virtual void move_selection_companion(const std::array<float, 3>& position) = 0; // 0058940B, +338h
};

// 00588C70, ECX=screen, six stack dwords, RET18h, no consumed return value.
// First stack argument is frame seconds and is never read; omitted here. The
// supplied root is used for lookup, not silently replaced with screen+110h.
// Updates zoom even without +330h; all geometry work requires that widget.
// Host must keep borrowed native screen/list ownership valid during callbacks.
// SEH, allocator failures, duplicate STL validation calls and exact exceptional
// x87 status/trap timing are outside this typed reconstruction.
void update_main_menu_map_geometry_00588c70(MainMenuMapGeometryState& state,
    const MainMenuMapGeometryInput& input, MainMenuMapGeometryHost& host);
} // namespace bsp
