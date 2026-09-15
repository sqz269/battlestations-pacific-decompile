#include "bsp/native_mesh_subset_loading.hpp"
#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_camera_group_resource.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
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
namespace {
template<class T> T& append_frame(std::vector<std::unique_ptr<T>>& frames) {
    auto frame = std::make_unique<T>();
    frames.push_back(std::move(frame));
    return *frames.back();
}
// Native B94559/5D/64 rounds to float32, reloads, and stores the stack
// argument again. Keep both x87 stores before the B72710 MOVSS bit copy.
__declspec(naked) U __cdecl read_lod_bits(void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 4
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr [ebp-4]
        fld dword ptr [ebp-4]
        fstp dword ptr [ebp-4]
        mov eax, dword ptr [ebp-4]
        mov esp, ebp
        pop ebp
        ret
    }
}
void unwind_mesh_child(NativeMeshFieldsAcquired& a, NativeAdoptedSubstreamDispatch& streams) noexcept {
    a.child_cleanup_armed = false;
    release_native_structured_node_handle_00be9ed0(&a.child, streams);
}
void require_loading_domain(NativeMeshLoadingContext& c) {
    require_domain(c.subsets);
    if (&c.metadata.reads != &c.subsets.texture_fields.reads)
        throw std::invalid_argument("Native mesh fields require the same actual reader");
}
} // namespace

U native_mesh_section_count_00b72b40(const void* mesh) noexcept { return word(mesh, 0x58); }

void read_native_mesh_fields_00b944e0(void* pair, void* parent,
    NativeMeshLoadingContext& c, NativeMeshFieldsAcquired& a) {
    using FieldPhase = NativeMeshFieldsAcquired::Phase;
    if (a.phase != FieldPhase::empty || a.child || a.child_cleanup_armed ||
        !a.subsets.empty() || !a.buffers.empty() || !a.metadata.empty())
        throw std::logic_error("Native mesh fields cannot replay acquired children");
    require_loading_domain(c);
    auto& reads = c.subsets.texture_fields.reads;
    NativeMeshBufferReadContext buffers{reads, c.subsets.generators.context().graphics};
    a.phase = FieldPhase::prefix; a.native_site = 0x00b94500;
    put(pair, 4, read_native_resource_node_control_dword_00be99f0(parent, reads));
    while (native_resource_node_has_remaining_00715bf0(parent)) {
        a.phase = FieldPhase::child; a.native_site = 0x00b9451f;
        create_native_resource_child_00bea680(parent, &a.child, reads);
        a.child_cleanup_armed = true;
        try {
            a.phase = FieldPhase::field;
            if (child_named(a.child, "LODValue")) {
                void* captured_mesh = ptr(word(pair)); // ESI before the scalar read.
                a.native_site = 0x00b94554;
                const auto value = read_lod_bits(&a.child, reads);
                a.native_site = 0x00b94567;
                set_native_mesh_lod_00b72710(captured_mesh, value);
            } else if (child_named(a.child, "LODPhases")) {
                const auto constants = c.subsets.geometry.mesh_constants();
                a.native_site = 0x00b9459d;
                read_native_mesh_lod_phases_00b93710(ptr(word(pair)), &a.child, reads,
                    constants.minimum_00ce4adc, constants.maximum_00ce4970);
            } else if (child_named(a.child, "Subset")) {
                auto& frame = append_frame(a.subsets);
                a.native_site = 0x00b945d3;
                read_native_mesh_subset_00b941d0(ptr(word(pair)), &a.child, c.subsets, frame);
            } else if (child_named(a.child, "Indices")) {
                auto& frame = append_frame(a.buffers);
                a.native_site = 0x00b94609;
                read_native_mesh_indices_00b93aa0(ptr(word(pair)), &a.child, buffers, frame);
            } else if (equal_native_string_header_00425850(at(a.child, 0x10), "VertexStream")) {
                auto& frame = append_frame(a.buffers);
                a.native_site = 0x00b94632;
                read_native_mesh_vertex_stream_00b93e60(ptr(word(pair)), &a.child, buffers, frame);
            } else if (equal_native_string_header_00425850(at(a.child, 0x10), "CompressedVertexFormatData")) {
                auto& frame = append_frame(a.metadata);
                a.native_site = 0x00b9465b;
                read_native_mesh_compressed_format_00b93800(ptr(word(pair)), &a.child, c.metadata, frame);
            } else if (equal_native_string_header_00425850(at(a.child, 0x10), "BoundingSphere")) {
                a.native_site = 0x00b94681;
                consume_native_mesh_sphere_00b93590(ptr(word(pair)), &a.child, reads);
            } else if (equal_native_string_header_00425850(at(a.child, 0x10), "BoundingBox")) {
                a.native_site = 0x00b946a7;
                consume_native_mesh_bounds_00b935c0(ptr(word(pair)), &a.child, reads);
            } else if (equal_native_string_header_00425850(at(a.child, 0x10), "WeightMapNames")) {
                a.native_site = 0x00b946cd;
                read_native_mesh_weight_names_00b93f90(ptr(word(pair)), &a.child, reads);
            } else {
                a.native_site = 0x00b946d4;
                skip_native_resource_node_00be9c40(&a.child, reads);
            }
        } catch (...) {
            unwind_mesh_child(a, reads.streams);
            throw;
        }
        a.child_cleanup_armed = false;
        a.phase = FieldPhase::child_release; a.native_site = 0x00b946e5;
        release_native_structured_node_handle_00be9ed0(&a.child, reads.streams);
    }
    a.phase = FieldPhase::complete;
}

void* construct_native_mesh_from_node_00b94710(void* pair, void* parent,
    NativeMeshLoadingContext& c, NativeMeshLoadingAcquired& a) {
    using LoadPhase = NativeMeshLoadingAcquired::Phase;
    if (a.phase != LoadPhase::empty || a.mesh.started ||
        a.fields.phase != NativeMeshFieldsAcquired::Phase::empty || a.poll_iterations)
        throw std::logic_error("Native mesh construction cannot replay its acquired operation");
    require_loading_domain(c);
    a.phase = LoadPhase::construction; a.native_site = 0x00b9472f;
    (void)c.subsets.geometry.create_native_mesh_and_publish(pair, a.mesh);
    a.phase = LoadPhase::fields; a.native_site = 0x00b9475e;
    read_native_mesh_fields_00b944e0(pair, parent, c, a.fields);
    a.phase = LoadPhase::polling; a.native_site = 0x00b94765;
    if (native_mesh_section_count_00b72b40(ptr(word(pair))) != 0) {
        U index = 0;
        U current_count;
        do {
            void* current_mesh = ptr(word(pair));
            ++index;
            a.poll_iterations = index;
            a.native_site = 0x00b94775;
            current_count = native_mesh_section_count_00b72b40(current_mesh);
        } while (index < current_count);
    }
    a.phase = LoadPhase::complete;
    return ptr(word(pair));
}
namespace {
using ResourcePhase = NativeMeshResourceAcquired::Phase;
void unwind_mesh_resource(NativeMeshResourceAcquired& a, int state,
    NativeRenderActualOwners& owners) noexcept {
    if (state == 1 && a.item) {
        singleton_lifetime_free(a.item); a.item = nullptr;
    }
    if (state >= 0) {
        // Native state0 is armed before B94710 writes the stack pair. Host
        // metadata failures may also leave a completed unregistered creator.
        // Neither case licenses dereferencing an uninitialized/stale identity
        // or retrying canonical admission as though it were native behavior.
        if (!a.loading.mesh.published ||
            (word(a.pair) && !a.loading.mesh.registered)) {
            a.cleanup_deferred = true;
            return;
        }
        release_native_mesh_handle_00b93ba0(a.pair, owners);
        a.mesh_creator_consumed = true;
    }
}
void* parse_mesh_resource(void* handle, NativeMeshLoadingContext& c,
    NativeMeshResourceAcquired& a, U entry, U profile) {
    if (a.phase != ResourcePhase::empty || a.loading.mesh.started || a.item)
        throw std::logic_error("Native mesh resource parsing requires one fresh frame");
    require_loading_domain(c);
    auto& owners = c.subsets.geometry.actual_owners();
    const U extra = profile == 0x00d63738 ? 0u : 6u;
    int state = 0;
    try {
        a.phase = ResourcePhase::mesh; a.native_site = entry + 0x2b;
        (void)construct_native_mesh_from_node_00b94710(a.pair, handle, c, a.loading);
        a.phase = ResourcePhase::allocation; a.native_site = entry + 0x32;
        a.item = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
        a.captured_mesh = ptr(word(a.pair));
        state = 1;
        if (a.item) {
            a.phase = ResourcePhase::construction; a.native_site = entry + 0x4f;
            construct_native_resource_item_base_00b868b0(a.item);
            put(a.item, 0, 0x00d63738);
            put(a.item, 8, bits(a.captured_mesh)); put(a.item, 0xc, word(a.pair, 4));
            a.native_site = entry + 0x68;
            static_cast<std::atomic<std::int32_t>*>(at(a.captured_mesh, 4))->fetch_add(
                1, std::memory_order_seq_cst);
            a.item_retained_mesh = true;
            if (extra) put(a.item, 0, profile);
        }
        state = -1;
    } catch (...) { unwind_mesh_resource(a, state, owners); throw; }
    void* const result = a.item; // Native ESI survives terminal callbacks.
    a.phase = ResourcePhase::mesh_release; a.native_site = entry + 0x82 + extra;
    // Native disarmed normal release uses captured EDI and does not clear the
    // local pair. A throwing terminal is not retried by an enclosing cleanup.
    a.mesh_creator_consumed = true;
    if (a.captured_mesh) release_native_render_actual_owner(owners, a.captured_mesh);
    a.phase = ResourcePhase::complete;
    return result;
}
void* delete_mesh_resource(void* item, U flags, NativeRenderActualOwners& owners) {
    destroy_native_mesh_item_00b93910(item, owners);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}
}
void* parse_native_mesh_item_00b947a0(void* h, NativeMeshLoadingContext& c, NativeMeshResourceAcquired& a) {
    return parse_mesh_resource(h, c, a, 0x00b947a0, 0x00d63738);
}
void* parse_native_mesh_item_00b94850(void* h, NativeMeshLoadingContext& c, NativeMeshResourceAcquired& a) {
    return parse_mesh_resource(h, c, a, 0x00b94850, 0x00d6375c);
}
void* parse_native_mesh_item_00b94900(void* h, NativeMeshLoadingContext& c, NativeMeshResourceAcquired& a) {
    return parse_mesh_resource(h, c, a, 0x00b94900, 0x00d63780);
}
void release_native_mesh_handle_00b93ba0(void* handle, NativeRenderActualOwners& owners) {
    if (void* mesh = ptr(word(handle))) {
        release_native_render_actual_owner(owners, mesh);
        put(handle, 0, 0);
    }
}
void destroy_native_mesh_item_00b93910(void* item, NativeRenderActualOwners& owners) {
    put(item, 0, 0x00d63738);
    try { release_native_render_actual_owner(owners, ptr(word(item, 8))); }
    catch (...) { destroy_native_resource_item_base_00b86890(item); throw; }
    destroy_native_resource_item_base_00b86890(item);
}
void* delete_native_mesh_item_00b93b40(void* p, U f, NativeRenderActualOwners& o) { return delete_mesh_resource(p, f, o); }
void* delete_native_mesh_item_00b93b60(void* p, U f, NativeRenderActualOwners& o) { return delete_mesh_resource(p, f, o); }
void* delete_native_mesh_item_00b93b80(void* p, U f, NativeRenderActualOwners& o) { return delete_mesh_resource(p, f, o); }
NativeMeshResourceCalls::NativeMeshResourceCalls(NativeResourceDispatchCalls& other, NativeMeshLoadingContext& loading)
    : other_(other), loading_(loading) {}
void NativeMeshResourceCalls::renderer_hook(std::uintptr_t e, void* r) { other_.renderer_hook(e, r); }
void* NativeMeshResourceCalls::parse_item(std::uintptr_t e, void* parser, void* handle) {
    if (e != 0x00b947a0 && e != 0x00b94850 && e != 0x00b94900)
        return other_.parse_item(e, parser, handle);
    auto a = std::make_unique<NativeMeshResourceAcquired>();
    acquisitions_.push_back(std::move(a));
    auto& frame = *acquisitions_.back();
    if (e == 0x00b947a0) return parse_native_mesh_item_00b947a0(handle, loading_, frame);
    if (e == 0x00b94850) return parse_native_mesh_item_00b94850(handle, loading_, frame);
    return parse_native_mesh_item_00b94900(handle, loading_, frame);
}
void NativeMeshResourceCalls::append_item(std::uintptr_t e, void* r, void* i) { other_.append_item(e, r, i); }
const std::vector<std::unique_ptr<NativeMeshResourceAcquired>>& NativeMeshResourceCalls::acquisitions() const noexcept {
    return acquisitions_;
}
NativeMeshResourceReferences::NativeMeshResourceReferences(NativeAdoptedSubstreamDispatch& other,
    NativeRenderActualOwners& owners) : other_(other), owners_(owners) {}
std::uint8_t NativeMeshResourceReferences::source_is_open(std::uintptr_t e, void* p) { return other_.source_is_open(e, p); }
U NativeMeshResourceReferences::source_seek(std::uintptr_t e, void* p, U lo, U hi, U origin) { return other_.source_seek(e, p, lo, hi, origin); }
void NativeMeshResourceReferences::source_read(std::uintptr_t e, void* p, void* d, U n, U* a) { other_.source_read(e, p, d, n, a); }
void NativeMeshResourceReferences::source_write(std::uintptr_t e, void* p, const void* d, U n, U* a) { other_.source_write(e, p, d, n, a); }
void NativeMeshResourceReferences::source_zero_reference(std::uintptr_t e, void* p, std::uintptr_t table) {
    if (e == 0x00bd30e0 && (table == 0x00d63738 || table == 0x00d6375c || table == 0x00d63780)) {
        if (!p) return;
        const U target = word(ptr(word(p)), 4);
        if (target == 0x00b93b40) delete_native_mesh_item_00b93b40(p, 1, owners_);
        else if (target == 0x00b93b60) delete_native_mesh_item_00b93b60(p, 1, owners_);
        else if (target == 0x00b93b80) delete_native_mesh_item_00b93b80(p, 1, owners_);
        else throw std::runtime_error("Reached mesh resource scalar deletion target is not reconstructed");
        return;
    }
    other_.source_zero_reference(e, p, table);
}
NativeMeshResourceTypeIds::NativeMeshResourceTypeIds(TypeIdCounterLifetime& counter,
    LightTypeBootstrap& root, NativeMeshResourceTypeStorage storage) noexcept
    : counter_(counter), root_(root), storage_(storage) {}
U NativeMeshResourceTypeIds::consume_id() {
    volatile auto* owner = counter_.get_006fac20();
    const U id = owner->next_id_04;
    owner->next_id_04 = id + 1u;
    return id;
}
void NativeMeshResourceTypeIds::initialize_scene_resource_00b869c0(volatile U* target) {
    if (storage_.scene_guard_0109020c == 0) {
        storage_.scene_guard_0109020c = 1;
        target[2] = 0x00d631e4;
        auto& root = root_.storage().root_0109db84;
        root_.initialize_root_00bea780(root);
        target[1] = root.own_id;
        target[0] = consume_id();
    }
}
void NativeMeshResourceTypeIds::initialize_mesh_resource_00b93bd0(volatile U* target) {
    if (storage_.mesh_guard_01090440 == 0) {
        storage_.mesh_guard_01090440 = 1;
        target[3] = 0x00d637a4;
        initialize_scene_resource_00b869c0(storage_.scene_01090210);
        // Runtime entry alternates each source load and destination store.
        target[1] = storage_.scene_01090210[0];
        target[2] = storage_.scene_01090210[1];
        target[0] = consume_id();
    }
}
void NativeMeshResourceTypeIds::initialize_mesh_after_guard_check() {
    storage_.mesh_guard_01090440 = 1;
    storage_.mesh_01090444[3] = 0x00d637a4;
    initialize_scene_resource_00b869c0(storage_.scene_01090210);
    // CRT form captures both source words before either destination store.
    const U scene = storage_.scene_01090210[0];
    const U root = storage_.scene_01090210[1];
    storage_.mesh_01090444[1] = scene;
    storage_.mesh_01090444[2] = root;
    storage_.mesh_01090444[0] = consume_id();
}
void NativeMeshResourceTypeIds::initialize_mesh_resource_00cd8690() {
    if (storage_.mesh_guard_01090440 == 0) initialize_mesh_after_guard_check();
}
void NativeMeshResourceTypeIds::initialize_derived(volatile std::uint8_t& guard,
    volatile U* descriptor, U name) {
    if (guard != 0) return;
    const bool initialize_mesh = storage_.mesh_guard_01090440 == 0;
    guard = 1;
    descriptor[4] = name;
    if (initialize_mesh) initialize_mesh_after_guard_check();
    const U mesh = storage_.mesh_01090444[0];
    const U scene = storage_.mesh_01090444[1];
    const U root = storage_.mesh_01090444[2];
    descriptor[1] = mesh; descriptor[2] = scene; descriptor[3] = root;
    descriptor[0] = consume_id();
}
void NativeMeshResourceTypeIds::initialize_skined_mesh_resource_00cd86f0() {
    initialize_derived(storage_.skined_guard_01090441, storage_.skined_01090454, 0x00d637b8);
}
void NativeMeshResourceTypeIds::initialize_matrix_mesh_resource_00cd87b0() {
    initialize_derived(storage_.matrix_guard_01090442, storage_.matrix_01090468, 0x00d637cc);
}
U read_native_mesh_resource_type_00b93000(const volatile U& v) { return v; }
U read_native_mesh_resource_name_00b93010(const volatile U& v) { return v; }
U read_native_mesh_resource_type_00b930d0(const volatile U& v) { return v; }
U read_native_mesh_resource_name_00b930e0(const volatile U& v) { return v; }
U read_native_mesh_resource_type_00b931d0(const volatile U& v) { return v; }
U read_native_mesh_resource_name_00b931e0(const volatile U& v) { return v; }
std::uint8_t matches_native_mesh_resource_type_00b936b0(U token, const volatile U* ids) {
    for (unsigned i = 0; i != 3; ++i) if (ids[i] == token) return 1;
    return 0;
}
std::uint8_t matches_native_mesh_resource_type_00b939c0(U token, const volatile U* ids) {
    for (unsigned i = 0; i != 4; ++i) if (ids[i] == token) return 1;
    return 0;
}
std::uint8_t matches_native_mesh_resource_type_00b93a40(U token, const volatile U* ids) {
    for (unsigned i = 0; i != 4; ++i) if (ids[i] == token) return 1;
    return 0;
}
NativeMeshResourceTypeCalls::NativeMeshResourceTypeCalls(NativeResourceItemTypeCalls& other,
    NativeMeshResourceTypeStorage storage) noexcept : other_(other), storage_(storage) {}
std::uint8_t NativeMeshResourceTypeCalls::matches_type(std::uintptr_t target, void* item, U token) {
    if (target == 0x00b936b0) return matches_native_mesh_resource_type_00b936b0(token, storage_.mesh_01090444);
    if (target == 0x00b939c0) return matches_native_mesh_resource_type_00b939c0(token, storage_.skined_01090454);
    if (target == 0x00b93a40) return matches_native_mesh_resource_type_00b93a40(token, storage_.matrix_01090468);
    return other_.matches_type(target, item, token);
}
NativeGameResourceSelectorTypes::NativeGameResourceSelectorTypes(TypeIdCounterLifetime& counter,
    NativeMeshResourceTypeIds& scene, NativeGameResourceSelectorStorage storage) noexcept
    : counter_(counter), scene_(scene), storage_(storage) {}
void NativeGameResourceSelectorTypes::initialize(volatile std::uint8_t& guard,
    volatile U* descriptor, U name) {
    if (guard != 0) return;
    guard = 1;
    descriptor[3] = name;
    auto* parent = scene_.storage().scene_01090210;
    scene_.initialize_scene_resource_00b869c0(parent);
    const U scene = parent[0];
    const U root = parent[1];
    descriptor[1] = scene; descriptor[2] = root;
    volatile auto* counter = counter_.get_006fac20();
    const U id = counter->next_id_04;
    counter->next_id_04 = id + 1u;
    descriptor[0] = id;
}
void NativeGameResourceSelectorTypes::initialize_convex_object_00ccec00() {
    initialize(storage_.convex_guard_00e19a94, storage_.convex_00e19a98, 0x00cfb6c8);
}
void NativeGameResourceSelectorTypes::initialize_aux_00ccf740() {
    initialize(storage_.aux_guard_00e19b51, storage_.aux_00e19b64, 0x00cfd880);
}
void NativeGameResourceSelectorTypes::initialize_geom_mesh_00ccf980() {
    initialize(storage_.geom_mesh_guard_00e19bd4, storage_.geom_mesh_00e19be4, 0x00cfdbbc);
}
} // namespace bsp
