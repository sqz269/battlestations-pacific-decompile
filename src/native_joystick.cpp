#include "bsp/native_joystick.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#pragma comment(lib, "dxguid.lib")

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeJoystickStorage) == 0xb48);
static_assert(sizeof(NativeString) == 8 && sizeof(DIDATAFORMAT) == 0x18);
static_assert(sizeof(DIOBJECTDATAFORMAT) == 0x10 && sizeof(DIEFFECT) == 0x38);
static_assert(sizeof(DIPROPRANGE) == 0x18 && sizeof(DIPROPDWORD) == 0x14);
static_assert(offsetof(DIDEVICEINSTANCEA, tszProductName) == 0x12c);
static_assert(offsetof(DIDEVICEOBJECTINSTANCEA, dwType) == 0x18);
static_assert(offsetof(DIDEVICEOBJECTINSTANCEA, tszName) == 0x20);
static_assert(sizeof(XINPUT_STATE) == 0x10);
void* at(void* p, std::uint32_t n) noexcept { return static_cast<std::byte*>(p) + n; }
const void* at(const void* p, std::uint32_t n) noexcept { return static_cast<const std::byte*>(p) + n; }
template<class T> T get(const void* p, std::uint32_t n) noexcept {
    T v; std::memcpy(&v, at(p, n), sizeof(v)); return v;
}
template<class T> void put(void* p, std::uint32_t n, T v) noexcept {
    std::memcpy(at(p, n), &v, sizeof(v));
}
IDirectInputDevice8A* device(void* p) noexcept { return get<IDirectInputDevice8A*>(p, 0x220); }
std::int32_t sub(std::int32_t a, std::int32_t b) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) - static_cast<std::uint32_t>(b));
}
std::uint32_t array_bytes(std::uint32_t count, std::uint32_t stride, bool cookie = false) noexcept {
    const auto product = static_cast<std::uint64_t>(count) * stride;
    std::uint32_t bytes = product > 0xffffffffu ? 0xffffffffu : static_cast<std::uint32_t>(product);
    if (cookie) bytes = bytes > 0xfffffffbu ? 0xffffffffu : bytes + 4u;
    return bytes;
}
void* allocate(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void copy_name(void* header, const char* source, NativeStringStorage& strings) {
    resize_native_string_header_0041dd40(header, strings,
        source ? static_cast<std::uint32_t>(std::strlen(source)) : 0u, false);
    if (auto* destination = get<char*>(header, 4))
        std::memcpy(destination, source, get<std::uint32_t>(header, 0));
}
bool match(const void* header, const char* text) noexcept {
    const auto* name = get<const char*>(header, 4);
    if (!name) return false;
    const auto* result = std::strstr(name, text);
    return result && static_cast<std::uint32_t>(result - name) != 0xffffffffu;
}
struct BaseCleanup {
    void* owner; NativeJoystickCalls& calls; bool armed = true;
    ~BaseCleanup() noexcept(false) { if (armed) calls.call_00a95a80(owner); }
};
struct NameCleanup {
    void* header; NativeStringStorage& strings; bool armed = true;
    ~NameCleanup() { if (armed) destroy_native_string_header_0041dd20(header, strings); }
};
struct ObjectCallbackFrame;
thread_local ObjectCallbackFrame* callback_frame;
struct ObjectCallbackFrame {
    void* owner; NativeStringStorage& strings; std::exception_ptr failure;
    ObjectCallbackFrame* previous;
    ObjectCallbackFrame(void* p, NativeStringStorage& s) : owner(p), strings(s), previous(callback_frame) {
        callback_frame = this;
    }
    ~ObjectCallbackFrame() { callback_frame = previous; }
};
bool pov(std::uint32_t kind, std::int32_t value) noexcept {
    switch (kind) {
    case 5: return value == 22500 || value == 27000 || value == 31500;
    case 6: return value == 4500 || value == 9000 || value == 13500;
    case 7: return value == 31500 || value == 0 || value == 4500;
    case 8: return value == 13500 || value == 18000 || value == 22500;
    default: return false;
    }
}
float axis_value(std::int32_t numerator, std::int32_t span, std::uint32_t kind,
    const NativeJoystickConstants& constants) noexcept {
    float result;
    const volatile double* one = &constants.one_00d7a210;
    __asm {
        fild numerator
        fidiv span
        cmp kind,2
        jne not_full
        fadd st(0),st(0)
        mov eax,one
        fsub qword ptr [eax]
        jmp store
    not_full:
        cmp kind,3
        jne store
        fld1
        fsubrp st(1),st(0)
    store:
        fstp result
    }
    return result;
}
std::int32_t deadzone(std::int32_t value, std::int32_t span, double threshold_scale, double gain,
    const volatile std::uint32_t* conversion_mode) noexcept {
    const std::int32_t midpoint = span / 2;
    float rounded_midpoint;
    std::int32_t limit;
    __asm {
        fild midpoint
        fstp rounded_midpoint
        fld rounded_midpoint
        fmul threshold_scale
        mov eax,value
        cmp eax,midpoint
        jle negative_threshold
        fadd rounded_midpoint
        jmp store_threshold
    negative_threshold:
        fsubr rounded_midpoint
    store_threshold:
        mov ecx,conversion_mode
        call native_crt_truncate_st0_00bf7420
        mov limit,eax
    }
    if ((value > midpoint && value < limit) || (value <= midpoint && value > limit)) return midpoint;
    const auto distance = value > midpoint ? sub(value, limit) : sub(limit, value);
    std::int32_t remapped;
    __asm {
        fild distance
        fmul gain
        mov eax,value
        cmp eax,midpoint
        jle negative_result
        fadd rounded_midpoint
        jmp store_result
    negative_result:
        fsubr rounded_midpoint
    store_result:
        mov ecx,conversion_mode
        call native_crt_truncate_st0_00bf7420
        mov remapped,eax
    }
    return remapped;
}
float plus_seconds(float seconds, const volatile double& duration) noexcept {
    const volatile double* amount = &duration;
    float result;
    __asm {
        fld seconds
        mov eax,amount
        fadd qword ptr [eax]
        fstp result
    }
    return result;
}
float trigger_value(std::uint8_t trigger, double maximum) noexcept {
    const std::int32_t value = trigger;
    float result;
    __asm {
        fild value
        fdiv maximum
        fstp result
    }
    return result;
}
std::int32_t magnitude(float value, const volatile double& scale,
    const volatile std::uint32_t* conversion_mode) noexcept {
    const volatile double* factor = &scale;
    std::int32_t product;
    __asm {
        fld value
        mov eax,factor
        fmul qword ptr [eax]
        mov ecx,conversion_mode
        call native_crt_truncate_st0_00bf7420
        mov product,eax
    }
    return product;
}
} // namespace

void* construct_native_joystick_object_00a99150(void* p) noexcept {
    put<std::uint32_t>(p, 0, 0); put<void*>(p, 4, nullptr); return p;
}
void destroy_native_joystick_object_00a99160(void* p, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(p, strings);
}
int WINAPI count_native_joystick_objects_00a98b90(const DIDEVICEOBJECTINSTANCEA* object, void* p) noexcept {
    if ((object->dwType & 0x1fu) != 0) put(p, 0x22c, get<std::uint32_t>(p, 0x22c) + 1u);
    return 1;
}
int describe_native_joystick_object_00a992f0(void* p, const DIDEVICEOBJECTINSTANCEA& object,
    NativeStringStorage& strings) {
    if ((object.dwType & 0x1fu) == 0) return 1;
    copy_name(at(get<void*>(p, 0x290), get<std::uint32_t>(p, 0x230) * 0x1cu), object.tszName, strings);
    auto* format = static_cast<DIOBJECTDATAFORMAT*>(at(get<void*>(p, 0x288), get<std::uint32_t>(p, 0x230) * 0x10u));
    format->pguid = nullptr;
    format->dwOfs = get<std::uint32_t>(p, 0x230) * 4u;
    format->dwType = object.dwType; format->dwFlags = object.dwFlags;
    auto set_kind = [&](std::uint32_t kind) {
        put(at(get<void*>(p, 0x290), get<std::uint32_t>(p, 0x230) * 0x1cu), 8, kind);
    };
    auto set_button = [&](std::uint32_t slot, std::uint32_t kind) {
        put<std::uint8_t>(p, 0x234u + slot, 1);
        put(p, 0x29cu + slot * 12u, kind);
        put(p, 0x2a0u + slot * 12u, get<std::uint32_t>(p, 0x230));
    };
    if ((object.dwType & 0xcu) != 0 && static_cast<std::uint16_t>(object.dwType >> 8) < 60) {
        set_kind(0);
        for (std::uint32_t slot = 0; slot < 60; ++slot) {
            if (slot >= 4 && slot <= 7) continue;
            if (get<std::uint8_t>(p, 0x234u + slot) != 1) { set_button(slot, 1); break; }
        }
    }
    if ((object.dwType & 3u) != 0 && static_cast<std::uint16_t>(object.dwType >> 8) < 30) {
        set_kind(1);
        auto axis = get<std::int32_t>(p, 0x270);
        put<std::uint32_t>(p, 0x56cu + static_cast<std::uint32_t>(axis) * 12u, 2);
        axis = get<std::int32_t>(p, 0x270);
        put(p, 0x570u + static_cast<std::uint32_t>(axis) * 12u, get<std::uint32_t>(p, 0x230));
        axis = get<std::int32_t>(p, 0x270);
        if (axis == 1) put<std::int32_t>(p, 0x270, 0);
        else if (axis == 0) put<std::int32_t>(p, 0x270, 3);
        else if (axis == 3) put<std::int32_t>(p, 0x270, 2);
        else if (axis == 2) put<std::int32_t>(p, 0x270, 4);
        else if (axis >= 4) put(p, 0x270, static_cast<std::uint32_t>(axis) + 1u);
    }
    if ((object.dwType & 0x10u) != 0) {
        set_kind(2);
        std::uint32_t slot = get<std::uint8_t>(p, 0x238) == 1 ? 20u : 4u;
        for (std::uint32_t kind = 5; kind <= 8; ++kind) {
            while (slot < 60 && get<std::uint8_t>(p, 0x234u + slot) == 1) ++slot;
            if (slot >= 60) break;
            set_button(slot, kind);
        }
    }
    if ((object.dwFlags & 1u) != 0) {
        if (get<std::uint32_t>(p, 0xb1c) == 0xffffffffu) {
            const auto offset = format->dwOfs;
            put(p, 0x230, get<std::uint32_t>(p, 0x230) + 1u);
            put(p, 0xb1c, offset); put<std::uint32_t>(p, 0xb18, 1); return 1;
        }
        if (get<std::uint32_t>(p, 0xb20) == 0xffffffffu) {
            put(p, 0xb20, format->dwOfs); put<std::uint32_t>(p, 0xb18, 2);
        }
    }
    put(p, 0x230, get<std::uint32_t>(p, 0x230) + 1u); return 1;
}
int WINAPI native_joystick_object_callback_00a99920(const DIDEVICEOBJECTINSTANCEA* object, void* p) noexcept {
    auto* frame = callback_frame;
    while (frame && frame->owner != p) frame = frame->previous;
    if (!frame || frame->failure) return DIENUM_STOP;
    try { return describe_native_joystick_object_00a992f0(p, *object, frame->strings); }
    catch (...) { frame->failure = std::current_exception(); return DIENUM_STOP; }
}

void* construct_native_joystick_00a99940(void* p, IDirectInput8A& input,
    const DIDEVICEINSTANCEA& instance, NativeJoystickContext& context) {
    context.calls.call_00a95d70(p);
    BaseCleanup base{p, context.calls};
    put<std::uint32_t>(p, 0, 0x00d5b7f0);
    construct_native_joystick_object_00a99150(at(p, 0x224));
    NameCleanup product{at(p, 0x224), context.strings};
    put<std::uint32_t>(p, 0xb10, 0); put<std::uint32_t>(p, 0xb14, 0);
    copy_name(at(p, 0x224), instance.tszProductName, context.strings);
    const bool xbox = (match(at(p, 0x224), "Xbox") || match(at(p, 0x224), "XBOX") ||
        match(at(p, 0x224), "XBox")) && match(at(p, 0x224), "360");
    put<std::uint8_t>(p, 0xb3c, xbox ? 1 : 0);
    put<std::uint8_t>(p, 0x28c, 0);
    put<void*>(p, 0xb24, nullptr); put<void*>(p, 0xb28, nullptr);
    put<std::uint32_t>(p, 0xb20, 0xffffffffu); put<std::uint32_t>(p, 0xb1c, 0xffffffffu);
    put<std::uint32_t>(p, 0xb18, 0);
    for (std::uint32_t i = 0; i < 90; ++i) {
        put<std::uint32_t>(p, 0x29cu + i * 12u, 0);
        put<std::uint32_t>(p, 0x2a0u + i * 12u, 0xffffffffu);
        put<std::uint8_t>(p, 0x2a4u + i * 12u, 0);
    }
    context.sdk.create_device(input, instance.guidInstance,
        static_cast<IDirectInputDevice8A**>(at(p, 0x220)), nullptr);
    put<std::uint32_t>(p, 0x22c, 0); put<std::uint32_t>(p, 0x230, 0);
    std::memset(at(p, 0x234), 0, 0x3c);
    put<std::int32_t>(p, 0x270, 1);
    device(p)->EnumObjects(&count_native_joystick_objects_00a98b90, p, 0);
    const auto count = get<std::uint32_t>(p, 0x22c);
    void* const allocation = allocate(array_bytes(count, 0x1c, true));
    void* objects = nullptr;
    if (allocation) {
        put(allocation, 0, count); objects = at(allocation, 4);
        // A99150 cannot throw; vector iterator's temporary-allocation EH is
        // unreachable in the supported C++ domain. Other bytes stay untouched.
        for (std::int32_t i = 0; i < static_cast<std::int32_t>(count); ++i)
            construct_native_joystick_object_00a99150(at(objects, static_cast<std::uint32_t>(i) * 0x1cu));
    }
    put(p, 0x290, objects);
    for (std::int32_t i = 0; i < get<std::int32_t>(p, 0x22c); ++i)
        put<std::uint8_t>(get<void*>(p, 0x290), static_cast<std::uint32_t>(i) * 0x1cu + 0xc, 0);
    put(p, 0x294, allocate(array_bytes(get<std::uint32_t>(p, 0x22c), 4)));
    put(p, 0x298, allocate(array_bytes(get<std::uint32_t>(p, 0x22c), 4)));
    const auto format_count = get<std::uint32_t>(p, 0x22c);
    put(p, 0x280, format_count * 4u);
    put<std::uint32_t>(p, 0x274, 0x18); put<std::uint32_t>(p, 0x278, 0x10);
    put<std::uint32_t>(p, 0x27c, 1); put(p, 0x284, format_count);
    put(p, 0x288, allocate(array_bytes(format_count, 0x10)));
    {
        ObjectCallbackFrame frame(p, context.strings);
        device(p)->EnumObjects(&native_joystick_object_callback_00a99920, p, 0);
        if (frame.failure) std::rethrow_exception(frame.failure);
    }
    device(p)->SetDataFormat(static_cast<DIDATAFORMAT*>(at(p, 0x274)));
    DIPROPRANGE range; // successful real SDK query supplies both output DWORDs
    for (std::int32_t i = 0; i < get<std::int32_t>(p, 0x230); ++i) {
        range.diph = {0x18, 0x10, static_cast<DWORD>(i) * 4u, 1};
        if (device(p)->GetProperty(DIPROP_RANGE, &range.diph) == 0) {
            void* row = at(get<void*>(p, 0x290), static_cast<std::uint32_t>(i) * 0x1cu);
            put<std::uint8_t>(row, 0xc, 1); put(row, 0x10, range.lMin);
            put(row, 0x14, range.lMax); put(row, 0x18, sub(range.lMax, get<std::int32_t>(row, 0x10)));
        }
    }
    using Cooperate = HRESULT (WINAPI*)(IDirectInputDevice8A*, HWND, DWORD);
    void* const cooperate_slot = at(get<void*>(device(p), 0), 0x34);
    const HWND window = context.calls.call_00bec230();
    auto* const cooperate_receiver = device(p);
    const auto cooperate = get<Cooperate>(cooperate_slot, 0);
    cooperate(cooperate_receiver, window, 5);
    if (get<std::uint32_t>(p, 0xb18) != 0) {
        DIPROPDWORD center{{0x14, 0x10, 0, 0}, 0};
        device(p)->SetProperty(DIPROP_AUTOCENTER, &center.diph);
        LONG directions[2]{}; DICONSTANTFORCE constant{}; DIRAMPFORCE ramp{};
        DIEFFECT effect{};
        effect.dwSize = 0x38; effect.dwFlags = 0x12; effect.dwDuration = 0xffffffffu;
        effect.dwGain = 10000; effect.dwTriggerButton = 0xffffffffu;
        effect.cAxes = get<DWORD>(p, 0xb18);
        effect.rgdwAxes = static_cast<DWORD*>(at(p, 0xb1c)); effect.rglDirection = directions;
        const auto kind = get<std::uint32_t>(p, 0xb10);
        if (kind == 0) { effect.cbTypeSpecificParams = 4; effect.lpvTypeSpecificParams = &constant; }
        else if (kind == 1) { effect.cbTypeSpecificParams = 8; effect.lpvTypeSpecificParams = &ramp; }
        for (std::uint32_t i = 0; i < 2; ++i) {
            const auto current_kind = get<std::uint32_t>(p, 0xb10);
            if (current_kind == 0 || current_kind == 1)
                context.sdk.create_effect(*device(p), current_kind == 0 ? GUID_ConstantForce : GUID_RampForce,
                    effect, static_cast<IDirectInputEffect**>(at(p, 0xb24u + i * 4u)), nullptr);
        }
    }
    put<std::uint8_t>(p, 0xb2c, 0); put<std::uint8_t>(p, 0x6d4, 0);
    put<std::uint32_t>(p, 0xb30, 0); put<float>(p, 0xb38, 0.0f);
    poll_native_joystick_00a98e30(p, 0.0f, context);
    poll_native_joystick_00a98e30(p, 0.0f, context);
    product.armed = false; base.armed = false; return p;
}
void destroy_native_joystick_00a991f0(void* p, NativeJoystickContext& context) {
    put<std::uint32_t>(p, 0, 0x00d5b7f0);
    BaseCleanup base{p, context.calls}; NameCleanup product{at(p, 0x224), context.strings};
    singleton_lifetime_free(get<void*>(p, 0x294));
    singleton_lifetime_free(get<void*>(p, 0x298));
    void* const objects = get<void*>(p, 0x290);
    if (objects) {
        void* const allocation = static_cast<std::byte*>(objects) - 4;
        auto count = get<std::int32_t>(allocation, 0);
        while (count > 0) {
            --count; destroy_native_joystick_object_00a99160(at(objects, static_cast<std::uint32_t>(count) * 0x1cu), context.strings);
        }
        singleton_lifetime_free(allocation);
    }
    singleton_lifetime_free(get<void*>(p, 0x288));
    for (std::uint32_t i = 0; i < 2; ++i)
        if (auto* effect = get<IDirectInputEffect*>(p, 0xb24u + i * 4u)) effect->Unload();
    product.armed = false; destroy_native_string_header_0041dd20(at(p, 0x224), context.strings);
    base.armed = false; context.calls.call_00a95a80(p);
}
void* scalar_delete_native_joystick_00a99900(void* p, std::uint32_t flags, NativeJoystickContext& context) {
    destroy_native_joystick_00a991f0(p, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(p);
    return p;
}
void reset_native_joystick_feedback_00a98400(void* p) {
    device(p)->SendForceFeedbackCommand(2);
    device(p)->SendForceFeedbackCommand(0x20);
    device(p)->SendForceFeedbackCommand(1);
}
const char* native_joystick_product_name_00a992e0(const void* p, const char* empty) noexcept {
    const auto* name = get<const char*>(p, 0x228);
    return name ? name : empty;
}
void set_native_joystick_relative_00a98bb0(void* p, std::uint32_t code, std::uint8_t value) noexcept {
    put(p, 0x2a4u + code * 12u, value);
}
std::uint8_t query_native_joystick_relative_00a98750(const void* p, std::uint32_t code) noexcept {
    return get<std::uint32_t>(p, 0x29cu + code * 12u) != 0 && get<std::uint8_t>(p, 0x2a4u + code * 12u) != 0;
}
std::uint8_t query_native_joystick_binding_down_00a98780(const void* p, const void* binding) noexcept {
    const auto kind = get<std::uint32_t>(binding, 0);
    if (kind == 0 || kind > 8) return 0;
    const auto index = get<std::uint32_t>(binding, 4);
    const auto value = get<std::int32_t>(get<void*>(p, 0x298), index * 4u);
    if (kind == 1) return value != 0;
    if (kind >= 5) return pov(kind, value);
    if (kind == 2 && get<std::uint8_t>(binding, 8) != 0)
        return value != get<std::int32_t>(get<void*>(p, 0x294), index * 4u);
    const auto span = get<std::int32_t>(get<void*>(p, 0x290), index * 0x1cu + 0x18);
    if (kind == 2) return value != span / 2;
    if (kind == 3) return value < span / 4;
    return value > static_cast<std::int32_t>(static_cast<std::uint32_t>(span) * 3u) / 4;
}
float query_native_joystick_binding_value_00a98940(const void* p, const void* binding,
    const NativeJoystickConstants& constants) noexcept {
    const auto kind = get<std::uint32_t>(binding, 0);
    if (kind == 0 || kind > 8) return 0.0f;
    const auto index = get<std::uint32_t>(binding, 4);
    const auto value = get<std::int32_t>(get<void*>(p, 0x298), index * 4u);
    if (kind == 1) return value != 0 ? constants.one_00d7a24c : 0.0f;
    if (kind >= 5) return pov(kind, value) ? 1.0f : 0.0f;
    const auto row = at(get<void*>(p, 0x290), index * 0x1cu);
    const auto span = get<std::int32_t>(row, 0x18);
    if ((kind == 2 && value == span / 2) || (kind == 3 && value >= span / 2) ||
        (kind == 4 && value <= span / 2)) return 0.0f;
    return axis_value(sub(value, get<std::int32_t>(row, 0x10)), span, kind, constants);
}
std::uint8_t query_native_joystick_down_00a98bd0(const void* p, std::uint32_t code,
    const NativeJoystickConstants& constants) noexcept {
    if (get<std::uint8_t>(p, 0xb3c) != 0) {
        if (code == 21 || code == 22) return get<float>(p, code == 21 ? 0xb40 : 0xb44) > constants.trigger_threshold_00ce3800;
        if (code == 64) return 0;
    }
    if (get<std::uint8_t>(p, 0x28c) == 0 || get<std::uint8_t>(p, 0xb2c) != 0) return 0;
    const auto binding = at(p, 0x29cu + code * 12u);
    return get<std::uint32_t>(binding, 0) == 0 ? 0 : query_native_joystick_binding_down_00a98780(p, binding);
}
float query_native_joystick_value_00a98c50(const void* p, std::uint32_t code,
    const NativeJoystickConstants& constants) noexcept {
    if (get<std::uint8_t>(p, 0xb3c) != 0) {
        if (code == 21 || code == 22) return get<float>(p, code == 21 ? 0xb40 : 0xb44);
        if (code == 64) return 0.0f;
    }
    if (get<std::uint8_t>(p, 0x28c) == 0 || get<std::uint8_t>(p, 0xb2c) != 0) return 0.0f;
    const auto binding = at(p, 0x29cu + code * 12u);
    return get<std::uint32_t>(binding, 0) == 0 ? 0.0f : query_native_joystick_binding_value_00a98940(p, binding, constants);
}
bool poll_native_joystick_00a98e30(void* p, float ignored_seconds, NativeJoystickContext& context) {
    (void)ignored_seconds;
    auto* const first = device(p); put<std::uint8_t>(p, 0x28c, 0);
    if (!first) return false;
    if (FAILED(first->Poll())) { device(p)->Acquire(); return false; }
    const auto bytes = get<std::uint32_t>(p, 0x22c) * 4u;
    const auto current = get<void*>(p, 0x298); const auto previous = get<void*>(p, 0x294);
    if (bytes) std::memcpy(previous, current, bytes);
    auto* const output = get<void*>(p, 0x298);
    auto* const input = device(p);
    if (FAILED(input->GetDeviceState(get<DWORD>(p, 0x280), output))) return false;
    put<std::uint8_t>(p, 0x28c, 1);
    if (get<std::int32_t>(p, 0x22c) > 0) {
        const double threshold = context.constants.deadzone_00ce3d10;
        const double gain = context.constants.deadzone_gain_00d5b7e8;
        for (std::int32_t i = 0; i < get<std::int32_t>(p, 0x22c); ++i) {
            const auto index = static_cast<std::uint32_t>(i);
            const auto row = at(get<void*>(p, 0x290), index * 0x1cu);
            if (get<std::uint8_t>(row, 0xc) != 0) {
                auto* const sample = at(get<void*>(p, 0x298), index * 4u);
                put(sample, 0, deadzone(get<std::int32_t>(sample, 0), get<std::int32_t>(row, 0x18), threshold, gain,
                    &context.constants.sse2_conversion_0109eea4));
            }
            if (get<std::int32_t>(p, 0xb30) != -1) continue;
            const auto kind = get<std::int32_t>(row, 8);
            const auto value = get<std::int32_t>(get<void*>(p, 0x298), index * 4u);
            if (kind == 0 && value != 0) { put(p, 0xb30, i); put<std::int32_t>(p, 0xb34, 0); }
            else if (kind == 1 && value != get<std::int32_t>(row, 0x18) / 2) {
                put(p, 0xb30, i); put<std::int32_t>(p, 0xb34, value >= get<std::int32_t>(row, 0x18) / 2 ? 1 : 0);
            } else if (value != get<std::int32_t>(get<void*>(p, 0x294), index * 4u)) {
                put(p, 0xb30, i); put<std::int32_t>(p, 0xb34, 0);
            } else if (kind == 2 && static_cast<std::uint16_t>(value) != 0xffff &&
                (value == 0 || value == 9000 || value == 18000 || value == 27000)) {
                put(p, 0xb30, i);
                put<std::int32_t>(p, 0xb34, value == 27000 ? 0 : value == 9000 ? 1 : value == 0 ? 2 : 3);
            }
        }
    }
    for (std::uint32_t code = 0; code < 90; ++code) {
        const float value = context.calls.call_device_vslot24(p, get<std::uint32_t>(p, 0), code);
        std::uint32_t bits; std::memcpy(&bits, &value, 4); bits &= 0x7fffffffu;
        float absolute; std::memcpy(&absolute, &bits, 4);
        if (absolute > context.constants.activity_threshold_00ce3868) {
            const auto& timestamp = context.calls.call_clock_01090ab0_vslot14();
            put(p, 0xb38, plus_seconds(timestamp_seconds_x87(timestamp), context.constants.activity_seconds_00ce3d68));
            break;
        }
    }
    if (get<std::uint8_t>(p, 0xb3c) != 0) {
        if (!context.xinput_get_state) throw std::logic_error("native joystick requires XINPUT1_3 ordinal2");
        XINPUT_STATE sample; std::memcpy(&sample, &context.xinput_stack_preimage, sizeof(sample));
        context.xinput_get_state(0, &sample); // native ignores status, including output-preserving failure
        const double maximum = context.constants.trigger_maximum_00ce4b48;
        put(p, 0xb40, trigger_value(sample.Gamepad.bLeftTrigger, maximum));
        put(p, 0xb44, trigger_value(sample.Gamepad.bRightTrigger, maximum));
    }
    return true;
}
void set_native_joystick_force_00a98cc0(void* p, std::uint32_t channel, float value,
    NativeJoystickContext& context) {
    if (get<void*>(p, 0xb24u + channel * 4u) == nullptr) return;
    const auto& timestamp = context.calls.call_clock_01090ab0_vslot14();
    const float deadline = get<float>(p, 0xb38);
    if (timestamp_seconds_x87(timestamp) > deadline) value = 0.0f;
    DIEFFECT effect{};
    effect.dwSize = 0x38; effect.dwFlags = 0x12; effect.dwDuration = 0xffffffffu;
    effect.dwGain = 10000; effect.dwTriggerButton = 0xffffffffu;
    DICONSTANTFORCE constant; DIRAMPFORCE ramp;
    const auto kind = get<std::uint32_t>(p, 0xb10);
    if (kind == 0 || kind == 1) {
        auto force = magnitude(value, context.constants.force_scale_00ce4bd8,
            &context.constants.sse2_conversion_0109eea4);
        const auto mode = get<std::int32_t>(p, 0xb14);
        if (kind == 1 && mode == 0 && channel == 1) force /= 3;
        else if (mode == 1 && channel == 1) force = sub(0, force);
        if (kind == 0) { constant.lMagnitude = force; effect.cbTypeSpecificParams = 4; effect.lpvTypeSpecificParams = &constant; }
        else { ramp.lStart = force; ramp.lEnd = force; effect.cbTypeSpecificParams = 8; effect.lpvTypeSpecificParams = &ramp; }
    }
    auto* const output = get<IDirectInputEffect*>(p, 0xb24u + channel * 4u);
    output->SetParameters(&effect, 0x20000100);
}
} // namespace bsp
