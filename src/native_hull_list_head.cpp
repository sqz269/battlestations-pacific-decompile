#include "bsp/native_hull_list_head.hpp"

#include "bsp/cc13_avoid1.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(Cc13AvoidZoneHullListNode) == 0x10);
static_assert(offsetof(Cc13AvoidZoneHullListNode, next) == 0);
static_assert(offsetof(Cc13AvoidZoneHullListNode, previous) == 4);
static_assert(offsetof(Cc13AvoidZoneHullListNode, value) == 8);

namespace {

__declspec(noinline) void* __cdecl allocate_hull_head_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

} // namespace

__declspec(naked) Cc13AvoidZoneHullListNode* __cdecl
buy_native_hull_list_head_0041bf50() {
    __asm {
        push 10h
        call allocate_hull_head_bytes
        add esp, 4
        test eax, eax
        jz next_stored
        mov dword ptr [eax], eax
    next_stored:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz previous_stored
        mov dword ptr [ecx], eax
    previous_stored:
        ret
    }
}

__declspec(naked) void* __fastcall construct_native_hull_list_0041e1b0(void*) {
    __asm {
        push esi
        mov esi, ecx
        call buy_native_hull_list_head_0041bf50
        mov dword ptr [esi + 4], eax
        mov dword ptr [esi + 8], 0
        mov eax, esi
        pop esi
        ret
    }
}

} // namespace bsp
