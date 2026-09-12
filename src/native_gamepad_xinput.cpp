#include "bsp/native_gamepad_xinput.hpp"
#include "bsp/native_int_pointer_tree18_erase.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p) + offset);
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p) + offset) = value;
}
void* tree_of(void* owner) noexcept { return static_cast<std::byte*>(owner) + 0x20c; }
void* head_of(void* tree) noexcept { return read<void*>(tree, 4); }
struct Iterator { void* owner; void* node; };

void destroy_input_base(void* owner) noexcept {
    destroy_native_input_device_base_00a93e70(owner);
}
void validate_node(void* owner, void* node) {
    if (node == head_of(owner)) _invalid_parameter_noinfo();
}
// Same recognized raw18h STL instantiations as A94230/A94730/A948C0/
// A94E90/A95330/A95A40. Reuse established source leaves, not library code.
void destroy_tree(void* tree) { destroy_native_int_pointer_tree18_0086fde0(tree); }
void clear_nodes(void* tree) {
    erase_subtree_native_int_pointer_tree18_0086aa60(tree, nullptr, read<void*>(head_of(tree), 4));
    write(head_of(tree), 4, head_of(tree));
    write<std::uint32_t>(tree, 8, 0);
    write(head_of(tree), 0, head_of(tree));
    write(head_of(tree), 8, head_of(tree));
}
void delete_payloads(void* tree, NativeGamepadContext& context) {
    Iterator it{tree, read<void*>(head_of(tree), 0)};
    for (;;) {
        void* const current_head = head_of(tree); // captured before validation
        if (!it.owner || it.owner != tree) _invalid_parameter_noinfo();
        if (it.node == current_head) return;
        if (!it.owner) _invalid_parameter_noinfo();
        validate_node(it.owner, it.node);
        if (void* const request = read<void*>(it.node, 0x10))
            context.dispatch.request_delete_vslot00(request, 1);
        increment_native_int_pointer_tree18_00869a20(&it);
    }
}
bool greater(float value, float previous) noexcept {
    unsigned char result;
    __asm {
        fld previous
        fld value
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool changed(float previous, float value) noexcept {
    unsigned char result;
    __asm {
        fld value
        fld previous
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        setp result
    }
    return result != 0; // unequal OR unordered; equal signed zero is unchanged
}
const double axis_scale = 0.000030517578125;
const double trigger_scale = 255.0;
const double deadzone_scale = 1.249980926513671875;
const double motor_scale = 65535.0;
float scaled_value(std::int32_t sample, bool trigger, bool negate) noexcept {
    float result;
    __asm { fild sample }
    if (negate) { __asm { fchs } }
    if (trigger) { __asm { fdiv trigger_scale } }
    else { __asm { fmul axis_scale } }
    __asm { fstp result }
    return result;
}
// Preserve the shared x87 multiplier across the first three conversions;
// the final FIMUL consumes it exactly as A9A983/A9A9A4. ECX/EDX are source
// arguments only; the converter receives the live CRT mode address in ECX.
__declspec(naked) void __fastcall normalize_axes(void*, const volatile std::uint32_t*) {
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        sub esp, 4
        lea esi, [ecx + 230h]
        mov edi, edx
        mov ebp, 3
        fld deadzone_scale
    next_axis:
        movsx eax, word ptr [esi]
        cmp eax, 1999h
        jge outside
        cmp eax, -1999h
        jle outside
        mov word ptr [esi], 0
        jmp advance
    outside:
        test eax, eax
        jle negative_axis
        sub eax, 1999h
        jmp convert_axis
    negative_axis:
        add eax, 1999h
    convert_axis:
        mov dword ptr [esp], eax
        fild dword ptr [esp]
        fmul st(0), st(1)
        mov ecx, edi
        call native_crt_truncate_st0_00bf7420
        mov word ptr [esi], ax
    advance:
        add esi, 2
        dec ebp
        jnz next_axis
        movsx eax, word ptr [esi]
        cmp eax, 1999h
        jge last_outside
        cmp eax, -1999h
        jle last_outside
        mov word ptr [esi], 0
        fstp st(0)
        jmp done
    last_outside:
        test eax, eax
        jle last_negative
        sub eax, 1999h
        jmp last_convert
    last_negative:
        add eax, 1999h
    last_convert:
        mov dword ptr [esp], eax
        fimul dword ptr [esp]
        mov ecx, edi
        call native_crt_truncate_st0_00bf7420
        mov word ptr [esi], ax
    done:
        add esp, 4
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}
} // namespace

void* construct_native_gamepad_00a95d70(void* owner, NativeGamepadContext& context) {
    write<std::uint32_t>(owner, 0, 0x00ceb130);
    write<std::int32_t>(owner, 4, 1);
    std::memset(static_cast<std::byte*>(owner) + 0x0c, 0, 0x100);
    std::memset(static_cast<std::byte*>(owner) + 0x10c, 0, 0x100);
    void* const tree = tree_of(owner);
    bool tree_completed = false;
    try {
        write<std::uint32_t>(owner, 0, 0x00d5b670);
        write(tree, 4, allocate_native_int_pointer_tree18_node_0086ac00());
        write<std::uint8_t>(head_of(tree), 0x15, 1);
        write(head_of(tree), 4, head_of(tree));
        write(head_of(tree), 0, head_of(tree));
        write(head_of(tree), 8, head_of(tree));
        write<std::uint32_t>(tree, 8, 0);
        tree_completed = true; // FH3 state1; only the tree, never its payloads
        write<float>(owner, 0x218, 0.0f);
        write<float>(owner, 0x21c, 0.0f);
        pump_native_gamepad_force_00a954c0(owner, 0.0f, context);
    } catch (...) {
        if (tree_completed) destroy_tree(tree);
        destroy_input_base(owner);
        throw;
    }
    return owner;
}

void pump_native_gamepad_force_00a954c0(void* owner, float seconds, NativeGamepadContext& context) {
    const float before[2]{read<float>(owner, 0x218), read<float>(owner, 0x21c)};
    void* const tree = tree_of(owner);
    Iterator it{tree, read<void*>(head_of(tree), 0)};
    write<float>(owner, 0x218, 0.0f);
    write<float>(owner, 0x21c, 0.0f);
    for (;;) {
        void* const iterator_owner = it.owner;
        void* const current_head = head_of(tree);
        if (!iterator_owner || iterator_owner != tree) _invalid_parameter_noinfo();
        if (it.node == current_head) break;
        void* const current = it.node;
        increment_native_int_pointer_tree18_00869a20(&it); // BEFORE update
        if (!iterator_owner) _invalid_parameter_noinfo();
        validate_node(iterator_owner, current);
        context.dispatch.request_update_vslot10(read<void*>(current, 0x10), seconds);
        validate_node(iterator_owner, current);
        if (context.dispatch.request_expired_vslot0c(read<void*>(current, 0x10))) {
            validate_node(iterator_owner, current);
            if (void* const request = read<void*>(current, 0x10))
                context.dispatch.request_delete_vslot00(request, 1);
            Iterator result;
            erase_native_int_pointer_tree18_iterator_0086e8a0(tree, nullptr,
                &result, iterator_owner, current);
        } else if (context.rumble_enabled_e12f2c) {
            validate_node(iterator_owner, current);
            const auto channel = context.dispatch.request_channel_vslot04(read<void*>(current, 0x10));
            validate_node(iterator_owner, current);
            const float value = context.dispatch.request_value_vslot08(read<void*>(current, 0x10));
            const auto offset = 0x218u + channel * 4u; // native unchecked DWORD index
            if (greater(value, read<float>(owner, offset))) write(owner, offset, value);
        }
    }
    if (context.rumble_enabled_e12f2c) {
        for (std::uint32_t channel = 0; channel != 2; ++channel) {
            const float value = read<float>(owner, 0x218 + channel * 4);
            if (changed(before[channel], value)) {
                if (read<std::uint32_t>(owner, 0) == 0x00d5b670) (void)_purecall();
                else context.dispatch.set_force_vslot38(owner, channel, value);
            }
        }
    }
}

void clear_native_gamepad_force_00a95890(void* owner, NativeGamepadContext& context) {
    delete_payloads(tree_of(owner), context);
    clear_nodes(tree_of(owner));
    pump_native_gamepad_force_00a954c0(owner, 0.0f, context);
}
void destroy_native_gamepad_00a95a80(void* owner, NativeGamepadContext& context) {
    write<std::uint32_t>(owner, 0, 0x00d5b670);
    bool tree_completed = true;
    try {
        delete_payloads(tree_of(owner), context);
        clear_nodes(tree_of(owner));
        pump_native_gamepad_force_00a954c0(owner, 0.0f, context);
        tree_completed = false; // native state0 before A95330
        destroy_tree(tree_of(owner));
    } catch (...) {
        if (tree_completed) destroy_tree(tree_of(owner));
        destroy_input_base(owner);
        throw;
    }
    destroy_input_base(owner);
}
std::int32_t native_gamepad_class_00a95bd0(const void*) noexcept { return 2; }
void* scalar_delete_native_gamepad_00a95e40(void* owner, std::uint32_t flags, NativeGamepadContext& context) {
    destroy_native_gamepad_00a95a80(owner, context);
    if (flags & 1) singleton_lifetime_free(owner);
    return owner;
}

void* construct_native_xinput_00a9a5a0(void* owner, std::int32_t index, NativeGamepadContext& context) {
    construct_native_gamepad_00a95d70(owner, context);
    write(owner, 0x220, index);
    write<std::uint8_t>(owner, 0x23c, 0);
    write<std::uint32_t>(owner, 0, 0x00d5bb48);
    for (std::size_t offset = 0x228; offset != 0x23c; offset += 4)
        write<std::uint32_t>(owner, offset, 0);
    return owner;
}
void destroy_native_xinput_00a9a600(void* owner, NativeGamepadContext& context) {
    write<std::uint32_t>(owner, 0, 0x00d5bb48);
    destroy_native_gamepad_00a95a80(owner, context);
}
void* scalar_delete_native_xinput_00a9a7c0(void* owner, std::uint32_t flags, NativeGamepadContext& context) {
    write<std::uint32_t>(owner, 0, 0x00d5bb48);
    destroy_native_gamepad_00a95a80(owner, context);
    if (flags & 1) singleton_lifetime_free(owner);
    return owner;
}
std::int32_t native_xinput_identifier_00a9a5f0(const void*) noexcept { return 0; }
std::uint32_t query_native_xinput_00a9a610(void* owner, std::uint32_t code, NativeGamepadContext& context) {
    const float value = context.dispatch.device_value_vslot24(owner, code);
    const double threshold = code < 16 ? 0.100000001490116119384765625 : 0.5;
    float magnitude;
    unsigned char result;
    __asm {
        fld value
        fabs
        fstp magnitude
        fld threshold
        fld magnitude
        fcomip st(0), st(1)
        fstp st(0)
        setnc result
    }
    return result;
}
float value_native_xinput_00a9a660(const void* owner, std::uint32_t code, const XInputDeviceGlobals& tables) {
    if (code == 12 || code == 13)
        return scaled_value(read<std::uint8_t>(owner, code == 12 ? 0x22e : 0x22f), true, false);
    if (code < 16)
        return (read<std::uint16_t>(owner, 0x22c) & tables.button_masks[code]) ? 1.0f : 0.0f;
    if (code - 60u < 4)
        return scaled_value(read<std::int16_t>(owner, 0x230 + (code - 60) * 2), false, (code & 1) != 0);
    return 0.0f;
}
bool poll_native_xinput_00a9a7f0(void* owner, float, NativeXInputContext& context) {
    auto& state = *reinterpret_cast<XINPUT_STATE*>(static_cast<std::byte*>(owner) + 0x228);
    const auto result = context.api.get_state(read<DWORD>(owner, 0x220), state);
    write<std::uint8_t>(owner, 0x23c, result != ERROR_DEVICE_NOT_CONNECTED ? 1 : 0);
    if (result != ERROR_SUCCESS) return false;
    auto& vibration = *reinterpret_cast<XINPUT_VIBRATION*>(static_cast<std::byte*>(owner) + 0x238);
    if (!context.gamepad.rumble_enabled_e12f2c) {
        write<std::uint16_t>(owner, 0x23a, 0);
        write<std::uint16_t>(owner, 0x238, 0);
    }
    (void)context.api.set_state(read<DWORD>(owner, 0x220), vibration);
    normalize_axes(owner, &context.crt_sse2_conversion_0109eea4);
    return true;
}
std::uint32_t stop_native_xinput_vibration_00a9a790(void* owner, XInputApi& api) {
    write<std::uint16_t>(owner, 0x238, 0);
    const auto index = read<DWORD>(owner, 0x220);
    write<std::uint16_t>(owner, 0x23a, 0);
    return api.set_state(index, *reinterpret_cast<XINPUT_VIBRATION*>(static_cast<std::byte*>(owner) + 0x238));
}
void set_native_xinput_motor_00a9a9c0(void* owner, std::uint32_t channel, float value) {
    if (channel > 1) return;
    std::uint16_t old_control, truncating_control;
    std::int32_t converted;
    __asm {
        fld value
        fmul motor_scale
        fnstcw old_control
        movzx eax, old_control
        or eax, 0c00h
        mov truncating_control, ax
        fldcw truncating_control
        fistp converted
    }
    write<std::uint16_t>(owner, channel == 0 ? 0x23a : 0x238, static_cast<std::uint16_t>(converted));
    __asm { fldcw old_control }
}
NativeString& name_native_xinput_control_00a9aa40(const void*, NativeString& result,
    std::uint32_t code, const XInputDeviceGlobals& tables, NativeStringStorage& strings) {
    const char* name = "Unknown";
    if (code < 16) name = tables.button_names[code];
    else if (code - 60u < 4) name = tables.axis_names[code - 60];
    return result.assign_0041e870(strings, name);
}
} // namespace bsp
