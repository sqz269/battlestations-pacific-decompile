#include "bsp/native_unit_owner_entity.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_unit_owner_list_heads.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

template<class T>
volatile T& at(void* object, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(object) + offset);
}

void* bytes_at(void* object, std::size_t offset) noexcept {
    return static_cast<std::byte*>(object) + offset;
}

void* wrapped_at(void* object, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + offset);
}

struct CompletedCountMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedCountMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};

void grow_owner_count(void* actual_list, std::uint32_t increment, std::uint32_t bound) {
    const std::uint32_t captured = at<std::uint32_t>(actual_list, 8);
    if (bound - captured < increment) {
        NativeLegacySboStringStorage message;
        message.capacity_18 = 15;
        message.length_14 = 0;
        message.buffer_04.inline_bytes[0] = '\0';
        native_legacy_sbo_string_assign_counted_00408720(message, "list<T> too long", 16);
        // C89F50/C89F90 arm only after assignment, then destroy the completed
        // message on legacy owner construction or throw unwind.
        const CompletedCountMessage completed{message};
        throw NativeAliasListLengthError{message};
    }
    at<std::uint32_t>(actual_list, 8) = captured + increment;
}
} // namespace

void construct_native_unit_owner_first_list_0077ef27(void* actual_root_after_recon) {
    void* const root = actual_root_after_recon;
    void* const recon_owner = bytes_at(root, 0x1e4);
    at<void*>(root, 0x214) = recon_owner;
    at<std::uint32_t>(root, 0x218) = 0;
    at<void*>(root, 0x248) = recon_owner;
    at<std::uint32_t>(root, 0x24c) = 1;
    at<void*>(root, 0x27c) = recon_owner;
    at<std::uint32_t>(root, 0x280) = 2;
    at<std::uint32_t>(root, 0x1e4) = 0x00d03e54u;
    at<std::uint32_t>(root, 0x00) = 0x00d03e80u;
    at<std::uint32_t>(root, 0x10) = 0x00d03e68u;
    at<std::uint32_t>(root, 0x24) = 0x00d03e60u;
    at<std::uint32_t>(root, 0x170) = 0x00d03e5cu;
    at<std::uint32_t>(root, 0x284) = 0;
    at<std::uint8_t>(root, 0x288) = 0;
    at<std::uint32_t>(root, 0x28c) = 0;
    at<std::uint32_t>(root, 0x294) = 0;
    at<void*>(root, 0x29c) = buy_native_unit_owner_list_head_0077b960();
    at<std::uint32_t>(root, 0x2a0) = 0;
}

void construct_native_unit_owner_second_list_and_fields_0077efa9(
    void* actual_root_after_first_list,
    const volatile std::uint32_t& actual_initial_word_00ce4adc) {
    void* const root = actual_root_after_first_list;
    at<void*>(root, 0x2a8) = buy_native_unit_owner_list_head_0077b940();
    at<std::uint32_t>(root, 0x2ac) = 0;
    at<std::uint32_t>(root, 0x2b4) = 0;
    at<std::uint32_t>(root, 0x2b8) = 0;
    at<std::uint32_t>(root, 0x2bc) = 0;
    at<std::uint32_t>(root, 0x2c4) = 0;
    at<std::uint8_t>(root, 0x2c0) = 1;
    at<std::uint32_t>(root, 0x2b0) = 0x00cf938cu;
    at<std::uint16_t>(root, 0x2c8) = 0;
    // Original MOVSS is a bit transfer, including non-finite bit patterns.
    const std::uint32_t initial_word = actual_initial_word_00ce4adc;
    at<std::uint32_t>(root, 0x2e8) = 0xffffffffu;
    at<std::uint32_t>(root, 0x2ec) = 0xffffffffu;
    at<std::uint32_t>(root, 0x308) = 0;
    at<std::uint32_t>(root, 0x30c) = 0xffffffffu;
    at<std::uint32_t>(root, 0xc4) = 2;
    at<std::uint32_t>(root, 0x2d8) = 0xffffffffu;
    at<std::uint32_t>(root, 0x2e4) = initial_word;
    at<std::uint32_t>(root, 0x2cc) = 0x13;
    at<std::uint32_t>(root, 0x2d0) = 2;
    at<std::uint32_t>(root, 0x2f8) = 0;
    at<std::uint32_t>(root, 0x2f4) = 0;
    at<std::uint8_t>(root, 0x2f0) = 0;
    at<std::uint32_t>(root, 0x2fc) = 0;
    at<std::uint32_t>(root, 0x300) = initial_word;
}

void tidy_native_unit_owner_peer_list_0077b980(void* actual_list_header) noexcept {
    void* sentinel = at<void*>(actual_list_header, 4);
    void* node = at<void*>(sentinel, 0);
    at<void*>(sentinel, 0) = sentinel;
    sentinel = at<void*>(actual_list_header, 4);
    at<void*>(sentinel, 4) = sentinel;
    const bool has_nodes = node != at<void*>(actual_list_header, 4);
    at<std::uint32_t>(actual_list_header, 8) = 0;
    if (has_nodes) {
        do {
            void* const next = at<void*>(node, 0);
            singleton_lifetime_free(node);
            node = next;
        } while (node != at<void*>(actual_list_header, 4));
    }
    singleton_lifetime_free(at<void*>(actual_list_header, 4));
    at<void*>(actual_list_header, 4) = nullptr;
}

void clear_native_unit_owner_holder_list_0077d930(void* actual_list_header) noexcept {
    void* sentinel = at<void*>(actual_list_header, 4);
    void* node = at<void*>(sentinel, 0);
    at<void*>(sentinel, 0) = sentinel;
    sentinel = at<void*>(actual_list_header, 4);
    at<void*>(sentinel, 4) = sentinel;
    const bool has_nodes = node != at<void*>(actual_list_header, 4);
    at<std::uint32_t>(actual_list_header, 8) = 0;
    if (has_nodes) {
        do {
            void* const payload = at<void*>(node, 0x14);
            void* const next = at<void*>(node, 0);
            if (payload != nullptr) {
                singleton_lifetime_free(payload);
                at<void*>(node, 0x14) = nullptr;
            }
            singleton_lifetime_free(node);
            node = next;
        } while (node != at<void*>(actual_list_header, 4));
    }
}

void destroy_native_unit_owner_lists_0077e42c(void* actual_root_after_observer) noexcept {
    void* const root = actual_root_after_observer;
    clear_native_unit_owner_holder_list_0077d930(bytes_at(root, 0x2a4));
    singleton_lifetime_free(at<void*>(root, 0x2a8));
    at<void*>(root, 0x2a8) = nullptr;
    tidy_native_unit_owner_peer_list_0077b980(bytes_at(root, 0x298));
}

void* __stdcall buy_native_unit_owner_holder_node_0077ba60(
    void* next, void* previous, const volatile std::uint32_t* actual_record) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, 0x18});
    if (node != nullptr) at<void*>(node, 0) = next;
    if (void* const destination = wrapped_at(node, 4))
        *static_cast<void* volatile*>(destination) = previous;
    if (void* const destination = wrapped_at(node, 8)) {
        auto* const output = static_cast<volatile std::uint32_t*>(destination);
        output[0] = actual_record[0];
        output[1] = actual_record[1];
        output[2] = actual_record[2];
        output[3] = actual_record[3];
    }
    return node;
}

void* __stdcall buy_native_unit_owner_peer_node_0077bb00(
    void* next, void* previous, const volatile std::uint32_t* actual_peer_slot) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x0c, 0x0c});
    if (node != nullptr) at<void*>(node, 0) = next;
    if (void* const destination = wrapped_at(node, 4))
        *static_cast<void* volatile*>(destination) = previous;
    if (void* const destination = wrapped_at(node, 8))
        *static_cast<volatile std::uint32_t*>(destination) = *actual_peer_slot;
    return node;
}

void grow_native_unit_owner_holder_count_0077dd00(void* actual_list, std::uint32_t increment) {
    grow_owner_count(actual_list, increment, 0x0fffffffu);
}

void grow_native_unit_owner_peer_count_0077de40(void* actual_list, std::uint32_t increment) {
    grow_owner_count(actual_list, increment, 0x3fffffffu);
}

void release_native_unit_owner_holder_scratch_0077b030(void* actual_record) noexcept {
    void* const payload = at<void*>(actual_record, 0x0c);
    if (payload != nullptr) {
        singleton_lifetime_free(payload);
        at<void*>(actual_record, 0x0c) = nullptr;
    }
}

void append_native_unit_owner_peer_0077eb50(
    void* actual_root,
    const volatile std::uint32_t& actual_peer_argument_slot,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190) {
    void* const scratch = actual_holder_scratch_preimage;
    at<std::uint32_t>(scratch, 0) = actual_peer_argument_slot;
    at<std::uint32_t>(scratch, 4) = 0xfff0bdc1u;
    at<std::uint16_t>(scratch, 8) = 0;
    at<void*>(scratch, 0x0c) = nullptr;
    void* const holder_head = at<void*>(actual_root, 0x2a8);
    void* const holder_previous = at<void*>(holder_head, 4);
    void* peer_head;
    void* peer_previous;
    try {
        void* const holder = buy_native_unit_owner_holder_node_0077ba60(
            holder_head, holder_previous, static_cast<const volatile std::uint32_t*>(scratch));
        grow_native_unit_owner_holder_count_0077dd00(bytes_at(actual_root, 0x2a4), 1);
        at<void*>(holder_head, 4) = holder;
        void* const current_holder_previous = at<void*>(holder, 4);
        at<void*>(current_holder_previous, 0) = holder;
        peer_head = at<void*>(actual_root, 0x29c);
        peer_previous = at<void*>(peer_head, 4);
    } catch (...) {
        // The actual state0 map owns only scratch+0C, not either list node.
        release_native_unit_owner_holder_scratch_0077b030(scratch);
        throw;
    }
    // Original state -1 is stored before this second allocation.
    void* const peer = buy_native_unit_owner_peer_node_0077bb00(
        peer_head, peer_previous, &actual_peer_argument_slot);
    grow_native_unit_owner_peer_count_0077de40(bytes_at(actual_root, 0x298), 1);
    at<void*>(peer_head, 4) = peer;
    void* const current_peer_previous = at<void*>(peer, 4);
    at<void*>(current_peer_previous, 0) = peer;
    actual_dirty_byte_00f87190 = 1;
}

void finish_native_unit_owner_peer_tail_0077f054(
    void* actual_root_after_physical_fields,
    void* volatile& actual_game_publication_00e188a8,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190) {
    void* const game = actual_game_publication_00e188a8;
    if (at<std::uint32_t>(game, 0x1fe4) == 2 && actual_network_byte_00e0af20 == 0) {
        volatile std::uint32_t peer_argument = 0;
        append_native_unit_owner_peer_0077eb50(actual_root_after_physical_fields,
            peer_argument, actual_holder_scratch_preimage, actual_dirty_byte_00f87190);
    }
}

} // namespace bsp
