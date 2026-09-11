#pragma once
#include "bsp/main_menu_map_point_geometry.hpp"
#include "bsp/mission_briefing_start.hpp"
#include <vector>

namespace bsp {
struct GuiLayoutWidget;
class GuiWidgetOwner;
class GuiWidgetOwnerRuntime;

// Caller-owned live screen+134h lists. Mission detail fills each list from the
// installed point icons (0058C662..0058C6B3); this adapter never synthesizes it.
using MainMenuMapPointLists = std::array<std::vector<std::array<float, 3>>, 5>;

// Concrete owner adapter for the existing complete typed 00588C70 sequence.
// All constructor arguments are BORROWED, including the pointer slots, so
// later binding/selection/list changes remain visible. Keep them, their GUI
// pages, the retained owners and real renderer services alive through update.
// No native object layout, widget-pointer ABI or executable replacement claim.
class MainMenuMapGeometryRuntime final : public MainMenuMapGeometryHost {
public:
    MainMenuMapGeometryRuntime(GuiWidgetOwnerRuntime&, const MissionTreeTables&,
        const MainMenuMissionSelection&, const MainMenuMapPointLists&,
        GuiLayoutWidget*& backdrop_328,
        GuiLayoutWidget*& selected_map_point_330,
        GuiLayoutWidget*& selector_338) noexcept;

    // input.lookup_root is the identity of a live GuiLayoutWidget, cast to
    // uintptr_t, as are host lookup results. It is not a native game address.
    // Recompute the +330h gate from its actual borrowed slot; preserve every
    // other input. This call already applies zoom: do not apply a second delta.
    void update(MainMenuMapGeometryState&, const MainMenuMapGeometryInput&);

    std::uint32_t selected_mission_index() override;
    std::size_t current_group_mission_count() override;
    std::array<float, 3> point_at(std::size_t, std::uint32_t) override;
    std::uintptr_t find_map_point(std::uintptr_t, std::string_view, int) override;
    std::uintptr_t find_map_flag(std::uintptr_t, std::string_view, int) override;
    void move_map_point(std::uintptr_t, const std::array<float, 3>&) override;
    void move_map_flag(std::uintptr_t, const std::array<float, 3>&) override;
    void move_backdrop(const std::array<float, 3>&) override;
    void resize_backdrop(const std::array<float, 2>&) override;
    std::array<float, 3> selected_point_position() override;
    void move_selection_companion(const std::array<float, 3>&) override;

private:
    GuiWidgetOwnerRuntime& owners_;
    const MissionTreeTables& missions_;
    const MainMenuMissionSelection& selection_;
    const MainMenuMapPointLists& points_;
    GuiLayoutWidget* const& backdrop_;
    GuiLayoutWidget* const& selected_point_;
    GuiLayoutWidget* const& selector_;

    GuiWidgetOwner& require_owner(GuiLayoutWidget*) const;
    std::uintptr_t find(std::uintptr_t, std::string_view);
    void move(GuiLayoutWidget*, const std::array<float, 3>&);
};
} // namespace bsp
