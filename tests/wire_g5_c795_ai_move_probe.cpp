// Packet-local adapter regression; run tests/wire_g5_c795_ai_move_probe.cmd.
// Including the host TU exposes its private-to-production retained owner without
// adding a production testing API. /Gy /OPT:REF discards unrelated game hosts.
#include "../src/game_hosts_ai.cpp"
#include "bsp/lua_binding_ai.hpp"
#include <cstdio>
#include <cstdlib>

namespace {
void require(bool condition) { if (!condition) std::abort(); }
}

int main() {
    using Type = bsp::AiCommandType;
    using Move = bsp::game::GameAiCoordinatorHost::Impl::NativeMoveCommand;
    int owner = 0;
    const float point[3]{137.25f, -4.5f, 811.0f};
    Move cautious(&owner, Type::CautiousMove, point);
    auto state = cautious.state();
    require(state.type == Type::CautiousMove && state.owner_group == &owner);
    require(std::memcmp(state.target_position, point, sizeof(point)) == 0);
    require(!state.route_flag_24 && state.route_counter_28 == 4);
    state.route_flag_24 = true;
    state.route_counter_28 = 2;
    cautious.store_route_state(state);
    state = cautious.state();
    require(state.route_flag_24 && state.route_counter_28 == 2);
    require(std::memcmp(state.target_position, point, sizeof(point)) == 0);

    Move regroup(&owner, Type::RegroupingMove, point);
    state = regroup.state();
    require(state.type == Type::RegroupingMove && state.owner_group == &owner);
    require(std::memcmp(state.target_position, point, sizeof(point)) == 0);

    struct Sequence final : bsp::LuaBindingAiSetCommandHost {
        int step = 0;
        void* ai_group_of_entity_argument(int index) override {
            require(step++ == 0 && index == 0); return this;
        }
        void* ai_command_from_table(void* group, int index) override {
            require(step++ == 1 && group == this && index == 1); return &step;
        }
        void ai_group_set_command(void* group, void* command) override {
            require(step++ == 2 && group == this && command == &step);
        }
    } sequence;
    require(bsp::lua_binding_ai_set_command(sequence) == 0 && sequence.step == 3);
    std::puts("c795 retained MOVE constructor/type/route state and AISetCommand sequence: PASS");
}
