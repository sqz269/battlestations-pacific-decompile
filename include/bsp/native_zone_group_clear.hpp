#pragma once

#include <cstdint>

#include "bsp/cc13_avoid1.hpp"

namespace bsp {

// 0041DED0 owns the registry's TerrainGridLayer list, not the manager's zone
// group array. The existing cc13 record declarations are the shared schema.
// This complete reconstruction has a new C++ interface; it is not an original
// ABI entry or a game binding. See docs/GOAL15_ZONE_GROUP_CLEAR.md.
class NativeAvoidZoneRegistryClearHost {
public:
    virtual ~NativeAvoidZoneRegistryClearHost() = default;

    // 0041DEFB: ECX = layer; PUSH 1; CALL [layer.vtable+0]. Dispatch through
    // the supplied object's vtable. The producer's 00CE38CC vtable selects
    // 0041F640, which releases grid storage and the object when flags&1.
    // The callback may change registry/list state; the caller reloads the head.
    virtual void scalar_delete_layer_vtable_0(
        Cc13TerrainGridLayerNative& layer, std::uint32_t flags) = 0;

    // 0041DF20: actual _free service, __cdecl(node), ADD ESP,4 at 0041DF25.
    // Release the unlinked node with the allocator that supplied it.
    virtual void free_list_node_00bf65ac(Cc13TerrainGridLayerListNode* node) = 0;

    // 0041DEE9 / 0041DF06: void __cdecl(), no noreturn promise. A registered
    // invalid-parameter handler can return; preserve subsequent native work.
    virtual void invalid_parameter_00bf6713() = 0;
};

// Native input: ECX = registry, no stack arguments; plain RET at 0041DF34.
// Deletes each non-null front payload, reloads the current front, unlinks and
// frees that node, and decrements the current count. A zero count does not
// access the sentinel. The sentinel, list proxy, registry vtable and cached
// maximum cell height remain allocated/preserved by this function.
//
// The caller/host must keep each pointer valid until its native last use.
// Returning invalid handlers retain the original unchecked consequences if
// they leave malformed count/link/value state; no repair policy is added.
void native_avoid_zone_registry_clear_layers_0041ded0(
    Cc13AvoidZoneRegistryNative& registry, NativeAvoidZoneRegistryClearHost& host);

} // namespace bsp
