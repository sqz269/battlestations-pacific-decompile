#include "bsp/native_tick_registration_constructor.hpp"

#include "bsp/native_pending_registry_getter.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& raw_word_at(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(storage) + offset);
}

void* volatile& raw_pointer_at(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(
        static_cast<std::byte*>(storage) + offset);
}
} // namespace

void* construct_native_tick_registration_00875890(
    void* actual_receiver,
    void* actual_payload,
    std::uint32_t raw_group_word,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0,
    void* actual_fixed_pending_tail_00e0b704,
    const volatile std::uint32_t& actual_timer_word_00d7a260) {
    void* const captured_receiver = actual_receiver;
    raw_word_at(captured_receiver, 0x00) = 0x00d0dec8u;
    raw_word_at(captured_receiver, 0x14) = raw_group_word;
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(captured_receiver) + 0x18) = 0;
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(captured_receiver) + 0x19) = 0;
    raw_word_at(captured_receiver, 0x20) = 0;
    raw_word_at(captured_receiver, 0x1c) = 0;
    raw_word_at(captured_receiver, 0x24) = 0;
    raw_pointer_at(captured_receiver, 0x28) = actual_payload;

    void* const registry = get_native_pending_registry_00875280(
        actual_registry_publication_00f878cc, actual_manager_publication_01090aa0);
    auto* const captured_section = static_cast<CRITICAL_SECTION*>(
        raw_pointer_at(registry, 0x04));
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = raw_word_at(captured_section, 0x18);
        depth = depth + 1u;
    }

    // The actual pending-last cell is the fixed tail's +04 field, not an
    // independent binding. Keep the second read after both receiver stores.
    void* volatile& pending_last = raw_pointer_at(actual_fixed_pending_tail_00e0b704, 0x04);
    void* const initial_last = pending_last;
    raw_pointer_at(captured_receiver, 0x04) = initial_last;
    raw_pointer_at(captured_receiver, 0x08) = actual_fixed_pending_tail_00e0b704;
    void* const current_last = pending_last;
    raw_pointer_at(current_last, 0x08) = captured_receiver;
    pending_last = captured_receiver;

    // Native MOVSS transfers only these 32-bit words; no float arithmetic.
    raw_word_at(captured_receiver, 0x2c) = 0;
    raw_word_at(captured_receiver, 0x30) = actual_timer_word_00d7a260;

    if (captured_section) {
        auto& depth = raw_word_at(captured_section, 0x18);
        depth = depth - 1u;
        LeaveCriticalSection(captured_section);
    }
    return captured_receiver;
}

} // namespace bsp
