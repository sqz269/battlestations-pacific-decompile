#pragma once

#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_stream_clone.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"

namespace bsp {

struct NativeNodeRawConstants;
struct NativeStringRawPoolContext;

// Compose the existing actual model, geometry, declaration, stream, material
// and layout providers. Every reference belongs to one application domain;
// no renderer, manager, pool, registry or reference count is introduced here.
struct NativeRendererGeneratedModelContext {
    NativeInstanceGeometryAccess& model_and_geometry;
    NativeVertexDeclarationCacheContext& declarations;
    NativeLogicalIndexCreationContext& indices;
    NativeMaterialEffectCacheContext& effects;
    GuiTextNativeLayoutServices& layouts;
    void* const volatile& actual_renderer_00f8d394;
    void* volatile& actual_manager_01090aa0;
};

// Exact raw-name mode for the application's null-actual_names model runtime.
// These are the same AA8/AA4/AA0 cells borrowed by models.nodes. The constants
// and caller's model-name header address survive the call and model lifetime.
struct NativeRendererRawModelBinding {
    NativeStringRawPoolContext& names;
    const NativeNodeRawConstants& constants;
};

// One-shot caller-owned diagnostics. Destruction performs no native release or
// rollback. Keep this frame and all service contexts alive until every recorded
// failed/outstanding acquisition has been resolved through its real terminal.
struct NativeRendererGeneratedModelAcquired {
    bool started{};
    bool complete{};
    std::uint32_t active_call_site{};
    void* raw_model{};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_reference{};
    GuiNativeMeshAcquired mesh;
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    NativeMaterialFactoryAcquired material;
    NativeStreamCloneAcquired index;
    GuiNativeSectionAcquired section;
};

// Complete B4C700[462]. Native ECX=model-name8h header, EDX=layout-name8h
// header; stack(effect-name header, section kind, vertex count, section
// primitive count, index count); EAX=constructed model; RET14h. Source adds
// explicit contexts and diagnostic frames; original callable ABI/FH3/SEH are
// not reproduced. Current renderer slots38/5C/60/48 must still identify
// B317E0/B287C0/B288B0/B318B0.
//
// The returned model creator remains outstanding. When index_count is nonzero,
// its creator also remains outstanding after B73B70 retains it; native B4C700
// has no balancing index decrement. No cleanup is invented for either.
void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t section_primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext&,
    NativeRendererGeneratedModelAcquired&);

// Same native schedule using the existing raw B75030 constructor. The model
// environment must have null actual_names and use this exact AA8/AA4/AA0 name
// context. No application owner or semantic fallback is created.
void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t section_primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext&,
    NativeRendererGeneratedModelAcquired&, const NativeRendererRawModelBinding&);

} // namespace bsp
