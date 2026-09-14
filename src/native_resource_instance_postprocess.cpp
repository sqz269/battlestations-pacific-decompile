#include "bsp/native_resource_instance_postprocess.hpp"
#include <Windows.h>
#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource postprocessing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> T read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
Word pair_count(const void* end, const void* begin) noexcept {
    const Word bytes = reinterpret_cast<Word>(end) - reinterpret_cast<Word>(begin);
    return (bytes >> 3u) | ((bytes & 0x80000000u) ? 0xe0000000u : 0u);
}
} // namespace

void __fastcall assign_native_node_animator_00b6ee80(void* node,
    NativeResourceAnimatorLifetime& lifetime, void* animator) {
    void* const old = read<void*>(node, 0x130);
    if (old == animator) return;
    *static_cast<void* volatile*>(at(node, 0x130)) = animator;
    if (animator) InterlockedIncrement(static_cast<volatile LONG*>(at(animator, 4)));
    if (old && InterlockedDecrement(static_cast<volatile LONG*>(at(old, 4))) == 0) {
        const Word target = lifetime.table(read<Word>(old))[0];
        lifetime.destroy(target, old);
    }
}

__declspec(naked) std::uint8_t __fastcall native_node_has_ancestor_00b75f00(
    const void*, const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 30h]
        test eax, eax
        jz absent
    next:
        cmp eax, edx
        jz found
        mov eax, dword ptr [eax + 30h]
        test eax, eax
        jnz next
    absent:
        xor al, al
        ret
    found:
        mov al, 1
        ret
    }
}

std::uint32_t __fastcall native_resource_instance_item_count_00b87200(
    const void* instance) noexcept {
    void* const begin = read<void*>(instance, 0x24);
    return begin ? pair_count(read<void*>(instance, 0x28), begin) : 0u;
}

void* __fastcall native_resource_instance_item_at_00b87260(
    const void* instance, void*, std::uint32_t index) {
    void* begin = read<void*>(instance, 0x24);
    if (!begin || index >= pair_count(read<void*>(instance, 0x28), begin)) {
        _invalid_parameter_noinfo();
        begin = read<void*>(instance, 0x24);
    }
    return read<void*>(begin, index * 8u + 4u);
}
} // namespace bsp
