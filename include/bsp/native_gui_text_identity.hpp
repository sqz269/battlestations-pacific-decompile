#pragma once
#include "bsp/native_render_context.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {
class GuiWidgetOwner;
class GuiTextChildDeletion;
struct NativeMaterialStorage;

// ONLY the proven first eight bytes of the SAME AB79E0/AB78A0 Text pool slot.
// +08..+1F3 and allocator metadata +1F4 are untouched. This is not a raw Text
// object, native strings/containers, a callable vtable or a complete native ABI.
struct NativeGuiTextIdentityPrefix {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
};
static_assert(sizeof(NativeGuiTextIdentityPrefix) == 8);
static_assert(offsetof(NativeGuiTextIdentityPrefix, references_04) == 4);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);

// Placement fragments: CEB130 -> fresh +04=1 -> D5C130, before any subsequent
// base field/container/clone effects. Copy does NOT copy the source count.
// Caller supplies a fresh aligned slot and executes the rest of the same base
// constructor; these do not claim that constructor has completed.
NativeGuiTextIdentityPrefix* initialize_native_gui_widget_identity_00aa9390_fragment(void*) noexcept;
NativeGuiTextIdentityPrefix* initialize_native_gui_widget_identity_00aa9520_fragment(void*) noexcept;
// After successful base construction, BEFORE any derived field/string effects.
void publish_native_gui_text_identity_00ab9650_fragment(NativeGuiTextIdentityPrefix&) noexcept;
void publish_native_gui_text_identity_00abb2c0_fragment(NativeGuiTextIdentityPrefix&) noexcept;

// Call at the corresponding canonical lifetime phases, including explicit
// scalar deletion: Text at AB826F, base at AA9752 before scene releases, and
// D5C104 -> BD30F0/CEB130 at AA999B..AA99A1. None changes the count.
void begin_native_gui_text_identity_destruction_00ab8250_fragment(NativeGuiTextIdentityPrefix&) noexcept;
void begin_native_gui_widget_identity_destruction_00aa9730_fragment(NativeGuiTextIdentityPrefix&) noexcept;
void finish_native_gui_widget_identity_destruction_00aa9730_fragment(NativeGuiTextIdentityPrefix&) noexcept;

// One non-owning canonical companion borrowing the actual prefix+04. Bind in
// the SAME NativeRenderActualOwners domain used by the cursor material. Owner,
// prefix and deletion transport must survive until their terminal callback.
// No hidden creator retain/release, second count, or alternate widget owner.
// Explicit scalar deletion can retire this companion at a NONZERO count:
// native AB8EE0 does not decrement/test +04 before destruction/pool return.
class NativeGuiTextIdentityReference final : public RenderCommandReference {
public:
    NativeGuiTextIdentityReference(NativeGuiTextIdentityPrefix&, GuiWidgetOwner&,
        GuiTextChildDeletion&);
    ~NativeGuiTextIdentityReference() override = default;
    NativeGuiTextIdentityReference(const NativeGuiTextIdentityReference&) = delete;
    NativeGuiTextIdentityReference& operator=(const NativeGuiTextIdentityReference&) = delete;
    NativeGuiTextIdentityPrefix& storage() noexcept { return storage_; }
    GuiWidgetOwner& canonical_owner() noexcept { return owner_; }
    // CURRENT D5C6C8 maps BD30E0 -> AB8EE0(flags1), concretely invoking the
    // existing same-owner child-deletion implementation. The factory owns
    // prefix profile hooks, lookup retirement and its existing AB75A0 return.
    // Unsupported profiles, unfinished construction, reentry or missing flags1
    // allocation transport terminate through the existing noexcept contract.
    // There is no null/no-op terminal fallback or storage access after delete.
    void release_zero_references() noexcept override;
private:
    NativeGuiTextIdentityPrefix& storage_;
    GuiWidgetOwner& owner_;
    GuiTextChildDeletion& deletion_;
};

// Actual B18A40 on this identity, using the existing material implementation.
// Old retained owner releases BEFORE new publication/increment, equal too.
// Caller guarantees incoming identity survives that callback; no protective
// retain or post-callback validation changes native ordering.
void bind_retained_native_gui_text_parameter_owner_00b18a40(
    NativeMaterialStorage&, NativeGuiTextIdentityReference&, NativeRenderActualOwners&);
} // namespace bsp
