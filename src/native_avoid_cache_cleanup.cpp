#include "bsp/native_avoid_cache_cleanup.hpp"
#include "bsp/native_dyn_release_queue.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

namespace bsp {
namespace {

void* allocate_current_hull(void*, std::size_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void release_current_hull(void*, void* block) {
    singleton_lifetime_free(block);
}
__declspec(noinline) void __fastcall destroy_current_hull(AvoidZoneDynHullHandle* handle) {
    const AvoidZoneDynHullMemory memory{nullptr, allocate_current_hull, release_current_hull};
    avoid_zone_dyn_hull_destroy_00c37450(*handle, memory);
}

} // namespace

__declspec(naked) void __fastcall clear_native_avoid_cache_0041e960(
    Cc13AvoidZoneManagerNative*, void* const*) {
    __asm {
        push edx // Explicit Source publication-cell address; no shadow state.
        push ebx
        push esi
        push edi
        mov esi, ecx
        xor edi, edi
    queue_next:
        mov ecx, dword ptr [esi + 60h]
        test ecx, ecx
        jz erase_body_range
        mov eax, dword ptr [esi + 64h]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jnc erase_body_range
        test ecx, ecx
        jz invalid_body_index
        mov eax, dword ptr [esi + 64h]
        sub eax, ecx
        sar eax, 2
        cmp edi, eax
        jc body_index_validated
    invalid_body_index:
        call invoke_native_invalid_parameter_00bf6713 // 0041E98A
    body_index_validated:
        mov eax, dword ptr [esi + 60h]
        mov ecx, dword ptr [eax + edi * 4]
        mov edx, dword ptr [esp + 0ch] // Replaces fixed Native cell address.
        mov edx, dword ptr [edx]       // Current actual publication, every time.
        push ecx
        mov ecx, dword ptr [edx + 18h]
        call queue_native_dyn_body_release_00c34f70 // 0041E99F
        add edi, 1
        jmp queue_next
    erase_body_range:
        mov ebx, dword ptr [esi + 64h]
        cmp ecx, ebx
        jbe old_last_validated
        call invoke_native_invalid_parameter_00bf6713 // 0041E9B0
    old_last_validated:
        mov edi, dword ptr [esi + 60h]
        cmp edi, dword ptr [esi + 64h]
        jbe first_validated
        call invoke_native_invalid_parameter_00bf6713 // 0041E9BD
    first_validated:
        cmp edi, ebx
        jz clear_hull_list
        mov eax, dword ptr [esi + 64h]
        sub eax, ebx
        sar eax, 2
        test eax, eax
        lea ecx, [eax * 4]
        push ebp
        lea ebp, [ecx + edi]
        jle tail_copied
        push ecx
        push ebx
        push ecx
        push edi
        call memmove_s // 0041E9E1; genuine current SDK policy.
        add esp, 10h
    tail_copied:
        mov dword ptr [esi + 64h], ebp
        pop ebp
    clear_hull_list:
        mov eax, dword ptr [esi + 70h]
        mov edi, dword ptr [eax]
        mov dword ptr [eax], eax
        mov eax, dword ptr [esi + 70h]
        mov dword ptr [eax + 4], eax
        cmp edi, dword ptr [esi + 70h]
        mov dword ptr [esi + 74h], 0
        jz cleared
    release_next_hull:
        mov ebx, dword ptr [edi]
        lea ecx, [edi + 8]
        call destroy_current_hull // 0041EA0B; existing typed provider, fixed snapshot.
        push edi
        call singleton_lifetime_free // 0041EA11; concrete matched node free.
        add esp, 4
        cmp ebx, dword ptr [esi + 70h]
        mov edi, ebx
        jnz release_next_hull
    cleared:
        pop edi
        pop esi
        pop ebx
        pop edx // Discard explicit Source context without changing return flags.
        ret
    }
}

} // namespace bsp
