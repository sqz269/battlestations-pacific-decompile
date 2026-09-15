#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native post-effect construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4 && alignof(std::atomic<std::int32_t>) == 4);
static_assert(sizeof(NativeString) == 8);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
void* allocate(Word bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void require_renderer(void* actual, const NativePostEffect20ConstructionContext& context,
    Word slot, Word callee) {
    require(actual && word(actual) == 0x00d5f0a8u &&
        word(context.renderer_profile_00d5f0a8, slot) == callee,
        "post-effect constructor requires its current concrete renderer slot");
}
template<class Reference>
void release_registered(void* actual, NativePostEffect20Decrement decrement,
    NativePostEffect20ConstructionContext& context, Word profile,
    const volatile Word* table, Word deleting) {
    require(decrement != nullptr, "missing current CE2220 decrement");
    if (decrement(reinterpret_cast<volatile long*>(bits(actual) + 4u)) != 0) return;
    require(word(actual) == profile && table && word(table) == 0x00bd30e0u,
        "unsupported current post-effect creator virtual zero");
    require(word(actual) == profile && word(table, 4) == deleting,
        "unsupported current post-effect creator deleting slot");
    auto& reference = context.destruction.actual_owners.resolve_actual(actual);
    auto* concrete = dynamic_cast<Reference*>(&reference);
    require(concrete && &reference.reference_count ==
        reinterpret_cast<std::atomic<std::int32_t>*>(bits(actual) + 4u),
        "post-effect creator lacks its canonical concrete counter companion");
    concrete->release_zero_references(); // BD30E0 does not decrement again.
}
NativeModelOwner& current_model(void* actual, NativePostEffect20ConstructionContext& context) {
    auto* binding = context.destruction.nodes.attachments.find_actual_node(bits(actual));
    auto* reference = dynamic_cast<NativeModelReference*>(binding);
    require(reference && &reference->model_owner().environment == &context.models,
        "current post-effect model is outside its canonical domain");
    return reference->model_owner();
}
NativeCameraOwner& current_camera(void* actual, NativePostEffect20ConstructionContext& context) {
    auto* binding = context.destruction.nodes.attachments.find_actual_node(bits(actual));
    auto* reference = dynamic_cast<NativeCameraReference*>(binding);
    require(reference && &reference->camera_owner().environment == &context.cameras,
        "current post-effect camera is outside its canonical domain");
    return reference->camera_owner();
}
// B3CD20/B3CD10, each MOV EAX,[ECX+offset]; RET (four bytes).
Word surface_height_00b3cd20(const void* actual) noexcept { return word(actual, 0x20); }
Word surface_width_00b3cd10(const void* actual) noexcept { return word(actual, 0x1c); }
Word surface_dimension(const void* actual, NativePostEffect20ConstructionContext& context, bool height) {
    const Word slot = height ? 0x20u : 0x1cu;
    const Word callee = height ? 0x00b3cd20u : 0x00b3cd10u;
    require(word(actual) == 0x00d619a0u && word(context.surface_profile_00d619a0, slot) == callee,
        "optional post-effect size input requires its current D619A0 dimension slot");
    return height ? surface_height_00b3cd20(actual) : surface_width_00b3cd10(actual);
}
}

NativePostEffect20ConstructionBlock::~NativePostEffect20ConstructionBlock() noexcept {
    if (phase_ != Phase::idle) std::terminate();
}
void NativePostEffect20ConstructionBlock::validate() const {
    auto& c = *context_;
    auto& owners = c.destruction.actual_owners;
    require(&c.meshes.retained_owners == &owners && &c.sections.retained_owners == &owners &&
        &c.materials.retained_owners == &owners && &c.streams.actual_owners == &owners &&
        &c.models.retained_owners == &owners && &c.declaration_companions.actual_owners() == &owners,
        "post-effect children require one canonical actual owner domain");
    require(&c.models.nodes == &c.destruction.nodes && &c.cameras.nodes == &c.destruction.nodes &&
        &c.destruction.nodes.attachments.scenes == &c.destruction.nodes.scenes &&
        c.destruction.nodes.node_virtual_0c && &c.cameras.viewport_views == &c.viewports,
        "post-effect nodes and viewport records require the installed canonical runtime");
    require(&c.models.nodes.require_raw_name_pool() == &c.raw_strings && !c.models.actual_names &&
        &c.models.constants.maximum_00ce4970 == &c.node_constants.positive_bound_00ce4970 &&
        &c.models.constants.minimum_00ce4adc == &c.node_constants.negative_bound_00ce4adc &&
        &c.mesh_constants.maximum_00ce4970 == &c.node_constants.positive_bound_00ce4970 &&
        &c.mesh_constants.minimum_00ce4adc == &c.node_constants.negative_bound_00ce4adc &&
        &c.mesh_constants.lod_00d7a24c == &c.node_constants.one_00d7a24c &&
        &c.section_bounds_w_00ce4970 == &c.node_constants.positive_bound_00ce4970 &&
        &c.cameras.viewport.one_bits_00d7a24c == &c.node_constants.one_00d7a24c,
        "post-effect construction requires persistent raw names and shared native constant cells");
    require(c.renderer_profile_00d5f0a8 && c.surface_profile_00d619a0 && c.viewport_profile_00d5e5f8 &&
        c.vertex_format_00d61ef8 && c.model_name_00d61ee4 && c.camera_prefix_00d61ed0 &&
        c.cameras.crt.dispatch_bypass_0109dd78 && c.cameras.crt.except_00c27489 &&
        c.cameras.vtable_00d62cf0 && c.cameras.vtable_00d62c88,
        "post-effect construction requires actual table, literal and CRT bindings");
    require(reinterpret_cast<const volatile void*>(&c.actual_renderer_00f8d394) ==
        reinterpret_cast<const volatile void*>(&c.streams.actual_renderer_00f8d394) &&
        c.renderer_profile_00d5f0a8 == c.streams.actual_renderer_profile_00d5f0a8 &&
        c.streams.actual_logical_profile_00d61d6c &&
        c.streams.actual_type_sizes_00d61cc0 == c.declarations.declarations.type_sizes_00d61cc0 &&
        c.surface_profile_00d619a0 == c.destruction.frame_targets.actual_surface_profile_00d619a0 &&
        &c.strings == &c.declarations.strings,
        "post-effect providers must borrow the same renderer and string domain");
}
void NativePostEffect20ConstructionBlock::prepare(NativePostEffect20ConstructionContext& context) {
    require(phase_ == Phase::idle, "post-effect block requires idle external storage");
    context_ = &context;
    phase_ = Phase::preparing;
    try {
        validate();
        camera_scene_ = context.cameras.nodes.scenes.reserve_binding();
        camera_lifetime_ = context.cameras.nodes.attachments.reserve_binding();
        viewport_admissions_[0] = context.viewports.admit(viewport_records_[0]);
        viewport_admissions_[1] = context.viewports.admit(viewport_records_[1]);
        phase_ = Phase::prepared;
    } catch (...) { cancel_preparation(); throw; }
}
void NativePostEffect20ConstructionBlock::cancel_preparation() noexcept {
    if (phase_ != Phase::preparing && phase_ != Phase::prepared) std::terminate();
    camera_scene_.cancel(); camera_lifetime_.cancel();
    for (auto& admission : viewport_admissions_) admission.cancel();
    for (auto& record : viewport_records_) context_->viewports.forget_quiescent(record);
    context_ = nullptr;
    phase_ = Phase::idle;
}
void NativePostEffect20ConstructionBlock::settle() noexcept {
    camera_scene_.cancel(); camera_lifetime_.cancel();
    for (auto& admission : viewport_admissions_) admission.cancel();
    phase_ = Phase::settled;
}
void NativePostEffect20ConstructionBlock::bind(Slot slot, void* actual, RenderCommandReference& reference) {
    keys_[slot] = actual;
    references_[slot] = &reference;
    const auto& registration = context_->declaration_companions.registration();
    registration.bind(registration.context, actual, reference);
    registered_[slot] = true;
}
void NativePostEffect20ConstructionBlock::retired(Slot slot, RenderCommandReference& reference) noexcept {
    if (references_[slot] != &reference || retired_[slot]) std::terminate();
    if (registered_[slot]) {
        const auto& registration = context_->declaration_companions.registration();
        registration.unbind(registration.context, keys_[slot], reference);
        registered_[slot] = false;
    }
    retired_[slot] = true;
    // Keep captured keys as diagnostic identities; never read ended storage.
}
#define BSP_BR_RETIRE(name, type, slot) \
void NativePostEffect20ConstructionBlock::name(void* block, type& reference) noexcept { \
    static_cast<NativePostEffect20ConstructionBlock*>(block)->retired(slot, reference); \
}
BSP_BR_RETIRE(retire_mesh, NativeMeshReference, mesh)
BSP_BR_RETIRE(retire_section, NativeMeshSectionReference, section)
BSP_BR_RETIRE(retire_material, NativeMaterialReference, material)
BSP_BR_RETIRE(retire_stream, NativeLogicalVertexReference, stream)
BSP_BR_RETIRE(retire_model, NativeModelReference, model)
BSP_BR_RETIRE(retire_camera, NativeCameraReference, camera)
BSP_BR_RETIRE(retire_owner, NativePostEffect20Reference, owner)
#undef BSP_BR_RETIRE

void NativePostEffect20ConstructionBlock::reset_after_host_quiescence() noexcept {
    if (phase_ != Phase::settled) std::terminate();
    for (std::size_t i = 0; i != slot_count; ++i)
        if (references_[i] && !retired_[i]) std::terminate();
    if ((model_owner_ && model_owner_->phase != NativeModelOwner::Phase::dead) ||
        (camera_owner_ && camera_owner_->phase != NativeCameraOwner::Phase::dead) ||
        (owner_ && owner_->phase() != NativePostEffect20Owner::Phase::dead)) std::terminate();
    for (auto& record : viewport_records_) {
        const auto phase = record.phase();
        if (phase != NativeViewportRegistry::Phase::unused && phase != NativeViewportRegistry::Phase::cancelled &&
            phase != NativeViewportRegistry::Phase::retired) std::terminate();
    }
    owner_reference_.reset(); owner_.reset(); camera_reference_.reset(); camera_owner_.reset();
    model_reference_.reset(); model_owner_.reset(); stream_reference_.reset(); material_reference_.reset();
    section_reference_.reset(); mesh_reference_.reset();
    for (auto& record : viewport_records_) context_->viewports.forget_quiescent(record);
    declaration_name_.~NativeString(); new (&declaration_name_) NativeString;
    model_name_.~NativeString(); new (&model_name_) NativeString;
    camera_name_.~NativeString(); new (&camera_name_) NativeString;
    keys_ = {}; references_ = {}; registered_ = {}; retired_ = {}; acquired_ = {};
    context_ = nullptr;
    phase_ = Phase::idle;
}
void NativePostEffect20ConstructionBlock::string_cleanup(NativeString& header) {
    acquired_.last_string_return_data = pointer(word(&header, 4));
    acquired_.last_string_return_bytes = word(&header) + 1u;
    acquired_.string_return_started = true;
    destroy_native_string_header_0041dd20(&header, context_->raw_strings);
}
void NativePostEffect20ConstructionBlock::return_captured_string(void* data, Word bytes) {
    acquired_.last_string_return_data = data;
    acquired_.last_string_return_bytes = bytes;
    acquired_.string_return_started = true;
    auto& raw = context_->raw_strings;
    auto* pool = native_string_pool_get_or_create_00419cc0(raw.actual_published_01090aa8,
        raw.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, raw.actual_small_returns_disabled_01090aa4);
}
void NativePostEffect20ConstructionBlock::unwind(int& state, void*& frame_raw, void*& mesh_raw,
    void*& late_raw, Word& mask) {
    // DF86D4/DF86F8: transition before action; a second C++ cleanup failure
    // continues the remaining schedule without replaying a consumed action.
    // This explicit source policy propagates the newest cleanup exception;
    // native FH3 equivalence excludes this secondary-exception branch.
    while (state >= 0) {
        const int action = state;
        state = (action == 0) ? -1 : (action == 5 ? 4 : (action == 8 ? 7 : 0));
        acquired_.native_state = state;
        try {
            switch (action) {
            case 0: put(acquired_.actual_owner, 0, 0x00ceb130u); break; // BD30F0
            case 1: singleton_lifetime_free(std::exchange(frame_raw, nullptr)); break;
            case 2: string_cleanup(declaration_name_); break;
            case 3: return_native_mesh_slot_00b72f70(std::exchange(mesh_raw, nullptr)); break;
            case 4: {
                void* raw = std::exchange(late_raw, nullptr);
                model_owner_.reset(); acquired_.model_owner = nullptr;
                return_native_model_slot_00b748c0(raw); break;
            }
            case 5: case 6:
                if (mask & 1u) { mask &= ~1u; string_cleanup(model_name_); } break;
            case 7: {
                void* raw = std::exchange(late_raw, nullptr);
                camera_owner_.reset(); acquired_.camera_owner = nullptr;
                return_native_camera_slot_00b71350(raw); break;
            }
            case 8: case 9:
                if (mask & 2u) { mask &= ~2u; string_cleanup(camera_name_); } break;
            case 10: case 11: singleton_lifetime_free(std::exchange(late_raw, nullptr)); break;
            default: std::terminate();
            }
        } catch (...) { unwind(state, frame_raw, mesh_raw, late_raw, mask); throw; }
    }
}

void* construct_native_post_effect20_00b4e470(void* actual, std::size_t allocation_bytes,
    NativeString& effect_name, Word vertex_count, const void* optional_size_input,
    NativePostEffect20ConstructionBlock& block) {
    require(block.phase_ == NativePostEffect20ConstructionBlock::Phase::prepared && actual &&
        !(bits(actual) & 3u) && allocation_bytes >= 0x20u,
        "post-effect constructor requires prepared storage and an aligned actual 20h allocation");
    block.validate();
    auto& c = *block.context_;
    auto& acquired = block.acquired_;
    acquired.actual_owner = actual;
    block.phase_ = NativePostEffect20ConstructionBlock::Phase::executing;
    int state = -1;
    Word mask = 0;
    void* frame_raw = nullptr;
    void* mesh_raw = nullptr;
    void* late_raw = nullptr;
    auto set_state = [&](int next) noexcept { state = next; acquired.native_state = next; };
    try {
        put(actual, 0, 0x00ceb130u); // B4E493
        new (pointer(bits(actual) + 4u)) std::atomic<std::int32_t>(1); // B4E49E, sole initial count write
        put(actual, 0, 0x00d61ec0u); // B4E4AF
        put(actual, 0x0c, 0); put(actual, 0x10, 0); put(actual, 0x14, 0);
        put(actual, 0x1c, vertex_count);
        set_state(0);
        frame_raw = allocate(0x40); // B4E4C1, raw CRT allocation identity
        set_state(1);
        auto* frame = frame_raw ? construct_native_frame_target_owner_00b1fbb0(frame_raw) : nullptr;
        set_state(0);
        acquired.frame_created = frame;
        put(actual, 0x08, bits(frame)); // B4E4ED

        resize_native_string_header_0041dd40(&block.declaration_name_, c.raw_strings, 17, true);
        if (void* data = pointer(word(&block.declaration_name_, 4)))
            std::memmove(data, c.vertex_format_00d61ef8, word(&block.declaration_name_) + 1u);
        void* renderer = c.actual_renderer_00f8d394; // B4E51B, fresh current renderer
        require_renderer(renderer, c, 0x38, 0x00b317e0u);
        set_state(2);
        void* declaration = load_native_renderer_vertex_declaration_00b317e0(renderer,
            &block.declaration_name_, c.declarations);
        acquired.declaration.reference = declaration;
        set_state(0); // B4E53A, before normal string cleanup
        c.declaration_companions.register_native_declaration_reference(acquired.declaration,
            c.declarations.declarations);
        block.string_cleanup(block.declaration_name_);

        renderer = c.actual_renderer_00f8d394; // B4E557: not the earlier captured receiver
        require_renderer(renderer, c, 0x5c, 0x00b287c0u);
        void* stream = create_native_registered_vertex_stream_00b287c0(renderer, 4,
            0x1000u, declaration, c.streams, &acquired.stream_created);
        acquired.stream_created = stream;
        if (stream) {
            block.stream_reference_.emplace(stream, c.streams,
                NativeLogicalVertexCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_stream});
            acquired.stream_reference = &*block.stream_reference_;
            block.bind(NativePostEffect20ConstructionBlock::stream, stream, *block.stream_reference_);
        }
        acquired.declaration_release_started = true;
        release_registered<NativeVertexDeclarationReference>(declaration, c.destruction.actual_decrement_00ce2220,
            c, 0x00d61d1cu, c.declarations.declarations.declaration_vtable_00d61d1c, 0x00b48ca0u);

        mesh_raw = allocate_native_mesh_slot_00b73b60();
        set_state(3);
        NativeMeshStorage* mesh = mesh_raw ? construct_native_mesh_00b73d70(mesh_raw, c.mesh_constants) : nullptr;
        set_state(0);
        acquired.mesh_created = mesh;
        if (mesh) {
            block.mesh_reference_.emplace(*mesh, c.meshes,
                NativeMeshCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_mesh});
            acquired.mesh_reference = &*block.mesh_reference_;
            block.bind(NativePostEffect20ConstructionBlock::mesh, mesh, *block.mesh_reference_);
        }
        require(mesh != nullptr, "native post-effect mesh dereference requires successful allocation");
        set_native_mesh_vertex_stream_00b73bb0(*mesh, c.destruction.actual_owners, 0, stream);
        acquired.stream_creator_release_started = true;
        release_registered<NativeLogicalVertexReference>(stream, c.destruction.actual_decrement_00ce2220,
            c, 0x00d61d6cu, c.streams.actual_logical_profile_00d61d6c, 0x00b4bf10u);

        NativeMaterialStorage* material = create_native_material_for_effect_00535320(effect_name,
            c.actual_renderer_00f8d394, c.materials.material_slots, c.destruction.actual_owners);
        acquired.material_created = material;
        put(actual, 0x14, bits(material)); // B4E5E1
        require(material != nullptr, "native post-effect material dereference requires successful allocation");
        block.material_reference_.emplace(*material, c.materials, c.destruction.actual_material_profile_00d5e520,
            NativeMaterialCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_material});
        acquired.material_reference = &*block.material_reference_;
        block.bind(NativePostEffect20ConstructionBlock::material, material, *block.material_reference_);
        set_native_material_parameter_owner_00b18a40(*material, actual, 0, c.destruction.actual_owners);

        NativeMeshSectionStorage* section = create_native_mesh_section_00533fa0(c.sections.pool_010901d4,
            c.section_bounds_w_00ce4970);
        acquired.section_created = section;
        require(section != nullptr, "native post-effect section dereference requires successful allocation");
        block.section_reference_.emplace(*section, c.sections,
            NativeMeshSectionCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_section});
        acquired.section_reference = &*block.section_reference_;
        block.bind(NativePostEffect20ConstructionBlock::section, section, *block.section_reference_);
        // B4E5FE..618: preserve write order and every original DWORD input.
        put(section, 0x0c, 0); put(section, 0x10, vertex_count); put(section, 0x14, 0);
        put(section, 0x08, vertex_count == 3u ? 4u : 5u);
        put(section, 0x18, vertex_count == 3u ? 1u : 2u);
        set_native_mesh_section_material_00b864c0(*section, c.destruction.actual_owners, pointer(word(actual, 0x14)));
        rebuild_native_mesh_section_vertex_layout_00b865a0(*section, c.destruction.actual_owners, mesh, c.layouts);
        append_native_mesh_draw_section_00b73c60(*mesh, section);

        late_raw = allocate_native_model_slot_00b74eb0();
        set_state(4);
        void* model = nullptr;
        if (late_raw) {
            block.model_owner_.emplace(late_raw, NativeModelPool::slot_bytes, c.models);
            acquired.model_owner = &*block.model_owner_;
            construct_native_string_cstring_0041e870(&block.model_name_, c.model_name_00d61ee4, c.raw_strings);
            mask |= 1u; set_state(5);
            model = construct_native_model_00b75030(*block.model_owner_, &block.model_name_, c.node_constants);
        }
        put(actual, 0x10, bits(model)); // B4E67D
        set_state(0);
        if (model) {
            block.model_reference_.emplace(*block.model_owner_,
                NativeModelCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_model});
            acquired.model_reference = &*block.model_reference_;
            block.bind(NativePostEffect20ConstructionBlock::model, model, *block.model_reference_);
        }
        if (mask & 1u) {
            void* data = pointer(word(&block.model_name_, 4)); // B4E686 before mask clear
            mask &= ~1u;
            if (data) block.return_captured_string(data, word(&block.model_name_) + 1u);
        }
        float scalar_17c, scalar_178;
        const volatile Word* unchanged = &c.models.constants.unchanged_00d7a260;
        void* current_model_actual;
        // B4E6AA..BE: one FLD, current receiver read, FST then FSTP.
        __asm {
            mov eax, unchanged
            fld dword ptr [eax]
            mov eax, actual
            mov eax, dword ptr [eax + 10h]
            mov current_model_actual, eax
            fst dword ptr scalar_17c
            fstp dword ptr scalar_178
        }
        set_native_model_geometry_00b75170(current_model(current_model_actual, c), 0, mesh, scalar_178, scalar_17c);

        late_raw = allocate_native_camera_slot_00b71930();
        set_state(7);
        void* camera = nullptr;
        if (late_raw) {
            block.camera_owner_.emplace(late_raw, NativeCameraPool::slot_bytes, c.cameras, std::move(block.camera_scene_));
            acquired.camera_owner = &*block.camera_owner_;
            prefix_native_string_header_0043c130(&block.camera_name_, c.camera_prefix_00d61ed0, &effect_name, c.raw_strings);
            mask |= 2u; set_state(8);
            camera = construct_native_camera_00b71a80(*block.camera_owner_, &block.camera_name_,
                c.node_constants, std::move(block.viewport_admissions_[0]));
        }
        put(actual, 0x0c, bits(camera)); // B4E70F
        set_state(0);
        if (camera) {
            block.camera_reference_.emplace(*block.camera_owner_,
                NativeCameraCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_camera},
                std::move(block.camera_lifetime_));
            acquired.camera_reference = &*block.camera_reference_;
            block.bind(NativePostEffect20ConstructionBlock::camera, camera, *block.camera_reference_);
        }
        if (mask & 2u) block.string_cleanup(block.camera_name_); // native normal route does not clear bit2

        late_raw = allocate(0x34);
        set_state(10);
        NativeViewportOwner* viewport = late_raw ? initialize_native_viewport_owner_00b1f850(late_raw, c.cameras.viewport) : nullptr;
        set_state(0);
        acquired.viewport_creator = viewport;
        if (viewport) c.viewports.constructed(block.viewport_admissions_[1], *viewport);
        if (optional_size_input) {
            const Word height = surface_dimension(optional_size_input, c, true);
            const Word width = surface_dimension(optional_size_input, c, false); // fresh current profile and slot
            const Word dimensions[2]{width, height};
            require(viewport != nullptr, "native post-effect viewport dimensions require successful allocation");
            set_native_viewport_dimensions_00b1f940(*viewport, dimensions);
        }
        set_native_camera_viewport_00b71990(current_camera(pointer(word(actual, 0x0c)), c), viewport);
        if (viewport) {
            acquired.viewport_release_started = true;
            auto decrement = c.destruction.actual_decrement_00ce2220; // B4E7A6, independent epoch
            require(decrement != nullptr, "missing current CE2220 viewport decrement");
            if (decrement(&viewport->references_04) == 0) {
                require(word(viewport) == 0x00d5e5f8u && word(c.viewport_profile_00d5e5f8) == 0x00bd30e0u &&
                    word(viewport) == 0x00d5e5f8u && word(c.viewport_profile_00d5e5f8, 4) == 0x00b1f8f0u,
                    "unsupported current post-effect viewport terminal");
                invoke_native_viewport_deleting_destructor_00bd30e0(viewport);
            }
        }

        late_raw = allocate(0x28);
        set_state(11);
        void* draw = nullptr;
        if (late_raw) {
            float visibility, leading;
            void* current_draw_model;
            void* current_draw_camera;
            // B4E7D4..E9: constants and current model/camera reads retain native order.
            __asm {
                fld1
                mov eax, actual
                mov ecx, dword ptr [eax + 10h]
                mov current_draw_model, ecx
                fstp dword ptr visibility
                mov ecx, dword ptr [eax + 0ch]
                mov current_draw_camera, ecx
                fldz
                fstp dword ptr leading
            }
            draw = construct_native_post_effect_draw_record_00b51bd0(late_raw, &c.draw_entries,
                leading, section, mesh, current_draw_model, current_draw_camera, visibility);
        }
        put(actual, 0x18, bits(draw)); // B4E7F5
        set_state(0);
        acquired.draw_record_created = draw;
        auto decrement = c.destruction.actual_decrement_00ce2220; // B4E7FF: ONE epoch for both releases
        acquired.section_creator_release_started = true;
        release_registered<NativeMeshSectionReference>(section, decrement, c, 0x00d63194u,
            c.sections.vtable_00d63194, 0x00b86690u);
        acquired.mesh_creator_release_started = true;
        release_registered<NativeMeshReference>(mesh, decrement, c, 0x00d62d60u,
            c.meshes.vtable_00d62d60, 0x00b74280u);

        acquired.native_completed = true; // HOST-only: original B4E470 has completed.
        set_state(-1);
        block.owner_.emplace(actual, c.destruction);
        acquired.completed_owner = &*block.owner_;
        block.owner_reference_.emplace(*block.owner_, c.declaration_companions.registration(),
            NativePostEffect20CompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_owner});
        acquired.completed_reference = &*block.owner_reference_;
        block.keys_[NativePostEffect20ConstructionBlock::owner] = actual;
        block.references_[NativePostEffect20ConstructionBlock::owner] = &*block.owner_reference_;
        // This reference registers/unregisters itself; the block must not do it twice.
        block.settle();
        return actual;
    } catch (...) {
        try {
            if (!acquired.native_completed) block.unwind(state, frame_raw, mesh_raw, late_raw, mask);
        } catch (...) { block.settle(); throw; }
        block.settle();
        throw;
    }
}
} // namespace bsp
