#pragma once

// Packet cc13-ver12: the Lua script-order natives checked against the bytes.
// docs/CC13_VER_12.md has the per-native verdicts. These are models of the two
// natives the host never routes (NavigatorStop, UnitFreeAttack/UnitFreeFire)
// and of the PilotSetTarget liveness gates the host's game path skips, so the
// owning lane can adopt them behind its own switch. Names are hypotheses.

#include <cstdint>

#include "bsp/scene_deferred_refs.hpp"

namespace bsp {

// The order a native hands 0077D600 BSP_Entity_IssueCommand(this = entity,
// command, &descriptor, flags).
struct ScriptOrderIssue {
    std::uint32_t command{0};
    SceneCommandTarget target{};
    int flags{0};
};

// 008A7400 `NavigatorStop(unit)`. Argument 0 through 00888AA0 (008A74FE, no
// null test: ECX = 0 faults inside 0077D600), then a descriptor with kind,
// position_valid, id, object and +14h all zero and the vector copied from the
// zero vector at 00F87574..7C (008A7516-008A7563), and
// 0077D600(entity, stop 00E08F88, &descriptor, 1) at 008A7569. Nothing is
// pushed. `zero_vector` is the content of 00F87574 (all zero in the image).
ScriptOrderIssue navigator_stop_order_008a7400(const float zero_vector[3]) noexcept;

// The literal stance each fire-order native hands 0071BE80 through its
// wrapper: 0071BED0 pushes 0 (UnitHoldFire 008A6AC0), 0071BF20 pushes 1
// (UnitFreeFire 008A6950, PUSH 1 at 0071BF2B), 0071BF70 pushes 2
// (UnitFreeAttack 008A67E0, PUSH 2 at 0071BF7B). UnitSetFireStance 008A6490
// passes its integer argument through unchanged.
enum class ScriptFireOrder : std::uint8_t { kHoldFire, kFreeFire, kFreeAttack };
int script_fire_order_stance_0071be80(ScriptFireOrder order) noexcept;

// 0071D560 / 0071D580 as the director reads them after 0071BE80: fire is
// allowed for stance 1 or 2, movement for stance 2 or 3. A stance-1 order
// (UnitFreeFire) therefore forbids movement on a director that never held.
struct ScriptFireStanceBits {
    bool allow_fire{true};  // director +3Ch
    bool allow_move{true};  // director +3Dh
};
ScriptFireStanceBits script_fire_stance_bits(int stance) noexcept;

// 008A4C90 `PilotSetTarget`: the 0043F080-shaped live test the native applies
// to the unit (008A4DA8-008A4DD2, before argument 1 is read) and to the
// resolved target (008A4E70-008A4E8A, after the first 007EEC50 call):
// [+5Ch] != 0 && [+5Dh] == 0 && [+60h] == 0 && [+5Eh] == 0. A position-only
// target resolves to null and is refused at 008A4E72. The repeated 007EEC50
// call (008A4E99) has the same three arguments and 007EEC50 writes nothing, so
// it can only repeat the first answer. pilot_order_bindings.cpp's
// pilot_set_target_008a4c90 already models the whole body; this answers the
// one question the host's game path needs: does the native issue at all.
struct ScriptEntityLiveBytes {
    bool active_5c{false};
    bool torn_down_5d{false};
    bool destroyed_5e{false};
    bool pending_destroy_60{false};
};
bool script_entity_live_0043f080(const ScriptEntityLiveBytes& bytes) noexcept;
bool pilot_set_target_issues_008a4c90(bool unit_present, const ScriptEntityLiveBytes& unit,
                                      bool target_present,
                                      const ScriptEntityLiveBytes& target,
                                      bool command_chosen) noexcept;

}  // namespace bsp
