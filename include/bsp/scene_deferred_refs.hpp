#pragma once
// Scene deferred references: the entity-command queue the scene database fills
// while it reads a `.scn` file and drains once the instantiate pass has built
// every entity. docs/SCENE_DEFERRED_REFS.md carries the evidence.
//
// Native chain: 004E6B30 (the `"Command"` sub-block) -> 00469610 (queue) ->
// 004690D0 (record) + 00468350 (push) -> 0046AAB0 (resolve) -> 0046A9F0 (clear).
// Names below are hypotheses, not recovered symbols.
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The record (00469627 `operator new(0x14)`, filled by 004690D0, freed by 00469010)
// ---------------------------------------------------------------------------

// Native offsets of the queued record. The two name fields are pooled native
// strings, {length, chars}; 00469010 returns exactly those two blocks.
inline constexpr int kSceneCommandRecordOwnerOffset = 0x00;
inline constexpr int kSceneCommandRecordCommandLengthOffset = 0x04;
inline constexpr int kSceneCommandRecordCommandCharsOffset = 0x08;
inline constexpr int kSceneCommandRecordTargetLengthOffset = 0x0C;
inline constexpr int kSceneCommandRecordTargetCharsOffset = 0x10;
inline constexpr int kSceneCommandRecordSize = 0x14;

// One queued command. `owner` is the opaque native entity the record was queued
// for (record+0h, the `this` of every later host call); the reconstruction never
// dereferences it. Both names are copied by value at queue time, so a later edit
// of the property bag cannot change what is resolved. A null name pointer became
// the empty string 00E18560 at 004690D0, so "" and "absent" are the same thing.
struct SceneCommandRecord {
    void* owner{nullptr};
    std::string command;  // the `Command = E CommandType : <name>` token
    std::string target;   // the `CommandTarget = R "<name>"` value, "" when unset
};

// 004690D0. Mirrors the native constructor, including the empty-string fallback.
SceneCommandRecord scene_command_record_004690d0(void* owner, const char* command,
                                                 const char* target);

// The list at scene database +14Ch: {count +14Ch, head +150h, tail +154h} of
// 12-byte nodes {prev, next, record}. 00468350 pushes at the tail and 0046A9F0
// erases every node front to back, so the drain order is insertion order.
// A vector carries the same observable order; the count is its size.
using SceneCommandQueue = std::vector<SceneCommandRecord>;

inline constexpr int kSceneCommandListHeaderOffset = 0x14C;  // count
inline constexpr int kSceneCommandListHeadOffset = 0x150;
inline constexpr int kSceneCommandListTailOffset = 0x154;

// ---------------------------------------------------------------------------
// The command-type registry (00E19A70) as an injected view
// ---------------------------------------------------------------------------

// One element of the global list the static initialisers at 00CDC6A0..00CDC7E0
// fill. `identity` is the native object (node+8h) and is what 0046AAB0 hands to
// 0077D600. `name` is its vtable[4] (base body 006F7F30 returns "undefined
// command name"); `requires_target` is its vtable[8] (base body 006F7F40 returns
// false). The "requires a target" reading of vtable[8] is provisional: what the
// binary proves is that a true answer drops a record that named no target.
struct SceneCommandType {
    void* identity{nullptr};
    std::string name;
    bool requires_target{false};
};

// The registry as 0046AAB0 walks it: insertion order, first match wins, compared
// case-insensitively through 00438E10. 00467170 is the same scan factored out.
using SceneCommandRegistry = std::vector<SceneCommandType>;

// 00467170 / the inlined scan at 0046AAE7..0046AB11. Returns nullptr when no
// entry matches; a matched entry whose `identity` is null ends the record too
// (0046AB1D), which the caller sees as SceneCommandOutcome::kNullCommandObject.
const SceneCommandType* scene_command_registry_find_00467170(const SceneCommandRegistry& registry,
                                                             const std::string& name) noexcept;

// ---------------------------------------------------------------------------
// The 0x18-byte target descriptor built on 0046AAB0's stack
// ---------------------------------------------------------------------------

// Field order and sizes are fixed by 0077D600, which copies all seven fields.
struct SceneCommandTarget {
    std::uint8_t kind{0};            // +0h: 0 position, 1 object
    std::uint8_t position_valid{0};  // +1h: 1 on the position branch (provisional)
    std::uint16_t object_id{0};      // +2h: target entity +174h, 0 otherwise
    void* object{nullptr};           // +4h: the resolved target entity, null otherwise
    float position[3]{};             // +8h/+Ch/+10h
    float reserved{0.0f};            // +14h: XORPS at 0046ABF8, always 0
};
inline constexpr int kSceneCommandTargetSize = 0x18;

// ---------------------------------------------------------------------------
// The resolution rule
// ---------------------------------------------------------------------------

// Why a record ended. Only kIssued reaches 0077D600. The native reports nothing
// on any other outcome and frees the record with the rest of the queue; the
// enum exists for this reconstruction's own reporting.
enum class SceneCommandOutcome {
    kIssued,
    kUnknownCommandName,    // no registry entry matched (0046AB13)
    kNullCommandObject,     // the matched entry's object is null (0046AB1D)
    kCommandRequiresTarget, // vtable[8] answered true with no target named (0046ABA6)
    kTargetNotFound,        // 00925A90 returned null (0046AB4F)
};

// What one record resolved to. `command` is the matched registry entry, null on
// the two failure outcomes that never find one.
struct SceneCommandResolution {
    SceneCommandOutcome outcome{SceneCommandOutcome::kUnknownCommandName};
    const SceneCommandType* command{nullptr};
    SceneCommandTarget target{};
};

// ---------------------------------------------------------------------------
// Host: one virtual per native call site inside 0046AAB0
// ---------------------------------------------------------------------------

// No method has a default implementation; none of them stands in for behaviour
// that has not been recovered. All entity pointers are the opaque natives.
struct SceneDeferredReferenceHost {
    virtual ~SceneDeferredReferenceHost() = default;

    // 0046ABA2, the matched command object's vtable[8] (ECX = the object).
    // Only reached on the position branch.
    virtual bool command_requires_target(void* command) = 0;

    // 0046ABA8: the owner's pose byte +C8h. False means the pose is stale.
    virtual bool owner_pose_is_current(void* owner) = 0;

    // 0046ABB2: 00414DB0 with ECX = owner. docs/POSE_REFRESH.md owns the body.
    virtual void refresh_owner_pose(void* owner) = 0;

    // 0046ABB7/0046ABC5/0046ABD8: the owner's world position +FCh/+100h/+104h,
    // read after the refresh. Three floats into `out`.
    virtual void owner_world_position(void* owner, float out[3]) = 0;

    // 0046AB48: 00925A90 with ECX = *(*(00E188A8)+19CCh). Null when no entity of
    // that name exists. The empty-string fallback 00E18560 is applied by the rule.
    virtual void* find_entity_by_name(const std::string& name) = 0;

    // 0046AB8D: the resolved target entity's uint16 at +174h.
    virtual std::uint16_t entity_object_id(void* entity) = 0;

    // 0046AC0B: 0077D600(ECX = owner, command, &target, flags). `flags` is the
    // literal 1 pushed at 0046ABFB; no other caller value is known from here.
    virtual void issue_command(void* owner, void* command, const SceneCommandTarget& target,
                               int flags) = 0;

    // 0046AC34: the tail JMP to 0046A9F0 on this+14Ch. Every record is destroyed
    // whatever its outcome, so the queue is empty when the pass returns.
    virtual void clear_queue() = 0;
};

// The pure rule for one record: everything 0046AAB0 decides between finding the
// command and calling 0077D600. The host supplies the pose refresh, the entity
// lookup and the id; the branch selection, the empty-name fallbacks, the
// first-match rule and the descriptor layout are here.
SceneCommandResolution resolve_scene_command_0046aab0(const SceneCommandRecord& record,
                                                      const SceneCommandRegistry& registry,
                                                      SceneDeferredReferenceHost& host);

// What the whole pass did. The native returns nothing; these counters are for
// this reconstruction's own reporting.
struct SceneDeferredResolveStats {
    int records{0};
    int issued{0};
    int unknown_command{0};
    int null_command_object{0};
    int requires_target{0};
    int target_not_found{0};
    bool queue_cleared{false};
};

// 0046AAB0 itself: walk the queue in order, resolve each record, issue the ones
// that resolved, then clear the queue through the tail call. The queue is passed
// by const reference because the native never edits a record in place; the clear
// is the host's job, exactly as the tail JMP is the native's last act.
SceneDeferredResolveStats resolve_scene_deferred_references_0046aab0(
    const SceneCommandQueue& queue, const SceneCommandRegistry& registry,
    SceneDeferredReferenceHost& host);

}  // namespace bsp
