#pragma once
#include "bsp/gui_icon_runtime.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_material_owner.hpp"
#include <functional>
#include <memory>

namespace bsp {
// Borrow the actual type16 ancestor's fields. The resolver must return stable
// addresses in that ancestor; it must not manufacture a clipping rectangle.
struct GuiClipBoxParameterSources {
    const float (&center_ec)[2];
    const float (&border_f4)[4];
};

using MaterialParameterOwnerAcquire =
    std::function<std::shared_ptr<const void>(const void* same_owner)>;

// Full00B18A40 field/ownership sequence over the EXISTING material projection.
// Native ECX material; stack owner pointer and byte; RET8, final00B18A98/3.
// Releases the old retained owner BEFORE writing/retaining the incoming owner,
// even for equal pointers. The incoming object must survive that release.
// Acquire must return a genuine owning token whose get() equals same_owner;
// it runs only for nonnull incoming and nonzero retain_byte. Its matching
// release belongs to that token. No second reference count or owner map exists.
// A failed host acquisition propagates after the native preceding stores.
void set_material_parameter_owner_00b18a40(MaterialCloneState&,
    const void* incoming, std::uint8_t retain_byte, const MaterialParameterOwnerAcquire&);

struct GuiMaterialBindingServices {
    GuiWidgetOwnerRuntime& widgets;
    const float& aspect_ratio_00e12fc0; // The actual mutable engine global.
    std::function<GuiClipBoxParameterSources(GuiWidgetOwner&)> clip_box_sources;
    // Retain the SAME GuiWidgetOwner AND its layout/borrowed fields until the
    // token releases. get() must be &owner. Current unique page ownership alone
    // cannot supply this contract: a real widget/page lifetime owner is needed.
    std::function<std::shared_ptr<const void>(GuiWidgetOwner&)> retain_widget;
    // SAME canonical domain used by the model's retained resources. Color
    // resolves section+20 to NativeMaterialReference and borrows its actual
    // storage; no MaterialCloneState copy or separate registry is permitted.
    NativeRenderActualOwners& actual_owners;
};

// Full00AA9F10: nearest self/parent type16; writes/borrows SAME widget+E8.
// Inactive registration changes cClip only; prior center/border/aspect records
// remain untouched. Active registration borrows real ancestor/global fields.
// Native ECX widget; stack material; RET4 at00AAA0D7, last instruction
//00AAA0E4 is a five-byte backward JMP. New C++ semantic ABI.
void register_gui_clip_parameters_00aa9f10(GuiWidgetOwner&, MaterialCloneState&,
    const GuiMaterialBindingServices&);

// Raw native accessors; +180 is geometry only in the actual model class.
// A cGroup root stores its child-array capacity integer at the same offset.
//00B74650 ECX node/model ->EAX bool, RET at00B7465B/1.
bool gui_model_has_geometry_00b74650(const NativeModelOwner&) noexcept;
//00B72B40 ECX actual mesh ->DWORD+58, RET at00B72B43/1.
std::uint32_t gui_mesh_element_count_00b72b40(const void* actual_mesh) noexcept;

//00B179F0: ECX actual material; ignored stack slot; EAX material+38; RET4.
// A live alias to the SAME first four lighting words. No copy or flag write.
float* native_material_diffuse_00b179f0(NativeMaterialStorage&,
    std::uint32_t ignored_slot) noexcept;

// Actual +50 of base, Group, Icon, FrameBox and Screen. Stores SAME layout
// color (and its existing alpha projection), then checks raw node+180 and
// mesh+58. Resolves element0's actual section+20 through the same model owner
// domain, verifies its canonical NativeMaterialReference/storage, then writes
// only that SAME raw material diffuse quartet; no dirty
// flag, child traversal, alpha multiplication or named-parameter registration.
// Requires this runtime's canonical live model companion before any write.
// An inherited +50 table entry alone does not validate a group-backed Screen.
// Native ECX widget, stack float4 pointer, RET4 at00AA68E7/3.
void set_gui_color_00aa6870(GuiWidgetOwner&, const float (&rgba)[4],
    const GuiMaterialBindingServices&);

// Installs the two required callbacks over the same supplied owner/services.
// Registration order is AA9F10 then B18A40(widget,1); publication passes the
// widget's own current Color to current+50 (AA6870 for these supported types).
// The services and their referenced actual storage must outlive callbacks and
// every material registration. Registration remains semantic and requires the
// real widget/clip lifetime; this does not complete the native GUI adapter.
// Color publication rejects roots without a canonical model companion.
void bind_gui_material_callbacks(GuiGeometryRuntimeServices&, GuiWidgetOwner&,
    GuiMaterialBindingServices);
} // namespace bsp
