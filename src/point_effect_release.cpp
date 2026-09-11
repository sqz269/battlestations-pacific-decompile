#include "bsp/point_effect_release.hpp"
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <stdexcept>

namespace bsp {

PointEffectReleaseRuntime::PointEffectReleaseRuntime(
    NativeFrameJobOwnerStorage* volatile& frame, NativeLiveEffectManagerStorage* volatile& live,
    EffectManager* volatile& lock, SingletonLifetimeDomain& domain,
    NativeFrameJobLifetimeBindings& bindings)
    : frame_(frame), live_(live), deletion_lock_(lock), domain_(domain),
      frame_bindings_(bindings), deletion_access_(domain) {}
PointEffectReleaseRuntime::~PointEffectReleaseRuntime() {
    if (!references_.empty()) std::terminate();
}
NativeFrameJobOwnerStorage* PointEffectReleaseRuntime::frame_pool_004c1130() {
    return native_frame_job_singleton_004c1130(frame_, domain_, frame_bindings_);
}
bool PointEffectReleaseRuntime::is_dispatching_current_10(const NativeFrameJobPoolStorage& pool) const {
    return frame_bindings_.execution.is_dispatching_current_10(pool);
}
NativeLiveEffectManagerStorage* PointEffectReleaseRuntime::live_manager_004d1100() {
    return live_effect_manager_singleton_004d1100(live_, domain_);
}
void PointEffectReleaseRuntime::enqueue_00868010(NativeLiveEffectManagerStorage& manager, void* raw) {
    enqueue_effect_deletion_00868010(manager, raw, deletion_lock_, deletion_access_);
}
void PointEffectReleaseRuntime::bind(NativePointEffectReference& reference) {
    for (auto* existing : references_)
        if (&existing->storage() == &reference.storage())
            throw std::invalid_argument("Point effect already has its canonical reference binding");
    references_.push_back(&reference);
}
void PointEffectReleaseRuntime::unbind(NativePointEffectReference& reference) noexcept {
    const auto found = std::find(references_.begin(), references_.end(), &reference);
    if (found == references_.end()) std::terminate();
    references_.erase(found);
}
NativePointEffectReference& PointEffectReleaseRuntime::find(void* raw) {
    for (auto* reference : references_)
        if (&reference->storage() == raw) return *reference;
    throw std::invalid_argument("Missing actual point effect scalar/reference binding");
}
RenderCommandReference& PointEffectReleaseRuntime::resolve_actual(void* raw) {
    return find(raw);
}
void PointEffectReleaseRuntime::scalar_delete_current_04(void* raw, std::uint32_t flags) {
    (void)find(raw).scalar_delete_current_04(flags);
}
void PointEffectReleaseRuntime::invalid_parameter_00bf6713() {
    _invalid_parameter_noinfo();
}

void release_point_effect_instance_008683e0(void* raw, PointEffectReleaseRuntime& runtime) {
    auto* const frame = runtime.frame_pool_004c1130();
    if (!runtime.is_dispatching_current_10(frame->pool_04)) {
        if (raw) runtime.scalar_delete_current_04(raw, 1);
    } else {
        auto* const live = runtime.live_manager_004d1100();
        runtime.enqueue_00868010(*live, raw);
    }
}

NativePointEffectReference::NativePointEffectReference(PointEffectInstanceStorage& effect,
    PointEffectReleaseRuntime& runtime, PointEffectTeardownBindings teardown,
    const volatile std::uint32_t* table, NativePointEffectCompanionDisposal disposal)
    : RenderCommandReference(effect.references_04), effect_(effect), runtime_(runtime),
      teardown_(teardown), table_(table), disposal_(disposal) {
    if (!table || !disposal.retire || effect.original_vtable_identity_00 != 0x00d0d3ecu ||
        table[0] != 0x008683e0u || table[1] != 0x00867ce0u ||
        effect.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("Point effect reference requires the actual owner, table and retirement binding");
    runtime_.bind(*this);
}
NativePointEffectReference::~NativePointEffectReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativePointEffectReference::require_slot(std::size_t slot, std::uint32_t address) const {
    if (phase_ != Phase::bound || effect_.original_vtable_identity_00 != 0x00d0d3ecu ||
        table_[slot] != address)
        throw std::invalid_argument("Missing current point effect virtual dispatch profile");
}
void NativePointEffectReference::release_zero_references() noexcept {
    require_slot(0, 0x008683e0u);
    release_point_effect_instance_008683e0(&effect_, runtime_);
    // Immediate scalar dispatch may have freed the storage AND this companion.
}
void NativePointEffectReference::retire(bool freed) noexcept {
    const auto disposal = disposal_;
    runtime_.unbind(*this);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this, freed);
}
PointEffectInstanceStorage* NativePointEffectReference::scalar_delete_current_04(std::uint32_t flags) {
    require_slot(1, 0x00867ce0u);
    auto* const original = &effect_;
    phase_ = Phase::destroying;
    try {
        (void)delete_point_effect_instance_00867ce0(original, flags, teardown_);
    } catch (...) {
        // Recovered member unwind has completed; native raw storage was not
        // freed. Retire only the unusable host association, then propagate.
        retire(false);
        throw;
    }
    retire((flags & 1u) != 0);
    return original; // captured pointer value only; no access after retirement
}
} // namespace bsp
