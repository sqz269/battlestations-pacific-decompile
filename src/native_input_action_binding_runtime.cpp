#include "bsp/native_input_action_binding_runtime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> T read(Word p, Word off = 0) noexcept {
    T value; std::memcpy(&value, pointer(p + off), sizeof value); return value;
}
template<class T = Word> void write(Word p, Word off, T value) noexcept {
    std::memcpy(pointer(p + off), &value, sizeof value);
}
Word array_end(Word header, Word stride) noexcept {
    return read(header) + read(header, 4) * stride;
}
Word distance(Word first, Word last) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2);
}
Word resolve(Word backend, Word device_class, Word index) {
    const auto header = backend + 0x6cu + device_class * 0x24u;
    const auto first = read(header, 4);
    if (!first || index >= distance(first, read(header, 8))) return 0;
    // The redundant second check uses the same first pointer but current end.
    // Native diagnostics may return; the final slot uses a reloaded begin.
    if (!first || index >= distance(first, read(header, 8))) _invalid_parameter_noinfo();
    return read(read(header, 4) + index * 4u);
}
bool rebind_modifiers(Word header, void* volatile& backend) {
    auto row = read(header);
    while (row != array_end(header, 0x14)) {
        const auto device_class = read(row);
        const auto current_backend = address(backend);
        const auto index = read(row, 4);
        const auto device = resolve(current_backend, device_class, index);
        write(row, 8, device);
        if (!device) break;
        const auto end = array_end(header, 0x14);
        row += 0x14u;
        if (row == end) break;
    }
    return row == array_end(header, 0x14);
}
std::uint8_t relative(Word binding, Word code, NativeInputActionBindingCalls& calls) {
    const auto device = read(binding, 0xc);
    const auto profile = read(device);
    return calls.relative_vslot1c(pointer(device), profile, code);
}
float paired_value(Word binding, Word code, NativeInputActionBindingCalls& calls) {
    const auto device = read(binding, 0xc);
    const auto profile = read(device);
    return calls.value_vslot24(pointer(device), profile, code);
}
bool modifiers_allow(Word header, bool forbidden, NativeInputActionBindingCalls& calls) {
    auto row = read(header);
    while (row != array_end(header, 0x14)) {
        const auto device = read(row, 8);
        const auto profile = read(device);
        const auto code = read(row, 0xc);
        const auto result = calls.query_vslot20(pointer(device), profile, code);
        if (forbidden ? result == 1 : result == 0) break;
        const auto end = array_end(header, 0x14);
        row += 0x14u;
        if (row == end) break;
    }
    // Even on a rejected result, the native compares against a fresh endpoint.
    return row == array_end(header, 0x14);
}
bool nonzero_or_unordered(float value) noexcept {
    bool result;
    __asm {
        fldz
        fld value
        fucomip st(0),st(1)
        fstp st(0)
        lahf
        test ah,0x44
        setp result
    }
    return result;
}
float response_curve(float value, const NativeInputActionBindingConstants& constants) noexcept {
    auto* threshold = &constants.half_00ce3800;
    auto* quarter = &constants.quarter_00d7a348;
    auto* half = &constants.half_00d7a280;
    auto* gain = &constants.seven_eighths_00ce42e0;
    auto* bias = &constants.eighth_00d04380;
    auto* negative_zero = &constants.negative_zero_00d7a208;
    float magnitude, result;
    __asm {
        movss xmm0,value
        mov eax,value
        and eax,0x7fffffff
        mov magnitude,eax
        mov eax,threshold
        movss xmm1,dword ptr [eax]
        comiss xmm1,magnitude
        xorps xmm1,xmm1
        jbe outer_curve
        fld value
        mov eax,quarter
        fmul qword ptr [eax]
        fstp result
        jmp curve_done
    outer_curve:
        comiss xmm1,xmm0
        fld magnitude
        mov eax,half
        fsub qword ptr [eax]
        mov eax,gain
        fmul qword ptr [eax]
        fadd st(0),st(0)
        mov eax,bias
        fadd qword ptr [eax]
        fstp result
        jbe curve_done
        mov eax,negative_zero
        movss xmm0,dword ptr [eax]
        subss xmm0,result
        movss result,xmm0
    curve_done:
    }
    return result;
}
void accumulate(Word action, float value) noexcept {
    float captured;
    __asm {
        mov eax,action
        xorps xmm1,xmm1
        fldz
        fld value
        fcomi st(0),st(1)
        fstp st(1)
        jb negative_or_unordered
        movss xmm0,dword ptr [eax+0x24]
        comiss xmm0,xmm1
        jb negative_or_unordered
        fld dword ptr [eax+0x24]
        fstp captured
        fld captured
        fxch st(1)
        fcomip st(0),st(1)
        fstp st(0)
        jbe choose_captured
        movss xmm0,value
        jmp store_result
    negative_or_unordered:
        movss xmm0,value
        comiss xmm1,xmm0
        jb add_values
        comiss xmm1,dword ptr [eax+0x24]
        jb add_values
        fld dword ptr [eax+0x24]
        fstp captured
        fld captured
        fcomip st(0),st(1)
        fstp st(0)
        ja store_result
    choose_captured:
        movss xmm0,captured
    store_result:
        movss dword ptr [eax+0x24],xmm0
        jmp accumulated
    add_values:
        fadd dword ptr [eax+0x24]
        fstp dword ptr [eax+0x24]
    accumulated:
    }
}
} // namespace

void rebind_native_input_action_00a91e80(void* actual_action, void* volatile& backend) {
    const auto action = address(actual_action), header = action + 0x10u;
    auto binding = read(header);
    while (binding != array_end(header, 0x34)) {
        const auto device_class = read(binding, 4); // read precedes resolved-byte clear
        write<std::uint8_t>(binding, 0, 0);
        if (device_class != 0xffffffffu) {
            const auto index = read(binding, 8);
            const auto current_backend = address(backend);
            const auto device = resolve(current_backend, device_class, index);
            write(binding, 0xc, device);
            if (device && rebind_modifiers(binding + 0x18u, backend)
                && rebind_modifiers(binding + 0x24u, backend)) write<std::uint8_t>(binding, 0, 1);
        }
        const auto end = array_end(header, 0x34);
        binding += 0x34u;
        if (binding == end) break;
    }
}
void rebind_all_native_input_actions_00a922a0(void* actual_owner, void* volatile& backend) {
    const auto header = address(actual_owner) + 4u;
    auto action = read(header);
    while (action != array_end(header, 0x30)) {
        rebind_native_input_action_00a91e80(pointer(action), backend);
        const auto end = array_end(header, 0x30);
        action += 0x30u;
        if (action == end) break;
    }
}
std::uint8_t query_native_input_action_relative_00a92090(void* actual_action,
    NativeInputActionBindingCalls& calls) {
    const auto header = address(actual_action) + 0x10u;
    auto binding = read(header);
    while (binding != array_end(header, 0x34)) {
        if (read(binding, 0xc)) {
            const auto device = read(binding, 0xc);
            const auto profile = read(device);
            const auto code = read(binding, 0x10);
            if (calls.relative_vslot1c(pointer(device), profile, code)) return 1;
        }
        const auto end = array_end(header, 0x34);
        binding += 0x34u;
        if (binding == end) break;
    }
    return 0;
}

float adjust_native_input_binding_pair_00a91d60(float primary, float paired,
    const CameraAxesCrtAccess& crt) {
    const auto* access = &crt;
    float first_abs, second_abs, maximum, reciprocal, normalized_primary, normalized_paired, sum, result;
    __asm {
        mov eax,primary
        and eax,0x7fffffff
        mov first_abs,eax
        mov eax,paired
        and eax,0x7fffffff
        mov second_abs,eax
        fld second_abs
        fld first_abs
        fcomip st(0),st(1)
        fstp st(0)
        jbe choose_second
        mov eax,first_abs
        jmp have_maximum
    choose_second:
        mov eax,second_abs
    have_maximum:
        mov maximum,eax
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
        fld sum
        mov ecx,access
        call native_crt_sqrt_st0_00bf7030
        fstp result
        fld result
        fmul primary
        fstp result
        fld1
        fld result
        fcomi st(0),st(1)
        jbe retain_result
        fstp st(0)
        jmp pair_done
    retain_result:
        fstp st(1)
    pair_done:
        fstp result
    }
    return result;
}

void poll_native_input_action_00a92370(void* actual_action, NativeInputActionBindingContext& context) {
    const auto action = address(actual_action), header = action + 0x10u;
    __asm {
        mov eax,action
        fld dword ptr [eax+0x24]
        mov dl,byte ptr [eax+0x28]
        fstp dword ptr [eax+0x1c]
        mov byte ptr [eax+0x20],dl
        mov dword ptr [eax+0x24],0
        mov byte ptr [eax+0x28],0
    }
    const auto any_relative = query_native_input_action_relative_00a92090(actual_action, context.devices);
    const auto first = read(header);
    const auto scan_end = first + read(header, 4) * 0x34u;
    bool unit_scale = false;
    for (auto row = first; row != scan_end; row += 0x34u) {
        if (read<std::uint8_t>(row, 0x14)) { unit_scale = true; break; }
    }
    auto binding = first;
    while (binding != array_end(header, 0x34)) {
        if (read<std::uint8_t>(binding) && modifiers_allow(binding + 0x18u, false, context.devices)
            && modifiers_allow(binding + 0x24u, true, context.devices)) {
            const auto device = read(binding, 0xc);
            const auto profile = read(device);
            const auto code = read(binding, 0x10);
            float value = context.devices.value_vslot24(pointer(device), profile, code);
            if (nonzero_or_unordered(value) && !any_relative && read(binding, 4) == 2u) {
                const auto initial_code = read(binding, 0x10);
                if (initial_code >= 0x3cu && initial_code <= 0x3fu) {
                    Word pair = 0;
                    // Each unsuccessful query may change the code or cached
                    // device before the NEXT comparison in this native chain.
                    if (initial_code == 0x3cu && !relative(binding, 0x3d, context.devices)) pair = 0x3d;
                    else if (read(binding, 0x10) == 0x3du && !relative(binding, 0x3c, context.devices)) pair = 0x3c;
                    else if (read(binding, 0x10) == 0x3eu && !relative(binding, 0x3f, context.devices)) pair = 0x3f;
                    else if (read(binding, 0x10) == 0x3fu && !relative(binding, 0x3e, context.devices)) pair = 0x3e;
                    if (pair) value = adjust_native_input_binding_pair_00a91d60(value,
                        paired_value(binding, pair, context.devices), context.crt);
                }
            }
            if (read<std::uint8_t>(binding, 1)) value = response_curve(value, context.constants);
            const float scale = unit_scale ? context.constants.one_00d7a24c : read<float>(binding, 0x30);
            __asm { fld scale }
            __asm { fmul value }
            __asm { fstp value }
            accumulate(action, value);
            std::uint8_t down = 1;
            if (!read<std::uint8_t>(action, 0x28)) {
                const auto current_device = read(binding, 0xc);
                const auto current_profile = read(current_device);
                const auto current_code = read(binding, 0x10);
                down = context.devices.query_vslot20(pointer(current_device), current_profile, current_code) ? 1 : 0;
            }
            write<std::uint8_t>(action, 0x28, down);
        }
        const auto end = array_end(header, 0x34);
        binding += 0x34u;
        if (binding == end) break;
    }
}
} // namespace bsp
