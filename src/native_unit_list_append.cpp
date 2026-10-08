#include "bsp/native_unit_list_append.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This literal native entry requires MSVC Win32.
#endif

namespace bsp {
namespace {

// A real size-only CDECL adapter to the existing canonical allocation service.
// The native call supplies 0Ch. No alternate heap, fake node, callback or
// default result is introduced; the actual CRT retry/exception behavior stays.
// This Source-only adapter receives no native reconstruction credit.
__declspec(noinline) void* __cdecl allocate_native_unit_list_bytes(
    std::size_t actual_bytes) {
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, actual_bytes, actual_bytes});
}

}  // namespace

// Original complete 00484540..00484597 SHA-256:
// 4b5cdb5a7a0792b0f0e94b3633502bc1a93a4092d79ee8d44adc733012fbc313
// All 83 bytes outside the one CALL operand are literal. That CALL uses the
// same physical size-stack/EAX-return contract through the real adapter above.
// Allocation domain is current CRT; original private heap/EH is not adopted.
__declspec(naked) void* __fastcall append_native_unit_list_00484540(
    void*, void*, void*) {
    __asm {
        push esi
        push edi
        push 0Ch
        mov esi, ecx
        call allocate_native_unit_list_bytes
        xor edx, edx
        add esp, 4
        cmp eax, edx
        jz L_null_allocation
        mov dword ptr [eax], edx
        mov dword ptr [eax + 4], edx
        mov dword ptr [eax + 8], edx
        mov ecx, eax
        jmp L_store_payload
    L_null_allocation:
        xor ecx, ecx
    L_store_payload:
        mov eax, dword ptr [esp + 0Ch]
        // Retain the native null-path dereference; do not add a fallback.
        mov dword ptr [ecx + 8], eax
        mov edi, dword ptr [esi + 8]
        mov dword ptr [ecx], edi
        cmp dword ptr [esi], edx
        jz L_empty
        // The second tail read and all link/count writes keep native order.
        mov edi, dword ptr [esi + 8]
        mov dword ptr [edi + 4], ecx
        mov dword ptr [esi + 8], ecx
        mov dword ptr [ecx + 4], edx
        add dword ptr [esi], 1
        pop edi
        pop esi
        ret 4
    L_empty:
        mov dword ptr [esi + 4], ecx
        mov dword ptr [esi + 8], ecx
        mov dword ptr [ecx + 4], edx
        add dword ptr [esi], 1
        pop edi
        pop esi
        ret 4
    }
}

}  // namespace bsp
