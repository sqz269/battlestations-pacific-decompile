#include "bsp/game_native_tick_registration_process.hpp"

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_pending_tick_group_initialization.hpp"
#include "bsp/native_tick_registration_constructor.hpp"
#include "bsp/native_unit_tick_receiver_fragment.hpp"

#include <stdexcept>

namespace bsp::game {
static_assert(sizeof(void*) == 4);

GameNativeTickRegistrationProcess::GameNativeTickRegistrationProcess(
    GameNativeStringProcess& strings) noexcept : strings_(strings) {
    // Original PE data contains exactly these two absolute pointer words;
    // every other pending-list byte and all 208h group bytes are zero. Source
    // addresses relocate the same head/tail roles, never the numeric PE VAs.
    *reinterpret_cast<void**>(pending_ + 0x08) = pending_ + 0x34;
    *reinterpret_cast<void**>(pending_ + 0x38) = pending_;
}

bool GameNativeTickRegistrationProcess::initialize_groups_crt_once() {
    if (groups_initialized_) return false;
    initialize_native_pending_tick_group_sentinels_00cd27c0(groups_);
    groups_initialized_ = true;
    return true;
}

void GameNativeTickRegistrationProcess::require_groups() const {
    if (!groups_initialized_) {
        throw std::logic_error("native tick groups require explicit CRT initialization");
    }
}

GameNativeTickRegistrationProcess::Lists GameNativeTickRegistrationProcess::lists() {
    require_groups();
    return {groups_, pending_, pending_ + 0x34, run_pass_};
}

void* volatile& GameNativeTickRegistrationProcess::pending_registry_00f878cc() noexcept {
    return strings_.pending_registry_00f878cc();
}

void* volatile& GameNativeTickRegistrationProcess::manager_01090aa0() noexcept {
    return strings_.manager_01090aa0();
}

void* GameNativeTickRegistrationProcess::construct_registration_00875890(
    void* actual_node, void* actual_payload, std::uint32_t raw_group_word,
    const volatile std::uint32_t& actual_timer_word_00d7a260) {
    require_groups();
    return construct_native_tick_registration_00875890(actual_node, actual_payload,
        raw_group_word, pending_registry_00f878cc(), manager_01090aa0(),
        pending_ + 0x34, actual_timer_word_00d7a260);
}

void GameNativeTickRegistrationProcess::register_parent_constructed_unit_0087b699(
    void* actual_receiver, const volatile std::uint32_t& actual_timer_word_00d7a260,
    const volatile std::uint32_t& actual_initial_word_00d0e12c) {
    require_groups();
    register_native_unit_tick_receiver_0087b699(actual_receiver,
        pending_registry_00f878cc(), manager_01090aa0(), pending_ + 0x34,
        actual_timer_word_00d7a260, actual_initial_word_00d0e12c);
}

GameNativeTickRegistrationProcess& game_native_tick_registration_process() {
    static auto* const process = new GameNativeTickRegistrationProcess(
        game_native_string_process());
    return *process;
}
} // namespace bsp::game
