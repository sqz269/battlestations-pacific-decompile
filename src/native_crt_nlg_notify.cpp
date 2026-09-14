#include "bsp/native_crt_nlg_notify.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native NLG register entry requires MSVC Win32.
#endif

namespace bsp {
static_assert(offsetof(game::NativeCrtNlgDescriptor, signature) == 0);
static_assert(offsetof(game::NativeCrtNlgDescriptor, destination) == 4);
static_assert(offsetof(game::NativeCrtNlgDescriptor, code) == 8);
static_assert(offsetof(game::NativeCrtNlgDescriptor, frame) == 12);

// Keep the actual canonical address and every original instruction. Calling
// an accessor here would change the native register, stack and flag contract.
__declspec(naked) void __stdcall notify_native_crt_nlg_00c16879(std::uint32_t) {
    __asm {
        push ebx
        push ecx
        mov ebx, 00e16830h
        mov ecx, dword ptr [esp + 0ch]
        mov dword ptr [ebx + 8], ecx
        mov dword ptr [ebx + 4], eax
        mov dword ptr [ebx + 0ch], ebp
        push ebp
        push ecx
        push eax
        pop eax
        pop ecx
        pop ebp
        pop ecx
        pop ebx
        ret 4
    }
}
} // namespace bsp
