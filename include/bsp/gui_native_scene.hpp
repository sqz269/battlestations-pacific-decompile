#pragma once
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_group_owner.hpp"
#include <memory>
#include <unordered_map>

namespace bsp {

// Actual plain-page cGroup owners and dispatch through the existing shared node
// lifetime runtime. Child models remain owned by GuiWidgetOwnerRuntime. This
// companion creates no parallel hierarchy, transform, count or backlink array.
// One explicit process callback binding bridges CameraTransform's native-style
// void(group_identity) notification ABI; a second live binding is rejected.
class GuiNativeScene final : public GuiWidgetNativeCalls {
public:
    explicit GuiNativeScene(NativeGroupEnvironment&);
    ~GuiNativeScene() override;
    GuiNativeScene(const GuiNativeScene&) = delete;
    GuiNativeScene& operator=(const GuiNativeScene&) = delete;

    NativeNodeBinding& resolve_node(CameraTransform&) override;
    void set_parent_00b6e680(NativeNodeBinding&, NativeNodeBinding*) override;
    // Same00AA5840 plain-root allocation/construction path, with required real
    // group reference registration. The caller prepares its per-page layer
    // constructor inputs (including the exact screen flag) before invoking this.
    // It then uses this same scene in GuiWidgetOwnerEnvironment.native.
    GuiWidgetOwner& create_plain_page_root_00aa5840_fragment(
        GuiWidgetOwnerRuntime&, GuiLayoutWidget&);
    void clear_page_root_registration_00b6d890(GuiWidgetOwner&);
    std::size_t retained_group_count() const noexcept { return groups_.size(); }
    NativeNodeParentingRuntime& parenting() noexcept { return parenting_; }
private:
    struct GroupRecord;
    NativeGroupEnvironment& environment_;
    NativeNodeParentingRuntime parenting_;
    std::unordered_map<void*, std::unique_ptr<GroupRecord>> groups_;
    NativeGroupReference& create_group(const std::string&);
    static void retire_group(void*, NativeGroupReference&) noexcept;
    static void set_attachment(NativeNodeParentingRuntime&, CameraTransform&, void*);
    static void notify_group(void* actual_group);
};

} // namespace bsp
