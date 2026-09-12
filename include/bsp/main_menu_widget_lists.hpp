#pragma once
#include <cstdint>
namespace bsp {
struct GuiLayoutWidget;
// Actual twelve-byte node transport on Win32; widgets are borrowed from the
// canonical GUI owner. Native widget bytes are never cast to GuiLayoutWidget.
struct MainMenuCommandWidgetListNode {
    MainMenuCommandWidgetListNode* next;
    MainMenuCommandWidgetListNode* previous;
    GuiLayoutWidget* widget;
};
struct MainMenuCommandWidgetListView {
    MainMenuCommandWidgetListNode* const volatile& head_04;
    const volatile std::uint32_t& count_08;
};
// One screen-owned intrusive list. 59041B..59045B creates lists280/298/2A4
// through582280. Command/selection readers borrow these exact head/count cells.
class MainMenuObjectiveWidgetList final {
public:
    MainMenuObjectiveWidgetList();
    ~MainMenuObjectiveWidgetList(); // node ownership only; widgets borrowed
    MainMenuObjectiveWidgetList(const MainMenuObjectiveWidgetList&) = delete;
    MainMenuObjectiveWidgetList& operator=(const MainMenuObjectiveWidgetList&) = delete;
    MainMenuCommandWidgetListView command_view() const noexcept { return {head_04_, count_08_}; }
    void append_005823b0_005896f0(GuiLayoutWidget&);
private:
    MainMenuCommandWidgetListNode* head_04_;
    std::uint32_t count_08_{};
};
} // namespace bsp
