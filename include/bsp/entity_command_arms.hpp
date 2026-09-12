#pragma once

#include <cstdint>

#include "bsp/cruise_command.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp {
// The arm cascade of 00816E30 BSP_UnitInstance_ApplyEntityCommand, 00816EA6 to
// 0081732E: which command singleton a routed order becomes before the tail at
// 00817330 hands it to the director. Semantic interfaces, not native object
// layouts. Names are hypotheses, not recovered symbols. Native addresses,
// evidence and uncertainty: docs/ENTITY_COMMAND_ARMS.md.
//
// This module is the front half of the routine. The tail is already
// reconstructed as unit_apply_entity_command_00816e30 in bsp/cruise_command.hpp
// and the message and descriptor types come from there and from
// bsp/scene_deferred_refs.hpp; neither is redeclared here.

// The command singletons the cascade tests, by address. The names are the
// recovered literals of docs/COMMAND_CLASSES.md, which this packet treats as a
// contract; only the addresses were re-read here.
enum class EntityCommandArmId : std::uint32_t {
    None = 0u,
    SetTarget = 0x00E08EF8u,
    ClearTarget = 0x00E08F00u,
    ClearOrders = 0x00E08F08u,
    Artillery = 0x00E08F10u,
    Follow = 0x00E08F60u,
    MoveTo = 0x00E08F68u,
    Cruise = 0x00E08F70u,
    AttackMove = 0x00E08F78u,
    MoveOnPath = 0x00E08F80u,
    Stop = 0x00E08F88u,
    Land = 0x00E08FA0u,
    Leave = 0x00E08FB0u,
    Disband = 0x00E08FB8u,
};

// What the cascade decided.
enum class EntityCommandArmResult : int {
    // The arm did its own work and returned; nothing is queued on the director.
    HandledWithoutQueueing = 0,
    // Fall through to 00817330: the tail runs, after testing the command for 0.
    IssueChecked = 1,
    // Jump to 00817334: the tail runs without that test, because the arm
    // substituted a non-null singleton (00816FB9, 00816FDE, 0081723D, 0081732E).
    IssueUnchecked = 2,
};

struct EntityCommandArmDecision {
    EntityCommandArmResult result{EntityCommandArmResult::IssueChecked};
    EntityCommandArmId command{EntityCommandArmId::None};
    // True when the arm manufactured a throwaway target entity and rewrote the
    // descriptor to name it (0081723D..0081732E).
    bool made_throwaway_target{false};
};

// 007AC9D0: __fastcall(entity), RET 0, body 007AC9D0-007ACA2C, complete. A
// cast to the interface a path-following order needs, by entity kind. The
// returned value is a byte offset from the entity, or 0 for "no interface".
// The three offsets 170h, 1E4h and 310h are the unit's secondary bases of
// docs/UNIT_INSTANCE_UPDATE.md, so this is a fixed-offset interface cast.
struct EntityCommandKindHost {
    virtual ~EntityCommandKindHost() = default;
    // 007AC9DE (47h), 007AC9F5 (48h), 007ACA0C (49h) and 007ACA21 (4Ah):
    // CALL EDX = entity->vtable[5Ch](kind).
    virtual bool entity_is_kind_of_vtable5c(int kind) = 0;
};
std::uint32_t entity_path_interface_offset_007ac9d0(bool entity_present,
                                                    EntityCommandKindHost& host);

struct EntityCommandArmsHost {
    virtual ~EntityCommandArmsHost() = default;
    // 00816EB6, 00816F08, 00816FA0, 00816FF1: 00521EA0
    // BSP_CommandTarget_ResolveObject, __fastcall(descriptor) with no stack
    // argument (RET 0, 00521EA7). Returns 0 when the descriptor's kind is 0.
    virtual std::uint32_t resolve_target_00521ea0() = 0;
    // 00816ECA and 00816F1C: target->vtable[5Ch](2).
    virtual bool target_is_kind_of_vtable5c(std::uint32_t target, int kind) = 0;
    // 00816FCF: this->vtable[5Ch](0Ch) on the unit itself, not on the target.
    virtual bool self_is_kind_of_vtable5c(int kind) = 0;
    // 00816ED5: 0077C8D0 BSP_Entity_RequestJoinFormation(target).
    virtual void request_join_formation_0077c8d0(std::uint32_t target) = 0;
    // 00816EE2: 0064A8E0(). Body unread: contract unread.
    virtual void call_0064a8e0() = 0;
    // 00816F27: 0077C980(target). Body unread: contract unread.
    virtual void call_0077c980(std::uint32_t target) = 0;
    // 00816F4B: 0077CA60(). Body unread: contract unread.
    virtual void call_0077ca60() = 0;
    // 00816FA7: 007AC9D0 on the resolved target; see the pure rule above.
    virtual std::uint32_t path_interface_007ac9d0(std::uint32_t target) = 0;
    // 00816FFD and 008171CF: 00835860 BSP_WeaponDirector_SetFireTarget,
    // __thiscall(director)(target, force). Both sites push two dwords; the
    // PUSH 1 at 00816FEB is that second argument, not an argument of
    // 00521EA0, which is why 00816FED reaches the descriptor at ESP+24h
    // instead of ESP+20h.
    virtual void set_fire_target_00835860(std::uint32_t target, int force) = 0;
    // 008171D6: 007788B0 BSP_Entity_ControllerBelongsToAnother.
    virtual bool controller_belongs_to_another_007788b0() = 0;
    // 008171E9: 0071D880 BSP_WeaponDirector_SendClearCommands. The tail calls
    // the same routine at 0081733E; that call belongs to
    // unit_apply_entity_command_00816e30.
    virtual void send_clear_commands_0071d880() = 0;
    // 008171F6: 0080DC70(). Body unread: contract unread.
    virtual bool call_0080dc70() = 0;
    // 00817205: 0071BF20 BSP_WeaponDirector_FreeFire.
    virtual void free_fire_0071bf20() = 0;
    // The cleartarget block 00817023..008171BB is not projected. A host that
    // needs it runs the native routine; this method stands for the whole block
    // and its eight call sites, listed in docs/ENTITY_COMMAND_ARMS.md.
    virtual void clear_target_block_00817023() = 0;
    // The throwaway-target block 00817243..0081732E, one method per native call.
    virtual std::uint32_t allocate_zeroed_00470b80(std::uint32_t size) = 0;  // 00817248
    virtual std::uint32_t construct_entity_004e5980(std::uint32_t memory) = 0; // 00817264
    virtual void place_entity_vtable98(std::uint32_t entity,
                                      std::uint32_t session_field_19cc) = 0;  // 008172FF
    virtual std::uint32_t transform_from_position_0059bd20() = 0;             // 0081730A
    virtual void set_entity_transform_006e8040(std::uint32_t entity,
                                              std::uint32_t transform) = 0;   // 00817312
    virtual void set_descriptor_target_00464f70(std::uint32_t entity,
                                               float value) = 0;              // 00817329
    // 008172E9: [00E188A8]+19CCh, read once for the placement call.
    virtual std::uint32_t session_field_19cc() = 0;
};

// The size 00470B80 is asked for at 00817243.
inline constexpr std::uint32_t kEntityCommandThrowawayTargetSize = 0x1E4u;
// The side id the throwaway target is given at 00817322.
inline constexpr int kEntityCommandThrowawaySide = 2;

// 00816EA6..0081732E: __thiscall(unit)(const EntityOrderMessage*) inside
// 00816E30 (body 00816E30-00817376, RET 4 at 00817374 and 00816F62/00817014).
// `command` is what 00816E9C already resolved from the message ordinal, the
// value unit_apply_entity_command_00816e30 calls `command`. `descriptor` is the
// 18h-byte record the prologue copies out of the message at 00816E52..00816E96.
EntityCommandArmDecision entity_command_arm_cascade_00816ea6(
    EntityCommandArmId command, const SceneCommandTarget& descriptor,
    EntityCommandArmsHost& host);

}  // namespace bsp
