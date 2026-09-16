#pragma once
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_stream_clone.hpp"

namespace bsp {
// Reuse the existing model companion preparation/binding and actual geometry,
// strings, pools and terminal graph. Its three model callbacks perform only
// canonical host bookkeeping; native construction is performed below. No new
// GeneratedModelLifetime projection, callable renderer table or owner domain.
struct NativeRendererGeneratedModelContext {
    NativeInstanceGeometryAccess& model_and_geometry;
    NativeVertexDeclarationCacheContext& declarations;
    NativeLogicalIndexCreationContext& indices;
    NativeMaterialEffectCacheContext& effects;
    GuiTextNativeLayoutServices& layouts;
    void* const volatile& actual_renderer_00f8d394;
    void* volatile& actual_manager_01090aa0;
};

// Explicit raw-model mode. These are the same publication cells borrowed by
// models.nodes, with null models.actual_names; they are not copied cells.
// Other material/declaration/effect names still use model_and_geometry.strings.
struct NativeRendererRawModelBinding {
    NativeStringRawPoolContext& names;
    const NativeNodeRawConstants& constants;
};

// Caller-owned diagnostic acquisitions. No destructor rolls back native state.
// It starts empty and may never be replayed after entry. It must survive any
// effect-loader failure with its real retained child frame. Completed native
// creators survive additional source metadata allocation/bind failures.
struct NativeRendererGeneratedModelAcquired {
    bool started{};
    bool complete{};
    std::uint32_t active_call_site{};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_reference{};
    void* model{};
    GuiNativeMeshAcquired mesh;
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    NativeStreamCloneAcquired index;
    NativeMaterialEffectCacheAcquired effect;
    GuiNativeMaterialAcquired material;
    GuiNativeSectionAcquired section;
};

// Complete B4C700[462]. Native ECX=model-name8h header, EDX=layout-name8h
// header; five stack arguments: effect-name header, section kind, vertex count,
// section primitive count, index count. EAX=constructed model; RET14h. Source
// adds the borrowed contexts; original callable ABI/FH3/SEH are not reproduced.
// Current renderer slots38/5C/60/48 must be D5F0A8's B317E0/B287C0/B288B0/B318B0.
// Use one actual raw AA0 domain and the SAME actual AA8/AA4 string service,
// allocator list, renderer and canonical references through all child lifetimes.
// Nonzero index_count leaves its native creator reference outstanding on
// success: B4C700 does NOT balance the index factory's reference. The returned
// model creator is also outstanding. No cleanup/release is invented for either.
void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t section_primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext&,
    NativeRendererGeneratedModelAcquired&);

// Same native schedule with the existing raw B75030 constructor. Pass the
// caller's same 8-byte model_name header address and actual bound constants.
// The runtime/pool bindings and null actual_names mode must survive the model;
// the constants must survive this call. No renderer/application owner is bound.
// Raw-runtime cell identity is checked; the separate string adapter's private
// publication bindings remain an application precondition.
void* create_native_renderer_generated_model_00b4c700(
    const NativeString& model_name, const NativeString& layout_name,
    NativeString& effect_name, std::uint32_t section_kind,
    std::uint32_t vertex_count, std::uint32_t section_primitive_count,
    std::uint32_t index_count, NativeRendererGeneratedModelContext&,
    NativeRendererGeneratedModelAcquired&, const NativeRendererRawModelBinding&);
} // namespace bsp
