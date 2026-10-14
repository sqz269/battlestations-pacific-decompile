#include "bsp/native_unit_owner_entity.hpp"

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

} // namespace bsp
