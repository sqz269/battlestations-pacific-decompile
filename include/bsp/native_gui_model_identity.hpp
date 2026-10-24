#pragma once
#include "bsp/native_gui_model_storage.hpp"

namespace bsp {
struct NativeGuiModelIdentityContext {
    NativeGuiModelStorageContext& model;
    const volatile std::uint32_t* profile_00d5cf38; // genuine first two cells minimum
};
// Canonical companion for one completed SAME GUI class9 actual11Ch/120h slot,
// borrowing its existing atomic+04. This is distinct from renderer Model
// NativeModelReference and creates no native count, wrapper, Model or tree.
class NativeGuiModelWidgetReference final : public RenderCommandReference {
public:
    NativeGuiModelWidgetReference(void* actual_widget, NativeGuiModelIdentityContext&);
    ~NativeGuiModelWidgetReference() override;
    NativeGuiModelWidgetReference(const NativeGuiModelWidgetReference&) = delete;
    NativeGuiModelWidgetReference& operator=(const NativeGuiModelWidgetReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    bool failed() const noexcept;
    void release_zero_references() noexcept override;
    // Direct ACC880, no decrement/count test. Unbind opaque key only after
    // successful scalar (flags0 keeps slot). Failure keeps canonical binding,
    // SAME raw storage/providers/record and forbids retry/reuse/dependency
    // drain. No RAII fallback or unfinished-resource recovery is fabricated.
    void* delete_scalar_00acc880(const volatile std::uint32_t& actual_flags_slot);
    const NativeGuiModelStorageAcquired& terminal_observation() const noexcept { return terminal_; }
private:
    enum class Phase { bound, destroying, failed, retired };
    void* const identity_;
    NativeGuiModelIdentityContext& context_;
    Phase phase_{Phase::bound};
    NativeGuiModelStorageAcquired terminal_;
};
} // namespace bsp
