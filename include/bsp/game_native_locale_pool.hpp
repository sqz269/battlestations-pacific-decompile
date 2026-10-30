#pragma once

#include <memory>

namespace bsp::game {

// Retained Source owner for the ONE E1768C projection. It borrows the physical
// process's SAME E188B4 list, without starting the unrelated physical pool.
// Keep every borrow/list use inside that common domain's lifetime window;
// retire all nodes/payloads before the registered locale-pool exit callback.
class GameNativeLocalePoolProcess final {
public:
    GameNativeLocalePoolProcess(const GameNativeLocalePoolProcess&) = delete;
    GameNativeLocalePoolProcess& operator=(const GameNativeLocalePoolProcess&) = delete;
    GameNativeLocalePoolProcess(GameNativeLocalePoolProcess&&) = delete;
    GameNativeLocalePoolProcess& operator=(GameNativeLocalePoolProcess&&) = delete;

    // Explicit one-attempt CC8D30 routing: real constructor, then actual
    // std::atexit. Cache its int INCLUDING nonzero, without rollback/retry.
    // Binding/constructor/registration reentry or failure stays poisoned.
    int initialize_once_00cc8d30();

    // SAME actual initialized38h cell, only after healthy startup returned and
    // before exit begins. Nonzero registration still leaves it initialized.
    // This supplies no slab/slot/payload, loaded-manager or startup authority.
    void* pool_storage_00e1768c();

private:
    friend GameNativeLocalePoolProcess& game_native_locale_pool_process();
    GameNativeLocalePoolProcess();
    ~GameNativeLocalePoolProcess(); // bookkeeping only; retained factory never deletes it
    static void destroy_at_exit_00cd9410() noexcept;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Metadata-only factory. No native construction or automatic C++ pool teardown.
// Externally serialize common-list mutations/uses across all process owners;
// this owner's recursive metadata mutex does not supply a common-list lock.
GameNativeLocalePoolProcess& game_native_locale_pool_process();
} // namespace bsp::game
