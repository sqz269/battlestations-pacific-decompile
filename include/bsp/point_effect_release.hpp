#pragma once

#include "bsp/live_effect_manager_lifetime.hpp"
#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/point_effect_teardown.hpp"
#include <vector>

namespace bsp {

class NativePointEffectReference;

// Canonical host association for actual point-effect owners. Stores borrowed
// companions only: no second effects container, owner, count or pending queue.
// All three publications and the singleton domain are the application's actual
// cells/domain. The deletion-lock adapter borrows that same domain. References,
// runtime, actual tables and frame entry/job bindings survive their native uses.
class PointEffectReleaseRuntime final : public NativeRenderActualOwners,
    public EffectDeletionDispatch {
public:
    PointEffectReleaseRuntime(NativeFrameJobOwnerStorage* volatile& actual_0109cf08,
        NativeLiveEffectManagerStorage* volatile& actual_00f8765c,
        EffectManager* volatile& actual_00f87654, SingletonLifetimeDomain&,
        NativeFrameJobLifetimeBindings&);
    ~PointEffectReleaseRuntime() override;
    PointEffectReleaseRuntime(const PointEffectReleaseRuntime&) = delete;
    PointEffectReleaseRuntime& operator=(const PointEffectReleaseRuntime&) = delete;

    NativeFrameJobOwnerStorage* frame_pool_004c1130();
    bool is_dispatching_current_10(const NativeFrameJobPoolStorage&) const;
    NativeLiveEffectManagerStorage* live_manager_004d1100();
    void enqueue_00868010(NativeLiveEffectManagerStorage&, void* actual_effect);
    // Pure association lookup; the caller has already decremented actual+04.
    RenderCommandReference& resolve_actual(void* actual_effect) override;
    // Scalar lookup is independent of the count: no additional decrement or
    // virtual0 call. This permits forced scalar deletion at a positive count.
    void scalar_delete_current_04(void* actual_effect, std::uint32_t flags) override;
    void invalid_parameter_00bf6713() override;
private:
    friend class NativePointEffectReference;
    void bind(NativePointEffectReference&);
    void unbind(NativePointEffectReference&) noexcept;
    NativePointEffectReference& find(void* actual_effect);
    NativeFrameJobOwnerStorage* volatile& frame_;
    NativeLiveEffectManagerStorage* volatile& live_;
    EffectManager* volatile& deletion_lock_;
    SingletonLifetimeDomain& domain_;
    NativeFrameJobLifetimeBindings& frame_bindings_;
    ConcreteEffectManagerLifetimeAccess deletion_access_;
    std::vector<NativePointEffectReference*> references_;
};

// Complete8683E0..868415: ECX captured nullable raw effect, RET. Always get
// frame singleton, then secondary+04 CURRENT virtual10/AL. Inactive: current
// effect virtual04(flags1) only if nonnull. Active: get actual4D1100 manager
// then enqueue captured raw pointer through868010, including null. No retain,
// decrement, direct thread-ID test, pending flush or exception cleanup is added.
void release_point_effect_instance_008683e0(void* actual_effect, PointEffectReleaseRuntime&);

struct NativePointEffectCompanionDisposal {
    void* context;
    // Retire host associations/companion only. Called after removing this entry
    // from the canonical lookup, on successful scalar deletion AND failed
    // teardown. May delete the companion; no subsequent access occurs.
    // storage_freed reports successful native flags1 free. The callback must
    // never free/release the native effect, node or other native owners itself.
    void (*retire)(void*, NativePointEffectReference&, bool storage_freed) noexcept;
};

// One stable canonical companion for the actual114h owner's existing+04 atomic.
// Construction validates/binds only; no initialization or retain. Current
// D0D3EC slots8683E0/867CE0 are required. A zero release may leave the SAME owner
// and companion alive in the native pending list until scalar dispatch occurs.
class NativePointEffectReference final : public RenderCommandReference {
public:
    NativePointEffectReference(PointEffectInstanceStorage&, PointEffectReleaseRuntime&,
        PointEffectTeardownBindings, const volatile std::uint32_t* actual_table_00d0d3ec,
        NativePointEffectCompanionDisposal);
    ~NativePointEffectReference() override;
    NativePointEffectReference(const NativePointEffectReference&) = delete;
    NativePointEffectReference& operator=(const NativePointEffectReference&) = delete;
    PointEffectInstanceStorage& storage() noexcept { return effect_; }
    void release_zero_references() noexcept override;
    PointEffectInstanceStorage* scalar_delete_current_04(std::uint32_t flags);
private:
    enum class Phase { bound, destroying, retired };
    PointEffectInstanceStorage& effect_;
    PointEffectReleaseRuntime& runtime_;
    PointEffectTeardownBindings teardown_;
    const volatile std::uint32_t* table_;
    NativePointEffectCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_slot(std::size_t slot, std::uint32_t address) const;
    void retire(bool storage_freed) noexcept;
};

// New C++ ABI. Intrusive zero callbacks use the established nonthrowing domain;
// missing profiles/associations or a thrown native operation there terminate.
// Explicit scalar/queue paths propagate failure without freeing native storage.
// Domain shutdown callbacks must keep these bindings alive while dispatching
// the actual frame/live-manager/deletion-lock scalar destructors.
} // namespace bsp
