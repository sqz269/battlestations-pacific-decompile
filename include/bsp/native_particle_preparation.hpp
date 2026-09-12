#pragma once
#include <cstdint>

namespace bsp {
struct NativeMaterialStorage;
struct NativeNodeStorage;

// Borrow the existing application's current member dispatch. These callbacks
// execute the supplied captured target on the SAME actual owner, with native
// ECX owner/no stack arguments. They must not replace preparation with a no-op.
// The caller's second slot/owner/table are reloaded after virtual14 returns.
struct NativeParticlePreparationDispatch {
    void* context;
    void (*member_virtual14)(void*, void* actual_member, std::uint32_t captured_target);
    void (*member_virtual0c)(void*, void* actual_member, std::uint32_t captured_target);
};
// Full AF40E0..AF4105 and AF9F50..AF9FA0. Native ECX actual variant/definition,
// plain RET. These interfaces add explicit application dispatch; they own no
// definition, table, hierarchy or member. Embedded pointer rows, current signed
// counts and pointer/table reloads follow the original instructions.
void prepare_native_particle_variant_00af40e0(void*, const NativeParticlePreparationDispatch&);
void prepare_native_particle_definition_00af9f50(void*, const NativeParticlePreparationDispatch&);

// Complete borrowed resource accessors, ECX actual resources/EAX result/RET.
void* __fastcall native_particle_index_stream_00af10a0(const void*);
void* __fastcall native_particle_vertex_descriptor_00af10b0(const void*);
NativeMaterialStorage* __fastcall native_particle_secondary_material_00af1120(const void*);
// AF10F0..AF111C: ECX resources, two stack DWORDs but only each low byte is
// tested; RET8. Unused EDX preserves that native stack layout in source ABI.
NativeMaterialStorage* __fastcall native_particle_material_variant_00af10f0(
    const void*, void* unused_edx, std::uint32_t first, std::uint32_t second);

// Actual borrowed shadow lookup chain, no reference operations or COM calls.
// B4D170/B4CB10 return holder+0C/+08. B0D130 reads owner3C before B4CB10;
// B0D140 reads owner60, calls B4D170, then tail-calls B4CB10. ECX/RET/EAX.
void* __fastcall native_shadow_texture_holder_00b4d170(const void*);
void* __fastcall native_shadow_holder_texture_00b4cb10(const void*);
void* __fastcall native_shadow_owner_texture_00b0d130(const void*);
void* __fastcall native_shadow_owner_map_texture_00b0d140(const void*);

// Complete7099C0..7099E3. Capture first child34 BEFORE storing mask48;
// recurse using the same mask, reload each captured child's next3C afterward.
// Native ECX node, stack DWORD mask, RET4; no scene/transform notifications.
void __fastcall set_native_node_hierarchy_mask_007099c0(
    NativeNodeStorage*, void* unused_edx, std::uint32_t mask);
// Descriptive names are hypotheses; original application dispatch, object
// allocation, native exception ABI and gameplay remain separate evidence.
} // namespace bsp
