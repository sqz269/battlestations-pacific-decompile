#include "bsp/native_unit_tick_receiver_fragment.hpp"

#include "bsp/native_tick_registration_constructor.hpp"

#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

volatile std::uint32_t& word_at(void* receiver, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(receiver) + offset);
}
} // namespace

void register_native_unit_tick_receiver_0087b699(
    void* actual_parent_constructed_receiver,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0,
    void* actual_fixed_pending_tail_00e0b704,
    const volatile std::uint32_t& actual_timer_word_00d7a260,
    const volatile std::uint32_t& actual_initial_word_00d0e12c) {
    void* const receiver = actual_parent_constructed_receiver;
    auto* const embedded_node = static_cast<std::byte*>(receiver) + 0x310;
    construct_native_tick_registration_00875890(
        embedded_node, receiver, 0u,
        actual_registry_publication_00f878cc,
        actual_manager_publication_01090aa0,
        actual_fixed_pending_tail_00e0b704,
        actual_timer_word_00d7a260);

    word_at(receiver, 0x00) = 0x00d0df70u;
    word_at(receiver, 0x10) = 0x00d0df54u;
    word_at(receiver, 0x24) = 0x00d0df4cu;
    word_at(receiver, 0x170) = 0x00d0df48u;
    word_at(receiver, 0x1e4) = 0x00d0df40u;
    word_at(receiver, 0x310) = 0x00d0df28u;
    word_at(receiver, 0x348) = 0;
    word_at(receiver, 0x34c) = 0;
    word_at(receiver, 0x350) = 0;

    // MOVSS is a bit transfer. Preserve the capture before this next store,
    // including when the borrowed word aliases the selected receiver field.
    const std::uint32_t initial_word = actual_initial_word_00d0e12c;
    word_at(receiver, 0x35c) = 0xffffffffu;
    word_at(receiver, 0x364) = initial_word;
    word_at(receiver, 0x374) = 0x3fu;
    word_at(receiver, 0x380) = 0;
    word_at(receiver, 0x384) = 0;
    word_at(receiver, 0x388) = 0;
}

} // namespace bsp
