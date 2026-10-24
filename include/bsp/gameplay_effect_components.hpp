#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/gui_lua_runtime.hpp"

namespace bsp {
struct EffectFlareDestructionHost {
    virtual ~EffectFlareDestructionHost() = default;
    virtual void string_destruct_0041dd20(void* header) = 0;
    virtual void free_00bf65ac(void* memory) = 0;
    virtual void free_00bf6989(void* memory) = 0;
    virtual void resize_words_0086aa10(void* vector, std::uint32_t count) = 0;
    virtual void return_string_00419cc0_00bd1510(void* data, std::uint32_t bytes) = 0;
    virtual void refcounted_base_00bd30f0(void* component) = 0;
};
// 0086E5A0: ECX=flare, plain RET; full normal teardown, no flag argument.
void destruct_effect_flare_0086e5a0(void* flare, EffectFlareDestructionHost& host);
// Actual raw storage, including padding and fields not initialized by the
// native constructor. Caller must supply the indicated allocation extent.
// These functions return the allocation base; Tracer's component is base+80h.
void* construct_effect_sound_00870733_fragment(void*) noexcept; //3Ch
void* construct_effect_particle_0086bc80(void*) noexcept; //34h
void* construct_effect_tracer_0086b8b0(void*) noexcept; //A0h
void* construct_effect_water_tracer_0086bcc0(void*) noexcept; //C4h
void* construct_effect_shake_0086bd30(void*) noexcept; //2Ch
void* construct_effect_flare_0086bd70(void*) noexcept; //34h
void* construct_effect_thunderstorm_0086fec0(void*) noexcept; //58h
void* construct_effect_waterdrops_0086bdc0(void*) noexcept; //4Ch
void* construct_effect_const_rumble_0086cf90(void*) noexcept; //30h
void* construct_effect_slope_rumble_0086cfe0(void*) noexcept; //30h
void* construct_effect_square_rumble_0086d030(void*) noexcept; //40h
void* construct_effect_light_0086be00(void*) noexcept; //48h
void* construct_effect_splash_0086be40(void*) noexcept; //2Ch

struct GameplayEffectComponentServices : GameplayEffectComponentLifetime {
    // Actual component pointer, including Tracer's secondary base. Dispatch
    // CURRENT vtable+14 with the retained Lua row. No default payload reader.
    virtual void read_lua_slot_14(void*, GuiLua51Host&, const GuiLuaRef&) = 0;
};
//00870400: ECX actual24h definition; stack Lua row; RET4. Appends to the
// existing component array. Filters reserved keys/platform, constructs one
// of thirteen actual allocations, reads its virtual, names it from a string
// key, retains through concrete array append, then releases the temporary.
void load_gameplay_effect_components_00870400(GameplayEffectDefinition&,
    GuiLua51Host&, const GuiLuaRef&, NativeStringStorage&, GameplayEffectComponentServices&);

//00868BF0: ECX actual component; stack Lua row; RET4. Autostart byte10,
// numeric-only Delay14 default0, numeric-only NoFilterDist18 defaulting to
// CURRENT18 read after field lookup. Integer mode is read after conversion.
void read_effect_component_base_00868bf0(void*, GuiLuaHost&, const GuiLuaRef&,
    const bool& crt_sse2_conversion);
} // namespace bsp
