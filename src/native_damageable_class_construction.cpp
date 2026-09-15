#include "bsp/native_damageable_class_construction.hpp"
#include "bsp/native_damageable_class_binding.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <exception>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable class construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}
using DeleteRecord = void (__thiscall*)(void*, Word);
using ReleaseOwner = void (__thiscall*)(void*);

void unwind_constructor(void* actual,
    const NativeDamageableClassConstructionAccess& access) {
    // DC8CE4: 5->4->3->2->1->0->-1. The tree allocation never completed:
    // there is NO tree destructor, free(actual), or header rollback here.
    destroy_native_string_header_0041dd20(at(actual, 0x58), access.strings);
    destroy_native_string_header_0041dd20(at(actual, 0x38), access.strings);
    destroy_native_class_point_array_0081b0a0(at(actual, 0x28));
    destroy_native_damageable_effect_vector_00879830(at(actual, 0x18));
    destroy_native_damageable_owner_vector_0087c260(at(actual, 8));
    // BD30F0 is exactly one vptr store and RET. Its existing native entrypoint
    // embeds the original VA; this borrowed-storage interface stores the
    // caller's actual equivalent table identity, as on entry above.
    field<Word>(actual) = access.actual_ref_counted_vtable_00ceb130;
}
} // namespace

void* allocate_native_damageable_tree_node_00877fa0() {
    void* const node = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x58, 0x58});
    // Native address guards are separate, including DWORD address wrap.
    if (node != nullptr) field<Word>(node) = 0;
    if (at(node, 4) != nullptr) field<Word>(node, 4) = 0;
    if (at(node, 8) != nullptr) field<Word>(node, 8) = 0;
    field<unsigned char>(node, 0x54) = 1;
    field<unsigned char>(node, 0x55) = 0;
    return node;
}

void release_native_damageable_owner_rows_0087ab30(void* first, const void* end) {
    while (first != end) {
        void* const owner = field<void*>(first, 0x0c);
        if (owner != nullptr) {
            if (_InterlockedDecrement(&field<long>(owner, 4)) == 0) {
                void* const table = field<void*>(owner);
                field<ReleaseOwner>(table)(owner);
            }
            field<void*>(first, 0x0c) = nullptr;
        }
        first = at(first, 0x10);
    }
}

void destroy_native_damageable_owner_vector_0087c260(void* header) {
    void* const first = field<void*>(header, 4);
    if (first != nullptr) {
        release_native_damageable_owner_rows_0087ab30(first, field<void*>(header, 8));
        singleton_lifetime_free(field<void*>(header, 4));
    }
    field<void*>(header, 4) = nullptr;
    field<void*>(header, 8) = nullptr;
    field<void*>(header, 0x0c) = nullptr;
}

void destroy_native_damageable_effect_vector_00879240(void* header) {
    void* current = field<void*>(header, 4);
    if (current != nullptr) {
        const void* const end = field<void*>(header, 8);
        while (current != end) {
            void* const table = field<void*>(current);
            field<DeleteRecord>(table)(current, 0);
            current = at(current, 0x30);
        }
        singleton_lifetime_free(field<void*>(header, 4));
    }
    field<void*>(header, 4) = nullptr;
    field<void*>(header, 8) = nullptr;
    field<void*>(header, 0x0c) = nullptr;
}

void destroy_native_damageable_effect_vector_00879830(void* header) {
    destroy_native_damageable_effect_vector_00879240(header);
}

void destroy_native_class_point_array_0081b0a0(void* header) {
    if (field<std::int32_t>(header, 8) < 0)
        reserve_native_class_point_array_0074d190(header, 0);
    while (field<std::int32_t>(header, 4) > 0)
        field<Word>(header, 4) = field<Word>(header, 4) - 1u;
    void* const data = field<void*>(header);
    field<Word>(header, 4) = 0;
    singleton_lifetime_free(data);
}

void* construct_native_damageable_class_0087c640(void* actual,
    const NativeDamageableClassConstructionAccess& access) {
    field<Word>(actual) = access.actual_ref_counted_vtable_00ceb130;
    field<Word>(actual, 4) = 1;
    field<Word>(actual) = access.actual_damageable_vtable_00d0e13c;
    field<Word>(actual, 0x0c) = 0;
    field<Word>(actual, 0x10) = 0;
    field<Word>(actual, 0x14) = 0;
    field<Word>(actual, 0x1c) = 0;
    field<Word>(actual, 0x20) = 0;
    field<Word>(actual, 0x24) = 0;
    field<Word>(actual, 0x28) = 0;
    field<Word>(actual, 0x2c) = 0;
    field<Word>(actual, 0x30) = 0;
    field<Word>(actual, 0x38) = 0;
    field<Word>(actual, 0x3c) = 0;
    field<Word>(actual, 0x40) = 0xffffffffu;
    field<unsigned char>(actual, 0x44) = 0;
    field<Word>(actual, 0x50) = 0;
    field<Word>(actual, 0x58) = 0;
    field<Word>(actual, 0x5c) = 0;
    void* node;
    try {
        node = allocate_native_damageable_tree_node_00877fa0();
    } catch (...) {
        try { unwind_constructor(actual, access); }
        catch (...) { std::terminate(); }
        throw;
    }
    field<void*>(actual, 0x64) = node;
    field<unsigned char>(node, 0x55) = 1;
    node = field<void*>(actual, 0x64);
    field<void*>(node, 4) = node;
    node = field<void*>(actual, 0x64);
    field<void*>(node) = node;
    node = field<void*>(actual, 0x64);
    field<void*>(node, 8) = node;
    field<Word>(actual, 0x68) = 0;
    return actual;
}

} // namespace bsp
