#pragma once
namespace bsp {
// Reserved for the B0h/snapshot host binding. Chain 512 completes the body;
// it does not turn player-seat 79h messages into replication messages.
inline constexpr bool kWireGoal4Chain512Bound = true;
}
