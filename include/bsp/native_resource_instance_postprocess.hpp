#pragma once
#include "bsp/native_resource_instance_bones.hpp"
#include <cstdint>

namespace bsp {
// B79BC0's four typed views use the same actual instance+30 tree and
// {node,item} pairs as B87CE0. Each caller supplies its SAME current type cell:
// B78750/B922C0:0109042C; B78810/B8A040:01090268;
// B788D0/B8A120:01090278; B78990/B8A1C0:01090288.
// The native bodies match B87CE0 after normalizing direct-call relocations.
// Reuse get_native_mesh_binding_range_00b87ce0 with the appropriate live cell;
// do not capture a startup token value or copy the mapped vector.
using NativeResourcePostprocessRange = NativeResourceMeshBindingRange;

// The original numeric profile selects the SAME live table, with no native
// call or owner mutation during lookup. Destroy must perform the complete
// current slot-zero service for that target, including its actual ownership.
class NativeResourceAnimatorLifetime {
public:
    virtual ~NativeResourceAnimatorLifetime() = default;
    virtual const volatile std::uint32_t* table(std::uint32_t profile) noexcept = 0;
    virtual void destroy(std::uint32_t target, void* animator) = 0;
};

// B6EE80: original ECX node, stacked incoming animator, RET4, no return value.
// Source EDX is borrowed lifetime context. Same-pointer assignment does
// nothing; otherwise publish+130, retain incoming+4, then release old+4 and
// dispatch its current slot zero if the decrement returns zero.
void __fastcall assign_native_node_animator_00b6ee80(void* node,
    NativeResourceAnimatorLifetime&, void* animator);

// B75F00: original ECX node, EDX candidate ancestor, AL boolean, RET.
// Starts at node+30: excludes the node itself and does not detect cycles.
std::uint8_t __fastcall native_node_has_ancestor_00b75f00(
    const void* node, const void* candidate) noexcept;

// B87200: original ECX instance, EAX arithmetic pair count, RET.
// B87260: ECX instance, stacked unsigned index, EAX pair.item, RET4.
// Both use the actual checked pair vector at instance+20. The latter retains
// the original invalid-parameter call and reload when that handler returns.
std::uint32_t __fastcall native_resource_instance_item_count_00b87200(
    const void* instance) noexcept;
void* __fastcall native_resource_instance_item_at_00b87260(
    const void* instance, void* unused_edx, std::uint32_t index);

// B79BC0 remains a required caller. These complete helpers do not stand in
// for animation construction, track publication, skin finalization or cameras.
} // namespace bsp
