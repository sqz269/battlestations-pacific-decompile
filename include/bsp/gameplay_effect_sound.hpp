#pragma once
#include "bsp/gameplay_effect_components.hpp"
#include "bsp/sound_sample_cache.hpp"

namespace bsp {
struct EffectSoundHost : GameplayEffectComponentLifetime, SoundLevelNameHost {
    virtual SoundAuxiliaryTreeOwner& current_sample_cache_00f8bbe8() = 0;
    virtual SoundSystemOwner& current_sound_owner_00f8bbd8() = 0;
};
struct EffectSoundContext {
    NativeStringStorage& strings;
    const bool& crt_sse2_conversion;
    SoundSampleCacheContext& cache;
    EffectSoundHost& host;
};
//005B8EA0/005B8FA0/005B9AF0 have the same actual12h intrusive array rules
// as0086E770/0086EDD0/0086EB60. Preserve live count/header/callback ordering.
void reserve_effect_sound_samples_005b8ea0(void*, std::int32_t, GameplayEffectComponentLifetime&);
void resize_effect_sound_samples_005b8fa0(void*, std::int32_t, GameplayEffectComponentLifetime&);
void append_effect_sound_sample_005b9af0(void*, const void*, GameplayEffectComponentLifetime&);
// ECX actual3Ch Sound component; stack LuaObject; RET4. Appends samples;
// SampleTable scans1..first nil with separate probe/read references, else
// use Sample. Category must convert to nonnull C string as in native code.
void read_effect_sound_0086ef60(void*, GuiLua51Host&, const GuiLuaRef&, EffectSoundContext&);
// ECX actual Sound; RET. Resize samples0, free current buffer, common name
// cleanup. Scalar ECX Sound, stack flags, EAX original pointer, RET4.
void destroy_effect_sound_0086fa90(void*, EffectSoundContext&);
void* scalar_delete_effect_sound_0086fb00(void*, std::uint32_t, EffectSoundContext&);
} // namespace bsp
