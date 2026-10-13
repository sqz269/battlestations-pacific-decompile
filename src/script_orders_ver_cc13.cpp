// Packet cc13-ver12: models of the script-order natives the host does not
// route or routes without the native's own gates. docs/CC13_VER_12.md.
// Pure functions; the owning host lane wires them behind its own switch.

#include "bsp/script_orders_ver_cc13.hpp"

namespace bsp {

namespace {
// 00E08F88, the `stop` command class (include/bsp/command_execution.hpp
// kCommandStop; docs/COMMAND_CLASSES.md: category 3, no target).
constexpr std::uint32_t kScriptStopCommand = 0x00E08F88u;
// 008A752C PUSH 1: the flags argument every navigator issue passes.
constexpr int kScriptNavigatorIssueFlags = 1;
}  // namespace

ScriptOrderIssue navigator_stop_order_008a7400(const float zero_vector[3]) noexcept {
    ScriptOrderIssue out;
    out.command = kScriptStopCommand;            // 008A754F PUSH 0E08F88h
    out.flags = kScriptNavigatorIssueFlags;      // 008A752C PUSH 1
    // Descriptor base = ESP+1Ch at 008A753C (one push down). EBX = 0.
    out.target.kind = 0;                         // 008A754A word +0h, 008A755A byte +0h
    out.target.position_valid = 0;               // 008A754A word +0h
    out.target.object_id = 0;                    // 008A755E word +2h
    out.target.object = nullptr;                 // 008A7556 dword +4h
    for (int lane = 0; lane < 3; ++lane) {
        out.target.position[lane] = zero_vector[lane];  // +8h/+Ch/+10h, 008A751E/752E/7541
    }
    out.target.trailing = 0.0f;                  // +14h, 008A7547 XORPS / 008A7563 MOVSS
    return out;
}

int script_fire_order_stance_0071be80(ScriptFireOrder order) noexcept {
    switch (order) {
        case ScriptFireOrder::kHoldFire: return 0;    // 0071BEDB / 0071BEE8 PUSH 0
        case ScriptFireOrder::kFreeFire: return 1;    // 0071BF2B / 0071BF38 PUSH 1
        case ScriptFireOrder::kFreeAttack: return 2;  // 0071BF7B PUSH 2
    }
    return 0;
}

ScriptFireStanceBits script_fire_stance_bits(int stance) noexcept {
    ScriptFireStanceBits bits;
    bits.allow_fire = stance == 1 || stance == 2;  // 0071D564-0071D56C
    bits.allow_move = stance == 3 || stance == 2;  // 0071D584-0071D58C
    return bits;
}

bool script_entity_live_0043f080(const ScriptEntityLiveBytes& bytes) noexcept {
    return bytes.active_5c && !bytes.torn_down_5d && !bytes.pending_destroy_60 &&
           !bytes.destroyed_5e;
}

bool pilot_set_target_issues_008a4c90(bool unit_present, const ScriptEntityLiveBytes& unit,
                                      bool target_present,
                                      const ScriptEntityLiveBytes& target,
                                      bool command_chosen) noexcept {
    // 008A4DA8 CMP EDI,EBX / JZ, then the four byte tests on the unit.
    if (!unit_present || !script_entity_live_0043f080(unit)) return false;
    // 008A4E70 TEST ESI,ESI / JZ, then the four byte tests on the target.
    if (!target_present || !script_entity_live_0043f080(target)) return false;
    // 008A4E8C / 008A4E9E: either 007EEC50 call answering non-zero issues.
    return command_chosen;
}

}  // namespace bsp
