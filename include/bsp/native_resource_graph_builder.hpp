#pragma once
#include "bsp/native_group_world_sphere.hpp"
#include "bsp/native_resource_postprocess_full.hpp"

namespace bsp {
// These are bindings to the SAME raw objects and existing canonical companions.
// Lookups have no native side effects. Dispatches must implement the complete
// selected target; no unknown factory, item or virtual service may be a no-op.
class NativeResourceGraphCalls {
public:
    virtual ~NativeResourceGraphCalls() = default;
    virtual const volatile std::uint32_t* table(std::uint32_t profile) noexcept = 0;
    virtual void* create_instance(std::uint32_t target, void* resource) = 0;
    virtual void* item_factory(std::uint32_t target, void* item) = 0;
    virtual void* create_node(std::uint32_t target, void* factory, const void* name) = 0;
    virtual void set_matrix(std::uint32_t target, void* node, const void* matrix) = 0;
    virtual std::uint8_t is_type(std::uint32_t target, void* node, std::uint32_t token) = 0;
    virtual void attach_item(std::uint32_t target, void* item, void* instance,
        void* record, void* node, std::uint32_t creation_word) = 0;
    virtual NativeNodeBinding& node_binding(void* actual_node) noexcept = 0;
    virtual NativeGroupOwner& group_owner(void* actual_group) noexcept = 0;
};
struct NativeResourceGraphContext {
    NativeResourceGraphCalls& calls;
    NativeStringStorage& strings;
    NativeNodeParentingRuntime& parenting;
    NativeGroupWorldSphereRuntime& group_spheres;
    NativeResourcePostprocessContext& postprocess;
    NativeResourceInstanceBoneContext& bones;
    const volatile std::uint32_t& model_type_01090034;
    const volatile std::uint32_t& group_type_0109032c;
};

// B891A0..B8969F: ECX resource, two stacked words, EAX instance, RET8. Source
// EDX borrows context. The second word is unconsumed; the first is forwarded
// verbatim to each item slot18. Source factories are temporary one-word native
// objects with profiles D63210/D63218/D63220, and cannot escape the call.
// Full caller including both postprocessors. Complete providers, current
// actual storage and existing companion projections are required; no binary
// FH3, arbitrary game factory admission or gameplay compatibility is asserted.
void* __fastcall build_native_resource_graph_00b891a0(void* resource,
    NativeResourceGraphContext&, std::uint32_t creation_word, std::uint32_t unused_word);
// 7137F0's second argument passes through FLD/FSTP before the same full call.
void* __fastcall build_native_game_resource_graph_007137f0(void* resource,
    NativeResourceGraphContext&, std::uint32_t creation_word, std::uint32_t float_word);

// Actual checked vector: opaque0/begin4/end8/capacity-endC. Original ECX,
// requested unsigned count and fill value on stack, RET8; EDX unused.
void __fastcall resize_native_resource_node_vector_00b88b60(void*, void*,
    std::uint32_t requested, std::uint32_t value);
// 4FCBE0: ECX vector; output, first-owner, first, last-owner, last on stack;
// EAX output, RET14. Validate iterator-owner equality, move actual tail through
// CRT memmove_s, then publish captured resulting end and iterator. No free.
void* __fastcall erase_native_graph_node_range_004fcbe0(void*, void*, void* output,
    void* first_owner, void* first, void* last_owner, void* last);

std::uint32_t native_graph_model_type_00b74310(const volatile std::uint32_t&) noexcept;
std::uint32_t native_graph_group_type_00b8e600(const volatile std::uint32_t&) noexcept;
// Complete56-byte body: flags138 &=~30h, byte175=0, six forward x87 copies.
void __fastcall set_native_graph_group_box_00b8e6f0(void*, void*, const void*) noexcept;
// Complete12-byte caller over the established group companion projection:
// set actual175=1 then perform the complete B8EBE0 aggregation.
void enable_native_graph_group_auto_bounds_00b8f0f0(NativeGroupWorldSphereRuntime&,
    NativeGroupOwner&);
} // namespace bsp
