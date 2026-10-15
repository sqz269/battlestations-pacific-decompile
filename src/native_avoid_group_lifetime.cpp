#include "bsp/native_avoid_group_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {

static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, zones) == 0x04);
static_assert(offsetof(Cc13AvoidZoneNativeGroup, count) == 0x08);
static_assert(offsetof(AvoidZoneNativeStorage, corners) == 0x00);
static_assert(sizeof(void*) == 4);

namespace {

// The normal destructor and its recovered unwind action both consume the
// current +04h field. Capture that pointer before free; null the field after
// free returns. This private Source helper does not claim a new Native entry.
__declspec(naked) void __fastcall release_current_group_slot_array(
    Cc13AvoidZoneNativeGroup*) noexcept {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        jz done
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi + 4], 0
    done:
        pop esi
        ret
    }
}

struct CurrentGroupArrayUnwindGuard {
    Cc13AvoidZoneNativeGroup* group;
    bool active = true;

    ~CurrentGroupArrayUnwindGuard() noexcept {
        if (active) release_current_group_slot_array(group);
    }
};

} // namespace

const NativeAvoidGroupLifetimeSourceVtable kNativeAvoidGroupLifetimeSourceVtable{
    scalar_delete_native_avoid_group_0041e940};

__declspec(naked) void __fastcall clear_native_avoid_group_0041d0c0(
    Cc13AvoidZoneNativeGroup*) {
    __asm {
        push ebx
        push ebp
        mov ebp, ecx
        mov eax, dword ptr [ebp + 8]
        mov ecx, dword ptr [ebp + 4]
        mov ebx, dword ptr [ebp + 4]
        lea edx, [ecx + eax * 4]
        cmp ebx, edx
        jz clear_count
        push esi
        push edi
    next_zone:
        mov esi, dword ptr [ebx]
        test esi, esi
        jz advance_zone
        mov eax, dword ptr [esi + 4]
        xor edi, edi
        test eax, eax
        jbe clear_zone_count
    next_record:
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [ecx + edi * 4]
        push edx
        call singleton_lifetime_free
        mov eax, dword ptr [esi + 4]
        add edi, 1
        add esp, 4
        cmp edi, eax
        jb next_record
    clear_zone_count:
        mov eax, dword ptr [esi]
        test eax, eax
        mov dword ptr [esi + 4], 0
        jz free_zone
        push eax
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [esi], 0
    free_zone:
        push esi
        call singleton_lifetime_free
        add esp, 4
        mov dword ptr [ebx], 0
    advance_zone:
        mov ecx, dword ptr [ebp + 8]
        mov edx, dword ptr [ebp + 4]
        add ebx, 4
        lea eax, [edx + ecx * 4]
        cmp ebx, eax
        jne next_zone
        pop edi
        pop esi
    clear_count:
        mov dword ptr [ebp + 8], 0
        pop ebp
        pop ebx
        ret
    }
}

__declspec(noinline) void __fastcall destroy_native_avoid_group_0041de70(
    Cc13AvoidZoneNativeGroup* group) {
    group->vtable = reinterpret_cast<std::uint32_t>(
        &kNativeAvoidGroupLifetimeSourceVtable);
    // Native state 0 is active before clear and through the normal free/null.
    // This real current C++ guard retains the corresponding cleanup operation;
    // Native FH3 registration/funclet/frame identity is not transplanted.
    // MSVC otherwise infers that this TU's naked clear cannot throw and removes
    // the registration. This local volatile read keeps current C++ unwinding
    // active over the actual fixed entry; it is not an injectable provider.
    using ConcreteClearEntry = void (__fastcall*)(Cc13AvoidZoneNativeGroup*);
    ConcreteClearEntry volatile clear_entry = clear_native_avoid_group_0041d0c0;
    CurrentGroupArrayUnwindGuard cleanup{group};
    clear_entry(group);
    release_current_group_slot_array(group);
    cleanup.active = false;
}

__declspec(naked) Cc13AvoidZoneNativeGroup* __fastcall
scalar_delete_native_avoid_group_0041e940(
    Cc13AvoidZoneNativeGroup*, void*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        call destroy_native_avoid_group_0041de70
        test byte ptr [esp + 8], 1
        jz retained
        push esi
        call singleton_lifetime_free
        add esp, 4
    retained:
        mov eax, esi
        pop esi
        ret 4
    }
}

} // namespace bsp
