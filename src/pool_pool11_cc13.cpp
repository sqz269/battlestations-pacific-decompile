// Pool packet pool11 (segment 6). See include/bsp/pool_pool11_cc13.hpp for the model. Disk
// listings were read whole; each body carries its address, original ABI, EH states and evidence.

#include "bsp/pool_pool11_cc13.hpp"

namespace bsp::pool_pool11_cc13 {

using cc13_seg33a::add;
using cc13_seg33a::at;
using cc13_seg33a::ptr;

namespace {

using EraseRange = void (Pool11Host::*)(void*, void*, void*, void*, void*, void*);
using BuyHead = void* (Pool11Host::*)(void*);

// The shared shape of the three registry destructor bodies (004755B0, 00479810, 0047B260):
// __fastcall(ECX = registry), RET; SEH frame, state 0 while the member tree at +4 is torn down.
// tree = registry+4 (allocator byte +4, head +8, size +0Ch of the registry). The tree's erase
// (out, {tree, [head]}, {tree, head}) clears every node: [head] is the leftmost node, so this is
// erase(begin(), end()). Then free(head), head = 0, size = 0, the lazy singleton global = 0, and
// the vtable is reset to the registry base 00CE3818. No base destructor call follows.
void registry_destruct_body(Pool11Host& host, void* registry, EraseRange erase, std::uint32_t singleton) {
    void* const tree = add(registry, 4);
    void* const head = ptr(tree, 4);
    void* out[2]{};  // the returned iterator, discarded
    (host.*erase)(tree, out, tree, ptr(head, 0), tree, head);
    host.free_00bf65ac(ptr(tree, 4));
    at<void*>(tree, 4) = nullptr;
    at<std::uint32_t>(tree, 8) = 0;
    host.global_pointer(singleton) = nullptr;
    at<std::uint32_t>(registry, 0) = kRegistryBaseVtable_00ce3818;
}

// The shared shape of the two registry constructors (004797A0, 0047B1F0): __fastcall(ECX =
// registry), RET, EAX = registry; SEH frame, state 0 around the tree construction. Vtable first,
// then head = _Buynode() (the tree at +4), the head's isnil byte = 1 (+19h for the Cloud node,
// +15h for the Vehicle node), then the head's parent (+4), left (+0) and right (+8) links = head,
// in that listing order (head re-read from the tree before each store), and size = 0.
void* registry_construct(Pool11Host& host, void* registry, std::uint32_t vtable, BuyHead buy,
                         std::size_t isnil_offset) {
    at<std::uint32_t>(registry, 0) = vtable;
    void* const tree = add(registry, 4);
    void* const head = (host.*buy)(tree);
    at<void*>(tree, 4) = head;
    at<std::uint8_t>(head, isnil_offset) = 1;
    at<void*>(ptr(tree, 4), 4) = ptr(tree, 4);
    at<void*>(ptr(tree, 4), 0) = ptr(tree, 4);
    at<void*>(ptr(tree, 4), 8) = ptr(tree, 4);
    at<std::uint32_t>(tree, 8) = 0;
    return registry;
}

// The shared shape of the two create-entity slots (00476360, 00479FE0): __fastcall(ECX = class),
// RET, EAX = entity; SEH frame, state 0 while the new entity is being constructed.
// entity = operator new(size), memset(entity, 0, size); when non-null: 00489F30 base constructor,
// the three vtables at +0 / +10h / +24h, the kind's own field stores, class id +C4h. Then (state
// -1) 0047B9E0(entity, class) and +188h = class. The null arm (XOR ESI,ESI) still calls 0047B9E0
// and stores +188h through the null pointer, as the image does; operator new throws on failure, so
// that arm is not reached in practice.
struct CreateShape {
    std::size_t size;
    std::uint32_t vtable_0, vtable_10, vtable_24;
    std::uint32_t class_id_c4;
};

void* create_entity(Pool11Host& host, void* cls, const CreateShape& shape, void (*extra)(void*)) {
    void* entity = host.operator_new_00bf55be(shape.size);
    host.memset_00bf79f0(entity, 0, shape.size);
    if (entity != nullptr) {
        host.class_entity_construct_base_00489f30(entity);
        at<std::uint32_t>(entity, 0x00) = shape.vtable_0;
        at<std::uint32_t>(entity, 0x10) = shape.vtable_10;
        at<std::uint32_t>(entity, 0x24) = shape.vtable_24;
        if (extra != nullptr) extra(entity);
        at<std::uint32_t>(entity, 0xC4) = shape.class_id_c4;
    }
    class_entity_set_class_0047b9e0(entity, cls);
    at<void*>(entity, 0x188) = cls;
    return entity;
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// 004755B0 BSP_BuoyClassRegistry_DestructBody (handler 00C62438): registry_destruct_body with the
// tree erase 00475090 and the singleton 00E18698.
// ---------------------------------------------------------------------------------------------
void buoy_class_registry_destruct_body_004755b0(Pool11Host& host, void* registry) {
    registry_destruct_body(host, registry, &Pool11Host::stl_tree_erase_range_00475090,
                           kBuoyClassRegistrySingleton_00e18698);
}

// ---------------------------------------------------------------------------------------------
// 00476360 BSP_CloudClass_CreateEntity (handler 00C624C8): size 1ACh; vtables 00CE5E68 /
// 00CE5E50 / 00CE5E48; +1A8h (byte) = 0 and +1A4h = 0; class id 3Dh. After the +188h store the
// entity is pushed pending (00926BE0, ECX = entity). The constants agree with the
// engine_seg4_6_9_cc13 kVisualEntityKinds row (pushes_pending = true).
// ---------------------------------------------------------------------------------------------
void* cloud_class_create_entity_00476360(Pool11Host& host, void* cls) {
    void* const entity = create_entity(host, cls, CreateShape{0x1AC, 0x00CE5E68, 0x00CE5E50, 0x00CE5E48, 0x3D},
                                       [](void* e) {
                                           at<std::uint8_t>(e, 0x1A8) = 0;
                                           at<std::uint32_t>(e, 0x1A4) = 0;
                                       });
    host.sentity_push_pending_00926be0(entity);
    return entity;
}

// ---------------------------------------------------------------------------------------------
// 004797A0 BSP_CloudClassRegistry_Construct (handler 00C62688): vtable 00CE60FC, head from
// 00476FA0, isnil byte at head+19h. Allocated 10h bytes by the lazy getter 004798A0.
// ---------------------------------------------------------------------------------------------
void* cloud_class_registry_construct_004797a0(Pool11Host& host, void* registry) {
    return registry_construct(host, registry, kCloudClassRegistryVtable_00ce60fc,
                              &Pool11Host::stl_tree_buy_head_node_00476fa0, 0x19);
}

// ---------------------------------------------------------------------------------------------
// 00479810 BSP_CloudClassRegistry_DestructBody (handler 00C626A8): erase 004792F0, singleton
// 00E186A0. Byte-for-byte the 004755B0 shape apart from those two constants and the handler.
// ---------------------------------------------------------------------------------------------
void cloud_class_registry_destruct_body_00479810(Pool11Host& host, void* registry) {
    registry_destruct_body(host, registry, &Pool11Host::stl_tree_erase_range_004792f0,
                           kCloudClassRegistrySingleton_00e186a0);
}

// ---------------------------------------------------------------------------------------------
// 00479FE0 BSP_VehicleVisualClass_CreateEntity (handler 00C62718): size 190h; vtables 00CE6130 /
// 00CE6114 / 00CE610C; class id 38h; no extra field stores and no push-pending call (the
// kVisualEntityKinds row has pushes_pending = false).
// ---------------------------------------------------------------------------------------------
void* vehicle_visual_class_create_entity_00479fe0(Pool11Host& host, void* cls) {
    return create_entity(host, cls, CreateShape{0x190, 0x00CE6130, 0x00CE6114, 0x00CE610C, 0x38}, nullptr);
}

// ---------------------------------------------------------------------------------------------
// 0047B1F0 BSP_VehicleVisualClassRegistry_Construct (handler 00C62798): vtable 00CE6264, head from
// 0047A510, isnil byte at head+15h. Allocated 10h bytes by the lazy getter 0047B2F0.
// ---------------------------------------------------------------------------------------------
void* vehicle_visual_class_registry_construct_0047b1f0(Pool11Host& host, void* registry) {
    return registry_construct(host, registry, kVehicleClassRegistryVtable_00ce6264,
                              &Pool11Host::stl_tree_buy_head_node_0047a510, 0x15);
}

// ---------------------------------------------------------------------------------------------
// 0047B260 BSP_VehicleVisualClassRegistry_DestructBody (handler 00C627B8): erase 0047ADF0,
// singleton 00E186DC; otherwise identical to 00479810 (listing diff: those three constants only).
// ---------------------------------------------------------------------------------------------
void vehicle_visual_class_registry_destruct_body_0047b260(Pool11Host& host, void* registry) {
    registry_destruct_body(host, registry, &Pool11Host::stl_tree_erase_range_0047adf0,
                           kVehicleClassRegistrySingleton_00e186dc);
}

// ---------------------------------------------------------------------------------------------
// 0047B9E0 BSP_ClassEntity_SetClass. __thiscall(ECX = entity, class), RET 4: [entity+170h] = class.
// No frame. Callers also store the class at +188h themselves.
// ---------------------------------------------------------------------------------------------
void class_entity_set_class_0047b9e0(void* entity, void* cls) noexcept {
    at<void*>(entity, 0x170) = cls;
}

}  // namespace bsp::pool_pool11_cc13
