#include "bsp/gui_native_clip_parameters.hpp"
#include "bsp/gui_clip_box.hpp"
#include "bsp/native_material_owner.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
class ParameterName {
public:
    explicit ParameterName(NativeStringStorage& storage) : storage_(storage) {}
    ~ParameterName() { destroy_native_string_header_0041dd20(&value, storage_); }
    NativeString value;
private:
    NativeStringStorage& storage_;
};
}

void register_native_gui_clip_parameters_00aa9f10(GuiWidgetOwner& widget,
    NativeMaterialStorage& material, GuiWidgetOwnerRuntime& widgets,
    const float& aspect_ratio, NativeMaterialParameterAccess& access) {
    if (&widgets.owner(widget.layout()) != &widget)
        throw std::logic_error("Native clip parameters require the same retained widget owner");
    auto& actual = widgets.environment().models.retained_owners.resolve_actual(&material);
    auto* reference = dynamic_cast<NativeMaterialReference*>(&actual);
    if (!reference || &reference->storage() != &material || reference->reference_count.load() <= 0)
        throw std::logic_error("Native clip parameters require their actual live material owner");

    auto* ancestor = &widget.layout();
    while (ancestor && ancestor->transform.type_id != 16) ancestor = ancestor->parent;
    auto& enabled = widget.extra_fields().clip_enabled_e8;
    enabled = ancestor ? 1.0f : 0.0f; //00AA9F62, before name allocation.
    {
        ParameterName name(access.parameter_names);
        name.value.resize_0041dd40(access.parameter_names, 5, true); //00AA9F6E
        if (name.value.data())
            std::memcpy(name.value.data(), "cClip", name.value.length() + 1u);
        //00AA9FA1 ->B18B20 ->B17E10(name,source,1,0).
        register_native_material_parameter_00b17e10(material, &name.value, &enabled, 1, 0, access);
    } //00AA9FC7 releases the name BEFORE the live flag reload.
    if (enabled == 0.0f) return; //00AA9FCC..9FDB: ordered zero only; NaN stays active.
    if (!ancestor)
        throw std::logic_error("Active clip flag has no captured native ClipBox ancestor");
    {
        ParameterName name(access.parameter_names);
        name.value.assign_0041e870(access.parameter_names, "cClipCenter"); //00AA9FEA
        const auto sources = gui_clip_box_parameter_sources(widgets.owner(*ancestor));
        register_native_material_parameter_00b17e10(material, &name.value,
            sources.center_ec, 2, 0, access); //00AAA005 ->B18B00
    }
    {
        ParameterName name(access.parameter_names);
        name.value.assign_0041e870(access.parameter_names, "cClipBorder"); //00AAA036
        const auto sources = gui_clip_box_parameter_sources(widgets.owner(*ancestor));
        register_native_material_parameter_00b17e10(material, &name.value,
            sources.border_f4, 4, 0, access); //00AAA053 ->B18AC0(count1)
    }
    {
        ParameterName name(access.parameter_names);
        name.value.assign_0041e870(access.parameter_names, "cAspectRatio"); //00AAA084
        register_native_material_parameter_00b17e10(material, &name.value,
            &aspect_ratio, 1, 0, access); //00AAA09D ->B18B20
    }
}
} // namespace bsp
