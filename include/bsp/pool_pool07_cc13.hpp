#pragma once
// Packet cc13-pool07 (segment 34): the ConvexObject payload array readers
// called by 006FAD70 BSP_ConvexObject_ReadPayload, and the Cruiser unit's
// constructor, creator and world-list unlink. Names are hypotheses, not
// recovered symbols. Objects are raw image storage; every routine the image
// calls is a host method invoked at the same point (operator new, memset and
// the EH vector constructor iterator are library code, bound with no credit).
// Build-only: nothing here is wired into a live host path.
#include <cstddef>
#include <cstdint>

namespace bsp::pool_pool07_cc13 {

struct Pool07Host {
    virtual ~Pool07Host() = default;

    // ---- library (bound) ----
    // 00BF55BE operator new, cdecl(bytes) -> EAX. 00BF79F0 memset, cdecl(dst, byte, count).
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void memset_00bf79f0(void* destination, std::int32_t value, std::uint32_t count) = 0;
    // 00BF7CD1 `eh vector constructor iterator', __stdcall(first, size, count, ctor, dtor) RET 14h;
    // callbacks by image address.
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
                                                         std::uint32_t ctor, std::uint32_t dtor) = 0;

    // ---- structured-node readers ----
    // 00BE9A80 BSP_StructuredNode_ReadU32ToOutParam: __thiscall(ECX = node; u32* out), RET 4.
    virtual void node_read_u32_00be9a80(void* node, std::uint32_t* out) = 0;
    // 00BE9A60 BSP_StructuredNodeHandle_ReadFloat: __thiscall(ECX = node; float* out), RET 4;
    // EAX = the node the next read is made on.
    virtual void* node_read_float_00be9a60(void* node, float* out) = 0;
    // 006F9AC0 BSP_ConvexObject_ReadU16Array: __fastcall(ECX = node, EDX = u16** out; u32* count),
    // RET 4 (built in src/cc13_seg33b_misc.cpp).
    virtual void convex_read_u16_array_006f9ac0(void* node, std::uint16_t** out, std::uint32_t* count) = 0;
    // 006F9B20 BSP_ConvexObject_ReadRecord14: __fastcall(ECX = node, EDX = record), RET; EAX = node
    // (built in src/cc13_seg33b_misc.cpp).
    virtual void* convex_read_record14_006f9b20(void* node, void* record) = 0;

    // ---- units ----
    // 0081ED40 BSP_UnitVehicleBase_Construct: __thiscall(ECX = unit; arg), RET 4.
    virtual void unit_vehicle_base_construct_0081ed40(void* unit, void* arg) = 0;
    // 009553D0: __thiscall(ECX = unit; class), RET 4 (sets the vehicle class).
    virtual void unit_set_vehicle_class_009553d0(void* unit, void* vehicle_class) = 0;
    // 006DFFC0 BSP_UnitInstance_DetachFromMidWorldLists: __fastcall(ECX = unit), RET.
    virtual void unit_detach_mid_world_lists_006dffc0(void* unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode: __thiscall(ECX = list; node), RET 4.
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;
};

// 006FA7F0 __fastcall(ECX = node, EDX = out array cell; u32* count), RET 4.
void convex_read_hull_arrays_006fa7f0(void* node, void** out, std::uint32_t* count, Pool07Host& host);
// 006FA910 __fastcall(ECX = node, EDX = out array cell; u32* count), RET 4.
void convex_read_hull_arrays2_006fa910(void* node, void** out, std::uint32_t* count, Pool07Host& host);
// 006FAA20 __fastcall(ECX = node, EDX = out array cell; u32* count), RET 4.
void convex_read_vec3_array_006faa20(void* node, void** out, std::uint32_t* count, Pool07Host& host);
// 006FACE0 __fastcall(ECX = node, EDX = out array cell; u32* count), RET 4.
void convex_read_record14_array_006face0(void* node, void** out, std::uint32_t* count, Pool07Host& host);
// 006FB300 __thiscall(ECX = unit; arg), RET 4; EAX = unit.
void* cruiser_construct_006fb300(void* unit, void* arg, Pool07Host& host);
// 006FB430 __thiscall(ECX = vehicle class; arg), RET 4; EAX = the new unit.
void* cruiser_create_006fb430(void* vehicle_class, void* arg, Pool07Host& host);
// 006FB510 __fastcall(ECX = unit), RET.
void cruiser_detach_from_world_lists_006fb510(void* unit, Pool07Host& host);

}  // namespace bsp::pool_pool07_cc13
