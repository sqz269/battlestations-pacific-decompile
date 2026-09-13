#include "bsp/native_stream_clone.hpp"
#include "bsp/gui_native_geometry.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Profile = const volatile std::uint32_t*;
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const unsigned char*>(base) + offset);
}
void require(bool condition, const char* reason) {
    if (!condition) throw std::logic_error(reason);
}
Profile stream_profile(const void* stream, bool vertex, NativeStreamCloneServices& s) {
    require(stream != nullptr, "stream clone reached null native stream");
    require(word(stream) == (vertex ? 0x00d61d6cu : 0x00d61de0u),
        "stream clone requires the current observed logical profile");
    auto* profile = vertex ? s.vertices.actual_logical_profile_00d61d6c :
        s.indices.actual_logical_profile_00d61de0;
    require(profile != nullptr, "stream clone requires the current native profile view");
    return profile;
}
void slot(Profile profile, std::uint32_t offset, std::uint32_t target) {
    require(profile[offset / 4] == target,
        "stream clone has no implementation for this current native slot");
}
std::uint32_t count(void* source, bool vertex, NativeStreamCloneServices& s) {
    slot(stream_profile(source, vertex, s), vertex ? 0x20u : 0x1cu,
        vertex ? 0x00b48cd0u : 0x00b48d90u);
    return word(source, vertex ? 0x64u : 0x14u);
}
std::uint32_t flags(void* source, bool vertex, NativeStreamCloneServices& s) {
    slot(stream_profile(source, vertex, s), vertex ? 0x1cu : 0x18u,
        vertex ? 0x00b48cc0u : 0x00b48d80u);
    return word(source, vertex ? 0x60u : 0x10u);
}
std::uint32_t format(void* source, NativeStreamCloneServices& s) {
    slot(stream_profile(source, false, s), 0x20, 0x00b48da0);
    return word(source, 0x18);
}
void* declaration(void* source, NativeStreamCloneServices& s) {
    slot(stream_profile(source, true, s), 0x24, 0x00b48ce0);
    return reinterpret_cast<void*>(word(source, 0x68));
}
void begin(void* source, bool vertex, NativeStreamCloneServices& s,
    NativeStreamCloneAcquired& acquired) {
    require(!acquired.creator && !acquired.companion &&
        (acquired.phase == NativeStreamClonePhase::empty ||
         acquired.phase == NativeStreamClonePhase::consumed),
        "stream clone cannot restart an acquired or interrupted operation");
    require(&s.geometry.actual_owners() == &s.vertices.actual_owners &&
        &s.vertices.actual_renderer_00f8d394 == &s.indices.lifetime.actual_renderer_00f8d394 &&
        &s.vertices.actual_renderer_00f8d394 == &s.mapping.actual_renderer_00f8d394 &&
        &s.vertices.actual_synchronization_0108d6dc == &s.indices.lifetime.actual_synchronization_0108d6dc &&
        &s.vertices.actual_synchronization_0108d6dc == &s.mapping.actual_synchronization_0108d6dc &&
        &s.vertices.actual_physical == &s.indices.lifetime.actual_physical &&
        &s.vertices.actual_physical_profiles == &s.mapping.actual_physical_profiles &&
        &s.vertices.actual_physical.actual_lifetime_01090aa0 == &s.mapping.actual_physical_lock.actual_lifetime_01090aa0,
        "stream clone services must borrow the same actual owner and renderer domains");
    (void)stream_profile(source, vertex, s);
    auto& reference = s.geometry.actual_owners().resolve_actual(source);
    if (vertex) {
        auto* actual = dynamic_cast<NativeLogicalVertexReference*>(&reference);
        require(actual && actual->storage() == source, "vertex clone source lacks its canonical companion");
    } else {
        auto* actual = dynamic_cast<NativeLogicalIndexReference*>(&reference);
        require(actual && actual->storage() == source, "index clone source lacks its canonical companion");
    }
    acquired = {};
    acquired.source = source;
    acquired.vertex = vertex;
}
Profile capture_renderer(void* renderer, bool vertex, NativeStreamCloneServices& s) {
    require(renderer && word(renderer) == 0x00d5f0a8u,
        "stream clone requires the captured actual D5F0A8 renderer");
    auto* profile = vertex ? s.vertices.actual_renderer_profile_00d5f0a8 :
        s.indices.actual_renderer_profile_00d5f0a8;
    require(profile != nullptr, "stream clone requires the captured renderer table view");
    return profile;
}
void map_pair(void* source, bool vertex, NativeStreamCloneServices& s,
    NativeStreamCloneAcquired& a) {
    auto* source_table = stream_profile(source, vertex, s); // D2/AA2 before count.
    const auto source_count = count(source, vertex, s);
    slot(source_table, vertex ? 0x10u : 0x0cu, vertex ? 0x00b49980u : 0x00b49b60u);
    a.phase = NativeStreamClonePhase::source_map;
    a.native_site = vertex ? 0x00b72ab7u : 0x00b729e7u;
    a.source_map = NativeStreamCloneMapPhase::call_in_progress;
    a.source_mapping = vertex ? lock_native_logical_vertex_stream_00b49980(source, s.mapping, source_count, 0, 0) :
        lock_native_logical_index_stream_00b49b60(source, s.mapping, source_count, 0, 0);
    a.source_map = NativeStreamCloneMapPhase::returned;
    auto* destination_table = stream_profile(a.creator, vertex, s); // EB/ABB before count.
    const auto destination_count = count(source, vertex, s);
    slot(destination_table, vertex ? 0x10u : 0x0cu, vertex ? 0x00b49980u : 0x00b49b60u);
    a.phase = NativeStreamClonePhase::destination_map;
    a.native_site = vertex ? 0x00b72ad0u : 0x00b72a00u;
    a.destination_map = NativeStreamCloneMapPhase::call_in_progress;
    a.destination_mapping = vertex ? lock_native_logical_vertex_stream_00b49980(a.creator, s.mapping, destination_count, 0, 0) :
        lock_native_logical_index_stream_00b49b60(a.creator, s.mapping, destination_count, 0, 0);
    a.destination_map = NativeStreamCloneMapPhase::returned;
}
void copy_and_unlock(void* source, bool vertex, std::uint32_t width,
    NativeStreamCloneServices& s, NativeStreamCloneAcquired& a) {
    const auto bytes = count(source, vertex, s) * width; // native low DWORD IMUL.
    a.phase = NativeStreamClonePhase::copy;
    a.native_site = vertex ? 0x00b72af8u : 0x00b72a44u;
    std::memcpy(a.destination_mapping, a.source_mapping, bytes); // native _memcpy, cdecl/ADD ESP,C.
    slot(stream_profile(source, vertex, s), vertex ? 0x14u : 0x10u,
        vertex ? 0x00b49a80u : 0x00b49c70u);
    a.phase = NativeStreamClonePhase::source_unlock;
    a.native_site = vertex ? 0x00b72b07u : 0x00b72a53u;
    a.source_map = NativeStreamCloneMapPhase::unlock_in_progress;
    if (vertex) unlock_native_logical_vertex_stream_00b49a80(source, s.mapping);
    else unlock_native_logical_index_stream_00b49c70(source, s.mapping);
    a.source_map = NativeStreamCloneMapPhase::unlocked;
    slot(stream_profile(a.creator, vertex, s), vertex ? 0x14u : 0x10u,
        vertex ? 0x00b49a80u : 0x00b49c70u);
    a.phase = NativeStreamClonePhase::destination_unlock;
    a.native_site = vertex ? 0x00b72b10u : 0x00b72a5cu;
    a.destination_map = NativeStreamCloneMapPhase::unlock_in_progress;
    if (vertex) unlock_native_logical_vertex_stream_00b49a80(a.creator, s.mapping);
    else unlock_native_logical_index_stream_00b49c70(a.creator, s.mapping);
    a.destination_map = NativeStreamCloneMapPhase::unlocked;
    a.phase = NativeStreamClonePhase::complete;
}
} // namespace

void* clone_native_index_stream_00b729a0(void* source, NativeStreamCloneServices& s,
    NativeStreamCloneAcquired& a) {
    begin(source, false, s, a);
    void* const renderer = const_cast<void*>(s.vertices.actual_renderer_00f8d394);
    auto* renderer_table = capture_renderer(renderer, false, s); // captured BEFORE queries.
    const auto source_format = format(source, s); // B729B4
    const auto source_flags = flags(source, false, s); // B729BE
    const auto source_count = count(source, false, s); // B729C8
    slot(renderer_table, 0x60, 0x00b288b0);
    a.phase = NativeStreamClonePhase::factory;
    a.native_site = 0x00b729d0;
    (void)create_native_registered_index_stream_00b288b0(renderer, source_count,
        source_flags, source_format, s.indices, &a.creator);
    if (!a.creator) throw std::bad_alloc(); // Native then dereferences the null result.
    a.phase = NativeStreamClonePhase::registration;
    s.geometry.register_stream_clone_creator(a, s);
    map_pair(source, false, s, a);
    const auto current_format = format(source, s); // B72A0B, AFTER both maps.
    const auto width = current_format == 0x65u ? 2u : current_format == 0x66u ? 4u : 0u;
    copy_and_unlock(source, false, width, s, a);
    return a.creator;
}
void* clone_native_vertex_stream_00b72a70(void* source, NativeStreamCloneServices& s,
    NativeStreamCloneAcquired& a) {
    begin(source, true, s, a);
    void* const renderer = const_cast<void*>(s.vertices.actual_renderer_00f8d394);
    auto* renderer_table = capture_renderer(renderer, true, s);
    void* const source_declaration = declaration(source, s); // B72A84
    const auto source_flags = flags(source, true, s); // B72A8E
    const auto source_count = count(source, true, s); // B72A98
    slot(renderer_table, 0x5c, 0x00b287c0);
    a.phase = NativeStreamClonePhase::factory;
    a.native_site = 0x00b72aa0;
    (void)create_native_registered_vertex_stream_00b287c0(renderer, source_count,
        source_flags, source_declaration, s.vertices, &a.creator);
    if (!a.creator) throw std::bad_alloc();
    a.phase = NativeStreamClonePhase::registration;
    s.geometry.register_stream_clone_creator(a, s);
    map_pair(source, true, s, a);
    const auto stride = word(declaration(source, s), 0xcc); // B72ADB/ADD, after maps.
    copy_and_unlock(source, true, stride, s, a);
    return a.creator;
}
} // namespace bsp
