#pragma once

#include "bsp/native_locale_manager_deletion_lifetime.hpp"
#include <memory>

namespace bsp::game {
class GameSingletonHost;

// ONE retained Source graph over actual F8BC4C/AA0/raw strings/E1768C. The
// factory constructs metadata only; every Native operation is explicit.
class GameNativeLocaleManagerProcess final
    : public NativeLocaleManagerDeletionLifetime {
public:
    GameNativeLocaleManagerProcess(const GameNativeLocaleManagerProcess&) = delete;
    GameNativeLocaleManagerProcess& operator=(const GameNativeLocaleManagerProcess&) = delete;
    GameNativeLocaleManagerProcess(GameNativeLocaleManagerProcess&&) = delete;
    GameNativeLocaleManagerProcess& operator=(GameNativeLocaleManagerProcess&&) = delete;

    // Enter through GameSingletonHost::prepare_locale_manager(), which retains
    // this object BEFORE binding. Actual AA8/AA4 deletion bindings must already
    // be installed by the real VFS services; a bare fresh host is not ready.
    // Fix the exact host/table/cells and consume the permanent claim; no Native
    // allocation/registration or automatic provider installation occurs here.
    void bind_once(GameSingletonHost&);
    // Genuine pool startup/status0, matched uninitialized4040 allocation and
    // actual73C960. Failed effects are retained, never rolled back or retried.
    void initialize_once();
    // Bracket the actual shared drain; no eager/second locale destruction.
    // Root host retains the whole binding graph until successful detach.
    void before_shared_drain(GameSingletonHost&);
    void after_shared_drain(GameSingletonHost&);
    // Healthy actual ctor-returned/SAME publication only: initialized, NOT
    // loaded tables, mutable container permission, uppercase or Font48 ready.
    void* initialized_receiver();
    void* delete_registered_current_profile(void* captured_receiver,
        std::uint32_t captured_profile,
        const volatile std::uint32_t& actual_flags) override;

private:
    friend GameNativeLocaleManagerProcess& game_native_locale_manager_process();
    GameNativeLocaleManagerProcess();
    ~GameNativeLocaleManagerProcess(); // metadata only; retained factory never deletes it
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Metadata-only retained factory; externally serialize factory/CRT/provider
// lifetimes and common allocator-list use. No startup call is installed.
GameNativeLocaleManagerProcess& game_native_locale_manager_process();
} // namespace bsp::game
