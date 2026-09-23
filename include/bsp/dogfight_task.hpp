#pragma once

// The dogfight bot task (kind 2), packet cc9_dogfight_task. docs/DOGFIGHT_TASK.md.
//
// ADDRESSES.  Factory 009AB570; derived constructor 009A9810
// (BSP_BotTaskDogfight_Construct), which runs 009F9980 (step 9) before the
// 007B8AD0 flight-leader test (step 6); the state objects are built by
// 009A94E0 with ESI = task+3F8h; the per-tick arm is primary vtable 00D1F9B0
// slot +64h = 009AB1C0 (no Ghidra function):
//
//   009AB1D1  task+4F8h = FFh
//   009AB1DB  009AAC70(approach = task+3F8h, dt)    ; reads AttackDist (+644h)
//   009AB1EA  009AAFA0(task, dt)                     ; the state transitions
//   009AB202  state(task+310h)->vtable[0Ch](dt)      ; the state tick
//   009AB20A  task+2E4h = task+4F8h
//
// The states (009A94E0, ESI = task+3F8h; names from 009A6A60):
//   moveto     +510h  009C2CA0 -> vtable 00D20B24, tick 009C18C0 (the generic
//                      moveto tick), speed slot +1Ch 009C1BC0
//   follow     +54Ch  009C2980 BSP_BotStateFollow_Construct (tick 009C1FD0)
//   prepare    +5E4h  009C2980, a second follow object
//   aim        +67Ch  vtable 00D1F8D8, tick 009A76E0
//   maneuver   +6A4h  009A84E0
//   attackrun  +6E4h  009A70E0
//   avoid_roll +708h  vtable 00D1F918, tick 009A7E80
//   avoid_turn +730h  vtable 00D1F938, tick 009A80E0

namespace bsp {

// 007EEC50's dogfight class (include/bsp/attack_commands.hpp kAttackCmdDogfight).
inline constexpr unsigned int kDogfightCommandClass = 0x00E08F58u;

enum class DogfightState : int {
    kNone = 0,
    kMoveTo,     // +510h
    kFollow,     // +54Ch
    kPrepare,    // +5E4h
    kAim,        // +67Ch
    kManeuver,   // +6A4h
    kAttackRun,  // +6E4h
    kAvoidRoll,  // +708h
    kAvoidTurn,  // +730h
};
inline constexpr int kDogfightStateCount = 9;
const char* dogfight_state_name(DogfightState s) noexcept;

// 009AAFA0's engagement test, used on both of its arms:
//   task+4C8h != 0  ||  (unit+370h == 2  &&  task+4C4h != 0)
bool dogfight_engaged_009aafa0(bool latch_4c8, int control_mode_370,
                               bool target_4c4) noexcept;

// 009AAFA0's arm for a task that is not engaged: moveto for the flight leader
// (007B8AD0 true), follow for a wing member. From moveto/follow it is the
// fall-through at 009AB19x; from any other state it is the 007B8AD0 test after
// 009AB0xx. Returns the state the task is in afterwards.
DogfightState dogfight_unengaged_state_009aafa0(bool is_flight_leader) noexcept;

// STAND-IN, labelled: the moveto state's tick 009C18C0 flies the glide the
// cruise profile 009AAF30 sets up; this host has not read the dogfight speed
// slot 009C1BC0 nor bound 009AAF30. What stands in is its end state: head at the
// commanded target (009F9E40) and pitch toward Pilot/Dogfight/CruisingAlt with
// the image's own point pitch law 009F9ED0, over at least 250 m, so the command
// is an elevation angle capped by the class climb, never a bang-bang cap.
struct DogfightMoveToInputs {
    float own_pos[3] = {0.0f, 0.0f, 0.0f};
    float target_pos[3] = {0.0f, 0.0f, 0.0f};
    float cruising_alt = 1400.0f;       // singleton+640h
    float min_distance = 250.0f;        // the fly-to arm's FollowedPointDist floor
    float class_climb_angle_1e4 = 0.0f; // desc+1E4h
};
struct DogfightMoveToCommand {
    float heading = 0.0f;        // plan+2C0h, mode 2
    float pitch = 0.0f;          // plan+2BCh, mode 2
    float horizontal_range = 0.0f;
};
DogfightMoveToCommand dogfight_moveto_standin(const DogfightMoveToInputs& in) noexcept;

}  // namespace bsp
