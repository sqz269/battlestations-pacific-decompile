#pragma once

#include "bsp/native_singleton_destruction.hpp"

namespace bsp::game {
class GameHostLog;

// One application-owned raw14h manager and raw10h gameplay-effect owner.
// Private publications outlive the menu and all shutdown calls. The only
// admitted registration is raw004C1650's D0DA64 owner; its fixed actual
// deletion binding is established before that getter can run.
class GameSingletonHost final {
public:
    explicit GameSingletonHost(GameHostLog&);
    ~GameSingletonHost();
    GameSingletonHost(const GameSingletonHost&) = delete;
    GameSingletonHost& operator=(const GameSingletonHost&) = delete;

    void probe_gameplay_effect_memory(const char* label);
    // 008F8449: capture current manager, rawBD0400 drain, free captured manager,
    // then clear its actual publication, while all bindings remain alive.
    void shutdown();

private:
    GameHostLog& log_;
    void* volatile manager_publication_01090aa0_{nullptr};
    void* volatile effect_publication_00f87664_{nullptr};
    NativeSingletonDeletionBindings deletion_bindings_;
};
} // namespace bsp::game
