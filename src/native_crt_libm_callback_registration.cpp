#include "bsp/native_crt_libm_callback_registration.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT libm callback registration requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
// MSVC reference-member bindings are checked by compiled member accessors in
// the ignored static proof; offsetof addresses the referent for this type.
static_assert(sizeof(NativeCrtPointerEncodeContext) == 16);
static_assert(sizeof(NativeCrtLibmCallbackRegistrationContext) == 12);

// Exact immutable native string bytes D69FD0[13] and D69FC0[14]. Actual OS
// named imports consume these bytes; this does not expose a resolver callback.
const char kernel32_name[] = "KERNEL32.DLL";
const char encode_pointer_name[] = "EncodePointer";

__declspec(noinline) std::int32_t __cdecl encode_module_gate(
    const NativeCrtPointerEncodeContext& context) {
    return native_crt_pointer_decode_module_gate_00c04efb(context.module_gate);
}
} // namespace

__declspec(naked) void* __cdecl native_crt_encode_pointer_00c04f67(
    void*, const NativeCrtPointerEncodeContext&) {
    __asm {
        push esi
        // Context reloads address the stable added word. Native state is not
        // copied into context or cached across the original read boundaries.
        mov ecx, dword ptr [esp + 0Ch]
        mov ecx, dword ptr [ecx]
        push dword ptr [ecx]             // current E15B00 before IAT capture
        mov ecx, dword ptr [esp + 10h]
        mov ecx, dword ptr [ecx + 8]
        mov esi, dword ptr [ecx]         // actual current CE20BC function word
        call esi
        test eax, eax
        jz fallback
        mov ecx, dword ptr [esp + 0Ch]
        mov ecx, dword ptr [ecx + 4]
        mov eax, dword ptr [ecx]         // current E15AFC
        cmp eax, -1
        jz fallback
        push eax                        // capture PTD index across second call
        mov ecx, dword ptr [esp + 10h]
        mov ecx, dword ptr [ecx]
        push dword ptr [ecx]             // current E15B00 again
        call esi                        // same captured real import
        call eax                        // newly returned actual raw getter
        test eax, eax
        jz fallback
        mov eax, dword ptr [eax + 1F8h]  // current actual PTD encoder
        jmp selected
    fallback:
        push offset kernel32_name
        call GetModuleHandleA
        mov esi, eax
        test esi, esi
        jz identity
        push dword ptr [esp + 0Ch]
        call encode_module_gate
        add esp, 4
        test eax, eax
        jz identity
        push offset encode_pointer_name
        push esi                        // module captured BEFORE gate
        call GetProcAddress
    selected:
        test eax, eax
        jz identity
        push dword ptr [esp + 8]         // actual first caller argument slot
        call eax                        // actual stdcall encoder
        mov dword ptr [esp + 8], eax     // original observable slot writeback
    identity:
        mov eax, dword ptr [esp + 8]
        pop esi
        ret
    }
}

__declspec(naked) void __cdecl register_native_crt_libm_callback_00c0f0bb(
    void*, const NativeCrtLibmCallbackRegistrationContext&) {
    __asm {
        cmp dword ptr [esp + 4], 0
        jne enable_callback
        mov ecx, dword ptr [esp + 8]
        mov ecx, dword ptr [ecx + 4]
        and dword ptr [ecx], 0           // real flag RMW; encoded stays stale
        ret
    enable_callback:
        mov ecx, dword ptr [esp + 8]
        push dword ptr [ecx]             // encoder context, new second argument
        push dword ptr [esp + 8]         // reread current original first slot
        call native_crt_encode_pointer_00c04f67
        add esp, 8
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [ecx + 8]
        mov dword ptr [edx], eax         // encoded result publication first
        mov ecx, dword ptr [ecx + 4]
        mov dword ptr [ecx], 1           // actual flag publication second
        ret
    }
}
} // namespace bsp
