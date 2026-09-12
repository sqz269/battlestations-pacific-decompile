#include "bsp/native_singleton_vector_reserve_construct.hpp"

#include "bsp/native_renderer_worker_lifetime.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton reserve and construction require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// A fixed real CRT edge, allowing the actual installed handler to return.
// Keep an ordinary cdecl target for the native five-byte CALL schedule.
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}

void store_word(void* owner, std::uint32_t offset, std::uint32_t value) {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(owner) + offset) = value;
}
} // namespace

__declspec(naked) void __fastcall reserve_native_singleton_slots_00bd0600(
    void*, void*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        mov ecx, dword ptr [esp + 8]
        cmp ecx, 3FFFFFFFh
        jbe capacity_in_range
        call native_singleton_length_error_00bd0590
    capacity_in_range:
        mov edx, dword ptr [esi + 4]
        test edx, edx
        jnz has_storage
        xor eax, eax
        jmp compare_capacity
    has_storage:
        mov eax, dword ptr [esi + 0Ch]
        sub eax, edx
        sar eax, 2
    compare_capacity:
        cmp eax, ecx
        jae finished
        push ebx
        push edi
        xor edx, edx
        call native_singleton_pointer_allocate_00bcfeb0
        mov edi, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], edi
        mov ebx, eax
        jbe end_validated
        call current_crt_invalid_parameter
    end_validated:
        push ebp
        mov ebp, dword ptr [esi + 4]
        cmp ebp, dword ptr [esi + 8]
        jbe begin_validated
        call current_crt_invalid_parameter
    begin_validated:
        sub edi, ebp
        sar edi, 2
        jz copy_finished
        lea eax, [edi * 4]
        push eax
        push ebp
        push eax
        push ebx
        call memmove_s
        add esp, 10h
    copy_finished:
        mov eax, dword ptr [esi + 4]
        test eax, eax
        pop ebp
        jnz current_storage
        xor edi, edi
        jmp free_current_storage
    current_storage:
        mov edi, dword ptr [esi + 8]
        sub edi, eax
        sar edi, 2
    free_current_storage:
        test eax, eax
        jz publish_storage
        push eax
        call singleton_lifetime_free
        // BD0689 is omitted by stale Ghidra flow; installed bytes return here.
        add esp, 4
    publish_storage:
        mov eax, dword ptr [esp + 10h]
        lea edx, [ebx + edi * 4]
        lea ecx, [ebx + eax * 4]
        pop edi
        mov dword ptr [esi + 4], ebx
        mov dword ptr [esi + 0Ch], ecx
        mov dword ptr [esi + 8], edx
        pop ebx
    finished:
        pop esi
        ret 4
    }
}

void* __fastcall construct_native_singleton_manager_00bd0960(
    void* owner, void*) {
    store_word(owner, 4, 0);
    store_word(owner, 8, 0);
    store_word(owner, 0x0c, 0);
    try {
        reserve_native_singleton_slots_00bd0600(owner, nullptr, 256);
        auto* const section = create_native_tracked_critical_section_00bd1860();
        store_word(owner, 0x10, reinterpret_cast<std::uint32_t>(section));
    } catch (...) {
        // Native state0 reloads the captured owner, clears slots, propagates.
        clear_native_singleton_storage_00bd0220(owner, nullptr);
        throw;
    }
    return owner;
}
} // namespace bsp
