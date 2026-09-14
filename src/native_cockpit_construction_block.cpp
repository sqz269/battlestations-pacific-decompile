#include "bsp/native_cockpit_construction_block.hpp"
#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {

NativeCockpitConstructionBlock::Admission::Admission(NativeCockpitConstructionBlock& block,
    NativeCameraEnvironment& environment, const NativeNodeRawConstants& constants) noexcept
    : block_(&block), camera_environment_(&environment), node_constants_(&constants) {}

NativeCockpitConstructionBlock::Admission::Admission(Admission&& other) noexcept {
    *this = std::move(other);
}

NativeCockpitConstructionBlock::Admission&
NativeCockpitConstructionBlock::Admission::operator=(Admission&& other) noexcept {
    if (this == &other) return *this;
    // Once executing, the outer capability stays in its native caller's frame.
    // Only its individual credits move into their corresponding callees.
    if ((block_ && block_->phase_ == Phase::executing) ||
        (other.block_ && other.block_->phase_ == Phase::executing)) std::terminate();
    cancel();
    block_ = std::exchange(other.block_, nullptr);
    camera_environment_ = std::exchange(other.camera_environment_, nullptr);
    node_constants_ = std::exchange(other.node_constants_, nullptr);
    scene_admission_ = std::move(other.scene_admission_);
    lifetime_admission_ = std::move(other.lifetime_admission_);
    viewport_admissions_[0] = std::move(other.viewport_admissions_[0]);
    viewport_admissions_[1] = std::move(other.viewport_admissions_[1]);
    return *this;
}

NativeCockpitConstructionBlock::Admission::~Admission() noexcept { cancel(); }

void NativeCockpitConstructionBlock::Admission::cancel() noexcept {
    if (!block_) return;
    auto& block = *block_;
    const bool rollback = block.phase_ == Phase::preparing || block.phase_ == Phase::prepared;
    if ((!rollback && block.phase_ != Phase::executing) || !block.viewport_registry_)
        std::terminate();
    if (rollback && (block.camera_owner_ || block.camera_reference_ || block.camera_retired_))
        std::terminate();

    scene_admission_.cancel();
    lifetime_admission_.cancel();
    viewport_admissions_[0].cancel();
    viewport_admissions_[1].cancel();
    if (rollback) {
        for (auto& record : block.viewport_records_) {
            const auto phase = record.phase();
            if (phase != NativeViewportRegistry::Phase::unused &&
                phase != NativeViewportRegistry::Phase::cancelled) std::terminate();
            // No view was exposed during preparation. This only unlinks host
            // records; it never destroys or releases an actual viewport.
            block.viewport_registry_->forget_quiescent(record);
        }
        block.viewport_registry_ = nullptr;
        block.phase_ = Phase::idle;
    } else {
        // Native EH and raw-allocation obligations belong to the constructor.
        // Published/retired records and companions remain in stable storage.
        block.phase_ = Phase::settled;
    }
    block_ = nullptr;
    camera_environment_ = nullptr;
    node_constants_ = nullptr;
}

void NativeCockpitConstructionBlock::validate_environment(NativeCameraEnvironment& environment,
    const NativeNodeRawConstants& constants, NativeViewportRegistry& registry) {
    (void)environment.nodes.require_raw_name_pool();
    if (&constants.one_00d7a24c != &environment.viewport.one_bits_00d7a24c)
        throw std::logic_error("cockpit camera and node require the same actual D7A24C cell");
    if (&environment.nodes.attachments.scenes != &environment.nodes.scenes ||
        !environment.nodes.node_virtual_0c)
        throw std::invalid_argument("cockpit requires one canonical scene runtime and actual node type predicate");
    if (&environment.viewport_views != &registry)
        throw std::logic_error("cockpit viewport admission requires its exact installed resolver");
    if (!environment.crt.dispatch_bypass_0109dd78 || !environment.crt.except_00c27489 ||
        !environment.vtable_00d62cf0 || !environment.vtable_00d62c88)
        throw std::invalid_argument("cockpit camera requires actual CRT and table bindings");
    // The caller wires the same canonical 0108FFB0 pool owner for allocation
    // and terminal return. No allocator probe or fabricated identity check.
}

NativeCockpitConstructionBlock::Admission NativeCockpitConstructionBlock::admit(
    NativeCameraEnvironment& environment, const NativeNodeRawConstants& constants,
    NativeViewportRegistry& registry) {
    if (phase_ != Phase::idle || viewport_registry_ || camera_owner_ || camera_reference_ ||
        camera_retired_ || viewport_records_[0].phase() != NativeViewportRegistry::Phase::unused ||
        viewport_records_[1].phase() != NativeViewportRegistry::Phase::unused)
        throw std::logic_error("cockpit construction block requires idle quiescent storage");
    phase_ = Phase::preparing;
    viewport_registry_ = &registry;
    Admission prepared(*this, environment, constants);
    // The local capability rolls back every acquired reservation if a later
    // validation/reserve/admit fails, before any native constructor event.
    validate_environment(environment, constants, registry);
    prepared.scene_admission_ = environment.nodes.scenes.reserve_binding();
    prepared.lifetime_admission_ = environment.nodes.attachments.reserve_binding();
    prepared.viewport_admissions_[0] = registry.admit(viewport_records_[0]);
    prepared.viewport_admissions_[1] = registry.admit(viewport_records_[1]);
    phase_ = Phase::prepared;
    return prepared;
}

void NativeCockpitConstructionBlock::Admission::validate_prepared() const {
    if (!block_ || block_->phase_ != Phase::prepared || !camera_environment_ || !node_constants_ ||
        !block_->viewport_registry_ || !scene_admission_ || !lifetime_admission_)
        throw std::logic_error("cockpit constructor requires its active prepared block admission");
    auto& registry = *block_->viewport_registry_;
    validate_environment(*camera_environment_, *node_constants_, registry);
    if (&viewport_admissions_[0].require_registry() != &registry ||
        &viewport_admissions_[1].require_registry() != &registry)
        throw std::logic_error("cockpit constructor requires both exact viewport admissions");
}

void NativeCockpitConstructionBlock::Admission::begin_execution() noexcept {
    if (!block_ || block_->phase_ != Phase::prepared) std::terminate();
    block_->phase_ = Phase::executing;
}

void NativeCockpitConstructionBlock::record_camera_retirement(void* context,
    NativeCameraReference& reference) noexcept {
    if (!context) std::terminate();
    auto& block = *static_cast<NativeCockpitConstructionBlock*>(context);
    if ((block.phase_ != Phase::executing && block.phase_ != Phase::settled) ||
        !block.camera_reference_ || &*block.camera_reference_ != &reference ||
        !block.camera_owner_ || &reference.camera_owner() != &*block.camera_owner_ ||
        block.camera_owner_->phase != NativeCameraOwner::Phase::dead || block.camera_retired_)
        std::terminate();
    block.camera_retired_ = true; // no companion reset, native read, release or free
}

void NativeCockpitConstructionBlock::reset_after_host_quiescence() noexcept {
    if (phase_ != Phase::settled || !viewport_registry_ ||
        (camera_reference_ && (!camera_owner_ || !camera_retired_)) ||
        (!camera_reference_ && camera_retired_) ||
        (camera_owner_ && camera_owner_->phase != NativeCameraOwner::Phase::dead))
        std::terminate();
    for (const auto& record : viewport_records_) {
        const auto phase = record.phase();
        if (phase != NativeViewportRegistry::Phase::unused &&
            phase != NativeViewportRegistry::Phase::cancelled &&
            phase != NativeViewportRegistry::Phase::retired) std::terminate();
    }
    // Caller has ended every semantic host-view/cache borrow. These destructors
    // touch only retired/dead companions, never their ended native backing.
    camera_reference_.reset();
    camera_owner_.reset();
    for (auto& record : viewport_records_) viewport_registry_->forget_quiescent(record);
    viewport_registry_ = nullptr;
    camera_retired_ = false;
    phase_ = Phase::idle;
}

NativeCockpitConstructionBlock::~NativeCockpitConstructionBlock() noexcept {
    if (phase_ != Phase::idle || viewport_registry_ || camera_owner_ || camera_reference_ ||
        camera_retired_ || viewport_records_[0].phase() != NativeViewportRegistry::Phase::unused ||
        viewport_records_[1].phase() != NativeViewportRegistry::Phase::unused)
        std::terminate();
}

} // namespace bsp
