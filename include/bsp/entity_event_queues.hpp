#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/hit_narrowphase.hpp"

// The three entity event drains of the fixed simulation step: fan-out rows 6
// and 14 (00926700, the deferred hit queue), row 15 (009273A0, the two pending
// entity lists) and row 16 (00903610, world expiry). bsp/fixed_step_fanout.hpp
// declares the three host methods this header fills in; docs/ENTITY_EVENT_QUEUES.md
// carries the evidence, and reports/entity_event_queues.json the call rows.
//
// Every descriptive name is a hypothesis, not a recovered symbol.
//
// What is modelled here is the order, the gates and the record layouts. The
// MSVC std::list node churn is not ported: the drains run over host-owned
// storage, because the sentinel ring adds nothing the listing does not state.
// Two properties of that ring are load-bearing and are kept:
//
//   * the deferred queue pops the end its producer appends to, so it dispatches
//     in reverse arrival order (LIFO), while the two pending lists are walked
//     from the other end and dispatch in arrival order (FIFO);
//   * the deferred drain asks the host for the back node twice, once to
//     dispatch and once to pop, exactly as 00926714 and 0092675B do, so a host
//     whose handler queues a new event reproduces the original's loss of it.
//
// SceneNodeFlags, scene_node_remove_009263c0, kHitRecordSize and
// kHitRecordOffDirection come from bsp/hit_narrowphase.hpp and are not
// redeclared.

namespace bsp {

// ---------------------------------------------------------------------------
// The deferred hit-event queue, 00F899C0
//
// One event kind. The 68h-byte node is eight bytes of links and a 60h-byte
// value: the 54h-byte hit record of docs/HIT_NARROWPHASE.md with the impact
// direction appended. 00926691 PUSH 0x68 is the node size; 00926F48 writes the
// first direction float at value+54h.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDeferredEventNodeOffLinkForward = 0x00;  // 009266AE, 00926780
inline constexpr std::size_t kDeferredEventNodeOffLinkBack = 0x04;     // 009266BA, 00926714
inline constexpr std::size_t kDeferredEventNodeOffValue = 0x08;        // 00926F3B ADD ESI,8
inline constexpr std::size_t kDeferredEventNodeSize = 0x68;            // 00926691
inline constexpr std::size_t kDeferredEventValueSize = 0x60;           // 0x68 - 8
inline constexpr std::size_t kDeferredEventNodeOffDirection = 0x5C;    // 0092674B LEA EDX,[EAX+0x54]

// The list object itself: 00F899C0, sentinel at +4h, size at +8h.
inline constexpr std::uint32_t kDeferredEventListAddress = 0x00F899C0u;
inline constexpr std::size_t kListOffSentinel = 0x04;  // 00F899C4, 00F899AC, 00F899B8
inline constexpr std::size_t kListOffSize = 0x08;      // 00F899C8, 00F899B0, 00F899BC

// Set by the producer at 00926ED4, cleared by the drain at 009267BB, read by
// nothing: those two writes are the byte's only references in the image.
inline constexpr std::uint32_t kDeferredEventPendingFlagAddress = 0x00E18684u;

// 0092673F and 00926745. The subject is the record's +0h field, the hit entity.
bool deferred_event_should_dispatch(bool subject_present, const SceneNodeFlags& subject) noexcept;

// One back() result. `record` is node+8h, `direction` is node+5Ch, `subject` is
// the first dword of the record.
struct DeferredEventBack {
    void* node{nullptr};
    void* record{nullptr};
    void* direction{nullptr};
    void* subject{nullptr};
    SceneNodeFlags subject_flags{};
};

struct DeferredEntityEventHost {
    virtual ~DeferredEntityEventHost() = default;

    // [00F899C8], the loop condition at 00926700 and 009267AD.
    virtual std::size_t deferred_queue_size() = 0;

    // 00926714 and again at 0092675B: list.back(). Called twice per iteration
    // on purpose; see the header comment.
    virtual DeferredEventBack deferred_queue_back() = 0;

    // 00926750, 009239A0, __thiscall(subject, record, direction), the hit
    // dispatch that walks the parent chain through vtable[ECh]
    // (docs/PROJECTILE_IMPACT.md). A contract: not read by this packet.
    virtual void dispatch_hit_009239a0(void* subject, void* record, void* direction) = 0;

    // 0092675B..009267A6: unlink the node the second back() returned, destroy
    // its record with 004704B0, free it, and drop the size by one.
    virtual void deferred_queue_pop_back_and_destroy(const DeferredEventBack& back) = 0;

    // 00926ED4 (true, in the producer) and 009267BB (false, on every exit).
    virtual void set_deferred_pending_flag_00e18684(bool pending) = 0;
};

// 00926700, __cdecl void(void). Fan-out rows 6 (00875E44) and 14 (00875EC4).
void drain_deferred_entity_events_00926700(DeferredEntityEventHost& host);

// ---------------------------------------------------------------------------
// The two pending entity lists, 00F899A8 (destroy) and 00F899B4 (kill)
//
// 0Ch nodes: two links and one entity pointer at +8h (00924B10 PUSH 0xC,
// 00927477 MOV ECX,[ESI+8]).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kPendingEntityNodeOffEntity = 0x08;  // 00927477
inline constexpr std::size_t kPendingEntityNodeSize = 0x0C;       // 00924B10

inline constexpr std::uint32_t kPendingDestroyListAddress = 0x00F899A8u;  // 00926D49
inline constexpr std::uint32_t kPendingKillListAddress = 0x00F899B4u;     // 00926E3B

// The producers' flag gates, 00926CBF and 00926DE1. Both are idempotent: the
// entity is queued only on the first call.
inline constexpr std::size_t kMissionEntityOffDestroyedFlag = 0x60;  // 00926CD5
inline constexpr std::size_t kMissionEntityOffObservers = 0x08;      // 009274F5
inline constexpr std::size_t kMissionEntityOffNextInWorld = 0x38;    // 00903662
inline constexpr std::size_t kMissionEntityOffFirstChild = 0x48;     // 00903638
inline constexpr std::size_t kMissionEntityOffChildCount = 0x50;     // 00903632
inline constexpr std::size_t kMissionEntityOffExpiryCounter = 0x6C;  // 00903620

// 00926D90: the cause 7 becomes 2, every other value passes through.
inline constexpr int kMissionEntityKillCauseRemap = 7;         // 00926DAD
inline constexpr int kMissionEntityKillCauseRemapped = 2;      // 00926DB5
int mission_entity_kill_cause_00926d90(int cause) noexcept;

struct PendingEntityQueueHost {
    virtual ~PendingEntityQueueHost() = default;

    // [00F899B0] and [00F899BC], the outer loop's exit test at 009273C2.
    virtual std::size_t pending_destroy_count() = 0;
    virtual std::size_t pending_kill_count() = 0;

    // 009273DF and 009273F1, 00926FA0: copy each global list into frame-local
    // storage. Both copies happen before either clear.
    virtual void copy_pending_lists_00926fa0() = 0;

    // 009273F6 and 00927435: reset both sentinels and sizes in place and free
    // the old nodes. No value destructor runs; the value is a raw pointer.
    virtual void clear_pending_lists() = 0;

    // The copies, in arrival order: the walk starts at the end opposite the
    // producers' appends (00927471, 00927494).
    virtual std::size_t destroy_copy_count() = 0;
    virtual void* destroy_copy_at(std::size_t index) = 0;
    virtual std::size_t kill_copy_count() = 0;
    virtual void* kill_copy_at(std::size_t index) = 0;

    // 0092747F, entity->vtable[74h](). For MDestroyer that is 00926390, which
    // sets +5Dh and +60h, notifies observers through 00925C90 and tail-calls
    // vtable[7Ch].
    virtual void on_entity_destroyed_74h(void* entity) = 0;

    // 009274A8 and again at 009274BB, entity->vtable[18h](): the render handle
    // of docs/CONTROLLED_UNIT.md. The native calls it twice; so does the drain.
    virtual void* render_handle_18h(void* entity) = 0;

    // 009274B3, [00E188DC].
    virtual void* controlled_unit_handle_00e188dc() = 0;

    // 009274C3, 004BCA80 with ECX = 0: stores 0 into 00E188DC and calls
    // 00B0D7B0. The controlled unit lets go of the dying entity.
    virtual void clear_controlled_unit_handle_004bca80() = 0;

    // The entity's +5Ch..+5Fh bytes, read at 009274C8 and written at
    // 009274CE..009274DA.
    virtual SceneNodeFlags scene_node_flags(void* entity) = 0;
    virtual void store_scene_node_flags(void* entity, const SceneNodeFlags& flags) = 0;

    // 009274F5 under the 00694280 observer lock: [entity+8h] != 0. The lock is
    // released before the notification, so the sample is what decides.
    virtual bool sample_has_observers_00925c40(void* entity) = 0;

    // 00927510, 00696330.
    virtual void notify_observers_00696330(void* entity) = 0;

    // 0092751F, entity->vtable[80h](): the on-killed hook. For a unit that is
    // 00951FB0, which clears [this+4A4h] and jumps to 00779AF0; the base is
    // 00928C80.
    virtual void on_entity_killed_80h(void* entity) = 0;
};

// 0092749E..00927521, the per-entity block of pass K. The same six steps are
// 009263C0, which no reference in the image reaches.
void remove_killed_entity_009263c0(PendingEntityQueueHost& host, void* entity);

// 009273A0, __cdecl void(void). Fan-out row 15 (00875EC9).
void flush_pending_entity_queues_009273a0(PendingEntityQueueHost& host);

// ---------------------------------------------------------------------------
// World expiry, 00903610
//
// The counter at +6Ch is set to 1 by 00922FD0 and aged once per pass. Three is
// the release value, so an entity is freed on the second pass after marking.
// ---------------------------------------------------------------------------
inline constexpr int kEntityExpiryMarkedValue = 1;   // 00922FE8
inline constexpr int kEntityExpiryReleaseValue = 3;  // 0090362A CMP EAX,0x3

enum class EntityExpiryAction : std::uint8_t {
    kSkip,     // 00903625 JLE: the counter is zero or negative, nothing happens
    kAge,      // 00903630 JL: kept, and this entity becomes the resume anchor
    kRelease,  // 00903632: children through 009035E0, then vtable[0](1)
};

struct EntityExpiryStep {
    EntityExpiryAction action{EntityExpiryAction::kSkip};
    int counter{0};  // the value stored back at 0090362D; unchanged when kSkip
};

EntityExpiryStep entity_expiry_step_00903620(int counter) noexcept;

struct WorldExpiryHost {
    virtual ~WorldExpiryHost() = default;

    // 00903613, [[world+4h]]: the head of the world node's child chain, the
    // one 00904BF0 walks (docs/GAME_WORLD_ENTITIES.md).
    virtual void* world_chain_head() = 0;

    // 00903662 and 00903654, entity+38h.
    virtual void* next_in_world_chain(void* entity) = 0;

    // 00903620 and 0090362D, entity+6Ch.
    virtual int expiry_counter(void* entity) = 0;
    virtual void store_expiry_counter(void* entity, int counter) = 0;

    // 00903632 and 00903638: entity+50h and entity+48h.
    virtual std::size_t child_count(void* entity) = 0;
    virtual void* first_child(void* entity) = 0;

    // 0090364E and 00903606, entity->vtable[0](1): the scalar deleting
    // destructor. What it unlinks is a contract, not read by this packet.
    virtual void destroy_entity_vtable0(void* entity) = 0;
};

// 009035E0, __thiscall(node): destroy every child subtree, deepest first, then
// the node. The loop depends on the destructor decrementing the parent's +50h.
void destroy_child_subtree_009035e0(WorldExpiryHost& host, void* node);

// 00903610, __thiscall(world). Fan-out row 16 (00875EDA).
void release_expired_world_objects_00903610(WorldExpiryHost& host);

}  // namespace bsp
