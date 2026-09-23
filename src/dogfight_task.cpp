// The dogfight bot task, packet cc9_dogfight_task. See include/bsp/dogfight_task.hpp
// and docs/DOGFIGHT_TASK.md. Reconstructed and build-tested; the moveto rule is a
// labelled stand-in.
#include "bsp/dogfight_task.hpp"

#include <cmath>

#include "bsp/plane_flight.hpp"   // heading_command_009f9e40, pitch_command_to_point_009f9ed0

namespace bsp {

const char* dogfight_state_name(DogfightState s) noexcept {
    switch (s) {
    case DogfightState::kMoveTo: return "moveto";
    case DogfightState::kFollow: return "follow";
    case DogfightState::kPrepare: return "prepare";
    case DogfightState::kAim: return "aim";
    case DogfightState::kManeuver: return "maneuver";
    case DogfightState::kAttackRun: return "attackrun";
    case DogfightState::kAvoidRoll: return "avoid_roll";
    case DogfightState::kAvoidTurn: return "avoid_turn";
    default: return "none";
    }
}

bool dogfight_engaged_009aafa0(bool latch_4c8, int control_mode_370,
                               bool target_4c4) noexcept {
    return latch_4c8 || (control_mode_370 == 2 && target_4c4);
}

DogfightState dogfight_unengaged_state_009aafa0(bool is_flight_leader) noexcept {
    return is_flight_leader ? DogfightState::kMoveTo : DogfightState::kFollow;
}

DogfightMoveToCommand dogfight_moveto_standin(const DogfightMoveToInputs& in) noexcept {
    DogfightMoveToCommand out;
    const double dx = static_cast<double>(in.target_pos[0]) - in.own_pos[0];
    const double dz = static_cast<double>(in.target_pos[2]) - in.own_pos[2];
    out.horizontal_range = static_cast<float>(std::sqrt(dx * dx + dz * dz));
    out.heading = heading_command_009f9e40(in.target_pos[0], in.target_pos[2],
                                           in.own_pos[0], in.own_pos[2]);
    const float distance = (out.horizontal_range > in.min_distance)
        ? out.horizontal_range : in.min_distance;
    out.pitch = pitch_command_to_point_009f9ed0(in.cruising_alt - in.own_pos[1], distance,
                                                in.class_climb_angle_1e4);
    return out;
}

}  // namespace bsp
