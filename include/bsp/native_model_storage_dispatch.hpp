#pragma once
#include "bsp/native_model_storage_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include <optional>

namespace bsp {
// Aliases of ONE independently retained Model record, not ownership copies.
// Bind before allocation: actual is null, reference empty, all records fresh.
// The real producer later fills these SAME cells (Text85 publishes188 before
// canonical bind). The optional reference MUST be constructed using precisely
// model/construction/lifetime_frame/lifetime below; diagnostics cannot prove it.
struct NativeModelStorageDispatchContext {
    NativeModelStorageIdentityContext& model;
    void* const& actual;
    std::optional<NativeModelStorageReference>& reference;
    const NativeModelStorageConstructAcquired& construction;
    NativeModelStorageLifetimeFrame& lifetime_frame;
    NativeModelStorageLifetimeAcquired& lifetime;
    NativeNodeTreeRetirementFrame& release_frame;
    NativeNodeTreeRetirementAcquired& release;
};
struct NativeModelStorageDispatchCallAcquired {
    enum class Phase : std::uint8_t { fresh, entered, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t captured_target{}, admitted_profile{};
    bool body_entered{}, body_returned{};
};
struct NativeModelStorageDispatchAcquired {
    bool bound{}, poisoned{};
    NativeModelStorageDispatchCallAcquired current18, current0;
    bool current04_entered{}, current04_returned{};
    std::uint32_t captured_current04_profile{}, captured_current04_target{};
};

// Private DEFAULT-Model provider for existing B6DFA0/B6F310 and BD30E0. This
// supplies real bodies, never a table-only/no-op/legacy-family implementation.
// No hierarchy, attachment, scene, retained owner or point-light publications
// are admitted. Mandatory geometry180 uses its real canonical member family.
// Bind into the SAME model.storage.node.trees.dispatch before native entry.
// All world/scene/type/recursive-frame/string/pool/import/member providers must
// already be genuine; this class neither manufactures nor validates their code.
class NativeModelStorageDispatch final : public NativeNodeTreeRetirementDispatch,
    private NativeRefCountedDeleteCalls {
public:
    explicit NativeModelStorageDispatch(NativeModelStorageDispatchAcquired&) noexcept;
    ~NativeModelStorageDispatch() override;
    NativeModelStorageDispatch(const NativeModelStorageDispatch&) = delete;
    NativeModelStorageDispatch& operator=(const NativeModelStorageDispatch&) = delete;

    // Pure one-time metadata binding. No slot, native getter, table/import
    // value read, canonical reference, retain, release or cleanup is required.
    void bind(NativeModelStorageDispatchContext&);
    // Pure binding identity only; no native access or dispatch permission.
    bool matches(const NativeModelStorageDispatchContext&,
        const NativeModelStorageDispatchAcquired&) const noexcept;
    const volatile std::uint32_t* resolve_profile(std::uint32_t) const override;
    void invoke_virtual18(std::uint32_t captured_target, void* actual) override;
    void invoke_virtual0(std::uint32_t captured_target, void* actual,
        RenderCommandReference& canonical_owner) override;
private:
    void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t flags) override;
    NativeModelStorageDispatchContext& context() const;
    NativeModelStorageReference& require_live(void*, bool observed_zero);
    void require_completed_scalar() const;
    NativeModelStorageDispatchAcquired& acquired_;
    NativeModelStorageDispatchContext* context_{};
    const volatile std::uint32_t* profile_{};
};

// One current18/zero/scalar invocation per actual Model lifetime. Recursive
// reuse rejects before another body call; distinct Models need distinct whole
// prepared graphs. Entered failure poisons/retains this provider and ALL backing.
// A zero-dispatch escape terminates, matching481's canonical noexcept policy;
// rejection there may follow earlier native mutations/decrement, never rollback.
// After any return that may end the payload, completion reads metadata ONLY.
// A retired companion is not cleanup/physical-return proof. Retain the entire
// graph for surviving credits, flags0 slot custody, or any entered failure.
// Serialize actual lifetime, exclude slot reuse/async mutation and prove opaque
// provider/derived extents. Known-region checks prove neither transitive backing
// nor callback correctness. Source MSVC Win32 only; no Original ABI/FH3/SEH,
// automatic graph disposal, shared Page routing or GUI completion is supplied.
} // namespace bsp
