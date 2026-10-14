#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp::game {
class GameNativeStringProcess;

// The application's single retained Source authority for Original mutable
// F876C0 (five 68h groups), E0B6D0/E0B704 (pending sentinels) and E0B6CE.
// This owns process data only. It neither creates a receiver nor attaches an
// empty list to normal fixed-step execution. All methods require quiescent
// startup, or the original caller's synchronization after startup.
class GameNativeTickRegistrationProcess final {
public:
    GameNativeTickRegistrationProcess(const GameNativeTickRegistrationProcess&) = delete;
    GameNativeTickRegistrationProcess& operator=(const GameNativeTickRegistrationProcess&) = delete;

    // Exactly the accepted CD27C0 twenty-write fragment, once. Accessing this
    // process facade does not run it. A repeated call preserves live links.
    bool initialize_groups_crt_once();
    bool groups_initialized() const noexcept { return groups_initialized_; }

    struct Lists {
        void* groups_00f876c0;
        void* pending_head_00e0b6d0;
        void* pending_tail_00e0b704;
        const volatile std::uint8_t& run_pass_00e0b6ce;
    };
    // For the genuine receiver parent to pass to the frame-job application.
    // Fails closed before explicit initialization; supplies no attachment.
    Lists lists();

    // SAME mutable byte for command-line 004E354F and Lua 008A199D writes.
    // SetSingleThread already uses !arg0 through DebugSwitchBytes; bind that
    // field to this address. The native waves reload this byte each dispatch.
    std::uint8_t& run_pass_00e0b6ce() noexcept { return run_pass_; }
    void* volatile& pending_registry_00f878cc() noexcept;
    void* volatile& manager_01090aa0() noexcept;

    // Actual 34h node/payload inputs only. Preserve the unchecked Original
    // group word and timer capture schedule. This does not own their storage.
    void* construct_registration_00875890(void* actual_node, void* actual_payload,
        std::uint32_t raw_group_word,
        const volatile std::uint32_t& actual_timer_word_00d7a260);

    // Qualified 87B699..87B728 only: caller has completed 77EED0 and retains
    // the actual receiver through +38B. Numeric profiles remain opaque; this
    // does not supply parent construction, callable tables or root teardown.
    void register_parent_constructed_unit_0087b699(void* actual_receiver,
        const volatile std::uint32_t& actual_timer_word_00d7a260,
        const volatile std::uint32_t& actual_initial_word_00d0e12c);

private:
    friend GameNativeTickRegistrationProcess& game_native_tick_registration_process();
    explicit GameNativeTickRegistrationProcess(GameNativeStringProcess&) noexcept;
    void require_groups() const;
    GameNativeStringProcess& strings_;
    alignas(4) std::byte groups_[5 * 0x68]{};
    alignas(4) std::byte pending_[0x68]{};
    std::uint8_t run_pass_{1};
    bool groups_initialized_{};
};
static_assert(std::is_trivially_destructible_v<GameNativeTickRegistrationProcess>);

// Permanent process lifetime, like the existing canonical string authority.
// No C++ exit cleanup invalidates links before native CRT/manager callbacks.
GameNativeTickRegistrationProcess& game_native_tick_registration_process();
} // namespace bsp::game
