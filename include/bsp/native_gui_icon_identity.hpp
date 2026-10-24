#pragma once
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp {
struct NativeGuiIconIdentityContext {
    NativeGuiIconStorageContext& storage;
    // SAME initialized F8BDA0 process pool used by AA1410/AB6100.
    NativeGuiWidgetPool<GuiIconPoolTraits>& actual_matching_pool;
};

// Canonical metadata for ONE completed actual138h Icon in its13Ch pool slot.
// Admit only after successful full raw AB5C60/AB5D30, with all base/derived
// subobjects, backing, model resources and transitive lifetime providers live.
// The producer proves that custody and allocator provenance; checking profile,
// +04 and pool identity cannot prove the allocation or complete construction.
// No logical GuiWidgetOwner cast, typed state mirror, extra count or retain.
//
// Borrows the SAME actual+04, using storage.actual_owners as the sole registry.
// Admission failure leaves the raw allocation untouched and caller-owned.
// After admission every zero/explicit scalar passes through this companion.
// Serialize dispatch and exclude slot reuse until matching unbind completes.
// Context, this companion, same registry/pool/delayed/publications and full
// graph/provider custody outlive calls, normal drain and failure disposition.
//
// Current raw Icon class/child transport must be genuinely supplied by that
// lifetime context. The existing closed Group/ClipBox dispatcher rejects Icon;
// this class does not expand its domain or invent page/typefactory authority.
class NativeGuiIconReference final : public RenderCommandReference {
public:
    NativeGuiIconReference(void* actual_icon, NativeGuiIconIdentityContext&);
    ~NativeGuiIconReference() override;
    NativeGuiIconReference(const NativeGuiIconReference&) = delete;
    NativeGuiIconReference& operator=(const NativeGuiIconReference&) = delete;
    NativeGuiIconReference(NativeGuiIconReference&&) = delete;
    NativeGuiIconReference& operator=(NativeGuiIconReference&&) = delete;

    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    NativeGuiWidgetLifetimeContext& lifetime_context() const noexcept { return context_.storage.lifetime; }
    const NativeGuiIconDestroyAcquired& destruction() const noexcept { return destruction_; }
    // Already-observed zero only: genuine CURRENT0 BD30E0 -> CURRENT4 AB6100(1).
    // No protective retain, deferred callback, resurrection or extra decrement.
    void release_zero_references() noexcept override;
    // Direct native scalar has NO count gate. Borrow stable/disjoint flags;
    // low byte is read only after full dtor. Flags0 retires payload metadata
    // but leaves physical storage with caller; flags1 returns its original slot.
    // Any entered exception retires/unbinds metadata; preserve the partial raw
    // graph/providers independently and never retry already-entered native work.
    void* delete_scalar_00ab6100(const volatile std::uint32_t& actual_flags_slot);
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiIconIdentityContext& context_;
    Phase phase_{Phase::bound};
    NativeGuiIconDestroyAcquired destruction_;
    void retire() noexcept;
};
} // namespace bsp
