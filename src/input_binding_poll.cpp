#include "bsp/input_binding_poll.hpp"
#include "bsp/input_tick.hpp"

#include <cstring>

namespace bsp {
namespace {

float absolute_bits(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits &= 0x7fffffffu;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

float apply_response_curve(float value) noexcept {
    const float magnitude = absolute_bits(value);
    const double quarter = 0.25; // 00d7a348
    const double half = 0.5; // 00d7a280
    const double seven_eighths = 0.875; // 00ce42e0
    const double eighth = 0.125; // 00d04380
    float result;
    // COMISS threshold,magnitude; JBE takes the outer branch for NaN too.
    if (magnitude < 0.5f) { // 00ce3800
        __asm {
            fld value
            fmul quarter
            fstp result
        }
    } else {
        __asm {
            fld magnitude
            fsub half
            fmul seven_eighths
            fadd st(0),st(0)
            fadd eighth
            fstp result
        }
        if (value < 0.0f) result = -0.0f - result; // 00d7a208; SUBSS
    }
    return result;
}

float accumulate_binding_value(float accumulated, float value) noexcept {
    // 00a92626..69b. Carry branches include unordered, and equality preserves
    // the accumulator's signed zero. MSVC /O2 can change the latter even for
    // the equivalent scalar comparison/select under /fp:strict.
    float result;
    __asm {
        xorps xmm1,xmm1
        fldz
        fld value
        fcomi st(0),st(1)
        fstp st(1)
        jb negative_or_unordered
        movss xmm0,accumulated
        comiss xmm0,xmm1
        jb negative_or_unordered
        fld accumulated
        fxch st(1)
        fcomip st(0),st(1)
        fstp st(0)
        jbe choose_accumulated
        jmp choose_value
    negative_or_unordered:
        movss xmm0,value
        comiss xmm1,xmm0
        jb add_values
        comiss xmm1,accumulated
        jb add_values
        fld accumulated
        fcomip st(0),st(1)
        fstp st(0)
        ja choose_value
    choose_accumulated:
        movss xmm0,accumulated
        movss result,xmm0
        jmp finished
    choose_value:
        movss xmm0,value
        movss result,xmm0
        jmp finished
    add_values:
        fadd accumulated
        fstp result
    finished:
    }
    return result;
}

} // namespace

bool any_input_binding_query_00a92090(
    const std::vector<InputActionBinding>& bindings, InputBindingPollHost& host) {
    for (const auto& binding : bindings) {
        if (binding.cached_device != nullptr &&
            host.device_query_1c(*binding.cached_device, binding.input_code) != 0) {
            return true;
        }
    }
    return false;
}

float adjust_input_binding_pair_00a91d60(float primary, float paired,
    InputBindingPollHost& host) {
    const float abs_primary = absolute_bits(primary);
    const float abs_paired = absolute_bits(paired);
    // FCOMIP/JBE selects abs_paired on equality and unordered.
    const float maximum = abs_primary > abs_paired ? abs_primary : abs_paired;
    float reciprocal, normalized_primary, normalized_paired, sum;
    __asm {
        fld maximum
        fld1
        fdivrp st(1),st(0)
        fstp reciprocal
        fld reciprocal
        fld st(0)
        fmul primary
        fstp normalized_primary
        fmul paired
        fstp normalized_paired
        fld normalized_paired
        fld normalized_primary
        fmul st(0),st(0)
        fld st(1)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fstp sum
    }
    const float root = host.crt_sqrt_00bf7030(sum); // 00a91def/00a91df4
    float result;
    __asm {
        fld root
        fmul primary
        fstp result
    }
    // JBE at 00a91e0c retains the value for unordered as well as <=1.
    return result > 1.0f ? 1.0f : result;
}

void poll_input_action_bindings_00a92370(InputActionRecord& record,
    InputBindingPollHost& host) {
    begin_action_frame_00a92370(record);
    const bool any_device_query = any_input_binding_query_00a92090(record.bindings, host);
    bool force_unit_scale = false;
    for (const auto& binding : record.bindings) {
        if (binding.force_unit_scale) {
            force_unit_scale = true;
            break;
        }
    }
    for (const auto& binding : record.bindings) {
        if (!binding.resolved) continue;
        bool accepted = true;
        for (const auto& modifier : binding.required_modifiers) {
            if (host.device_query_20(*modifier.cached_device, modifier.input_code) == 0) {
                accepted = false;
                break;
            }
        }
        if (!accepted) continue;
        for (const auto& modifier : binding.forbidden_modifiers) {
            if (host.device_query_20(*modifier.cached_device, modifier.input_code) == 1) {
                accepted = false;
                break;
            }
        }
        if (!accepted) continue;

        float value = host.device_value_24(*binding.cached_device, binding.input_code);
        // FUCOMIP/LAHF/TEST 44h/JNP rejects equality. Unordered has both PF/ZF
        // set, so TEST's even parity keeps it on the nonzero path too.
        if (value != 0.0f && !any_device_query &&
            binding.device_class == 2 && binding.input_code >= 0x3cu &&
            binding.input_code <= 0x3fu) {
            const auto paired_code = binding.input_code ^ 1u; // 3C<->3D, 3E<->3F
            if (host.device_query_1c(*binding.cached_device, paired_code) == 0) {
                const float paired = host.device_value_24(*binding.cached_device, paired_code);
                value = adjust_input_binding_pair_00a91d60(value, paired, host);
            }
        }
        if (binding.response_curve) value = apply_response_curve(value);
        value = (force_unit_scale ? 1.0f : binding.scale) * value;
        record.current_hold = accumulate_binding_value(record.current_hold, value);
        if (!record.current_down) {
            record.current_down = host.device_query_20(
                *binding.cached_device, binding.input_code) != 0;
        }
    }
}

void rebind_all_input_actions_00a922a0(std::vector<InputActionRecord>& records,
    const InputBindingDeviceGroups& groups) noexcept {
    for (auto& record : records) rebind_input_action_00a91e80(record.bindings, groups);
}

} // namespace bsp
