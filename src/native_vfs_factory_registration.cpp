#include "bsp/native_vfs_factory_registration.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
template<class T> volatile T& field(void* p, std::uint32_t offset) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}

struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};

// Consumed STL library contract BDECE0: its throw-site name does not imply
// unconditional failure. Existing4CE780 has a DIFFERENT1FFFFFFF bound, so only
// its established message/payload transport is reused here.
void grow_factory_list_count(void* list, std::uint32_t increment) {
    const std::uint32_t captured = field<std::uint32_t>(list, 8);
    if (std::uint32_t{0x3fffffff} - captured < increment) {
        NativeLegacySboStringStorage temporary;
        temporary.capacity_18 = 15;
        temporary.length_14 = 0;
        temporary.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(
            temporary, "list<T> too long", 16);
        const CompletedMessage completed{temporary}; // native state0 now armed
        throw NativeAliasListLengthError{temporary};
    }
    field<std::uint32_t>(list, 8) = captured + increment;
}
} // namespace

void* __stdcall allocate_native_vfs_factory_node_00bdab20(
    void* next, void* previous, void* const* factory_slot) {
    void* const node = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x0c, 0x0c});
    if (node) field<void*>(node, 0) = next;
    if (void* const destination = at(node, 4))
        *static_cast<void* volatile*>(destination) = previous;
    if (void* const destination = at(node, 8))
        *static_cast<void* volatile*>(destination) =
            *static_cast<void* const volatile*>(factory_slot);
    return node;
}

void __fastcall register_native_vfs_provider_factory_00be0660(
    void* manager, void*, void* factory) {
    void* const sentinel = field<void*>(manager, 0x34);
    void* const list = at(manager, 0x30);
    void* const previous = field<void*>(sentinel, 4);
    void* const node = allocate_native_vfs_factory_node_00bdab20(
        sentinel, previous, &factory);
    grow_factory_list_count(list, 1);
    field<void*>(sentinel, 4) = node;
    void* const current_previous = field<void*>(node, 4);
    field<void*>(current_previous, 0) = node;
}

} // namespace bsp
