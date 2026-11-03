#pragma once

// Goal 4 chain 250 (agent/g4-w3): the AI planner leaves in game_hosts_ai.cpp
// run their reconstructed bodies (reports/wire-g4-c250.json).
//  - 00A24870 ai_collect_threats_near (with 00A07D40 / 00A1A660) replaces the
//    capture defender stand-in loop;
//  - 00A16450 ai_party_schedule_think_within runs at the spawn arm's 0.5 s
//    publish (00CE3800);
//  - 00A03760 ai_unit_arrival_value replaces the second implementation the
//    capture score host called.
// False: the previous host code runs unchanged.
namespace bsp {
inline constexpr bool kWireGoal4Chain250Bound = true;
}
