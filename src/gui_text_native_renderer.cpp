#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* reason) {
    if (!condition) throw std::logic_error(reason);
}
void current_slot(void* renderer, const volatile std::uint32_t* profile,
    std::uint32_t offset, std::uint32_t expected) {
    require(renderer && *static_cast<const volatile std::uint32_t*>(renderer) == 0x00d5f0a8u,
        "Text native factory requires the captured actual D5F0A8 renderer");
    require(profile && profile[offset / 4] == expected,
        "Text native factory has no source implementation for the current renderer slot");
}
void begin_stream(NativeStreamCloneAcquired& a, bool vertex) {
    require(!a.creator && !a.companion && a.phase == NativeStreamClonePhase::empty,
        "Text stream creation cannot replay an interrupted factory");
    a.vertex = vertex;
    a.phase = NativeStreamClonePhase::factory;
}
void register_stream(GuiTextNativeRendererServices& s, NativeStreamCloneAcquired& a) {
    if (!a.creator) throw std::bad_alloc(); // Native caller dereferences its result.
    a.phase = NativeStreamClonePhase::registration;
    s.streams.geometry.register_stream_clone_creator(a, s.streams);
    a.phase = NativeStreamClonePhase::complete;
}
} // namespace

void require_gui_text_native_renderer_domain(GuiTextNativeRendererServices& s,
    void* const volatile& renderer, NativeStringStorage& strings, NativeRenderActualOwners& owners) {
    auto& streams = s.streams;
    require(&s.declarations.strings == &strings && &s.declarations.declarations.strings == &strings &&
        &streams.geometry.actual_owners() == &owners && &streams.vertices.actual_owners == &owners &&
        static_cast<const volatile void*>(&renderer) ==
            static_cast<const volatile void*>(&streams.vertices.actual_renderer_00f8d394) &&
        &streams.vertices.actual_renderer_00f8d394 == &streams.indices.lifetime.actual_renderer_00f8d394 &&
        &streams.vertices.actual_renderer_00f8d394 == &streams.mapping.actual_renderer_00f8d394 &&
        &streams.vertices.actual_synchronization_0108d6dc == &streams.indices.lifetime.actual_synchronization_0108d6dc &&
        &streams.vertices.actual_synchronization_0108d6dc == &streams.mapping.actual_synchronization_0108d6dc &&
        &streams.vertices.actual_physical == &streams.indices.lifetime.actual_physical &&
        &streams.vertices.actual_physical_profiles == &streams.mapping.actual_physical_profiles &&
        streams.vertices.actual_type_sizes_00d61cc0 == s.declarations.declarations.type_sizes_00d61cc0 &&
        streams.vertices.actual_renderer_profile_00d5f0a8 == streams.indices.actual_renderer_profile_00d5f0a8,
        "Text factories require the same actual renderer, string, declaration and stream domains");
}

void* load_gui_text_native_declaration_current38(void* renderer, const NativeString& name,
    GuiTextNativeRendererServices& s, GuiNativeDeclarationAcquired& a) {
    require(!a.reference && !a.companion && !a.canonical_registration,
        "Text declaration load cannot reuse an acquired reference");
    current_slot(renderer, s.streams.vertices.actual_renderer_profile_00d5f0a8, 0x38, 0x00b317e0);
    a.reference = load_native_renderer_vertex_declaration_00b317e0(renderer, &name, s.declarations);
    if (!a.reference) throw std::bad_alloc();
    s.streams.geometry.register_native_declaration_reference(a, s.declarations.declarations);
    return a.reference;
}
void* create_gui_text_native_vertex_current5c(void* renderer, std::uint32_t count,
    std::uint32_t flags, void* declaration, GuiTextNativeRendererServices& s,
    NativeStreamCloneAcquired& a) {
    current_slot(renderer, s.streams.vertices.actual_renderer_profile_00d5f0a8, 0x5c, 0x00b287c0);
    begin_stream(a, true);
    (void)create_native_registered_vertex_stream_00b287c0(renderer, count, flags,
        declaration, s.streams.vertices, &a.creator);
    register_stream(s, a);
    return a.creator;
}
void* create_gui_text_native_index_current60(void* renderer, std::uint32_t count,
    std::uint32_t flags, std::uint32_t format, GuiTextNativeRendererServices& s,
    NativeStreamCloneAcquired& a) {
    current_slot(renderer, s.streams.indices.actual_renderer_profile_00d5f0a8, 0x60, 0x00b288b0);
    begin_stream(a, false);
    (void)create_native_registered_index_stream_00b288b0(renderer, count, flags,
        format, s.streams.indices, &a.creator);
    register_stream(s, a);
    return a.creator;
}
} // namespace bsp
