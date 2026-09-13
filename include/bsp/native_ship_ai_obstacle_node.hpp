#pragma once

#include "bsp/observer_lifetime.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual 90h storage, distinct from the semantic ShipAiObstacleNode. There are
// no default values: the constructor preserves every byte of the four holes.
// In particular, geometry_20 is allocation preimage, not valid default geometry.
struct NativeShipAiObstacleNodeStorage {
    NativeObserverOwnerStorage callback_owner_00;
    std::uint8_t enabled_10;
    std::uint8_t untouched_11[3];
    NativeObserverOwnerStorage* owner_14;
    std::uint32_t zero_18;
    void* controller_1c;
    std::uint8_t geometry_20[0x48];
    std::uint8_t no_pose_68;
    std::uint8_t no_arc_69;
    std::uint8_t untouched_6a[6];
    std::uint32_t cache_bits_70;
    std::uint8_t flag_74;
    std::uint8_t flag_75;
    std::uint8_t untouched_76[2];
    std::uint32_t lifetime_bits_78;
    std::uint32_t cache_bits_7c;
    std::uint32_t maximum_y_bits_80;
    std::uint32_t minimum_y_bits_84;
    std::int32_t pass_side_88;
    std::uint32_t zero_8c;
};
static_assert(sizeof(NativeShipAiObstacleNodeStorage) == 0x90);
static_assert(alignof(NativeShipAiObstacleNodeStorage) == 4);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, callback_owner_00) == 0);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, enabled_10) == 0x10);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, owner_14) == 0x14);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, zero_18) == 0x18);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, controller_1c) == 0x1c);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, geometry_20) == 0x20);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, no_pose_68) == 0x68);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, cache_bits_70) == 0x70);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, lifetime_bits_78) == 0x78);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, minimum_y_bits_84) == 0x84);
static_assert(offsetof(NativeShipAiObstacleNodeStorage, zero_8c) == 0x8c);

inline constexpr std::uint32_t kNativeShipAiObstacleVtable00cf5c94 = 0x00cf5c94;

// Complete normal 009E52E0..009E53A5; original ECX=storage, stack(owner,
// controller,float bits), EAX=storage, RET0Ch. Existing allocator caller owns
// the exact90h allocation; this function neither allocates nor zero-fills it.
// Registration sees owner+14 set and controller/tail still untouched. Actual
// observer endpoints, global lifetime domain and geometry preimage are required.
// Raw lifetime bits preserve MOVSS even for signaling NaNs. A C++ exception in
// registration runs the evidenced base cleanup, then propagates; native FH3,
// SEH faults and binary replacement ABI are not supplied by this interface.
NativeShipAiObstacleNodeStorage* construct_native_ship_ai_obstacle_009e52e0(
    NativeShipAiObstacleNodeStorage&, NativeObserverOwnerStorage* owner,
    void* controller, std::uint32_t lifetime_bits, NativeObserverLifetime&);

// Complete normal 0064A610..0064A667, ECX=base, RET. This common destructor is
// also used by other native classes; this API accepts only the raw node above.
// Writes base table CF5C20, unregisters a nonnull owner, destroys callback base.
// Does not clear owner or free the90h backing. A C++ unregister exception runs
// the callback-base cleanup as the native unwind funclet does, then propagates.
void destroy_native_ship_ai_obstacle_base_0064a610(
    NativeShipAiObstacleNodeStorage&, NativeObserverLifetime&);

// CF5C94 slot zero, existing CG_scalar_deleting_dtor_0064b5f0, RET4. Calls the
// common destructor, frees backing through the existing CRT boundary iff
// flags&1, then returns its original identity (which must not be dereferenced
// after deletion). Caller owns storage lifetime and must not delete twice.
NativeShipAiObstacleNodeStorage* delete_native_ship_ai_obstacle_0064b5f0(
    NativeShipAiObstacleNodeStorage*, std::uint32_t flags, NativeObserverLifetime&);

// Actual event storage and slot+4 dispatch remain borrowed: this packet has no
// event producer or complete callable vtable. The provider must invoke that
// event's real slot once, retaining its side effects and returned identity.
struct NativeShipAiObstacleEventAccess {
    void* context;
    NativeObserverOwnerStorage* (*event_virtual_04)(void* context, void* event);
};
// CF5C94 slot+4, complete 0064B5C0..0064B5E6, ECX=node, stack=event, RET4.
// Reloads node.owner after dispatch. Equality clears it before unregistering;
// there is no extra null guard, filtering, event construction or node deletion.
void native_ship_ai_obstacle_owner_callback_0064b5c0(
    NativeShipAiObstacleNodeStorage&, void* event,
    const NativeShipAiObstacleEventAccess&, NativeObserverLifetime&);

} // namespace bsp
