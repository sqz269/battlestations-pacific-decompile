#pragma once
// Plane bot-task and ship-AI leaves from the 00990000-009DFFFF naming pass
// (cc13-gp99, docs/CC13_GP_0099_009D.md). Every name is a hypothesis, not a
// recovered symbol; the original address and ABI are recorded beside each
// declaration.
//
// Status: reconstructed, build-tested. New C++ interfaces, not drop-in binary
// replacements; nothing here is wired into a host, and nothing draws randomness.

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>
#include <vector>

namespace bsp::band0099 {

// ---------------------------------------------------------------------------
// Bot conditions. Each condition class has four slots: Test, deleting dtor,
// Load, Save (vtables 00D1E9F0 "simpletimed", 00D1EA00 "timed").
//
// 009961B0 BSP_BotConditionSimpleTimed_Test, __fastcall(ECX = condition), bool
// in AL: FLD [ECX+4] / FLD now / FCOMI / JBE, so true only when now > deadline
// (equal or unordered answers false).
bool condition_simple_timed_test_009961b0(float deadline, float now) noexcept;

// 009961D0 BSP_BotConditionTimed_Test: now > deadline (+8h, FCOMI / JA), else the
// decorated condition (+4h) answers its own Test; no inner condition answers false.
bool condition_timed_test_009961d0(float deadline, float now, const std::function<bool()>* inner);

// 009963E0 / 00996390: the "timeLeft" field is saved as deadline - now and loaded
// as now + timeLeft (both single-precision, each stored as a float).
float condition_time_left_009963e0(float deadline, float now) noexcept;
float condition_deadline_from_time_left_00996390(float now, float time_left) noexcept;

// ---------------------------------------------------------------------------
// 009A1A90 BSP_BotTask_DescribeAvoidFlags, base bot-task slot +18h,
// __thiscall(task)(string* out), RET 4. The listing assigns (not appends) the
// three names into the same string in the order bit 2, bit 1, bit 0, so the
// lowest set bit's name is what survives: bit 0 "AV_TERR", else bit 1 "AV_VEH ",
// else bit 2 "AV_GUN ", else the empty string.
std::string_view describe_avoid_flags_009a1a90(unsigned avoid_flags) noexcept;

// ---------------------------------------------------------------------------
// 0099BA90 BSP_Math_DeadZone, __fastcall(ECX = float* value, EDX = const float*
// width), unreferenced. v > d: v - d; else v < -d: v + d; else v - v (0, or NaN
// for an infinite or NaN v; the listing's FSUBR ST0,ST0).
void dead_zone_0099ba90(float& value, float width) noexcept;

// ---------------------------------------------------------------------------
// Bot-task slot +50h, the three-value step result (docs/BOT_TASKS.md). The
// image compares the current-state pointer (+310h) with member addresses; here
// the caller says which state is current.
//
// 009A2BD0 BSP_BotTaskCloseToShip_StepResult: 2 when the current state is
// neither follow state (+45Ch, +498h) and the byte +43Ch is set, else 0.
int close_to_ship_step_result_009a2bd0(bool in_follow_state, bool flag_43c) noexcept;
// 009C8290 BSP_BotTaskDiveBomb_StepResult: 1 outside the attacking states
// (BSP_BotTaskDiveBomb_IsAttackingState); inside them 2 in the state at +734h,
// 0 in the others.
int dive_bomb_step_result_009c8290(bool attacking_state, bool in_state_734) noexcept;
// 009D49E0 BSP_BotTaskTorpedo_StepResult: 1 outside the five states +710h,
// +618h, +6D8h, +6B4h, +740h; inside them 2 when +528h is set, else 0.
int torpedo_step_result_009d49e0(bool in_listed_state, bool flag_528) noexcept;

// ---------------------------------------------------------------------------
// 0099C690 BSP_BotTaskFactory_FindByName, __fastcall(ECX = counted string
// {length, chars}), returns the factory. The list head is 00F8A314, next at +0Ch,
// name length +4h, chars +8h. Lengths must match; two empty names match; otherwise
// __stricmp decides. Returns the index into `factories` or -1.
struct BotTaskFactoryName {
    std::size_t length;  // +4h
    const char* chars;   // +8h
};
int bot_task_factory_find_0099c690(const std::vector<BotTaskFactoryName>& factories,
                                   std::size_t length, const char* chars);

// ---------------------------------------------------------------------------
// 009DA9D0 BSP_ShipAiHeading_SetVector, __thiscall(record)(float heading), RET 4,
// unreferenced. +8h = heading; a = 1.5707964 (00CE3830, double) - heading,
// wrapped by + 6.2831855 (00CE3828) when negative and rounded to float; then
// +0Ch = cos a, +10h = 0, +14h = sin a (x87 FCOS / FSIN, each stored as float).
struct HeadingRecord {
    float heading{};                    // +8h
    std::array<float, 3> direction{};   // +0Ch..+14h
};
void ship_ai_heading_set_vector_009da9d0(HeadingRecord& record, float heading) noexcept;

}  // namespace bsp::band0099
