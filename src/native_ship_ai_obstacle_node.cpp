#include "bsp/native_ship_ai_obstacle_node.hpp"
#include "bsp/observer_edges.hpp"

namespace bsp {

NativeShipAiObstacleNodeStorage* construct_native_ship_ai_obstacle_009e52e0(
    NativeShipAiObstacleNodeStorage& node, NativeObserverOwnerStorage* const owner,
    void* const controller, const std::uint32_t lifetime_bits,
    NativeObserverLifetime& lifetime) {
    node.callback_owner_00.edges_04.data_00 = nullptr;
    node.callback_owner_00.edges_04.count_04 = 0;
    node.callback_owner_00.edges_04.capacity_08 = 0;
    node.owner_14 = nullptr;
    node.enabled_10 = 1;
    node.callback_owner_00.native_vtable_00 = kNativeShipAiObstacleVtable00cf5c94;
    node.zero_18 = 0;
    try {
        if (owner) {
            node.owner_14 = owner;
            register_observer_pair_00694a60(*owner, node.callback_owner_00, lifetime);
        }
    } catch (...) {
        // CB0820 loads saved node and jumps via0064A8D0 to0064A610.
        destroy_native_ship_ai_obstacle_base_0064a610(node, lifetime);
        throw;
    }
    node.controller_1c = controller;
    node.lifetime_bits_78 = owner ? lifetime_bits : 0xbf800000u;
    node.no_arc_69 = 0;
    node.pass_side_88 = 0;
    node.flag_75 = 0;
    node.zero_8c = 0;
    node.flag_74 = 0;
    node.no_pose_68 = 1;
    node.cache_bits_7c = 0;
    node.minimum_y_bits_84 = 0xc47a0000u; // exact00D7A240 word
    node.maximum_y_bits_80 = 0xc47a0000u;
    node.cache_bits_70 = 0;
    return &node;
}

void destroy_native_ship_ai_obstacle_base_0064a610(
    NativeShipAiObstacleNodeStorage& node, NativeObserverLifetime& lifetime) {
    node.callback_owner_00.native_vtable_00 = 0x00cf5c20;
    try {
        if (auto* const owner = node.owner_14) {
            lifetime.unregister_pair_006952a0(*owner, node.callback_owner_00);
        }
    } catch (...) {
        // C7A930 invokes00695870 while native unwind state is zero.
        lifetime.destroy_callback_owner_00695870(node.callback_owner_00);
        throw;
    }
    lifetime.destroy_callback_owner_00695870(node.callback_owner_00);
}

NativeShipAiObstacleNodeStorage* delete_native_ship_ai_obstacle_0064b5f0(
    NativeShipAiObstacleNodeStorage* const node, const std::uint32_t flags,
    NativeObserverLifetime& lifetime) {
    destroy_native_ship_ai_obstacle_base_0064a610(*node, lifetime);
    if ((flags & 1u) != 0) singleton_lifetime_free(node);
    return node;
}

void native_ship_ai_obstacle_owner_callback_0064b5c0(
    NativeShipAiObstacleNodeStorage& node, void* const event,
    const NativeShipAiObstacleEventAccess& access, NativeObserverLifetime& lifetime) {
    auto* const first = access.event_virtual_04(access.context, event);
    if (first == node.owner_14) {
        node.owner_14 = nullptr;
        lifetime.unregister_pair_006952a0(*first, node.callback_owner_00);
    }
}

} // namespace bsp
