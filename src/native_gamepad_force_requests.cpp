#include "bsp/native_gamepad_force_requests.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "raw request ABI requires Win32");
template<class T> T read(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
void* allocate(std::size_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void* delete_request(void* request, std::uint32_t flags) noexcept {
    void* const identity = request;
    write(request, 0, native_force_request_base_profile);
    if ((flags & 1) != 0) singleton_lifetime_free(request);
    return identity;
}
[[noreturn]] void pure_slot() {
    (void)_purecall(); // Actual CRT service; not an empty base implementation.
    std::terminate(); // The CRT contract does not return, including after its hook.
}
[[noreturn]] void unknown_profile() {
    throw std::invalid_argument("unbound raw gamepad force request profile");
}
}

void* construct_native_constant_force_request_008722d0(void* request,
    std::uint32_t channel, float amplitude, float remaining) noexcept {
    write(request, 0xc, amplitude);
    write<std::uint32_t>(request, 4, 0);
    write(request, 0, native_constant_force_request_profile);
    write(request, 8, channel);
    write(request, 0x10, remaining);
    return request;
}
void* construct_native_fading_force_request_00872350(void* request,
    std::uint32_t channel, float amplitude, float duration) noexcept {
    write(request, 0xc, amplitude);
    write(request, 0x10, duration);
    write<std::uint32_t>(request, 4, 1);
    write(request, 0, native_fading_force_request_profile);
    write(request, 8, channel);
    write<std::uint32_t>(request, 0x14, 0);
    return request;
}
void* construct_native_alternating_force_request_008723f0(void* request,
    std::uint32_t channel, std::uint8_t selector, float first_value, float second_value,
    float first_period, float second_period, float duration) noexcept {
    write(request, 0x10, first_value);
    write(request, 0x14, second_value);
    write(request, 0x18, first_period);
    write(request, 0x1c, second_period);
    write(request, 0x20, duration);
    write<std::uint32_t>(request, 4, 2);
    write(request, 0, native_alternating_force_request_profile);
    write(request, 8, channel);
    write(request, 0xc, selector); // BYTE only; preserve +D..+F.
    write<std::uint32_t>(request, 0x24, 0);
    return request;
}
void* allocate_native_constant_force_request(std::uint32_t channel,
    float amplitude, float remaining) {
    void* const request = allocate(0x14);
    return request ? construct_native_constant_force_request_008722d0(
        request, channel, amplitude, remaining) : nullptr;
}
void* allocate_native_fading_force_request(std::uint32_t channel,
    float amplitude, float duration) {
    void* const request = allocate(0x18);
    return request ? construct_native_fading_force_request_00872350(
        request, channel, amplitude, duration) : nullptr;
}
void* allocate_native_alternating_force_request(std::uint32_t channel,
    std::uint8_t selector, float first_value, float second_value, float first_period,
    float second_period, float duration) {
    void* const request = allocate(0x28);
    return request ? construct_native_alternating_force_request_008723f0(request,
        channel, selector, first_value, second_value, first_period, second_period, duration) : nullptr;
}

void* delete_native_force_request_base_008724c0(void* request, std::uint32_t flags) noexcept {
    return delete_request(request, flags);
}
void* delete_native_constant_force_request_008724e0(void* request, std::uint32_t flags) noexcept {
    return delete_request(request, flags);
}
void* delete_native_fading_force_request_00872500(void* request, std::uint32_t flags) noexcept {
    return delete_request(request, flags);
}
void* delete_native_alternating_force_request_00872520(void* request, std::uint32_t flags) noexcept {
    return delete_request(request, flags);
}
std::uint32_t native_constant_force_request_channel_00872310(const void* request) noexcept {
    return read<std::uint32_t>(request, 8);
}
std::uint32_t native_fading_force_request_channel_008723a0(const void* request) noexcept {
    return read<std::uint32_t>(request, 8);
}
std::uint32_t native_alternating_force_request_channel_00872460(const void* request) noexcept {
    return read<std::uint32_t>(request, 8);
}

// Naked cdecl source boundaries retain the original ST0 return and intermediate
// spill instructions. The only ABI translation is loading this from [ESP+4].
__declspec(naked) float native_constant_force_request_value_00872320(const void*) noexcept {
    __asm {
        mov ecx,dword ptr [esp+4]
        fld dword ptr [ecx+0ch]
        ret
    }
}
__declspec(naked) float native_fading_force_request_value_008723b0(const void*) noexcept {
    __asm {
        mov ecx,dword ptr [esp+4]
        push ecx
        fld dword ptr [ecx+10h]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld st(0)
        fsub dword ptr [ecx+14h]
        fdivrp st(1),st(0)
        fmul dword ptr [ecx+0ch]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        pop ecx
        ret
    }
}
__declspec(naked) float native_alternating_force_request_value_00872470(const void*) noexcept {
    __asm {
        mov ecx,dword ptr [esp+4]
        cmp byte ptr [ecx+0ch],0
        fld dword ptr [ecx+24h]
        jz first_period
        fld dword ptr [ecx+1ch]
        fcomip st(0),st(1)
        fstp st(0)
        ja second_value
    first_value:
        fld dword ptr [ecx+10h]
        ret
    first_period:
        fld dword ptr [ecx+18h]
        fcomip st(0),st(1)
        fstp st(0)
        ja first_value
    second_value:
        fld dword ptr [ecx+14h]
        ret
    }
}
bool native_constant_force_request_expired_00872330(const void* request) noexcept {
    std::uint32_t result;
    __asm {
        mov ecx,request
        xorps xmm0,xmm0
        comiss xmm0,dword ptr [ecx+10h]
        jb unexpired
        mov eax,1
        jmp compared
    unexpired:
        xor eax,eax
    compared:
        mov result,eax
    }
    return result != 0;
}
bool native_fading_force_request_expired_008723d0(const void* request) noexcept {
    std::uint32_t result;
    __asm {
        mov ecx,request
        fld dword ptr [ecx+10h]
        fld dword ptr [ecx+14h]
        fcomip st(0),st(1)
        fstp st(0)
        jb unexpired
        mov eax,1
        jmp compared
    unexpired:
        xor eax,eax
    compared:
        mov result,eax
    }
    return result != 0;
}
bool native_alternating_force_request_expired_008724a0(const void* request) noexcept {
    std::uint32_t result;
    __asm {
        mov ecx,request
        fld dword ptr [ecx+20h]
        fld dword ptr [ecx+24h]
        fcomip st(0),st(1)
        fstp st(0)
        jb unexpired
        mov eax,1
        jmp compared
    unexpired:
        xor eax,eax
    compared:
        mov result,eax
    }
    return result != 0;
}
void update_native_constant_force_request_00a93f90(void* request, float seconds,
    const volatile double& infinite_remaining) noexcept {
    const volatile double* const sentinel = &infinite_remaining;
    float spill;
    __asm {
        mov ecx,request
        fld dword ptr [ecx+10h]
        fstp spill
        fld spill
        fld st(0)
        mov eax,sentinel
        fld qword ptr [eax]
        fxch
        fucomip st(0),st(1)
        fstp st(0)
        lahf
        test ah,044h
        jnp unchanged
        fsub seconds
        fstp dword ptr [ecx+10h]
        jmp finished
    unchanged:
        fstp st(0)
    finished:
    }
}
void update_native_fading_force_request_00a93fc0(void* request, float seconds) noexcept {
    __asm {
        mov ecx,request
        fld seconds
        fadd dword ptr [ecx+14h]
        fstp dword ptr [ecx+14h]
    }
}
void update_native_alternating_force_request_00a94510(void* request, float seconds) {
    float next, period;
    __asm {
        mov ecx,request
        fld seconds
        fadd dword ptr [ecx+24h]
        fstp next
        fld dword ptr [ecx+1ch]
        fadd dword ptr [ecx+18h]
        fstp period
    }
    // BF857A/E15500 is the recognized CRT fmod operation, with original
    // ST1=rounded next and ST0=rounded period. Reuse the genuine host CRT;
    // original CRT dispatcher/error-hook binary identity is not claimed.
    const float result = static_cast<float>(std::fmod(
        static_cast<double>(next), static_cast<double>(period)));
    write(request, 0x24, result);
}

NativeGamepadForceRequestDispatch::NativeGamepadForceRequestDispatch(
    const volatile double& sentinel) noexcept : infinite_remaining_(sentinel) {}
void NativeGamepadForceRequestDispatch::request_delete_vslot00(void* request,
    std::uint32_t flags) {
    switch (read<std::uint32_t>(request, 0)) {
    case native_force_request_base_profile: delete_native_force_request_base_008724c0(request, flags); return;
    case native_constant_force_request_profile: delete_native_constant_force_request_008724e0(request, flags); return;
    case native_fading_force_request_profile: delete_native_fading_force_request_00872500(request, flags); return;
    case native_alternating_force_request_profile: delete_native_alternating_force_request_00872520(request, flags); return;
    default: unknown_profile();
    }
}
std::uint32_t NativeGamepadForceRequestDispatch::request_channel_vslot04(void* request) {
    switch (read<std::uint32_t>(request, 0)) {
    case native_force_request_base_profile: pure_slot();
    case native_constant_force_request_profile: return native_constant_force_request_channel_00872310(request);
    case native_fading_force_request_profile: return native_fading_force_request_channel_008723a0(request);
    case native_alternating_force_request_profile: return native_alternating_force_request_channel_00872460(request);
    default: unknown_profile();
    }
}
float NativeGamepadForceRequestDispatch::request_value_vslot08(void* request) {
    switch (read<std::uint32_t>(request, 0)) {
    case native_force_request_base_profile: pure_slot();
    case native_constant_force_request_profile: return native_constant_force_request_value_00872320(request);
    case native_fading_force_request_profile: return native_fading_force_request_value_008723b0(request);
    case native_alternating_force_request_profile: return native_alternating_force_request_value_00872470(request);
    default: unknown_profile();
    }
}
bool NativeGamepadForceRequestDispatch::request_expired_vslot0c(void* request) {
    switch (read<std::uint32_t>(request, 0)) {
    case native_force_request_base_profile: pure_slot();
    case native_constant_force_request_profile: return native_constant_force_request_expired_00872330(request);
    case native_fading_force_request_profile: return native_fading_force_request_expired_008723d0(request);
    case native_alternating_force_request_profile: return native_alternating_force_request_expired_008724a0(request);
    default: unknown_profile();
    }
}
void NativeGamepadForceRequestDispatch::request_update_vslot10(void* request, float seconds) {
    switch (read<std::uint32_t>(request, 0)) {
    case native_force_request_base_profile: pure_slot();
    case native_constant_force_request_profile: update_native_constant_force_request_00a93f90(request, seconds, infinite_remaining_); return;
    case native_fading_force_request_profile: update_native_fading_force_request_00a93fc0(request, seconds); return;
    case native_alternating_force_request_profile: update_native_alternating_force_request_00a94510(request, seconds); return;
    default: unknown_profile();
    }
}

} // namespace bsp
