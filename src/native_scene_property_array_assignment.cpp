#include "bsp/native_scene_property_array_assignment.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {

// Genuine current CDECL exact-size bridge; no Native-body credit.
__declspec(noinline) void* __cdecl allocate_native_array_assignment_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

} // namespace

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4 &&
    native_scene_property_array_assignment_record_bytes == 56,
    "The physical array-assignment ABI requires MSVC Win32.");

// Whole86 SHA256 1aa4e7ed871e8547fdc979337d00ec3017e793b3c63be3ea630a672c81151ea0.
// Only CALL operands [13,17),[43,47),[58,62) bind current providers.
// The other 74 bytes retain both RET0C and every original read/store order.
__declspec(naked) void* __fastcall reassign_native_scene_property_raw_bytes_008f0460(
    void*, void*, const void*, std::uint32_t, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 20h]
        test eax, eax
        push edi
        jz L_old_null
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 20h], 0
    L_old_null:
        cmp byte ptr [esp + 14h], 0
        mov edi, dword ptr [esp + 10h]
        mov dword ptr [esi + 24h], edi
        jz L_transfer
        push edi
        call allocate_native_array_assignment_bytes
        mov ecx, dword ptr [esp + 10h]
        push edi
        push ecx
        push eax
        mov dword ptr [esi + 20h], eax
        call memcpy
        add esp, 10h
        pop edi
        mov eax, esi
        pop esi
        ret 0ch
    L_transfer:
        mov edx, dword ptr [esp + 0ch]
        pop edi
        mov dword ptr [esi + 20h], edx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

// Whole90 SHA256 07c8d0cca1c813e7496447bc2834fcff05897b6f1d5f7886cf17e96b336026bb.
// Only CALL operands [21,25),[47,51),[62,66) bind current providers;78 literal bytes.
__declspec(naked) void* __fastcall reassign_native_scene_property_four_byte_groups_008f04c0(
    void*, void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        mov eax, dword ptr [esi + 20h]
        add edi, edi
        add edi, edi
        test eax, eax
        jz L_old_null
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 20h], 0
    L_old_null:
        cmp byte ptr [esp + 14h], 0
        mov dword ptr [esi + 24h], edi
        jz L_transfer
        push edi
        call allocate_native_array_assignment_bytes
        mov ecx, dword ptr [esp + 14h]
        push edi
        push ecx
        push eax
        mov dword ptr [esi + 20h], eax
        call memcpy
        add esp, 10h
        pop edi
        mov eax, esi
        pop esi
        ret 0ch
    L_transfer:
        mov edx, dword ptr [esp + 10h]
        pop edi
        mov dword ptr [esi + 20h], edx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

// Whole90 SHA256 0c17b9c96e87c2b67b337c81a3abf3856ae7787f0ffc2edff04546a879d2ad37.
// Only CALL operands [21,25),[47,51),[62,66) bind current providers;78 literal bytes.
__declspec(naked) void* __fastcall reassign_native_scene_property_four_byte_groups_008f0520(
    void*, void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        push esi
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov esi, ecx
        mov eax, dword ptr [esi + 20h]
        add edi, edi
        add edi, edi
        test eax, eax
        jz L_old_null
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 20h], 0
    L_old_null:
        cmp byte ptr [esp + 14h], 0
        mov dword ptr [esi + 24h], edi
        jz L_transfer
        push edi
        call allocate_native_array_assignment_bytes
        mov ecx, dword ptr [esp + 14h]
        push edi
        push ecx
        push eax
        mov dword ptr [esi + 20h], eax
        call memcpy
        add esp, 10h
        pop edi
        mov eax, esi
        pop esi
        ret 0ch
    L_transfer:
        mov edx, dword ptr [esp + 10h]
        pop edi
        mov dword ptr [esi + 20h], edx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

// Whole93 SHA256 3dc15a9fde6100ce0c7362250fa4af5882e65112edde826da325d9fd94d80aed.
// Only CALL operands [24,28),[50,54),[65,69) bind current providers;81 literal bytes.
__declspec(naked) void* __fastcall reassign_native_scene_property_twelve_byte_groups_008f0580(
    void*, void*, std::uint32_t, const void*, std::uint32_t) {
    __asm {
        mov eax, dword ptr [esp + 4]
        push esi
        push edi
        lea edi, [eax + eax * 2]
        mov esi, ecx
        mov eax, dword ptr [esi + 20h]
        add edi, edi
        add edi, edi
        test eax, eax
        jz L_old_null
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 20h], 0
    L_old_null:
        cmp byte ptr [esp + 14h], 0
        mov dword ptr [esi + 24h], edi
        jz L_transfer
        push edi
        call allocate_native_array_assignment_bytes
        mov ecx, dword ptr [esp + 14h]
        push edi
        push ecx
        push eax
        mov dword ptr [esi + 20h], eax
        call memcpy
        add esp, 10h
        pop edi
        mov eax, esi
        pop esi
        ret 0ch
    L_transfer:
        mov edx, dword ptr [esp + 10h]
        pop edi
        mov dword ptr [esi + 20h], edx
        mov eax, esi
        pop esi
        ret 0ch
    }
}

} // namespace bsp
