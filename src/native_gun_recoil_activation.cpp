#include "bsp/native_gun_recoil_activation.hpp"

#include "bsp/native_damageable_class_model.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native gun recoil finalisation requires MSVC Win32 SSE/x87 assembly.
#endif

namespace bsp {
namespace {

// Exact live/disk bytes at00D7A218 and00D7A258 respectively. Integer storage
// prevents source floating expressions from changing the original load widths.
const std::uint32_t kRecoilFloatZero = 0;
const std::uint64_t kRecoilDoubleZero = 0;

// All floating instructions from00730CB0..00730D32, including the reachable
// cleanup tail00730D47..00730D4A. ECX remains the original class throughout.
// Only the original final integer gate/call/AL1 is composed in C++ below.
__declspec(naked) void __fastcall run_recoil_loop_00730cb0(const void*) {
    __asm {
        sub esp, 8 // 00730cb0
        movss xmm0, dword ptr [ecx + 0b0h] // 00730cb3
        ucomiss xmm0, dword ptr [kRecoilFloatZero] // 00730cbb
        lahf // 00730cc2
        test ah, 44h // 00730cc3
        movss dword ptr [esp + 4], xmm0 // 00730cc6
        jnp recoil_done // 00730ccc: ordered equal alone skips the loop
        fld dword ptr [ecx + 0a8h] // 00730cce
        fstp dword ptr [esp] // 00730cd4
        fld dword ptr [esp] // 00730cd7
        fld qword ptr [kRecoilDoubleZero] // 00730cda
        fadd st(0), st(1) // 00730ce0
        fstp dword ptr [esp] // 00730ce2
        fld dword ptr [esp] // 00730ce5
        fld dword ptr [esp + 4] // 00730ce8
        fcomi st(0), st(1) // 00730cec
        ja recoil_initial_exit // 00730cee
        movss xmm0, dword ptr [ecx + 0ach] // 00730cf0
        movss dword ptr [esp + 4], xmm0 // 00730cf8
        fld dword ptr [esp + 4] // 00730cfe
        jmp recoil_multiply // 00730d02

    recoil_again:
        fxch st(2) // 00730d04
        fxch st(3) // 00730d06
        fxch st(1) // 00730d08
        fxch st(2) // 00730d0a
        fxch st(1) // 00730d0c
    recoil_multiply:
        fld st(0) // 00730d0e
        fmulp st(4), st(0) // 00730d10
        fxch st(3) // 00730d12
        fstp dword ptr [esp] // 00730d14
        fld dword ptr [esp] // 00730d17
        fld st(0) // 00730d1a
        faddp st(3), st(0) // 00730d1c
        fxch st(2) // 00730d1e
        fstp dword ptr [esp] // 00730d20
        fld dword ptr [esp] // 00730d23
        fxch st(1) // 00730d26
        fcomi st(0), st(1) // 00730d28
        jbe recoil_again // 00730d2a: includes unordered, no iteration limit
        fstp st(2) // 00730d2c
        fstp st(0) // 00730d2e
    recoil_pop_two:
        fstp st(0) // 00730d30
        fstp st(0) // 00730d32
    recoil_done:
        add esp, 8
        ret
    recoil_initial_exit:
        fstp st(2) // 00730d47: reachable beyond native RET00730d46
        jmp recoil_pop_two // 00730d49
    }
}

} // namespace

std::uint8_t finalise_native_gun_class_00730cb0(void* actual_class,
    NativeDamageableClassModelContext& context,
    NativeDamageableClassModelAcquired& acquired) {
    run_recoil_loop_00730cb0(actual_class);
    // Native00730D34 performs this load after every reached FP instruction.
    // Volatile retains that read even when a compiler can see caller storage.
    const auto* const bytes = static_cast<const std::uint8_t*>(actual_class);
    if (*reinterpret_cast<const volatile std::uint32_t*>(bytes + 0x38) != 0)
        load_native_damageable_class_model_00879590(actual_class, 0, context, acquired);
    return 1;
}

} // namespace bsp
