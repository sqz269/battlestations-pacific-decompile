#include "bsp/native_unit_owner_entity.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_entity_intrusive_unlink.hpp"
#include "bsp/native_game_array_lifetime.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_unit_owner_list_heads.hpp"
#include "bsp/native_unit_list_remove.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/observer_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>
#include <new>

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

struct ReconConstructorUnwind {
    void* record;
    NativeObserverLifetime& observers;
    bool armed = true;
    ~ReconConstructorUnwind() noexcept(false) {
        if (armed) {
            NativeGameArrayLifetimeStage stage;
            stage.owner = record;
            destroy_native_game_participant_observer_004b7ef0(
                bytes_at(record, 0x14), observers, stage);
        }
    }
};

NativeWeakOwnerView scene_weak_view(void* root) noexcept {
    void* const owner = bytes_at(root, 0x24);
    return {owner, *static_cast<std::uint32_t*>(owner),
        *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(bytes_at(owner, 4))),
        *reinterpret_cast<void**>(bytes_at(owner, 8))};
}

enum class SceneCleanup { observed, callback, weak, first_name, second_name };
struct SceneConstructorUnwind {
    void* root;
    NativeWeakOwnerDomain& weak;
    NativeObserverLifetime& observers;
    NativeStringRawPoolContext& strings;
    SceneCleanup kind;
    bool armed = false;
    ~SceneConstructorUnwind() noexcept(false) {
        if (!armed) return;
        switch (kind) {
        case SceneCleanup::observed:
            observers.destroy_observed_owner_00695760(
                *static_cast<NativeObserverOwnerStorage*>(root));
            break;
        case SceneCleanup::callback:
            observers.destroy_callback_owner_00695870(
                *static_cast<NativeObserverOwnerStorage*>(bytes_at(root, 0x10)));
            break;
        case SceneCleanup::weak:
            weak.destroy_00925540(scene_weak_view(root));
            break;
        case SceneCleanup::first_name:
            destroy_native_string_header_0041dd20(bytes_at(root, 0x154), strings);
            break;
        case SceneCleanup::second_name:
            destroy_native_string_header_0041dd20(bytes_at(root, 0x15c), strings);
            break;
        }
    }
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

void* construct_native_unit_owner_scene_base_00925ce0(
    void* root, NativeWeakOwnerDomain& weak, NativeObserverLifetime& observers,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_one_bits_00d7a24c) {
    SceneConstructorUnwind observed{root, weak, observers, strings, SceneCleanup::observed};
    SceneConstructorUnwind callback{root, weak, observers, strings, SceneCleanup::callback};
    SceneConstructorUnwind weak_owner{root, weak, observers, strings, SceneCleanup::weak};
    SceneConstructorUnwind first_name{root, weak, observers, strings, SceneCleanup::first_name};
    SceneConstructorUnwind second_name{root, weak, observers, strings, SceneCleanup::second_name};
    at<std::uint32_t>(root, 0) = 0x00ceccc8u;
    at<std::uint32_t>(root, 4) = 0;
    at<std::uint32_t>(root, 8) = 0;
    at<std::uint32_t>(root, 0xc) = 0;
    at<std::uint32_t>(root, 0x10) = 0x00ce3cd4u;
    at<std::uint32_t>(root, 0x14) = 0;
    at<std::uint32_t>(root, 0x18) = 0;
    at<std::uint32_t>(root, 0x1c) = 0;
    observed.armed = true; // Native state0 at 925D23.
    at<std::uint8_t>(root, 0x20) = 0;
    callback.armed = true; // Native state1 precedes the actual weak constructor.
    weak.construct_00925490(scene_weak_view(root)); // 925D34
    weak_owner.armed = true;
    const std::uint32_t first_one = actual_one_bits_00d7a24c;
    at<std::uint32_t>(root, 0) = 0x00d19120u;
    at<std::uint32_t>(root, 0x10) = 0x00d19104u;
    at<std::uint32_t>(root, 0x24) = 0x00d190fcu;
    at<std::uint32_t>(root, 0x4c) = 0;
    at<std::uint32_t>(root, 0x48) = 0;
    at<std::uint32_t>(root, 0x50) = 0;
    const std::uint32_t first_matrix[16] = {
        first_one, 0, 0, 0, 0, first_one, 0, 0, 0, 0, first_one, 0, 0, 0, 0, first_one};
    copy_native_camera_matrix_004134f0(bytes_at(root, 0x74), nullptr, first_matrix); // 925DCA
    at<std::uint32_t>(root, 0x154) = 0;
    at<std::uint32_t>(root, 0x158) = 0;
    first_name.armed = true;
    at<std::uint32_t>(root, 0x15c) = 0;
    at<std::uint32_t>(root, 0x160) = 0;
    const std::uint32_t second_one = actual_one_bits_00d7a24c;
    second_name.armed = true; // Native state4 at 925DF1.
    at<std::uint32_t>(root, 0xc0) = 0;
    at<std::uint32_t>(root, 0xc4) = 0;
    at<std::uint8_t>(root, 0xbd) = 0;
    at<std::uint8_t>(root, 0x60) = 0;
    at<std::uint8_t>(root, 0x5f) = 0;
    at<std::uint8_t>(root, 0x5c) = 0;
    at<std::uint8_t>(root, 0x5e) = 0;
    at<std::uint8_t>(root, 0x5d) = 0;
    at<std::uint32_t>(root, 0x6c) = 0;
    at<std::uint32_t>(root, 0x70) = 0;
    at<std::uint32_t>(root, 0x54) = 2;
    at<std::uint32_t>(root, 0x58) = 0xffffffffu;
    at<std::uint8_t>(root, 0xbc) = 0;
    at<std::uint32_t>(root, 0x150) = 0;
    const std::uint32_t second_matrix[16] = {
        second_one, 0, 0, 0, 0, second_one, 0, 0, 0, 0, second_one, 0, 0, 0, 0, second_one};
    copy_native_camera_matrix_004134f0(bytes_at(root, 0x74), nullptr, second_matrix); // 925EA0
    resize_native_string_header_0041dd40(bytes_at(root, 0x154), strings, 0, false); // 925EA9
    void* const data = at<void*>(root, 0x158);
    if (data != nullptr) {
        const std::uint32_t length = at<std::uint32_t>(root, 0x154);
        std::memcpy(data, nullptr, length); // 925EBA; Native fault/CRT domain qualified.
    }
    at<std::uint32_t>(root, 0x16c) = 0;
    at<std::uint32_t>(root, 0x168) = 0;
    at<std::uint8_t>(root, 0x61) = 0;
    at<std::uint8_t>(root, 0x62) = 0;
    at<std::uint8_t>(root, 0x68) = 0;
    at<std::uint32_t>(root, 0x64) = 0;
    second_name.armed = first_name.armed = weak_owner.armed = callback.armed = observed.armed = false;
    return root;
}

void restore_native_unit_owner_scene_destructor_profiles_009257a7(void* root) noexcept {
    at<std::uint32_t>(root, 0) = 0x00d19120u;
    at<std::uint32_t>(root, 0x10) = 0x00d19104u;
    at<std::uint32_t>(root, 0x24) = 0x00d190fcu;
}

void finish_native_unit_owner_scene_retirement_009257f5(
    void* root, NativeWeakOwnerDomain& weak, NativeObserverLifetime& observers,
    NativeStringRawPoolContext& strings) {
    // The admitted entry is the continuation with Native state4 already
    // owning all five subobjects, after actual life-ref/child retirement.
    SceneConstructorUnwind observed{root, weak, observers, strings, SceneCleanup::observed, true};
    SceneConstructorUnwind callback{root, weak, observers, strings, SceneCleanup::callback, true};
    SceneConstructorUnwind weak_owner{root, weak, observers, strings, SceneCleanup::weak, true};
    SceneConstructorUnwind first_name{root, weak, observers, strings, SceneCleanup::first_name, true};
    SceneConstructorUnwind second_name{root, weak, observers, strings, SceneCleanup::second_name, true};
    void* const world = at<void*>(root, 0x30);
    unlink_native_entity_world_chain_00903f30(at<void*>(world, 4), 0, root); // 9257FC
    void* const parent = at<void*>(root, 0x3c);
    if (parent != nullptr) {
        unlink_native_entity_sibling_chain_00924710(bytes_at(parent, 0x48), 0, root); // 92580C
        at<void*>(root, 0x3c) = nullptr;
    } else {
        void* const current_world = at<void*>(root, 0x30);
        unlink_native_entity_sibling_chain_00924710(at<void*>(current_world, 8), 0, root); // 925820
        if (at<std::uint32_t>(root, 0xb8) != 0) {
            void* const list_world = at<void*>(root, 0x30);
            remove_native_unit_list_payload_004845a0(bytes_at(list_world, 0xc), nullptr, root); // 925835
        }
    }
    // Disarm each member before its normal destruction, exactly the owning
    // state at calls925856/92587E/925891/92589D/9258AC. A failure leaves
    // that member's effects intact and unwinds only the still-owned suffix.
    second_name.armed = false;
    destroy_native_string_header_0041dd20(bytes_at(root, 0x15c), strings);
    first_name.armed = false;
    destroy_native_string_header_0041dd20(bytes_at(root, 0x154), strings);
    weak_owner.armed = false;
    weak.destroy_00925540(scene_weak_view(root)); // 925891
    callback.armed = false;
    observers.destroy_callback_owner_00695870(
        *static_cast<NativeObserverOwnerStorage*>(bytes_at(root, 0x10))); // 92589D
    observed.armed = false;
    observers.destroy_observed_owner_00695760(
        *static_cast<NativeObserverOwnerStorage*>(root)); // 9258AC
}

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

void restore_native_unit_owner_destructor_profiles_0077e3a0(void* actual_root) noexcept {
    at<std::uint32_t>(actual_root, 0x00) = 0x00d03e80u;
    at<std::uint32_t>(actual_root, 0x10) = 0x00d03e68u;
    at<std::uint32_t>(actual_root, 0x24) = 0x00d03e60u;
    at<std::uint32_t>(actual_root, 0x170) = 0x00d03e5cu;
    at<std::uint32_t>(actual_root, 0x1e4) = 0x00d03e54u;
}

void remove_native_unit_owner_global_matches_0077bea0(
    void* actual_global_list,
    const volatile std::uint32_t& actual_target_value_cell) {
    void* const sentinel = at<void*>(actual_global_list, 4);
    const std::uint32_t target = actual_target_value_cell;
    void* node = at<void*>(sentinel, 0);
    for (;;) {
        if (actual_global_list == nullptr) invoke_native_invalid_parameter_00bf6713();
        if (node == sentinel) break;
        if (actual_global_list == nullptr) invoke_native_invalid_parameter_00bf6713();
        if (node == at<void*>(actual_global_list, 4)) invoke_native_invalid_parameter_00bf6713();
        if (at<std::uint32_t>(node, 8) == target) {
            if (node == at<void*>(actual_global_list, 4)) invoke_native_invalid_parameter_00bf6713();
            const bool erase = node != at<void*>(actual_global_list, 4);
            void* const next = at<void*>(node, 0);
            if (erase) {
                void* const previous = at<void*>(node, 4);
                at<void*>(previous, 0) = next;
                void* const current_next = at<void*>(node, 0);
                void* const current_previous = at<void*>(node, 4);
                at<void*>(current_next, 4) = current_previous;
                singleton_lifetime_free(node);
                const std::uint32_t count = at<std::uint32_t>(actual_global_list, 8);
                at<std::uint32_t>(actual_global_list, 8) = count - 1;
            }
            node = next;
        } else {
            if (node == at<void*>(actual_global_list, 4)) invoke_native_invalid_parameter_00bf6713();
            node = at<void*>(node, 0);
        }
    }
}

void withdraw_native_unit_owner_global_0077e409(
    void* actual_root_after_session_release,
    void* actual_global_list_00f87194) {
    volatile std::uint32_t target = reinterpret_cast<std::uintptr_t>(actual_root_after_session_release);
    remove_native_unit_owner_global_matches_0077bea0(actual_global_list_00f87194, target);
}

void* construct_native_unit_owner_recon_record_00805a70(
    void* actual_record, NativeObserverLifetime& actual_observers) {
    void* const callback = bytes_at(actual_record, 0x14);
    at<std::uint32_t>(actual_record, 0) = 0x00d08e74u;
    at<void*>(callback, 4) = nullptr;
    at<std::uint32_t>(callback, 8) = 0;
    at<std::uint32_t>(callback, 0x0c) = 0;
    at<void*>(callback, 0x14) = nullptr;
    at<std::uint8_t>(callback, 0x10) = 1;
    at<std::uint32_t>(callback, 0) = 0x00ce75ccu;
    at<std::uint32_t>(actual_record, 4) = 0;
    at<std::uint32_t>(actual_record, 8) = 0;
    at<std::uint32_t>(actual_record, 0x0c) = 0;
    at<std::uint8_t>(actual_record, 0x10) = 0;
    void* const first = at<void*>(callback, 0x14);
    ReconConstructorUnwind unwind{actual_record, actual_observers};
    if (first != nullptr) {
        actual_observers.unregister_pair_006952a0(
            *static_cast<NativeObserverOwnerStorage*>(first),
            *static_cast<NativeObserverOwnerStorage*>(callback));
        at<void*>(callback, 0x14) = nullptr;
    }
    unwind.armed = false;
    return actual_record;
}

void destroy_native_unit_owner_recon_record_00804300(
    void* actual_record, NativeObserverLifetime& actual_observers) {
    at<std::uint32_t>(actual_record, 0) = 0x00d08e74u;
    NativeGameArrayLifetimeStage stage;
    stage.owner = actual_record;
    destroy_native_game_participant_observer_004b7ef0(
        bytes_at(actual_record, 0x14), actual_observers, stage);
}

void* construct_native_unit_owner_entity_0077eed0(
    void* actual_root, std::uint32_t full_flag, NativeObserverLifetime& observers,
    const volatile std::uint32_t& actual_initial_word_00ce4adc,
    void* volatile& actual_game_publication_00e188a8,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* actual_holder_scratch_preimage,
    volatile std::uint8_t& actual_dirty_byte_00f87190,
    NativeUnitOwnerEntityCalls& calls) {
    calls.game_entity_construct_00928630(actual_root, 1, full_flag); // 0077EEF9
    at<std::uint32_t>(actual_root, 0x1e4) = 0x00d03d98u;          // 0077EF1C
    for (std::uint32_t index = 0; index != 3; ++index) {          // 0077EF22
        construct_native_unit_owner_recon_record_00805a70(
            bytes_at(actual_root, 0x1e8 + index * 0x34), observers);
    }
    construct_native_unit_owner_first_list_0077ef27(actual_root);
    construct_native_unit_owner_second_list_and_fields_0077efa9(
        actual_root, actual_initial_word_00ce4adc);
    finish_native_unit_owner_peer_tail_0077f054(actual_root,
        actual_game_publication_00e188a8, actual_network_byte_00e0af20,
        actual_holder_scratch_preimage, actual_dirty_byte_00f87190);
    return actual_root;                                         // 0077F07C/RET4
}

void destruct_native_unit_owner_entity_0077e380(
    void* actual_root, NativeObserverLifetime& observers,
    const volatile std::uint8_t& actual_network_byte_00e0af20,
    void* volatile& actual_game_publication_00e188a8,
    void* actual_global_list_00f87194, NativeUnitOwnerEntityCalls& calls) {
    restore_native_unit_owner_destructor_profiles_0077e3a0(actual_root);
    if (actual_network_byte_00e0af20 != 0) {                      // 0077E3C8
        void* const game = actual_game_publication_00e188a8;     // 0077E3D9
        if (game != nullptr && at<std::uint32_t>(game, 0x1fe4) == 1) {
            alignas(4) std::byte message_storage[0x24];
            void* const message = calls.entity_release_construct_00779780(
                message_storage, at<std::uint32_t>(actual_root, 0x70)); // 0077E3F3
            calls.route_message_0077c2a0(actual_root, message, 4, nullptr); // 0077E404
        }
    }
    withdraw_native_unit_owner_global_0077e409(actual_root, actual_global_list_00f87194);
    calls.observer_member_destruct_006e0860(bytes_at(actual_root, 0x2b0)); // 0077E427
    destroy_native_unit_owner_lists_0077e42c(actual_root);
    for (std::uint32_t remaining = 3; remaining != 0; --remaining) { // 0077E46C
        destroy_native_unit_owner_recon_record_00804300(
            bytes_at(actual_root, 0x1e8 + (remaining - 1) * 0x34), observers);
    }
    calls.game_entity_destruct_009287b0(actual_root);              // 0077E47B
}

} // namespace bsp
