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

// Source composition for an already-captured renderer/+5C pair. The caller
// supplies the ACTUAL live renderer/object, descriptor and native profile view,
// with matching physical/device/pool/canonical-owner services. Header/token and
// nonnull-view checks do not authenticate a copied Native layout or its origin.
// Services and a fresh caller-owned acquired frame survive every partial effect
// and terminal callback. No new custody, rollback, retry or extra Native retain.
// Dispatches only saved B287C0; does not reload +5C or replace the saved receiver.
// Original B287C0 late +58 behavior and existing canonical admission remain.
// New MSVC Win32 interface, not a Native entry/SEH replacement or factory owner.
void* create_native_vertex_from_captured_renderer_5c(void* actual_renderer,
    std::uint32_t captured_entry, std::uint32_t first, std::uint32_t bytes,
    void* actual_descriptor, NativeStreamCloneServices&, NativeStreamCloneAcquired&);
} // namespace bsp
