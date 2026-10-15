#include "bsp/native_grid_list_node.hpp"

#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {

__declspec(noinline) void* __cdecl allocate_grid_node_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

} // namespace

__declspec(naked) void* __stdcall buy_native_grid_list_node_0041c0b0(
    std::uint32_t, std::uint32_t, const void*) {
    __asm {
        push 0ch
        call allocate_grid_node_bytes
        add esp, 4
        test eax, eax
        jz next_stored
        mov ecx, dword ptr [esp + 4]
        mov dword ptr [eax], ecx
    next_stored:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz previous_stored
        mov edx, dword ptr [esp + 8]
        mov dword ptr [ecx], edx
    previous_stored:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz value_stored
        mov edx, dword ptr [esp + 0ch]
        mov edx, dword ptr [edx]
        mov dword ptr [ecx], edx
    value_stored:
        ret 0ch
    }
}

} // namespace bsp
