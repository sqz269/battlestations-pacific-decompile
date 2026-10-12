#pragma once
// Gameplay leaves from the 009F0000-00A3FFFF naming pass (cc13-gp9f,
// docs/CC13_GP_009F_00A3.md). Every name is a hypothesis, not a recovered
// symbol; the original address and ABI are recorded beside each declaration.
//
// Status: reconstructed, build-tested. New C++ interfaces, not drop-in binary
// replacements; nothing here is wired into a host, and nothing draws randomness.
// The image's entity and group objects are replaced by plain inputs.

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "bsp/ai_planners.hpp"

namespace bsp::band009f {

// ---------------------------------------------------------------------------
// 009FEC40 BSP_AiKey4_Less, __stdcall(const Key*, const Key*), RET 8, bool in AL.
// The comparator of the four-field tree 00A00370 searches. The listing compares
// fields 0 and 2 unsigned (JB/JA) and fields 1 and 3 signed (JL/JG).
struct AiKey4 {
    std::uint32_t a{};  // +0h
    std::int32_t b{};   // +4h
    std::uint32_t c{};  // +8h
    std::int32_t d{};   // +0Ch
};
bool ai_key4_less_009fec40(const AiKey4& lhs, const AiKey4& rhs) noexcept;

// ---------------------------------------------------------------------------
// 00A1A660 BSP_AiPointFilter_OutsideSphere, slot 0 of the point-filter vtable
// 00D22CC0, __thiscall(filter)(const float* point), RET 4. Filter layout:
// centre +4h..+0Ch, squared radius +10h.
struct AiSphereFilter {
    std::array<float, 3> centre{};  // +4h
    float radius_squared{};         // +10h
};
bool ai_point_outside_sphere_00a1a660(const AiSphereFilter& filter, const float point[3]) noexcept;

// ---------------------------------------------------------------------------
// 00A2C150 / 00A2C1C0 / 00A2C230: BSP_AiGroup_IsMovingTo, _IsRegroupingTo,
// _IsPatrollingTo. __thiscall(group)(const float* point), RET 4. The group's
// command must answer IsType(family) (2 MOVE, 5 REGROUPINGMOVE, 0Ch PATROLTO),
// and its destination (+8h, +10h; y ignored) must lie strictly within 1.0 of the
// point by squared horizontal distance.
bool ai_group_command_targets_point(AiCommandType command, AiCommandType family,
                                    const float destination[3], const float point[3]) noexcept;

// ---------------------------------------------------------------------------
// The AI command class slots this pass named (docs/AI_COMMAND_OBJECT.md: +20h
// CloneForGroup, +1Ch the slot nothing reads). CloneForGroup rebuilds the same
// class for a new owner except two: CAUTIOUSMOVE clones as MOVETO (00A10540) and
// CAUTIOUSATTACK as MOVETOATTACK (00A12D40), so a clone drops the cautious route.
// Abstract and slot-less types answer nullopt.
std::optional<AiCommandType> ai_command_clone_type(AiCommandType type) noexcept;

// ---------------------------------------------------------------------------
// Planner virtuals +18h GetType, +1Ch IsType and +2Ch GetName (vtables in
// docs/AI_PLANNERS.md). GetType answers 0..7 in brain-slot order (Capture 3,
// 00A1F480 MOV EAX,3), but the Capture IsType 00A1F490 tests id == 1, the Attack
// id, so Capture::IsType(3) is false and Capture::IsType(1) true, as read.
int ai_planner_get_type(AiPlannerKind kind) noexcept;
bool ai_planner_is_type(AiPlannerKind kind, int id) noexcept;
std::string_view ai_planner_coordinator_name(AiPlannerKind kind) noexcept;

// ---------------------------------------------------------------------------
// 00A2C0A0 / 00A2C0D0 / 00A2C100 BSP_InputDevice_IsShiftDown / IsCtrlDown /
// IsAltDown, __fastcall(device), unreferenced. Each asks the device's
// vtable[20h](scan code) for the left key, then the right key (DirectInput
// scan codes).
using KeyDownQuery = std::function<bool(int scan_code)>;
bool input_shift_down_00a2c0a0(const KeyDownQuery& key_down);
bool input_ctrl_down_00a2c0d0(const KeyDownQuery& key_down);
bool input_alt_down_00a2c100(const KeyDownQuery& key_down);

// ---------------------------------------------------------------------------
// 00A1C960 BSP_AiPlanner_NearestGroupOfTeam, __fastcall(ECX = point, EDX = team)
// (float* out_distance_squared), RET 4. Walks the team's group list
// (00F8AA4C + team*0Ch); empty groups (+5644h == 0) are skipped. Returns the
// index into `leaders` or nullopt; the image returns the group or 0.
struct AiGroupLeader {
    bool has_members{};             // group+5644h != 0
    std::array<float, 3> position{};  // first member's +FCh..+104h
};
struct AiNearestGroup {
    std::optional<std::size_t> index;
    float distance_squared{};  // seeded with -1.0 (00D7A260) when nothing is found
};
AiNearestGroup ai_nearest_group_00a1c960(const std::vector<AiGroupLeader>& leaders,
                                         const float point[3]) noexcept;

// 00A1CA60 BSP_AiPlanner_TeamWeightedCentroid, __fastcall(ECX = out, EDX = team),
// RET. Leader positions weighted by BSP_AiGroup_TotalLeaderWeight; groups with no
// members (+5644h == 0) are skipped (the inner fallback to the zero vector
// 00F87574 is unreachable behind that test). The image divides by the total
// without a zero check; so does this.
struct AiWeightedLeader {
    bool has_members{};
    std::array<float, 3> position{};
    float weight{};
};
std::array<float, 3> ai_team_weighted_centroid_00a1ca60(const std::vector<AiWeightedLeader>& groups) noexcept;

// ---------------------------------------------------------------------------
// 00A2B8F0 / 00A2B950 / 00A2BCC0: the group's candidate score list at
// group+24h (128 records of ACh bytes, count at group+5624h). Its only record
// reader is the unreached debug overlay 00A2CDD0 (src/game_hosts_ai.cpp,
// packet cc9_group_score_list_release). Record layout, from 00A2B950's stores:
//   +0h + 4*i  float value i (i = 0..15; value 0 is the score the list sorts on)
//   +40h + 6*i label i, copied with an unbounded strcpy, so a label longer than
//              five characters runs into the next label (and label 15 at +9Ah
//              into +A0h); this copy is clipped at the record end, the image's
//              is not
//   +A0h       aux dword (the insert's second argument)
//   +A4h       chosen byte, cleared on insert, set by 00A2BCC0
//   +A8h       key (the group or entity the record scores)
class AiGroupScoreList {
public:
    static constexpr std::size_t kRecordSize = 0xAC;
    static constexpr std::size_t kCapacity = 128;
    using Record = std::array<std::uint8_t, kRecordSize>;

    struct Term {
        const char* label;  // null is stored as ""
        float value;
    };

    // 00A2B8F0, __thiscall(list)(key), RET 4: removes the first record with the
    // key and shifts the later records down.
    void remove(std::uint32_t key) noexcept;
    // 00A2B950, __thiscall(list)(key, aux, value0, label0, value1, label1, ...),
    // 16 (value, label) pairs. Removes the key, then inserts before the first
    // record whose score is below value0 (or unordered: FCOMI / JB), so equal
    // scores keep insertion order. The image has no capacity check; this refuses
    // a full list and returns false.
    bool insert_sorted(std::uint32_t key, std::uint32_t aux, const std::array<Term, 16>& terms) noexcept;
    // 00A2BCC0, __thiscall(list)(key), RET 4: sets +A4h on every matching record.
    void mark_chosen(std::uint32_t key) noexcept;

    std::size_t size() const noexcept { return count_; }
    const Record& record(std::size_t index) const { return records_.at(index); }
    static float value(const Record& record, int index) noexcept;
    static std::uint32_t key(const Record& record) noexcept;
    static std::uint32_t aux(const Record& record) noexcept;
    static bool chosen(const Record& record) noexcept;

private:
    std::array<Record, kCapacity> records_{};
    std::size_t count_{};
};

// ---------------------------------------------------------------------------
// 009F6C80 BSP_SquadronAutoTarget_PickFilter, __fastcall(ECX = auto-target),
// RET. Chooses one of three filter sub-objects (+0Ch, +14h, +1Ch); the inputs
// are the predicates the body calls, in its order.
enum class SquadronTargetFilter { At0C, At14, At1C };
struct SquadronFilterInputs {
    bool leader_uncommitted_kamikaze{};  // BSP_PlaneSquadron_LeaderIsUncommittedKamikaze
    bool bomber_command_class{};         // unit+54h is 00E08F68, 00E08F88 or 00E08F80
    bool has_007edac0{};                 // 007EDAC0 (unread predicate)
    bool has_bomb_ordnance{};            // BSP_Unit_HasGeneralBombOrdnance(0)
    bool has_torpedo_ordnance{};         // BSP_Unit_HasTorpedoOrdnance(0)
    bool has_depth_charge_ordnance{};    // BSP_Unit_HasDepthChargeOrdnance(0)
    bool has_drop_kamikaze_ordnance{};   // BSP_Unit_HasDropKamikazeOrdnance(0)
    bool latch_3c{};                     // unit+3Ch
    bool latch_3d{};                     // unit+3Dh
};
SquadronTargetFilter squadron_auto_target_pick_filter_009f6c80(const SquadronFilterInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 00A16450 BSP_AiParty_ScheduleThinkWithin, __thiscall(ECX = party index)(float
// dt), RET 4. Pulls next_think[party] (00F8A87C) forward to now + dt when that
// is earlier; never pushes it later.
void ai_party_schedule_think_within_00a16450(std::array<float, 8>& next_think, int party,
                                             float now, float dt);

// 00A15950 BSP_Ai_FirstActivePartySlot, no arguments, RET. The first index 0..7
// whose entry in 00F8A89C is non-null with +0Ch non-zero, else -1.
// src/ai_tuning_globals.cpp reads the result as a difficulty index.
int ai_first_active_party_slot_00a15950(const std::array<bool, 8>& slot_active) noexcept;

// 00A2C3D0 BSP_AiGroup_ReconRatio, __fastcall(group), float on the x87 stack.
// Parties 0 and 1 only (group+5638h < 2, else 0): the count of members for which
// 00779D00 answers a positive value, FILD'd and divided by the member count
// group+5644h (an empty party-0/1 group divides by zero, as in the image).
float ai_group_recon_ratio_00a2c3d0(int party, const std::vector<int>& member_values);

}  // namespace bsp::band009f
