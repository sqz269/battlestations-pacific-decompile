#include "bsp/native_fileblock_gate_list.hpp"

#include "bsp/native_alias_count_growth.hpp"

namespace bsp {
namespace {
void* at(void* value, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(value) + offset);
}

template<class T> volatile T& field(void* value, std::uint32_t offset) noexcept {
    return *static_cast<volatile T*>(at(value, offset));
}

struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
} // namespace

void* __stdcall allocate_native_fileblock_gate_node_007f8390(
    void* next, void* previous, const std::uint8_t* gate_slot) {
    void* const node = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x0c, 0x0c});
    if (node) field<void*>(node, 0) = next;
    if (void* const destination = at(node, 4))
        *static_cast<void* volatile*>(destination) = previous;
    if (void* const destination = at(node, 8))
        *static_cast<volatile std::uint8_t*>(destination) =
            *static_cast<const volatile std::uint8_t*>(gate_slot);
    return node;
}

std::uint32_t grow_native_fileblock_gate_count_007fa3a0(
    void* actual_list, std::uint32_t increment) {
    const std::uint32_t captured = field<std::uint32_t>(actual_list, 8);
    if (std::uint32_t{0xffffffff} - captured < increment) {
        NativeLegacySboStringStorage temporary;
        temporary.capacity_18 = 15;
        temporary.length_14 = 0;
        temporary.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(
            temporary, "list<T> too long", 16);
        // Native state0 is armed only after assignment; C8F9A0 destroys the
        // completed message if the owner constructor or native throw unwinds.
        const CompletedMessage completed{temporary};
        throw NativeAliasListLengthError{temporary};
    }
    const std::uint32_t updated = captured + increment;
    field<std::uint32_t>(actual_list, 8) = updated;
    return updated;
}

void* erase_native_fileblock_gate_iterator_00bdaf40(void* actual_list,
    void* output, void* iterator_owner, void* node,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!iterator_owner) callbacks.invalid_parameter(callbacks.context);
    if (node == field<void*>(iterator_owner, 4))
        callbacks.invalid_parameter(callbacks.context);
    const bool is_head = node == field<void*>(actual_list, 4);
    void* const successor = field<void*>(node, 0);
    if (!is_head) {
        void* const previous = field<void*>(node, 4);
        field<void*>(previous, 0) = successor;
        // BDAF76/78 reload both node links after the first neighbor store.
        void* const current_next = field<void*>(node, 0);
        void* const current_previous = field<void*>(node, 4);
        field<void*>(current_next, 4) = current_previous;
        singleton_lifetime_free(node);
        // Decoded BDAF84..8A is outside current Ghidra body membership.
        field<std::uint32_t>(actual_list, 8) =
            field<std::uint32_t>(actual_list, 8) - 1u;
    }
    field<void*>(output, 4) = successor;
    field<void*>(output, 0) = iterator_owner;
    return output;
}

} // namespace bsp
