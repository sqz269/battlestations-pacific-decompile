#include "bsp/native_mesh_subset_loading.hpp"
#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh subset loading requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Phase = NativeMeshSubsetAcquired::Phase;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U v) noexcept { *static_cast<volatile U*>(at(p, offset)) = v; }
bool child_named(void* child, const char* expected) {
    const auto* name = static_cast<const char*>(ptr(word(child, 0x14)));
    return name && _stricmp(name, expected) == 0;
}
void return_name(void* data, U bytes, NativeStringRawPoolContext& strings) {
    auto* pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
}
void unwind_name(NativeMeshSubsetAcquired& a, NativeStringRawPoolContext& strings) noexcept {
    a.name_cleanup_armed = false;
    if (void* data = ptr(word(&a.name, 4))) return_name(data, word(&a.name) + 1u, strings);
    a.name_returned = true;
}
void unwind_child(NativeMeshSubsetAcquired& a, NativeAdoptedSubstreamDispatch& streams) noexcept {
    a.child_cleanup_armed = false;
    release_native_structured_node_handle_00be9ed0(&a.child, streams);
}
// Preserve direct FSTP ST0 at each native site, without a float32 spill.
__declspec(naked) void __cdecl discard_scalar(void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp st(0)
        pop ebp
        ret
    }
}
void require_domain(NativeMeshSubsetLoadingContext& c) {
    auto& actual = c.geometry.actual_owners();
    if (&c.materials.retained_owners != &actual || &c.effects.effects.owners.actual_owners() != &actual ||
        &c.texture_fields.owners != &actual ||
        &c.generators.context().graphics.streams.geometry != &c.geometry ||
        &c.effects.strings != &c.texture_fields.textures.strings ||
        &c.materials.parameter_names != &c.effects.strings)
        throw std::invalid_argument("Native subset requires shared actual cache, geometry and material domains");
    require_gui_text_native_renderer_domain(c.generators.context().graphics,
        c.effects.effects.construction.current_renderer_00f8d394, c.effects.strings, actual);
}
} // namespace

void read_native_mesh_subset_00b941d0(void* mesh, void* parent,
    NativeMeshSubsetLoadingContext& c, NativeMeshSubsetAcquired& a) {
    if (a.phase != Phase::empty || a.section.started || a.factory.phase != NativeMaterialFactoryAcquired::Phase::empty ||
        a.name_completed || a.child || !a.texture_children.empty() || a.generator.attachment_started)
        throw std::logic_error("Native subset cannot replay an acquired operation");
    require_domain(c);
    auto& reads = c.texture_fields.reads;
    auto& owners = c.geometry.actual_owners();
    a.phase = Phase::section; a.native_site = 0x00b941ee;
    auto* section = c.geometry.create_native_section_00533fa0(a.section);
    a.captured_section = section;
    a.phase = Phase::prefix; a.native_site = 0x00b941fb;
    const U primitive = read_native_resource_node_control_dword_00be99f0(parent, reads);
    U mapped = 1;
    switch (primitive) { case 1: mapped = 3; break; case 2: mapped = 2; break;
        case 3: mapped = 5; break; case 4: mapped = 6; break; case 5: mapped = 4; break; default: break; }
    put(section, 8, mapped);
    constexpr U prefix_sites[] = {0x00b94239,0x00b94243,0x00b9424d,0x00b94257};
    for (U i = 0; i != 4; ++i) {
        a.native_site = prefix_sites[i];
        put(section, 0xcu + i * 4u, read_native_resource_node_control_dword_00be99f0(parent, reads));
    }
    a.phase = Phase::name; a.native_site = 0x00b94266;
    (void)read_native_resource_handle_string_00bea010(parent, &a.name, reads);
    a.name_completed = true; a.name_cleanup_armed = true;
    try {
        a.phase = Phase::alias; a.native_site = 0x00b94281;
        const auto* name_data = static_cast<const char*>(ptr(word(&a.name, 4)));
        if (name_data && _stricmp(name_data, "soldiers.mshd") == 0) {
            a.native_site = 0x00b9429a;
            resize_native_string_header_0041dd40(&a.name, c.effects.strings, 12, false);
            if (void* data = ptr(word(&a.name, 4))) std::memcpy(data, "soldier.mshd", word(&a.name));
        }
        a.phase = Phase::material; a.native_site = 0x00b942bf;
        auto* material = create_native_material_from_effect_cache_00535320(a.name,
            c.materials.material_slots, owners, c.effects, a.factory);
        a.captured_material = material;
        a.phase = Phase::material_admission;
        c.geometry.register_native_material_creator(a.factory, c.materials, c.material_profile_00d5e520);
        a.phase = Phase::material_assignment; a.native_site = 0x00b942cd;
        set_native_mesh_section_material_00b864c0(*section, owners, material);
        a.phase = Phase::material_release; a.native_site = 0x00b942d6;
        a.material_creator_consumed = true; a.factory.material = nullptr;
        release_native_render_actual_owner(owners, material);
        a.phase = Phase::streams_clear; a.native_site = 0x00b942eb;
        clear_native_mesh_section_vertex_streams_00b86550(*section, owners);
        while (native_resource_node_has_remaining_00715bf0(parent)) {
            a.phase = Phase::child; a.native_site = 0x00b9430a;
            create_native_resource_child_00bea680(parent, &a.child, reads);
            a.child_cleanup_armed = true;
            try {
                a.phase = Phase::child_fields;
                if (child_named(a.child, "Texture")) {
                    auto frame = std::make_unique<NativeMeshTextureFieldAcquired>();
                    a.texture_children.push_back(std::move(frame));
                    a.native_site = 0x00b94344;
                    read_native_mesh_texture_field_00b93d30(material, &a.child, c.texture_fields, *a.texture_children.back());
                } else if (child_named(a.child, "LightingSettings")) {
                    a.native_site = 0x00b9437e;
                    read_native_material_lighting_field_00b937a0(material, &a.child, reads, c.lighting);
                } else if (child_named(a.child, "BoundingSphere")) {
                    for (U site : {0x00b943aeu,0x00b943b9u,0x00b943c4u,0x00b943cfu}) {
                        a.native_site = site; discard_scalar(&a.child, reads);
                    }
                } else if (child_named(a.child, "VertexStreamIndex")) {
                    a.native_site = 0x00b943fe;
                    const auto index = read_native_resource_node_control_dword_00be99f0(&a.child, reads);
                    a.native_site = 0x00b94406;
                    void* stream = native_mesh_vertex_stream_unchecked_00b73260(mesh, index);
                    a.native_site = 0x00b9440e;
                    append_native_mesh_section_vertex_stream_00b85b80(*section, stream);
                } else {
                    a.native_site = 0x00b94419; skip_native_resource_node_00be9c40(&a.child, reads);
                }
            } catch (...) {
                unwind_child(a, reads.streams);
                throw;
            }
            a.child_cleanup_armed = false;
            a.phase = Phase::child_release; a.native_site = 0x00b94427;
            release_native_structured_node_handle_00be9ed0(&a.child, reads.streams);
        }
        if (word(section, 0x4c) == 0) {
            a.phase = Phase::default_stream; a.native_site = 0x00b94445;
            void* stream = native_mesh_vertex_stream_unchecked_00b73260(mesh, 0);
            a.native_site = 0x00b9444d; append_native_mesh_section_vertex_stream_00b85b80(*section, stream);
        }
        a.phase = Phase::layout; a.native_site = 0x00b94455;
        rebuild_native_mesh_section_vertex_layout_00b865a0(*section, owners, mesh, c.generators.context().layouts);
        a.phase = Phase::mesh_publication; a.native_site = 0x00b9445d;
        append_native_mesh_draw_section_00b73c60(*static_cast<NativeMeshStorage*>(mesh), section);
        a.section_published = true;
        a.phase = Phase::generator; a.native_site = 0x00b94468;
        finalize_native_mesh_section_generator_00b85610(section, ptr(word(mesh, 0x60)), c.generators, a.generator);
        a.phase = Phase::section_release; a.native_site = 0x00b94471;
        a.section_creator_consumed = true; a.section.creator = nullptr;
        release_native_render_actual_owner(owners, section);
        void* captured_name = ptr(word(&a.name, 4));
        a.name_cleanup_armed = false;
        a.phase = Phase::name_return; a.native_site = 0x00b944a8;
        if (captured_name) return_name(captured_name, word(&a.name) + 1u, reads.strings);
        a.name_returned = true; a.phase = Phase::complete;
    } catch (...) {
        if (a.name_cleanup_armed) unwind_name(a, reads.strings);
        throw;
    }
}
} // namespace bsp
