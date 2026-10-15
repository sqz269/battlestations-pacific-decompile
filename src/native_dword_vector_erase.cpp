#include "bsp/native_dword_vector_erase.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "Raw DWORD vector storage requires Win32");

void* __fastcall erase_native_checked_dword_vector_range_006977f0(
    void* receiver, std::uint32_t, void* output,
    const void* first_owner, void* first, const void* last_owner, void* last) {
    const void* captured_owner = first_owner;
    if (captured_owner == nullptr || captured_owner != last_owner)
        invoke_native_invalid_parameter_00bf6713();
    void* result;
    __asm {
        mov esi, first
        mov eax, last
        cmp esi, eax
        je publish_output
        mov edi, receiver
        mov ecx, dword ptr [edi + 8]
        mov edx, ecx
        sub edx, eax
        sar edx, 2
        cmp eax, ecx
        lea ebx, [esi + edx*4]
        je publish_end
        mov edx, esi
        sub edx, eax
copy_current:
        mov edi, dword ptr [eax]
        mov dword ptr [eax + edx], edi
        add eax, 4
        cmp eax, ecx
        jne copy_current
        mov edi, first_owner
        mov captured_owner, edi
publish_end:
        mov edi, receiver
        mov dword ptr [edi + 8], ebx
publish_output:
        mov eax, output
        mov result, eax
        mov dword ptr [eax + 4], esi
        mov ecx, captured_owner
        mov dword ptr [eax], ecx
    }
    return result;
}

void __fastcall clear_native_checked_dword_vector_00697890(
    void* receiver, std::uint32_t) {
    void* original_end;
    void* captured_begin;
    std::uint32_t report;
    __asm {
        mov eax, receiver
        mov edx, dword ptr [eax + 8]
        mov original_end, edx
        xor ecx, ecx
        cmp dword ptr [eax + 4], edx
        seta cl
        mov report, ecx
    }
    if (report != 0)
        invoke_native_invalid_parameter_00bf6713();
    __asm {
        mov eax, receiver
        mov edx, dword ptr [eax + 4]
        mov captured_begin, edx
        xor ecx, ecx
        cmp edx, dword ptr [eax + 8]
        seta cl
        mov report, ecx
    }
    if (report != 0)
        invoke_native_invalid_parameter_00bf6713();
    std::uint32_t private_output[2];
    erase_native_checked_dword_vector_range_006977f0(receiver, 0, private_output,
        receiver, captured_begin, receiver, original_end);
}
} // namespace bsp
