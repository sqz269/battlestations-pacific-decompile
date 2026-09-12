#include "bsp/native_particle_unit_random.hpp"
#include <cstddef>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle unit random requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleUnitRandomAccess, random) == 0);
static_assert(offsetof(NativeParticleUnitRandomAccess, unsigned_correction_00ce3978) == 4);
static_assert(offsetof(NativeParticleUnitRandomAccess, unit_scale_00d63b80) == 8);
RandomState* __fastcall state_bridge(RandomStream stream,
    const NativeParticleUnitRandomAccess* access) {
    return &access->random->state_00bd2ed0(stream);
}
std::uint32_t __fastcall next_bridge(RandomState* state) {
    return random_next_u32_00ba2c20(*state);
}
} // namespace
__declspec(naked) float __fastcall native_particle_unit_random_00bd2f40(
    RandomStream, const NativeParticleUnitRandomAccess*) {
    __asm {
        push edx // borrowed access; original scratch remains at ESP
        push ecx // 00bd2f40
        call state_bridge // 00bd2f41
        cmp byte ptr [eax + 09c5h],0 // 00bd2f46
        jz draw // 00bd2f4d
        cmp byte ptr [eax + 09c4h],0 // 00bd2f4f
        jnz draw // 00bd2f56
        xor ecx,ecx // 00bd2f58
        mov dword ptr [ecx],3 // 00bd2f5a: original deliberate fault
    draw:
        mov ecx,eax // 00bd2f60
        call next_bridge // 00bd2f62
        test eax,eax // 00bd2f67
        mov dword ptr [esp],eax // 00bd2f69
        fild dword ptr [esp] // 00bd2f6c
        jge scale_value // 00bd2f6f
        mov edx,dword ptr [esp + 4]
        mov edx,dword ptr [edx + 4]
        fadd dword ptr [edx] // 00bd2f71, current00CE3978
    scale_value:
        mov edx,dword ptr [esp + 4]
        mov edx,dword ptr [edx + 8]
        fmul qword ptr [edx] // 00bd2f77, current00D63B80
        fstp dword ptr [esp] // 00bd2f7d
        fld dword ptr [esp] // 00bd2f80
        pop ecx // 00bd2f83
        lea esp,[esp + 4]
        ret // 00bd2f84
    }
}
} // namespace bsp
