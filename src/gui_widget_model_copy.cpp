#include "bsp/gui_widget_model_copy.hpp"
#include "bsp/gui_native_geometry.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_stream_clone.hpp"
#include <stdexcept>

namespace bsp {
GuiWidgetModelCopyRuntime::GuiWidgetModelCopyRuntime(GuiWidgetOwnerRuntime& widgets,
    GuiNativeGeometryOwners& geometry, NativeMaterialDestructionAccess& materials,
    const volatile std::uint32_t* material_profile,
    const volatile std::uint32_t* mesh_profile, NativeStreamCloneServices& streams)
    : widgets_(widgets), geometry_(geometry), materials_(materials),
      material_profile_(material_profile), mesh_profile_(mesh_profile), streams_(streams) {}

NativeModelReference* GuiWidgetModelCopyRuntime::clone_current10(NativeModelOwner& source,
    std::uint32_t flags, NativeNodeBinding* parent, NativeGuiTextModelCloneAcquired& acquired) {
    if (parent || (flags != 0x26u && flags != 0x3eu))
        throw std::logic_error("widget Model copy requires established flags26/3E with parent0");
    if (&source.environment != &widgets_.environment().models ||
        &source.environment.retained_owners != &geometry_.actual_owners() ||
        &materials_.retained_owners != &geometry_.actual_owners() || &streams_.geometry != &geometry_)
        throw std::logic_error("widget Model copy must use the same canonical native owners");
    // The caller supplies the live D5C0B8 value. Text/type3 reaches3E, whose
    // positive streams are cloned by real factory/map/copy/unmap operations.
    // Types17/18 retain the existing26 sharing sequence. Never substitute26
    // when a3E provider fails, and never roll back a partially acquired owner.
    if (flags == 0x3eu) {
        clone_native_gui_text_model_00b752b0_flags3e(source, widgets_, geometry_, materials_,
            material_profile_, mesh_profile_, acquired, streams_);
    } else {
        clone_native_gui_text_model_00b752b0(source, widgets_, geometry_, materials_,
            material_profile_, mesh_profile_, acquired);
    }
    return acquired.model; // Same creator is transferred by AA9520 after return.
}
} // namespace bsp
