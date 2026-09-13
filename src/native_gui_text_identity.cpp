#include "bsp/native_gui_text_identity.hpp"
#include "bsp/gui_text_child_lifetime.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t reference_profile = 0x00ceb130;
constexpr std::uint32_t widget_profile = 0x00d5c130;
constexpr std::uint32_t text_profile = 0x00d5c6c8;

NativeGuiTextIdentityPrefix* initialize_prefix(void* slot) noexcept {
    auto* prefix = ::new (slot) NativeGuiTextIdentityPrefix;
    prefix->native_vtable_00 = reference_profile;
    prefix->references_04.store(1, std::memory_order_relaxed);
    prefix->native_vtable_00 = widget_profile;
    return prefix;
}

class SameTextDelete final : public NativeRefCountedDeleteCalls {
public:
    SameTextDelete(NativeGuiTextIdentityPrefix& prefix, GuiWidgetOwner& owner,
        GuiTextChildDeletion& deletion) noexcept
        : prefix_(prefix), owner_(owner), deletion_(deletion) {}
    void delete_vslot04(void* actual, std::uint32_t profile, std::uint32_t flags) override {
        // D5C6C8 bytes: E0 30 BD 00 E0 8E AB 00. Base profile scalar bodies
        // differ; never reinterpret them as Text or silently suppress a release.
        if (actual != &prefix_ || profile != text_profile || flags != 1)
            throw std::logic_error("Text terminal requires current D5C6C8 -> AB8EE0(flags1)");
        deletion_.delete_text_child_virtual4(owner_.layout(), flags);
        // The callback can retire owner_, prefix_ AND the originating companion.
    }
private:
    NativeGuiTextIdentityPrefix& prefix_;
    GuiWidgetOwner& owner_;
    GuiTextChildDeletion& deletion_;
};
} // namespace

NativeGuiTextIdentityPrefix* initialize_native_gui_widget_identity_00aa9390_fragment(void* slot) noexcept {
    return initialize_prefix(slot); // AA93AA, AA93B5, AA93CB.
}
NativeGuiTextIdentityPrefix* initialize_native_gui_widget_identity_00aa9520_fragment(void* slot) noexcept {
    return initialize_prefix(slot); // AA953B, AA9546, AA9551.
}
void publish_native_gui_text_identity_00ab9650_fragment(NativeGuiTextIdentityPrefix& prefix) noexcept {
    prefix.native_vtable_00 = text_profile; // AB9678.
}
void publish_native_gui_text_identity_00abb2c0_fragment(NativeGuiTextIdentityPrefix& prefix) noexcept {
    prefix.native_vtable_00 = text_profile; // ABB2FD, before first string copy.
}
void begin_native_gui_text_identity_destruction_00ab8250_fragment(NativeGuiTextIdentityPrefix& prefix) noexcept {
    prefix.native_vtable_00 = text_profile; // AB826F, before shader/shadow release.
}
void begin_native_gui_widget_identity_destruction_00aa9730_fragment(NativeGuiTextIdentityPrefix& prefix) noexcept {
    prefix.native_vtable_00 = widget_profile; // AA9752, before AA9760 -> AA8320.
}
void finish_native_gui_widget_identity_destruction_00aa9730_fragment(NativeGuiTextIdentityPrefix& prefix) noexcept {
    prefix.native_vtable_00 = 0x00d5c104; // AA999B.
    destroy_native_ref_counted_base_00bd30f0(&prefix); // AA99A1; count unchanged.
}

NativeGuiTextIdentityReference::NativeGuiTextIdentityReference(
    NativeGuiTextIdentityPrefix& prefix, GuiWidgetOwner& owner, GuiTextChildDeletion& deletion)
    : RenderCommandReference(prefix.references_04), storage_(prefix), owner_(owner), deletion_(deletion) {
    if (owner.layout().type != GuiWidgetType::Text || owner.layout().transform.type_id != 3 ||
        prefix.references_04.load(std::memory_order_relaxed) <= 0 ||
        (prefix.native_vtable_00 != widget_profile && prefix.native_vtable_00 != text_profile))
        throw std::invalid_argument("Text identity requires its freshly produced prefix and same type3 owner");
}
void NativeGuiTextIdentityReference::release_zero_references() noexcept {
    if (storage_.references_04.load(std::memory_order_relaxed) != 0 ||
        storage_.native_vtable_00 != text_profile) std::terminate();
    SameTextDelete calls(storage_, owner_, deletion_);
    invoke_native_ref_counted_delete_00bd30e0(&storage_, calls);
    // No member access: canonical deletion can have destroyed this companion.
}
void bind_retained_native_gui_text_parameter_owner_00b18a40(NativeMaterialStorage& material,
    NativeGuiTextIdentityReference& identity, NativeRenderActualOwners& owners) {
    auto* const raw = &identity.storage();
    if (raw->native_vtable_00 != text_profile ||
        raw->references_04.load(std::memory_order_relaxed) <= 0 ||
        &identity.reference_count != &raw->references_04 || &owners.resolve_actual(raw) != &identity)
        throw std::invalid_argument("cursor material requires its registered actual Text prefix/count/companion");
    set_native_material_parameter_owner_00b18a40(material, raw, 1, owners);
}
} // namespace bsp
