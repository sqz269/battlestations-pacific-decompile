// Exercise the actual private command adapter against retained formation
// publications, then apply the recovered 0077D791..0077D7A3 gate. No game loop.
#include "../src/game_hosts_commands.cpp"
#include <cstdio>
#include <stdexcept>

int main() {
    using namespace bsp::game;
    GameHostLog log;
    GameCommandsHost::Impl owner(log);
    owner.units.resize(3);
    for (std::size_t i = 0; i < owner.units.size(); ++i) owner.units[i].index = i;
    auto& unit = owner.units[1];
    GameDirector director;
    ChainState chain{owner, unit, director};
    EntityIssueBinding binding(chain);
    int group_identity = 0; // Opaque non-null AI group for the recovered gate.
    auto require = [](bool condition) {
        if (!condition) throw std::runtime_error("c809 formation forwarding gate regression");
    };
    auto notified = [&] {
        void* controller = binding.entity_controller(&unit);
        return bsp::entity_order_ai_group_is_notified(&group_identity, controller,
            controller != nullptr ? binding.controller_owner(controller) : nullptr, &unit);
    };

    require(binding.entity_controller(&unit) == nullptr && notified());
    unit.formation_follower = true;
    unit.formation_leader = 0;
    void* const controller = binding.entity_controller(&unit);
    require(controller != nullptr && binding.controller_owner(controller) == &owner.units[0]);
    require(!notified()); // A follower takes the skip arm at 0077D7A3.
    unit.formation_leader = 2; // Same retained controller cell, fresh leader publication.
    require(binding.controller_owner(controller) == &owner.units[2] && !notified());
    unit.formation_leader = unit.index; // Native +14h == entity admits a leader.
    require(binding.controller_owner(controller) == &unit && notified());
    unit.formation_follower = false; // Departure withdraws the controller publication.
    require(binding.entity_controller(&unit) == nullptr && notified());
    std::puts("c809 live formation controller/leader forwarding gate: PASS");
}
