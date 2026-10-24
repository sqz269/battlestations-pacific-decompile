#pragma once
#include "bsp/native_gui_anim_icon_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
struct NativeGuiAnimIconIdentityContext {
    NativeGuiAnimIconStorageContext& storage;
    NativeGuiWidgetPool<GuiAnimIconPoolTraits>& actual_matching_pool;
};
// Canonical metadata for the SAME completed144h AnimIcon/148h F8C1D4 slot.
// Full raw constructor, genuine allocator provenance, subobjects/backing and
// all transitive providers are caller proofs, not implied by profile/+04 checks.
// Borrows actual+04 in storage.icon.actual_owners; no native store/retain, extra
// count, owner map, logical widget cast, separate Icon identity or frame mirror.
// Admit this ONE derived companion after full construction, not an Icon
// companion as well. Every later scalar/zero path must use this companion.
//
// Retain/serialize companion, context, same registry/pool, raw child/class/timed/
// parent/model providers, strings, delayed service/publications through calls,
// full process drain and failure disposition. Prevent slot reuse until opaque
// unbind completes. Existing closed Group/ClipBox tree transport rejects this
// type; future same-tree admission remains a genuine external prerequisite.
class NativeGuiAnimIconReference final : public RenderCommandReference {
public:
    NativeGuiAnimIconReference(void* actual_anim_icon, NativeGuiAnimIconIdentityContext&);
    ~NativeGuiAnimIconReference() override;
    NativeGuiAnimIconReference(const NativeGuiAnimIconReference&) = delete;
    NativeGuiAnimIconReference& operator=(const NativeGuiAnimIconReference&) = delete;
    NativeGuiAnimIconReference(NativeGuiAnimIconReference&&) = delete;
    NativeGuiAnimIconReference& operator=(NativeGuiAnimIconReference&&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    bool failed() const noexcept;
    NativeGuiWidgetLifetimeContext& lifetime_context() const noexcept { return context_.storage.icon.lifetime; }
    const NativeGuiAnimIconDestroyAcquired& destruction() const noexcept { return destruction_; }
    // Already-zero CURRENT0 BD30E0 freshly selects CURRENT4 AA1D50(1).
    // No extra decrement, protection retain, resurrection or deferred terminal.
    void release_zero_references() noexcept override;
    // Direct scalar has no count gate. Flags are stable/disjoint; low byte read
    // only after full derived/Icon/base destruction. Flags0 leaves physical slot
    // with caller. Success retires/unbinds metadata. Any entered failure keeps
    // this canonical binding in a failed phase: no retry, normal destruction or
    // identity reuse; retain the partial graph/providers for process disposition.
    void* delete_scalar_00aa1d50(const volatile std::uint32_t& actual_flags_slot);
private:
    enum class Phase { bound, destroying, failed, retired };
    void* const identity_;
    NativeGuiAnimIconIdentityContext& context_;
    Phase phase_{Phase::bound};
    NativeGuiAnimIconDestroyAcquired destruction_;
    void retire() noexcept;
};
} // namespace bsp
