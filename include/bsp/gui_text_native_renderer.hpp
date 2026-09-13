#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_stream_clone.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"

namespace bsp {
// Actual declaration cache/decoder and existing canonical raw stream factory
// services. All publications, profiles, pools and companions share one domain
// and outlive every created resource. Native table cells remain numeric tokens.
struct GuiTextNativeRendererServices {
    NativeVertexDeclarationCacheContext& declarations;
    NativeStreamCloneServices& streams;
};

void require_gui_text_native_renderer_domain(GuiTextNativeRendererServices&,
    void* const volatile& current_renderer_00f8d394, NativeStringStorage&,
    NativeRenderActualOwners&);

// Callers capture the current renderer at their original call sites. These
// adapters inspect that SAME receiver/profile and dispatch only the observed
// D5F0A8 slots38=B317E0,5C=B287C0,60=B288B0. No callable replacement vtable.
// Acquired records start empty and survive exceptions. No retry or rollback.
// Declaration returns one caller reference in addition to the cache's creator;
// streams return one creator and register exactly one canonical companion.
void* load_gui_text_native_declaration_current38(void* actual_renderer,
    const NativeString& name, GuiTextNativeRendererServices&, GuiNativeDeclarationAcquired&);
void* create_gui_text_native_vertex_current5c(void* actual_renderer,
    std::uint32_t count, std::uint32_t flags, void* actual_declaration,
    GuiTextNativeRendererServices&, NativeStreamCloneAcquired&);
void* create_gui_text_native_index_current60(void* actual_renderer,
    std::uint32_t count, std::uint32_t flags, std::uint32_t format,
    GuiTextNativeRendererServices&, NativeStreamCloneAcquired&);
} // namespace bsp
