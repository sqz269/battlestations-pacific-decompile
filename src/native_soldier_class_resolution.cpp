#include "bsp/native_soldier_class_resolution.hpp"
#include "bsp/native_damageable_section.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SoldierClass reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
void* at(const void* value, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(value) + offset);
}
template<class T> volatile T& field(const void* value, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(value, offset));
}
void* allocate_blank_1c_node() {
    void* const node = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x1c, 0x1c});
    if (node != nullptr) field<Word>(node) = 0;
    if (at(node, 4) != nullptr) field<Word>(node, 4) = 0;
    if (at(node, 8) != nullptr) field<Word>(node, 8) = 0;
    field<unsigned char>(node, 0x18) = 1;
    field<unsigned char>(node, 0x19) = 0;
    return node;
}
void publish_sentinel(void* header, void* node) noexcept {
    field<void*>(header, 4) = node;
    field<unsigned char>(node, 0x19) = 1;
    node = field<void*>(header, 4);
    field<void*>(node, 4) = node;
    node = field<void*>(header, 4);
    field<void*>(node) = node;
    node = field<void*>(header, 4);
    field<void*>(node, 8) = node;
    field<Word>(header, 8) = 0;
}
} // namespace

void* allocate_native_string_value_tree_node_00443e20() {
    return allocate_blank_1c_node();
}
void* allocate_native_soldier_registry_node_004afed0() {
    return allocate_blank_1c_node();
}

void* construct_native_named_class_base_00489e60(void* actual, Word actual_vtable) {
    field<Word>(actual) = actual_vtable;
    field<Word>(actual, 4) = 0;
    field<Word>(actual, 8) = 0;
    field<unsigned char>(actual, 0x0c) = 0;
    field<Word>(actual, 0x10) = 0;
    field<Word>(actual, 0x14) = 0;
    return actual;
}

void destroy_native_named_class_base_00489e80(void* actual,
    const NativeSoldierClassConstructionAccess& access) {
    field<Word>(actual) = access.actual_base_vtable_00ce660c;
    int state = 1;
    try {
        release_native_ref_counted_handle_0041de40(at(actual, 0x10));
        state = 0;
        release_native_ref_counted_handle_0041de40(at(actual, 0x14));
        state = -1;
        destroy_native_string_header_0041dd20(at(actual, 4), access.strings);
    } catch (...) {
        // D8A4F8: state1->0 releases+14; state0->-1 destroys name+4.
        try {
            if (state >= 1)
                release_native_ref_counted_handle_0041de40(at(actual, 0x14));
            if (state >= 0)
                destroy_native_string_header_0041dd20(at(actual, 4), access.strings);
        } catch (...) { std::terminate(); }
        throw;
    }
}

void unwind_native_soldier_registry_004af950(void* actual,
    const NativeSoldierClassConstructionAccess& access) noexcept {
    access.actual_registry_publication_00e187f0 = nullptr;
    field<Word>(actual) = access.actual_singleton_base_vtable_00ce3818;
}

void* construct_native_soldier_registry_004b11a0(void* actual,
    const NativeSoldierClassConstructionAccess& access) {
    field<Word>(actual) = access.actual_registry_vtable_00ce7160;
    void* node;
    try {
        node = allocate_native_soldier_registry_node_004afed0();
    } catch (...) {
        unwind_native_soldier_registry_004af950(actual, access);
        throw;
    }
    publish_sentinel(at(actual, 4), node);
    return actual;
}

void* construct_native_soldier_class_004b12a0(void* actual,
    const NativeSoldierClassConstructionAccess& access) {
    construct_native_named_class_base_00489e60(actual, access.actual_base_vtable_00ce660c);
    field<Word>(actual) = access.actual_soldier_vtable_00ce7150;
    void* node;
    try {
        node = allocate_native_string_value_tree_node_00443e20();
    } catch (...) {
        try { destroy_native_named_class_base_00489e80(actual, access); }
        catch (...) { std::terminate(); }
        throw;
    }
    publish_sentinel(at(actual, 0x28), node);
    return actual;
}

bool native_soldier_class_true_004af520() noexcept { return true; }
} // namespace bsp
