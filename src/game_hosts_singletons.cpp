#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts.hpp"

#include <cstdlib>

namespace bsp::game {

GameSingletonHost::GameSingletonHost(GameHostLog& log)
    : log_(log), lifetime_01090aa0_({this, &destroy_registered, &invalid_parameter}),
      effect_context_{lifetime_01090aa0_, effect_publication_00f87664_,
          effect_allocation_words_} {}

GameSingletonHost::~GameSingletonHost() {
    // Run before automatic member destruction removes the context required
    // by the domain's callbacks. Normal WinMain shutdown has already drained it.
    shutdown();
}

void GameSingletonHost::destroy_registered(void* context, void* owner,
    std::uint32_t flags) noexcept {
    auto& host = *static_cast<GameSingletonHost*>(context);
    // The only registration path this host exposes is 004C1650. Its D0DA64
    // owner reaches the existing full weak-map destructor and paired free.
    scalar_delete_gameplay_effect_manager_008703e0(
        static_cast<GameplayEffectManager*>(owner), flags, host.effect_context_);
}

void GameSingletonHost::invalid_parameter(void*) {
    _invalid_parameter_noinfo();
}

void GameSingletonHost::shutdown() {
    auto* manager = lifetime_01090aa0_.published_manager();
    if (manager == nullptr) return;
    const std::uint32_t registered = manager->count_00bcf910();
    lifetime_01090aa0_.shutdown();
    log_.notef("singleton lifetime drained: registered_slots=%u effect_publication=%s "
        "manager_publication=%s", registered,
        effect_publication_00f87664_ == nullptr ? "null" : "non-null",
        lifetime_01090aa0_.published_manager() == nullptr ? "null" : "non-null");
}
} // namespace bsp::game
