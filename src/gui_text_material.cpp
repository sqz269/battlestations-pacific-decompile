#include "bsp/gui_text_material.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void retain_actual(void* identity) noexcept {
    auto* count = reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4);
    count->fetch_add(1, std::memory_order_seq_cst);
}
void publish_resource(void*& slot, void* incoming, NativeRenderActualOwners& owners) {
    void* const old = slot;
    if (old == incoming) return;
    slot = incoming;
    if (incoming) retain_actual(incoming);
    if (old) release_native_render_actual_owner(owners, old);
}
template<class Function> Function current_slot(void* object, std::size_t offset) {
    require(object != nullptr, "Text shader factory requires the live renderer");
    const auto* table = *static_cast<const std::uintptr_t* const*>(object);
    require(table && table[offset / 4], "Text renderer48 has no actual callable binding");
    return reinterpret_cast<Function>(table[offset / 4]);
}
struct BorrowedNameHeader {
    std::uint32_t length;
    const char* data;
};
static_assert(sizeof(BorrowedNameHeader) == 8 && offsetof(BorrowedNameHeader, data) == 4);
void* load_effect(const void* name, GuiTextShaderServices& services) {
    using Load = void* (__thiscall*)(void*, const void*);
    void* renderer = services.buffers.current_renderer_00f8d394;
    return current_slot<Load>(renderer, 0x48)(renderer, name);
}
void publish_shader(GuiTextLifetime& lifetime, void* shader) noexcept {
    lifetime.cached_shader_slot_1ec() = shader;
    lifetime.text().has_cached_shader = shader != nullptr;
}
bool reference_height_matches(std::int32_t height) noexcept {
    float converted;
    __asm {
        mov eax, height
        cvtsi2ss xmm0, eax
        movss converted, xmm0
    }
    return converted == 720.0f; //00D5C56C, ordered equality like UCOMISS/JP.
}
} // namespace

void set_native_material_shader_00b19210(NativeMaterialStorage& material,
    void* effect, NativeMaterialDestructionAccess& access) {
    publish_resource(material.effect_7c, effect, access.retained_owners);
    //00B1924B reloads after the old effect's potentially terminal callback.
    if (void* current = material.effect_7c)
        *(static_cast<std::uint8_t*>(current) + 0xb4) = 1;
    for (std::int32_t index = 0; index < material.parameter_count_100; ++index) {
        require(index < 32, "Native material parameter extent exceeds32");
        void* const parameter = material.parameters_80[static_cast<std::size_t>(index)];
        if (parameter) {
            destroy_native_string_header_0041dd20(parameter, access.parameter_names);
            //B17AF0 is the same F8D3E4/88h return algorithm reconstructed by
            //the concrete parameter pool's B193FA inline-return entry.
            access.parameter_slots.return_slot_00b193fa_fragment(parameter);
        }
    }
    material.parameter_count_100 = 0; //Negative native count also reaches zero.
}

void set_native_material_texture_00b189f0(NativeMaterialStorage& material,
    std::uint32_t index, void* texture, NativeRenderActualOwners& owners) {
    require(index < material.textures_10.size() && material.texture_count_34 >= 0 &&
        material.texture_count_34 <= 9, "Native material texture slot/count is outside0..8/0..9");
    if (index >= static_cast<std::uint32_t>(material.texture_count_34))
        material.texture_count_34 = static_cast<std::int16_t>(index + 1u);
    publish_resource(material.textures_10[index], texture, owners);
}

NativeMaterialParameterStorage* register_native_material_float4_00b18aa0(
    NativeMaterialStorage& material, const void* name, const void* source,
    NativeMaterialParameterAccess& access) {
    return register_native_material_parameter_00b17e10(material, name, source, 4, 0, access);
}
NativeMaterialParameterStorage* register_native_material_float2_00b18b00(
    NativeMaterialStorage& material, const void* name, const void* source,
    NativeMaterialParameterAccess& access) {
    return register_native_material_parameter_00b17e10(material, name, source, 2, 0, access);
}
NativeMaterialParameterStorage* register_native_material_float_00b18b20(
    NativeMaterialStorage& material, const void* name, const void* source,
    NativeMaterialParameterAccess& access) {
    return register_native_material_parameter_00b17e10(material, name, source, 1, 0, access);
}

bool ensure_gui_text_font_shader_00ab8ce0(GuiTextLifetime& lifetime,
    GuiTextShaderServices& services) {
    if (lifetime.cached_shader_slot_1ec()) return false;
    auto& text = lifetime.text();
    require(text.shader_name.size() <= (std::numeric_limits<std::uint32_t>::max)() &&
        text.shader_name.find('\0') == std::string::npos,
        "Text shader name requires the valid native string domain");
    //Native constructs/destroys an empty comparison wrapper; its null buffer
    //does not allocate. Existing semantic source supplies the same length test.
    if (!text.shader_name.empty()) {
        const BorrowedNameHeader name{
            static_cast<std::uint32_t>(text.shader_name.size()), text.shader_name.c_str()};
        //Renderer B318B0 copies the supplied header/data; it does not retain
        //or mutate this wrapper. No extra override-name allocation is inserted.
        publish_shader(lifetime, load_effect(&name, services));
        return true;
    }
    void* configuration = services.configuration_0109cf04;
    require(configuration != nullptr, "Text shader selection has no live configuration");
    std::int32_t reference_height;
    std::memcpy(&reference_height, static_cast<std::byte*>(configuration) + 0x28, 4);
    bool use_point = false;
    if (reference_height_matches(reference_height)) {
        //Native reads the font only on the ordered-height-match arm.
        require(text.font != nullptr, "Text shader height match requires a live font");
        use_point = text.font->scale_ratio == 1.0f; //00D7A24C.
    }
    NativeString name;
    name.assign_0041e870(services.buffers.strings,
        use_point ? "GuiFont.mshd" : "GuiFontBilinear.mshd");
    try {
        publish_shader(lifetime, load_effect(&name, services));
    } catch (...) {
        destroy_native_string_header_0041dd20(&name, services.buffers.strings);
        throw;
    }
    //Publish is before cleanup, including when the renderer returned null.
    destroy_native_string_header_0041dd20(&name, services.buffers.strings);
    return true;
}
} // namespace bsp
