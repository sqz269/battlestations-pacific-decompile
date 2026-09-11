#pragma once

#include "bsp/gameplay_effect_components.hpp"

namespace bsp {
// Required native call sites, without a fabricated renderer or texture.
struct EffectComponentTextureServices {
    virtual ~EffectComponentTextureServices() = default;
    // Resolve CURRENT renderer [00F8D394] and its CURRENT vtable+64h here.
    // Native stack arguments are &name, flags; EAX is an owned texture ref.
    virtual void* load_texture_current_slot_64(const NativeString& name,
        std::uint32_t flags) = 0;
    // Dispatch the CURRENT texture slot0 after its actual refs+4 reach zero.
    virtual void texture_zero_references_slot_00(void*) = 0;
};
struct EffectScalarComponentContext {
    NativeStringStorage& strings;
    const bool& crt_sse2_conversion;
    EffectComponentTextureServices& textures;
};

// Native readers: ECX actual component, stack retained LuaObject, RET4.
// Rebuilt interfaces use host refs; they are not native ABI replacements.
void read_effect_shake_00868de0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_rumble_base_00868ec0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_const_rumble_00868fa0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_slope_rumble_00869080(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_square_rumble_00869160(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_splash_008694c0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_light_00869fd0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
void read_effect_waterdrops_0086b2b0(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);

// ECX actual component, RET. Name headers remain dangling as in the image.
void destroy_effect_component_base_0086b7e0(void*, NativeStringStorage&) noexcept;
void destroy_effect_waterdrops_0086cd40(void*, EffectScalarComponentContext&);
// ECX component, stack flags, EAX original pointer, RET4. Only bit0 frees.
void* scalar_delete_effect_shake_0086d0a0(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_waterdrops_0086d0c0(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_const_rumble_0086d0e0(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_slope_rumble_0086d100(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_square_rumble_0086d120(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_light_0086d140(void*, std::uint32_t, EffectScalarComponentContext&);
void* scalar_delete_effect_splash_0086d160(void*, std::uint32_t, EffectScalarComponentContext&);

// Read CURRENT component vtable at each dispatch. Seven read-only native
// tables have concrete readers and scalar cleanup; every other table goes
// to the required remaining services. Reuse this for definition destruction.
class GameplayEffectScalarComponentDispatcher final : public GameplayEffectComponentServices {
public:
    GameplayEffectScalarComponentDispatcher(EffectScalarComponentContext& context,
        GameplayEffectComponentServices& remaining) noexcept
        : context_(context), remaining_(remaining) {}
    void read_lua_slot_14(void*, GuiLua51Host&, const GuiLuaRef&) override;
    void zero_references_slot_00(void*) override;
private:
    EffectScalarComponentContext& context_;
    GameplayEffectComponentServices& remaining_;
};
} // namespace bsp
