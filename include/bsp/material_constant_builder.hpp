#pragma once
#include "bsp/material_entry_dispatch.hpp"
#include "bsp/camera_transform.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/mesh_decode_bindings.hpp"

namespace bsp {
// Native callback receives the addresses immediately before the float banks.
// Header meaning remains unresolved: preserve the actual DWORDs as opaque state.
struct MaterialConstantCallbackBlocks {
    std::uint32_t& vertex_header_0108ebf0;
    std::uint32_t& pixel_header_0108dbe8;
    MaterialEntryConstantState& words;
};
class MaterialDynamicTextureSource {
public:
    virtual ~MaterialDynamicTextureSource() = default;
    virtual bool texture_2c(std::shared_ptr<LogicalTexture>&, std::string&) = 0;
    virtual bool constants_30(InstanceRenderEntry&, const ShaderConstantBindings&,
        const ShaderConstantBindings&, MaterialConstantCallbackBlocks&, std::string&) = 0;
};
struct MaterialDynamicTextureBinding {
    std::uint32_t slot{};
    std::shared_ptr<MaterialDynamicTextureSource> source;
};
struct MaterialBoneModel {
    std::int32_t count_188{};
    std::vector<CameraTransform*> nodes_184; // native pointer-array element stride4
};
struct MaterialSkinBone {
    CameraTransform* transform{};
    CameraMatrix inverse_bind_20; // native skin record stride60h, matrix at20h
};
class MaterialSkinAnimator {
public:
    virtual ~MaterialSkinAnimator() = default;
    virtual bool accepts_type_0c(std::uint32_t, bool&, std::string&) = 0;
    // Side-effect-free projection of [captured animator+34]+8, NOT a native
    // virtual callback. The captured animator stays alive across its predicate.
    virtual bool palette_count_34_08(std::uint32_t&, std::string&) = 0;
};
struct MaterialSkinModel {
    CameraTransform& transform;
    MaterialSkinAnimator* animator_130{};
    const void* palette_190{}; // read AFTER type predicate; null skips fallback
    std::size_t available_palette_bytes{};
    std::int32_t count_188{};
    std::vector<MaterialSkinBone> bones_184;
};
class MaterialShadowMapOwner {
public:
    virtual ~MaterialShadowMapOwner() = default;
    virtual bool texture_08(std::shared_ptr<LogicalTexture>&, std::string&) = 0;
};
// These projections resolve current retained owners at the native stage. They
// are side-effect-free except explicit native virtual calls above. Missing owner
// bindings fail; they must never manufacture zero parameters/palettes/lights.
// Objects, dynamic slots and output storage stay alive throughout callbacks.
class MaterialConstantOwners {
public:
    virtual ~MaterialConstantOwners() = default;
    virtual bool material(InstanceRenderEntry&, MaterialCloneState*&, std::string&) = 0;
    virtual bool camera_mode(InstanceRenderEntry&, std::uint32_t&, std::string&) = 0;
    virtual bool transform(InstanceRenderEntry&, CameraTransform*&, std::string&) = 0;
    virtual bool bone_model(InstanceRenderEntry&, MaterialBoneModel*&, std::string&) = 0;
    virtual bool skin_model(InstanceRenderEntry&, MaterialSkinModel*&, std::string&) = 0;
    virtual bool decode_inputs(InstanceRenderEntry&, const CompiledMaterialPass&,
        std::vector<const LogicalVertexStream*>&, MeshDecodeDescriptorLimits&,
        std::string&) = 0;
    virtual bool lod_threshold(InstanceRenderEntry&, const float*&, std::string&) = 0;
    virtual bool model_lifetime(InstanceRenderEntry&, GeneratedModelLifetime*&, std::string&) = 0;
    virtual bool shadow_texture(std::shared_ptr<LogicalTexture>&, std::string&) = 0;
    // False has_first is the native failure path; null owner uses pass+84.
    virtual bool first_shadow_light(InstanceRenderEntry&, bool& has_first,
        MaterialShadowMapOwner*&, std::string&) = 0;
};
struct MaterialConstantBuilderEnvironment {
    D3D9StateCache*& renderer; // global00F8D394, callback may replace it
    const std::uint32_t* optimized_animator_type_010900fc; // required only for an actual animator predicate
    std::uint32_t& vertex_header_0108ebf0;
    std::uint32_t& pixel_header_0108dbe8;
    MaterialConstantOwners& owners;
};
//00B423C5..00B42693: direct live borrowed table, selected pass metadata. Native
// matrix x87 copies and row2 MOVSS staging; VS then PS for EACH parameter.
// Native00BF7680 raw copies support overlap; matrix overlap preserves the
// native sequential effects. Checked failures retain earlier writes.
bool write_material_parameters_00b423c5(const MaterialParameterBindings&,
    const CompiledMaterialPass&, std::uint32_t selector, MaterialEntryConstantState&,
    std::string&);
//00B42350, ECX pass/stack entry,override/RET8. Override has no body read.
// Recovered ordered builder composed with actual owner interfaces above.
// HRESULT failures are accumulated and later native stages still run. Invalid
// host bindings stop with prior side effects retained. Native owner factories,
// dynamic source implementations and system builder00B46A70 remain separate.
bool build_material_constants_00b42350(CompiledMaterialPass&, InstanceRenderEntry&,
    MaterialEntryOverride*, MaterialEntryConstantState&,
    MaterialConstantBuilderEnvironment&, HRESULT& device_result, std::string&);
}
