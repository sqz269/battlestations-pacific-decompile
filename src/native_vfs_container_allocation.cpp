#include "bsp/native_vfs_container_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);

// Actual shared BF681B service, not a separate heap or test allocator. Keep the
// one-DWORD cdecl call shape used by every original leaf.
void* __cdecl allocate_container_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
}

} // namespace

// Preserve each native address test independently. In particular EAX==0 does
// not suppress the following [EAX+4] access. Native EAX remains the result and
// ECX is only a postallocation scratch register; no caller input is consumed.
__declspec(naked) void* __cdecl allocate_native_list_head_00bda960() {
    __asm {
        push 0Ch
        call allocate_container_bytes
        add esp, 4
        test eax, eax
        jz link_four
        mov dword ptr [eax], eax
    link_four:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz done
        mov dword ptr [ecx], eax
    done:
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_list_head_00bda980() {
    __asm {
        push 28h
        call allocate_container_bytes
        add esp, 4
        test eax, eax
        jz link_four
        mov dword ptr [eax], eax
    link_four:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz done
        mov dword ptr [ecx], eax
    done:
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_tree_node_00bdabf0() {
    __asm {
        push 24h
        call allocate_container_bytes
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
        mov byte ptr [eax + 20h], 1
        mov byte ptr [eax + 21h], 0
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_tree_node_00bdaba0() {
    __asm {
        push 20h
        call allocate_container_bytes
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
        mov byte ptr [eax + 1Ch], 1
        mov byte ptr [eax + 1Dh], 0
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_tree_node_004c26b0() {
    __asm {
        push 18h
        call allocate_container_bytes
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
        mov byte ptr [eax + 14h], 1
        mov byte ptr [eax + 15h], 0
        ret
    }
}

__declspec(naked) void* __cdecl allocate_native_list_head_007f82f0() {
    __asm {
        push 0Ch
        call allocate_container_bytes
        add esp, 4
        test eax, eax
        jz link_four
        mov dword ptr [eax], eax
    link_four:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz done
        mov dword ptr [ecx], eax
    done:
        ret
    }
}

} // namespace bsp
