#include "bsp/native_post_effect_parent_lifetime.hpp"
#include "bsp/native_mesh_section_lifetime.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_post_effect_ordinary_lifetime.hpp"
#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/native_material_lifetime.hpp"
#include "bsp/native_material_effect_cache.hpp"
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
Word surface_dimension(const void* actual, Word captured_profile,
    NativePostEffect20ConstructionContext& context, bool height) {
    const Word slot = height ? 0x20u : 0x1cu;
    const Word callee = height ? 0x00b3cd20u : 0x00b3cd10u;
    require(captured_profile == 0x00d619a0u && word(context.surface_profile_00d619a0, slot) == callee,
        "optional post-effect size input requires its current D619A0 dimension slot");
    return height ? surface_height_00b3cd20(actual) : surface_width_00b3cd10(actual);
}
}

NativePostEffect20ConstructionBlock::~NativePostEffect20ConstructionBlock() noexcept {
    if (phase_ != Phase::idle) std::terminate();
}
bool NativePostEffect20ConstructionBlock::ordinary_mesh_mode() const noexcept { return ordinary_lifetime_ != nullptr; }
void* NativePostEffect20ConstructionBlock::allocate_ordinary_mesh() {
    if (!ordinary_lifetime_ || !mesh_frame_ || !context_)
        throw std::logic_error("ordinary Mesh requires its complete cold Block preparation");
    NativeMeshCanonicalAccess::require_domain_identity(ordinary_lifetime_->meshes,
        context_->meshes, context_->destruction.actual_owners, context_->mesh_constants);
    return NativeMeshCanonicalAccess::allocate_static(*mesh_frame_);
}
NativeMeshStorage* NativePostEffect20ConstructionBlock::construct_ordinary_mesh() {
    if (!ordinary_lifetime_ || !mesh_frame_)
        throw std::logic_error("ordinary Mesh construction lost its exact retained Block Frame");
    return NativeMeshCanonicalAccess::construct(*mesh_frame_);
}
NativeMeshReference& NativePostEffect20ConstructionBlock::publish_ordinary_mesh() noexcept {
    auto& reference = NativeMeshCanonicalAccess::publish(*mesh_frame_);
    // Same private constructor continuation; opaque Block bookkeeping only.
    keys_[mesh] = acquired_.mesh_created;
    references_[mesh] = &reference;
    registered_[mesh] = false; // exact central node, never public bind/unbind
    retired_[mesh] = false;
    return reference;
}
void NativePostEffect20ConstructionBlock::release_ordinary_mesh_creator(long (__stdcall* captured)(volatile long*)) {
    NativeMeshCanonicalAccess::release_creator(*mesh_frame_, captured);
}
bool NativePostEffect20ConstructionBlock::return_ordinary_mesh_raw(void* captured_slot) {
    return NativeMeshCanonicalAccess::return_unconstructed_static_slot(*mesh_frame_, captured_slot);
}
void NativePostEffect20ConstructionBlock::validate() const {
    auto& c = *context_;
    auto& owners = c.destruction.actual_owners;
    require(&c.material_effects.effects.owners.actual_owners() == &owners &&
        &c.material_effects.strings == &c.strings &&
        &c.material_effects.effects.construction.strings == &c.strings &&
        &c.material_effects.effects.construction.current_renderer_00f8d394 ==
            &c.actual_renderer_00f8d394 &&
        c.material_effects.effects.renderer_profile_00d5f0a8 == c.renderer_profile_00d5f0a8 &&
        c.material_effects.effects.cache == &c.material_effects,
        "post-effect material cache must borrow the same owners, strings and renderer publication");
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
    if (context.ordinary_lifetime) { prepare(context,*context.ordinary_lifetime); return; }
    prepare_legacy(context);
}
void NativePostEffect20ConstructionBlock::prepare(NativePostEffect20ConstructionContext& context, const NativePostEffectOrdinaryLifetimeBinding& binding) {
    require(!ordinary_lifetime_ && (!context.ordinary_lifetime || context.ordinary_lifetime == &binding) &&
        context.destruction.ordinary_models == &binding.models,
        "ordinary post-effect preparation requires its immutable actual Model binding");
    prepare_legacy(context); // unchanged other-family reservation/exception policy
    ordinary_lifetime_ = &binding;
    try {
        model_frame_.emplace(binding.models,this);
        model_frame_->require_mesh_domain(binding.meshes);
        auto& raw = model_frame_->lifetime_for_owner().storage;
        require(&context.models == &model_frame_->environment_for_owner() &&
            &context.node_constants == &raw.constants && &context.raw_strings == &raw.node.strings &&
            &context.destruction.actual_owners == static_cast<NativeRenderActualOwners*>(&raw.node.scenes.owners) &&
            reinterpret_cast<const volatile void*>(&context.destruction.actual_decrement_00ce2220) ==
                reinterpret_cast<const volatile void*>(&raw.node.scenes.decrement_00ce2220),
            "ordinary post-effect Model must keep exact environment/constants/import/registry roles");
        model_frame_->bind_block(*this);
        mesh_frame_.emplace(binding.meshes,this);
        NativeMeshCanonicalAccess::bind_block(*mesh_frame_,*this);
        // Last fallible cold reservation, BEFORE any Parent allocation.
        // Parent.prepare_after_link performs the later authentic association
        // only after its own retained attachment and block.parent_frame_.
        if (binding.materials) material_frame_ = &binding.materials->prepare();
    } catch (...) {
        try { cancel_ordinary_preparation(); }
        catch (...) { phase_ = Phase::settled; throw; }
        throw;
    }
}
void NativePostEffect20ConstructionBlock::cancel_preparation() noexcept {
    if (ordinary_lifetime_) std::terminate(); // distinct ordinary API required
    cancel_preparation_legacy();
}
void NativePostEffect20ConstructionBlock::cancel_ordinary_preparation() {
    if (ordinary_lifetime_ && (ordinary_lifetime_->materials || ordinary_lifetime_->sections)) {
        require(phase_ == Phase::preparing || phase_ == Phase::prepared,
            "ordinary Material cold cancellation cannot recover an entered attempt");
        try {
            preflight_close_after_host_quiescence();
            if (ordinary_lifetime_->sections && parent_frame_)
                require(parent_frame_->section_caller_ended(),
                    "bound Parent Section callers must end before cold Block/factory reset");
            if (material_frame_) {
                if (parent_frame_)
                    require(parent_frame_->material_caller_closed(),
                        "bound Parent Material loan must close before cold factory reset");
                else
                    material_frame_->close_after_host_quiescence(); // genuine unassociated cold record
            }
            if (model_frame_) { model_frame_->cancel_without_native_payload(); model_frame_.reset(); }
            if (mesh_frame_) {
                NativeMeshCanonicalAccess::cancel_without_native_payload(*mesh_frame_);
                mesh_frame_.reset();
            }
            material_frame_ = nullptr; // closed loan only; Domain keeps immutable history
            ordinary_lifetime_ = nullptr;
            cancel_preparation_legacy();
        } catch (...) {
            if (parent_frame_) parent_frame_->record_source_failure();
            if (model_frame_) model_frame_->record_source_failure();
            if (mesh_frame_) NativeMeshCanonicalAccess::record_source_failure(*mesh_frame_);
            throw;
        }
        return;
    }
    require(ordinary_lifetime_ && (phase_ == Phase::preparing || phase_ == Phase::prepared),
        "ordinary cold cancellation cannot recover an entered attempt");
    try {
        if (model_frame_) { model_frame_->cancel_without_native_payload(); model_frame_.reset(); }
        if (mesh_frame_) {
            NativeMeshCanonicalAccess::cancel_without_native_payload(*mesh_frame_);
            mesh_frame_.reset();
        }
        ordinary_lifetime_ = nullptr;
        cancel_preparation_legacy();
    } catch (...) {
        if (model_frame_) model_frame_->record_source_failure();
        if (mesh_frame_) NativeMeshCanonicalAccess::record_source_failure(*mesh_frame_);
        throw;
    }
}
bool NativePostEffect20ConstructionBlock::requires_ordinary_process_retention() const noexcept {
    if (ordinary_lifetime_ && (ordinary_lifetime_->materials || ordinary_lifetime_->sections)) {
        return phase_ == Phase::preparing || phase_ == Phase::executing ||
            (parent_frame_ && parent_frame_->requires_process_retention()) ||
            (material_frame_ && material_frame_->requires_process_retention()) ||
            (ordinary_lifetime_->materials && ordinary_lifetime_->materials->requires_process_retention()) ||
            (ordinary_lifetime_->sections && ordinary_lifetime_->sections->requires_process_retention()) ||
            (model_frame_ && model_frame_->requires_process_retention()) ||
            ordinary_lifetime_->meshes.requires_process_retention();
    }
    return ordinary_lifetime_ &&
        (phase_ == Phase::preparing || phase_ == Phase::executing ||
         (model_frame_ && model_frame_->requires_process_retention()) ||
         ordinary_lifetime_->meshes.requires_process_retention());
}
void NativePostEffect20ConstructionBlock::section_source_retired(
    NativePostEffectParentFrame& parent, NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionReference& reference) noexcept {
    if(parent_frame_!=&parent||section_frame_!=&frame||!ordinary_lifetime_||!ordinary_lifetime_->sections||
        references_[section]!=&reference||acquired_.section_reference!=&reference||registered_[section]||
        section_reference_||retired_[section]){parent.record_source_failure();return;}
    // The SAME exact Section record already retired. No second unregister,
    // payload read or optional destruction; the pointer loan ends here only.
    retired_[section]=true;references_[section]=nullptr;acquired_.section_reference=nullptr;
    section_frame_=nullptr;
}
void NativePostEffect20ConstructionBlock::parent_source_retired(
    NativePostEffectParentFrame& frame, RenderCommandReference& reference) noexcept {
    if (parent_frame_ != &frame || references_[owner] != &reference) {
        frame.record_source_failure(); return;
    }
    retired_[owner] = true; // captured exact record, no optional reset/self-delete
}
void NativePostEffect20ConstructionBlock::record_ordinary_constructor_failure() noexcept {
    // Exact trusted Block bridge. Same ordinary catch sequence/receivers;
    // no public Mesh action, new permit, Native work or extra health sample.
    model_frame_->record_source_failure();
    NativeMeshCanonicalAccess::record_source_failure(*mesh_frame_);
}
void NativePostEffect20ConstructionBlock::model_source_retired(NativePostEffectModelFrame& frame) noexcept {
    if (!model_frame_ || &*model_frame_ != &frame || references_[model] != frame.canonical() || retired_[model]) {
        frame.record_source_failure(); return;
    }
    retired_[model] = true; // same exact record already retired; no unbind/reset
}
void NativePostEffect20ConstructionBlock::mesh_source_retired(NativeMeshCanonicalProducerFrame& frame) noexcept {
    if (!mesh_frame_ || &*mesh_frame_ != &frame ||
        references_[mesh] != NativeMeshCanonicalAccess::canonical(frame) || retired_[mesh]) {
        NativeMeshCanonicalAccess::record_source_failure(frame); return;
    }
    retired_[mesh] = true; // central record/reference stays alive through outer fences
}
void NativePostEffect20ConstructionBlock::reset_after_host_quiescence() noexcept {
    if (ordinary_lifetime_) std::terminate(); // preserves explicit legacy policy
    reset_after_host_quiescence_legacy();
}
void NativePostEffect20ConstructionBlock::preflight_close_after_host_quiescence(ReadinessRejection rejection) {
    // Readiness only for the old normal-NULL path which already genuinely
    // cancelled this Block. Parent must authenticate its private cold_cancelled
    // history before using this idle result; it cannot authorize a new cancel.
    if (phase_ == Phase::idle && !context_ && !ordinary_lifetime_ && !material_frame_) return;
    require(ordinary_lifetime_ && context_,
        "ordinary Block preflight requires its genuine prepared context");
    // Retention/failure is distinct from expected live or flags0 custody.
    // These checks never issue Native origin, a credit or a cleanup permission.
    require(!ordinary_lifetime_->meshes.requires_process_retention() &&
        (!ordinary_lifetime_->materials || !ordinary_lifetime_->materials->requires_process_retention()) &&
        (!ordinary_lifetime_->sections || !ordinary_lifetime_->sections->requires_process_retention()) &&
        (!material_frame_ || !material_frame_->requires_process_retention()),
        "failed ordinary Block graph must keep every factory/provider loan");
    if (model_frame_) model_frame_->require_source_healthy();
    if (ordinary_lifetime_->materials)
        require(material_factory_ && acquired_.material_factory == &*material_factory_ &&
            (material_factory_->phase == NativeMaterialFactoryAcquired::Phase::empty ||
             material_factory_->phase == NativeMaterialFactoryAcquired::Phase::complete),
            "ordinary factory reset diagnostics are not ready; retain its real loan");
    if (phase_ == Phase::preparing || phase_ == Phase::prepared) {
        // This private phase precedes the actual constructor's admission to
        // Native effects. Root owning-module checks validate every cold record;
        // an empty public Acquired/registry/profile is not cancellation authority.
        for (std::size_t i=0;i!=slot_count;++i)
            require(!references_[i],"cold ordinary Block has an entered companion");
        if (model_frame_) model_frame_->preflight_cancel_without_native_payload();
        if (mesh_frame_) NativeMeshCanonicalAccess::preflight_cancel_without_native_payload(*mesh_frame_,
        rejection == ReadinessRejection::preserve ?
            NativeMeshCanonicalAccess::ReadinessRejection::preserve : NativeMeshCanonicalAccess::ReadinessRejection::sticky);
        for (auto& record : viewport_records_) {
            const auto phase = record.phase();
            require(phase == NativeViewportRegistry::Phase::unused ||
                phase == NativeViewportRegistry::Phase::reserved || phase == NativeViewportRegistry::Phase::cancelled,
                "cold ordinary Block viewport has a live/unknown Native owner");
        }
        return;
    }
    require(phase_ == Phase::settled,
        "ordinary Block close needs actual completed caller quiescence");
    if (ordinary_lifetime_->materials)
        require(material_frame_ && parent_frame_ && material_factory_ && !material_reference_ &&
            acquired_.material_factory == &*material_factory_,
            "ordinary Material close requires its exact Parent/Frame/factory loan");
    for (std::size_t i=0;i!=slot_count;++i)
        if ((i != material || !ordinary_lifetime_->materials) &&
            (i != section || !ordinary_lifetime_->sections))
            require(!references_[i] || retired_[i],"ordinary Block still has a live/unknown non-Material companion");
    require((!model_owner_ || model_owner_->phase == NativeModelOwner::Phase::dead) &&
        (!camera_owner_ || camera_owner_->phase == NativeCameraOwner::Phase::dead) &&
        (!owner_ || owner_->phase() == NativePostEffect20Owner::Phase::dead),
        "ordinary Block retains a live/unknown semantic owner");
    for (auto& record : viewport_records_) {
        const auto phase = record.phase();
        require(phase == NativeViewportRegistry::Phase::unused || phase == NativeViewportRegistry::Phase::cancelled ||
            phase == NativeViewportRegistry::Phase::retired,
            "ordinary Block viewport has not reached its actual final disposition");
    }
    if (ordinary_lifetime_->sections)
        require(parent_frame_ && !section_reference_ && !registered_[section] &&
            ((!references_[section] && !section_frame_) ||
             (section_frame_ && references_[section] == acquired_.section_reference)),
            "ordinary Section close requires the SAME borrowed Frame canonical");
    if (model_frame_) model_frame_->preflight_close_at_quiescence(
        rejection == ReadinessRejection::preserve ?
            NativePostEffectModelFrame::ReadinessRejection::preserve : NativePostEffectModelFrame::ReadinessRejection::sticky);
    if (mesh_frame_) NativeMeshCanonicalAccess::preflight_producer_link_at_quiescence(*mesh_frame_,
        rejection == ReadinessRejection::preserve ?
            NativeMeshCanonicalAccess::ReadinessRejection::preserve : NativeMeshCanonicalAccess::ReadinessRejection::sticky);
    // Parent performs the separate Material and BOTH caller-observation
    // preflights; do not recurse into it or close/drain anything here.
}
void NativePostEffect20ConstructionBlock::close_ordinary_after_host_quiescence() {
    if (ordinary_lifetime_ && (ordinary_lifetime_->materials || ordinary_lifetime_->sections)) {
        try {
            preflight_close_after_host_quiescence();
            if (ordinary_lifetime_->materials)
                require(parent_frame_ && parent_frame_->material_caller_closed(),
                    "actual Parent Material/caller chains must close before Block/factory reset");
            if (ordinary_lifetime_->sections)
                require(parent_frame_ && parent_frame_->section_caller_ended(),
                    "actual Parent Section callers must end before Block/factory reset");
            // This is only C++ slot bookkeeping from the SAME genuine private
            // Frame's completion, not a legacy callback/profile/count receipt.
            if (ordinary_lifetime_->materials) retired_[material] = true;
            if (model_frame_) {
                model_frame_->close_at_quiescence();
                model_frame_.reset(); references_[model] = nullptr;
            }
            if (mesh_frame_) {
                NativeMeshCanonicalAccess::close_producer_link_at_quiescence(*mesh_frame_);
                mesh_frame_.reset(); references_[mesh] = nullptr;
            }
            material_frame_ = nullptr;
            ordinary_lifetime_ = nullptr;
            reset_after_host_quiescence_legacy(); // factory reset is last, after loan close
        } catch (...) {
            if (parent_frame_) parent_frame_->record_source_failure();
            if (model_frame_) model_frame_->record_source_failure();
            if (mesh_frame_) NativeMeshCanonicalAccess::record_source_failure(*mesh_frame_);
            throw;
        }
        return;
    }
    require(ordinary_lifetime_ && phase_ == Phase::settled,
        "ordinary post-effect close needs actual completed caller quiescence");
    try {
        for (std::size_t i=0;i!=slot_count;++i)
            require(!references_[i] || retired_[i],"ordinary post-effect still has a live/unknown companion");
        if (model_frame_) {
            model_frame_->close_at_quiescence();
            model_frame_.reset(); references_[model] = nullptr;
        }
        if (mesh_frame_) {
            NativeMeshCanonicalAccess::close_producer_link_at_quiescence(*mesh_frame_);
            mesh_frame_.reset(); references_[mesh] = nullptr;
        }
        ordinary_lifetime_ = nullptr;
        reset_after_host_quiescence_legacy();
    } catch (...) {
        if (model_frame_) model_frame_->record_source_failure();
        if (mesh_frame_) NativeMeshCanonicalAccess::record_source_failure(*mesh_frame_);
        throw;
    }
}

void NativePostEffect20ConstructionBlock::prepare_legacy(NativePostEffect20ConstructionContext& context) {
    require(phase_ == Phase::idle, "post-effect block requires idle external storage");
    context_ = &context;
    phase_ = Phase::preparing;
    try {
        validate();
        material_factory_.emplace();
        acquired_.material_factory = &*material_factory_;
        camera_scene_ = context.cameras.nodes.scenes.reserve_binding();
        camera_lifetime_ = context.cameras.nodes.attachments.reserve_binding();
        viewport_admissions_[0] = context.viewports.admit(viewport_records_[0]);
        viewport_admissions_[1] = context.viewports.admit(viewport_records_[1]);
        phase_ = Phase::prepared;
    } catch (...) { cancel_preparation(); throw; }
}
void NativePostEffect20ConstructionBlock::cancel_preparation_legacy() noexcept {
    if (phase_ != Phase::preparing && phase_ != Phase::prepared) std::terminate();
    camera_scene_.cancel(); camera_lifetime_.cancel();
    for (auto& admission : viewport_admissions_) admission.cancel();
    for (auto& record : viewport_records_) context_->viewports.forget_quiescent(record);
    material_factory_.reset();
    acquired_.material_factory = nullptr;
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

void NativePostEffect20ConstructionBlock::reset_after_host_quiescence_legacy() noexcept {
    if (phase_ != Phase::settled) std::terminate();
    // Failed factory/cache frames retain unresolved native obligations. Even a
    // successful factory may precede a failed material companion registration.
    if (!material_factory_ ||
        (material_factory_->phase != NativeMaterialFactoryAcquired::Phase::empty &&
         material_factory_->phase != NativeMaterialFactoryAcquired::Phase::complete) ||
        (material_factory_->material && !retired_[material])) std::terminate();
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
    material_factory_.reset();
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
    volatile Word& allocation_argument, Word& mask) {
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
            case 3: {
                void* const captured = std::exchange(mesh_raw, nullptr); // consume before action
                if (ordinary_mesh_mode()) (void)return_ordinary_mesh_raw(captured);
                else return_native_mesh_slot_00b72f70(captured);
                break;
            }
            case 4: {
                void* raw = pointer(allocation_argument); // CBFB53: CURRENT [EH EBP+8]
                if (ordinary_lifetime_) {
                    if (model_frame_->abandon_before_constructor_return()) {
                        acquired_.model_owner = nullptr;
                        return_native_model_slot_00b748c0(raw,*model_frame_);
                    }
                } else {
                    model_owner_.reset(); acquired_.model_owner = nullptr;
                    return_native_model_slot_00b748c0(raw);
                }
                break;
            }
            case 5: case 6:
                if (mask & 1u) { mask &= ~1u; string_cleanup(model_name_); } break;
            case 7: {
                void* raw = pointer(allocation_argument); // CBFB74: CURRENT [EH EBP+8]
                camera_owner_.reset(); acquired_.camera_owner = nullptr;
                return_native_camera_slot_00b71350(raw); break;
            }
            case 8: case 9:
                if (mask & 2u) { mask &= ~2u; string_cleanup(camera_name_); } break;
            case 10: case 11: singleton_lifetime_free(pointer(allocation_argument)); break;
            default: std::terminate();
            }
        } catch (...) { unwind(state, frame_raw, mesh_raw, allocation_argument, mask); throw; }
    }
}

void* construct_native_post_effect20_00b4e470(void* actual, std::size_t allocation_bytes,
    NativeString& effect_name, Word vertex_count, const void* optional_size_input,
    NativePostEffect20ConstructionBlock& block) {
    volatile Word words[3]{bits(&effect_name),vertex_count,bits(optional_size_input)};
    const NativePostEffect20ArgumentView arguments{words};
    return construct_native_post_effect20_00b4e470(actual,allocation_bytes,arguments,block);
}

void* construct_native_post_effect20_00b4e470(void* actual, std::size_t allocation_bytes,
    const NativePostEffect20ArgumentView& arguments, NativePostEffect20ConstructionBlock& block) {
    require(block.phase_ == NativePostEffect20ConstructionBlock::Phase::prepared && actual &&
        !(bits(actual) & 3u) && allocation_bytes >= 0x20u,
        "post-effect constructor requires prepared storage and an aligned actual 20h allocation");
    block.validate();
    if (block.ordinary_lifetime_ && block.ordinary_lifetime_->materials) {
        require(block.material_frame_ && block.parent_frame_,
            "ordinary Material Native entry requires its genuine retained Parent caller");
        block.parent_frame_->require_material_constructor_operation(actual);
    }
    const bool selected_sections = block.ordinary_lifetime_ && block.ordinary_lifetime_->sections;
    if (selected_sections) {
        require(block.parent_frame_ != nullptr,
            "ordinary Section Native entry requires its genuine retained Parent caller");
        block.parent_frame_->require_section_constructor_operation(actual);
    }
    auto& c = *block.context_;
    auto& acquired = block.acquired_;
    acquired.actual_owner = actual;
    block.phase_ = NativePostEffect20ConstructionBlock::Phase::executing;
    int state = -1;
    Word mask = 0;
    void* frame_raw = nullptr;
    void* mesh_raw = nullptr;
    auto set_state = [&](int next) noexcept { state = next; acquired.native_state = next; };
    try {
        if (block.ordinary_lifetime_) block.model_frame_->begin_constructor();
        put(actual, 0, 0x00ceb130u); // B4E493
        new (pointer(bits(actual) + 4u)) std::atomic<std::int32_t>(1); // B4E49E, sole initial count write
        if (block.parent_frame_) block.parent_frame_->native_atomic_born(actual);
        const Word initial_count = arguments.words[1]; // B4E4A5 before derived profile/clears
        set_state(0); // B4E4AB
        put(actual, 0, 0x00d61ec0u); // B4E4AF
        put(actual, 0x0c, 0); put(actual, 0x10, 0); put(actual, 0x14, 0);
        put(actual, 0x1c, initial_count);
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

        mesh_raw = block.ordinary_mesh_mode() ? block.allocate_ordinary_mesh() :
            allocate_native_mesh_slot_00b73b60();
        set_state(3);
        NativeMeshStorage* mesh = mesh_raw ? (block.ordinary_mesh_mode() ?
            block.construct_ordinary_mesh() : construct_native_mesh_00b73d70(mesh_raw, c.mesh_constants)) : nullptr;
        set_state(0);
        acquired.mesh_created = mesh;
        if (mesh) {
            if (block.ordinary_mesh_mode()) {
                acquired.mesh_reference = &block.publish_ordinary_mesh(); // SAME central canonical; no bind/count
            } else {
                block.mesh_reference_.emplace(*mesh, c.meshes,
                    NativeMeshCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_mesh});
                acquired.mesh_reference = &*block.mesh_reference_;
                block.bind(NativePostEffect20ConstructionBlock::mesh, mesh, *block.mesh_reference_);
            }
        }
        require(mesh != nullptr, "native post-effect mesh dereference requires successful allocation");
        set_native_mesh_vertex_stream_00b73bb0(*mesh, c.destruction.actual_owners, 0, stream);
        acquired.stream_creator_release_started = true;
        release_registered<NativeLogicalVertexReference>(stream, c.destruction.actual_decrement_00ce2220,
            c, 0x00d61d6cu, c.streams.actual_logical_profile_00d61d6c, 0x00b4bf10u);

        const void* const effect_name = pointer(arguments.words[0]); // B4E5D4
        // The selected Frame supplies the SAME validated pool/owners pair;
        // name, cache and stable Acquired remain the actual original operands.
        NativeMaterialStorage* material = block.material_frame_ ?
            create_native_material_from_effect_cache_00535320(*block.material_frame_,effect_name,
                c.material_effects,*block.material_factory_) :
            create_native_material_from_effect_cache_00535320(effect_name,
                c.materials.material_slots,c.destruction.actual_owners,
                c.material_effects,*block.material_factory_); // B4E5D8
        acquired.material_created = material;
        put(actual, 0x14, bits(material)); // B4E5E1
        if (block.material_frame_) block.parent_frame_->material_field_published(material);
        require(material != nullptr, "native post-effect material dereference requires successful allocation");
        if (block.material_frame_) {
            acquired.material_reference = &block.material_frame_->canonical();
            block.keys_[NativePostEffect20ConstructionBlock::material] = material;
            block.references_[NativePostEffect20ConstructionBlock::material] = acquired.material_reference;
            block.registered_[NativePostEffect20ConstructionBlock::material] = false; // SAME Domain canonical, no second bind/count
            block.retired_[NativePostEffect20ConstructionBlock::material] = false;
            block.parent_frame_->set_material_parameter_owner(*material,actual,0,c.destruction.actual_owners);
        } else {
            block.material_reference_.emplace(*material, c.materials, c.destruction.actual_material_profile_00d5e520,
                NativeMaterialCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_material});
            acquired.material_reference = &*block.material_reference_;
            block.bind(NativePostEffect20ConstructionBlock::material, material, *block.material_reference_);
            set_native_material_parameter_owner_00b18a40(*material, actual, 0, c.destruction.actual_owners);
        }

        NativeMeshSectionStorage* section;
        if (selected_sections) section = block.parent_frame_->construct_section();
        else section = create_native_mesh_section_00533fa0(c.sections.pool_010901d4,
            c.section_bounds_w_00ce4970);
        acquired.section_created = section;
        require(section != nullptr, "native post-effect section dereference requires successful allocation");
        if (selected_sections) {
            acquired.section_reference = &block.parent_frame_->publish_section_borrow();
        } else {
            block.section_reference_.emplace(*section, c.sections,
                NativeMeshSectionCompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_section});
            acquired.section_reference = &*block.section_reference_;
            block.bind(NativePostEffect20ConstructionBlock::section, section, *block.section_reference_);
        }
        const Word vertex_count = arguments.words[1]; // B4E5F0, captured for all three stores
        // B4E5FE..618: preserve write order and every original DWORD input.
        put(section, 0x0c, 0); put(section, 0x10, vertex_count); put(section, 0x14, 0);
        put(section, 0x08, vertex_count == 3u ? 4u : 5u);
        put(section, 0x18, vertex_count == 3u ? 1u : 2u);
        if (selected_sections) block.parent_frame_->set_section_material(pointer(word(actual, 0x14)));
        else set_native_mesh_section_material_00b864c0(*section, c.destruction.actual_owners, pointer(word(actual, 0x14)));
        if (selected_sections) block.parent_frame_->rebuild_section_layout(mesh);
        else rebuild_native_mesh_section_vertex_layout_00b865a0(*section, c.destruction.actual_owners, mesh, c.layouts);
        append_native_mesh_draw_section_00b73c60(*mesh, section);

        void* const model_raw = block.ordinary_lifetime_ ?
            allocate_native_model_slot_00b74eb0(*block.model_frame_) : allocate_native_model_slot_00b74eb0();
        arguments.words[1] = bits(model_raw); // B4E640; normal callee keeps captured allocation
        set_state(4);
        void* model = nullptr;
        if (model_raw) {
            if (block.ordinary_lifetime_) acquired.model_owner = &block.model_frame_->prepare_owner();
            else {
            block.model_owner_.emplace(model_raw, NativeModelPool::slot_bytes, c.models);
            acquired.model_owner = &*block.model_owner_;
            }
            construct_native_string_cstring_0041e870(&block.model_name_, c.model_name_00d61ee4, c.raw_strings);
            mask |= 1u; set_state(5);
            if (block.ordinary_lifetime_) {
                model = construct_native_model_00b75030(*acquired.model_owner, &block.model_name_, c.node_constants);
                auto& reference = block.model_frame_->publish_constructed_owner();
                acquired.model_reference = &reference;
                block.keys_[NativePostEffect20ConstructionBlock::model] = model;
                block.references_[NativePostEffect20ConstructionBlock::model] = &reference;
            } else {
            model = construct_native_model_00b75030(*block.model_owner_, &block.model_name_, c.node_constants);
            }
        }
        put(actual, 0x10, bits(model)); // B4E67D
        set_state(0);
        if (model && block.ordinary_lifetime_) block.model_frame_->parent_field_published();
        if (model && !block.ordinary_lifetime_) {
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
        if (block.ordinary_lifetime_) {
            volatile Word* homes = block.model_frame_->geometry_homes();
            volatile Word* scalar_17c_home = homes + 3;
            volatile Word* scalar_178_home = homes + 2;
            const volatile Word* unchanged = &c.models.constants.unchanged_00d7a260;
            void* current_model_actual;
            // B4E6AA..BE: same one-FLD/current+10/FST/FSTP input epoch.
            __asm {
                mov eax, unchanged
                fld dword ptr [eax]
                mov eax, actual
                mov eax, dword ptr [eax + 10h]
                mov current_model_actual, eax
                mov edx, scalar_17c_home
                fst dword ptr [edx]
                mov edx, scalar_178_home
                fstp dword ptr [edx]
            }
            homes[0] = 0;
            homes[1] = bits(mesh);
            block.model_frame_->set_geometry_on_captured_model(current_model_actual);
        } else {
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
        }

        void* const camera_raw = allocate_native_camera_slot_00b71930();
        arguments.words[1] = bits(camera_raw); // B4E6D4
        set_state(7);
        void* camera = nullptr;
        if (camera_raw) {
            block.camera_owner_.emplace(camera_raw, NativeCameraPool::slot_bytes, c.cameras, std::move(block.camera_scene_));
            acquired.camera_owner = &*block.camera_owner_;
            const void* const current_name = pointer(arguments.words[0]); // B4E6E1, independently current
            prefix_native_string_header_0043c130(&block.camera_name_, c.camera_prefix_00d61ed0, current_name, c.raw_strings);
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

        void* const viewport_raw = allocate(0x34);
        arguments.words[1] = bits(viewport_raw); // B4E741
        set_state(10);
        NativeViewportOwner* viewport = viewport_raw ? initialize_native_viewport_owner_00b1f850(viewport_raw, c.cameras.viewport) : nullptr;
        const void* const first_size_input = pointer(arguments.words[2]); // B4E75B, before state0
        set_state(0);
        acquired.viewport_creator = viewport;
        if (viewport) c.viewports.constructed(block.viewport_admissions_[1], *viewport);
        if (first_size_input) {
            const Word height = surface_dimension(first_size_input, word(first_size_input), c, true);
            const void* const second_size_input = pointer(arguments.words[2]); // B4E76E
            const Word second_profile = word(second_size_input); // B4E772 before height overwrite
            arguments.words[1] = height; // B4E774; current slot1C is loaded only afterward
            const Word width = surface_dimension(second_size_input, second_profile, c, false);
            const Word current_height = arguments.words[1]; // B4E77D after width getter
            const Word dimensions[2]{width, current_height};
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

        void* const draw_raw = allocate(0x28);
        arguments.words[1] = bits(draw_raw); // B4E7C3
        set_state(11);
        void* draw = nullptr;
        if (draw_raw) {
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
            draw = construct_native_post_effect_draw_record_00b51bd0(draw_raw, &c.draw_entries,
                leading, section, mesh, current_draw_model, current_draw_camera, visibility);
        }
        put(actual, 0x18, bits(draw)); // B4E7F5
        set_state(0);
        acquired.draw_record_created = draw;
        auto decrement = c.destruction.actual_decrement_00ce2220; // B4E7FF: ONE epoch for both releases
        acquired.section_creator_release_started = true;
        if (selected_sections) block.parent_frame_->release_section_creator(decrement);
        else release_registered<NativeMeshSectionReference>(section, decrement, c, 0x00d63194u,
            c.sections.vtable_00d63194, 0x00b86690u);
        acquired.mesh_creator_release_started = true;
        if (block.ordinary_mesh_mode()) block.release_ordinary_mesh_creator(decrement);
        else release_registered<NativeMeshReference>(mesh, decrement, c, 0x00d62d60u,
            c.meshes.vtable_00d62d60, 0x00b74280u);

        // Private body-return fact is BEFORE any following host publication.
        if (block.parent_frame_) block.parent_frame_->native_constructor_returned(actual);
        acquired.native_completed = true; // HOST-only: original B4E470 has completed.
        set_state(-1);
        if (block.parent_frame_) {
            block.parent_frame_->publish_after_native_constructor();
            acquired.completed_owner = &block.parent_frame_->owner20_after_publication();
            acquired.completed_reference = static_cast<NativePostEffect20Reference*>(
                &block.parent_frame_->canonical_after_publication());
            block.keys_[NativePostEffect20ConstructionBlock::owner] = actual;
            block.references_[NativePostEffect20ConstructionBlock::owner] = acquired.completed_reference;
        } else {
            block.owner_.emplace(actual, c.destruction);
            acquired.completed_owner = &*block.owner_;
            block.owner_reference_.emplace(*block.owner_, c.declaration_companions.registration(),
                NativePostEffect20CompanionDisposal{&block, &NativePostEffect20ConstructionBlock::retire_owner});
            acquired.completed_reference = &*block.owner_reference_;
            block.keys_[NativePostEffect20ConstructionBlock::owner] = actual;
            block.references_[NativePostEffect20ConstructionBlock::owner] = &*block.owner_reference_;
            // This reference registers/unregisters itself; the block must not do it twice.
        }
        if (block.ordinary_lifetime_) block.model_frame_->finish_constructor();
        block.settle();
        return actual;
    } catch (...) {
        if (block.parent_frame_) block.parent_frame_->record_source_failure();
        if (block.ordinary_lifetime_) {
            block.record_ordinary_constructor_failure();
        }
        try {
            if (block.parent_frame_ ? !block.parent_frame_->native_constructor_has_returned()
                                    : !acquired.native_completed) block.unwind(state, frame_raw, mesh_raw, arguments.words[1], mask);
        } catch (...) {
            if (block.ordinary_lifetime_) block.model_frame_->finish_constructor();
            block.settle(); throw;
        }
        if (block.ordinary_lifetime_) block.model_frame_->finish_constructor();
        block.settle();
        throw;
    }
}
} // namespace bsp
