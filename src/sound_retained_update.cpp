#include "bsp/sound_retained_update.hpp"

namespace bsp {
namespace {
// D7A24C is the float bit pattern 3F800000. Keep COMISS rather than C++
// min/max: the lower x87 JA and upper SSE JBE both retain an unordered ramp.
float retained_ramp(float current, const float& native_68, const float& native_6c) noexcept {
    const float one = 1.0f;
    const float* lower = &native_68;
    const float* upper = &native_6c;
    float lower_copy, ratio, gain;
    __asm {
        movss xmm1, current
        xorps xmm0, xmm0
        comiss xmm1, xmm0
        movss gain, xmm0
        jbe finished
        mov eax, lower
        fld dword ptr [eax]
        fstp lower_copy
        fld current
        fld lower_copy
        fld st(0)
        fsubp st(2), st(0)
        mov eax, upper
        fsubr dword ptr [eax]
        fdivp st(1), st(0)
        fstp ratio
        fld1
        fldz
        fsub st(1), st(0)
        fxch st(1)
        fmul ratio
        faddp st(1), st(0)
        fstp gain
        fld gain
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        ja publish
        movss xmm0, gain
        movss xmm1, one
        comiss xmm0, xmm1
        jbe publish
        movaps xmm0, xmm1
    publish:
        movss gain, xmm0
    finished:
    }
    return gain;
}

float nonpositive_gain(float current) noexcept {
    const float one = 1.0f;
    float gain;
    __asm {
        movss xmm0, current
        xorps xmm1, xmm1
        comiss xmm0, xmm1
        movss xmm2, one
        jbe nonpositive_or_unordered
        movaps xmm0, xmm1
        jmp clamp_lower
    nonpositive_or_unordered:
        movaps xmm0, xmm2
    clamp_lower:
        comiss xmm1, xmm0
        jbe clamp_upper
        movss gain, xmm1
        jmp finished
    clamp_upper:
        comiss xmm0, xmm2
        jbe publish
        movss gain, xmm2
        jmp finished
    publish:
        movss gain, xmm0
    finished:
    }
    return gain;
}

float scaled_gain(const float& native_70, float gain) noexcept {
    const float* multiplier = &native_70;
    float result;
    __asm {
        mov eax, multiplier
        fld dword ptr [eax]
        fmul gain
        fstp result
    }
    return result;
}

void publish_and_update(SoundChannelInstance& instance, float gain, float dt,
    std::uint32_t listener, SoundInstanceContext& context) {
    void* options = sound_sample_options_00a81860(instance.sample_4c);
    float* volume = &instance.volume_24;
    std::uint8_t* dirty = &instance.dirty_14;
    float copied_dt;
    __asm {
        mov eax, options
        fld dword ptr [eax + 4]
        fmul gain
        mov eax, volume
        fstp dword ptr [eax]
        fld dt
        mov eax, dirty
        mov byte ptr [eax], 1
        fstp copied_dt
    }
    update_sound_channel_00a7af10(instance, copied_dt, listener, context);
}
} // namespace

void update_retained_sound_ramp_00a7b520(SoundChannelInstance& instance,
    const float& native_68, const float& native_6c, float dt,
    std::uint32_t listener, SoundInstanceContext& context) {
    const float current = context.current_owner_00f8bbd8->listener.transform_c4[13];
    const float gain = retained_ramp(current, native_68, native_6c);
    publish_and_update(instance, gain, dt, listener, context);
}

void update_retained_sound_scaled_ramp_00a7b5d0(SoundChannelInstance& instance,
    const float& native_68, const float& native_6c, const float& native_70,
    float dt, std::uint32_t listener, SoundInstanceContext& context) {
    const float current = context.current_owner_00f8bbd8->listener.transform_c4[13];
    const float gain = retained_ramp(current, native_68, native_6c);
    // B64D..B657 multiplies and rounds BEFORE A81860 returns sample options.
    const float scaled = scaled_gain(native_70, gain);
    publish_and_update(instance, scaled, dt, listener, context);
}

void update_retained_sound_nonpositive_00a7b690(SoundChannelInstance& instance,
    float dt, std::uint32_t listener, SoundInstanceContext& context) {
    const float current = context.current_owner_00f8bbd8->listener.transform_c4[13];
    publish_and_update(instance, nonpositive_gain(current), dt, listener, context);
}
} // namespace bsp
