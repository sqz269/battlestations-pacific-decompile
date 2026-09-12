#include "bsp/native_resource_registry_construction.hpp"

#include "bsp/native_resource_registry_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry construction requires MSVC Win32.
#endif

namespace bsp {
namespace {

// Fixed bridge for the original one-DWORD operator-new call. The sole caller
// pushes 1Ch; the existing service receives identical native/host sizes.
__declspec(noinline) void* __cdecl allocate_registry_storage(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

// Original normal-path owner operations, excluding native FH3 bookkeeping.
// The public base constructor arms its source catch before calling this body.
// This private raw entry has no cleanup or additional allocation policy.
__declspec(naked) void* __fastcall construct_registry_body(void*) {
    __asm {
        push esi
        push edi
        mov edi, ecx
        lea esi, [edi + 4]
        mov ecx, esi
        mov dword ptr [edi], 0x00d5e594
        call allocate_native_resource_registry_sentinel_00b19c30
        mov dword ptr [esi + 4], eax
        mov byte ptr [eax + 0x19], 1
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 4], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax], eax
        mov eax, dword ptr [esi + 4]
        mov dword ptr [eax + 8], eax
        mov eax, edi
        mov dword ptr [esi + 8], 0
        pop edi
        pop esi
        ret
    }
}

} // namespace

__declspec(naked) void* __cdecl allocate_native_resource_registry_sentinel_00b19c30() {
    __asm {
        push 0x1c
        call allocate_registry_storage
        add esp, 4
        test eax, eax
        jz parent_link
        mov dword ptr [eax], 0
    parent_link:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz right_link
        mov dword ptr [ecx], 0
    right_link:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz node_flags
        mov dword ptr [ecx], 0
    node_flags:
        mov byte ptr [eax + 0x18], 1
        mov byte ptr [eax + 0x19], 0
        ret
    }
}

__declspec(noinline) void* __fastcall construct_native_resource_registry_00b1aa70(
    void* actual_owner, void* volatile& actual_publication_00f8d41c) {
    // Native state0 is armed before the profile store and allocator call.
    // The raw helper performs both only after entry into this source try.
    try {
        return construct_registry_body(actual_owner);
    } catch (...) {
        reset_native_resource_registry_00b19760(
            actual_owner, actual_publication_00f8d41c);
        throw;
    }
}

__declspec(naked) void* __fastcall construct_native_resource_registry_00b1b6e0(
    void*, void* volatile&) {
    __asm {
        push esi
        mov esi, ecx
        call construct_native_resource_registry_00b1aa70
        mov dword ptr [esi], 0x00d5e59c
        mov eax, esi
        pop esi
        ret
    }
}

} // namespace bsp
