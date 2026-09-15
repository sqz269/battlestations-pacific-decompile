#include "bsp/native_shadow_entry_iteration.hpp"

#include <cstdlib>

namespace bsp {
namespace {

// Concrete current SDK service, matching the established singleton wrappers.
// It can return; never replace it with a noop, unconditional throw or fatal.
__declspec(noinline) void __cdecl current_crt_invalid_parameter() {
    _invalid_parameter_noinfo();
}

// Each complete native function is a separately named one-byte RET. These are
// proved bodies, not placeholder behavior for an unread callee. ECX is unused.
__declspec(naked) void __fastcall native_entry_return_00ae2ca0(void*, void*) {
    __asm { ret }
}

__declspec(naked) void __fastcall native_entry_return_00ae0750(void*, void*) {
    __asm { ret }
}

} // namespace

__declspec(naked) void __fastcall dispatch_native_entry_pass_00ad7440(
    void*, void*) {
    __asm {
        push ebx
        push ebp
        push esi
        mov ebp, ecx
        lea esi, [ebp + 64h]
        push edi
        mov edi, dword ptr [esi + 4]
        cmp edi, dword ptr [esi + 8]
        jbe loc_ad7456
        call current_crt_invalid_parameter
    loc_ad7456:
        mov ebx, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], ebx
        jbe loc_ad7463
        call current_crt_invalid_parameter
    loc_ad7463:
        test esi, esi
        jz loc_ad746b
        cmp esi, esi
        jz loc_ad7470
    loc_ad746b:
        call current_crt_invalid_parameter
    loc_ad7470:
        cmp edi, ebx
        jz loc_ad749d
        test esi, esi
        jnz loc_ad747d
        call current_crt_invalid_parameter
    loc_ad747d:
        cmp edi, dword ptr [esi + 8]
        jb loc_ad7487
        call current_crt_invalid_parameter
    loc_ad7487:
        mov ecx, dword ptr [edi]
        call native_entry_return_00ae2ca0
        cmp edi, dword ptr [esi + 8]
        jb loc_ad7498
        call current_crt_invalid_parameter
    loc_ad7498:
        add edi, 4
        jmp loc_ad7456
    loc_ad749d:
        mov edi, dword ptr [ebp + 58h]
        cmp edi, dword ptr [ebp + 5ch]
        lea esi, [ebp + 54h]
        jbe loc_ad74b0
        call current_crt_invalid_parameter
        // AD74AD: preserve the three-byte LEA ECX,[ECX+0] alignment form.
        _emit 08dh
        _emit 049h
        _emit 000h
    loc_ad74b0:
        mov ebx, dword ptr [esi + 8]
        cmp dword ptr [esi + 4], ebx
        jbe loc_ad74bd
        call current_crt_invalid_parameter
    loc_ad74bd:
        test esi, esi
        jz loc_ad74c5
        cmp esi, esi
        jz loc_ad74ca
    loc_ad74c5:
        call current_crt_invalid_parameter
    loc_ad74ca:
        cmp edi, ebx
        jz loc_ad74f7
        test esi, esi
        jnz loc_ad74d7
        call current_crt_invalid_parameter
    loc_ad74d7:
        cmp edi, dword ptr [esi + 8]
        jb loc_ad74e1
        call current_crt_invalid_parameter
    loc_ad74e1:
        mov ecx, dword ptr [edi]
        call native_entry_return_00ae0750
        cmp edi, dword ptr [esi + 8]
        jb loc_ad74f2
        call current_crt_invalid_parameter
    loc_ad74f2:
        add edi, 4
        jmp loc_ad74b0
    loc_ad74f7:
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}

} // namespace bsp
