#include "bsp/native_directional_shadow_construction_block.hpp"
#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {

NativeDirectionalShadowConstructionBlock::NativeDirectionalShadowConstructionBlock() noexcept {
    for (std::size_t i = 0; i < camera_count; ++i) cameras_[i].retirement = {this, i};
}

NativeDirectionalShadowConstructionBlock::Admission::Admission(
    NativeDirectionalShadowConstructionBlock& block, NativeCameraEnvironment& environment,
    const NativeNodeRawConstants& constants) noexcept
    : block_(&block), camera_environment_(&environment), node_constants_(&constants) {}

NativeDirectionalShadowConstructionBlock::Admission::Admission(Admission&& other) noexcept {
    *this = std::move(other);
}

NativeDirectionalShadowConstructionBlock::Admission&
NativeDirectionalShadowConstructionBlock::Admission::operator=(Admission&& other) noexcept {
    if (this == &other) return *this;
    if ((block_ && block_->phase_ == Phase::executing) ||
        (other.block_ && other.block_->phase_ == Phase::executing)) std::terminate();
    cancel();
    block_ = std::exchange(other.block_, nullptr);
    camera_environment_ = std::exchange(other.camera_environment_, nullptr);
    node_constants_ = std::exchange(other.node_constants_, nullptr);
    for (std::size_t i = 0; i < camera_count; ++i) {
        scene_admissions_[i] = std::move(other.scene_admissions_[i]);
        lifetime_admissions_[i] = std::move(other.lifetime_admissions_[i]);
    }
    for (std::size_t i = 0; i < viewport_count; ++i)
        viewport_admissions_[i] = std::move(other.viewport_admissions_[i]);
    return *this;
}

NativeDirectionalShadowConstructionBlock::Admission::~Admission() noexcept { cancel(); }

void NativeDirectionalShadowConstructionBlock::Admission::cancel() noexcept {
    if (!block_) return;
    auto& block = *block_;
    const bool rollback = block.phase_ == Phase::preparing || block.phase_ == Phase::prepared;
    if ((!rollback && block.phase_ != Phase::executing) || !block.viewport_registry_)
        std::terminate();
    if (rollback) {
        for (const auto& camera : block.cameras_)
            if (camera.owner || camera.reference || camera.camera_completed || camera.retired)
                std::terminate();
        if (block.cache_acquisition_ &&
            block.cache_acquisition_->phase != NativeTextureCacheAcquired::Phase::not_started)
            std::terminate();
    }
    for (std::size_t i = 0; i < camera_count; ++i) {
        scene_admissions_[i].cancel();
        lifetime_admissions_[i].cancel();
    }
    for (auto& admission : viewport_admissions_) admission.cancel();
    if (rollback) {
        for (auto& record : block.viewport_records_) {
            if (record.phase() != NativeViewportRegistry::Phase::unused &&
                record.phase() != NativeViewportRegistry::Phase::cancelled) std::terminate();
            block.viewport_registry_->forget_quiescent(record);
        }
        // No cache/provider invocation occurs during admission. This optional
        // contains fresh metadata only; no name/resource cleanup is introduced.
        block.cache_acquisition_.reset();
        block.viewport_registry_ = nullptr;
        block.phase_ = Phase::idle;
    } else {
        // The outer native source entry must finish its own normal/EH actions
        // before destroying this capability. No native survivor is touched here.
        block.phase_ = Phase::settled;
    }
    block_ = nullptr;
    camera_environment_ = nullptr;
    node_constants_ = nullptr;
}

void NativeDirectionalShadowConstructionBlock::validate_environment(
    NativeCameraEnvironment& environment, const NativeNodeRawConstants& constants,
    NativeViewportRegistry& registry) {
    (void)environment.nodes.require_raw_name_pool();
    if (&constants.one_00d7a24c != &environment.viewport.one_bits_00d7a24c)
        throw std::logic_error("shadow camera and node require the same actual D7A24C cell");
    if (&environment.nodes.attachments.scenes != &environment.nodes.scenes ||
        !environment.nodes.node_virtual_0c)
        throw std::invalid_argument("shadow requires one canonical scene runtime and node type predicate");
    if (&environment.viewport_views != &registry)
        throw std::logic_error("shadow viewport admission requires its exact installed resolver");
    if (!environment.crt.dispatch_bypass_0109dd78 || !environment.crt.except_00c27489 ||
        !environment.vtable_00d62cf0 || !environment.vtable_00d62c88)
        throw std::invalid_argument("shadow camera requires actual CRT and table bindings");
    // Before execution the caller binds the same actual0108FFB0 camera pool,
    // cache/VFS raw string domain and current-global contexts. No probes or
    // additional provider bindings are performed by this host-only block.
}

bool NativeDirectionalShadowConstructionBlock::idle_storage() const noexcept {
    if (phase_ != Phase::idle || viewport_registry_ || cache_acquisition_) return false;
    for (std::size_t i = 0; i < camera_count; ++i) {
        const auto& camera = cameras_[i];
        if (camera.owner || camera.reference || camera.camera_completed || camera.retired ||
            camera.retirement.block != this || camera.retirement.index != i) return false;
    }
    for (const auto& record : viewport_records_)
        if (record.phase() != NativeViewportRegistry::Phase::unused) return false;
    return true;
}

NativeDirectionalShadowConstructionBlock::Admission NativeDirectionalShadowConstructionBlock::admit(
    NativeCameraEnvironment& environment, const NativeNodeRawConstants& constants,
    NativeViewportRegistry& registry) {
    if (!idle_storage())
        throw std::logic_error("shadow construction block requires idle quiescent storage");
    phase_ = Phase::preparing;
    viewport_registry_ = &registry;
    Admission prepared(*this, environment, constants);
    validate_environment(environment, constants, registry);
    cache_acquisition_.emplace();
    for (std::size_t i = 0; i < camera_count; ++i)
        prepared.scene_admissions_[i] = environment.nodes.scenes.reserve_binding();
    for (std::size_t i = 0; i < camera_count; ++i)
        prepared.lifetime_admissions_[i] = environment.nodes.attachments.reserve_binding();
    for (std::size_t i = 0; i < viewport_count; ++i)
        prepared.viewport_admissions_[i] = registry.admit(viewport_records_[i]);
    phase_ = Phase::prepared;
    return prepared;
}

void NativeDirectionalShadowConstructionBlock::Admission::validate_prepared() const {
    if (!block_ || block_->phase_ != Phase::prepared || !camera_environment_ || !node_constants_ ||
        !block_->viewport_registry_ || !block_->cache_acquisition_ ||
        block_->cache_acquisition_->phase != NativeTextureCacheAcquired::Phase::not_started)
        throw std::logic_error("shadow constructor requires its active prepared block admission");
    auto& registry = *block_->viewport_registry_;
    validate_environment(*camera_environment_, *node_constants_, registry);
    for (std::size_t i = 0; i < camera_count; ++i)
        if (!scene_admissions_[i] || !lifetime_admissions_[i])
            throw std::logic_error("shadow constructor requires all eight camera binding credits");
    for (const auto& admission : viewport_admissions_)
        if (&admission.require_registry() != &registry)
            throw std::logic_error("shadow constructor requires all nine exact viewport admissions");
}

void NativeDirectionalShadowConstructionBlock::Admission::begin_execution() noexcept {
    if (!block_ || block_->phase_ != Phase::prepared) std::terminate();
    block_->phase_ = Phase::executing;
}

void NativeDirectionalShadowConstructionBlock::record_camera_retirement(void* context,
    NativeCameraReference& reference) noexcept {
    if (!context) std::terminate();
    const auto& cookie = *static_cast<const RetirementCookie*>(context);
    if (!cookie.block || cookie.index >= camera_count) std::terminate();
    auto& block = *cookie.block;
    auto& camera = block.cameras_[cookie.index];
    if ((block.phase_ != Phase::executing && block.phase_ != Phase::settled) ||
        &camera.retirement != &cookie || !camera.reference || &*camera.reference != &reference ||
        !camera.owner || &reference.camera_owner() != &*camera.owner ||
        camera.owner->phase != NativeCameraOwner::Phase::dead ||
        !camera.camera_completed || camera.retired) std::terminate();
    camera.retired = true; // only a host fact; no native read or companion reset
}

bool NativeDirectionalShadowConstructionBlock::cache_tree_quiescent(
    const NativeTextureCacheAcquired& root) noexcept {
    for (auto* acquired = &root; acquired; acquired = acquired->fallback.get()) {
        if (acquired->phase != NativeTextureCacheAcquired::Phase::not_started &&
            acquired->phase != NativeTextureCacheAcquired::Phase::complete) return false;
        if (acquired->loader.phase != NativeTextureLoadAcquired::Phase::not_started &&
            acquired->loader.phase != NativeTextureLoadAcquired::Phase::complete) return false;
        // The concrete VFS operation remains owned by each resolution call.
        // Neither a failed factory nor a begun/unreturned child can be discarded.
        for (const auto* call : {&acquired->resolution, &acquired->fallback_resolution})
            if (call->failed || (call->started && !call->returned)) return false;
    }
    return true;
}

void NativeDirectionalShadowConstructionBlock::reset_after_host_quiescence() noexcept {
    if (phase_ != Phase::settled || !viewport_registry_ || !cache_acquisition_ ||
        !cache_tree_quiescent(*cache_acquisition_)) std::terminate();
    for (const auto& camera : cameras_)
        if ((camera.reference && (!camera.owner || !camera.retired)) ||
            (!camera.reference && camera.retired) ||
            (camera.owner && camera.owner->phase != NativeCameraOwner::Phase::dead))
            std::terminate();
    for (const auto& record : viewport_records_)
        if (record.phase() != NativeViewportRegistry::Phase::unused &&
            record.phase() != NativeViewportRegistry::Phase::cancelled &&
            record.phase() != NativeViewportRegistry::Phase::retired) std::terminate();

    // All checks precede disposal. Caller has ended every host borrow and dealt
    // with native resources through their real lifetime domain. Completed cache
    // metadata owns no additional native reference; its disposal releases none.
    for (auto& camera : cameras_) camera.reference.reset();
    for (auto& camera : cameras_) {
        camera.owner.reset();
        camera.camera_completed = false;
        camera.retired = false;
    }
    for (auto& record : viewport_records_) viewport_registry_->forget_quiescent(record);
    cache_acquisition_.reset();
    viewport_registry_ = nullptr;
    base_state_ = {};
    derived_state_ = {};
    factory_state_ = {};
    // Raw lanes stay as consumed preimages until later reached native stores.
    phase_ = Phase::idle;
}

NativeDirectionalShadowConstructionBlock::~NativeDirectionalShadowConstructionBlock() noexcept {
    if (!idle_storage()) std::terminate();
}
} // namespace bsp
