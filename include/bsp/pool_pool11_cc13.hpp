// Pool packet pool11 (segment 6, 004755B0..0047B9E0): the Cloud and Vehicle visual class
// registries, the Buoy registry destructor, the Cloud / Vehicle visual-entity create slots and the
// class-entity class setter. Bodies left in the progress-board pool after the oct24 credit audit;
// engine_seg4_6_9_cc13.cpp only tabulates their constants (kVisualEntityKinds,
// kLazySingletonGetters).
//
// Model as in cc13_seg33a: raw image offsets over a Pool11Host whose methods name the image routine
// they stand for. The STL tree helpers, operator new / free and memset are library code bound as
// host calls. New C++ interfaces, not drop-in binary replacements; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols. Marker: pool_pool11_cc13.

#ifndef BSP_POOL_POOL11_CC13_HPP
#define BSP_POOL_POOL11_CC13_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg33a.hpp"

namespace bsp::pool_pool11_cc13 {

struct Pool11Host {
    virtual ~Pool11Host() = default;

    // A pointer-sized global of the image (the lazy registry singletons 00E18698, 00E186A0, 00E186DC).
    virtual void*& global_pointer(std::uint32_t address) = 0;

    // 00BF55BE operator new(size), cdecl; 00BF79F0 memset, cdecl; 00BF65AC free, cdecl.
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void memset_00bf79f0(void* destination, int value, std::size_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;

    // 00489F30 BSP_ClassEntity37_ConstructBase, __fastcall(ECX = entity), RET.
    virtual void class_entity_construct_base_00489f30(void* entity) = 0;
    // 00926BE0 BSP_SEntity_PushPending, __fastcall(ECX = entity), RET.
    virtual void sentity_push_pending_00926be0(void* entity) = 0;

    // STL tree erase(out, first, last) with checked iterators {container, node}:
    // __thiscall(ECX = tree, out, first.container, first.node, last.container, last.node), RET 14h.
    virtual void stl_tree_erase_range_00475090(void* tree, void* out, void* first_container, void* first_node,
                                               void* last_container, void* last_node) = 0;
    virtual void stl_tree_erase_range_004792f0(void* tree, void* out, void* first_container, void* first_node,
                                               void* last_container, void* last_node) = 0;
    virtual void stl_tree_erase_range_0047adf0(void* tree, void* out, void* first_container, void* first_node,
                                               void* last_container, void* last_node) = 0;
    // STL tree _Buynode() for the head: __fastcall(ECX = tree), RET, EAX = node.
    virtual void* stl_tree_buy_head_node_00476fa0(void* tree) = 0;
    virtual void* stl_tree_buy_head_node_0047a510(void* tree) = 0;
};

inline constexpr std::uint32_t kBuoyClassRegistrySingleton_00e18698 = 0x00E18698u;
inline constexpr std::uint32_t kCloudClassRegistrySingleton_00e186a0 = 0x00E186A0u;
inline constexpr std::uint32_t kVehicleClassRegistrySingleton_00e186dc = 0x00E186DCu;
inline constexpr std::uint32_t kRegistryBaseVtable_00ce3818 = 0x00CE3818u;
inline constexpr std::uint32_t kCloudClassRegistryVtable_00ce60fc = 0x00CE60FCu;
inline constexpr std::uint32_t kVehicleClassRegistryVtable_00ce6264 = 0x00CE6264u;

void buoy_class_registry_destruct_body_004755b0(Pool11Host& host, void* registry);
void* cloud_class_create_entity_00476360(Pool11Host& host, void* cls);
void* cloud_class_registry_construct_004797a0(Pool11Host& host, void* registry);
void cloud_class_registry_destruct_body_00479810(Pool11Host& host, void* registry);
void* vehicle_visual_class_create_entity_00479fe0(Pool11Host& host, void* cls);
void* vehicle_visual_class_registry_construct_0047b1f0(Pool11Host& host, void* registry);
void vehicle_visual_class_registry_destruct_body_0047b260(Pool11Host& host, void* registry);
void class_entity_set_class_0047b9e0(void* entity, void* cls) noexcept;

}  // namespace bsp::pool_pool11_cc13

#endif
