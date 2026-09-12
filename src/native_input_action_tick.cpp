#include "bsp/native_input_action_tick.hpp"
#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native action tick requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
template<class T> T read(void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
template<class T> void write(void* p, std::uint32_t offset, T value) noexcept {
    *static_cast<volatile T*>(at(p, offset)) = value;
}
bool down(void* record, std::uint32_t latch, std::uint32_t value,
    const volatile float& zero) noexcept {
    if (!read<std::uint8_t>(record, latch)) return false;
    const void* const amount = at(record, value);
    const volatile float* const threshold = &zero;
    std::uint8_t result;
    __asm {
        mov eax, amount
        movss xmm0, dword ptr [eax]
        mov eax, threshold
        comiss xmm0, dword ptr [eax]
        seta result
    }
    return result != 0;
}
}

void update_native_input_action_listener_00a91a50(void* listener, float seconds,
    std::uint8_t previous, std::uint8_t current,
    const NativeInputActionTimingGlobals& globals) noexcept {
    const volatile float* const quick = &globals.quick_edge_and_hold_00e12f20;
    const volatile float* const delay = &globals.repeat_delay_00e12f24;
    const volatile float* const step = &globals.repeat_step_00e12f28;
    float spill;
    // Literal native x87 schedule with address operands bound to live source
    // globals. The compiler owns EBX save/restore; native parameter-slot spills
    // use one private float. No typed listener projection or blanket clear.
    __asm {
        mov ecx, listener
        fld dword ptr [ecx+14h]
        xor edx, edx
        fld seconds
        mov byte ptr [ecx+8], dl
        fld st(0)
        mov byte ptr [ecx+0ah], dl
        faddp st(2), st(0)
        mov byte ptr [ecx+0bh], dl
        fxch
        mov byte ptr [ecx+0dh], dl
        mov byte ptr [ecx+0eh], dl
        fstp spill
        mov byte ptr [ecx+0fh], dl
        fld spill
        mov byte ptr [ecx+10h], dl
        fst dword ptr [ecx+14h]
        fld dword ptr [ecx+18h]
        fadd st(0), st(2)
        fstp spill
        fld spill
        fst dword ptr [ecx+18h]
        mov eax, quick
        fld dword ptr [eax]
        fxch st(2)
        fcomi st(0), st(2)
        ja first_timer_expired
        fxch
        fcomi st(0), st(2)
        fstp st(2)
        jbe timers_ready
        fxch
    clear_history:
        mov byte ptr [ecx+11h], dl
        fxch
        mov byte ptr [ecx+12h], dl
    timers_ready:
        cmp current, dl
        mov ebx, 1
        jz released
        cmp previous, dl
        fstp st(1)
        fld dword ptr [ecx+1ch]
        fadd st(0), st(2)
        fstp spill
        fld spill
        fst dword ptr [ecx+1ch]
        fld dword ptr [ecx+20h]
        faddp st(3), st(0)
        fxch st(2)
        fstp spill
        fld spill
        fst dword ptr [ecx+20h]
        jnz continuing_down
        cmp byte ptr [ecx+11h], dl
        fstp st(0)
        mov byte ptr [ecx+13h], bl
        jz quick_press
        mov byte ptr [ecx+0ah], bl
    quick_press:
        mov eax, quick
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        jbe press_ready
        mov byte ptr [ecx+8], bl
        mov byte ptr [ecx+11h], bl
    press_ready:
        xorps xmm0, xmm0
        movss dword ptr [ecx+18h], xmm0
        jmp hold_test
    first_timer_expired:
        fstp st(2)
        jmp clear_history
    continuing_down:
        fstp st(1)
        mov eax, delay
        fld dword ptr [eax]
        fxch
        fcomi st(0), st(1)
        fstp st(1)
        jbe no_repeat
        mov byte ptr [ecx+0eh], bl
        mov eax, step
        fsub dword ptr [eax]
        fstp dword ptr [ecx+20h]
        jmp hold_test
    no_repeat:
        fstp st(0)
    hold_test:
        mov eax, quick
        fld dword ptr [eax]
        fxch
        fcomip st(0), st(1)
        fstp st(0)
        jc derive_flags
        cmp byte ptr [ecx+13h], dl
        mov byte ptr [ecx+0fh], bl
        jz derive_flags
        mov byte ptr [ecx+10h], bl
        mov byte ptr [ecx+13h], dl
        jmp derive_flags
    released:
        cmp previous, dl
        fstp st(2)
        xorps xmm0, xmm0
        fstp st(1)
        movss dword ptr [ecx+1ch], xmm0
        movss dword ptr [ecx+20h], xmm0
        mov byte ptr [ecx+13h], dl
        jz no_release_edge
        cmp byte ptr [ecx+12h], dl
        jz quick_release
        mov byte ptr [ecx+0dh], bl
    quick_release:
        mov eax, quick
        fld dword ptr [eax]
        fcomip st(0), st(1)
        fstp st(0)
        jbe release_ready
        mov byte ptr [ecx+0bh], bl
        mov byte ptr [ecx+12h], bl
    release_ready:
        movss dword ptr [ecx+14h], xmm0
        jmp derive_flags
    no_release_edge:
        fstp st(0)
    derive_flags:
        cmp byte ptr [ecx+8], dl
        jz no_confirmed_press
        cmp byte ptr [ecx+0ah], dl
        jnz no_confirmed_press
        mov eax, ebx
        jmp store_confirmed_press
    no_confirmed_press:
        xor eax, eax
    store_confirmed_press:
        cmp byte ptr [ecx+0bh], dl
        mov byte ptr [ecx+9], al
        jz no_confirmed_release
        cmp byte ptr [ecx+0dh], dl
        jnz no_confirmed_release
        mov eax, ebx
        jmp store_confirmed_release
    no_confirmed_release:
        xor eax, eax
    store_confirmed_release:
        mov byte ptr [ecx+0ch], al
    }
}

void update_native_input_action_owner_00a92c40(void* owner, float seconds,
    NativeInputActionTickContext& context) {
    void* volatile* const publication = &context.backend_00f8bbf4;
    void* backend;
    std::uint32_t profile;
    float forwarded_seconds;
    __asm {
        fld seconds
        mov eax, publication
        mov ecx, dword ptr [eax]
        mov backend, ecx
        mov eax, dword ptr [ecx]
        mov profile, eax
        fstp forwarded_seconds
    }
    context.calls.backend_vslot04(backend, profile, forwarded_seconds);
    void* const dirty_backend = context.backend_00f8bbf4;
    const auto dirty = read<std::uint8_t>(dirty_backend, 0xd4);
    write<std::uint8_t>(dirty_backend, 0xd4, 0);
    if (dirty) context.calls.call_00a922a0(owner);

    const auto initial_count = read<std::uint32_t>(owner, 8);
    void* record = read<void*>(owner, 4);
    void* endpoint = at(record, initial_count * 0x30u);
    if (record != endpoint) {
        do {
            if (read<std::uint8_t>(record, 1)) {
                context.calls.call_00a92370(record);
                void* const listener = read<void*>(record, 0x2c);
                // NEG/SBB/TEST immediateF8BC00 is only a nonnull test; there
                // is no read of a boolean/global at original F8BC00 here.
                if (listener) {
                    const auto current = down(record, 0x28, 0x24, context.zero_00d7a218);
                    const auto previous = down(record, 0x20, 0x1c, context.zero_00d7a218);
                    float listener_seconds;
                    __asm { fld seconds }
                    __asm { fstp listener_seconds }
                    update_native_input_action_listener_00a91a50(listener, listener_seconds,
                        static_cast<std::uint8_t>(previous), static_cast<std::uint8_t>(current), context.timing);
                }
            }
            const auto count = read<std::uint32_t>(owner, 8);
            endpoint = at(read<void*>(owner, 4), count * 0x30u);
            record = at(record, 0x30);
        } while (record != endpoint);
    }
    const auto callback = context.callback_00f8bbfc;
    if (callback) context.calls.post_tick_callback_00f8bbfc(callback);
}
} // namespace bsp
