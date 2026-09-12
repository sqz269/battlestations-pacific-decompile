#include "bsp/native_filestore_container_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
void* __cdecl allocate_file_store_node_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
} // namespace

__declspec(naked) void* __cdecl allocate_native_file_store_node_00be55e0() {
    __asm {
        push 1Ch
        call allocate_file_store_node_bytes
        add esp, 4
        test eax, eax
        jz link_four
        mov dword ptr [eax], 0
    link_four:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz link_eight
        mov dword ptr [ecx], 0
    link_eight:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz flags
        mov dword ptr [ecx], 0
    flags:
        mov byte ptr [eax + 18h], 1
        mov byte ptr [eax + 19h], 0
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_file_store_node_00be5690() {
    __asm {
        push 1Ch
        call allocate_file_store_node_bytes
        add esp, 4
        test eax, eax
        jz link_four
        mov dword ptr [eax], 0
    link_four:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz link_eight
        mov dword ptr [ecx], 0
    link_eight:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz flags
        mov dword ptr [ecx], 0
    flags:
        mov byte ptr [eax + 18h], 1
        mov byte ptr [eax + 19h], 0
        ret
    }
}

} // namespace bsp
