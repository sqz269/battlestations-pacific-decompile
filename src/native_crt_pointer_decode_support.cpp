#include "bsp/native_crt_pointer_decode_support.hpp"

#include "bsp/legacy_crt_math.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native CRT pointer-decode support requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
alignas(8) const char mixcrt_name[8] = {'.', 'm', 'i', 'x', 'c', 'r', 't', '\0'};

// Actual image reads, with native width/order even when the following branch
// does not need the value. Inline assembly avoids typed C++ alias assumptions
// about the current mapped module and preserves the zero-section read.
std::uint32_t image_dword(std::uintptr_t address) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
std::uint16_t image_word(std::uintptr_t address) noexcept {
    std::uint16_t value;
    __asm {
        mov eax, address
        mov ax, word ptr [eax]
        mov value, ax
    }
    return value;
}
} // namespace

__declspec(naked) std::int32_t __cdecl native_crt_strcmp_00c05920(
    const char*, const char*) noexcept {
    __asm {
        mov edx, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        test edx, 3
        jne unaligned_first
    compare_dword:
        mov eax, dword ptr [edx]
        cmp al, byte ptr [ecx]
        jne different
        or al, al
        je equal
        cmp ah, byte ptr [ecx + 1]
        jne different
        or ah, ah
        je equal
        shr eax, 16
        cmp al, byte ptr [ecx + 2]
        jne different
        or al, al
        je equal
        cmp ah, byte ptr [ecx + 3]
        jne different
        add ecx, 4
        add edx, 4
        or ah, ah
        jne compare_dword
        mov edi, edi
    equal:
        xor eax, eax
        ret
        nop
    different:
        sbb eax, eax
        shl eax, 1
        add eax, 1
        ret
    unaligned_first:
        test edx, 1
        je compare_word
        mov al, byte ptr [edx]
        add edx, 1
        cmp al, byte ptr [ecx]
        jne different
        add ecx, 1
        or al, al
        je equal
        test edx, 2
        je compare_dword
    compare_word:
        mov ax, word ptr [edx]
        add edx, 2
        cmp al, byte ptr [ecx]
        jne different
        or al, al
        je equal
        cmp ah, byte ptr [ecx + 1]
        jne different
        or ah, ah
        je equal
        add ecx, 2
        jmp compare_dword
    }
}

std::int32_t get_native_crt_winmajor_00bfbb61(
    std::uint32_t* output, const NativeCrtPointerDecodeSupportContext& context) {
    auto* const captured_output = output;
    if (!captured_output || context.osplatform_0109dd84 == 0) {
        auto* const current_errno = context.owning_crt.errno_location_00bffb8b();
        *current_errno = 22;
        // Existing actual all-zero CRT invalid-parameter service. It may
        // return; the native failure path neither retries nor stores output.
        context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
        return 22;
    }
    *captured_output = context.winmajor_0109dd90;
    return 0;
}

std::int32_t native_crt_pointer_decode_module_gate_00c04efb(
    const NativeCrtPointerDecodeSupportContext& context) {
    std::uint32_t major = 0;
    std::int32_t result = 1;
    (void)get_native_crt_winmajor_00bfbb61(&major, context);
    if (static_cast<std::int32_t>(major) > 5) return 1;

    const auto module = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
    const auto nt = module + image_dword(module + 0x3cU);
    const auto initial_count = image_word(nt + 6U);
    const auto optional_header_size = image_word(nt + 0x14U);
    auto section = nt + 0x18U + optional_header_size;
    std::uint32_t index = 0;
    if (initial_count != 0) {
        for (;;) {
            if (native_crt_strcmp_00c05920(
                    mixcrt_name, reinterpret_cast<const char*>(section)) == 0) {
                result = 0;
                break;
            }
            const auto current_count = image_word(nt + 6U);
            ++index;
            section += 0x28U;
            if (index >= current_count) break;
        }
    }
    return result;
}
} // namespace bsp
