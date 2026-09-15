#pragma once
#include "bsp/native_instance_generator_owner.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_texture_field.hpp"
#include <memory>
#include <vector>

namespace bsp {
struct NativeMeshSubsetLoadingContext {
    GuiNativeGeometryOwners& geometry;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialEffectCacheContext& effects;
    NativeMeshTextureFieldContext& texture_fields; // SAME reads/string/renderer/owners.
    NativeMeshLightingConstants lighting;
    NativeInstanceGeneratorOwners& generators; // SAME actual layout and geometry domain.
    const volatile std::uint32_t* material_profile_00d5e520;
};
struct NativeMeshSubsetAcquired {
    enum class Phase { empty, section, prefix, name, alias, material,
        material_admission, material_assignment, material_release, streams_clear,
        child, child_fields, child_release, default_stream, layout,
        mesh_publication, generator, section_release, name_return, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{};
    GuiNativeSectionAcquired section;
    NativeMeshSectionStorage* captured_section{}; // Audit identity; no extra retain.
    NativeMaterialFactoryAcquired factory;
    NativeMaterialStorage* captured_material{};
    bool material_creator_consumed{}, section_published{}, section_creator_consumed{};
    NativeString name;
    bool name_completed{}, name_cleanup_armed{}, name_returned{};
    void* child{};
    bool child_cleanup_armed{};
    // Each Texture child has a distinct persistent cache/provider frame. A
    // failed child or generator is not retried/discarded by this parent.
    std::vector<std::unique_ptr<NativeMeshTextureFieldAcquired>> texture_children;
    NativeInstanceGeneratorAcquired generator;
};

// Complete B941D0[750], ECX unused output-pair context; stacked mesh/parent;
// RET8. Actual533FA0, five ordered DWORDs, case-insensitive soldiers.mshd
// alias to soldier.mshd, numeric535320 and canonical admission. Ordered
// Texture/LightingSettings/BoundingSphere/VertexStreamIndex children; unknown
// children skip. Sphere values are discarded straight from x87 ST0.
// Default first stream only when section count==0, actual B865A0 layout,
// append/retain section in mesh BEFORE B85610, then consume creator and name.
// Native EH owns only the completed name and current completed child, never
// material/section creators or mesh publications. Persist acquired on error.
// All services borrow the same actual domains and outlive native resources.
// Cold shader/program dependencies must be implemented by their real owners;
// no successful substitute is provided. New Win32 source, not native SEH/ABI.
void read_native_mesh_subset_00b941d0(void* actual_mesh, void* actual_parent_handle,
    NativeMeshSubsetLoadingContext&, NativeMeshSubsetAcquired&);
} // namespace bsp
