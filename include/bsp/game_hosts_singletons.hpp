#pragma once

#include "bsp/gameplay_effect_manager.hpp"

namespace bsp::game {
class GameHostLog;

// One application-owned source lifetime domain. The menu borrows its effect
// context; both publication and destructor bindings outlive the menu. The
// current executable admits only GameplayEffectManager through this domain.
// Add each concrete owner's deleting binding before admitting another type.
// This integrates the existing typed source contracts, not native raw layout.
class GameSingletonHost final {
public:
    explicit GameSingletonHost(GameHostLog&);
    ~GameSingletonHost();
    GameSingletonHost(const GameSingletonHost&) = delete;
    GameSingletonHost& operator=(const GameSingletonHost&) = delete;

    GameplayEffectManagerContext& gameplay_effect_context() noexcept {
        return effect_context_;
    }
    // 008F8449: drain while publication/context remain alive, then free and
    // clear the manager through SingletonLifetimeDomain::shutdown.
    void shutdown();

private:
    static void destroy_registered(void*, void*, std::uint32_t) noexcept;
    static void invalid_parameter(void*);

    GameHostLog& log_;
    GameplayEffectManager* volatile effect_publication_00f87664_{nullptr};
    GameplayEffectManagerAllocationWords effect_allocation_words_{0};
    SingletonLifetimeDomain lifetime_01090aa0_;
    GameplayEffectManagerContext effect_context_;
};
} // namespace bsp::game
