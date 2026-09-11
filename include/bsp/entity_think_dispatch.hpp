#pragma once
// Entity think dispatch: the fixed-step pass that runs each entity's script think
// function. Native routine 00929460 (__cdecl void(float), RET 4, body
// 00929460-009295AB), its call helper 00929150 and the two list primitives 00928380
// and 00928330. docs/ENTITY_THINK_DISPATCH.md carries the evidence.
//
// Nothing here is a binary-compatible replacement: the native lists are intrusive
// heap nodes and the think call goes through the mission Lua host. This projects the
// node layout, the due rules and the call order onto an injected host.
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/lua_binding_core.hpp"

namespace bsp {

// The 0Ch-byte list object. Two instances exist: the live think list at 00F89AB0 and
// the pending-registration list at 00F89ABC. Offsets settled by the producers
// 00928380 (reads +4h as the head) and 0088A240 (writes all three).
inline constexpr std::size_t kThinkListCountOffset = 0x0; // 00929524
inline constexpr std::size_t kThinkListHeadOffset = 0x4;  // 00929466
inline constexpr std::size_t kThinkListTailOffset = 0x8;  // 0092951E

// The 0Ch-byte node, allocated at 00928396 (PUSH 0xC) and 0088A240.
inline constexpr std::size_t kThinkNodePrevOffset = 0x0;   // 009294FC
inline constexpr std::size_t kThinkNodeNextOffset = 0x4;   // 0092948B
inline constexpr std::size_t kThinkNodeEntityOffset = 0x8; // 00929484
inline constexpr std::size_t kThinkNodeSize = 0xC;

// Entity fields the walk reads. +1D8h and +1DCh are declared in lua_binding_core.hpp
// as kEntityThinkScriptNameOffset and kEntityThinkStateByteOffset and are reused, not
// redeclared. +1DCh is the delay-armed flag: the SetThink callee 0088A330 clears it at
// 0088A37F and the delay binding 00898150 sets it at 008982D1.
inline constexpr std::size_t kEntityThinkDelayOffset = 0x1E0;      // 009294B4
inline constexpr std::size_t kEntityInitialisedFlagOffset = 0x5C;  // 00929487, written 1 at 00922F4B
inline constexpr std::size_t kEntityThinkBlockFlagOffsetB = 0x5D;  // 00929490, meaning unread
inline constexpr std::size_t kEntityThinkBlockFlagOffsetC = 0x5E;  // 0092949C, meaning unread
inline constexpr std::size_t kEntityThinkBlockFlagOffsetD = 0x60;  // 00929496, meaning unread
inline constexpr std::size_t kEntityLuaSelfKeyOffset = 0x178;      // 009290A0, the named call's self key

// 00D7A2B0, a double (00929557 FADD double ptr), bytes 00 00 00 00 00 00 08 40.
inline constexpr double kScriptCountdownPeriodSeconds = 3.0;
// 00CE3800, the floor the delay binding clamps to at 008982B0.
inline constexpr float kMinimumThinkDelaySeconds = 0.5f;
// The chunk 00929460 runs when the countdown expires; text at 00D19480, mode literal
// 2 pushed at 0092957D into BSP_LuaMachine_RunString (006B8AD0, RET 10h).
inline constexpr const char* kCollectGarbageChunk = "collectgarbage()";
inline constexpr int kCollectGarbageRunMode = 2;

// The mode byte 00874D00 takes in CL. Non-zero skips the think list entirely and
// zeroes every step argument; the three Lua bindings that can run inside a script
// (00944D8F, 0094530F, 00896969) all pass it.
inline constexpr std::uint8_t kExtraStepSettleOnly = 1;
inline constexpr std::uint8_t kExtraStepFull = 0;

// One node of either list. The native node holds the entity pointer and nothing else;
// the name, the flag and the delay all live on the entity, which is why 00928380 can
// rebuild a node from *(src+8h) alone.
struct EntityThinkNode {
    std::uint32_t entity{0}; // +8h, the ECX of every think call
};

// The list object. The vector stands in for the prev/next chain; order is the native
// order, because every insertion is a tail append (0088A240, 009283C7).
struct EntityThinkList {
    std::vector<EntityThinkNode> nodes{};
    std::size_t count() const noexcept { return nodes.size(); } // +0h
};

// The entity fields the walk tests, in test order (00929487..009294B4).
struct EntityThinkFields {
    std::uint32_t entity{0};
    bool initialised{false};   // +5Ch, must be set
    bool blocked_5d{false};    // +5Dh, must be clear
    bool blocked_60{false};    // +60h, must be clear
    bool blocked_5e{false};    // +5Eh, must be clear
    bool has_think_name{false}; // +1D8h non-null
    bool delay_armed{false};   // +1DCh
    float delay_seconds{0.0f}; // +1E0h
};

// 00929487-009294A9. A false result sends the node to the erase path at 009294FC.
bool entity_think_entry_eligible(const EntityThinkFields& fields) noexcept;

// What the walk does with one eligible entity.
enum class EntityThinkBranch : std::uint8_t {
    Erase,      // 009294FC: ineligible, unlink and free
    TimedWait,  // 009294C7: delay decremented, still above zero
    TimedFire,  // 009294E5: delay reached zero this call
    UntimedWait, // 009294EC: countdown has not expired
    UntimedFire, // 009294F5: countdown expired
};

// The branch for one entity given the already-decremented countdown. `countdown` is
// 00F89A04 after step 1 of the routine, never before: 00929477 stores it before the
// loop is entered at 0092947D.
EntityThinkBranch entity_think_branch(const EntityThinkFields& fields,
                                      float step,
                                      float countdown) noexcept;

// 009294C7-009294D7. The new value of +1E0h on the timed branch. The routine never
// re-arms the delay, so once this returns a value at or below zero the entity takes
// the untimed branch until a script calls the binding at 00898150 again.
float entity_think_delay_after_step(float delay_seconds, float step) noexcept;

// 00929460 step 1 (00929460-00929477) and step 3 (0092954B-0092955D). The refill is
// additive, not a reset, so the phase of the three-second pass is preserved.
float script_countdown_after_step(float countdown, float step) noexcept;
bool script_countdown_expired(float countdown) noexcept; // 009294EC and 00929542
float script_countdown_refilled(float countdown) noexcept;

// 008982B0-008982D1: the delay binding clamps its Lua argument to kMinimumThinkDelay.
float clamp_think_delay(float requested_seconds) noexcept;

// Integration boundary. One virtual per native call site of 00929460 and 00929150, in
// execution order. There are no default implementations: nothing here stands in for
// unrecovered game behaviour.
struct EntityThinkHost {
    virtual ~EntityThinkHost() = default;

    // 00929150 through the two sites 009294E5 and 009294F5, ECX = the entity. The
    // helper builds a NativeString from entity+1D8h (0041E870 at 0092917A) and calls
    // 009290A0(entity, &name, 0, 0, -1) at 00929194, which forwards
    // BSP_MissionLuaHost_CallNamedThreadSafe(entity+178h, name, 0, 0, -1). The think
    // function receives no arguments; the entity is reached through the self key.
    virtual void run_entity_think_00929150(std::uint32_t entity) = 0;

    // 0092952C, __cdecl _free(node) with ADD ESP,4 at 00929531. The walk continues
    // with the successor captured at 0092948B.
    virtual void free_think_node_0092952c(const EntityThinkNode& node) = 0;

    // 00929568, [0109CEFC]->vtable[+0Ch](), byte result tested at 0092956A. The
    // predicate's body was not read, so this is named by slot and not by a verb.
    virtual bool gc_gate_predicate_0109cefc_vtable0c() = 0;

    // 00929588, BSP_LuaMachine_RunString(machine, "collectgarbage()", 0, 0, 2) on
    // *(*(00E188A8)+1A08h)+4h, the mission Lua host's machine.
    virtual void lua_run_string_006b8ad0(const char* chunk, int mode) = 0;

    // 00929597, 00928380(this = 00F89AB0, src = 00F89ABC), RET 4. Appends a new live
    // node per pending entity; the pending nodes themselves are not reused.
    virtual void splice_pending_into_live_00928380(EntityThinkList& live,
                                                   const EntityThinkList& pending) = 0;

    // 009295A1, 00928330(this = 00F89ABC), RET. The listing gap 0092836C-00928373
    // (83 C4 04 83 3E 00 75 C4) makes this a loop, so it frees every pending node.
    virtual void clear_pending_00928330(EntityThinkList& pending) = 0;
};

// The observable result of one call, for a host that wants to assert the order.
struct EntityThinkRunSummary {
    std::size_t thinks_run{0};
    std::size_t nodes_erased{0};
    std::size_t pending_spliced{0};
    bool countdown_expired{false};
    bool garbage_collected{false};
};

// 00929460, __cdecl void(float), RET 4. `countdown` is 00F89A04 and is updated in
// place. `live` is 00F89AB0 and `pending` is 00F89ABC. `fields` supplies the entity
// state the native walk reads through the node's +8h pointer; an entity missing from
// the map is treated as ineligible, which is the native behaviour for a cleared +5Ch.
EntityThinkRunSummary run_entity_think_list_00929460(
    float step,
    float& countdown,
    EntityThinkList& live,
    EntityThinkList& pending,
    const std::vector<EntityThinkFields>& fields,
    EntityThinkHost& host);

// 0088A240 through 0088A34B: the SetThink callee appends to the pending list only on
// the null-to-name transition, so an entity joins the live list at most once.
void register_pending_think_entity_0088a240(EntityThinkList& pending,
                                            std::uint32_t entity);

// 00928300 with 00928030: linear search by payload, unlink, count -= 1, free. Used by
// the entity death path 00929800 at 00929AA7 and 00929AB2 against both lists.
bool erase_think_entity_00928300(EntityThinkList& list, std::uint32_t entity) noexcept;

} // namespace bsp
