#include "bsp/native_zone_group_clear.hpp"

#include <cstddef>

namespace bsp {

static_assert(sizeof(Cc13TerrainGridLayerListNode) == 0x0c);
static_assert(offsetof(Cc13TerrainGridLayerListNode, next) == 0x00);
static_assert(offsetof(Cc13TerrainGridLayerListNode, previous) == 0x04);
static_assert(offsetof(Cc13TerrainGridLayerListNode, value) == 0x08);
static_assert(offsetof(Cc13AvoidZoneRegistryNative, head) == 0x08);
static_assert(offsetof(Cc13AvoidZoneRegistryNative, size) == 0x0c);

void native_avoid_zone_registry_clear_layers_0041ded0(
    Cc13AvoidZoneRegistryNative& registry, NativeAvoidZoneRegistryClearHost& host) {
    while (registry.size != 0) { // 0041DED3..0041DED7, 0041DF2C..0041DF30
        Cc13TerrainGridLayerListNode* node = registry.head->next;
        if (node == registry.head) {
            host.invalid_parameter_00bf6713(); // 0041DEE9; may return
        }
        if (node->value != nullptr) {
            host.scalar_delete_layer_vtable_0(*node->value, 1); // 0041DEFB
        }

        // The destructor is an external call: reload both sentinel and front.
        node = registry.head->next; // 0041DEFD..0041DF00
        if (node == registry.head) {
            host.invalid_parameter_00bf6713(); // 0041DF06; may return
        }
        if (node != registry.head) { // 0041DF0B..0041DF0E, after the handler
            node->previous->next = node->next;
            node->next->previous = node->previous; // 0041DF10..0041DF1D
            host.free_list_node_00bf65ac(node); // 0041DF20
            --registry.size; // 0041DF28, after the external free call
        }
    }
}

} // namespace bsp
