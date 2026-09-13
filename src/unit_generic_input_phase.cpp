#include "bsp/unit_generic_input_phase.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Generic unit input requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_after_shot_fire_time) == 0x30);
static_assert(offsetof(GameplayTuningSettings, player_artillery_throw_throw_decrement_time) == 0x40);

template<class T> T read(const void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(base) + offset);
}
void copy_float(volatile float& destination, const volatile float& source) noexcept {
    volatile float* const output_address = &destination;
    const volatile float* const input_address = &source;
    __asm { mov eax, input_address }
    __asm { movss xmm0, dword ptr [eax] }
    __asm { mov eax, output_address }
    __asm { movss dword ptr [eax], xmm0 }
}
bool sse_above(const volatile float& left, const volatile float& right) noexcept {
    const volatile float* const a = &left;
    const volatile float* const b = &right;
    std::uint8_t result;
    __asm {
        mov eax, a
        movss xmm0, dword ptr [eax]
        mov eax, b
        comiss xmm0, dword ptr [eax]
        seta result
    }
    return result != 0;
}
bool x87_below(const volatile float& left, const volatile float& right) noexcept {
    const volatile float* const a = &left;
    const volatile float* const b = &right;
    std::uint8_t result;
    __asm {
        mov eax, b
        fld dword ptr [eax]
        mov eax, a
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        setb result
    }
    return result != 0; // Includes unordered, exactly the native JC.
}
bool x87_above(const volatile float& left, const volatile float& right) noexcept {
    const volatile float* const a = &left;
    const volatile float* const b = &right;
    std::uint8_t result;
    __asm {
        mov eax, b
        fld dword ptr [eax]
        mov eax, a
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool elapsed_above_sum(const float& elapsed, const float& wait, const float& fire) noexcept {
    const float* const e = &elapsed;
    const float* const w = &wait;
    const float* const f = &fire;
    std::uint8_t result;
    __asm {
        mov eax, e
        fld dword ptr [eax]
        mov eax, w
        fld dword ptr [eax]
        mov eax, f
        fadd dword ptr [eax]
        fxch
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
void decay(volatile float& value, const float& rate, float step) noexcept {
    volatile float* const v = &value;
    const float* const r = &rate;
    __asm {
        mov eax, v
        fld dword ptr [eax]
        mov ecx, r
        fld dword ptr [ecx]
        fmul step
        fsubp st(1), st(0)
        fstp step
        fld step
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        jbe keep_value
        xorps xmm0, xmm0
        jmp store_value
    keep_value:
        movss xmm0, step
    store_value:
        movss dword ptr [eax], xmm0
    }
}
void recover(volatile float& value, const float& rate, float step) noexcept {
    volatile float* const v = &value;
    float spill;
    copy_float(spill, rate);
    __asm {
        mov eax, v
        fld spill
        fmul step
        fadd dword ptr [eax]
        fstp step
        fld1
        fld step
        fcomip st(0), st(1)
        fstp st(0)
        jbe keep_value
        mov dword ptr [eax], 3f800000h
        jmp done
    keep_value:
        movss xmm0, step
        movss dword ptr [eax], xmm0
    done:
    }
}
} // namespace

void unit_generic_input_unassigned_0095dd71(volatile float& value) noexcept {
    const float one = 1.0f;
    copy_float(value, one);
    // The native following COMISS sees 1 == 1 and returns. Execute it as well
    // to preserve the reached SSE floating-point comparison effects.
    static_cast<void>(sse_above(one, value));
}

void unit_generic_input_phase_0095dc40(UnitGenericInputFields state, float step,
    UnitGenericInputContext& context) {
    const std::uint32_t local_slot = context.local_player_slot_18ec;
    if (local_slot > 7 || static_cast<std::uint32_t>(context.current_role4_1bc) != local_slot) {
        unit_generic_input_unassigned_0095dd71(state.value_63c);
        return;
    }

    float now;
    copy_float(now, context.mission_clock_00f876a4);
    float elapsed = 0.0f;
    const auto& owner = context.calls.input_owner_004bec00();
    const auto* action = static_cast<const unsigned char*>(read<void*>(&owner, 4)) + 0x1cb0;
    const auto& input_value = *reinterpret_cast<const volatile float*>(action + 0x24);
    const float zero = 0.0f;
    const bool held = read<std::uint8_t>(action, 0x28) != 0
        && sse_above(input_value, zero)
        && (state.latch_644 != 0 || !x87_below(state.field_708, state.timestamp_640));
    if (held) {
        copy_float(state.timestamp_640, now);
        state.latch_644 = 1;
    } else {
        const volatile float* const timestamp = &state.timestamp_640;
        volatile std::uint8_t* const latch = &state.latch_644;
        __asm {
            fld now
            mov eax, latch
            mov byte ptr [eax], 0
            mov eax, timestamp
            fsub dword ptr [eax]
            fstp elapsed
        }
    }

    const void* const view = context.global_00e198c4;
    const void* const child = read<void*>(view, 0xcc);
    const bool target = read<std::int32_t>(child, 0x4c) != 0;
    const auto& first = context.calls.settings_00424c40();
    if (x87_above(elapsed, first.player_artillery_throw_after_shot_fire_time)) {
        const auto& fire_settings = context.calls.settings_00424c40();
        const auto& wait_settings = context.calls.settings_00424c40();
        if (!elapsed_above_sum(elapsed, wait_settings.player_artillery_throw_after_shot_wait_time,
                fire_settings.player_artillery_throw_after_shot_fire_time)) return;
        if (!sse_above(state.value_63c, zero)) return;
        const auto& settings = context.calls.settings_00424c40();
        decay(state.value_63c, settings.player_artillery_throw_throw_decrement_time, step);
        return;
    }
    const float one = 1.0f;
    if (!sse_above(one, state.value_63c)) return;
    const auto& settings = context.calls.settings_00424c40();
    recover(state.value_63c, target ? settings.player_artillery_throw_throw_increment_time_has_target
                                  : settings.player_artillery_throw_throw_increment_time_no_target, step);
}
} // namespace bsp
