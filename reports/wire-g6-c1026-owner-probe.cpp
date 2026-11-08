// Focused ownership regression probe; no game startup or synthetic game world.
// cl /std:c++20 /MD /EHsc /O2 /Iinclude <this file> /link /MANIFEST:EMBED
#include "bsp/game_hosts_gunnery.hpp"

#include <cassert>
#include <cstdio>

int main() {
    using namespace bsp::game;
    int actual_identity = 0;
    int unrelated_identity = 0;
    const std::size_t accounting_source = 7;

    // An observer is bound while live. Deferred records retain that relation,
    // not a copied unit index; a present-but-queued notice has not delivered.
    auto projectile = std::make_shared<GameProjectileObservedOwner>(
        accounting_source, &actual_identity, false, false);
    GameProjectileRow launched;
    launched.owner_unit = accounting_source + 1; // production row's one-based source
    launched.observed_owner = projectile;
    GameProjectileRow retained = launched;
    const GameProjectileOwnerRef deferred = retained.observed_owner;
    deferred->refresh(&actual_identity, false, false);
    assert(deferred->unit() == accounting_source);
    projectile->target_gone(&unrelated_identity);
    assert(deferred->unit() == accounting_source);

    // 006E17D0 clears an in-flight owner BEFORE impact and all queued aliases.
    projectile->target_gone(&actual_identity);
    assert(!projectile->unit() && !deferred->unit());
    assert(launched.owner_unit == 8 && retained.owner_unit == 8);

    // The projectile may be retired after enqueue. Delivery still resolves the
    // current shared observer; a completed pass-K notice clears it at dispatch.
    projectile = std::make_shared<GameProjectileObservedOwner>(
        accounting_source, &actual_identity, false, false);
    const GameProjectileOwnerRef queued = projectile;
    projectile.reset();
    std::vector<bool> notified(accounting_source + 1, false);
    std::vector<std::size_t> pending;
    notified[accounting_source] = true;
    pending.push_back(accounting_source);
    queued->refresh(&actual_identity, false,
        projectile_owner_notice_delivered(accounting_source, notified, pending));
    assert(queued->unit() == accounting_source);
    pending.clear(); // the existing pass-K drain consumes this notification
    queued->refresh(&actual_identity, false,
        projectile_owner_notice_delivered(accounting_source, notified, pending));
    assert(!queued->unit());
    assert(accounting_source == 7);

    // Immediate removal publishes +5Eh before the slot04 callback. Wreck/
    // Destroy state alone leaves this flag false and must retain the owner.
    GameProjectileObservedOwner immediate(3, &actual_identity, false, false);
    immediate.refresh(&actual_identity, false, false);
    assert(immediate.unit() == std::size_t{3});
    immediate.refresh(&actual_identity, true, false);
    assert(!immediate.unit());

    // A clear is latched, while an explicit later bind captures its own event
    // baseline; neither source reuse nor old notice state recreates the slot.
    queued->refresh(&actual_identity, false, false);
    assert(!queued->unit());
    GameProjectileObservedOwner rebound(9, &actual_identity, true, true);
    rebound.refresh(&actual_identity, true, true);
    assert(rebound.unit() == std::size_t{9});
    rebound.refresh(&unrelated_identity, true, true);
    assert(!rebound.unit());

    std::puts("PASS: in-flight and deferred owner invalidation; queued versus delivered notices; source identity retained");
}
