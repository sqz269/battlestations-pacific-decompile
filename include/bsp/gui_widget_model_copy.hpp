#pragma once
#include "bsp/gui_widget_copy.hpp"

namespace bsp {
class GuiNativeGeometryOwners;
struct NativeMaterialDestructionAccess;
struct NativeStreamCloneServices;

// Concrete AA9520 current10 binding. The same Model/mesh/material/stream
// domains and live profile views must outlive all produced owners. This
// adapter owns no native state and supplies no alternate renderer or pool.
class GuiWidgetModelCopyRuntime final : public GuiWidgetModelCopyCalls {
public:
    GuiWidgetModelCopyRuntime(GuiWidgetOwnerRuntime&, GuiNativeGeometryOwners&,
        NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* material_current_vtable_00d5e520,
        const volatile std::uint32_t* mesh_current_vtable_00d62d60,
        NativeStreamCloneServices&);
    NativeModelReference* clone_current10(NativeModelOwner&, std::uint32_t flags,
        NativeNodeBinding* parent, NativeGuiTextModelCloneAcquired&) override;
private:
    GuiWidgetOwnerRuntime& widgets_;
    GuiNativeGeometryOwners& geometry_;
    NativeMaterialDestructionAccess& materials_;
    const volatile std::uint32_t* material_profile_;
    const volatile std::uint32_t* mesh_profile_;
    NativeStreamCloneServices& streams_;
};
} // namespace bsp
