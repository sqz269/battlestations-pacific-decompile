#include "bsp/native_mesh_buffer_fields.hpp"
#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
using Profile = const volatile U*;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void require(bool condition, const char* reason) { if (!condition) throw std::logic_error(reason); }
void begin(NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& acquired) {
    require(acquired.phase == NativeMeshBufferReadPhase::empty && !acquired.stream.creator &&
        !acquired.stream.companion && !acquired.declaration.reference && !acquired.declaration.companion,
        "Native mesh buffer reader cannot replay an acquired operation");
    auto& s = context.graphics.streams;
    require(&s.geometry.actual_owners() == &s.vertices.actual_owners &&
        &s.vertices.actual_renderer_00f8d394 == &s.indices.lifetime.actual_renderer_00f8d394 &&
        &s.vertices.actual_renderer_00f8d394 == &s.mapping.actual_renderer_00f8d394 &&
        &s.vertices.actual_synchronization_0108d6dc == &s.indices.lifetime.actual_synchronization_0108d6dc &&
        &s.vertices.actual_synchronization_0108d6dc == &s.mapping.actual_synchronization_0108d6dc &&
        &s.vertices.actual_physical == &s.indices.lifetime.actual_physical &&
        &s.vertices.actual_physical_profiles == &s.mapping.actual_physical_profiles &&
        &s.vertices.actual_physical.actual_lifetime_01090aa0 == &s.mapping.actual_physical_lock.actual_lifetime_01090aa0 &&
        s.vertices.actual_renderer_profile_00d5f0a8 == s.indices.actual_renderer_profile_00d5f0a8 &&
        s.vertices.actual_type_sizes_00d61cc0 == context.graphics.declarations.declarations.type_sizes_00d61cc0 &&
        &context.graphics.declarations.strings == &context.graphics.declarations.declarations.strings,
        "Native mesh buffer reader requires one actual renderer and owner domain");
    acquired.phase = NativeMeshBufferReadPhase::header;
}
void* current_renderer(NativeMeshBufferReadContext& context, U offset, U entry) {
    auto& s = context.graphics.streams;
    auto* const renderer = const_cast<void*>(s.vertices.actual_renderer_00f8d394);
    auto* const profile = s.vertices.actual_renderer_profile_00d5f0a8;
    require(renderer && word(renderer) == 0x00d5f0a8 && profile && profile[offset / 4] == entry,
        "Native mesh reader reached an unsupported current renderer slot");
    return renderer;
}
Profile stream_profile(void* stream, bool vertex, NativeMeshBufferReadContext& context) {
    auto& s = context.graphics.streams;
    require(stream && word(stream) == (vertex ? 0x00d61d6cu : 0x00d61de0u),
        "Native mesh reader requires the current actual logical stream profile");
    auto* const profile = vertex ? s.vertices.actual_logical_profile_00d61d6c : s.indices.actual_logical_profile_00d61de0;
    require(profile != nullptr, "Native mesh reader requires its original logical profile view");
    return profile;
}
void slot(Profile profile, U offset, U entry) {
    require(profile[offset / 4] == entry, "Native mesh reader reached an unsupported current logical stream slot");
}
void mark(NativeMeshBufferReadAcquired& a, NativeMeshBufferReadPhase phase, U site) noexcept {
    a.phase = phase; a.native_site = site;
}
void* map(void* stream, bool vertex, U count, NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& a) {
    a.stream.destination_map = NativeStreamCloneMapPhase::call_in_progress;
    a.stream.destination_mapping = vertex ? lock_native_logical_vertex_stream_00b49980(stream, context.graphics.streams.mapping, count, 0, 0) :
        lock_native_logical_index_stream_00b49b60(stream, context.graphics.streams.mapping, count, 0, 0);
    a.stream.destination_map = NativeStreamCloneMapPhase::returned;
    return a.stream.destination_mapping;
}
void unmap(void* stream, bool vertex, NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& a) {
    a.stream.destination_map = NativeStreamCloneMapPhase::unlock_in_progress;
    if (vertex) unlock_native_logical_vertex_stream_00b49a80(stream, context.graphics.streams.mapping);
    else unlock_native_logical_index_stream_00b49c70(stream, context.graphics.streams.mapping);
    a.stream.destination_map = NativeStreamCloneMapPhase::unlocked;
}
void release_creator(NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& a) {
    auto* const owner = a.stream.creator;
    a.stream.creator = nullptr; a.stream.companion = nullptr; a.stream.canonical_registration = false;
    a.stream.phase = NativeStreamClonePhase::consumed;
    release_native_render_actual_owner(context.graphics.streams.geometry.actual_owners(), owner);
}
} // namespace

void read_native_resource_raw_00bf02f0(void* reader, void* destination, U count, U* budget, NativeResourceStreamReadContext& reads) {
    auto* const stream = ptr(word(reader));
    const auto entry = word(ptr(word(stream)), 0x24);
    reads.streams.source_read(entry, stream, destination, count, &count);
    auto* const current = static_cast<volatile U*>(budget); *current = *current - count;
}
void read_native_resource_node_raw_00be9a20(void* handle, void* destination, U count, NativeResourceStreamReadContext& reads) {
    auto* const node = ptr(word(handle));
    auto* const budget = static_cast<U*>(at(node, 0x20));
    auto* const reader = ptr(word(node, 8));
    read_native_resource_raw_00bf02f0(reader, destination, count, budget, reads);
}
U native_mesh_vertex_stream_count_00b72b20(const void* mesh) noexcept { return word(mesh, 0x7c); }
void native_logical_stream_post_upload_00b49b30(void*) noexcept {}

void read_native_mesh_indices_00b93aa0(void* mesh, void* handle, NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& a) {
    begin(context, a);
    const auto count = read_native_resource_node_control_dword_00be99f0(handle, context.reads);
    const auto format = read_native_resource_node_control_dword_00be99f0(handle, context.reads);
    auto* const renderer = current_renderer(context, 0x60, 0x00b288b0);
    mark(a, NativeMeshBufferReadPhase::factory, 0x00b93ac9);
    auto* const stream = create_gui_text_native_index_current60(renderer, count, 1, format, context.graphics, a.stream);
    a.captured_stream = stream;
    const U width = format == 0x65 ? 2u : format == 0x66 ? 4u : 0u;
    slot(stream_profile(stream, false, context), 0x0c, 0x00b49b60);
    mark(a, NativeMeshBufferReadPhase::map, 0x00b93af3);
    auto* const destination = map(stream, false, count, context, a);
    const auto bytes = width * count;
    mark(a, NativeMeshBufferReadPhase::read, 0x00b93afc);
    read_native_resource_node_raw_00be9a20(handle, destination, bytes, context.reads);
    slot(stream_profile(stream, false, context), 0x10, 0x00b49c70);
    mark(a, NativeMeshBufferReadPhase::unmap, 0x00b93b08); unmap(stream, false, context, a);
    slot(stream_profile(stream, false, context), 0x2c, 0x00b49b30);
    mark(a, NativeMeshBufferReadPhase::post_upload, 0x00b93b11); native_logical_stream_post_upload_00b49b30(stream);
    mark(a, NativeMeshBufferReadPhase::mesh_publication, 0x00b93b18);
    set_native_mesh_index_stream_00b73b70(*static_cast<NativeMeshStorage*>(mesh), context.graphics.streams.geometry.actual_owners(), stream);
    mark(a, NativeMeshBufferReadPhase::creator_release, 0x00b93b21); release_creator(context, a);
    a.phase = NativeMeshBufferReadPhase::complete;
}
void read_native_mesh_vertex_stream_00b93e60(void* mesh, void* handle, NativeMeshBufferReadContext& context, NativeMeshBufferReadAcquired& a) {
    begin(context, a); U name[2]; bool complete_name = false;
    try {
        const auto count = read_native_resource_node_control_dword_00be99f0(handle, context.reads);
        (void)read_native_resource_handle_string_00bea010(handle, name, context.reads);
        auto* renderer = current_renderer(context, 0x38, 0x00b317e0); complete_name = true;
        mark(a, NativeMeshBufferReadPhase::declaration, 0x00b93ead);
        auto* const declaration = load_gui_text_native_declaration_current38(renderer,
            *reinterpret_cast<const NativeString*>(name), context.graphics, a.declaration);
        a.captured_declaration = declaration;
        renderer = current_renderer(context, 0x5c, 0x00b287c0);
        mark(a, NativeMeshBufferReadPhase::factory, 0x00b93ec0);
        auto* const stream = create_gui_text_native_vertex_current5c(renderer, count, 1, declaration, context.graphics, a.stream);
        a.captured_stream = stream;
        mark(a, NativeMeshBufferReadPhase::declaration_release, 0x00b93ec8); a.declaration = {};
        release_native_render_actual_owner(context.graphics.streams.geometry.actual_owners(), declaration);
        auto* const profile = stream_profile(stream, true, context);
        const auto stride = word(declaration, 0xcc); slot(profile, 0x10, 0x00b49980);
        const auto bytes = stride * count;
        mark(a, NativeMeshBufferReadPhase::map, 0x00b93eef);
        auto* const destination = map(stream, true, count, context, a);
        mark(a, NativeMeshBufferReadPhase::read, 0x00b93ef5);
        read_native_resource_node_raw_00be9a20(handle, destination, bytes, context.reads);
        auto* const text = static_cast<const char*>(ptr(word(name, 4)));
        if (text && _stricmp(text, "rope.mvfm") == 0) {
            mark(a, NativeMeshBufferReadPhase::skip_tail, 0x00b93f1b); skip_native_resource_node_00be9c40(handle, context.reads);
        }
        slot(stream_profile(stream, true, context), 0x14, 0x00b49a80);
        mark(a, NativeMeshBufferReadPhase::unmap, 0x00b93f27); unmap(stream, true, context, a);
        const auto index = native_mesh_vertex_stream_count_00b72b20(mesh);
        mark(a, NativeMeshBufferReadPhase::mesh_publication, 0x00b93f38);
        set_native_mesh_vertex_stream_00b73bb0(*static_cast<NativeMeshStorage*>(mesh), context.graphics.streams.geometry.actual_owners(),
            static_cast<std::int32_t>(index), stream);
        mark(a, NativeMeshBufferReadPhase::creator_release, 0x00b93f41); release_creator(context, a);
        auto* const data = ptr(word(name, 4)); complete_name = false;
        if (data) {
            const auto size = word(name) + 1u; auto& strings = context.reads.strings;
            mark(a, NativeMeshBufferReadPhase::name_return, 0x00b93f78);
            auto* const pool = native_string_pool_get_or_create_00419cc0(strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size, strings.actual_small_returns_disabled_01090aa4);
        }
        a.phase = NativeMeshBufferReadPhase::complete;
    } catch (...) {
        if (complete_name) {
            try { destroy_native_string_header_0041dd20(name, context.reads.strings); }
            catch (...) { std::terminate(); }
        }
        throw;
    }
}
} // namespace bsp
