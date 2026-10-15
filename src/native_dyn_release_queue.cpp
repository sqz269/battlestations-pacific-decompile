#include "bsp/native_dyn_release_queue.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
// Concrete adapter for the native one-DWORD byte-count call. It adds only the
// existing provider's request metadata; zero/wrapped byte counts are retained.
__declspec(noinline) void* __cdecl allocate_queue_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace

__declspec(naked) void __fastcall queue_native_dyn_body_release_00c34f70(
    void*, void*, std::uint32_t) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 440h]
        cmp dword ptr [esi + 43ch], eax
        jnz append_body
        lea eax, [eax + eax + 2]
        mov dword ptr [esi + 440h], eax
        add eax, eax
        add eax, eax
        push edi
        push eax
        call allocate_queue_bytes // 00C34F91 / genuine allocator service
        mov edi, eax
        xor eax, eax
        add esp, 4
        cmp dword ptr [esi + 43ch], eax
        jbe old_words_copied
        mov ecx, edi
    copy_word:
        test ecx, ecx
        jz word_copied
        mov edx, dword ptr [esi + 438h]
        mov edx, dword ptr [edx + eax * 4]
        mov dword ptr [ecx], edx
    word_copied:
        add eax, 1
        add ecx, 4
        cmp eax, dword ptr [esi + 43ch]
        jb copy_word
    old_words_copied:
        mov eax, dword ptr [esi + 438h]
        test eax, eax
        jz old_storage_freed
        push eax
        call singleton_lifetime_free // 00C34FCF / genuine CRT free
        add esp, 4
    old_storage_freed:
        mov dword ptr [esi + 438h], edi
        pop edi
    append_body:
        mov eax, dword ptr [esi + 43ch]
        mov ecx, dword ptr [esi + 438h]
        lea eax, [ecx + eax * 4]
        test eax, eax
        jz body_stored
        mov edx, dword ptr [esp + 8] // 00C34FF1: current writable input slot
        mov dword ptr [eax], edx
    body_stored:
        add dword ptr [esi + 43ch], 1
        pop esi
        ret 4
    }
}
} // namespace bsp
