#include "bsp/native_system_parameter_slots.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4, "Native parameter storage requires Win32");

void publish_native_system_parameter_slot_00b40860(
    void* parameters_0108fc30, std::uint32_t index, const void* source) noexcept {
    __asm {
        mov edx, source
        mov ecx, index
        fld dword ptr [edx]
        shl ecx, 4
        add ecx, parameters_0108fc30
        fstp dword ptr [ecx]
        fld dword ptr [edx + 4]
        fstp dword ptr [ecx + 4]
        fld dword ptr [edx + 8]
        fstp dword ptr [ecx + 8]
        fld dword ptr [edx + 0ch]
        fstp dword ptr [ecx + 0ch]
    }
}

void set_native_system_parameter_slot0_00b9a030(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* divisor_00ce47a0) noexcept {
    float quotient_spill;
    void* record;
    __asm {
        mov ecx, owner
        mov eax, argument_slots
        fld dword ptr [eax]
        movss xmm0, dword ptr [eax + 4]
        mov edx, divisor_00ce47a0
        fdiv qword ptr [edx]
        lea edx, [ecx + 03bch]
        movss dword ptr [edx + 4], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [edx + 8], xmm0
        movss xmm0, dword ptr [eax + 0ch]
        xor ecx, ecx
        movss dword ptr [edx + 0ch], xmm0
        fstp quotient_spill
        fld quotient_spill
        fstp dword ptr [edx]
        mov record, edx
    }
    publish_native_system_parameter_slot_00b40860(parameters_0108fc30, 0, record);
}

void set_native_system_parameter_slot2_00b9a080(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* one_00d7a24c) noexcept {
    void* record;
    __asm {
        mov ecx, owner
        mov eax, argument_slots
        movss xmm0, dword ptr [eax]
        lea edx, [ecx + 05d0h]
        movss dword ptr [edx], xmm0
        movss xmm0, dword ptr [eax + 4]
        movss dword ptr [ecx + 05d4h], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [ecx + 05d8h], xmm0
        mov eax, one_00d7a24c
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx + 05dch], xmm0
        mov record, edx
    }
    publish_native_system_parameter_slot_00b40860(parameters_0108fc30, 2, record);
}

void set_native_system_parameter_slot3_00b9a0d0(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* one_00d7a24c) noexcept {
    void* record;
    __asm {
        mov ecx, owner
        mov eax, argument_slots
        fld dword ptr [eax]
        movss xmm0, dword ptr [eax]
        fld1
        lea edx, [ecx + 05e0h]
        fdivrp st(1), st(0)
        movss dword ptr [edx], xmm0
        movss xmm0, dword ptr [eax + 4]
        movss dword ptr [ecx + 05e4h], xmm0
        movss xmm0, dword ptr [eax + 8]
        movss dword ptr [ecx + 05e8h], xmm0
        mov eax, one_00d7a24c
        movss xmm0, dword ptr [eax]
        movss dword ptr [ecx + 05ech], xmm0
        fstp dword ptr [ecx + 05f0h]
        mov record, edx
    }
    publish_native_system_parameter_slot_00b40860(parameters_0108fc30, 3, record);
}

void set_native_system_parameter_slot1_00b9a130(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* divisor_00ce47a0, const void* one_00d7a24c) noexcept {
    float quotient_spill;
    void* record;
    __asm {
        mov ecx, owner
        mov eax, argument_slots
        fld dword ptr [eax]
        movss xmm0, dword ptr [eax + 4]
        mov edx, divisor_00ce47a0
        fdiv qword ptr [edx]
        lea edx, [ecx + 03cch]
        movss dword ptr [edx + 4], xmm0
        xorps xmm0, xmm0
        movss dword ptr [edx + 8], xmm0
        mov eax, one_00d7a24c
        movss xmm0, dword ptr [eax]
        mov ecx, 1
        movss dword ptr [edx + 0ch], xmm0
        fstp quotient_spill
        fld quotient_spill
        fstp dword ptr [edx]
        mov record, edx
    }
    publish_native_system_parameter_slot_00b40860(parameters_0108fc30, 1, record);
}

} // namespace bsp
