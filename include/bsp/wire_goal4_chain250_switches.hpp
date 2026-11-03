#pragma once

// Goal 4 chain 250 (agent/g4-w3): reconstructed bodies run on live host paths
// (reports/wire-g4-c250.json).
//  game_hosts_ai.cpp:
//  - 00A24870 ai_collect_threats_near (with 00A07D40 / 00A1A660) replaces the
//    capture defender stand-in loop;
//  - 00A16450 ai_party_schedule_think_within runs at the spawn arm's 0.5 s
//    publish (00CE3800);
//  - 00A03760 ai_unit_arrival_value replaces the second implementation the
//    capture score host called.
//  game_hosts_gunnery.cpp:
//  - 00730498's seat-bot BulletThrowMul getters 008FB4C0 / 008FB4E0 /
//    008FB500 / 008FB550 / 008FB570 on the descriptors 00901610 loaded
//    (cc13_torp_goaway keeps the copies);
//  - the bot slot accepts 008FBE00, 008FBF70 / 008FBFC0 and 008FC010;
//  - 00731020 for the category rebuild's MaxRange.
//  game_hosts_ship_ai.cpp:
//  - 008FB530 TorpedoBot FireTargetAccuracy on the loaded descriptor.
// False: the previous host code runs unchanged (build-checked both ways).
namespace bsp {
inline constexpr bool kWireGoal4Chain250Bound = true;
}
