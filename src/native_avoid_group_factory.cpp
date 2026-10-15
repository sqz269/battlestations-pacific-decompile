#include "bsp/native_avoid_group_factory.hpp"
#include "bsp/native_avoid_group_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {
__declspec(noinline) void* __cdecl allocate_group_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace

static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, layer_key) == 0x10);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_slots) == 8);
static_assert(offsetof(Cc13AvoidZoneManagerNative, group_count) == 0x58);

__declspec(naked) Cc13AvoidZoneNativeGroup* __fastcall
find_or_create_native_avoid_group_00417ca0(
    Cc13AvoidZoneManagerNative*, void*, std::int32_t) {
    __asm {
        push ebx
        mov ebx, dword ptr [esp + 8] // Native captures layer BEFORE allocation.
        push esi
        push edi
        mov esi, ecx
        xor edi, edi
        xor eax, eax
        cmp dword ptr [esi + 58h], edi
        jle allocate_group
        lea ecx, [esi + 8]
    find_exact:
        mov edx, dword ptr [ecx]
        cmp dword ptr [edx + 10h], ebx
        je found_group
        add eax, 1
        add ecx, 4
        cmp eax, dword ptr [esi + 58h]
        jl find_exact
    allocate_group:
        push 14h
        call allocate_group_bytes // 00417CC9: actual allocator adapter.
        add esp, 4
        cmp eax, edi
        je null_allocation
        mov dword ptr [eax], offset kNativeAvoidGroupLifetimeSourceVtable
        mov dword ptr [eax + 4], edi
        mov dword ptr [eax + 8], edi
        mov dword ptr [eax + 0ch], edi
        mov dword ptr [eax + 10h], 0ffffffffh
        jmp initialize_key
    found_group:
        mov eax, dword ptr [esi + eax * 4 + 8]
        pop edi
        pop esi
        pop ebx
        ret 4
    null_allocation:
        xor eax, eax
    initialize_key:
        mov dword ptr [eax + 10h], ebx
        mov edx, dword ptr [esi + 58h]
        xor ecx, ecx
        cmp edx, edi
        jle insertion_selected
        lea edi, [esi + 8]
        push ebp
        // Native 7-byte LEA ESP,[ESP+00000000h] alignment instruction.
        _emit 08dh
        _emit 0a4h
        _emit 024h
        _emit 000h
        _emit 000h
        _emit 000h
        _emit 000h
    find_position:
        mov ebp, dword ptr [edi]
        cmp dword ptr [ebp + 10h], ebx
        jg position_found
        add ecx, 1
        add edi, 4
        cmp ecx, edx
        jl find_position
    position_found:
        pop ebp
    insertion_selected:
        cmp edx, ecx
        jle append_group
        lea edi, [esi + edx * 4 + 8]
        sub edx, ecx
        // Native 4-byte LEA ESP,[ESP+00h] alignment instruction.
        _emit 08dh
        _emit 064h
        _emit 024h
        _emit 000h
    shift_slot:
        mov ebx, dword ptr [edi - 4]
        mov dword ptr [edi], ebx
        add edi, -4
        sub edx, 1
        jnz shift_slot
        mov dword ptr [esi + ecx * 4 + 8], eax
        add dword ptr [esi + 58h], 1
        pop edi
        pop esi
        pop ebx
        ret 4
    append_group:
        mov dword ptr [esi + edx * 4 + 8], eax
        add dword ptr [esi + 58h], 1
        pop edi
        pop esi
        pop ebx
        ret 4
    }
}
} // namespace bsp
